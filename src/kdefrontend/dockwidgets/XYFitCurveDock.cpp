/***************************************************************************
    File             : XYFitCurveDock.cpp
    Project          : LabPlot
    --------------------------------------------------------------------
    Copyright        : (C) 2014-2017 Alexander Semke (alexander.semke@web.de)
    Copyright        : (C) 2016-2018 Stefan Gerlach (stefan.gerlach@uni.kn)
    Description      : widget for editing properties of fit curves

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

#include "XYFitCurveDock.h"
#include "backend/core/AspectTreeModel.h"
#include "backend/core/Project.h"
#include "backend/lib/macros.h"
#include "backend/gsl/ExpressionParser.h"
#include "commonfrontend/widgets/TreeViewComboBox.h"
#include "kdefrontend/widgets/ConstantsWidget.h"
#include "kdefrontend/widgets/FunctionsWidget.h"
#include "kdefrontend/widgets/FitOptionsWidget.h"
#include "kdefrontend/widgets/FitParametersWidget.h"

#include <QMenu>
#include <QWidgetAction>
#include <QStandardItemModel>
#include <QStandardPaths>
#include <QClipboard>
#include <QDir>

extern "C" {
#include "backend/nsl/nsl_sf_stats.h"
}

/*!
  \class XYFitCurveDock
  \brief  Provides a widget for editing the properties of the XYFitCurves
		(2D-curves defined by a fit model) currently selected in
		the project explorer.

  If more then one curves are set, the properties of the first column are shown.
  The changes of the properties are applied to all curves.
  The exclusions are the name, the comment and the datasets (columns) of
  the curves  - these properties can only be changed if there is only one single curve.

  \ingroup kdefrontend
*/

XYFitCurveDock::XYFitCurveDock(QWidget* parent) : XYCurveDock(parent),
	cbDataSourceCurve(nullptr), cbXDataColumn(nullptr), cbYDataColumn(nullptr), cbXErrorColumn(nullptr),
	cbYErrorColumn(nullptr), m_fitCurve(nullptr) {

	//remove the tab "Error bars"
	ui.tabWidget->removeTab(5);
}

/*!
 * 	set up "General" tab
 */
void XYFitCurveDock::setupGeneral() {
	DEBUG("XYFitCurveDock::setupGeneral()");
	QWidget* generalTab = new QWidget(ui.tabGeneral);
	uiGeneralTab.setupUi(generalTab);
	QGridLayout* gridLayout = qobject_cast<QGridLayout*>(generalTab->layout());
	if (gridLayout) {
		gridLayout->setContentsMargins(2, 2, 2, 2);
		gridLayout->setHorizontalSpacing(2);
		gridLayout->setVerticalSpacing(2);
	}

	uiGeneralTab.cbDataSourceType->addItem(i18n("Spreadsheet"));
	uiGeneralTab.cbDataSourceType->addItem(i18n("XY-Curve"));

	cbDataSourceCurve = new TreeViewComboBox(generalTab);
	gridLayout->addWidget(cbDataSourceCurve, 6, 4, 1, 4);

	cbXDataColumn = new TreeViewComboBox(generalTab);
	gridLayout->addWidget(cbXDataColumn, 7, 4, 1, 4);

	cbXErrorColumn = new TreeViewComboBox(generalTab);
	cbXErrorColumn->setEnabled(false);
	uiGeneralTab.hlXError->addWidget(cbXErrorColumn);

	cbYDataColumn = new TreeViewComboBox(generalTab);
	gridLayout->addWidget(cbYDataColumn, 8, 4, 1, 4);

	cbYErrorColumn = new TreeViewComboBox(generalTab);
	cbYErrorColumn->setEnabled(false);
	uiGeneralTab.hlYWeight->addWidget(cbYErrorColumn);

	// X/Y-Weight
	for(int i = 0; i < NSL_FIT_WEIGHT_TYPE_COUNT; i++) {
		uiGeneralTab.cbXWeight->addItem(nsl_fit_weight_type_name[i]);
		uiGeneralTab.cbYWeight->addItem(nsl_fit_weight_type_name[i]);
	}
	uiGeneralTab.cbXWeight->setCurrentIndex(nsl_fit_weight_no);
	uiGeneralTab.cbYWeight->setCurrentIndex(nsl_fit_weight_no);

	for(int i = 0; i < NSL_FIT_MODEL_CATEGORY_COUNT; i++)
		uiGeneralTab.cbCategory->addItem(nsl_fit_model_category_name[i]);

	//show the fit-model category for the currently selected default (first) fit-model category
	//TODO: CHECK
	// categoryChanged(uiGeneralTab.cbCategory->currentIndex());

	uiGeneralTab.teEquation->setMaximumHeight(uiGeneralTab.leName->sizeHint().height() * 2);

	//TODO: don't need m_fitData
	fitParametersWidget = new FitParametersWidget(generalTab, &m_fitData);
	gridLayout->addWidget(fitParametersWidget, 19, 4, 2, 4);

	//use white background in the preview label
	QPalette p;
	p.setColor(QPalette::Window, Qt::white);
	uiGeneralTab.lFuncPic->setAutoFillBackground(true);
	uiGeneralTab.lFuncPic->setPalette(p);

	uiGeneralTab.tbConstants->setIcon(QIcon::fromTheme("labplot-format-text-symbol"));
	uiGeneralTab.tbFunctions->setIcon(QIcon::fromTheme("preferences-desktop-font"));
	uiGeneralTab.pbRecalculate->setIcon(QIcon::fromTheme("run-build"));

	uiGeneralTab.twLog->setEditTriggers(QAbstractItemView::NoEditTriggers);
	uiGeneralTab.twParameters->setEditTriggers(QAbstractItemView::NoEditTriggers);
	uiGeneralTab.twGoodness->setEditTriggers(QAbstractItemView::NoEditTriggers);

	//don't allow word wrapping in the log-table for the multi-line iterations string
	uiGeneralTab.twLog->setWordWrap(false);

	// show these options per default
	showDataOptions(true);
	showFitOptions(true);
	// hide these options per default
	showWeightsOptions(false);

	// context menus
	uiGeneralTab.twParameters->setContextMenuPolicy(Qt::CustomContextMenu);
	uiGeneralTab.twGoodness->setContextMenuPolicy(Qt::CustomContextMenu);
	uiGeneralTab.twLog->setContextMenuPolicy(Qt::CustomContextMenu);
	connect(uiGeneralTab.twParameters, SIGNAL(customContextMenuRequested(QPoint)), this,
			SLOT(resultParametersContextMenuRequest(QPoint)) );
	connect(uiGeneralTab.twGoodness, SIGNAL(customContextMenuRequested(QPoint)), this,
			SLOT(resultGoodnessContextMenuRequest(QPoint)) );
	connect(uiGeneralTab.twLog, SIGNAL(customContextMenuRequested(QPoint)), this,
			SLOT(resultLogContextMenuRequest(QPoint)) );

	uiGeneralTab.twLog->horizontalHeader()->resizeSections(QHeaderView::ResizeToContents);
	uiGeneralTab.twGoodness->horizontalHeader()->resizeSections(QHeaderView::ResizeToContents);
	uiGeneralTab.twGoodness->item(0, 1)->setText(UTF8_QSTRING("χ²"));
	uiGeneralTab.twGoodness->item(1, 1)->setText(i18n("reduced") + ' ' + UTF8_QSTRING("χ²")
		+ " (" + UTF8_QSTRING("χ²") + "/dof)");
	uiGeneralTab.twGoodness->item(3, 1)->setText(UTF8_QSTRING("R²"));
	uiGeneralTab.twGoodness->item(4, 1)->setText(UTF8_QSTRING("R̄²"));
	uiGeneralTab.twGoodness->item(5, 0)->setText(UTF8_QSTRING("χ²") + ' ' + i18n("test"));
	uiGeneralTab.twGoodness->item(5, 1)->setText("P > " + UTF8_QSTRING("χ²"));

	QHBoxLayout* layout = new QHBoxLayout(ui.tabGeneral);
	layout->setMargin(0);
	layout->addWidget(generalTab);

	//Slots
	connect(uiGeneralTab.leName, &QLineEdit::textChanged, this, &XYFitCurveDock::nameChanged);
	connect(uiGeneralTab.leComment, &QLineEdit::textChanged, this, &XYFitCurveDock::commentChanged);
	connect(uiGeneralTab.chkVisible, SIGNAL(clicked(bool)), this, SLOT(visibilityChanged(bool)));
	connect(uiGeneralTab.cbDataSourceType, SIGNAL(currentIndexChanged(int)), this, SLOT(dataSourceTypeChanged(int)));

	connect(uiGeneralTab.lData, SIGNAL(clicked(bool)), this, SLOT(showDataOptions(bool)));
	connect(uiGeneralTab.lWeights, SIGNAL(clicked(bool)), this, SLOT(showWeightsOptions(bool)));
	connect(uiGeneralTab.cbXWeight, SIGNAL(currentIndexChanged(int)), this, SLOT(xWeightChanged(int)));
	connect(uiGeneralTab.cbYWeight, SIGNAL(currentIndexChanged(int)), this, SLOT(yWeightChanged(int)));
	connect(uiGeneralTab.cbCategory, SIGNAL(currentIndexChanged(int)), this, SLOT(categoryChanged(int)));
	connect(uiGeneralTab.cbModel, SIGNAL(currentIndexChanged(int)), this, SLOT(modelTypeChanged(int)));
	connect(uiGeneralTab.sbDegree, SIGNAL(valueChanged(int)), this, SLOT(updateModelEquation()));
	connect(uiGeneralTab.teEquation, SIGNAL(expressionChanged()), this, SLOT(expressionChanged()));
	connect(uiGeneralTab.tbConstants, SIGNAL(clicked()), this, SLOT(showConstants()));
	connect(uiGeneralTab.tbFunctions, SIGNAL(clicked()), this, SLOT(showFunctions()));
	//TODO connect(uiGeneralTab.pbParameters, SIGNAL(clicked()), this, SLOT(showParameters()));
	connect(uiGeneralTab.pbOptions, SIGNAL(clicked()), this, SLOT(showOptions()));
	connect(uiGeneralTab.pbRecalculate, SIGNAL(clicked()), this, SLOT(recalculateClicked()));
	connect(uiGeneralTab.lFit, SIGNAL(clicked(bool)), this, SLOT(showFitOptions(bool)));

	connect(cbDataSourceCurve, SIGNAL(currentModelIndexChanged(QModelIndex)), this, SLOT(dataSourceCurveChanged(QModelIndex)));
	connect(cbXDataColumn, SIGNAL(currentModelIndexChanged(QModelIndex)), this, SLOT(xDataColumnChanged(QModelIndex)));
	connect(cbYDataColumn, SIGNAL(currentModelIndexChanged(QModelIndex)), this, SLOT(yDataColumnChanged(QModelIndex)));
	connect(cbXErrorColumn, SIGNAL(currentModelIndexChanged(QModelIndex)), this, SLOT(xErrorColumnChanged(QModelIndex)));
	connect(cbYErrorColumn, SIGNAL(currentModelIndexChanged(QModelIndex)), this, SLOT(yErrorColumnChanged(QModelIndex)));

}

