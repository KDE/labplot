/*
	File                 : DropValuesDialog.cpp
	Project              : LabPlot
	Description          : Dialog for dropping and masking values in columns
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2015-2025 Alexander Semke <alexander.semke@web.de>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "DropValuesDialog.h"
#include "backend/core/Settings.h"
#include "backend/core/column/Column.h"
#include "backend/core/datatypes/DateTime2StringFilter.h"
#include "backend/lib/macros.h"
#include "backend/spreadsheet/Spreadsheet.h"

#include <QDialogButtonBox>
#include <QPushButton>
#include <QThreadPool>
#include <QWindow>

#include <KLocalizedString>
#include <KMessageBox>
#include <KWindowConfig>

enum class Operator { EqualTo, NotEqualTo, BetweenIncl, BetweenExcl, GreaterThan, GreaterThanEqualTo, LessThan, LessThanEqualTo };
enum class OperatorText { EqualTo, NotEqualTo, StartsWith, EndsWith, Contain, NotContain };

/*!
	\class DropValuesDialog
	\brief Dialog for generating values from a mathematical function.

	\ingroup frontend
 */
DropValuesDialog::DropValuesDialog(Spreadsheet* s, bool mask, QWidget* parent)
	: QDialog(parent)
	, m_spreadsheet(s)
	, m_mask(mask) {
	ui.setupUi(this);
	setAttribute(Qt::WA_DeleteOnClose);

	QStringList items;
	items << i18n("Equal to");
	items << i18n("Not Equal to");
	items << i18n("Between (Incl. End Points)");
	items << i18n("Between (Excl. End Points)");
	items << i18n("Greater than");
	items << i18n("Greater than or Equal to");
	items << i18n("Less than");
	items << i18n("Less than or Equal to");

	ui.cbOperator->addItems(items);
	ui.cbOperatorDateTime->addItems(items);

	ui.cbOperatorText->addItem(i18n("Equal To"));
	ui.cbOperatorText->addItem(i18n("Not Equal To"));
	ui.cbOperatorText->addItem(i18n("Starts With"));
	ui.cbOperatorText->addItem(i18n("Ends With"));
	ui.cbOperatorText->addItem(i18n("Contains"));
	ui.cbOperatorText->addItem(i18n("Does Not Contain"));

	ui.leValue1->setValidator(new QDoubleValidator(ui.leValue1));
	ui.leValue2->setValidator(new QDoubleValidator(ui.leValue2));

	auto* btnBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
	ui.verticalLayout->addWidget(btnBox);
	m_okButton = btnBox->button(QDialogButtonBox::Ok);

	connect(btnBox->button(QDialogButtonBox::Cancel), &QPushButton::clicked, this, &DropValuesDialog::close);

	if (m_mask) {
		m_okButton->setText(i18n("&Mask"));
		m_okButton->setToolTip(i18n("Mask values in the specified region"));
		setWindowTitle(i18nc("@title:window", "Mask Values"));
		ui.chbDeleteRows->setVisible(false);
	} else {
		m_okButton->setText(i18n("&Drop"));
		m_okButton->setToolTip(i18n("Drop values in the specified region"));
		setWindowTitle(i18nc("@title:window", "Drop Values"));
		ui.chbDeleteRows->setToolTip(i18n("Delete entire rows in the spreadsheet if the condition is met."));
	}

	connect(ui.cbOperator, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &DropValuesDialog::operatorChanged);
	connect(ui.cbOperatorDateTime, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &DropValuesDialog::operatorDateTimeChanged);
	connect(m_okButton, &QPushButton::clicked, this, &DropValuesDialog::okClicked);
	connect(btnBox, &QDialogButtonBox::accepted, this, &DropValuesDialog::accept);
	connect(btnBox, &QDialogButtonBox::rejected, this, &DropValuesDialog::reject);

	// restore saved settings if available
	KConfigGroup conf = Settings::group(QLatin1String("DropValuesDialog"));
	ui.leValue1->setText(conf.readEntry("Value1", QString()));
	ui.leValue2->setText(conf.readEntry("Value2", QString()));
	ui.cbOperator->setCurrentIndex(conf.readEntry("Operator", 0));
	operatorChanged(ui.cbOperator->currentIndex());

	ui.leValueText->setText(conf.readEntry("ValueText", QString()));
	ui.cbOperatorText->setCurrentIndex(conf.readEntry("OperatorText", 0));

	qint64 now = QDateTime::currentDateTime().toMSecsSinceEpoch();
	ui.dteValue1->setMSecsSinceEpochUTC(conf.readEntry("Value1DateTime", now));
	ui.dteValue2->setMSecsSinceEpochUTC(conf.readEntry("Value2DateTime", now));
	ui.cbOperatorDateTime->setCurrentIndex(conf.readEntry("OperatorDateTime", 0));
	operatorDateTimeChanged(ui.cbOperatorDateTime->currentIndex());

	ui.chbDeleteRows->setChecked(conf.readEntry("DeleteRows", false));

	create(); // ensure there's a window created
	if (conf.exists()) {
		KWindowConfig::restoreWindowSize(windowHandle(), conf);
		resize(windowHandle()->size()); // workaround for QTBUG-40584
	} else
		resize(QSize(400, 0).expandedTo(minimumSize()));
}

