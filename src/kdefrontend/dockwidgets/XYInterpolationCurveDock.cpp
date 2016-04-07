/***************************************************************************
    File             : XYInterpolationCurveDock.h
    Project          : LabPlot
    --------------------------------------------------------------------
    Copyright        : (C) 2016 Stefan Gerlach (stefan.gerlach@uni.kn)
    Description      : widget for editing properties of interpolation curves

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

#include "XYInterpolationCurveDock.h"
#include "backend/core/AspectTreeModel.h"
#include "backend/core/Project.h"
#include "backend/worksheet/plots/cartesian/XYInterpolationCurve.h"
#include "commonfrontend/widgets/TreeViewComboBox.h"
#include <QMenu>
#include <QWidgetAction>
#include <QDebug>

/*!
  \class XYInterpolationCurveDock
 \brief  Provides a widget for editing the properties of the XYInterpolationCurves
		(2D-curves defined by an interpolation) currently selected in
		the project explorer.

  If more then one curves are set, the properties of the first column are shown.
  The changes of the properties are applied to all curves.
  The exclusions are the name, the comment and the datasets (columns) of
  the curves  - these properties can only be changed if there is only one single curve.

  \ingroup kdefrontend
*/

XYInterpolationCurveDock::XYInterpolationCurveDock(QWidget *parent): 
	XYCurveDock(parent), cbXDataColumn(0), cbYDataColumn(0), m_interpolationCurve(0) {
}

/*!
 * 	// Tab "General"
 */
void XYInterpolationCurveDock::setupGeneral() {
	QWidget* generalTab = new QWidget(ui.tabGeneral);
	uiGeneralTab.setupUi(generalTab);

	QGridLayout* gridLayout = dynamic_cast<QGridLayout*>(generalTab->layout());
	if (gridLayout) {
		gridLayout->setContentsMargins(2,2,2,2);
		gridLayout->setHorizontalSpacing(2);
		gridLayout->setVerticalSpacing(2);
	}

	cbXDataColumn = new TreeViewComboBox(generalTab);
	gridLayout->addWidget(cbXDataColumn, 4, 1, 1, 2);
	cbYDataColumn = new TreeViewComboBox(generalTab);
	gridLayout->addWidget(cbYDataColumn, 5, 1, 1, 2);

	uiGeneralTab.cbType->addItem(i18n("Low pass"));
	uiGeneralTab.cbType->addItem(i18n("High pass"));
	uiGeneralTab.cbType->addItem(i18n("Band pass"));
	uiGeneralTab.cbType->addItem(i18n("Band reject"));
	uiGeneralTab.cbType->addItem(i18n("Threshold"));

	uiGeneralTab.cbForm->addItem(i18n("Ideal"));
	uiGeneralTab.cbForm->addItem(i18n("Butterworth"));
	uiGeneralTab.cbForm->addItem(i18n("Chebyshev type I"));
	uiGeneralTab.cbForm->addItem(i18n("Chebyshev type II"));

	uiGeneralTab.cbUnit->addItem(i18n("Frequency (Hz)"));
	uiGeneralTab.cbUnit->addItem(i18n("Fraction"));
	uiGeneralTab.cbUnit->addItem(i18n("Index"));
	uiGeneralTab.cbUnit2->addItem(i18n("Frequency (Hz)"));
	uiGeneralTab.cbUnit2->addItem(i18n("Fraction"));
	uiGeneralTab.cbUnit2->addItem(i18n("Index"));
	uiGeneralTab.pbRecalculate->setIcon(KIcon("run-build"));

	QHBoxLayout* layout = new QHBoxLayout(ui.tabGeneral);
	layout->setMargin(0);
	layout->addWidget(generalTab);

	//Slots
	connect( uiGeneralTab.leName, SIGNAL(returnPressed()), this, SLOT(nameChanged()) );
	connect( uiGeneralTab.leComment, SIGNAL(returnPressed()), this, SLOT(commentChanged()) );
	connect( uiGeneralTab.chkVisible, SIGNAL(clicked(bool)), this, SLOT(visibilityChanged(bool)) );

	connect( uiGeneralTab.cbType, SIGNAL(currentIndexChanged(int)), this, SLOT(typeChanged(int)) );
	connect( uiGeneralTab.cbForm, SIGNAL(currentIndexChanged(int)), this, SLOT(formChanged(int)) );
	connect( uiGeneralTab.sbOrder, SIGNAL(valueChanged(int)), this, SLOT(orderChanged(int)) );
	connect( uiGeneralTab.sbCutoff, SIGNAL(valueChanged(double)), this, SLOT(enableRecalculate()) );
	connect( uiGeneralTab.sbCutoff2, SIGNAL(valueChanged(double)), this, SLOT(enableRecalculate()) );
	connect( uiGeneralTab.cbUnit, SIGNAL(currentIndexChanged(int)), this, SLOT(unitChanged(int)) );
	connect( uiGeneralTab.cbUnit2, SIGNAL(currentIndexChanged(int)), this, SLOT(unit2Changed(int)) );

//	connect( uiGeneralTab.pbOptions, SIGNAL(clicked()), this, SLOT(showOptions()) );
	connect( uiGeneralTab.pbRecalculate, SIGNAL(clicked()), this, SLOT(recalculateClicked()) );
}