/*
 * load curve settings
 */
void XYFitCurveDock::initGeneralTab() {
	DEBUG("XYFitCurveDock::initGeneralTab()");
	//if there are more then one curve in the list, disable the tab "general"
	if (m_curvesList.size() == 1) {
		uiGeneralTab.lName->setEnabled(true);
		uiGeneralTab.leName->setEnabled(true);
		uiGeneralTab.lComment->setEnabled(true);
		uiGeneralTab.leComment->setEnabled(true);

		uiGeneralTab.leName->setText(m_curve->name());
		uiGeneralTab.leComment->setText(m_curve->comment());
	} else {
		uiGeneralTab.lName->setEnabled(false);
		uiGeneralTab.leName->setEnabled(false);
		uiGeneralTab.lComment->setEnabled(false);
		uiGeneralTab.leComment->setEnabled(false);

		uiGeneralTab.leName->setText("");
		uiGeneralTab.leComment->setText("");
	}

	//show the properties of the first curve
	//CHECK: already done in setCurve
	//m_fitCurve = dynamic_cast<XYFitCurve*>(m_curve);

	uiGeneralTab.cbDataSourceType->setCurrentIndex(m_fitCurve->dataSourceType());
	this->dataSourceTypeChanged(uiGeneralTab.cbDataSourceType->currentIndex());
	XYCurveDock::setModelIndexFromAspect(cbDataSourceCurve, m_fitCurve->dataSourceCurve());
	XYCurveDock::setModelIndexFromAspect(cbXDataColumn, m_fitCurve->xDataColumn());
	XYCurveDock::setModelIndexFromAspect(cbYDataColumn, m_fitCurve->yDataColumn());
	XYCurveDock::setModelIndexFromAspect(cbXErrorColumn, m_fitCurve->xErrorColumn());
	XYCurveDock::setModelIndexFromAspect(cbYErrorColumn, m_fitCurve->yErrorColumn());

	int tmpModelType = m_fitData.modelType;	// save type because it's reset when category changes
	if (m_fitData.modelCategory == nsl_fit_model_custom)
		uiGeneralTab.cbCategory->setCurrentIndex(uiGeneralTab.cbCategory->count() - 1);
	else
		uiGeneralTab.cbCategory->setCurrentIndex(m_fitData.modelCategory);
	m_fitData.modelType = tmpModelType;
	if (m_fitData.modelCategory != nsl_fit_model_custom)
		uiGeneralTab.cbModel->setCurrentIndex(m_fitData.modelType);

	uiGeneralTab.cbXWeight->setCurrentIndex(m_fitData.xWeightsType);
	uiGeneralTab.cbYWeight->setCurrentIndex(m_fitData.yWeightsType);
	uiGeneralTab.sbDegree->setValue(m_fitData.degree);

	DEBUG("	B start value 0 = " << m_fitData.paramStartValues.at(0));
	DEBUG("	B model degree = " << m_fitData.degree);

	//show the fit-model category for the currently selected default (first) fit-model category
	//TODO: CHECK
	DEBUG("	CALLING categoryChanged()");
	categoryChanged(uiGeneralTab.cbCategory->currentIndex());

	DEBUG("	CALLING updateModelEquation()");
	updateModelEquation();

	this->showFitResult();

	uiGeneralTab.chkVisible->setChecked(m_curve->isVisible());

	//Slots
	connect(m_fitCurve, SIGNAL(aspectDescriptionChanged(const AbstractAspect*)), this, SLOT(curveDescriptionChanged(const AbstractAspect*)));
	connect(m_fitCurve, SIGNAL(dataSourceTypeChanged(XYAnalysisCurve::DataSourceType)), this, SLOT(curveDataSourceTypeChanged(XYAnalysisCurve::DataSourceType)));
	connect(m_fitCurve, SIGNAL(dataSourceCurveChanged(const XYCurve*)), this, SLOT(curveDataSourceCurveChanged(const XYCurve*)));
	connect(m_fitCurve, SIGNAL(xDataColumnChanged(const AbstractColumn*)), this, SLOT(curveXDataColumnChanged(const AbstractColumn*)));
	connect(m_fitCurve, SIGNAL(yDataColumnChanged(const AbstractColumn*)), this, SLOT(curveYDataColumnChanged(const AbstractColumn*)));
	connect(m_fitCurve, SIGNAL(xErrorColumnChanged(const AbstractColumn*)), this, SLOT(curveXErrorColumnChanged(const AbstractColumn*)));
	connect(m_fitCurve, SIGNAL(yErrorColumnChanged(const AbstractColumn*)), this, SLOT(curveYErrorColumnChanged(const AbstractColumn*)));
	connect(m_fitCurve, SIGNAL(fitDataChanged(XYFitCurve::FitData)), this, SLOT(curveFitDataChanged(XYFitCurve::FitData)));
	connect(m_fitCurve, SIGNAL(sourceDataChanged()), this, SLOT(enableRecalculate()));

	connect(fitParametersWidget, &FitParametersWidget::parametersChanged, this, &XYFitCurveDock::parametersChanged);
}