DropValuesDialog::~DropValuesDialog() {
	// save the current settings
	KConfigGroup conf = Settings::group(QLatin1String("DropValuesDialog"));
	conf.writeEntry("Operator", ui.cbOperator->currentIndex());
	conf.writeEntry("Value1", ui.leValue1->text());
	conf.writeEntry("Value2", ui.leValue2->text());
	conf.writeEntry("OperatorText", ui.cbOperatorText->currentIndex());
	conf.writeEntry("ValueText", ui.leValueText->text());
	conf.writeEntry("OperatorDateTime", ui.cbOperatorDateTime->currentIndex());
	conf.writeEntry("Value1DateTime", ui.dteValue1->dateTime().toMSecsSinceEpoch());
	conf.writeEntry("Value2DateTime", ui.dteValue2->dateTime().toMSecsSinceEpoch());
	conf.writeEntry("DeleteRows", ui.chbDeleteRows->isChecked());
	KWindowConfig::saveWindowSize(windowHandle(), conf);
}

void DropValuesDialog::setColumns(const QVector<Column*>& columns) {
	m_columns = columns;

	for (auto* col : m_columns) {
		if (col->isNumeric()) {
			m_hasNumeric = true;
			break;
		}
	}

	for (auto* col : m_columns) {
		if (col->columnMode() == AbstractColumn::ColumnMode::Text) {
			m_hasText = true;
			break;
		}
	}

	QString dateTimeFormat;
	for (auto* col : m_columns) {
		if (col->columnMode() == AbstractColumn::ColumnMode::DateTime) {
			m_hasDateTime = true;
			auto* filter = static_cast<DateTime2StringFilter*>(col->outputFilter());
			dateTimeFormat = filter->format();
			break;
		}
	}

	ui.frameNumeric->setVisible(m_hasNumeric);
	ui.frameText->setVisible(m_hasText);
	ui.frameDateTime->setVisible(m_hasDateTime);
	if (m_hasDateTime) {
		ui.dteValue1->setDisplayFormat(dateTimeFormat);
		ui.dteValue2->setDisplayFormat(dateTimeFormat);
	}

	// resize the dialog to have the minimum height
	layout()->activate();
	resize(QSize(this->width(), 0).expandedTo(minimumSize()));
}

void DropValuesDialog::operatorChanged(int index) const {
	bool value2 = (index == 2) || (index == 3);
	ui.lMin->setVisible(value2);
	ui.lMax->setVisible(value2);
	ui.lAnd->setVisible(value2);
	ui.leValue2->setVisible(value2);
}

void DropValuesDialog::operatorDateTimeChanged(int index) const {
	bool value2 = (index == 2) || (index == 3);
	ui.lMinDateTime->setVisible(value2);
	ui.lMaxDateTime->setVisible(value2);
	ui.lAndDateTime->setVisible(value2);
	ui.dteValue2->setVisible(value2);
}

void DropValuesDialog::okClicked() const {
	if (m_mask)
		maskValues();
	else
		dropValues();
}

// TODO: m_column->setMasked() is slow, we need direct access to the masked-container -> redesign
// TODO: provide template based solution to avoid code duplication
class MaskValuesTask : public QRunnable {
public:
	MaskValuesTask(Column* col, Operator op, double value1, double value2) {
		m_column = col;
		m_operator = op;
		m_value1 = value1;
		m_value2 = value2;
	}