void XYInterpolationCurveDock::initGeneralTab() {
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
	m_interpolationCurve = dynamic_cast<XYInterpolationCurve*>(m_curve);
	Q_ASSERT(m_interpolationCurve);
	XYCurveDock::setModelIndexFromColumn(cbXDataColumn, m_interpolationCurve->xDataColumn());
	XYCurveDock::setModelIndexFromColumn(cbYDataColumn, m_interpolationCurve->yDataColumn());

	uiGeneralTab.cbType->setCurrentIndex(m_interpolationData.type);
	this->typeChanged(m_interpolationData.type);
	//uiGeneralTab.cbForm->setCurrentIndex(m_interpolationData.form);
	//this->formChanged(m_interpolationData.form);
	//uiGeneralTab.sbOrder->setValue(m_interpolationData.order);
	//uiGeneralTab.cbUnit->setCurrentIndex(m_interpolationData.unit);
	//this->unitChanged(m_interpolationData.unit);
	// after unit has set
	//uiGeneralTab.sbCutoff->setValue(m_interpolationData.cutoff);
	//uiGeneralTab.cbUnit2->setCurrentIndex(m_interpolationData.unit2);
	//this->unit2Changed(m_interpolationData.unit2);
	// after unit has set
	//uiGeneralTab.sbCutoff2->setValue(m_interpolationData.cutoff2);
	this->showInterpolationResult();

	//enable the "recalculate"-button if the source data was changed since the last interpolation
	uiGeneralTab.pbRecalculate->setEnabled(m_interpolationCurve->isSourceDataChangedSinceLastInterpolation());

	uiGeneralTab.chkVisible->setChecked( m_curve->isVisible() );

	//Slots
	connect(m_interpolationCurve, SIGNAL(aspectDescriptionChanged(const AbstractAspect*)), this, SLOT(curveDescriptionChanged(const AbstractAspect*)));
	connect(m_interpolationCurve, SIGNAL(xDataColumnChanged(const AbstractColumn*)), this, SLOT(curveXDataColumnChanged(const AbstractColumn*)));
	connect(m_interpolationCurve, SIGNAL(yDataColumnChanged(const AbstractColumn*)), this, SLOT(curveYDataColumnChanged(const AbstractColumn*)));
	connect(m_interpolationCurve, SIGNAL(interpolationDataChanged(XYInterpolationCurve::InterpolationData)), this, SLOT(curveInterpolationDataChanged(XYInterpolationCurve::InterpolationData)));
	connect(m_interpolationCurve, SIGNAL(sourceDataChangedSinceLastInterpolation()), this, SLOT(enableRecalculate()));
}

void XYInterpolationCurveDock::setModel() {
	QList<const char*>  list;
	list<<"Folder"<<"Workbook"<<"Spreadsheet"<<"FileDataSource"<<"Column"<<"Datapicker";
	cbXDataColumn->setTopLevelClasses(list);
	cbYDataColumn->setTopLevelClasses(list);

 	list.clear();
	list<<"Column";
	cbXDataColumn->setSelectableClasses(list);
	cbYDataColumn->setSelectableClasses(list);

	cbXDataColumn->setModel(m_aspectTreeModel);
	cbYDataColumn->setModel(m_aspectTreeModel);

	connect( cbXDataColumn, SIGNAL(currentModelIndexChanged(QModelIndex)), this, SLOT(xDataColumnChanged(QModelIndex)) );
	connect( cbYDataColumn, SIGNAL(currentModelIndexChanged(QModelIndex)), this, SLOT(yDataColumnChanged(QModelIndex)) );
	XYCurveDock::setModel();
}

/*!
  sets the curves. The properties of the curves in the list \c list can be edited in this widget.
*/
void XYInterpolationCurveDock::setCurves(QList<XYCurve*> list) {
	m_initializing=true;
	m_curvesList=list;
	m_curve=list.first();
	m_interpolationCurve = dynamic_cast<XYInterpolationCurve*>(m_curve);
	Q_ASSERT(m_interpolationCurve);
	m_aspectTreeModel = new AspectTreeModel(m_curve->project());
	this->setModel();
	m_interpolationData = m_interpolationCurve->interpolationData();
	initGeneralTab();
	initTabs();
	m_initializing=false;
}

