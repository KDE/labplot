/***************************************************************************
    File             : XYDataReductionCurveDock.cpp
    Project          : LabPlot
    --------------------------------------------------------------------
    Copyright        : (C) 2016 Stefan Gerlach (stefan.gerlach@uni.kn)
    Description      : widget for editing properties of data reduction curves

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

#include "XYDataReductionCurveDock.h"
#include "backend/core/AspectTreeModel.h"
#include "backend/core/Project.h"
#include "backend/worksheet/plots/cartesian/XYDataReductionCurve.h"
#include "commonfrontend/widgets/TreeViewComboBox.h"

#include <QMenu>
#include <QWidgetAction>
#include <QStandardItemModel>

#include <cmath>        // isnan

/*!
  \class XYDataReductionCurveDock
 \brief  Provides a widget for editing the properties of the XYDataReductionCurves
		(2D-curves defined by an data reduction) currently selected in
		the project explorer.

  If more then one curves are set, the properties of the first column are shown.
  The changes of the properties are applied to all curves.
  The exclusions are the name, the comment and the datasets (columns) of
  the curves  - these properties can only be changed if there is only one single curve.

  \ingroup kdefrontend
*/

XYDataReductionCurveDock::XYDataReductionCurveDock(QWidget *parent): 
	XYCurveDock(parent), cbXDataColumn(0), cbYDataColumn(0), m_dataReductionCurve(0), dataPoints(0) {

	//hide the line connection type
	ui.cbLineType->setDisabled(true);

	//remove the tab "Error bars"
	ui.tabWidget->removeTab(5);
}

/*!
 * 	// Tab "General"
 */
void XYDataReductionCurveDock::setupGeneral() {
	QWidget* generalTab = new QWidget(ui.tabGeneral);
	uiGeneralTab.setupUi(generalTab);

	QGridLayout* gridLayout = dynamic_cast<QGridLayout*>(generalTab->layout());
	if (gridLayout) {
		gridLayout->setContentsMargins(2,2,2,2);
		gridLayout->setHorizontalSpacing(2);
		gridLayout->setVerticalSpacing(2);
	}

	cbXDataColumn = new TreeViewComboBox(generalTab);
	gridLayout->addWidget(cbXDataColumn, 4, 3, 1, 2);
	cbYDataColumn = new TreeViewComboBox(generalTab);
	gridLayout->addWidget(cbYDataColumn, 5, 3, 1, 2);

	for (int i=0; i < NSL_GEOM_LINESIM_TYPE_COUNT; i++)
		uiGeneralTab.cbType->addItem(i18n(nsl_geom_linesim_type_name[i]));

	uiGeneralTab.pbRecalculate->setIcon(KIcon("run-build"));

	QHBoxLayout* layout = new QHBoxLayout(ui.tabGeneral);
	layout->setMargin(0);
	layout->addWidget(generalTab);

	//Slots
	connect( uiGeneralTab.leName, SIGNAL(returnPressed()), this, SLOT(nameChanged()) );
	connect( uiGeneralTab.leComment, SIGNAL(returnPressed()), this, SLOT(commentChanged()) );
	connect( uiGeneralTab.chkVisible, SIGNAL(clicked(bool)), this, SLOT(visibilityChanged(bool)) );

	connect( uiGeneralTab.cbType, SIGNAL(currentIndexChanged(int)), this, SLOT(typeChanged()) );
	//TODO: connect Auto

	connect( uiGeneralTab.pbRecalculate, SIGNAL(clicked()), this, SLOT(recalculateClicked()) );
}

