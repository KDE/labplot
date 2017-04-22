/***************************************************************************
    File             : XYIntegrationCurveDock.cpp
    Project          : LabPlot
    --------------------------------------------------------------------
    Copyright        : (C) 2016 Stefan Gerlach (stefan.gerlach@uni.kn)
    Description      : widget for editing properties of integration curves

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

#include "XYIntegrationCurveDock.h"
#include "backend/core/AspectTreeModel.h"
#include "backend/core/Project.h"
#include "backend/worksheet/plots/cartesian/XYIntegrationCurve.h"
#include "commonfrontend/widgets/TreeViewComboBox.h"

#include <QMenu>
#include <QWidgetAction>
#include <QStandardItemModel>

extern "C" {
#include "backend/nsl/nsl_int.h"
}
#include <cmath>        // isnan

/*!
  \class XYIntegrationCurveDock
 \brief  Provides a widget for editing the properties of the XYIntegrationCurves
		(2D-curves defined by a integration) currently selected in
		the project explorer.

  If more then one curves are set, the properties of the first column are shown.
  The changes of the properties are applied to all curves.
  The exclusions are the name, the comment and the datasets (columns) of
  the curves  - these properties can only be changed if there is only one single curve.

  \ingroup kdefrontend
*/

XYIntegrationCurveDock::XYIntegrationCurveDock(QWidget *parent): 
	XYCurveDock(parent), cbXDataColumn(0), cbYDataColumn(0), m_integrationCurve(0) {

	//hide the line connection type
	ui.cbLineType->setDisabled(true);

	//remove the tab "Error bars"
	ui.tabWidget->removeTab(5);
}

/*!
 * 	// Tab "General"
 */
void XYIntegrationCurveDock::setupGeneral() {
	QWidget* generalTab = new QWidget(ui.tabGeneral);
	uiGeneralTab.setupUi(generalTab);

	QGridLayout* gridLayout = dynamic_cast<QGridLayout*>(generalTab->layout());
	if (gridLayout) {
		gridLayout->setContentsMargins(2,2,2,2);
		gridLayout->setHorizontalSpacing(2);
		gridLayout->setVerticalSpacing(2);
	}

	cbXDataColumn = new TreeViewComboBox(generalTab);
	gridLayout->addWidget(cbXDataColumn, 4, 2, 1, 3);
	cbYDataColumn = new TreeViewComboBox(generalTab);
	gridLayout->addWidget(cbYDataColumn, 5, 2, 1, 3);

	for (int i=0; i < NSL_INT_NETHOD_COUNT; i++)
		uiGeneralTab.cbMethod->addItem(i18n(nsl_int_method_name[i]));

	uiGeneralTab.pbRecalculate->setIcon(QIcon::fromTheme("run-build"));

	QHBoxLayout* layout = new QHBoxLayout(ui.tabGeneral);
	layout->setMargin(0);
	layout->addWidget(generalTab);

	//Slots
	connect( uiGeneralTab.leName, SIGNAL(returnPressed()), this, SLOT(nameChanged()) );
	connect( uiGeneralTab.leComment, SIGNAL(returnPressed()), this, SLOT(commentChanged()) );
	connect( uiGeneralTab.chkVisible, SIGNAL(clicked(bool)), this, SLOT(visibilityChanged(bool)) );
	connect( uiGeneralTab.cbAutoRange, SIGNAL(clicked(bool)), this, SLOT(autoRangeChanged()) );
	connect( uiGeneralTab.sbMin, SIGNAL(valueChanged(double)), this, SLOT(xRangeMinChanged()) );
	connect( uiGeneralTab.sbMax, SIGNAL(valueChanged(double)), this, SLOT(xRangeMaxChanged()) );

	connect( uiGeneralTab.cbMethod, SIGNAL(currentIndexChanged(int)), this, SLOT(methodChanged()) );
	connect( uiGeneralTab.cbAbsolute, SIGNAL(clicked(bool)), this, SLOT(absoluteChanged()) );

	connect( uiGeneralTab.pbRecalculate, SIGNAL(clicked()), this, SLOT(recalculateClicked()) );
}