//*************************************************************
//**** SLOTs for changes triggered in XYFitCurveDock *****
//*************************************************************
void XYInterpolationCurveDock::nameChanged(){
	if (m_initializing)
		return;

	m_curve->setName(uiGeneralTab.leName->text());
}

void XYInterpolationCurveDock::commentChanged(){
	if (m_initializing)
		return;

	m_curve->setComment(uiGeneralTab.leComment->text());
}

void XYInterpolationCurveDock::xDataColumnChanged(const QModelIndex& index) {
	if (m_initializing)
		return;

	AbstractAspect* aspect = static_cast<AbstractAspect*>(index.internalPointer());
	AbstractColumn* column = 0;
	if (aspect) {
		column = dynamic_cast<AbstractColumn*>(aspect);
		Q_ASSERT(column);
	}

	foreach(XYCurve* curve, m_curvesList)
		dynamic_cast<XYInterpolationCurve*>(curve)->setXDataColumn(column);

	// update range of cutoff spin boxes (like a unit change)
//	unitChanged(uiGeneralTab.cbUnit->currentIndex());
//	unit2Changed(uiGeneralTab.cbUnit2->currentIndex());
}

void XYInterpolationCurveDock::yDataColumnChanged(const QModelIndex& index) {
	if (m_initializing)
		return;

	AbstractAspect* aspect = static_cast<AbstractAspect*>(index.internalPointer());
	AbstractColumn* column = 0;
	if (aspect) {
		column = dynamic_cast<AbstractColumn*>(aspect);
		Q_ASSERT(column);
	}

	foreach(XYCurve* curve, m_curvesList)
		dynamic_cast<XYInterpolationCurve*>(curve)->setYDataColumn(column);
}

void XYInterpolationCurveDock::typeChanged(int index) {
	XYInterpolationCurve::InterpolationType type = (XYInterpolationCurve::InterpolationType)index;
	m_interpolationData.type = (XYInterpolationCurve::InterpolationType)uiGeneralTab.cbType->currentIndex();

	//TODO

	uiGeneralTab.pbRecalculate->setEnabled(true);
}

/*void XYInterpolationCurveDock::formChanged(int index) {
	XYInterpolationCurve::InterpolationForm form = (XYInterpolationCurve::InterpolationForm)index;
	m_interpolationData.form = (XYInterpolationCurve::InterpolationForm)uiGeneralTab.cbForm->currentIndex();

	switch(form) {
	case XYInterpolationCurve::Ideal:
		uiGeneralTab.sbOrder->setVisible(false);
		uiGeneralTab.lOrder->setVisible(false);
		break;
	case XYInterpolationCurve::Butterworth:
	case XYInterpolationCurve::ChebyshevI:
	case XYInterpolationCurve::ChebyshevII:
		uiGeneralTab.sbOrder->setVisible(true);
		uiGeneralTab.lOrder->setVisible(true);
		break;
	}

	uiGeneralTab.pbRecalculate->setEnabled(true);
}

void XYInterpolationCurveDock::orderChanged(int index) {
	Q_UNUSED(index)
	m_interpolationData.order = uiGeneralTab.sbOrder->value();

	uiGeneralTab.pbRecalculate->setEnabled(true);
}

void XYInterpolationCurveDock::unitChanged(int index) {
	XYInterpolationCurve::CutoffUnit unit = (XYInterpolationCurve::CutoffUnit)index;
	m_interpolationData.unit = (XYInterpolationCurve::CutoffUnit)uiGeneralTab.cbUnit->currentIndex();

	int n=100;
	double T=1.0;
	if(m_interpolationCurve->xDataColumn() != NULL) {
		n = m_interpolationCurve->xDataColumn()->rowCount();
		T = m_interpolationCurve->xDataColumn()->maximum() - m_interpolationCurve->xDataColumn()->minimum();
	}
	switch(unit) {
	case XYInterpolationCurve::Frequency:
		uiGeneralTab.sbCutoff->setDecimals(6);
		uiGeneralTab.sbCutoff->setMaximum(1.0/T);
		uiGeneralTab.sbCutoff->setSingleStep(0.01/T);
		break;
	case XYInterpolationCurve::Fraction:
		uiGeneralTab.sbCutoff->setDecimals(6);
		uiGeneralTab.sbCutoff->setMaximum(1.0);
		uiGeneralTab.sbCutoff->setSingleStep(0.01);
		break;
	case XYInterpolationCurve::Index:
		uiGeneralTab.sbCutoff->setDecimals(0);
		uiGeneralTab.sbCutoff->setSingleStep(1);
		uiGeneralTab.sbCutoff->setMaximum(n);
		break;
	}

	uiGeneralTab.pbRecalculate->setEnabled(true);
}

void XYInterpolationCurveDock::unit2Changed(int index) {
	XYInterpolationCurve::CutoffUnit unit2 = (XYInterpolationCurve::CutoffUnit)index;
	m_interpolationData.unit2 = (XYInterpolationCurve::CutoffUnit)uiGeneralTab.cbUnit2->currentIndex();

	int n=100;
	double T=1.0;
	if(m_interpolationCurve->xDataColumn() != NULL) {
		n = m_interpolationCurve->xDataColumn()->rowCount();
		T = m_interpolationCurve->xDataColumn()->maximum() - m_interpolationCurve->xDataColumn()->minimum();
	}
	switch(unit2) {
	case XYInterpolationCurve::Frequency:
		uiGeneralTab.sbCutoff2->setDecimals(6);
		uiGeneralTab.sbCutoff2->setMaximum(1.0/T);
		uiGeneralTab.sbCutoff2->setSingleStep(0.01/T);
		break;
	case XYInterpolationCurve::Fraction:
		uiGeneralTab.sbCutoff2->setDecimals(6);
		uiGeneralTab.sbCutoff2->setMaximum(1.0);
		uiGeneralTab.sbCutoff2->setSingleStep(0.01);
		break;
	case XYInterpolationCurve::Index:
		uiGeneralTab.sbCutoff2->setDecimals(0);
		uiGeneralTab.sbCutoff2->setSingleStep(1);
		uiGeneralTab.sbCutoff2->setMaximum(n);
		break;
	}

	uiGeneralTab.pbRecalculate->setEnabled(true);
}
*/
void XYInterpolationCurveDock::recalculateClicked() {
	QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
	//m_interpolationData.cutoff = uiGeneralTab.sbCutoff->value();

	foreach(XYCurve* curve, m_curvesList)
		dynamic_cast<XYInterpolationCurve*>(curve)->setInterpolationData(m_interpolationData);

	uiGeneralTab.pbRecalculate->setEnabled(false);
	QApplication::restoreOverrideCursor();
}