void XYDataReductionCurveDock::initGeneralTab() {
	//if there are more then one curve in the list, disable the tab "general"
	if (m_curvesList.size()==1) {
		uiGeneralTab.lName->setEnabled(true);
		uiGeneralTab.leName->setEnabled(true);
		uiGeneralTab.lComment->setEnabled(true);
		uiGeneralTab.leComment->setEnabled(true);

		uiGeneralTab.leName->setText(m_curve->name());
		uiGeneralTab.leComment->setText(m_curve->comment());
	}else {
		uiGeneralTab.lName->setEnabled(false);
		uiGeneralTab.leName->setEnabled(false);
		uiGeneralTab.lComment->setEnabled(false);
		uiGeneralTab.leComment->setEnabled(false);

		uiGeneralTab.leName->setText("");
		uiGeneralTab.leComment->setText("");
	}

	//show the properties of the first curve
	m_dataReductionCurve = dynamic_cast<XYDataReductionCurve*>(m_curve);
	Q_ASSERT(m_dataReductionCurve);
	XYCurveDock::setModelIndexFromColumn(cbXDataColumn, m_dataReductionCurve->xDataColumn());
	XYCurveDock::setModelIndexFromColumn(cbYDataColumn, m_dataReductionCurve->yDataColumn());
	// update list of selectable types
	xDataColumnChanged(cbXDataColumn->currentModelIndex());

	uiGeneralTab.cbType->setCurrentIndex(m_dataReductionData.type);
	this->typeChanged();

	this->showDataReductionResult();

	//enable the "recalculate"-button if the source data was changed since the last dataReduction
	uiGeneralTab.pbRecalculate->setEnabled(m_dataReductionCurve->isSourceDataChangedSinceLastDataReduction());

	uiGeneralTab.chkVisible->setChecked( m_curve->isVisible() );

	//Slots
	connect(m_dataReductionCurve, SIGNAL(aspectDescriptionChanged(const AbstractAspect*)), this, SLOT(curveDescriptionChanged(const AbstractAspect*)));
	connect(m_dataReductionCurve, SIGNAL(xDataColumnChanged(const AbstractColumn*)), this, SLOT(curveXDataColumnChanged(const AbstractColumn*)));
	connect(m_dataReductionCurve, SIGNAL(yDataColumnChanged(const AbstractColumn*)), this, SLOT(curveYDataColumnChanged(const AbstractColumn*)));
	connect(m_dataReductionCurve, SIGNAL(dataReductionDataChanged(XYDataReductionCurve::DataReductionData)), this, SLOT(curveDataReductionDataChanged(XYDataReductionCurve::DataReductionData)));
	connect(m_dataReductionCurve, SIGNAL(sourceDataChangedSinceLastDataReduction()), this, SLOT(enableRecalculate()));
}

void XYDataReductionCurveDock::setModel() {
	QList<const char*>  list;
	list<<"Folder"<<"Workbook"<<"Datapicker"<<"DatapickerCurve"<<"Spreadsheet"
		<<"FileDataSource"<<"Column"<<"Worksheet"<<"CartesianPlot"<<"XYFitCurve";
	cbXDataColumn->setTopLevelClasses(list);
	cbYDataColumn->setTopLevelClasses(list);

 	list.clear();
	list<<"Column";
	cbXDataColumn->setSelectableClasses(list);
	cbYDataColumn->setSelectableClasses(list);

	connect( cbXDataColumn, SIGNAL(currentModelIndexChanged(QModelIndex)), this, SLOT(xDataColumnChanged(QModelIndex)) );
	connect( cbYDataColumn, SIGNAL(currentModelIndexChanged(QModelIndex)), this, SLOT(yDataColumnChanged(QModelIndex)) );

	cbXDataColumn->setModel(m_aspectTreeModel);
	cbYDataColumn->setModel(m_aspectTreeModel);

	XYCurveDock::setModel();
}

/*!
  sets the curves. The properties of the curves in the list \c list can be edited in this widget.
*/
void XYDataReductionCurveDock::setCurves(QList<XYCurve*> list) {
	m_initializing=true;
	m_curvesList=list;
	m_curve=list.first();
	m_dataReductionCurve = dynamic_cast<XYDataReductionCurve*>(m_curve);
	Q_ASSERT(m_dataReductionCurve);
	m_aspectTreeModel = new AspectTreeModel(m_curve->project());
	this->setModel();
	m_dataReductionData = m_dataReductionCurve->dataReductionData();
	initGeneralTab();
	initTabs();
	m_initializing=false;

	//hide the "skip gaps" option after the curves were set
	ui.lLineSkipGaps->hide();
	ui.chkLineSkipGaps->hide();
}

//*************************************************************
//**** SLOTs for changes triggered in XYFitCurveDock *****
//*************************************************************
void XYDataReductionCurveDock::nameChanged() {
	if (m_initializing)
		return;

	m_curve->setName(uiGeneralTab.leName->text());
}