void XYIntegrationCurveDock::initGeneralTab() {
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
	if (m_curve != 0)
		m_integrationCurve = dynamic_cast<XYIntegrationCurve*>(m_curve);
	Q_ASSERT(m_integrationCurve);
	XYCurveDock::setModelIndexFromColumn(cbXDataColumn, m_integrationCurve->xDataColumn());
	XYCurveDock::setModelIndexFromColumn(cbYDataColumn, m_integrationCurve->yDataColumn());
	uiGeneralTab.cbAutoRange->setChecked(m_integrationData.autoRange);
	uiGeneralTab.sbMin->setValue(m_integrationData.xRange.first());
	uiGeneralTab.sbMax->setValue(m_integrationData.xRange.last());
	this->autoRangeChanged();
	// update list of selectable types
	xDataColumnChanged(cbXDataColumn->currentModelIndex());

	uiGeneralTab.cbMethod->setCurrentIndex(m_integrationData.method);
	this->methodChanged();
	uiGeneralTab.cbAbsolute->setChecked(m_integrationData.absolute);
	this->absoluteChanged();

	this->showIntegrationResult();

	//enable the "recalculate"-button if the source data was changed since the last integration
	uiGeneralTab.pbRecalculate->setEnabled(m_integrationCurve->isSourceDataChangedSinceLastIntegration());

	uiGeneralTab.chkVisible->setChecked( m_curve->isVisible() );

	//Slots
	connect(m_integrationCurve, SIGNAL(aspectDescriptionChanged(const AbstractAspect*)), this, SLOT(curveDescriptionChanged(const AbstractAspect*)));
	connect(m_integrationCurve, SIGNAL(xDataColumnChanged(const AbstractColumn*)), this, SLOT(curveXDataColumnChanged(const AbstractColumn*)));
	connect(m_integrationCurve, SIGNAL(yDataColumnChanged(const AbstractColumn*)), this, SLOT(curveYDataColumnChanged(const AbstractColumn*)));
	connect(m_integrationCurve, SIGNAL(integrationDataChanged(XYIntegrationCurve::IntegrationData)), this, SLOT(curveIntegrationDataChanged(XYIntegrationCurve::IntegrationData)));
	connect(m_integrationCurve, SIGNAL(sourceDataChangedSinceLastIntegration()), this, SLOT(enableRecalculate()));
}

