/*
    File                 : DropValuesDialog.cpp
    Project              : LabPlot
    Description          : Dialog for droping and masking values in columns
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2015-2020 Alexander Semke <alexander.semke@web.de>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "DropValuesDialog.h"
#include "backend/core/column/Column.h"
#include "backend/lib/macros.h"
#include "backend/spreadsheet/Spreadsheet.h"

#include <QThreadPool>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QWindow>

#include <KLocalizedString>
#include <KWindowConfig>

#include <cmath>

enum class Operator {EqualTo, BetweenIncl, BetweenExcl, GreaterThan, GreaterThanEqualTo, LessThan, LessThanEqualTo};

/*!
	\class DropValuesDialog
	\brief Dialog for generating values from a mathematical function.

	\ingroup kdefrontend
 */
DropValuesDialog::DropValuesDialog(Spreadsheet* s, bool mask, QWidget* parent) : QDialog(parent),
	m_spreadsheet(s), m_mask(mask) {

	setWindowTitle(i18nc("@title:window", "Drop Values"));

	ui.setupUi(this);
	setAttribute(Qt::WA_DeleteOnClose);

	ui.cbOperator->addItem(i18n("Equal to"));
	ui.cbOperator->addItem(i18n("Between (Incl. End Points)"));
	ui.cbOperator->addItem(i18n("Between (Excl. End Points)"));
	ui.cbOperator->addItem(i18n("Greater than"));
	ui.cbOperator->addItem(i18n("Greater than or Equal to"));
	ui.cbOperator->addItem(i18n("Less than"));
	ui.cbOperator->addItem(i18n("Less than or Equal to"));

	ui.leValue1->setValidator( new QDoubleValidator(ui.leValue1) );
	ui.leValue2->setValidator( new QDoubleValidator(ui.leValue2) );

	auto* btnBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);

	ui.horizontalLayout->addWidget(btnBox);
	m_okButton = btnBox->button(QDialogButtonBox::Ok);

	connect(btnBox->button(QDialogButtonBox::Cancel), &QPushButton::clicked, this, &DropValuesDialog::close);

	if (m_mask) {
		m_okButton->setText(i18n("&Mask"));
		m_okButton->setToolTip(i18n("Mask values in the specified region"));
		ui.lMode->setText(i18n("Mask values"));
		setWindowTitle(i18nc("@title:window", "Mask Values"));
	} else {
		m_okButton->setText(i18n("&Drop"));
		m_okButton->setToolTip(i18n("Drop values in the specified region"));
	}

	connect(ui.cbOperator, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &DropValuesDialog::operatorChanged );
	connect(m_okButton, &QPushButton::clicked, this, &DropValuesDialog::okClicked);
	connect(btnBox, &QDialogButtonBox::accepted, this, &DropValuesDialog::accept);
	connect(btnBox, &QDialogButtonBox::rejected, this, &DropValuesDialog::reject);

	//restore saved settings if available
	KConfigGroup conf(KSharedConfig::openConfig(), QLatin1String("DropValuesDialog"));
	ui.cbOperator->setCurrentIndex(conf.readEntry("Operator", 0));
	operatorChanged(ui.cbOperator->currentIndex());

	create(); // ensure there's a window created
	if (conf.exists()) {
		KWindowConfig::restoreWindowSize(windowHandle(), conf);
		resize(windowHandle()->size()); // workaround for QTBUG-40584
	} else
		resize(QSize(400, 0).expandedTo(minimumSize()));
}

DropValuesDialog::~DropValuesDialog() {
	//save the current settings
	KConfigGroup conf(KSharedConfig::openConfig(), QLatin1String("DropValuesDialog"));
	conf.writeEntry("Operator", ui.cbOperator->currentIndex());
	KWindowConfig::saveWindowSize(windowHandle(), conf);
}

void DropValuesDialog::setColumns(const QVector<Column*>& columns) {
	m_columns = columns;
}

void DropValuesDialog::operatorChanged(int index) const {
	bool value2 = (index == 1) || (index == 2);
	ui.lMin->setVisible(value2);
	ui.lMax->setVisible(value2);
	ui.lAnd->setVisible(value2);
	ui.leValue2->setVisible(value2);
}