void XYDataReductionCurveDock::commentChanged() {
	if (m_initializing)
		return;

	m_curve->setComment(uiGeneralTab.leComment->text());
}

void XYDataReductionCurveDock::xDataColumnChanged(const QModelIndex& index) {
	AbstractAspect* aspect = static_cast<AbstractAspect*>(index.internalPointer());
	AbstractColumn* column = 0;
	if (aspect) {
		column = dynamic_cast<AbstractColumn*>(aspect);
		Q_ASSERT(column);
	}

	foreach(XYCurve* curve, m_curvesList)
		dynamic_cast<XYDataReductionCurve*>(curve)->setXDataColumn(column);

/*	// disable types that need more data points
	if (column != 0) {
		unsigned int n=0;
		for (int row=0; row < column->rowCount(); row++)
			if (!std::isnan(column->valueAt(row)) && !column->isMasked(row)) 
				n++;
		dataPoints = n;
		if(m_dataReductionData.pointsMode == XYDataReductionCurve::Auto)
			pointsModeChanged();

		const QStandardItemModel* model = qobject_cast<const QStandardItemModel*>(uiGeneralTab.cbType->model());
		QStandardItem* item = model->item(nsl_interp_type_polynomial);
		if (dataPoints < gsl_interp_type_min_size(gsl_interp_polynomial) || dataPoints > 100) {	// not good for many points
			item->setFlags(item->flags() & ~(Qt::ItemIsSelectable|Qt::ItemIsEnabled));
			if (uiGeneralTab.cbType->currentIndex() == nsl_interp_type_polynomial)
				uiGeneralTab.cbType->setCurrentIndex(0);
		}
		else
			item->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEnabled);

		item = model->item(nsl_interp_type_cspline);
		if (dataPoints < gsl_interp_type_min_size(gsl_interp_cspline)) {
			item->setFlags(item->flags() & ~(Qt::ItemIsSelectable|Qt::ItemIsEnabled));
			if (uiGeneralTab.cbType->currentIndex() == nsl_interp_type_cspline)
				uiGeneralTab.cbType->setCurrentIndex(0);
		}
		else
			item->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEnabled);

		item = model->item(nsl_interp_type_cspline_periodic);
		if (dataPoints < gsl_interp_type_min_size(gsl_interp_cspline_periodic)) {
			item->setFlags(item->flags() & ~(Qt::ItemIsSelectable|Qt::ItemIsEnabled));
			if (uiGeneralTab.cbType->currentIndex() == nsl_interp_type_cspline_periodic)
				uiGeneralTab.cbType->setCurrentIndex(0);
		}
		else
			item->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEnabled);

		item = model->item(nsl_interp_type_akima);
		if (dataPoints < gsl_interp_type_min_size(gsl_interp_akima)) {
			item->setFlags(item->flags() & ~(Qt::ItemIsSelectable|Qt::ItemIsEnabled));
			if (uiGeneralTab.cbType->currentIndex() == nsl_interp_type_akima)
				uiGeneralTab.cbType->setCurrentIndex(0);
		}
		else
			item->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEnabled);

		item = model->item(nsl_interp_type_akima_periodic);
		if (dataPoints < gsl_interp_type_min_size(gsl_interp_akima_periodic)) {
			item->setFlags(item->flags() & ~(Qt::ItemIsSelectable|Qt::ItemIsEnabled));
			if (uiGeneralTab.cbType->currentIndex() == nsl_interp_type_akima_periodic)
				uiGeneralTab.cbType->setCurrentIndex(0);
		}
		else
			item->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEnabled);

#if GSL_MAJOR_VERSION >= 2
		item = model->item(nsl_interp_type_steffen);
		if (dataPoints < gsl_interp_type_min_size(gsl_interp_steffen)) {
			item->setFlags(item->flags() & ~(Qt::ItemIsSelectable|Qt::ItemIsEnabled));
			if (uiGeneralTab.cbType->currentIndex() == nsl_interp_type_steffen)
				uiGeneralTab.cbType->setCurrentIndex(0);
		}
		else
			item->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEnabled);
#endif

		// own types work with 2 or more data points
	}
*/
}