	void run() override {
		m_column->setSuppressDataChangedSignal(true);
		bool changed = false;
		auto* data = static_cast<QVector<double>*>(m_column->data());
		auto* data_int = static_cast<QVector<int>*>(m_column->data());
		auto* data_bigint = static_cast<QVector<qint64>*>(m_column->data());
		auto* data_datetime = static_cast<QVector<QDateTime>*>(m_column->data());
		const int rows = m_column->rowCount();

		auto mode = m_column->columnMode();
		switch (m_operator) {
		case Operator::EqualTo:
			if (mode == AbstractColumn::ColumnMode::Double) {
				for (int i = 0; i < rows; ++i) {
					if (data->at(i) == m_value1) {
						m_column->setMasked(i, true);
						changed = true;
					}
				}
			} else if (mode == AbstractColumn::ColumnMode::Integer) {
				for (int i = 0; i < rows; ++i) {
					if (data_int->at(i) == m_value1) {
						m_column->setMasked(i, true);
						changed = true;
					}
				}
			} else if (mode == AbstractColumn::ColumnMode::BigInt) {
				for (int i = 0; i < rows; ++i) {
					if (data_bigint->at(i) == m_value1) {
						m_column->setMasked(i, true);
						changed = true;
					}
				}
			} else if (mode == AbstractColumn::ColumnMode::DateTime) {
				for (int i = 0; i < rows; ++i) {
					if (data_datetime->at(i).toMSecsSinceEpoch() == m_value1) {
						m_column->setMasked(i, true);
						changed = true;
					}
				}
			}
			break;
		case Operator::NotEqualTo:
			if (mode == AbstractColumn::ColumnMode::Double) {
				for (int i = 0; i < rows; ++i) {
					if (data->at(i) != m_value1) {
						m_column->setMasked(i, true);
						changed = true;
					}
				}
			} else if (mode == AbstractColumn::ColumnMode::Integer) {
				for (int i = 0; i < rows; ++i) {
					if (data_int->at(i) != m_value1) {
						m_column->setMasked(i, true);
						changed = true;
					}
				}
			} else if (mode == AbstractColumn::ColumnMode::BigInt) {
				for (int i = 0; i < rows; ++i) {
					if (data_bigint->at(i) != m_value1) {
						m_column->setMasked(i, true);
						changed = true;
					}
				}
			} else if (mode == AbstractColumn::ColumnMode::DateTime) {
				for (int i = 0; i < rows; ++i) {
					if (data_datetime->at(i).toMSecsSinceEpoch() != m_value1) {
						m_column->setMasked(i, true);
						changed = true;
					}
				}
			}
			break;
		case Operator::BetweenIncl:
			if (mode == AbstractColumn::ColumnMode::Double) {
				for (int i = 0; i < rows; ++i) {
					if (data->at(i) >= m_value1 && data->at(i) <= m_value2) {
						m_column->setMasked(i, true);
						changed = true;
					}
				}
			} else if (mode == AbstractColumn::ColumnMode::Integer) {
				for (int i = 0; i < rows; ++i) {
					if (data_int->at(i) >= m_value1 && data_int->at(i) <= m_value2) {
						m_column->setMasked(i, true);
						changed = true;
					}
				}
			} else if (mode == AbstractColumn::ColumnMode::BigInt) {
				for (int i = 0; i < rows; ++i) {
					if (data_bigint->at(i) >= m_value1 && data_bigint->at(i) <= m_value2) {
						m_column->setMasked(i, true);
						changed = true;
					}
				}
			} else if (mode == AbstractColumn::ColumnMode::DateTime) {
				for (int i = 0; i < rows; ++i) {
					if (data_datetime->at(i).toMSecsSinceEpoch() >= m_value1 && data_datetime->at(i).toMSecsSinceEpoch() <= m_value2) {
						m_column->setMasked(i, true);
						changed = true;
					}
				}
			}
			break;
		case Operator::BetweenExcl:
			if (mode == AbstractColumn::ColumnMode::Double) {
				for (int i = 0; i < rows; ++i) {
					if (data->at(i) > m_value1 && data->at(i) < m_value2) {
						m_column->setMasked(i, true);
						changed = true;
					}
				}
			} else if (mode == AbstractColumn::ColumnMode::Integer) {
				for (int i = 0; i < rows; ++i) {
					if (data_int->at(i) > m_value1 && data_int->at(i) < m_value2) {
						m_column->setMasked(i, true);
						changed = true;
					}
				}
			} else if (mode == AbstractColumn::ColumnMode::BigInt) {
				for (int i = 0; i < rows; ++i) {
					if (data_bigint->at(i) > m_value1 && data_bigint->at(i) < m_value2) {
						m_column->setMasked(i, true);
						changed = true;
					}
				}
			} else if (mode == AbstractColumn::ColumnMode::DateTime) {
				for (int i = 0; i < rows; ++i) {
					if (data_datetime->at(i).toMSecsSinceEpoch() > m_value1 && data_datetime->at(i).toMSecsSinceEpoch() < m_value2) {
						m_column->setMasked(i, true);
						changed = true;
					}
				}
			}
			break;
		case Operator::GreaterThan:
			if (mode == AbstractColumn::ColumnMode::Double) {
				for (int i = 0; i < rows; ++i) {
					if (data->at(i) > m_value1) {
						m_column->setMasked(i, true);
						changed = true;
					}
				}
			} else if (mode == AbstractColumn::ColumnMode::Integer) {
				for (int i = 0; i < rows; ++i) {
					if (data_int->at(i) > m_value1) {
						m_column->setMasked(i, true);
						changed = true;
					}
				}
			} else if (mode == AbstractColumn::ColumnMode::BigInt) {
				for (int i = 0; i < rows; ++i) {
					if (data_bigint->at(i) > m_value1) {
						m_column->setMasked(i, true);
						changed = true;
					}
				}
			} else if (mode == AbstractColumn::ColumnMode::DateTime) {
				for (int i = 0; i < rows; ++i) {
					if (data_datetime->at(i).toMSecsSinceEpoch() > m_value1) {
						m_column->setMasked(i, true);
						changed = true;
					}
				}
			}
			break;
		case Operator::GreaterThanEqualTo:
			if (mode == AbstractColumn::ColumnMode::Double) {
				for (int i = 0; i < rows; ++i) {
					if (data->at(i) >= m_value1) {
						m_column->setMasked(i, true);
						changed = true;
					}
				}
			} else if (mode == AbstractColumn::ColumnMode::Integer) {
				for (int i = 0; i < rows; ++i) {
					if (data_int->at(i) >= m_value1) {
						m_column->setMasked(i, true);
						changed = true;
					}
				}
			} else if (mode == AbstractColumn::ColumnMode::BigInt) {
				for (int i = 0; i < rows; ++i) {
					if (data_bigint->at(i) >= m_value1) {
						m_column->setMasked(i, true);
						changed = true;
					}
				}
			} else if (mode == AbstractColumn::ColumnMode::DateTime) {
				for (int i = 0; i < rows; ++i) {
					if (data_datetime->at(i).toMSecsSinceEpoch() >= m_value1) {
						m_column->setMasked(i, true);
						changed = true;
					}
				}
			}
			break;
		case Operator::LessThan:
			if (mode == AbstractColumn::ColumnMode::Double) {
				for (int i = 0; i < rows; ++i) {
					if (data->at(i) < m_value1) {
						m_column->setMasked(i, true);
						changed = true;
					}
				}
			} else if (mode == AbstractColumn::ColumnMode::Integer) {
				for (int i = 0; i < rows; ++i) {
					if (data_int->at(i) < m_value1) {
						m_column->setMasked(i, true);
						changed = true;
					}
				}
			} else if (mode == AbstractColumn::ColumnMode::BigInt) {
				for (int i = 0; i < rows; ++i) {
					if (data_bigint->at(i) < m_value1) {
						m_column->setMasked(i, true);
						changed = true;
					}
				}
			} else if (mode == AbstractColumn::ColumnMode::DateTime) {
				for (int i = 0; i < rows; ++i) {
					if (data_datetime->at(i).toMSecsSinceEpoch() < m_value1) {
						m_column->setMasked(i, true);
						changed = true;
					}
				}
			}
			break;
		case Operator::LessThanEqualTo:
			if (mode == AbstractColumn::ColumnMode::Double) {
				for (int i = 0; i < rows; ++i) {
					if (data->at(i) <= m_value1) {
						m_column->setMasked(i, true);
						changed = true;
					}
				}
			} else if (mode == AbstractColumn::ColumnMode::Integer) {
				for (int i = 0; i < rows; ++i) {
					if (data_int->at(i) <= m_value1) {
						m_column->setMasked(i, true);
						changed = true;
					}
				}
			} else if (mode == AbstractColumn::ColumnMode::BigInt) {
				for (int i = 0; i < rows; ++i) {
					if (data_bigint->at(i) <= m_value1) {
						m_column->setMasked(i, true);
						changed = true;
					}
				}
			} else if (mode == AbstractColumn::ColumnMode::DateTime) {
				for (int i = 0; i < rows; ++i) {
					if (data_datetime->at(i).toMSecsSinceEpoch() <= m_value1) {
						m_column->setMasked(i, true);
						changed = true;
					}
				}
			}
		}

		m_column->setSuppressDataChangedSignal(false);
		if (changed)
			m_column->setDataChanged();
	}

private:
	Operator m_operator;
	double m_value1;
	double m_value2;
	Column* m_column;
};