void XYFitCurveDock::setModel() {
	DEBUG("XYFitCurveDock::setModel()");
	QList<const char*> list;
	list << "Folder" << "Datapicker" << "Worksheet" << "CartesianPlot" << "XYCurve";
	cbDataSourceCurve->setTopLevelClasses(list);

	QList<const AbstractAspect*> hiddenAspects;
	for (auto* curve : m_curvesList)
		hiddenAspects << curve;
	cbDataSourceCurve->setHiddenAspects(hiddenAspects);

	list.clear();
	list << "Folder" << "Workbook" << "Spreadsheet" << "FileDataSource" << "Column" << "CantorWorksheet" << "Datapicker";
	cbXDataColumn->setTopLevelClasses(list);
	cbYDataColumn->setTopLevelClasses(list);
	cbXErrorColumn->setTopLevelClasses(list);
	cbYErrorColumn->setTopLevelClasses(list);

	cbDataSourceCurve->setModel(m_aspectTreeModel);
	cbXDataColumn->setModel(m_aspectTreeModel);
	cbYDataColumn->setModel(m_aspectTreeModel);
	cbXErrorColumn->setModel(m_aspectTreeModel);
	cbYErrorColumn->setModel(m_aspectTreeModel);

	XYCurveDock::setModel();
}

/*!
  sets the curves. The properties of the curves in the list \c list can be edited in this widget.
*/
void XYFitCurveDock::setCurves(QList<XYCurve*> list) {
	DEBUG("XYFitCurveDock::setCurves()");
	m_initializing = true;
	m_curvesList = list;
	m_curve = list.first();

	m_fitCurve = dynamic_cast<XYFitCurve*>(m_curve);
	m_aspectTreeModel = new AspectTreeModel(m_curve->project());
	this->setModel();
	m_fitData = m_fitCurve->fitData();
	DEBUG("	A start value 0 = " << m_fitData.paramStartValues.at(0));
	DEBUG("	A model degree = " << m_fitData.degree);
	DEBUG("	CALLING FitParametersWidget::setFitData()");
	fitParametersWidget->setFitData(&m_fitData);

	initGeneralTab();
	initTabs();

	m_initializing = false;

	enableRecalculate();
}

//*************************************************************
//**** SLOTs for changes triggered in XYFitCurveDock *****
//*************************************************************
void XYFitCurveDock::nameChanged() {
	if (m_initializing)
		return;

	m_curve->setName(uiGeneralTab.leName->text());
}

void XYFitCurveDock::commentChanged() {
	if (m_initializing)
		return;

	m_curve->setComment(uiGeneralTab.leComment->text());
}

void XYFitCurveDock::dataSourceTypeChanged(int index) {
	const XYAnalysisCurve::DataSourceType type = (XYAnalysisCurve::DataSourceType)index;
	if (type == XYAnalysisCurve::DataSourceSpreadsheet) {
		uiGeneralTab.lDataSourceCurve->hide();
		cbDataSourceCurve->hide();
		uiGeneralTab.lXColumn->show();
		cbXDataColumn->show();
		uiGeneralTab.lYColumn->show();
		cbYDataColumn->show();
	} else {
		uiGeneralTab.lDataSourceCurve->show();
		cbDataSourceCurve->show();
		uiGeneralTab.lXColumn->hide();
		cbXDataColumn->hide();
		uiGeneralTab.lYColumn->hide();
		cbYDataColumn->hide();
	}

	if (m_initializing)
		return;

	for (auto* curve : m_curvesList)
		dynamic_cast<XYFitCurve*>(curve)->setDataSourceType(type);
}

void XYFitCurveDock::dataSourceCurveChanged(const QModelIndex& index) {
	AbstractAspect* aspect = static_cast<AbstractAspect*>(index.internalPointer());
	XYCurve* dataSourceCurve = dynamic_cast<XYCurve*>(aspect);

	if (m_initializing)
		return;

	for (auto* curve : m_curvesList)
		dynamic_cast<XYFitCurve*>(curve)->setDataSourceCurve(dataSourceCurve);
}

void XYFitCurveDock::xDataColumnChanged(const QModelIndex& index) {
	DEBUG("XYFitCurveDock::xDataColumnChanged()");
	if (m_initializing)
		return;

	AbstractAspect* aspect = static_cast<AbstractAspect*>(index.internalPointer());
	AbstractColumn* column = dynamic_cast<AbstractColumn*>(aspect);

	for (auto* curve : m_curvesList)
		dynamic_cast<XYFitCurve*>(curve)->setXDataColumn(column);

	// set model dependent start values from new data
	XYFitCurve::initStartValues(m_fitData, m_curve);
}

void XYFitCurveDock::yDataColumnChanged(const QModelIndex& index) {
	DEBUG("XYFitCurveDock::yDataColumnChanged()");
	if (m_initializing)
		return;

	AbstractAspect* aspect = static_cast<AbstractAspect*>(index.internalPointer());
	AbstractColumn* column = dynamic_cast<AbstractColumn*>(aspect);

	for (auto* curve : m_curvesList)
		dynamic_cast<XYFitCurve*>(curve)->setYDataColumn(column);

	// set model dependent start values from new data
	XYFitCurve::initStartValues(m_fitData, m_curve);
}

void XYFitCurveDock::xErrorColumnChanged(const QModelIndex& index) {
	if (m_initializing)
		return;

	AbstractAspect* aspect = static_cast<AbstractAspect*>(index.internalPointer());
	AbstractColumn* column = dynamic_cast<AbstractColumn*>(aspect);

	for (auto* curve : m_curvesList)
		dynamic_cast<XYFitCurve*>(curve)->setXErrorColumn(column);
}

void XYFitCurveDock::yErrorColumnChanged(const QModelIndex& index) {
	if (m_initializing)
		return;

	AbstractAspect* aspect = static_cast<AbstractAspect*>(index.internalPointer());
	AbstractColumn* column = dynamic_cast<AbstractColumn*>(aspect);

	for (auto* curve : m_curvesList)
		dynamic_cast<XYFitCurve*>(curve)->setYErrorColumn(column);
}

void XYFitCurveDock::showDataOptions(bool checked) {
	if (checked) {
		uiGeneralTab.lData->setIcon(QIcon::fromTheme("arrow-down"));
		uiGeneralTab.lDataSourceType->show();
		uiGeneralTab.cbDataSourceType->show();
		// select options for current source type
		dataSourceTypeChanged(uiGeneralTab.cbDataSourceType->currentIndex());
	} else {
		uiGeneralTab.lData->setIcon(QIcon::fromTheme("arrow-right"));
		uiGeneralTab.lDataSourceType->hide();
		uiGeneralTab.cbDataSourceType->hide();
		uiGeneralTab.lXColumn->hide();
		cbXDataColumn->hide();
		uiGeneralTab.lYColumn->hide();
		cbYDataColumn->hide();
		uiGeneralTab.lDataSourceCurve->hide();
		cbDataSourceCurve->hide();
	}
}