void XYIntegrationCurveDock::setModel() {
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
void XYIntegrationCurveDock::setCurves(QList<XYCurve*> list) {
	m_initializing=true;
	m_curvesList=list;
	m_curve=list.first();
	m_integrationCurve = dynamic_cast<XYIntegrationCurve*>(m_curve);
	Q_ASSERT(m_integrationCurve);
	m_aspectTreeModel = new AspectTreeModel(m_curve->project());
	this->setModel();
	m_integrationData = m_integrationCurve->integrationData();
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
void XYIntegrationCurveDock::nameChanged() {
	if (m_initializing)
		return;

	m_curve->setName(uiGeneralTab.leName->text());
}

void XYIntegrationCurveDock::commentChanged() {
	if (m_initializing)
		return;

	m_curve->setComment(uiGeneralTab.leComment->text());
}

void XYIntegrationCurveDock::xDataColumnChanged(const QModelIndex& index) {
	AbstractAspect* aspect = static_cast<AbstractAspect*>(index.internalPointer());
	AbstractColumn* column = 0;
	if (aspect) {
		column = dynamic_cast<AbstractColumn*>(aspect);
		Q_ASSERT(column);
	}

	foreach(XYCurve* curve, m_curvesList)
		dynamic_cast<XYIntegrationCurve*>(curve)->setXDataColumn(column);

	if (column != 0) {
		if (uiGeneralTab.cbAutoRange->isChecked()) {
			uiGeneralTab.sbMin->setValue(column->minimum());
			uiGeneralTab.sbMax->setValue(column->maximum());
		}

		size_t n=0;
		for (int row=0; row < column->rowCount(); row++)
			if (!std::isnan(column->valueAt(row)) && !column->isMasked(row))
				n++;

		// TODO: disable integration methods that need more data points
	}
}

void XYIntegrationCurveDock::yDataColumnChanged(const QModelIndex& index) {
	if (m_initializing)
		return;

	AbstractAspect* aspect = static_cast<AbstractAspect*>(index.internalPointer());
	AbstractColumn* column = 0;
	if (aspect) {
		column = dynamic_cast<AbstractColumn*>(aspect);
		Q_ASSERT(column);
	}

	foreach(XYCurve* curve, m_curvesList)
		dynamic_cast<XYIntegrationCurve*>(curve)->setYDataColumn(column);
}

void XYIntegrationCurveDock::autoRangeChanged() {
	bool autoRange = uiGeneralTab.cbAutoRange->isChecked();
	m_integrationData.autoRange = autoRange;

	if (autoRange) {
		uiGeneralTab.lMin->setEnabled(false);
		uiGeneralTab.sbMin->setEnabled(false);
		uiGeneralTab.lMax->setEnabled(false);
		uiGeneralTab.sbMax->setEnabled(false);
		m_integrationCurve = dynamic_cast<XYIntegrationCurve*>(m_curve);
		Q_ASSERT(m_integrationCurve);
		if (m_integrationCurve->xDataColumn()) {
			uiGeneralTab.sbMin->setValue(m_integrationCurve->xDataColumn()->minimum());
			uiGeneralTab.sbMax->setValue(m_integrationCurve->xDataColumn()->maximum());
		}
	} else {
		uiGeneralTab.lMin->setEnabled(true);
		uiGeneralTab.sbMin->setEnabled(true);
		uiGeneralTab.lMax->setEnabled(true);
		uiGeneralTab.sbMax->setEnabled(true);
	}

}
void XYIntegrationCurveDock::xRangeMinChanged() {
	double xMin = uiGeneralTab.sbMin->value();

	m_integrationData.xRange.first() = xMin;
	uiGeneralTab.pbRecalculate->setEnabled(true);
}

void XYIntegrationCurveDock::xRangeMaxChanged() {
	double xMax = uiGeneralTab.sbMax->value();

	m_integrationData.xRange.last() = xMax;
	uiGeneralTab.pbRecalculate->setEnabled(true);
}

void XYIntegrationCurveDock::methodChanged() {
	nsl_int_method_type method = (nsl_int_method_type)uiGeneralTab.cbMethod->currentIndex();
	m_integrationData.method = method;

	// update absolute option
	switch (method) {
	case nsl_int_method_rectangle:
	case nsl_int_method_trapezoid:
		uiGeneralTab.cbAbsolute->setEnabled(true);
		break;
	case nsl_int_method_simpson:
	case nsl_int_method_simpson_3_8:
		uiGeneralTab.cbAbsolute->setChecked(false);
		uiGeneralTab.cbAbsolute->setEnabled(false);
	}

	uiGeneralTab.pbRecalculate->setEnabled(true);
}

void XYIntegrationCurveDock::absoluteChanged() {
	bool absolute = uiGeneralTab.cbAbsolute->isChecked();
	m_integrationData.absolute = absolute;

	uiGeneralTab.pbRecalculate->setEnabled(true);
}

void XYIntegrationCurveDock::recalculateClicked() {
	QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

	foreach(XYCurve* curve, m_curvesList)
		dynamic_cast<XYIntegrationCurve*>(curve)->setIntegrationData(m_integrationData);

	uiGeneralTab.pbRecalculate->setEnabled(false);
	QApplication::restoreOverrideCursor();
}

void XYIntegrationCurveDock::enableRecalculate() const {
	if (m_initializing)
		return;

	//no integrationing possible without the x- and y-data
	AbstractAspect* aspectX = static_cast<AbstractAspect*>(cbXDataColumn->currentModelIndex().internalPointer());
	AbstractAspect* aspectY = static_cast<AbstractAspect*>(cbYDataColumn->currentModelIndex().internalPointer());
	bool data = (aspectX!=0 && aspectY!=0);

	uiGeneralTab.pbRecalculate->setEnabled(data);
}

/*!
 * show the result and details of the integration
 */
void XYIntegrationCurveDock::showIntegrationResult() {
	const XYIntegrationCurve::IntegrationResult& integrationResult = m_integrationCurve->integrationResult();
	if (!integrationResult.available) {
		uiGeneralTab.teResult->clear();
		return;
	}

	//const XYIntegrationCurve::IntegrationData& integrationData = m_integrationCurve->integrationData();
	QString str = i18n("status:") + ' ' + integrationResult.status + "<br>";

	if (!integrationResult.valid) {
		uiGeneralTab.teResult->setText(str);
		return; //result is not valid, there was an error which is shown in the status-string, nothing to show more.
	}

	if (integrationResult.elapsedTime>1000)
		str += i18n("calculation time: %1 s").arg(QString::number(integrationResult.elapsedTime/1000)) + "<br>";
	else
		str += i18n("calculation time: %1 ms").arg(QString::number(integrationResult.elapsedTime)) + "<br>";

	str += i18n("value: ") + QString::number(integrationResult.value) + "<br>";
 	str += "<br><br>";

	uiGeneralTab.teResult->setText(str);
}

//*************************************************************
//*********** SLOTs for changes triggered in XYCurve **********
//*************************************************************
//General-Tab
void XYIntegrationCurveDock::curveDescriptionChanged(const AbstractAspect* aspect) {
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

void XYIntegrationCurveDock::curveXDataColumnChanged(const AbstractColumn* column) {
	m_initializing = true;
	XYCurveDock::setModelIndexFromColumn(cbXDataColumn, column);
	m_initializing = false;
}

void XYIntegrationCurveDock::curveYDataColumnChanged(const AbstractColumn* column) {
	m_initializing = true;
	XYCurveDock::setModelIndexFromColumn(cbYDataColumn, column);
	m_initializing = false;
}

void XYIntegrationCurveDock::curveIntegrationDataChanged(const XYIntegrationCurve::IntegrationData& data) {
	m_initializing = true;
	m_integrationData = data;
	uiGeneralTab.cbMethod->setCurrentIndex(m_integrationData.method);
	this->methodChanged();
	uiGeneralTab.cbAbsolute->setChecked(m_integrationData.absolute);
	this->absoluteChanged();

	this->showIntegrationResult();
	m_initializing = false;
}

void XYIntegrationCurveDock::dataChanged() {
	this->enableRecalculate();
}