// TODO: provide template based solution to avoid code duplication
class DropValuesTask : public QRunnable {
public:
	DropValuesTask(Column* col, Operator op, double value1, double value2, QSet<int>& rows)
		: m_column(col)
		, m_operator(op)
		, m_value1(value1)
		, m_value2(value2)
		, m_rows(rows) {
	}

	void run() override {
		bool changed = false;

		int row = 0;
		auto mode = m_column->columnMode();
		if (mode == AbstractColumn::ColumnMode::Double) {
			const auto* data = static_cast<QVector<double>*>(m_column->data());
			QVector<double> new_data(*data);

			switch (m_operator) {
			case Operator::EqualTo:
				for (auto& d : new_data) {
					if (d == m_value1) {
						d = NAN;
						changed = true;
						m_rows << row;
					}
					++row;
				}
				break;
			case Operator::NotEqualTo:
				for (auto& d : new_data) {
					if (d != m_value1) {
						d = NAN;
						changed = true;
						m_rows << row;
					}
					++row;
				}
				break;
			case Operator::BetweenIncl:
				for (auto& d : new_data) {
					if (d >= m_value1 && d <= m_value2) {
						d = NAN;
						changed = true;
						m_rows << row;
					}
					++row;
				}
				break;
			case Operator::BetweenExcl:
				for (auto& d : new_data) {
					if (d > m_value1 && d < m_value2) {
						d = NAN;
						changed = true;
						m_rows << row;
					}
					++row;
				}
				break;
			case Operator::GreaterThan:
				for (auto& d : new_data) {
					if (d > m_value1) {
						d = NAN;
						changed = true;
						m_rows << row;
					}
					++row;
				}
				break;
			case Operator::GreaterThanEqualTo:
				for (auto& d : new_data) {
					if (d >= m_value1) {
						d = NAN;
						changed = true;
						m_rows << row;
					}
					++row;
				}
				break;
			case Operator::LessThan:
				for (auto& d : new_data) {
					if (d < m_value1) {
						d = NAN;
						changed = true;
						m_rows << row;
					}
					++row;
				}
				break;
			case Operator::LessThanEqualTo:
				for (auto& d : new_data) {
					if (d <= m_value1) {
						d = NAN;
						changed = true;
						m_rows << row;
					}
					++row;
				}
			}

			if (changed)
				m_column->setValues(new_data);
		} else if (mode == AbstractColumn::ColumnMode::Integer) {
			const auto* data = static_cast<QVector<int>*>(m_column->data());
			QVector<int> new_data(*data);

			switch (m_operator) {
			case Operator::EqualTo:
				for (auto& d : new_data) {
					if (d == m_value1) {
						d = 0;
						changed = true;
						m_rows << row;
					}
					++row;
				}
				break;
			case Operator::NotEqualTo:
				for (auto& d : new_data) {
					if (d != m_value1) {
						d = 0;
						changed = true;
						m_rows << row;
					}
					++row;
				}
				break;
			case Operator::BetweenIncl:
				for (auto& d : new_data) {
					if (d >= m_value1 && d <= m_value2) {
						d = 0;
						changed = true;
						m_rows << row;
					}
					++row;
				}
				break;
			case Operator::BetweenExcl:
				for (auto& d : new_data) {
					if (d > m_value1 && d < m_value2) {
						d = 0;
						changed = true;
						m_rows << row;
					}
					++row;
				}
				break;
			case Operator::GreaterThan:
				for (auto& d : new_data) {
					if (d > m_value1) {
						d = 0;
						changed = true;
						m_rows << row;
					}
					++row;
				}
				break;
			case Operator::GreaterThanEqualTo:
				for (auto& d : new_data) {
					if (d >= m_value1) {
						d = 0;
						changed = true;
						m_rows << row;
					}
					++row;
				}
				break;
			case Operator::LessThan:
				for (auto& d : new_data) {
					if (d < m_value1) {
						d = 0;
						changed = true;
						m_rows << row;
					}
					++row;
				}
				break;
			case Operator::LessThanEqualTo:
				for (auto& d : new_data) {
					if (d <= m_value1) {
						d = 0;
						changed = true;
						m_rows << row;
					}
					++row;
				}
			}

			if (changed)
				m_column->setIntegers(new_data);
		} else if (mode == AbstractColumn::ColumnMode::BigInt) {
			const auto* data = static_cast<QVector<qint64>*>(m_column->data());
			QVector<qint64> new_data(*data);

			switch (m_operator) {
			case Operator::EqualTo:
				for (auto& d : new_data) {
					if (d == m_value1) {
						d = 0;
						changed = true;
						m_rows << row;
					}
					++row;
				}
				break;
			case Operator::NotEqualTo:
				for (auto& d : new_data) {
					if (d != m_value1) {
						d = 0;
						changed = true;
						m_rows << row;
					}
					++row;
				}
				break;
			case Operator::BetweenIncl:
				for (auto& d : new_data) {
					if (d >= m_value1 && d <= m_value2) {
						d = 0;
						changed = true;
						m_rows << row;
					}
					++row;
				}
				break;
			case Operator::BetweenExcl:
				for (auto& d : new_data) {
					if (d > m_value1 && d < m_value2) {
						d = 0;
						changed = true;
						m_rows << row;
					}
					++row;
				}
				break;
			case Operator::GreaterThan:
				for (auto& d : new_data) {
					if (d > m_value1) {
						d = 0;
						changed = true;
						m_rows << row;
					}
					++row;
				}
				break;
			case Operator::GreaterThanEqualTo:
				for (auto& d : new_data) {
					if (d >= m_value1) {
						d = 0;
						changed = true;
						m_rows << row;
					}
					++row;
				}
				break;
			case Operator::LessThan:
				for (auto& d : new_data) {
					if (d < m_value1) {
						d = 0;
						changed = true;
						m_rows << row;
					}
					++row;
				}
				break;
			case Operator::LessThanEqualTo:
				for (auto& d : new_data) {
					if (d <= m_value1) {
						d = 0;
						changed = true;
						m_rows << row;
					}
					++row;
				}
			}

			if (changed)
				m_column->setBigInts(new_data);
		} else if (mode == AbstractColumn::ColumnMode::DateTime) {
			const auto* data = static_cast<QVector<QDateTime>*>(m_column->data());
			QVector<QDateTime> new_data(*data);

			switch (m_operator) {
			case Operator::EqualTo:
				for (auto& d : new_data) {
					if (d.toMSecsSinceEpoch() == m_value1) {
						d = QDateTime();
						changed = true;
						m_rows << row;
					}
					++row;
				}
				break;
			case Operator::NotEqualTo:
				for (auto& d : new_data) {
					if (d.toMSecsSinceEpoch() != m_value1) {
						d = QDateTime();
						changed = true;
						m_rows << row;
					}
					++row;
				}
				break;
			case Operator::BetweenIncl:
				for (auto& d : new_data) {
					if (d.toMSecsSinceEpoch() >= m_value1 && d.toMSecsSinceEpoch() <= m_value2) {
						d = QDateTime();
						changed = true;
						m_rows << row;
					}
					++row;
				}
				break;
			case Operator::BetweenExcl:
				for (auto& d : new_data) {
					if (d.toMSecsSinceEpoch() > m_value1 && d.toMSecsSinceEpoch() < m_value2) {
						d = QDateTime();
						changed = true;
						m_rows << row;
						++row;
					}
				}
				break;
			case Operator::GreaterThan:
				for (auto& d : new_data) {
					if (d.toMSecsSinceEpoch() > m_value1) {
						d = QDateTime();
						changed = true;
						m_rows << row;
					}
					++row;
				}
				break;
			case Operator::GreaterThanEqualTo:
				for (auto& d : new_data) {
					if (d.toMSecsSinceEpoch() >= m_value1) {
						d = QDateTime();
						changed = true;
						m_rows << row;
					}
					++row;
				}
				break;
			case Operator::LessThan:
				for (auto& d : new_data) {
					if (d.toMSecsSinceEpoch() < m_value1) {
						d = QDateTime();
						changed = true;
						m_rows << row;
					}
					++row;
				}
				break;
			case Operator::LessThanEqualTo:
				for (auto& d : new_data) {
					if (d.toMSecsSinceEpoch() <= m_value1) {
						d = QDateTime();
						changed = true;
						m_rows << row;
					}
					++row;
				}
			}

			if (changed)
				m_column->setDateTimes(new_data);
		}
	}

private:
	Column* m_column;
	Operator m_operator;
	double m_value1;
	double m_value2;
	QSet<int>& m_rows;
};