void XYFitCurveDock::showWeightsOptions(bool checked) {
	if (checked) {
		uiGeneralTab.lWeights->setIcon(QIcon::fromTheme("arrow-down"));
		uiGeneralTab.lXWeight->show();
		uiGeneralTab.cbXWeight->show();
		uiGeneralTab.lXErrorCol->show();
		cbXErrorColumn->show();
		uiGeneralTab.lYWeight->show();
		uiGeneralTab.cbYWeight->show();
		uiGeneralTab.lYErrorCol->show();
		cbYErrorColumn->show();
	} else {
		uiGeneralTab.lWeights->setIcon(QIcon::fromTheme("arrow-right"));
		uiGeneralTab.lXWeight->hide();
		uiGeneralTab.cbXWeight->hide();
		uiGeneralTab.lXErrorCol->hide();
		cbXErrorColumn->hide();
		uiGeneralTab.lYWeight->hide();
		uiGeneralTab.cbYWeight->hide();
		uiGeneralTab.lYErrorCol->hide();
		cbYErrorColumn->hide();
	}
}

void XYFitCurveDock::showFitOptions(bool checked) {
	if (checked) {
		uiGeneralTab.lFit->setIcon(QIcon::fromTheme("arrow-down"));
		uiGeneralTab.lCategory->show();
		uiGeneralTab.cbCategory->show();
		uiGeneralTab.lModel->show();
		uiGeneralTab.cbModel->show();
		uiGeneralTab.lEquation->show();

		modelTypeChanged(uiGeneralTab.cbModel->currentIndex());
	} else {
		uiGeneralTab.lFit->setIcon(QIcon::fromTheme("arrow-right"));
		uiGeneralTab.lCategory->hide();
		uiGeneralTab.cbCategory->hide();
		uiGeneralTab.lModel->hide();
		uiGeneralTab.cbModel->hide();
		uiGeneralTab.lDegree->hide();
		uiGeneralTab.sbDegree->hide();
		uiGeneralTab.lEquation->hide();
		uiGeneralTab.lFuncPic->hide();
		uiGeneralTab.teEquation->hide();
		uiGeneralTab.tbFunctions->hide();
		uiGeneralTab.tbConstants->hide();
	}
}

void XYFitCurveDock::xWeightChanged(int index) {
	DEBUG("xWeightChanged() weight = " << nsl_fit_weight_type_name[index]);

	m_fitData.xWeightsType = (nsl_fit_weight_type)index;

	// enable/disable weight column
	switch ((nsl_fit_weight_type)index) {
	case nsl_fit_weight_no:
	case nsl_fit_weight_statistical:
	case nsl_fit_weight_statistical_fit:
	case nsl_fit_weight_relative:
	case nsl_fit_weight_relative_fit:
		cbXErrorColumn->setEnabled(false);
		uiGeneralTab.lXErrorCol->setEnabled(false);
		break;
	case nsl_fit_weight_instrumental:
	case nsl_fit_weight_direct:
	case nsl_fit_weight_inverse:
		cbXErrorColumn->setEnabled(true);
		uiGeneralTab.lXErrorCol->setEnabled(true);
		break;
	}
	enableRecalculate();
}

void XYFitCurveDock::yWeightChanged(int index) {
	DEBUG("yWeightChanged() weight = " << nsl_fit_weight_type_name[index]);

	m_fitData.yWeightsType = (nsl_fit_weight_type)index;

	// enable/disable weight column
	switch ((nsl_fit_weight_type)index) {
	case nsl_fit_weight_no:
	case nsl_fit_weight_statistical:
	case nsl_fit_weight_statistical_fit:
	case nsl_fit_weight_relative:
	case nsl_fit_weight_relative_fit:
		cbYErrorColumn->setEnabled(false);
		uiGeneralTab.lYErrorCol->setEnabled(false);
		break;
	case nsl_fit_weight_instrumental:
	case nsl_fit_weight_direct:
	case nsl_fit_weight_inverse:
		cbYErrorColumn->setEnabled(true);
		uiGeneralTab.lYErrorCol->setEnabled(true);
		break;
	}
	enableRecalculate();
}

/*!
 * called when the fit model category (basic functions, peak functions etc.) was changed.
 * In the combobox for the model type shows the model types for the current category \index and calls \c modelTypeChanged()
 * to update the model type dependent widgets in the general-tab.
 */
void XYFitCurveDock::categoryChanged(int index) {
	if (index == nsl_fit_model_custom) {
		DEBUG("categoryChanged() category = \"nsl_fit_model_custom\"");
	} else {
		DEBUG("categoryChanged() category = \"" << nsl_fit_model_category_name[index] << "\"");
	}

	// nothing has changed
	bool hasChanged = true;
	if (m_fitData.modelCategory == (nsl_fit_model_category)index || (m_fitData.modelCategory == nsl_fit_model_custom && index == uiGeneralTab.cbCategory->count() - 1) )
		hasChanged = false;

	if (uiGeneralTab.cbCategory->currentIndex() == uiGeneralTab.cbCategory->count() - 1)
		m_fitData.modelCategory = nsl_fit_model_custom;
	else
		m_fitData.modelCategory = (nsl_fit_model_category)index;

	m_initializing = true;
	uiGeneralTab.cbModel->clear();
	uiGeneralTab.cbModel->show();
	uiGeneralTab.lModel->show();

	switch (m_fitData.modelCategory) {
	case nsl_fit_model_basic:
		for(int i = 0; i < NSL_FIT_MODEL_BASIC_COUNT; i++)
			uiGeneralTab.cbModel->addItem(nsl_fit_model_basic_name[i]);
		break;
	case nsl_fit_model_peak: {
		for(int i = 0; i < NSL_FIT_MODEL_PEAK_COUNT; i++)
			uiGeneralTab.cbModel->addItem(nsl_fit_model_peak_name[i]);
#if defined(_MSC_VER)
		// disable voigt model
		const QStandardItemModel* model = qobject_cast<const QStandardItemModel*>(uiGeneralTab.cbModel->model());
		QStandardItem* item = model->item(nsl_fit_model_voigt);
		item->setFlags(item->flags() & ~(Qt::ItemIsSelectable|Qt::ItemIsEnabled));
#endif
		break;
	}
	case nsl_fit_model_growth:
		for(int i = 0; i < NSL_FIT_MODEL_GROWTH_COUNT; i++)
			uiGeneralTab.cbModel->addItem(nsl_fit_model_growth_name[i]);
		break;
	case nsl_fit_model_distribution: {
		for(int i = 0; i < NSL_SF_STATS_DISTRIBUTION_COUNT; i++)
			uiGeneralTab.cbModel->addItem(nsl_sf_stats_distribution_name[i]);

		// not-used items are disabled here
		const QStandardItemModel* model = qobject_cast<const QStandardItemModel*>(uiGeneralTab.cbModel->model());

		for(int i = 1; i < NSL_SF_STATS_DISTRIBUTION_COUNT; i++) {
			// unused distributions
			if (i == nsl_sf_stats_levy_alpha_stable || i == nsl_sf_stats_levy_skew_alpha_stable || i == nsl_sf_stats_bernoulli) {
					QStandardItem* item = model->item(i);
					item->setFlags(item->flags() & ~(Qt::ItemIsSelectable|Qt::ItemIsEnabled));
			}
		}
		break;
	}
	case nsl_fit_model_custom:
		uiGeneralTab.cbModel->addItem(i18n("Custom"));
		uiGeneralTab.cbModel->hide();
		uiGeneralTab.lModel->hide();
	}

	if (hasChanged) {
		//show the fit-model for the currently selected default (first) fit-model
		uiGeneralTab.cbModel->setCurrentIndex(0);
		uiGeneralTab.sbDegree->setValue(1);
	}

	m_initializing = false;
	enableRecalculate();
}

/*!
 * called when the fit model type (depends on category) was changed.
 * Updates the model type dependent widgets in the general-tab and calls \c updateModelEquation() to update the preview pixmap.
 */
