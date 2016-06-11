/***************************************************************************
    File                 : DropValuesDialog.cpp
    Project              : LabPlot
    Description          : Dialog for droping and masking values in columns
    --------------------------------------------------------------------
    Copyright            : (C) 2015 by Alexander Semke (alexander.semke@web.de)

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

#include <cmath>

/*!
	\class DropValuesDialog
	\brief Dialog for generating values from a mathematical function.

	\ingroup kdefrontend
 */

DropValuesDialog::DropValuesDialog(Spreadsheet* s, bool mask, QWidget* parent, Qt::WFlags fl) : KDialog(parent, fl),
	m_spreadsheet(s), m_mask(mask) {

	setWindowTitle(i18n("Drop values"));

	QFrame* mainWidget = new QFrame(this);
	ui.setupUi(mainWidget);
	setMainWidget( mainWidget );

	ui.cbOperator->addItem(i18n("equal to"));
	ui.cbOperator->addItem(i18n("between (including end points)"));
	ui.cbOperator->addItem(i18n("between (excluding end points)"));
	ui.cbOperator->addItem(i18n("greater then"));
	ui.cbOperator->addItem(i18n("greater then or equal to"));
	ui.cbOperator->addItem(i18n("lesser then"));
	ui.cbOperator->addItem(i18n("lesser then or equal to"));

	ui.leValue1->setValidator( new QDoubleValidator(ui.leValue1) );
	ui.leValue2->setValidator( new QDoubleValidator(ui.leValue2) );

	setButtons( KDialog::Ok | KDialog::Cancel );
	if (m_mask) {
		setButtonText(KDialog::Ok, i18n("&Mask"));
		setButtonToolTip(KDialog::Ok, i18n("Mask values in the specified region"));
		ui.lMode->setText(i18n("Mask values"));
		setWindowTitle(i18n("Mask values"));
	} else {
		setButtonText(KDialog::Ok, i18n("&Drop"));
		setButtonToolTip(KDialog::Ok, i18n("Drop values in the specified region"));
	}

	connect( ui.cbOperator, SIGNAL(currentIndexChanged(int)), this, SLOT(operatorChanged(int)) );
	connect(this, SIGNAL(okClicked()), this, SLOT(okClicked()));

	resize( QSize(400,0).expandedTo(minimumSize()) );
	operatorChanged(0);
}

void DropValuesDialog::setColumns(QList<Column*> list) {
	m_columns = list;
}

void DropValuesDialog::operatorChanged(int index) const {
	bool value2 = (index==1) || (index==2);
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
		MaskValuesTask(Column* col, int op, double value1, double value2){
			m_column = col;
			m_operator = op;
			m_value1 = value1;
			m_value2 = value2;
		};

		void run() {
			m_column->setSuppressDataChangedSignal(true);
			bool changed = false;
			QVector<double>* data = static_cast<QVector<double>* >(m_column->data());

			//equal to
			if (m_operator == 0) {
				for (int i=0; i<data->size(); ++i) {
					if (data->at(i) == m_value1) {
						m_column->setMasked(i, true);
						changed = true;
					}
				}
			}

			//between (including end points)
			else if (m_operator == 1) {
				for (int i=0; i<data->size(); ++i) {
					if (data->at(i) >= m_value1 && data->at(i) <= m_value2) {
						m_column->setMasked(i, true);
						changed = true;
					}
				}
			}

			//between (excluding end points)
			else if (m_operator == 2) {
				for (int i=0; i<data->size(); ++i) {
					if (data->at(i) > m_value1 && data->at(i) < m_value2) {
						m_column->setMasked(i, true);
						changed = true;
					}
				}
			}

			//greater then
			else if (m_operator == 3) {
				for (int i=0; i<data->size(); ++i) {
					if (data->at(i) > m_value1) {
						m_column->setMasked(i, true);
						changed = true;
					}
				}
			}

			//greater then or equal to
			else if (m_operator == 4) {
				for (int i=0; i<data->size(); ++i) {
					if (data->at(i) >= m_value1) {
						m_column->setMasked(i, true);
						changed = true;
					}
				}
			}

			//lesser then
			else if (m_operator == 5) {
				for (int i=0; i<data->size(); ++i) {
					if (data->at(i) < m_value1) {
						m_column->setMasked(i, true);
						changed = true;
					}
				}
			}

			//lesser then or equal to
			else if (m_operator == 6) {
				for (int i=0; i<data->size(); ++i) {
					if (data->at(i) <= m_value1) {
						m_column->setMasked(i, true);
						changed = true;
					}
				}
			}

			m_column->setSuppressDataChangedSignal(false);
			if (changed)
				m_column->setChanged();
		}

	private:
		Column* m_column;
		int m_operator;
		double m_value1;
		double m_value2;
};

