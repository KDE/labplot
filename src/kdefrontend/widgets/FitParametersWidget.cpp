/***************************************************************************
    File                 : FitParametersWidget.cc
    Project              : LabPlot
    Description          : widget for editing fit parameters
    --------------------------------------------------------------------
    Copyright            : (C) 2014 Alexander Semke (alexander.semke@web.de)
    Copyright            : (C) 2016 Stefan Gerlach (stefan.gerlach@uni.kn)

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
#include "FitParametersWidget.h"
#include <QLineEdit>
#include <QKeyEvent>
#include <cfloat>

/*!
	\class FitParametersWidget
	\brief Widget for editing fit parameters. For predefined models the number of parameters,
	their names and default values are given - the user can change the start values.
	For custom models the user has to define here the parameter names and their start values.

	\ingroup kdefrontend
 */
FitParametersWidget::FitParametersWidget(QWidget* parent, XYFitCurve::FitData* data) : QWidget(parent), m_fitData(data), m_changed(false) {
	ui.setupUi(this);
	ui.pbApply->setIcon(KIcon("dialog-ok-apply"));
	ui.pbCancel->setIcon(KIcon("dialog-cancel"));

	ui.tableWidget->setColumnCount(5);

	QTableWidgetItem* headerItem = new QTableWidgetItem();
	headerItem->setText(i18n("Name"));
	ui.tableWidget->setHorizontalHeaderItem(0, headerItem);

	headerItem = new QTableWidgetItem();
	headerItem->setText(i18n("Start value"));
	ui.tableWidget->setHorizontalHeaderItem(1, headerItem);

	headerItem = new QTableWidgetItem();
	headerItem->setText(i18n("Fixed"));
	ui.tableWidget->setHorizontalHeaderItem(2, headerItem);

	headerItem = new QTableWidgetItem();
	headerItem->setText(i18n("Lower limit"));
	ui.tableWidget->setHorizontalHeaderItem(3, headerItem);

	headerItem = new QTableWidgetItem();
	headerItem->setText(i18n("Upper limit"));
	ui.tableWidget->setHorizontalHeaderItem(4, headerItem);

	ui.tableWidget->horizontalHeader()->setResizeMode(0, QHeaderView::ResizeToContents);
	ui.tableWidget->horizontalHeader()->setResizeMode(1, QHeaderView::Stretch);
	ui.tableWidget->horizontalHeader()->setResizeMode(2, QHeaderView::ResizeToContents);
	ui.tableWidget->horizontalHeader()->setResizeMode(3, QHeaderView::ResizeToContents);
	ui.tableWidget->horizontalHeader()->setResizeMode(4, QHeaderView::ResizeToContents);

	if (m_fitData->modelType != XYFitCurve::Custom) {	// pre-defined model
		ui.tableWidget->setRowCount(m_fitData->paramNames.size());

		for (int i=0; i < m_fitData->paramNames.size(); ++i){
			// name
			QTableWidgetItem* item = new QTableWidgetItem(m_fitData->paramNames.at(i));
			item->setFlags(item->flags() ^ Qt::ItemIsEditable);
			item->setBackground(QBrush(Qt::lightGray));
			ui.tableWidget->setItem(i, 0, item);

			// start value
			QLineEdit *le = new QLineEdit(ui.tableWidget);
			le->setValidator(new QDoubleValidator(le));
			le->setFrame(false);
			le->insert(QString::number(m_fitData->paramStartValues.at(i), 'g'));
			ui.tableWidget->setCellWidget(i, 1, le);
			connect( le, SIGNAL(textChanged(QString)), this, SLOT(changed()) );

			//TODO: fixed parameter widget
			ui.tableWidget->setItem(i, 2, new QTableWidgetItem());

			// limits
			le = new QLineEdit(ui.tableWidget);
			le->setValidator(new QDoubleValidator(le));
			le->setFrame(false);
			if (m_fitData->paramLowerLimits.at(i) > -DBL_MAX)
				le->insert(QString::number(m_fitData->paramLowerLimits.at(i), 'g'));
			ui.tableWidget->setCellWidget(i, 3, le);
			connect( le, SIGNAL(textChanged(QString)), this, SLOT(changed()) );

			le = new QLineEdit(ui.tableWidget);
			le->setValidator(new QDoubleValidator(le));
			le->setFrame(false);
			if (m_fitData->paramUpperLimits.at(i) < DBL_MAX)
				le->insert(QString::number(m_fitData->paramUpperLimits.at(i), 'g'));
			ui.tableWidget->setCellWidget(i, 4, le);
			connect( le, SIGNAL(textChanged(QString)), this, SLOT(changed()) );
		}
		ui.tableWidget->setCurrentCell(0, 1);
		ui.pbAdd->setVisible(false);
		ui.pbRemove->setVisible(false);
	} else {	// custom model
		if (m_fitData->paramNames.size()) {	// parameters for the custom model are already available -> show them
			ui.tableWidget->setRowCount(m_fitData->paramNames.size());

			for (int i=0; i < m_fitData->paramNames.size(); ++i){
				// name
				QTableWidgetItem* item = new QTableWidgetItem(m_fitData->paramNames.at(i));
				item->setBackground(QBrush(Qt::lightGray));
				ui.tableWidget->setItem(i, 0, item);

				// start value
				QLineEdit *le = new QLineEdit(ui.tableWidget);
				le->setValidator(new QDoubleValidator(le));
				le->setFrame(false);
				le->insert(QString::number(m_fitData->paramStartValues.at(i), 'g'));
				ui.tableWidget->setCellWidget(i, 1, le);
				connect( le, SIGNAL(textChanged(QString)), this, SLOT(changed()) );

				//TODO: fixed parameter
				ui.tableWidget->setItem(i, 2, new QTableWidgetItem());

				// limits
				le = new QLineEdit(ui.tableWidget);
				le->setValidator(new QDoubleValidator(le));
				le->setFrame(false);
				if (m_fitData->paramLowerLimits.at(i) > -DBL_MAX)
					le->insert(QString::number(m_fitData->paramLowerLimits.at(i), 'g'));
				ui.tableWidget->setCellWidget(i, 3, le);
				connect( le, SIGNAL(textChanged(QString)), this, SLOT(changed()) );
				
				le = new QLineEdit(ui.tableWidget);
				le->setValidator(new QDoubleValidator(le));
				le->setFrame(false);
				if (m_fitData->paramUpperLimits.at(i) < DBL_MAX)
					le->insert(QString::number(m_fitData->paramUpperLimits.at(i), 'g'));
				ui.tableWidget->setCellWidget(i, 4, le);
				connect( le, SIGNAL(textChanged(QString)), this, SLOT(changed()) );
			}
		} else {			// no parameters available yet -> create the first row in the table for the first parameter
			ui.tableWidget->setRowCount(1);
			// name
			QTableWidgetItem* item = new QTableWidgetItem();
			item->setBackground(QBrush(Qt::lightGray));
			ui.tableWidget->setItem(0, 0, item);

			// start value
			QLineEdit *le = new QLineEdit(ui.tableWidget);
			le->setValidator(new QDoubleValidator(le));
			le->setFrame(false);
			ui.tableWidget->setCellWidget(0, 1, le);
			connect( le, SIGNAL(textChanged(QString)), this, SLOT(changed()) );

			// TODO: fixed
			ui.tableWidget->setItem(0, 2, new QTableWidgetItem());

			// limits
			le = new QLineEdit(ui.tableWidget);
			le->setValidator(new QDoubleValidator(le));
			le->setFrame(false);
			ui.tableWidget->setCellWidget(0, 3, le);
			connect( le, SIGNAL(textChanged(QString)), this, SLOT(changed()) );

			le = new QLineEdit(ui.tableWidget);
			le->setValidator(new QDoubleValidator(le));
			le->setFrame(false);
			ui.tableWidget->setCellWidget(0, 4, le);
			connect( le, SIGNAL(textChanged(QString)), this, SLOT(changed()) );
		}
		ui.tableWidget->setCurrentCell(0, 0);
		ui.pbAdd->setIcon(KIcon("list-add"));
		ui.pbAdd->setVisible(true);
		ui.pbRemove->setIcon(KIcon("list-remove"));
		ui.pbRemove->setVisible(true);
		ui.pbRemove->setEnabled(m_fitData->paramNames.size() > 1);
	}

	ui.tableWidget->installEventFilter(this);

	connect( ui.tableWidget, SIGNAL(cellChanged(int,int)), this, SLOT(changed()) );
	connect( ui.pbApply, SIGNAL(clicked()), this, SLOT(applyClicked()) );
	connect( ui.pbCancel, SIGNAL(clicked()), this, SIGNAL(finished()) );
	connect( ui.pbAdd, SIGNAL(clicked()), this, SLOT(addParameter()) );
	connect( ui.pbRemove, SIGNAL(clicked()), this, SLOT(removeParameter()) );
}