// implementation of tasks for text columns
class MaskTextValuesTask : public QRunnable {
public:
	MaskTextValuesTask(Column* col, OperatorText op, const QString& value)
		: m_operator(op)
		, m_value(value)
		, m_column(col) {
	}

	void run() override {
		m_column->setSuppressDataChangedSignal(true);
		bool changed = false;
		auto* data = static_cast<QVector<QString>*>(m_column->data());
		const int rows = m_column->rowCount();

		switch (m_operator) {
		case OperatorText::EqualTo:
			for (int i = 0; i < rows; ++i) {
				if (data->at(i) == m_value) {
					m_column->setMasked(i, true);
					changed = true;
				}
			}
			break;
		case OperatorText::NotEqualTo:
			for (int i = 0; i < rows; ++i) {
				if (data->at(i) != m_value) {
					m_column->setMasked(i, true);
					changed = true;
				}
			}
			break;
		case OperatorText::StartsWith:
			for (int i = 0; i < rows; ++i) {
				if (data->at(i).startsWith(m_value)) {
					m_column->setMasked(i, true);
					changed = true;
				}
			}
			break;
		case OperatorText::EndsWith:
			for (int i = 0; i < rows; ++i) {
				if (data->at(i).endsWith(m_value)) {
					m_column->setMasked(i, true);
					changed = true;
				}
			}
			break;
		case OperatorText::Contain:
			for (int i = 0; i < rows; ++i) {
				if (data->at(i).indexOf(m_value) != -1) {
					m_column->setMasked(i, true);
					changed = true;
				}
			}

			break;
		case OperatorText::NotContain:
			for (int i = 0; i < rows; ++i) {
				if (data->at(i).indexOf(m_value) == -1) {
					m_column->setMasked(i, true);
					changed = true;
				}
			}
			break;
		}

		m_column->setSuppressDataChangedSignal(false);
		if (changed)
			m_column->setDataChanged();
	}

private:
	OperatorText m_operator;
	QString m_value;
	Column* m_column;
};