void XYFitCurveDock::modelTypeChanged(int index) {
	DEBUG("modelTypeChanged() type = " << index << ", initializing = " << m_initializing << ", current type = " << m_fitData.modelType);
	// leave if there is no selection
	if(index == -1)
		return;

	bool custom = false;
	if (m_fitData.modelCategory == nsl_fit_model_custom)
		custom = true;
	uiGeneralTab.teEquation->setReadOnly(!custom);
	uiGeneralTab.lModel->setVisible(!custom);
	uiGeneralTab.cbModel->setVisible(!custom);
	uiGeneralTab.tbFunctions->setVisible(custom);
	uiGeneralTab.tbConstants->setVisible(custom);

	// default settings
	uiGeneralTab.lDegree->setText(i18n("Degree:"));
	if (m_fitData.modelType != index)
		uiGeneralTab.sbDegree->setValue(1);

	// TODO: reset start values

	switch (m_fitData.modelCategory) {
	case nsl_fit_model_basic:
		switch (index) {
		case nsl_fit_model_polynomial:
		case nsl_fit_model_fourier:
			uiGeneralTab.lDegree->setVisible(true);
			uiGeneralTab.sbDegree->setVisible(true);
			uiGeneralTab.sbDegree->setMaximum(10);
			break;
		case nsl_fit_model_power:
			uiGeneralTab.lDegree->setVisible(true);
			uiGeneralTab.sbDegree->setVisible(true);
			uiGeneralTab.sbDegree->setMaximum(2);
			break;
		case nsl_fit_model_exponential:
			uiGeneralTab.lDegree->setVisible(true);
			uiGeneralTab.sbDegree->setVisible(true);
			uiGeneralTab.sbDegree->setMaximum(10);
			break;
		default:
			uiGeneralTab.lDegree->setVisible(false);
			uiGeneralTab.sbDegree->setVisible(false);
		}
		break;
	case nsl_fit_model_peak:	// all models support multiple peaks
		uiGeneralTab.lDegree->setText(i18n("Number of peaks:"));
		uiGeneralTab.lDegree->setVisible(true);
		uiGeneralTab.sbDegree->setVisible(true);
		uiGeneralTab.sbDegree->setMaximum(9);
		break;
	case nsl_fit_model_growth:
	case nsl_fit_model_distribution:
	case nsl_fit_model_custom:
		uiGeneralTab.lDegree->setVisible(false);
		uiGeneralTab.sbDegree->setVisible(false);
	}

	if (m_fitData.modelType != index) {
		DEBUG("	type has changed. CALLING updateModelEquation()");
		updateModelEquation();
	}
	m_fitData.modelType = index;
}

/*!
 * Show the preview pixmap of the fit model expression for the current model category and type.
 * Called when the model type or the degree of the model were changed.
 */
void XYFitCurveDock::updateModelEquation() {

	if (m_fitData.modelCategory == nsl_fit_model_custom) {
		DEBUG("XYFitCurveDock::updateModelEquation() category = nsl_fit_model_custom, type = " << m_fitData.modelType);
	} else {
		DEBUG("XYFitCurveDock::updateModelEquation() category = " << nsl_fit_model_category_name[m_fitData.modelCategory] << ", type = " << m_fitData.modelType);
	}

	//this function can also be called when the value for the degree was changed -> update the fit data structure
	int degree = uiGeneralTab.sbDegree->value();
	m_fitData.degree = degree;
	XYFitCurve::initFitData(m_fitData);
	// set model dependent start values from curve data
	XYFitCurve::initStartValues(m_fitData, m_curve);
//TODO	fitParametersWidget->setFitData(&m_fitData);

	// variables/parameter that are known
	QStringList vars = {"x"};
	vars << m_fitData.paramNames;
	uiGeneralTab.teEquation->setVariables(vars);

	// set formula picture
	uiGeneralTab.lEquation->setText(QLatin1String("f(x) ="));
	QString file;
	switch (m_fitData.modelCategory) {
	case nsl_fit_model_basic: {
		// formula pic depends on degree
		QString numSuffix = QString::number(degree);
		if (degree > 4)
			numSuffix = '4';
		if ((nsl_fit_model_type_basic)m_fitData.modelType == nsl_fit_model_power && degree > 2)
			numSuffix = '2';
		file = QStandardPaths::locate(QStandardPaths::AppDataLocation, "pics/fit_models/"
			+ QString(nsl_fit_model_basic_pic_name[m_fitData.modelType]) + numSuffix + ".png");
		break;
	}
	case nsl_fit_model_peak: {
		// formula pic depends on number of peaks
		QString numSuffix = QString::number(degree);
		if (degree > 4)
			numSuffix = '4';
		file = QStandardPaths::locate(QStandardPaths::AppDataLocation, "pics/fit_models/"
			+ QString(nsl_fit_model_peak_pic_name[m_fitData.modelType]) + numSuffix + ".png");
		break;
	}
	case nsl_fit_model_growth:
		file = QStandardPaths::locate(QStandardPaths::AppDataLocation, "pics/fit_models/"
			+ QString(nsl_fit_model_growth_pic_name[m_fitData.modelType]) + ".png");
		break;
	case nsl_fit_model_distribution:
		file = QStandardPaths::locate(QStandardPaths::AppDataLocation, "pics/gsl_distributions/"
			+ QString(nsl_sf_stats_distribution_pic_name[m_fitData.modelType]) + ".png");
		// change label
		if (m_fitData.modelType == nsl_sf_stats_poisson)
			uiGeneralTab.lEquation->setText(QLatin1String("f(k)/A ="));
		else
			uiGeneralTab.lEquation->setText(QLatin1String("f(x)/A ="));
		break;
	case nsl_fit_model_custom:
		uiGeneralTab.lFuncPic->hide();
		uiGeneralTab.teEquation->show();
		uiGeneralTab.teEquation->setPlainText(m_fitData.model);
	}

	if (m_fitData.modelCategory != nsl_fit_model_custom) {
		DEBUG("Model pixmap path = " << file.toStdString());
		uiGeneralTab.lFuncPic->setPixmap(file);
		uiGeneralTab.lFuncPic->show();
		uiGeneralTab.teEquation->hide();
	}

	enableRecalculate();
}

void XYFitCurveDock::showConstants() {
	QMenu menu;
	ConstantsWidget constants(&menu);

	connect(&constants, SIGNAL(constantSelected(QString)), this, SLOT(insertConstant(QString)));
	connect(&constants, SIGNAL(constantSelected(QString)), &menu, SLOT(close()));
	connect(&constants, SIGNAL(canceled()), &menu, SLOT(close()));

	QWidgetAction* widgetAction = new QWidgetAction(this);
	widgetAction->setDefaultWidget(&constants);
	menu.addAction(widgetAction);

	QPoint pos(-menu.sizeHint().width() + uiGeneralTab.tbConstants->width(), -menu.sizeHint().height());
	menu.exec(uiGeneralTab.tbConstants->mapToGlobal(pos));
}

void XYFitCurveDock::showFunctions() {
	QMenu menu;
	FunctionsWidget functions(&menu);
	connect(&functions, SIGNAL(functionSelected(QString)), this, SLOT(insertFunction(QString)));
	connect(&functions, SIGNAL(functionSelected(QString)), &menu, SLOT(close()));
	connect(&functions, SIGNAL(canceled()), &menu, SLOT(close()));

	QWidgetAction* widgetAction = new QWidgetAction(this);
	widgetAction->setDefaultWidget(&functions);
	menu.addAction(widgetAction);

	QPoint pos(-menu.sizeHint().width() + uiGeneralTab.tbFunctions->width(), -menu.sizeHint().height());
	menu.exec(uiGeneralTab.tbFunctions->mapToGlobal(pos));
}

/*!
 * Update parameter by parsing expression
 * Only called for custom fit model
 */