bool FitParametersWidget::eventFilter(QObject* watched, QEvent* event) {
	if (watched == ui.tableWidget) {
		if (event->type() == QEvent::KeyPress) {
			QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);
			if (keyEvent->key() == Qt::Key_Return || keyEvent->key() == Qt::Key_Enter) {
				if (m_fitData->modelType != XYFitCurve::Custom) {
					//on the second column with the values is editable.
					//navigate to the next cell in the second column, or to the apply-button
					if (ui.tableWidget->currentRow() == ui.tableWidget->rowCount()-1) {
						ui.pbApply->setFocus();
						ui.tableWidget->clearSelection();
					} else {
						ui.tableWidget->setCurrentCell(ui.tableWidget->currentRow()+1, 1);
					}
				} else {
					//both columns (names and start values) are editable
					if (ui.tableWidget->currentColumn() == 0) {
						//name was entered, navigate to the value-cell
						ui.tableWidget->setCurrentCell(ui.tableWidget->currentRow(), 1);
					} else {
						//start value was entered, navigate to the next name-cell or to the apply-button
						if (ui.tableWidget->currentRow() == ui.tableWidget->rowCount()-1) {
							ui.pbApply->setFocus();
							ui.tableWidget->clearSelection();
						} else {
							ui.tableWidget->setCurrentCell(ui.tableWidget->currentRow()+1, 0);
						}
					}
				}
				return true;
			}
		}
	}

	return QWidget::eventFilter(watched, event);
}