void XYInterpolationCurveDock::enableRecalculate() const {
	if (m_initializing)
		return;

	//no interpolationing possible without the x- and y-data
	AbstractAspect* aspectX = static_cast<AbstractAspect*>(cbXDataColumn->currentModelIndex().internalPointer());
	AbstractAspect* aspectY = static_cast<AbstractAspect*>(cbYDataColumn->currentModelIndex().internalPointer());
	bool data = (aspectX!=0 && aspectY!=0);

	uiGeneralTab.pbRecalculate->setEnabled(data);
}

/*!
 * show the result and details of the interpolation
 */
void XYInterpolationCurveDock::showInterpolationResult() {
	const XYInterpolationCurve::InterpolationResult& interpolationResult = m_interpolationCurve->interpolationResult();
	if (!interpolationResult.available) {
		uiGeneralTab.teResult->clear();
		return;
	}

	//const XYInterpolationCurve::InterpolationData& interpolationData = m_interpolationCurve->interpolationData();
	QString str = i18n("status") + ": " + interpolationResult.status + "<br>";

	if (!interpolationResult.valid) {
		uiGeneralTab.teResult->setText(str);
		return; //result is not valid, there was an error which is shown in the status-string, nothing to show more.
	}

	if (interpolationResult.elapsedTime>1000)
		str += i18n("calculation time: %1 s").arg(QString::number(interpolationResult.elapsedTime/1000)) + "<br>";
	else
		str += i18n("calculation time: %1 ms").arg(QString::number(interpolationResult.elapsedTime)) + "<br>";

 	str += "<br><br>";

	uiGeneralTab.teResult->setText(str);
}

//*************************************************************
//*********** SLOTs for changes triggered in XYCurve **********
//*************************************************************
//General-Tab
void XYInterpolationCurveDock::curveDescriptionChanged(const AbstractAspect* aspect) {
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

void XYInterpolationCurveDock::curveXDataColumnChanged(const AbstractColumn* column) {
	m_initializing = true;
	XYCurveDock::setModelIndexFromColumn(cbXDataColumn, column);
	m_initializing = false;
}

void XYInterpolationCurveDock::curveYDataColumnChanged(const AbstractColumn* column) {
	m_initializing = true;
	XYCurveDock::setModelIndexFromColumn(cbYDataColumn, column);
	m_initializing = false;
}

void XYInterpolationCurveDock::curveInterpolationDataChanged(const XYInterpolationCurve::InterpolationData& data) {
	m_initializing = true;
	m_interpolationData = data;
	uiGeneralTab.cbType->setCurrentIndex(m_interpolationData.type);
	this->typeChanged(m_interpolationData.type);

	this->showInterpolationResult();
	m_initializing = false;
}

void XYInterpolationCurveDock::dataChanged() {
	this->enableRecalculate();
}
