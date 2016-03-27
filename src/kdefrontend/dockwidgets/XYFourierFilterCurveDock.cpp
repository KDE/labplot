/***************************************************************************
    File             : XYFourierFilterCurveDock.h
    Project          : LabPlot
    --------------------------------------------------------------------
    Copyright        : (C) 2016 Stefan Gerlach (stefan.gerlach@uni.kn)
    Description      : widget for editing properties of Fourier filter curves

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

#include "XYFourierFilterCurveDock.h"
#include "backend/core/AspectTreeModel.h"
#include "backend/core/Project.h"
#include "backend/worksheet/plots/cartesian/XYFourierFilterCurve.h"
#include "commonfrontend/widgets/TreeViewComboBox.h"
#include <QMenu>
#include <QWidgetAction>
#include <QDebug>

/*!
  \class XYFourierFilterCurveDock
 \brief  Provides a widget for editing the properties of the XYFourierFilterCurves
		(2D-curves defined by a Fourier filter) currently selected in
		the project explorer.

  If more then one curves are set, the properties of the first column are shown.
  The changes of the properties are applied to all curves.
  The exclusions are the name, the comment and the datasets (columns) of
  the curves  - these properties can only be changed if there is only one single curve.

  \ingroup kdefrontend
*/

XYFourierFilterCurveDock::XYFourierFilterCurveDock(QWidget *parent): 
	XYCurveDock(parent), cbXDataColumn(0), cbYDataColumn(0), m_filterCurve(0) {
}

/*!
 * 	// Tab "General"
 */
void XYFourierFilterCurveDock::setupGeneral() {
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

	uiGeneralTab.cbUnit->addItem(i18n("Index"));
	uiGeneralTab.cbUnit->addItem(i18n("Frequency (Hz)"));
	uiGeneralTab.cbUnit->addItem(i18n("Wavelength"));
	uiGeneralTab.cbUnit->addItem(i18n("Ratio"));
	uiGeneralTab.pbRecalculate->setIcon(KIcon("run-build"));

	QHBoxLayout* layout = new QHBoxLayout(ui.tabGeneral);
	layout->setMargin(0);
	layout->addWidget(generalTab);

	//Slots
	connect( uiGeneralTab.leName, SIGNAL(returnPressed()), this, SLOT(nameChanged()) );
	connect( uiGeneralTab.leComment, SIGNAL(returnPressed()), this, SLOT(commentChanged()) );
	connect( uiGeneralTab.chkVisible, SIGNAL(clicked(bool)), this, SLOT(visibilityChanged(bool)) );

	connect( uiGeneralTab.cbType, SIGNAL(currentIndexChanged(int)), this, SLOT(typeChanged(int)) );
//	connect( uiGeneralTab.cbForm, SIGNAL(currentIndexChanged(int)), this, SLOT(formChanged(int)) );
//	connect( uiGeneralTab.cbUnit, SIGNAL(currentIndexChanged(int)), this, SLOT(unitChanged(int)) );
//	connect( uiGeneralTab.cbUnit2, SIGNAL(currentIndexChanged(int)), this, SLOT(unit2Changed(int)) );

//	connect( uiGeneralTab.pbOptions, SIGNAL(clicked()), this, SLOT(showOptions()) );
	connect( uiGeneralTab.pbRecalculate, SIGNAL(clicked()), this, SLOT(recalculateClicked()) );
}

void XYFourierFilterCurveDock::initGeneralTab() {
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
	m_filterCurve = dynamic_cast<XYFourierFilterCurve*>(m_curve);
	Q_ASSERT(m_filterCurve);
	qDebug()<<"	filter curve	x index="<<m_filterCurve->xDataColumn();
	qDebug()<<"	init():	"<<m_filterCurve->xDataColumnPath();
	XYCurveDock::setModelIndexFromColumn(cbXDataColumn, m_filterCurve->xDataColumn());
	XYCurveDock::setModelIndexFromColumn(cbYDataColumn, m_filterCurve->yDataColumn());

	uiGeneralTab.cbType->setCurrentIndex(m_filterData.type);
	this->typeChanged(m_filterData.type);
	uiGeneralTab.cbForm->setCurrentIndex(m_filterData.form);
	this->formChanged(m_filterData.form);
	uiGeneralTab.sbValue->setValue(m_filterData.value);
	uiGeneralTab.cbUnit->setCurrentIndex(m_filterData.unit);
	this->unitChanged(m_filterData.unit);
	//uiGeneralTab.sbValue2->setValue(m_filterData.value2);
	//uiGeneralTab.cbUnit2->setCurrentIndex(m_filterData.unit2);
	//this->unit2Changed(m_filterData.unit2);
	this->showFilterResult();

	//enable the "recalculate"-button if the source data was changed since the last filter
	//uiGeneralTab.pbRecalculate->setEnabled(m_filterCurve->isSourceDataChangedSinceLastFilter());

	uiGeneralTab.chkVisible->setChecked( m_curve->isVisible() );

	//Slots
	connect(m_filterCurve, SIGNAL(aspectDescriptionChanged(const AbstractAspect*)), this, SLOT(curveDescriptionChanged(const AbstractAspect*)));
	connect(m_filterCurve, SIGNAL(xDataColumnChanged(const AbstractColumn*)), this, SLOT(curveXDataColumnChanged(const AbstractColumn*)));
	connect(m_filterCurve, SIGNAL(yDataColumnChanged(const AbstractColumn*)), this, SLOT(curveYDataColumnChanged(const AbstractColumn*)));
	connect(m_filterCurve, SIGNAL(filterDataChanged(XYFourierFilterCurve::FilterData)), this, SLOT(curveFilterDataChanged(XYFourierFilterCurve::FilterData)));
	connect(m_filterCurve, SIGNAL(sourceDataChangedSinceLastFilter()), this, SLOT(enableRecalculate()));
}