void XYDataReductionCurveDock::yDataColumnChanged(const QModelIndex& index) {
	if (m_initializing)
		return;

	AbstractAspect* aspect = static_cast<AbstractAspect*>(index.internalPointer());
	AbstractColumn* column = 0;
	if (aspect) {
		column = dynamic_cast<AbstractColumn*>(aspect);
		Q_ASSERT(column);
	}

	foreach(XYCurve* curve, m_curvesList)
		dynamic_cast<XYDataReductionCurve*>(curve)->setYDataColumn(column);
}

void XYDataReductionCurveDock::typeChanged() {
	nsl_geom_linesim_type type = (nsl_geom_linesim_type)uiGeneralTab.cbType->currentIndex();
	m_dataReductionData.type = type;

//	switch (type) {
// TODO
//	}

	uiGeneralTab.pbRecalculate->setEnabled(true);
}

void XYDataReductionCurveDock::recalculateClicked() {
	QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

	foreach(XYCurve* curve, m_curvesList)
		dynamic_cast<XYDataReductionCurve*>(curve)->setDataReductionData(m_dataReductionData);

	uiGeneralTab.pbRecalculate->setEnabled(false);
	QApplication::restoreOverrideCursor();
}

void XYDataReductionCurveDock::enableRecalculate() const {
	if (m_initializing)
		return;

	//no dataReductioning possible without the x- and y-data
	AbstractAspect* aspectX = static_cast<AbstractAspect*>(cbXDataColumn->currentModelIndex().internalPointer());
	AbstractAspect* aspectY = static_cast<AbstractAspect*>(cbYDataColumn->currentModelIndex().internalPointer());
	bool data = (aspectX!=0 && aspectY!=0);

	uiGeneralTab.pbRecalculate->setEnabled(data);
}

/*!
 * show the result and details of the dataReduction
 */
void XYDataReductionCurveDock::showDataReductionResult() {
	const XYDataReductionCurve::DataReductionResult& dataReductionResult = m_dataReductionCurve->dataReductionResult();
	if (!dataReductionResult.available) {
		uiGeneralTab.teResult->clear();
		return;
	}

	//const XYDataReductionCurve::DataReductionData& dataReductionData = m_dataReductionCurve->dataReductionData();
	QString str = i18n("status:") + ' ' + dataReductionResult.status + "<br>";

	if (!dataReductionResult.valid) {
		uiGeneralTab.teResult->setText(str);
		return; //result is not valid, there was an error which is shown in the status-string, nothing to show more.
	}

	if (dataReductionResult.elapsedTime>1000)
		str += i18n("calculation time: %1 s").arg(QString::number(dataReductionResult.elapsedTime/1000)) + "<br>";
	else
		str += i18n("calculation time: %1 ms").arg(QString::number(dataReductionResult.elapsedTime)) + "<br>";

 	str += "<br><br>";

	uiGeneralTab.teResult->setText(str);
}

//*************************************************************
//*********** SLOTs for changes triggered in XYCurve **********
//*************************************************************
//General-Tab
void XYDataReductionCurveDock::curveDescriptionChanged(const AbstractAspect* aspect) {
	if (m_curve != aspect)
		return;

	m_initializing = true;
	if (aspect->name() != uiGeneralTab.leName->text()) {
		uiGeneralTab.leName->setText(aspect->name());
	} else if (aspect->comment() != uiGeneralTab.leComment->text()) {
		uiGeneralTab.leComment->setText(aspect->comment());
	}
	m_initializing = false;
}

void XYDataReductionCurveDock::curveXDataColumnChanged(const AbstractColumn* column) {
	m_initializing = true;
	XYCurveDock::setModelIndexFromColumn(cbXDataColumn, column);
	m_initializing = false;
}

void XYDataReductionCurveDock::curveYDataColumnChanged(const AbstractColumn* column) {
	m_initializing = true;
	XYCurveDock::setModelIndexFromColumn(cbYDataColumn, column);
	m_initializing = false;
}

void XYDataReductionCurveDock::curveDataReductionDataChanged(const XYDataReductionCurve::DataReductionData& data) {
	m_initializing = true;
	m_dataReductionData = data;
	//uiGeneralTab.cbType->setCurrentIndex(m_dataReductionData.type);
	//this->typeChanged();

	this->showDataReductionResult();
	m_initializing = false;
}

void XYDataReductionCurveDock::dataChanged() {
	this->enableRecalculate();
}