void FitParametersWidget::applyClicked() {
	//TODO: set fixed flag
	if (m_fitData->modelType != XYFitCurve::Custom) {	// pre-defined models
		for (int i=0; i < ui.tableWidget->rowCount(); ++i) {
			m_fitData->paramStartValues[i] = ((QLineEdit *)ui.tableWidget->cellWidget(i, 1))->text().toDouble();
			if ( !((QLineEdit *)ui.tableWidget->cellWidget(i, 3))->text().isEmpty() )
				m_fitData->paramLowerLimits[i] = ((QLineEdit *)ui.tableWidget->cellWidget(i, 3))->text().toDouble();
			else
				m_fitData->paramLowerLimits[i] = -DBL_MAX;
			if ( !((QLineEdit *)ui.tableWidget->cellWidget(i, 4))->text().isEmpty() )
				m_fitData->paramUpperLimits[i] = ((QLineEdit *)ui.tableWidget->cellWidget(i, 4))->text().toDouble();
			else
				m_fitData->paramUpperLimits[i] = DBL_MAX;
		}
	} else {	// custom model
		m_fitData->paramNames.clear();
		m_fitData->paramStartValues.clear();
		m_fitData->paramLowerLimits.clear();
		m_fitData->paramUpperLimits.clear();
		for (int i=0; i < ui.tableWidget->rowCount(); ++i) {
			//skip those rows where either the name or the value are empty
			if ( !ui.tableWidget->item(i, 0)->text().simplified().isEmpty()
				&& !((QLineEdit *)ui.tableWidget->cellWidget(i, 1))->text().simplified().isEmpty() ) {
				m_fitData->paramNames.append( ui.tableWidget->item(i, 0)->text() );
				m_fitData->paramStartValues.append( ((QLineEdit *)ui.tableWidget->cellWidget(i, 1))->text().toDouble() );
				if ( !((QLineEdit *)ui.tableWidget->cellWidget(i, 3))->text().isEmpty() )
					m_fitData->paramLowerLimits.append( ((QLineEdit *)ui.tableWidget->cellWidget(i, 3))->text().toDouble() );
				else
					m_fitData->paramLowerLimits.append(-DBL_MAX);
				if ( !((QLineEdit *)ui.tableWidget->cellWidget(i, 4))->text().isEmpty() )
					m_fitData->paramUpperLimits.append( ((QLineEdit *)ui.tableWidget->cellWidget(i, 4))->text().toDouble() );
				else
					m_fitData->paramUpperLimits.append(DBL_MAX);
			}
		}
	}

	if (m_changed)
		emit(parametersChanged());

	emit(finished());
}

void FitParametersWidget::addParameter() {
	int rows = ui.tableWidget->rowCount();
	ui.tableWidget->setRowCount(rows+1);

	// name
	QTableWidgetItem* item = new QTableWidgetItem();
	item->setBackground(QBrush(Qt::lightGray));
	ui.tableWidget->setItem(rows, 0, item);

	// start value
	QLineEdit *le = new QLineEdit(ui.tableWidget);
	le->setValidator(new QDoubleValidator(le));
	le->setFrame(false);
	le->insert("1");
	ui.tableWidget->setCellWidget(rows, 1, le);
	connect( le, SIGNAL(textChanged(QString)), this, SLOT(changed()) );

	// TODO: fixed
	ui.tableWidget->setItem(rows, 2, new QTableWidgetItem());

	// limits
	le = new QLineEdit(ui.tableWidget);
	le->setValidator(new QDoubleValidator(le));
	le->setFrame(false);
	ui.tableWidget->setCellWidget(rows, 3, le);
	connect( le, SIGNAL(textChanged(QString)), this, SLOT(changed()) );

	le = new QLineEdit(ui.tableWidget);
	le->setValidator(new QDoubleValidator(le));
	le->setFrame(false);
	ui.tableWidget->setCellWidget(rows, 4, le);
	connect( le, SIGNAL(textChanged(QString)), this, SLOT(changed()) );

	ui.tableWidget->setCurrentCell(rows, 0);
	ui.pbRemove->setEnabled(true);
}

void FitParametersWidget::removeParameter() {
	ui.tableWidget->removeRow(ui.tableWidget->currentRow());
	if (ui.tableWidget->rowCount() == 1)
		ui.pbRemove->setEnabled(false);
}

void FitParametersWidget::changed() {
	m_changed = true;
}
