/***************************************************************************
    File             : XYSmoothCurveDock.cpp
    Project          : LabPlot
    --------------------------------------------------------------------
    Copyright        : (C) 2016 Stefan Gerlach (stefan.gerlach@uni.kn)
    Description      : widget for editing properties of smooth curves

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

#include "XYSmoothCurveDock.h"
#include "backend/core/AspectTreeModel.h"
#include "backend/core/Project.h"
#include "backend/worksheet/plots/cartesian/XYSmoothCurve.h"
#include "commonfrontend/widgets/TreeViewComboBox.h"

#include <QMenu>
#include <QWidgetAction>
#include <QStandardItemModel>
#include <QDebug>

#include <cmath>        // isnan

/*!
  \class XYSmoothCurveDock
 \brief  Provides a widget for editing the properties of the XYSmoothCurves
		(2D-curves defined by an smooth) currently selected in
		the project explorer.

  If more then one curves are set, the properties of the first column are shown.
  The changes of the properties are applied to all curves.
  The exclusions are the name, the comment and the datasets (columns) of
  the curves  - these properties can only be changed if there is only one single curve.

  \ingroup kdefrontend
*/

XYSmoothCurveDock::XYSmoothCurveDock(QWidget *parent): 
	XYCurveDock(parent), cbXDataColumn(0), cbYDataColumn(0), m_smoothCurve(0) {

	//hide the line connection type
	ui.cbLineType->setDisabled(true);

	//remove the tab "Error bars"
	ui.tabWidget->removeTab(5);
}

/*!
 * 	// Tab "General"
 */
void XYSmoothCurveDock::setupGeneral() {
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

	uiGeneralTab.cbType->addItem(i18n("moving average (central)"));
	uiGeneralTab.cbType->addItem(i18n("moving average (lagged)"));
	uiGeneralTab.cbType->addItem(i18n("percentile"));
	uiGeneralTab.cbType->addItem(i18n("Savitzky-Golay"));
//	uiGeneralTab.cbType->addItem(i18n("LOWESS/LOESS"));
//	etc.

	uiGeneralTab.cbWeight->addItem(i18n("uniform (rectangular)"));
	uiGeneralTab.cbWeight->addItem(i18n("triangular"));
	uiGeneralTab.cbWeight->addItem(i18n("binomial"));
	uiGeneralTab.cbWeight->addItem(i18n("parabolic (Epanechnikov)"));
	uiGeneralTab.cbWeight->addItem(i18n("quartic (biweight)"));
	uiGeneralTab.cbWeight->addItem(i18n("triweight"));
	uiGeneralTab.cbWeight->addItem(i18n("tricube"));
	uiGeneralTab.cbWeight->addItem(i18n("cosine"));
// TODO: IIR	-> np="all"?
//	uiGeneralTab.cbWeight->addItem(i18n("exponential"));
//	uiGeneralTab.cbWeight->addItem(i18n("Gaussian"));
//	etc. -> see nsl_sf_kernel

	uiGeneralTab.cbMode->addItem(i18n("interpolating"));
	uiGeneralTab.cbMode->addItem(i18n("mirror"));
	uiGeneralTab.cbMode->addItem(i18n("nearest"));
	uiGeneralTab.cbMode->addItem(i18n("constant"));
	uiGeneralTab.cbMode->addItem(i18n("wrap"));

	uiGeneralTab.pbRecalculate->setIcon(KIcon("run-build"));

	QHBoxLayout* layout = new QHBoxLayout(ui.tabGeneral);
	layout->setMargin(0);
	layout->addWidget(generalTab);

	//Slots
	connect( uiGeneralTab.leName, SIGNAL(returnPressed()), this, SLOT(nameChanged()) );
	connect( uiGeneralTab.leComment, SIGNAL(returnPressed()), this, SLOT(commentChanged()) );
	connect( uiGeneralTab.chkVisible, SIGNAL(clicked(bool)), this, SLOT(visibilityChanged(bool)) );

	connect( uiGeneralTab.cbType, SIGNAL(currentIndexChanged(int)), this, SLOT(typeChanged()) );
	connect( uiGeneralTab.sbPoints, SIGNAL(valueChanged(int)), this, SLOT(pointsChanged()) );
	connect( uiGeneralTab.cbWeight, SIGNAL(currentIndexChanged(int)), this, SLOT(weightChanged()) );
	connect( uiGeneralTab.sbPercentile, SIGNAL(valueChanged(double)), this, SLOT(percentileChanged()) );
	connect( uiGeneralTab.sbOrder, SIGNAL(valueChanged(int)), this, SLOT(orderChanged()) );
	connect( uiGeneralTab.cbMode, SIGNAL(currentIndexChanged(int)), this, SLOT(modeChanged()) );
	connect( uiGeneralTab.sbLeftValue, SIGNAL(valueChanged(double)), this, SLOT(valueChanged()) );
	connect( uiGeneralTab.sbRightValue, SIGNAL(valueChanged(double)), this, SLOT(valueChanged()) );

	connect( uiGeneralTab.pbRecalculate, SIGNAL(clicked()), this, SLOT(recalculateClicked()) );
}