void DropValuesDialog::okClicked() const {
	if (m_mask)
		maskValues();
	else
		dropValues();
}

//TODO: m_column->setMasked() is slow, we need direct access to the masked-container -> redesign
//TODO: provide template based solution to avoid code duplication
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
			auto* data = static_cast<QVector<double>* >(m_column->data());
			auto* data_int = static_cast<QVector<int>* >(m_column->data());
			auto* data_bigint = static_cast<QVector<qint64>* >(m_column->data());
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
				} else 	if (mode == AbstractColumn::ColumnMode::Integer) {
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
				}
			}

			m_column->setSuppressDataChangedSignal(false);
			if (changed)
				m_column->setChanged();
		}

	private:
		Operator m_operator;
		double m_value1;
		double m_value2;
		Column* m_column;
};

//TODO: provide template based solution to avoid code duplication
class DropValuesTask : public QRunnable {
	public:
		DropValuesTask(Column* col, Operator op, double value1, double value2) {
			m_column = col;
			m_operator = op;
			m_value1 = value1;
			m_value2 = value2;
		}

		void run() override {
			bool changed = false;

			auto mode = m_column->columnMode();
			if (mode == AbstractColumn::ColumnMode::Double) {
				auto* data = static_cast<QVector<double>* >(m_column->data());
				QVector<double> new_data(*data);

				switch (m_operator) {
				case Operator::EqualTo:
					for (auto& d : new_data) {
						if (d == m_value1) {
							d = NAN;
							changed = true;
						}
					}
					break;
				case Operator::BetweenIncl:
					for (auto& d : new_data) {
						if (d >= m_value1 && d <= m_value2) {
							d = NAN;
							changed = true;
						}
					}
					break;
				case Operator::BetweenExcl:
					for (auto& d : new_data) {
						if (d > m_value1 && d < m_value2) {
							d = NAN;
							changed = true;
						}
					}
					break;
				case Operator::GreaterThan:
					for (auto& d : new_data) {
						if (d > m_value1) {
							d = NAN;
							changed = true;
						}
					}
					break;
				case Operator::GreaterThanEqualTo:
					for (auto& d : new_data) {
						if (d >= m_value1) {
							d = NAN;
							changed = true;
						}
					}
					break;
				case Operator::LessThan:
					for (auto& d : new_data) {
						if (d < m_value1) {
							d = NAN;
							changed = true;
						}
					}
					break;
				case Operator::LessThanEqualTo:
					for (auto& d : new_data) {
						if (d <= m_value1) {
							d = NAN;
							changed = true;
						}
					}
				}

				if (changed)
					m_column->replaceValues(0, new_data);
			} else if (mode == AbstractColumn::ColumnMode::Integer) {
				auto* data = static_cast<QVector<int>* >(m_column->data());
				QVector<int> new_data(*data);

				switch (m_operator) {
				case Operator::EqualTo:
					for (auto& d : new_data) {
						if (d == m_value1) {
							d = 0;
							changed = true;
						}
					}
					break;
				case Operator::BetweenIncl:
					for (auto& d : new_data) {
						if (d >= m_value1 && d <= m_value2) {
							d = 0;
							changed = true;
						}
					}
					break;
				case Operator::BetweenExcl:
					for (auto& d : new_data) {
						if (d > m_value1 && d < m_value2) {
							d = 0;
							changed = true;
						}
					}
					break;
				case Operator::GreaterThan:
					for (auto& d : new_data) {
						if (d > m_value1) {
							d = 0;
							changed = true;
						}
					}
					break;
				case Operator::GreaterThanEqualTo:
					for (auto& d : new_data) {
						if (d >= m_value1) {
							d = 0;
							changed = true;
						}
					}
					break;
				case Operator::LessThan:
					for (auto& d : new_data) {
						if (d < m_value1) {
							d = 0;
							changed = true;
						}
					}
					break;
				case Operator::LessThanEqualTo:
					for (auto& d : new_data) {
						if (d <= m_value1) {
							d = 0;
							changed = true;
						}
					}
				}

				if (changed)
					m_column->replaceInteger(0, new_data);
			} else if (mode == AbstractColumn::ColumnMode::BigInt) {
				auto* data = static_cast<QVector<qint64>* >(m_column->data());
				QVector<qint64> new_data(*data);

				switch (m_operator) {
				case Operator::EqualTo:
					for (auto& d : new_data) {
						if (d == m_value1) {
							d = 0;
							changed = true;
						}
					}
					break;
				case Operator::BetweenIncl:
					for (auto& d : new_data) {
						if (d >= m_value1 && d <= m_value2) {
							d = 0;
							changed = true;
						}
					}
					break;
				case Operator::BetweenExcl:
					for (auto& d : new_data) {
						if (d > m_value1 && d < m_value2) {
							d = 0;
							changed = true;
						}
					}
					break;
				case Operator::GreaterThan:
					for (auto& d : new_data) {
						if (d > m_value1) {
							d = 0;
							changed = true;
						}
					}
					break;
				case Operator::GreaterThanEqualTo:
					for (auto& d : new_data) {
						if (d >= m_value1) {
							d = 0;
							changed = true;
						}
					}
					break;
				case Operator::LessThan:
					for (auto& d : new_data) {
						if (d < m_value1) {
							d = 0;
							changed = true;
						}
					}
					break;
				case Operator::LessThanEqualTo:
					for (auto& d : new_data) {
						if (d <= m_value1) {
							d = 0;
							changed = true;
						}
					}
				}

				if (changed)
					m_column->replaceBigInt(0, new_data);
			}
		}