void XYFitCurveDock::updateParameterList() {
	DEBUG("XYFitCurveDock::updateParameterList()");
	// use current model function
	m_fitData.model = uiGeneralTab.teEquation->toPlainText();

	ExpressionParser* parser = ExpressionParser::getInstance();
	QStringList vars; // variables that are known
	vars << "x";	//TODO: others?
	m_fitData.paramNames = m_fitData.paramNamesUtf8 = parser->getParameter(m_fitData.model, vars);

	// if number of parameter changed
	int oldNumberOfParameter = m_fitData.paramStartValues.size();
	int numberOfParameter = m_fitData.paramNames.size();
	DEBUG(" old number of parameter: " << oldNumberOfParameter << " new number of parameter: " << numberOfParameter);
	if (numberOfParameter != oldNumberOfParameter) {
		m_fitData.paramStartValues.resize(numberOfParameter);
		m_fitData.paramFixed.resize(numberOfParameter);
		m_fitData.paramLowerLimits.resize(numberOfParameter);
		m_fitData.paramUpperLimits.resize(numberOfParameter);
	}
	if (numberOfParameter > oldNumberOfParameter) {
		for (int i = oldNumberOfParameter; i < numberOfParameter; ++i) {
			m_fitData.paramStartValues[i] = 1.0;
			m_fitData.paramFixed[i] = false;
			m_fitData.paramLowerLimits[i] = -std::numeric_limits<double>::max();
			m_fitData.paramUpperLimits[i] = std::numeric_limits<double>::max();
		}
	}

	parametersChanged();
}

/*!
 * open parameter dialog to change parameter settings
 * TODO: not needed anymore
 */
void XYFitCurveDock::showParameters() {
	DEBUG("XYFitCurveDock::showParameters()");
	if (m_fitData.modelCategory == nsl_fit_model_custom)
		updateParameterList();

	QMenu menu;
	FitParametersWidget w(&menu, &m_fitData);
	connect(&w, SIGNAL(finished()), &menu, SLOT(close()));
	connect(&w, SIGNAL(parametersChanged()), this, SLOT(parametersChanged()));

	QWidgetAction* widgetAction = new QWidgetAction(this);
	widgetAction->setDefaultWidget(&w);
	menu.addAction(widgetAction);
	menu.setMinimumWidth(w.width());
	menu.setTearOffEnabled(true);

	//menu.setWindowFlags(menu.windowFlags() & Qt::MSWindowsFixedSizeDialogHint);

	//QPoint pos(-menu.sizeHint().width() + uiGeneralTab.pbParameters->width(), 0);
	//menu.exec(uiGeneralTab.pbParameters->mapToGlobal(pos));
}

/*!
 * called when parameter names and/or start values for the model were changed
 * also called from parameter widget
 */
void XYFitCurveDock::parametersChanged() {
	DEBUG("XYFitCurveDock::parametersChanged() m_initializing = " << m_initializing);

	//parameter names were (probably) changed -> set the new names in EquationTextEdit
	uiGeneralTab.teEquation->setVariables(m_fitData.paramNames);

	if (m_initializing)
		return;

	enableRecalculate();
}

void XYFitCurveDock::showOptions() {
	QMenu menu;
	FitOptionsWidget w(&menu, &m_fitData, m_fitCurve);
	connect(&w, SIGNAL(finished()), &menu, SLOT(close()));
	connect(&w, SIGNAL(optionsChanged()), this, SLOT(enableRecalculate()));

	QWidgetAction* widgetAction = new QWidgetAction(this);
	widgetAction->setDefaultWidget(&w);
	menu.addAction(widgetAction);
	menu.setTearOffEnabled(true);

	//menu.setWindowFlags(menu.windowFlags() & Qt::MSWindowsFixedSizeDialogHint);

	QPoint pos(-menu.sizeHint().width() + uiGeneralTab.pbOptions->width(), 0);
	menu.exec(uiGeneralTab.pbOptions->mapToGlobal(pos));
}

void XYFitCurveDock::insertFunction(const QString& str) const {
	//TODO: not all function have only one argument!
	uiGeneralTab.teEquation->insertPlainText(str + "(x)");
}

void XYFitCurveDock::insertConstant(const QString& str) const {
	uiGeneralTab.teEquation->insertPlainText(str);
}

void XYFitCurveDock::recalculateClicked() {
	QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
	m_fitData.degree = uiGeneralTab.sbDegree->value();
	if (m_fitData.modelCategory == nsl_fit_model_custom)
		updateParameterList();

	for (XYCurve* curve: m_curvesList)
		dynamic_cast<XYFitCurve*>(curve)->setFitData(m_fitData);

	fitParametersWidget->setFitData(&m_fitData);
	this->showFitResult();
	uiGeneralTab.pbRecalculate->setEnabled(false);
	emit info(i18n("Fit status: %1", m_fitCurve->fitResult().status));
	QApplication::restoreOverrideCursor();
}

void XYFitCurveDock::expressionChanged() {
	DEBUG("XYFitCurveDock::expressionChanged()");
	if (m_initializing)
		return;

	// update parameter list for custom model
	if (m_fitData.modelCategory == nsl_fit_model_custom)
		updateParameterList();

	enableRecalculate();
}

void XYFitCurveDock::enableRecalculate() const {
	DEBUG("XYFitCurveDock::enableRecalculate()");
	if (m_initializing) {
		DEBUG("	initializing");
	}
	if (m_initializing || m_fitCurve == nullptr)
		return;

	//no fitting possible without the x- and y-data
	bool hasSourceData = false;
	if (m_fitCurve->dataSourceType() == XYAnalysisCurve::DataSourceSpreadsheet) {
		AbstractAspect* aspectX = static_cast<AbstractAspect*>(cbXDataColumn->currentModelIndex().internalPointer());
		AbstractAspect* aspectY = static_cast<AbstractAspect*>(cbYDataColumn->currentModelIndex().internalPointer());
		hasSourceData = (aspectX != nullptr && aspectY != nullptr);
	} else {
		hasSourceData = (m_fitCurve->dataSourceCurve() != nullptr);
	}

	uiGeneralTab.pbRecalculate->setEnabled(hasSourceData);
	if (hasSourceData) {

		DEBUG("	enable and preview");
		// PREVIEW as soon as recalculate is enabled
		//TODO: this breaks loading a project with fit curve
		//	(sets starting values again!)
		//m_fitCurve->evaluate(true);
		//TODO: Test	(breaks context menu fit)
//		m_fitCurve->dataChanged();
	}
	else {
		DEBUG("	disable");
	}
}

void XYFitCurveDock::resultCopySelection() {
	QTableWidget* tw{nullptr};
	int currentTab = uiGeneralTab.twResults->currentIndex();
	DEBUG("current tab = " << currentTab);
	if (currentTab == 0)
		tw = uiGeneralTab.twParameters;
	else if (currentTab == 1)
		tw = uiGeneralTab.twGoodness;
	else if (currentTab == 2)
		tw = uiGeneralTab.twLog;
	else
		return;

	const QTableWidgetSelectionRange& range = tw->selectedRanges().constFirst();
	QString str;
	for (int i = 0; i < range.rowCount(); ++i) {
		if (i > 0)
			str += '\n';
		for (int j = 0; j < range.columnCount(); ++j) {
			if (j > 0)
				str += '\t';
			str += tw->item(range.topRow() + i, range.leftColumn() + j)->text();
		}
	}
	str += '\n';
	QApplication::clipboard()->setText(str);
	DEBUG(QApplication::clipboard()->text().toStdString());
}

