/***************************************************************************
    File                 : FitParametersWidget.cc
    Project              : LabPlot
    --------------------------------------------------------------------
    Copyright            : (C) 2014 Alexander Semke (alexander.semke*web.de)
    Description          : widget for editing fit parameters

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

/*!
	\class FitParametersWidget
	\brief Widget for editing fit parameters. For predefined models the number of parameters,
	their names and default values are given - the user can change the start values.
	For custom models the user has to define here the parameter names and their start values.

	\ingroup kdefrontend
 */
FitParametersWidget::FitParametersWidget(QWidget* parent, XYFitCurve::FitData* data) : QWidget(parent), m_fitData(data) {
	ui.setupUi(this);
	ui.pbApply->setIcon(KIcon("dialog-ok-apply"));

	ui.tableWidget->setColumnCount(2);
	ui.tableWidget->setRowCount(m_fitData->paramNames.size());

	QTableWidgetItem* headerItem = new QTableWidgetItem();
	headerItem->setText(i18n("Name"));
	ui.tableWidget->setHorizontalHeaderItem(0, headerItem);

	headerItem = new QTableWidgetItem();
	headerItem->setText(i18n("Start value"));
	ui.tableWidget->setHorizontalHeaderItem(1, headerItem);

	ui.tableWidget->horizontalHeader()->setResizeMode(0, QHeaderView::ResizeToContents);
	ui.tableWidget->horizontalHeader()->setResizeMode(1, QHeaderView::Stretch);

	for (int i=0; i<m_fitData->paramNames.size(); ++i){
		ui.tableWidget->setItem(i, 0, new QTableWidgetItem(m_fitData->paramNames.at(i)));
		ui.tableWidget->setItem(i, 1, new QTableWidgetItem(QString::number(m_fitData->paramStartValues.at(i), 'g')));
	}

	//SLOTS
	connect( ui.pbApply, SIGNAL(clicked()), this, SLOT(applyClicked()) );
}

void FitParametersWidget::applyClicked() {
	if (m_fitData->paramStartValues.size() != ui.tableWidget->rowCount())
		m_fitData->paramStartValues.resize(ui.tableWidget->rowCount());

	for (int i=0; i<ui.tableWidget->rowCount(); ++i)
		m_fitData->paramStartValues[i] = ui.tableWidget->item(i,1)->text().toDouble();

	emit(finished());
}