class DropTextValuesTask : public QRunnable {
public:
	DropTextValuesTask(Column* col, OperatorText op, const QString& value, QSet<int>& rows)
		: m_operator(op)
		, m_value(value)
		, m_column(col)
		, m_rows(rows) {
	}

	void run() override {
		bool changed = false;
		const auto* data = static_cast<QVector<QString>*>(m_column->data());
		QVector<QString> new_data(*data);
		int row = 0;

		switch (m_operator) {
		case OperatorText::EqualTo:
			for (auto& d : new_data) {
				if (d == m_value) {
					d = QString();
					changed = true;
					m_rows << row;
					++row;
				}
			}
			break;
		case OperatorText::NotEqualTo:
			for (auto& d : new_data) {
				if (d != m_value) {
					d = QString();
					changed = true;
					m_rows << row;
					++row;
				}
			}
			break;
		case OperatorText::StartsWith:
			for (auto& d : new_data) {
				if (d.startsWith(m_value)) {
					d = QString();
					changed = true;
					m_rows << row;
					++row;
				}
			}
			break;
		case OperatorText::EndsWith:
			for (auto& d : new_data) {
				if (d.endsWith(m_value)) {
					d = QString();
					changed = true;
					m_rows << row;
					++row;
				}
			}
			break;
		case OperatorText::Contain:
			for (auto& d : new_data) {
				if (d.indexOf(m_value) != -1) {
					d = QString();
					changed = true;
					m_rows << row;
					++row;
				}
			}
			break;
		case OperatorText::NotContain:
			for (auto& d : new_data) {
				if (d.indexOf(m_value) == -1) {
					d = QString();
					changed = true;
					m_rows << row;
					++row;
				}
			}
			break;
		}

		if (changed)
			m_column->replaceTexts(0, new_data);
	}

private:
	OperatorText m_operator;
	QString m_value;
	Column* m_column;
	QSet<int>& m_rows;
};