void XYFitCurveDock::resultCopyAll() {
	const XYFitCurve::FitResult& fitResult = m_fitCurve->fitResult();
	int currentTab = uiGeneralTab.twResults->currentIndex();
	QString str;
	if (currentTab == 0) {
		str = i18n("Parameters:") + '\n';

		const int np = fitResult.paramValues.size();
		for (int i = 0; i < np; i++) {
			if (m_fitData.paramFixed.at(i))
				str += m_fitData.paramNamesUtf8.at(i) + QString(" = ") + QString::number(fitResult.paramValues.at(i)) + '\n';
			else {
				str += m_fitData.paramNamesUtf8.at(i) + QString(" = ") + QString::number(fitResult.paramValues.at(i))
					+ UTF8_QSTRING("±") + QString::number(fitResult.errorValues.at(i))
					+ " (" + QString::number(100.*fitResult.errorValues.at(i)/std::abs(fitResult.paramValues.at(i)), 'g', 3) + " %)\n";

				const double margin = fitResult.tdist_marginValues.at(i);
				QString tdistValueString;
				if (fitResult.tdist_tValues.at(i) < std::numeric_limits<double>::max())
					tdistValueString = QString::number(fitResult.tdist_tValues.at(i), 'g', 3);
				else
					tdistValueString = UTF8_QSTRING("∞");
				str += " (" + i18n("t statistic:") + ' ' + tdistValueString + ", "
					+ i18n("p value:") + ' ' + QString::number(fitResult.tdist_pValues.at(i), 'g', 3) + ", "
					+ i18n("conf. interval:") + ' ';
				if (std::abs(fitResult.tdist_tValues.at(i)) < 1.e6) {
					str += QString::number(fitResult.paramValues.at(i) - margin) + " .. "
						+ QString::number(fitResult.paramValues.at(i) + margin) + ")\n";
				} else {
					str += i18n("too small");
				}
			}
		}
	} else if (currentTab == 1) {
		str = i18n("Goodness of fit:") + '\n';
		str += i18n("sum of squared residuals") + " (" + UTF8_QSTRING("χ²") + "): " + QString::number(fitResult.sse) + '\n';
		if (fitResult.dof != 0) {
			str += i18n("reduced") + ' ' + UTF8_QSTRING("χ²") + ": " + QString::number(fitResult.rms) + '\n';
			str += i18n("root mean square error") + " (RMSE): " + QString::number(fitResult.rsd) + '\n';
			str += i18n("coefficient of determination") + " (" + UTF8_QSTRING("R²") + "): " + QString::number(fitResult.rsquare, 'g', 15) + '\n';
			str += i18n("adj. coefficient of determination")+ " (" + UTF8_QSTRING("R̄²")
				+ "): " + QString::number(fitResult.rsquareAdj, 'g', 15) + "\n\n";

			str += i18n("P > ") + UTF8_QSTRING("χ²") + ": " + QString::number(fitResult.chisq_p, 'g', 3) + '\n';
			str += i18n("F statistic") + ": " + QString::number(fitResult.fdist_F, 'g', 3) + '\n';
			str += i18n("P > F") + ": " + QString::number(fitResult.fdist_p, 'g', 3) + '\n';
		}
		str += i18n("mean absolute error:") + ' ' + QString::number(fitResult.mae) + '\n';
		str += i18n("Akaike information criterion:") + ' ' + QString::number(fitResult.aic) + '\n';
		str += i18n("Bayesian information criterion:") + ' ' + QString::number(fitResult.bic) + '\n';
	} else if (currentTab == 2) {
		str = i18n("status:") + ' ' + fitResult.status + '\n';
		str += i18n("iterations:") + ' ' + QString::number(fitResult.iterations) + '\n';
		str += i18n("tolerance:") + ' ' + QString::number(m_fitData.eps) + '\n';
		if (fitResult.elapsedTime > 1000)
			str += i18n("calculation time: %1 s", fitResult.elapsedTime/1000) + '\n';
		else
			str += i18n("calculation time: %1 ms", fitResult.elapsedTime) + '\n';
		str += i18n("degrees of freedom:") + ' ' + QString::number(fitResult.dof) + '\n';
		str += i18n("number of parameters:") + ' ' + QString::number(fitResult.paramValues.size()) + '\n';
		str += i18n("fit range:") + ' ' + QString::number(m_fitData.fitRange.first()) + " .. " + QString::number(m_fitData.fitRange.last()) + '\n';

		str += i18n("Iterations:") + '\n';
		for (const auto &s: m_fitData.paramNamesUtf8)
			str += s + '\t';
		str += UTF8_QSTRING("χ²");

		const QStringList iterations = fitResult.solverOutput.split(';');
		for (const auto &s: iterations)
			if (!s.isEmpty())
				str += '\n' + s;
	}

	QApplication::clipboard()->setText(str);
	DEBUG(QApplication::clipboard()->text().toStdString());
}

void XYFitCurveDock::resultParametersContextMenuRequest(QPoint pos) {
	QMenu *contextMenu = new QMenu;
	contextMenu->addAction(i18n("Copy Selection"), this, SLOT(resultCopySelection()));
	contextMenu->addAction(i18n("Copy All"), this, SLOT(resultCopyAll()));
	contextMenu->exec(uiGeneralTab.twParameters->mapToGlobal(pos));
}
void XYFitCurveDock::resultGoodnessContextMenuRequest(QPoint pos) {
	QMenu *contextMenu = new QMenu;
	contextMenu->addAction(i18n("Copy Selection"), this, SLOT(resultCopySelection()));
	contextMenu->addAction(i18n("Copy All"), this, SLOT(resultCopyAll()));
	contextMenu->exec(uiGeneralTab.twGoodness->mapToGlobal(pos));
}
void XYFitCurveDock::resultLogContextMenuRequest(QPoint pos) {
	QMenu *contextMenu = new QMenu;
	contextMenu->addAction(i18n("Copy Selection"), this, SLOT(resultCopySelection()));
	contextMenu->addAction(i18n("Copy All"), this, SLOT(resultCopyAll()));
	contextMenu->exec(uiGeneralTab.twLog->mapToGlobal(pos));
}

			 /*!
 * show the result and details of fit
 */