void XYFourierFilterCurveDock::setModel() {
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
void XYFourierFilterCurveDock::setCurves(QList<XYCurve*> list) {
	m_initializing=true;
	m_curvesList=list;
	m_curve=list.first();
	m_filterCurve = dynamic_cast<XYFourierFilterCurve*>(m_curve);
	Q_ASSERT(m_filterCurve);
	m_aspectTreeModel = new AspectTreeModel(m_curve->project());
	this->setModel();
	m_filterData = m_filterCurve->filterData();
	initGeneralTab();
	initTabs();
	m_initializing=false;
}

//*************************************************************
//**** SLOTs for changes triggered in XYFitCurveDock *****
//*************************************************************
void XYFourierFilterCurveDock::nameChanged(){
	if (m_initializing)
		return;

	m_curve->setName(uiGeneralTab.leName->text());
}

void XYFourierFilterCurveDock::commentChanged(){
	if (m_initializing)
		return;

	m_curve->setComment(uiGeneralTab.leComment->text());
}

void XYFourierFilterCurveDock::xDataColumnChanged(const QModelIndex& index) {
	qDebug()<<"XYFitCurveDock::xDataColumnChanged()";
	if (m_initializing)
		return;

	AbstractAspect* aspect = static_cast<AbstractAspect*>(index.internalPointer());
	AbstractColumn* column = 0;
	if (aspect) {
		column = dynamic_cast<AbstractColumn*>(aspect);
		Q_ASSERT(column);
	}

	foreach(XYCurve* curve, m_curvesList)
		dynamic_cast<XYFourierFilterCurve*>(curve)->setXDataColumn(column);
}

void XYFourierFilterCurveDock::yDataColumnChanged(const QModelIndex& index) {
	if (m_initializing)
		return;

	AbstractAspect* aspect = static_cast<AbstractAspect*>(index.internalPointer());
	AbstractColumn* column = 0;
	if (aspect) {
		column = dynamic_cast<AbstractColumn*>(aspect);
		Q_ASSERT(column);
	}

	foreach(XYCurve* curve, m_curvesList)
		dynamic_cast<XYFourierFilterCurve*>(curve)->setYDataColumn(column);
}

void XYFourierFilterCurveDock::typeChanged(int index) {
	XYFourierFilterCurve::FilterType type = (XYFourierFilterCurve::FilterType)index;
	m_filterData.type = (XYFourierFilterCurve::FilterType)uiGeneralTab.cbType->currentIndex();

	if(type == XYFourierFilterCurve::LowPass) {
		//TODO	
	}
	
/*
	if (type == XYFitCurve::Polynomial) {
		uiGeneralTab.lDegree->setVisible(true);
		uiGeneralTab.sbDegree->setVisible(true);
		uiGeneralTab.sbDegree->setMaximum(10);
		uiGeneralTab.sbDegree->setValue(1);
	} else if (type == XYFitCurve::Power) {
		uiGeneralTab.lDegree->setVisible(true);
		uiGeneralTab.sbDegree->setVisible(true);
		uiGeneralTab.sbDegree->setMaximum(2);
		uiGeneralTab.sbDegree->setValue(1);
	} else if (type == XYFitCurve::Exponential) {
		uiGeneralTab.lDegree->setVisible(true);
		uiGeneralTab.sbDegree->setVisible(true);
		uiGeneralTab.sbDegree->setMaximum(3);
		uiGeneralTab.sbDegree->setValue(1);
	} else if (type == XYFitCurve::Fourier) {
		uiGeneralTab.lDegree->setVisible(true);
		uiGeneralTab.sbDegree->setVisible(true);
		uiGeneralTab.sbDegree->setMaximum(10);
		uiGeneralTab.sbDegree->setValue(1);
	} else if (type == XYFitCurve::Gaussian) {
		uiGeneralTab.lDegree->setVisible(true);
		uiGeneralTab.sbDegree->setVisible(true);
		uiGeneralTab.sbDegree->setMaximum(10);
		uiGeneralTab.sbDegree->setValue(1);
	} else if (type == XYFitCurve::Lorentz || type == XYFitCurve::Maxwell || type == XYFitCurve::Inverse_Exponential || type == XYFitCurve::Custom) {
		uiGeneralTab.lDegree->setVisible(false);
		uiGeneralTab.sbDegree->setVisible(false);
	}
*/
}

void XYFourierFilterCurveDock::formChanged(int index) {
	//TODO
}

void XYFourierFilterCurveDock::unitChanged(int index) {
	//TODO
}

void XYFourierFilterCurveDock::recalculateClicked() {
	QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
	m_filterData.value = uiGeneralTab.sbValue->value();

	foreach(XYCurve* curve, m_curvesList)
		dynamic_cast<XYFourierFilterCurve*>(curve)->setFilterData(m_filterData);

	//uiGeneralTab.pbRecalculate->setEnabled(false);
	QApplication::restoreOverrideCursor();
}

void XYFourierFilterCurveDock::enableRecalculate() const {
	if (m_initializing)
		return;

	//no fitting possible without the x- and y-data
//	AbstractAspect* aspectX = static_cast<AbstractAspect*>(cbXDataColumn->currentModelIndex().internalPointer());
//	AbstractAspect* aspectY = static_cast<AbstractAspect*>(cbYDataColumn->currentModelIndex().internalPointer());
//	bool data = (aspectX!=0 && aspectY!=0);

//	XYFitCurve::ModelType type = (XYFitCurve::ModelType)uiGeneralTab.cbModel->currentIndex();
//	if (type==XYFitCurve::Custom)
//		uiGeneralTab.pbRecalculate->setEnabled( data && uiGeneralTab.teEquation->isValid() );
//	else
//		uiGeneralTab.pbRecalculate->setEnabled(data);
}

/*!
 * show the result and details of the filter
 */
void XYFourierFilterCurveDock::showFilterResult() {
	const XYFourierFilterCurve::FilterResult& filterResult = m_filterCurve->filterResult();
	if (!filterResult.available) {
		uiGeneralTab.teResult->clear();
		return;
	}

	const XYFourierFilterCurve::FilterData& filterData = m_filterCurve->filterData();
	QString str = i18n("status") + ": " + filterResult.status + "<br>";

	if (!filterResult.valid) {
		uiGeneralTab.teResult->setText(str);
		return; //result is not valid, there was an error which is shown in the status-string, nothing to show more.
	}

	if (filterResult.elapsedTime>1000)
		str += i18n("calculation time: %1 s").arg(QString::number(filterResult.elapsedTime/1000)) + "<br>";
	else
		str += i18n("calculation time: %1 ms").arg(QString::number(filterResult.elapsedTime)) + "<br>";

 	str += "<br><br>";

	uiGeneralTab.teResult->setText(str);
}

//*************************************************************
//*********** SLOTs for changes triggered in XYCurve **********
//*************************************************************
//General-Tab
void XYFourierFilterCurveDock::curveDescriptionChanged(const AbstractAspect* aspect) {
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

void XYFourierFilterCurveDock::curveXDataColumnChanged(const AbstractColumn* column) {
	qDebug()<<"XYFourierFilterCurveDock::curveXDataColumnChanged()";
	m_initializing = true;
	XYCurveDock::setModelIndexFromColumn(cbXDataColumn, column);
	m_initializing = false;
}

void XYFourierFilterCurveDock::curveYDataColumnChanged(const AbstractColumn* column) {
	m_initializing = true;
	XYCurveDock::setModelIndexFromColumn(cbYDataColumn, column);
	m_initializing = false;
}

void XYFourierFilterCurveDock::curveFilterDataChanged(const XYFourierFilterCurve::FilterData& data) {
	m_initializing = true;
	m_filterData = data;
	uiGeneralTab.cbType->setCurrentIndex(m_filterData.type);
	this->typeChanged(m_filterData.type);

	this->showFilterResult();
	m_initializing = false;
}

void XYFourierFilterCurveDock::dataChanged() {
	this->enableRecalculate();
}