void DropValuesDialog::maskValues() const {
	Q_ASSERT(m_spreadsheet);

	// settings for numeric columns
	const auto op = static_cast<Operator>(ui.cbOperator->currentIndex());
	const auto numberLocale = QLocale();
	bool ok;
	const double value1 = numberLocale.toDouble(ui.leValue1->text(), &ok);
	if (!ok && m_hasNumeric) {
		KMessageBox::error(nullptr, i18n("Invalid numeric value."));
		ui.leValue1->setFocus();
		return;
	}

	const double value2 = numberLocale.toDouble(ui.leValue2->text(), &ok);
	if (ui.leValue2->isVisible() && !ok) {
		KMessageBox::error(nullptr, i18n("Invalid numeric value."));
		ui.leValue2->setFocus();
		return;
	}

	// settings for text columns
	const auto opText = static_cast<OperatorText>(ui.cbOperatorText->currentIndex());
	const auto& valueText = ui.leValueText->text();

	// settings for DateTime columns;
	const auto opDateTime = static_cast<Operator>(ui.cbOperatorDateTime->currentIndex());
	double value1DateTime = ui.dteValue1->dateTime().toMSecsSinceEpoch();
	double value2DateTime = ui.dteValue2->dateTime().toMSecsSinceEpoch();

	WAIT_CURSOR_AUTO_RESET;
	m_spreadsheet->beginMacro(i18n("%1: mask values", m_spreadsheet->name()));
	for (auto* col : m_columns) {
		if (col->isNumeric()) {
			auto* task = new MaskValuesTask(col, op, value1, value2);
			task->run();
			// TODO: writing to the undo-stack in Column::setMasked() is not tread-safe -> redesign
			// 		QThreadPool::globalInstance()->start(task);
			delete task;
		} else if (col->columnMode() == AbstractColumn::ColumnMode::DateTime) {
			auto* task = new MaskValuesTask(col, opDateTime, value1DateTime, value2DateTime);
			task->run();
			delete task;
		} else {
			auto* task = new MaskTextValuesTask(col, opText, valueText);
			task->run();
			delete task;
		}
	}

	// wait until all columns were processed
	// 	QThreadPool::globalInstance()->waitForDone();

	m_spreadsheet->endMacro();
}