void XYFitCurveDock::showFitResult() {
	DEBUG("XYFitCurveDock::showFitResult()");

	//clear the previous result
	uiGeneralTab.twParameters->setRowCount(0);
	for (int row = 0; row < uiGeneralTab.twGoodness->rowCount(); ++row)
		uiGeneralTab.twGoodness->item(row, 2)->setText("");
	for (int row = 0; row < uiGeneralTab.twLog->rowCount(); ++row)
		uiGeneralTab.twLog->item(row, 1)->setText("");

	const XYFitCurve::FitResult& fitResult = m_fitCurve->fitResult();

	if (!fitResult.available) {
		DEBUG("fit result not available");
		return;
	}

	// Log
	uiGeneralTab.twLog->item(0, 1)->setText(fitResult.status);

	if (!fitResult.valid) {
		DEBUG("fit result not valid");
		return;
	}

	uiGeneralTab.twLog->item(1, 1)->setText(QString::number(fitResult.iterations));
	uiGeneralTab.twLog->item(2, 1)->setText(QString::number(m_fitData.eps));
	if (fitResult.elapsedTime > 1000)
		uiGeneralTab.twLog->item(3, 1)->setText(QString::number(fitResult.elapsedTime/1000) + " s");
	else
		uiGeneralTab.twLog->item(3, 1)->setText(QString::number(fitResult.elapsedTime) + " ms");

	uiGeneralTab.twLog->item(4, 1)->setText(QString::number(fitResult.dof));
	uiGeneralTab.twLog->item(5, 1)->setText(QString::number(fitResult.paramValues.size()));
	uiGeneralTab.twLog->item(6, 1)->setText(QString::number(m_fitData.fitRange.first()) + " .. " + QString::number(m_fitData.fitRange.last()) );

	// show all iterations
	QString str;
	for (const auto &s: m_fitData.paramNamesUtf8)
		str += s + '\t';
	str += UTF8_QSTRING("χ²");

	const QStringList iterations = fitResult.solverOutput.split(';');
	for (const auto &s: iterations)
		if (!s.isEmpty())
			str += '\n' + s;
	uiGeneralTab.twLog->item(7, 1)->setText(str);
	uiGeneralTab.twLog->resizeRowsToContents();

	// Parameters
	const int np = m_fitData.paramNames.size();
	uiGeneralTab.twParameters->setRowCount(np);
	QStringList headerLabels;
	headerLabels << i18n("Name") << i18n("Value") << i18n("Error") << i18n("Error, %") << i18n("t statistic") << QLatin1String("P > |t|") << i18n("Conf. Interval");
	uiGeneralTab.twParameters->setHorizontalHeaderLabels(headerLabels);

	for (int i = 0; i < np; i++) {
		const double paramValue = fitResult.paramValues.at(i);
		const double errorValue = fitResult.errorValues.at(i);

		QTableWidgetItem* item = new QTableWidgetItem(m_fitData.paramNamesUtf8.at(i));
		item->setBackground(QApplication::palette().color(QPalette::Window));
		uiGeneralTab.twParameters->setItem(i, 0, item);
		item = new QTableWidgetItem(QString::number(paramValue));
		uiGeneralTab.twParameters->setItem(i, 1, item);

		if (!m_fitData.paramFixed.at(i)) {
			if (!std::isnan(errorValue)) {
				item = new QTableWidgetItem(QString::number(errorValue, 'g', 6));
				uiGeneralTab.twParameters->setItem(i, 2, item);
				item = new QTableWidgetItem(QString::number(100.*errorValue/std::abs(paramValue), 'g', 3));
				uiGeneralTab.twParameters->setItem(i, 3, item);
			} else {
				item = new QTableWidgetItem(UTF8_QSTRING("∞"));
				uiGeneralTab.twParameters->setItem(i, 2, item);
				item = new QTableWidgetItem(UTF8_QSTRING("∞"));
				uiGeneralTab.twParameters->setItem(i, 3, item);
			}

			// t values
			QString tdistValueString;
			if (fitResult.tdist_tValues.at(i) < std::numeric_limits<double>::max())
				tdistValueString = QString::number(fitResult.tdist_tValues.at(i), 'g', 3);
			else
				tdistValueString = UTF8_QSTRING("∞");
			item = new QTableWidgetItem(tdistValueString);
			uiGeneralTab.twParameters->setItem(i, 4, item);

			// p values
			const double p = fitResult.tdist_pValues.at(i);
			item = new QTableWidgetItem(QString::number(p, 'g', 3));
			// color p values depending on value
			if (p > 0.05)
				item->setTextColor(QApplication::palette().color(QPalette::LinkVisited));
			else if (p > 0.01)
				item->setTextColor(Qt::darkGreen);
			else if (p > 0.001)
				item->setTextColor(Qt::darkCyan);
			else if (p > 0.0001)
				item->setTextColor(QApplication::palette().color(QPalette::Link));
			else
				item->setTextColor(QApplication::palette().color(QPalette::Highlight));
			uiGeneralTab.twParameters->setItem(i, 5, item);

			// Conf. interval
			if (!std::isnan(errorValue)) {
				const double margin = fitResult.tdist_marginValues.at(i);
				if (fitResult.tdist_tValues.at(i) < 1.e6)
					item = new QTableWidgetItem(QString::number(paramValue - margin) + QLatin1String(" .. ") + QString::number(paramValue + margin));
				else
					item = new QTableWidgetItem(i18n("too small"));
				uiGeneralTab.twParameters->setItem(i, 6, item);
			}
		}
	}

	// Goodness of fit
	uiGeneralTab.twGoodness->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
	uiGeneralTab.twGoodness->item(0, 2)->setText(QString::number(fitResult.sse));

	if (fitResult.dof != 0) {
		uiGeneralTab.twGoodness->item(1, 2)->setText(QString::number(fitResult.rms));
		uiGeneralTab.twGoodness->item(2, 2)->setText(QString::number(fitResult.rsd));

		uiGeneralTab.twGoodness->item(3, 2)->setText(QString::number(fitResult.rsquare, 'g', 15));
		uiGeneralTab.twGoodness->item(4, 2)->setText(QString::number(fitResult.rsquareAdj, 'g', 15));

		// chi^2 and F test p-values
		uiGeneralTab.twGoodness->item(5, 2)->setText(QString::number(fitResult.chisq_p, 'g', 3));
		uiGeneralTab.twGoodness->item(6, 2)->setText(QString::number(fitResult.fdist_F, 'g', 3));
		uiGeneralTab.twGoodness->item(7, 2)->setText(QString::number(fitResult.fdist_p, 'g', 3));
		uiGeneralTab.twGoodness->item(9, 2)->setText(QString::number(fitResult.aic, 'g', 3));
		uiGeneralTab.twGoodness->item(10, 2)->setText(QString::number(fitResult.bic, 'g', 3));
	}

	uiGeneralTab.twGoodness->item(8, 2)->setText(QString::number(fitResult.mae));

	//resize the table headers to fit the new content
	uiGeneralTab.twLog->resizeColumnsToContents();
	uiGeneralTab.twParameters->resizeColumnsToContents();
	//twGoodness doesn't have any header -> resize sections
	uiGeneralTab.twGoodness->resizeColumnToContents(0);
	uiGeneralTab.twGoodness->resizeColumnToContents(1);
	uiGeneralTab.twGoodness->resizeColumnToContents(2);

	//enable the "recalculate"-button if the source data was changed since the last fit
	uiGeneralTab.pbRecalculate->setEnabled(m_fitCurve->isSourceDataChangedSinceLastRecalc());
}

//*************************************************************
//*********** SLOTs for changes triggered in XYCurve **********
//*************************************************************
//General-Tab
void XYFitCurveDock::curveDescriptionChanged(const AbstractAspect* aspect) {
	if (m_curve != aspect)
		return;

	m_initializing = true;
	if (aspect->name() != uiGeneralTab.leName->text())
		uiGeneralTab.leName->setText(aspect->name());
	else if (aspect->comment() != uiGeneralTab.leComment->text())
		uiGeneralTab.leComment->setText(aspect->comment());
	m_initializing = false;
}

void XYFitCurveDock::curveDataSourceTypeChanged(XYAnalysisCurve::DataSourceType type) {
	m_initializing = true;
	uiGeneralTab.cbDataSourceType->setCurrentIndex(type);
	m_initializing = false;
}

void XYFitCurveDock::curveDataSourceCurveChanged(const XYCurve* curve) {
	m_initializing = true;
	XYCurveDock::setModelIndexFromAspect(cbDataSourceCurve, curve);
	m_initializing = false;
}

void XYFitCurveDock::curveXDataColumnChanged(const AbstractColumn* column) {
	m_initializing = true;
	XYCurveDock::setModelIndexFromAspect(cbXDataColumn, column);
	m_initializing = false;
}

void XYFitCurveDock::curveYDataColumnChanged(const AbstractColumn* column) {
	m_initializing = true;
	XYCurveDock::setModelIndexFromAspect(cbYDataColumn, column);
	m_initializing = false;
}

void XYFitCurveDock::curveXErrorColumnChanged(const AbstractColumn* column) {
	m_initializing = true;
	XYCurveDock::setModelIndexFromAspect(cbXErrorColumn, column);
	m_initializing = false;
}

void XYFitCurveDock::curveYErrorColumnChanged(const AbstractColumn* column) {
	m_initializing = true;
	XYCurveDock::setModelIndexFromAspect(cbYErrorColumn, column);
	m_initializing = false;
}

/*!
 * called when fit data of fit curve changes
 */
void XYFitCurveDock::curveFitDataChanged(const XYFitCurve::FitData& fitData) {
	DEBUG("XYFitCurveDock::curveFitDataChanged()");
	m_initializing = true;
	m_fitData = fitData;

	if (m_fitData.modelCategory != nsl_fit_model_custom)
		uiGeneralTab.cbModel->setCurrentIndex(m_fitData.modelType);

	uiGeneralTab.sbDegree->setValue(m_fitData.degree);
	this->showFitResult();
	m_initializing = false;
}

void XYFitCurveDock::dataChanged() {
	this->enableRecalculate();
}