	private:
		Operator m_operator;
		double m_value1;
		double m_value2;
		Column* m_column;
};

void DropValuesDialog::maskValues() const {
	Q_ASSERT(m_spreadsheet);

	WAIT_CURSOR;
	m_spreadsheet->beginMacro(i18n("%1: mask values", m_spreadsheet->name()));

	const auto op = static_cast<Operator>(ui.cbOperator->currentIndex());
	SET_NUMBER_LOCALE
	bool ok;
	const double value1 = numberLocale.toDouble(ui.leValue1->text(), &ok);
	if (!ok) {
		DEBUG("Double value 1 invalid!")
		m_spreadsheet->endMacro();
		return;
	}
	const double value2 = numberLocale.toDouble(ui.leValue2->text(), &ok);
	if (!ok) {
		DEBUG("Double value 2 invalid!")
		m_spreadsheet->endMacro();
		return;
	}

	for (Column* col: m_columns) {
		auto* task = new MaskValuesTask(col, op, value1, value2);
		task->run();
		//TODO: writing to the undo-stack in Column::setMasked() is not tread-safe -> redesign
// 		QThreadPool::globalInstance()->start(task);
		delete task;
	}

	//wait until all columns were processed
// 	QThreadPool::globalInstance()->waitForDone();

	m_spreadsheet->endMacro();
	RESET_CURSOR;
}

void DropValuesDialog::dropValues() const {
	Q_ASSERT(m_spreadsheet);

	WAIT_CURSOR;
	m_spreadsheet->beginMacro(i18n("%1: drop values", m_spreadsheet->name()));

	const auto op = static_cast<Operator>(ui.cbOperator->currentIndex());
	SET_NUMBER_LOCALE
	bool ok;
	const double value1 = numberLocale.toDouble(ui.leValue1->text(), &ok);
	if (!ok) {
		DEBUG("Double value 1 invalid!")
		m_spreadsheet->endMacro();
		return;
	}
	const double value2 = numberLocale.toDouble(ui.leValue2->text(), &ok);
	if (!ok) {
		DEBUG("Double value 2 invalid!")
		m_spreadsheet->endMacro();
		return;
	}

	for (Column* col: m_columns) {
		auto* task = new DropValuesTask(col, op, value1, value2);
		QThreadPool::globalInstance()->start(task);
	}

	//wait until all columns were processed
	QThreadPool::globalInstance()->waitForDone();

	m_spreadsheet->endMacro();
	RESET_CURSOR;
}