void XYSmoothCurveDock::initGeneralTab() {
	//if there are more then one curve in the list, disable the tab "general"
	if (m_curvesList.size()==1){
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
	m_smoothCurve = dynamic_cast<XYSmoothCurve*>(m_curve);
	Q_ASSERT(m_smoothCurve);
	XYCurveDock::setModelIndexFromColumn(cbXDataColumn, m_smoothCurve->xDataColumn());
	XYCurveDock::setModelIndexFromColumn(cbYDataColumn, m_smoothCurve->yDataColumn());
	// update list of selectable types
	xDataColumnChanged(cbXDataColumn->currentModelIndex());

	uiGeneralTab.cbType->setCurrentIndex(m_smoothData.type);
	typeChanged();	// needed, when type does not change
	uiGeneralTab.sbPoints->setValue(m_smoothData.points);
	uiGeneralTab.cbWeight->setCurrentIndex(m_smoothData.weight);
	uiGeneralTab.sbPercentile->setValue(m_smoothData.percentile);
	uiGeneralTab.sbOrder->setValue(m_smoothData.order);
	uiGeneralTab.cbMode->setCurrentIndex(m_smoothData.mode-1);
	modeChanged();	// needed, when mode does not change
	uiGeneralTab.sbLeftValue->setValue(m_smoothData.lvalue);
	uiGeneralTab.sbRightValue->setValue(m_smoothData.rvalue);
	valueChanged();
	this->showSmoothResult();

	//enable the "recalculate"-button if the source data was changed since the last smooth
	uiGeneralTab.pbRecalculate->setEnabled(m_smoothCurve->isSourceDataChangedSinceLastSmooth());

	uiGeneralTab.chkVisible->setChecked( m_curve->isVisible() );

	//Slots
	connect(m_smoothCurve, SIGNAL(aspectDescriptionChanged(const AbstractAspect*)), this, SLOT(curveDescriptionChanged(const AbstractAspect*)));
	connect(m_smoothCurve, SIGNAL(xDataColumnChanged(const AbstractColumn*)), this, SLOT(curveXDataColumnChanged(const AbstractColumn*)));
	connect(m_smoothCurve, SIGNAL(yDataColumnChanged(const AbstractColumn*)), this, SLOT(curveYDataColumnChanged(const AbstractColumn*)));
	connect(m_smoothCurve, SIGNAL(smoothDataChanged(XYSmoothCurve::SmoothData)), this, SLOT(curveSmoothDataChanged(XYSmoothCurve::SmoothData)));
	connect(m_smoothCurve, SIGNAL(sourceDataChangedSinceLastSmooth()), this, SLOT(enableRecalculate()));
}

void XYSmoothCurveDock::setModel() {
	QList<const char*>  list;
	list<<"Folder"<<"Workbook"<<"Spreadsheet"<<"FileDataSource"<<"Column"<<"Datapicker";
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
void XYSmoothCurveDock::setCurves(QList<XYCurve*> list) {
	m_initializing=true;
	m_curvesList=list;
	m_curve=list.first();
	m_smoothCurve = dynamic_cast<XYSmoothCurve*>(m_curve);
	Q_ASSERT(m_smoothCurve);
	m_aspectTreeModel = new AspectTreeModel(m_curve->project());
	this->setModel();
	m_smoothData = m_smoothCurve->smoothData();
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
void XYSmoothCurveDock::nameChanged() {
	if (m_initializing)
		return;

	m_curve->setName(uiGeneralTab.leName->text());
}

void XYSmoothCurveDock::commentChanged() {
	if (m_initializing)
		return;

	m_curve->setComment(uiGeneralTab.leComment->text());
}

void XYSmoothCurveDock::xDataColumnChanged(const QModelIndex& index) {
	AbstractAspect* aspect = static_cast<AbstractAspect*>(index.internalPointer());
	AbstractColumn* column = 0;
	if (aspect) {
		column = dynamic_cast<AbstractColumn*>(aspect);
		Q_ASSERT(column);
	}

	foreach(XYCurve* curve, m_curvesList)
		dynamic_cast<XYSmoothCurve*>(curve)->setXDataColumn(column);

	// disable types that need more data points
	if(column != 0) {
		unsigned int n=0;
		for(int row=0;row < column->rowCount();row++)
			if (!std::isnan(column->valueAt(row)) && !column->isMasked(row)) 
				n++;

		// set maximum of sbPoints to number of columns
		uiGeneralTab.sbPoints->setMaximum(n);
	}

}

void XYSmoothCurveDock::yDataColumnChanged(const QModelIndex& index) {
	if (m_initializing)
		return;

	AbstractAspect* aspect = static_cast<AbstractAspect*>(index.internalPointer());
	AbstractColumn* column = 0;
	if (aspect) {
		column = dynamic_cast<AbstractColumn*>(aspect);
		Q_ASSERT(column);
	}

	foreach(XYCurve* curve, m_curvesList)
		dynamic_cast<XYSmoothCurve*>(curve)->setYDataColumn(column);
}

void XYSmoothCurveDock::typeChanged() {
	XYSmoothCurve::SmoothType type = (XYSmoothCurve::SmoothType)uiGeneralTab.cbType->currentIndex();
	m_smoothData.type = type;

	if(type == XYSmoothCurve::MovingAverage || type == XYSmoothCurve::MovingAverageLagged) {
		uiGeneralTab.lWeight->show();
		uiGeneralTab.cbWeight->show();
	} else {
		uiGeneralTab.lWeight->hide();
		uiGeneralTab.cbWeight->hide();
	}
	if(type == XYSmoothCurve::MovingAverageLagged) {
		uiGeneralTab.sbPoints->setSingleStep(1);
		uiGeneralTab.sbPoints->setMinimum(2);
	} else {
		uiGeneralTab.sbPoints->setSingleStep(2);
		uiGeneralTab.sbPoints->setMinimum(3);
	}
	if(type == XYSmoothCurve::Percentile) {
		uiGeneralTab.lPercentile->show();
		uiGeneralTab.sbPercentile->show();
	} else {
		uiGeneralTab.lPercentile->hide();
		uiGeneralTab.sbPercentile->hide();
	}
	if(type == XYSmoothCurve::SavitzkyGolay) {
		uiGeneralTab.sbPoints->setValue(5);
		uiGeneralTab.lOrder->show();
		uiGeneralTab.sbOrder->show();
		uiGeneralTab.lMode->show();
		uiGeneralTab.cbMode->show();
	} else {
		uiGeneralTab.lOrder->hide();
		uiGeneralTab.sbOrder->hide();
		uiGeneralTab.lMode->hide();
		uiGeneralTab.cbMode->hide();
	}

	uiGeneralTab.pbRecalculate->setEnabled(true);
}

void XYSmoothCurveDock::pointsChanged() {
	m_smoothData.points = uiGeneralTab.sbPoints->value();

	// set maximum order
	uiGeneralTab.sbOrder->setMaximum(m_smoothData.points-1);

	uiGeneralTab.pbRecalculate->setEnabled(true);
}

void XYSmoothCurveDock::weightChanged() {
	m_smoothData.weight = (XYSmoothCurve::WeightType)uiGeneralTab.cbWeight->currentIndex();

	uiGeneralTab.pbRecalculate->setEnabled(true);
}

void XYSmoothCurveDock::percentileChanged() {
	m_smoothData.percentile = uiGeneralTab.sbPercentile->value();

	uiGeneralTab.pbRecalculate->setEnabled(true);
}

void XYSmoothCurveDock::orderChanged() {
	m_smoothData.order = uiGeneralTab.sbOrder->value();

	uiGeneralTab.pbRecalculate->setEnabled(true);
}

void XYSmoothCurveDock::modeChanged() {
	m_smoothData.mode = (nsl_smooth_savgol_mode)(uiGeneralTab.cbMode->currentIndex()+1);

	if(m_smoothData.mode == nsl_smooth_savgol_constant) {
		uiGeneralTab.lLeftValue->show();
		uiGeneralTab.sbLeftValue->show();
		uiGeneralTab.lRightValue->show();
		uiGeneralTab.sbRightValue->show();
	} else {
		uiGeneralTab.lLeftValue->hide();
		uiGeneralTab.sbLeftValue->hide();
		uiGeneralTab.lRightValue->hide();
		uiGeneralTab.sbRightValue->hide();
	}

	uiGeneralTab.pbRecalculate->setEnabled(true);
}

void XYSmoothCurveDock::valueChanged() {
	m_smoothData.lvalue = uiGeneralTab.sbLeftValue->value();
	m_smoothData.rvalue = uiGeneralTab.sbRightValue->value();

	uiGeneralTab.pbRecalculate->setEnabled(true);
}

void XYSmoothCurveDock::recalculateClicked() {
	QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

	foreach(XYCurve* curve, m_curvesList)
		dynamic_cast<XYSmoothCurve*>(curve)->setSmoothData(m_smoothData);

	uiGeneralTab.pbRecalculate->setEnabled(false);
	QApplication::restoreOverrideCursor();
}

void XYSmoothCurveDock::enableRecalculate() const {
	if (m_initializing)
		return;

	//no smoothing possible without the x- and y-data
	AbstractAspect* aspectX = static_cast<AbstractAspect*>(cbXDataColumn->currentModelIndex().internalPointer());
	AbstractAspect* aspectY = static_cast<AbstractAspect*>(cbYDataColumn->currentModelIndex().internalPointer());
	bool data = (aspectX!=0 && aspectY!=0);

	uiGeneralTab.pbRecalculate->setEnabled(data);
}

/*!
 * show the result and details of the smooth
 */
void XYSmoothCurveDock::showSmoothResult() {
	const XYSmoothCurve::SmoothResult& smoothResult = m_smoothCurve->smoothResult();
	if (!smoothResult.available) {
		uiGeneralTab.teResult->clear();
		return;
	}

	//const XYSmoothCurve::SmoothData& smoothData = m_smoothCurve->smoothData();
	QString str = i18n("status:") + " " + smoothResult.status + "<br>";

	if (!smoothResult.valid) {
		uiGeneralTab.teResult->setText(str);
		return; //result is not valid, there was an error which is shown in the status-string, nothing to show more.
	}

	if (smoothResult.elapsedTime>1000)
		str += i18n("calculation time: %1 s").arg(QString::number(smoothResult.elapsedTime/1000)) + "<br>";
	else
		str += i18n("calculation time: %1 ms").arg(QString::number(smoothResult.elapsedTime)) + "<br>";

 	str += "<br><br>";

	uiGeneralTab.teResult->setText(str);
}

//*************************************************************
//*********** SLOTs for changes triggered in XYCurve **********
//*************************************************************
//General-Tab
void XYSmoothCurveDock::curveDescriptionChanged(const AbstractAspect* aspect) {
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

void XYSmoothCurveDock::curveXDataColumnChanged(const AbstractColumn* column) {
	m_initializing = true;
	XYCurveDock::setModelIndexFromColumn(cbXDataColumn, column);
	m_initializing = false;
}

void XYSmoothCurveDock::curveYDataColumnChanged(const AbstractColumn* column) {
	m_initializing = true;
	XYCurveDock::setModelIndexFromColumn(cbYDataColumn, column);
	m_initializing = false;
}

void XYSmoothCurveDock::curveSmoothDataChanged(const XYSmoothCurve::SmoothData& data) {
	m_initializing = true;
	m_smoothData = data;
	uiGeneralTab.cbType->setCurrentIndex(m_smoothData.type);

	this->showSmoothResult();
	m_initializing = false;
}

void XYSmoothCurveDock::dataChanged() {
	this->enableRecalculate();
}