void DropValuesDialog::dropValues() const {
	Q_ASSERT(m_spreadsheet);

	// settings for numeric columns
	const auto op = static_cast<Operator>(ui.cbOperator->currentIndex());
	const auto numberLocale = QLocale();
	bool ok;
	const double value1 = numberLocale.toDouble(ui.leValue1->text(), &ok);
	if (!ok && m_hasNumeric) {
		KMessageBox::error(nullptr, i18n("Invalid numeric value."));
		ui.leValue1->setFocus();
		return;
	}
	const double value2 = numberLocale.toDouble(ui.leValue2->text(), &ok);
	if (ui.leValue2->isVisible() && !ok && m_hasNumeric) {
		KMessageBox::error(nullptr, i18n("Invalid numeric value."));
		ui.leValue2->setFocus();
		return;
	}

	// settings for text columns
	const auto opText = static_cast<OperatorText>(ui.cbOperatorText->currentIndex());
	const auto& valueText = ui.leValueText->text();

	// settings for DateTime columns;
	const auto opDateTime = static_cast<Operator>(ui.cbOperatorDateTime->currentIndex());
	double value1DateTime = ui.dteValue1->dateTime().toMSecsSinceEpoch();
	double value2DateTime = ui.dteValue2->dateTime().toMSecsSinceEpoch();

	WAIT_CURSOR_AUTO_RESET;
	m_spreadsheet->beginMacro(i18n("%1: drop values", m_spreadsheet->name()));

	QSet<int> rows; // rows in which the values were dropped/deleted

	for (auto* col : m_columns) {
		if (col->isNumeric()) {
			auto* task = new DropValuesTask(col, op, value1, value2, rows);
			QThreadPool::globalInstance()->start(task);
		} else if (col->columnMode() == AbstractColumn::ColumnMode::DateTime) {
			auto* task = new DropValuesTask(col, opDateTime, value1DateTime, value2DateTime, rows);
			QThreadPool::globalInstance()->start(task);
		} else {
			auto* task = new DropTextValuesTask(col, opText, valueText, rows);
			QThreadPool::globalInstance()->start(task);
		}
	}

	// wait until all columns were processed
	QThreadPool::globalInstance()->waitForDone();

	// delete the affected rows completely in the spreadsheet, if requested
	if (ui.chbDeleteRows->isChecked()) {
		auto rowList = rows.values();
		std::sort(rowList.begin(), rowList.end(), std::greater<int>());
		for (const int row : rowList)
			m_spreadsheet->removeRows(row, 1);
	}

	m_spreadsheet->endMacro();
}
