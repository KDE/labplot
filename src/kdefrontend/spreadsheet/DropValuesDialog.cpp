/***************************************************************************
    File                 : DropValuesDialog.cpp
    Project              : LabPlot
    Description          : Dialog for droping and masking values in columns
    --------------------------------------------------------------------
	Copyright            : (C) 2015-2020 by Alexander Semke (alexander.semke@web.de)

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *  This program is free software; you can redistribute it and/or modify   *
 *  it under the terms of the GNU General Public License as published by   *
 *  the Free Software Foundation; either version 2 of the License, or      *
 *  (at your option) any later version.                                    *
 *                                                                         *
 *  This program is distributed in the hope that it will be useful,        *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 *  GNU General Public License for more details.                           *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the Free Software           *
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor,                    *
 *   Boston, MA  02110-1301  USA                                           *
 *                                                                         *
 ***************************************************************************/
#include "DropValuesDialog.h"
#include "backend/core/column/Column.h"
#include "backend/lib/macros.h"
#include "backend/spreadsheet/Spreadsheet.h"

#include <QThreadPool>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QWindow>

#include <KLocalizedString>
#include <KConfigGroup>
#include <KSharedConfig>
#include <KWindowConfig>

#include <cmath>

enum Operator {EqualTo, BetweenIncl, BetweenExcl, GreaterThan, GreaterThanEqualTo, LessThan, LessThanEqualTo};

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

	QDialogButtonBox* btnBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);

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

	connect(ui.cbOperator, static_cast<void (QComboBox::*) (int)>(&QComboBox::currentIndexChanged), this, &DropValuesDialog::operatorChanged );
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

			switch (m_operator) {
			case EqualTo: {
				for (int i = 0; i < data->size(); ++i) {
					if (data->at(i) == m_value1) {
						m_column->setMasked(i, true);
						changed = true;
					}
				}
				break;
			}
			case BetweenIncl: {
				for (int i = 0; i < data->size(); ++i) {
					if (data->at(i) >= m_value1 && data->at(i) <= m_value2) {
						m_column->setMasked(i, true);
						changed = true;
					}
				}
				break;
			}

			case BetweenExcl: {
				for (int i = 0; i < data->size(); ++i) {
					if (data->at(i) > m_value1 && data->at(i) < m_value2) {
						m_column->setMasked(i, true);
						changed = true;
					}
				}
				break;
			}
			case GreaterThan: {
				for (int i = 0; i < data->size(); ++i) {
					if (data->at(i) > m_value1) {
						m_column->setMasked(i, true);
						changed = true;
					}
				}
				break;
			}

			case GreaterThanEqualTo: {
				for (int i = 0; i < data->size(); ++i) {
					if (data->at(i) >= m_value1) {
						m_column->setMasked(i, true);
						changed = true;
					}
				}
				break;
			}
			case LessThan: {
				for (int i = 0; i < data->size(); ++i) {
					if (data->at(i) < m_value1) {
						m_column->setMasked(i, true);
						changed = true;
					}
				}
				break;
			}
			case LessThanEqualTo: {
				for (int i = 0; i < data->size(); ++i) {
					if (data->at(i) <= m_value1) {
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
			auto* data = static_cast<QVector<double>* >(m_column->data());
			QVector<double> new_data(*data);

			switch (m_operator) {
			case EqualTo: {
				for (auto& d : new_data) {
					if (d == m_value1) {
						d = NAN;
						changed = true;
					}
				}
				break;
			}
			case BetweenIncl: {
				for (auto& d : new_data) {
					if (d >= m_value1 && d <= m_value2) {
						d = NAN;
						changed = true;
					}
				}
				break;
			}
			case BetweenExcl: {
				for (auto& d : new_data) {
					if (d > m_value1 && d < m_value2) {
						d = NAN;
						changed = true;
					}
				}
				break;
			}
			case GreaterThan: {
				for (auto& d : new_data) {
					if (d > m_value1) {
						d = NAN;
						changed = true;
					}
				}
				break;
			}
			case GreaterThanEqualTo: {
				for (auto& d : new_data) {
					if (d >= m_value1) {
						d = NAN;
						changed = true;
					}
				}
				break;
			}
			case LessThan: {
				for (auto& d : new_data) {
					if (d < m_value1) {
						d = NAN;
						changed = true;
					}
				}
				break;
			}
			case LessThanEqualTo: {
				for (auto& d : new_data) {
					if (d <= m_value1) {
						d = NAN;
						changed = true;
					}
				}
			}
			}

			if (changed)
				m_column->replaceValues(0, new_data);
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

	const Operator op = static_cast<Operator>(ui.cbOperator->currentIndex());
	const double value1 = ui.leValue1->text().toDouble();
	const double value2 = ui.leValue2->text().toDouble();

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

	const Operator op = static_cast<Operator>(ui.cbOperator->currentIndex());
	const double value1 = ui.leValue1->text().toDouble();
	const double value2 = ui.leValue2->text().toDouble();

	for (Column* col: m_columns) {
		auto* task = new DropValuesTask(col, op, value1, value2);
		QThreadPool::globalInstance()->start(task);
	}

	//wait until all columns were processed
	QThreadPool::globalInstance()->waitForDone();

	m_spreadsheet->endMacro();
	RESET_CURSOR;
}