class DropValuesTask : public QRunnable {
	public:
		DropValuesTask(Column* col, int op, double value1, double value2){
			m_column = col;
			m_operator = op;
			m_value1 = value1;
			m_value2 = value2;
		};

		void run() {
			bool changed = false;
			QVector<double>* data = static_cast<QVector<double>* >(m_column->data());
			QVector<double> new_data(*data);

			//equal to
			if (m_operator == 0) {
				for (int i=0; i<new_data.size(); ++i) {
					if (new_data[i] == m_value1) {
						new_data[i] = NAN;
						changed = true;
					}
				}
			}

			//between (including end points)
			else if (m_operator == 1) {
				for (int i=0; i<new_data.size(); ++i) {
					if (new_data[i] >= m_value1 && new_data[i] <= m_value2) {
						new_data[i] = NAN;
						changed = true;
					}
				}
			}

			//between (excluding end points)
			else if (m_operator == 2) {
				for (int i=0; i<new_data.size(); ++i) {
					if (new_data[i] > m_value1 && new_data[i] < m_value2) {
						new_data[i] = NAN;
						changed = true;
					}
				}
			}

			//greater then
			else if (m_operator == 3) {
				for (int i=0; i<new_data.size(); ++i) {
					if (new_data[i] > m_value1) {
						new_data[i] = NAN;
						changed = true;
					}
				}
			}

			//greater then or equal to
			else if (m_operator == 4) {
				for (int i=0; i<new_data.size(); ++i) {
					if (new_data[i] >= m_value1) {
						new_data[i] = NAN;
						changed = true;
					}
				}
			}

			//lesser then
			else if (m_operator == 5) {
				for (int i=0; i<new_data.size(); ++i) {
					if (new_data[i] < m_value1) {
						new_data[i] = NAN;
						changed = true;
					}
				}
			}

			//lesser then or equal to
			else if (m_operator == 6) {
				for (int i=0; i<new_data.size(); ++i) {
					if (new_data[i] <= m_value1) {
						new_data[i] = NAN;
						changed = true;
					}
				}
			}

			if (changed)
				m_column->replaceValues(0, new_data);
		}

	private:
		Column* m_column;
		int m_operator;
		double m_value1;
		double m_value2;
};

void DropValuesDialog::maskValues() const {
	Q_ASSERT(m_spreadsheet);

	WAIT_CURSOR;
	m_spreadsheet->beginMacro(i18n("%1: mask values", m_spreadsheet->name()));

	const int op = ui.cbOperator->currentIndex();
	const double value1 = ui.leValue1->text().toDouble();
	const double value2 = ui.leValue2->text().toDouble();

	foreach(Column* col, m_columns) {
		MaskValuesTask* task = new MaskValuesTask(col, op, value1, value2);
		task->run();
		//TODO: writing to the undo-stack in Column::setMasked() is not tread-safe -> redesign
// 		QThreadPool::globalInstance()->start(task);
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

	const int op = ui.cbOperator->currentIndex();
	const double value1 = ui.leValue1->text().toDouble();
	const double value2 = ui.leValue2->text().toDouble();

	foreach(Column* col, m_columns) {
		DropValuesTask* task = new DropValuesTask(col, op, value1, value2);
		QThreadPool::globalInstance()->start(task);
	}

	//wait until all columns were processed
	QThreadPool::globalInstance()->waitForDone();

	m_spreadsheet->endMacro();
	RESET_CURSOR;
}
