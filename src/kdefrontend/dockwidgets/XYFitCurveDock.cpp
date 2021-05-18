/***************************************************************************
    File             : XYFitCurveDock.cpp
    Project          : LabPlot
    --------------------------------------------------------------------
    Copyright        : (C) 2014-2020 Alexander Semke (alexander.semke@web.de)
    Copyright        : (C) 2016-2021 Stefan Gerlach (stefan.gerlach@uni.kn)
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
#include "backend/worksheet/plots/cartesian/CartesianPlot.h"
#include "commonfrontend/widgets/TreeViewComboBox.h"
#include "kdefrontend/widgets/ConstantsWidget.h"
#include "kdefrontend/widgets/FunctionsWidget.h"
#include "kdefrontend/widgets/FitOptionsWidget.h"
#include "kdefrontend/widgets/FitParametersWidget.h"

#include <KMessageWidget>

#include <QMenu>
#include <QWidgetAction>
#include <QStandardItemModel>
#include <QStandardPaths>
#include <QClipboard>

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

XYFitCurveDock::XYFitCurveDock(QWidget* parent) : XYCurveDock(parent) {
}

/*!
 * 	set up "General" tab
 */
void XYFitCurveDock::setupGeneral() {
	DEBUG("XYFitCurveDock::setupGeneral()");
	QWidget* generalTab = new QWidget(ui.tabGeneral);
	uiGeneralTab.setupUi(generalTab);
	m_leName = uiGeneralTab.leName;
	m_teComment = uiGeneralTab.teComment;
	m_teComment->setFixedHeight(m_leName->height());

	auto* gridLayout = static_cast<QGridLayout*>(generalTab->layout());
	gridLayout->setContentsMargins(2, 2, 2, 2);
	gridLayout->setHorizontalSpacing(2);
	gridLayout->setVerticalSpacing(2);

	uiGeneralTab.cbDataSourceType->addItem(i18n("Spreadsheet"));
	uiGeneralTab.cbDataSourceType->addItem(i18n("XY-Curve"));

	cbDataSourceCurve = new TreeViewComboBox(generalTab);
	gridLayout->addWidget(cbDataSourceCurve, 5, 3, 1, 4);

	cbXDataColumn = new TreeViewComboBox(generalTab);
	gridLayout->addWidget(cbXDataColumn, 6, 3, 1, 4);

	cbXErrorColumn = new TreeViewComboBox(generalTab);
	cbXErrorColumn->setEnabled(false);
	uiGeneralTab.hlXError->addWidget(cbXErrorColumn);

	cbYDataColumn = new TreeViewComboBox(generalTab);
	gridLayout->addWidget(cbYDataColumn, 7, 3, 1, 4);

	cbYErrorColumn = new TreeViewComboBox(generalTab);
	cbYErrorColumn->setEnabled(false);
	uiGeneralTab.hlYWeight->addWidget(cbYErrorColumn);

	// X/Y-Weight
	for (int i = 0; i < NSL_FIT_WEIGHT_TYPE_COUNT; i++) {
		uiGeneralTab.cbXWeight->addItem(nsl_fit_weight_type_name[i]);
		uiGeneralTab.cbYWeight->addItem(nsl_fit_weight_type_name[i]);
	}
	uiGeneralTab.cbXWeight->setCurrentIndex(nsl_fit_weight_no);
	uiGeneralTab.cbYWeight->setCurrentIndex(nsl_fit_weight_no);

	for (int i = 0; i < NSL_FIT_MODEL_CATEGORY_COUNT; i++)
		uiGeneralTab.cbCategory->addItem(nsl_fit_model_category_name[i]);

	uiGeneralTab.teEquation->setMaximumHeight(uiGeneralTab.leName->sizeHint().height() * 2);

	fitParametersWidget = new FitParametersWidget(uiGeneralTab.frameParameters);
	auto* l = new QVBoxLayout();
	l->setContentsMargins(0, 0, 0, 0);
	l->addWidget(fitParametersWidget);
	uiGeneralTab.frameParameters->setLayout(l);

	//use white background in the preview label
	QPalette p;
	p.setColor(QPalette::Window, Qt::white);
	uiGeneralTab.lFuncPic->setAutoFillBackground(true);
	uiGeneralTab.lFuncPic->setPalette(p);

	uiGeneralTab.tbConstants->setIcon(QIcon::fromTheme("labplot-format-text-symbol"));
	uiGeneralTab.tbFunctions->setIcon(QIcon::fromTheme("preferences-desktop-font"));
	uiGeneralTab.pbRecalculate->setIcon(QIcon::fromTheme("run-build"));

	// TODO: setting checked background color to unchecked color
//	p = uiGeneralTab.lData->palette();
	// QWidget::palette().color(QWidget::backgroundRole())
	// not working with 'transparent'
//	p.setColor(QPalette::Base, Qt::transparent);
//	uiGeneralTab.lData->setPalette(p);
	// see https://forum.qt.io/topic/41325/solved-background-of-checked-qpushbutton-with-stylesheet/2
	// Styles not usable (here: text color not theme dependent). see https://forum.qt.io/topic/60546/qpushbutton-default-windows-style-sheet/9
//	uiGeneralTab.lData->setStyleSheet("QToolButton:checked{background-color: transparent;border: 3px transparent;padding: 3px;}");

//	uiGeneralTab.lData->setAutoFillBackground(true);

	uiGeneralTab.twLog->setEditTriggers(QAbstractItemView::NoEditTriggers);
	uiGeneralTab.twParameters->setEditTriggers(QAbstractItemView::NoEditTriggers);
	uiGeneralTab.twGoodness->setEditTriggers(QAbstractItemView::NoEditTriggers);

	//don't allow word wrapping in the log-table for the multi-line iterations string
	uiGeneralTab.twLog->setWordWrap(false);

	//header labels
	QStringList headerLabels;
	headerLabels << QString() << i18n("Value") << i18n("Error") << i18n("Error, %") << i18n("t statistic") << QLatin1String("P > |t|")
		<< i18n("Lower") << i18n("Upper");
	uiGeneralTab.twParameters->setHorizontalHeaderLabels(headerLabels);

	// show all options per default
	showDataOptions(true);
	showFitOptions(true);
	showWeightsOptions(true);
	showParameters(true);
	showResults(true);

	//CTRL+C copies only the last cell in the selection, we want to copy the whole selection.
	//install event filters to handle CTRL+C key events.
	uiGeneralTab.twParameters->installEventFilter(this);
	uiGeneralTab.twGoodness->installEventFilter(this);
	uiGeneralTab.twLog->installEventFilter(this);

	// context menus
	uiGeneralTab.twParameters->setContextMenuPolicy(Qt::CustomContextMenu);
	uiGeneralTab.twGoodness->setContextMenuPolicy(Qt::CustomContextMenu);
	uiGeneralTab.twLog->setContextMenuPolicy(Qt::CustomContextMenu);
	connect(uiGeneralTab.twParameters, &QTableWidget::customContextMenuRequested,
			this, &XYFitCurveDock::resultParametersContextMenuRequest);
	connect(uiGeneralTab.twGoodness, &QTableWidget::customContextMenuRequested,
			this, &XYFitCurveDock::resultGoodnessContextMenuRequest);
	connect(uiGeneralTab.twLog, &QTableWidget::customContextMenuRequested,
			this, &XYFitCurveDock::resultLogContextMenuRequest);

	uiGeneralTab.twLog->horizontalHeader()->resizeSections(QHeaderView::ResizeToContents);
	uiGeneralTab.twGoodness->horizontalHeader()->resizeSections(QHeaderView::ResizeToContents);
	// append symbols
	uiGeneralTab.twGoodness->item(0, 0)->setText(uiGeneralTab.twGoodness->item(0, 0)->text() + UTF8_QSTRING(" (χ²)"));
	uiGeneralTab.twGoodness->item(1, 0)->setText(uiGeneralTab.twGoodness->item(1, 0)->text() + UTF8_QSTRING(" (χ²/dof)"));
	uiGeneralTab.twGoodness->item(3, 0)->setText(uiGeneralTab.twGoodness->item(3, 0)->text() + UTF8_QSTRING(" (R²)"));
	uiGeneralTab.twGoodness->item(4, 0)->setText(uiGeneralTab.twGoodness->item(4, 0)->text() + UTF8_QSTRING(" (R̄²)"));
	uiGeneralTab.twGoodness->item(5, 0)->setText(UTF8_QSTRING("χ²-") + i18n("test") + UTF8_QSTRING(" ( P > χ²)"));

	auto* layout = new QHBoxLayout(ui.tabGeneral);
	layout->setMargin(0);
	layout->addWidget(generalTab);

	//Slots
	connect(uiGeneralTab.leName, &QLineEdit::textChanged, this, &XYFitCurveDock::nameChanged);
	connect(uiGeneralTab.teComment, &QTextEdit::textChanged, this, &XYFitCurveDock::commentChanged);
	connect(uiGeneralTab.chkVisible, &QCheckBox::clicked, this, &XYFitCurveDock::visibilityChanged);
	connect(uiGeneralTab.cbDataSourceType, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &XYFitCurveDock::dataSourceTypeChanged);
	connect(uiGeneralTab.lWeights, &QPushButton::clicked, this, &XYFitCurveDock::showWeightsOptions);
	connect(uiGeneralTab.cbXWeight, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &XYFitCurveDock::xWeightChanged);
	connect(uiGeneralTab.cbYWeight, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &XYFitCurveDock::yWeightChanged);
	connect(uiGeneralTab.cbCategory, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &XYFitCurveDock::categoryChanged);
	connect(uiGeneralTab.cbModel, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &XYFitCurveDock::modelTypeChanged);
	connect(uiGeneralTab.sbDegree, QOverload<int>::of(&QSpinBox::valueChanged), this, &XYFitCurveDock::updateModelEquation);
	connect(uiGeneralTab.teEquation, &ExpressionTextEdit::expressionChanged, this, &XYFitCurveDock::expressionChanged);
	connect(uiGeneralTab.tbConstants, &QToolButton::clicked, this, &XYFitCurveDock::showConstants);
	connect(uiGeneralTab.tbFunctions, &QToolButton::clicked, this, &XYFitCurveDock::showFunctions);
	connect(uiGeneralTab.pbOptions, &QPushButton::clicked, this, &XYFitCurveDock::showOptions);
	connect(uiGeneralTab.pbRecalculate, &QPushButton::clicked, this, &XYFitCurveDock::recalculateClicked);
	connect(uiGeneralTab.lData, &QPushButton::clicked, this, &XYFitCurveDock::showDataOptions);
	connect(uiGeneralTab.lFit, &QPushButton::clicked, this, &XYFitCurveDock::showFitOptions);
	connect(uiGeneralTab.lParameters, &QPushButton::clicked, this, &XYFitCurveDock::showParameters);
	connect(uiGeneralTab.lResults, &QPushButton::clicked, this, &XYFitCurveDock::showResults);
	connect(uiGeneralTab.cbPlotRanges, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &XYFitCurveDock::plotRangeChanged );

	connect(cbDataSourceCurve, &TreeViewComboBox::currentModelIndexChanged, this, &XYFitCurveDock::dataSourceCurveChanged);
	connect(cbXDataColumn, &TreeViewComboBox::currentModelIndexChanged, this, &XYFitCurveDock::xDataColumnChanged);
	connect(cbYDataColumn, &TreeViewComboBox::currentModelIndexChanged, this, &XYFitCurveDock::yDataColumnChanged);
	connect(cbXErrorColumn, &TreeViewComboBox::currentModelIndexChanged, this, &XYFitCurveDock::xErrorColumnChanged);
	connect(cbYErrorColumn, &TreeViewComboBox::currentModelIndexChanged, this, &XYFitCurveDock::yErrorColumnChanged);
}

/*
 * load curve settings
 */
void XYFitCurveDock::initGeneralTab() {
	//if there are more then one curve in the list, disable the tab "general"
	if (m_curvesList.size() == 1) {
		uiGeneralTab.lName->setEnabled(true);
		uiGeneralTab.leName->setEnabled(true);
		uiGeneralTab.lComment->setEnabled(true);
		uiGeneralTab.teComment->setEnabled(true);

		uiGeneralTab.leName->setText(m_curve->name());
		uiGeneralTab.teComment->setText(m_curve->comment());
	} else {
		uiGeneralTab.lName->setEnabled(false);
		uiGeneralTab.leName->setEnabled(false);
		uiGeneralTab.lComment->setEnabled(false);
		uiGeneralTab.teComment->setEnabled(false);

		uiGeneralTab.leName->setText(QString());
		uiGeneralTab.teComment->setText(QString());
	}

	uiGeneralTab.cbDataSourceType->setCurrentIndex(static_cast<int>(m_fitCurve->dataSourceType()));
	this->dataSourceTypeChanged(uiGeneralTab.cbDataSourceType->currentIndex());
	cbDataSourceCurve->setAspect(m_fitCurve->dataSourceCurve());
	cbXDataColumn->setColumn(m_fitCurve->xDataColumn(), m_fitCurve->xDataColumnPath());
	cbYDataColumn->setColumn(m_fitCurve->yDataColumn(), m_fitCurve->yDataColumnPath());
	cbXErrorColumn->setColumn(m_fitCurve->xErrorColumn(), m_fitCurve->xErrorColumnPath());
	cbYErrorColumn->setColumn(m_fitCurve->yErrorColumn(), m_fitCurve->yErrorColumnPath());

	int tmpModelType = m_fitData.modelType;	// save type because it's reset when category changes
	if (m_fitData.modelCategory == nsl_fit_model_custom)
		uiGeneralTab.cbCategory->setCurrentIndex(uiGeneralTab.cbCategory->count() - 1);
	else
		uiGeneralTab.cbCategory->setCurrentIndex(m_fitData.modelCategory);
	categoryChanged(m_fitData.modelCategory);	// fill model types

	m_fitData.modelType = tmpModelType;
	if (m_fitData.modelCategory != nsl_fit_model_custom)
		uiGeneralTab.cbModel->setCurrentIndex(m_fitData.modelType);

	uiGeneralTab.cbXWeight->setCurrentIndex(m_fitData.xWeightsType);
	uiGeneralTab.cbYWeight->setCurrentIndex(m_fitData.yWeightsType);
	uiGeneralTab.sbDegree->setValue(m_fitData.degree);

	if (m_fitData.paramStartValues.size() > 0)
		DEBUG(Q_FUNC_INFO << ", start value 1 = " << m_fitData.paramStartValues.at(0));

	DEBUG(Q_FUNC_INFO << ", model degree = " << m_fitData.degree);

	uiGeneralTab.chkVisible->setChecked(m_curve->isVisible());

	//Slots
	connect(m_fitCurve, &XYFitCurve::aspectDescriptionChanged, this, &XYFitCurveDock::curveDescriptionChanged);
	connect(m_fitCurve, &XYFitCurve::dataSourceTypeChanged, this, &XYFitCurveDock::curveDataSourceTypeChanged);
	connect(m_fitCurve, &XYFitCurve::dataSourceCurveChanged, this, &XYFitCurveDock::curveDataSourceCurveChanged);
	connect(m_fitCurve, &XYFitCurve::xDataColumnChanged, this, &XYFitCurveDock::curveXDataColumnChanged);
	connect(m_fitCurve, &XYFitCurve::yDataColumnChanged, this, &XYFitCurveDock::curveYDataColumnChanged);
	connect(m_fitCurve, &XYFitCurve::xErrorColumnChanged, this, &XYFitCurveDock::curveXErrorColumnChanged);
	connect(m_fitCurve, &XYFitCurve::yErrorColumnChanged, this, &XYFitCurveDock::curveYErrorColumnChanged);
	connect(m_fitCurve, &XYFitCurve::fitDataChanged, this, &XYFitCurveDock::curveFitDataChanged);
	connect(m_fitCurve, &XYFitCurve::sourceDataChanged, this, &XYFitCurveDock::enableRecalculate);
	connect(m_fitCurve, &WorksheetElement::plotRangeListChanged, this, &XYFitCurveDock::updatePlotRanges);
	connect(m_fitCurve, QOverload<bool>::of(&XYCurve::visibilityChanged), this, &XYFitCurveDock::curveVisibilityChanged);

	connect(fitParametersWidget, &FitParametersWidget::parametersChanged, this, &XYFitCurveDock::parametersChanged);
	connect(fitParametersWidget, &FitParametersWidget::parametersValid, this, &XYFitCurveDock::parametersValid);
}

void XYFitCurveDock::setModel() {
	QList<AspectType> list{AspectType::Folder, AspectType::Datapicker, AspectType::Worksheet,
							AspectType::CartesianPlot, AspectType::XYCurve, AspectType::XYAnalysisCurve};
	cbDataSourceCurve->setTopLevelClasses(list);

	QList<const AbstractAspect*> hiddenAspects;
	for (auto* curve : m_curvesList)
		hiddenAspects << curve;
	cbDataSourceCurve->setHiddenAspects(hiddenAspects);

	list = {AspectType::Folder, AspectType::Workbook, AspectType::Spreadsheet, AspectType::LiveDataSource,
	        AspectType::Column, AspectType::CantorWorksheet, AspectType::Datapicker};
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
	m_initializing = true;
	m_curvesList = list;
	m_curve = list.first();
	m_aspect = m_curve;
	m_fitCurve = static_cast<XYFitCurve*>(m_curve);
	m_aspectTreeModel = new AspectTreeModel(m_curve->project());
	this->setModel();
	m_fitData = m_fitCurve->fitData();

	DEBUG(Q_FUNC_INFO << ", model type = " << m_fitData.modelType);
	DEBUG(Q_FUNC_INFO << ", model = " << STDSTRING(m_fitData.model));
	DEBUG(Q_FUNC_INFO << ", model degree = " << m_fitData.degree);
	DEBUG(Q_FUNC_INFO << ", # params = " << m_fitData.paramNames.size());
	DEBUG(Q_FUNC_INFO << ", # start values = " << m_fitData.paramStartValues.size());
	//for (auto startValue: m_fitData.paramStartValues)
	//	DEBUG("XYFitCurveDock::setCurves()	start value = " << startValue);

	fitParametersWidget->setFitData(&m_fitData);

	initGeneralTab();
	initTabs();

	if (m_messageWidget && m_messageWidget->isVisible())
		m_messageWidget->close();

	showFitResult();
	enableRecalculate();
	m_initializing = false;

	updatePlotRanges();


	//init parameter list when not available
	if (m_fitData.paramStartValues.size() == 0)
		updateModelEquation();
}

void XYFitCurveDock	::updatePlotRanges() const {
	updatePlotRangeList(uiGeneralTab.cbPlotRanges);
}

bool XYFitCurveDock::eventFilter(QObject* obj, QEvent* event) {
	if (event->type() == QEvent::KeyPress
		&& (obj == uiGeneralTab.twParameters || obj == uiGeneralTab.twGoodness || obj == uiGeneralTab.twLog)) {
		auto* key_event = static_cast<QKeyEvent*>(event);
		if (key_event->matches(QKeySequence::Copy)) {
			resultCopy();
			return true;
		}
	}
	return QWidget::eventFilter(obj, event);
}

//*************************************************************
//**** SLOTs for changes triggered in XYFitCurveDock *****
//*************************************************************
void XYFitCurveDock::dataSourceTypeChanged(int index) {
	const auto type = (XYAnalysisCurve::DataSourceType)index;
	if (type == XYAnalysisCurve::DataSourceType::Spreadsheet) {
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
	if (m_initializing)
		return;

	auto* aspect = static_cast<AbstractAspect*>(index.internalPointer());
	auto* dataSourceCurve = dynamic_cast<XYCurve*>(aspect);

	for (auto* curve : m_curvesList)
		dynamic_cast<XYFitCurve*>(curve)->setDataSourceCurve(dataSourceCurve);
}

void XYFitCurveDock::xDataColumnChanged(const QModelIndex& index) {
	if (m_initializing)
		return;

	auto* aspect = static_cast<AbstractAspect*>(index.internalPointer());
	auto* column = dynamic_cast<AbstractColumn*>(aspect);

	for (auto* curve : m_curvesList)
		dynamic_cast<XYFitCurve*>(curve)->setXDataColumn(column);

	// set model dependent start values from new data
	XYFitCurve::initStartValues(m_fitData, m_curve);

	// update model limits depending on number of points
	modelTypeChanged(uiGeneralTab.cbModel->currentIndex());

	cbXDataColumn->useCurrentIndexText(true);
	cbXDataColumn->setInvalid(false);
}

void XYFitCurveDock::yDataColumnChanged(const QModelIndex& index) {
	if (m_initializing)
		return;

	auto* aspect = static_cast<AbstractAspect*>(index.internalPointer());
	auto* column = dynamic_cast<AbstractColumn*>(aspect);

	for (auto* curve : m_curvesList)
		dynamic_cast<XYFitCurve*>(curve)->setYDataColumn(column);

	// set model dependent start values from new data
	XYFitCurve::initStartValues(m_fitData, m_curve);

	cbYDataColumn->useCurrentIndexText(true);
	cbYDataColumn->setInvalid(false);
}

void XYFitCurveDock::xErrorColumnChanged(const QModelIndex& index) {
	if (m_initializing)
		return;

	auto* aspect = static_cast<AbstractAspect*>(index.internalPointer());
	auto* column = dynamic_cast<AbstractColumn*>(aspect);

	for (auto* curve : m_curvesList)
		dynamic_cast<XYFitCurve*>(curve)->setXErrorColumn(column);

	cbXErrorColumn->useCurrentIndexText(true);
	cbXErrorColumn->setInvalid(false);
}

void XYFitCurveDock::yErrorColumnChanged(const QModelIndex& index) {
	if (m_initializing)
		return;

	auto* aspect = static_cast<AbstractAspect*>(index.internalPointer());
	auto* column = dynamic_cast<AbstractColumn*>(aspect);

	for (auto* curve : m_curvesList)
		dynamic_cast<XYFitCurve*>(curve)->setYErrorColumn(column);

	cbYErrorColumn->useCurrentIndexText(true);
	cbYErrorColumn->setInvalid(false);
}

///////////////////////// fold/unfold options //////////////////////////////////////////////////

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

		m_initializing = true;	// do not change start parameter
		modelTypeChanged(uiGeneralTab.cbModel->currentIndex());
		m_initializing = false;
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

void XYFitCurveDock::showParameters(bool checked) {
	if (checked) {
		uiGeneralTab.lParameters->setIcon(QIcon::fromTheme("arrow-down"));
		uiGeneralTab.frameParameters->show();
	} else {
		uiGeneralTab.lParameters->setIcon(QIcon::fromTheme("arrow-right"));
		uiGeneralTab.frameParameters->hide();
	}
}

void XYFitCurveDock::showResults(bool checked) {
	if (checked) {
		uiGeneralTab.lResults->setIcon(QIcon::fromTheme("arrow-down"));
		uiGeneralTab.twResults->show();
	} else {
		uiGeneralTab.lResults->setIcon(QIcon::fromTheme("arrow-right"));
		uiGeneralTab.twResults->hide();
	}
}

///////////////////////////////////////////////////////////////////////////

void XYFitCurveDock::xWeightChanged(int index) {
	DEBUG(Q_FUNC_INFO << ", weight = " << nsl_fit_weight_type_name[index]);

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
	DEBUG(Q_FUNC_INFO << ", weight = " << nsl_fit_weight_type_name[index]);

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
		DEBUG(Q_FUNC_INFO << ", category = \"nsl_fit_model_custom\"");
	} else {
		DEBUG(Q_FUNC_INFO << ", category = \"" << nsl_fit_model_category_name[index] << "\"");
	}

	bool hasChanged = true;
	// nothing has changed when ...
	if (m_fitData.modelCategory == (nsl_fit_model_category)index || (m_fitData.modelCategory == nsl_fit_model_custom && index == uiGeneralTab.cbCategory->count() - 1) )
		hasChanged = false;

	if (uiGeneralTab.cbCategory->currentIndex() == uiGeneralTab.cbCategory->count() - 1)
		m_fitData.modelCategory = nsl_fit_model_custom;
	else
		m_fitData.modelCategory = (nsl_fit_model_category)index;

	uiGeneralTab.cbModel->clear();
	uiGeneralTab.cbModel->show();
	uiGeneralTab.lModel->show();

	switch (m_fitData.modelCategory) {
	case nsl_fit_model_basic:
		for (int i = 0; i < NSL_FIT_MODEL_BASIC_COUNT; i++)
			uiGeneralTab.cbModel->addItem(nsl_fit_model_basic_name[i]);
		break;
	case nsl_fit_model_peak: {
		for (int i = 0; i < NSL_FIT_MODEL_PEAK_COUNT; i++)
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
		for (int i = 0; i < NSL_FIT_MODEL_GROWTH_COUNT; i++)
			uiGeneralTab.cbModel->addItem(nsl_fit_model_growth_name[i]);
		break;
	case nsl_fit_model_distribution: {
		for (int i = 0; i < NSL_SF_STATS_DISTRIBUTION_COUNT; i++)
			uiGeneralTab.cbModel->addItem(nsl_sf_stats_distribution_name[i]);

		// not-used items are disabled here
		const auto* model = qobject_cast<const QStandardItemModel*>(uiGeneralTab.cbModel->model());

		for (int i = 1; i < NSL_SF_STATS_DISTRIBUTION_COUNT; i++) {
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
		// when model type does not change, call it here
		updateModelEquation();
	}

	enableRecalculate();
}

/*!
 * called when the fit model type (depends on category) was changed.
 * Updates the model type dependent widgets in the general-tab and calls \c updateModelEquation() to update the preview pixmap.
 */
void XYFitCurveDock::modelTypeChanged(int index) {
	DEBUG("modelTypeChanged() type = " << (unsigned int)index << ", initializing = " << m_initializing << ", current type = " << m_fitData.modelType);
	// leave if there is no selection
	if (index == -1)
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

	const AbstractColumn* xColumn = nullptr;
	if (m_fitCurve->dataSourceType() == XYAnalysisCurve::DataSourceType::Spreadsheet) {
		DEBUG("	data source: Spreadsheet")
		//auto* aspect = static_cast<AbstractAspect*>(cbXDataColumn->currentModelIndex().internalPointer());
		//xColumn = dynamic_cast<AbstractColumn*>(aspect);
		xColumn = m_fitCurve->xDataColumn();
	} else {
		DEBUG("	data source: Curve")
		if (m_fitCurve->dataSourceCurve())
			xColumn = m_fitCurve->dataSourceCurve()->xColumn();
	}
	// with no xColumn: show all models (assume 100 data points)
	const int availableRowCount = xColumn ? xColumn->availableRowCount() : 100;
	DEBUG("	available row count = " << availableRowCount)

	bool disableFit = false;
	switch (m_fitData.modelCategory) {
	case nsl_fit_model_basic:
		switch (index) {
		case nsl_fit_model_polynomial:
			uiGeneralTab.lDegree->setVisible(true);
			uiGeneralTab.sbDegree->setVisible(true);
			uiGeneralTab.sbDegree->setMaximum(qMin(availableRowCount - 1, 10));
			break;
		case nsl_fit_model_fourier:
			if (availableRowCount < 4) {	// too few data points
				uiGeneralTab.lDegree->setVisible(false);
				uiGeneralTab.sbDegree->setVisible(false);
				disableFit = true;
			} else {
				uiGeneralTab.lDegree->setVisible(true);
				uiGeneralTab.sbDegree->setVisible(true);
				uiGeneralTab.sbDegree->setMaximum(qMin(availableRowCount/2 - 1, 10));
			}
			break;
		case nsl_fit_model_power:
			uiGeneralTab.lDegree->setVisible(true);
			uiGeneralTab.sbDegree->setVisible(true);
			uiGeneralTab.sbDegree->setMaximum(2);
			//TODO: limit degree depending on availableRowCount
			break;
		case nsl_fit_model_exponential:
			uiGeneralTab.lDegree->setVisible(true);
			uiGeneralTab.sbDegree->setVisible(true);
			uiGeneralTab.sbDegree->setMaximum(10);
			//TODO: limit degree depending on availableRowCount
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

	m_fitData.modelType = index;

	updateModelEquation();

	if (disableFit)
		uiGeneralTab.pbRecalculate->setEnabled(false);
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
	if (!m_initializing) {
		m_fitData.degree = degree;
		XYFitCurve::initFitData(m_fitData);
		// set model dependent start values from curve data
		XYFitCurve::initStartValues(m_fitData, m_curve);
		// udpate parameter widget
		fitParametersWidget->setFitData(&m_fitData);
	}

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
		DEBUG("Model pixmap path = " << STDSTRING(file));
		uiGeneralTab.lFuncPic->setPixmap(file);
		uiGeneralTab.lFuncPic->show();
		uiGeneralTab.teEquation->hide();
	}

	enableRecalculate();
}

void XYFitCurveDock::showConstants() {
	QMenu menu;
	ConstantsWidget constants(&menu);

	connect(&constants, &ConstantsWidget::constantSelected, this, &XYFitCurveDock::insertConstant);
	connect(&constants, &ConstantsWidget::constantSelected, &menu, &QMenu::close);
	connect(&constants, &ConstantsWidget::canceled, &menu, &QMenu::close);

	auto* widgetAction = new QWidgetAction(this);
	widgetAction->setDefaultWidget(&constants);
	menu.addAction(widgetAction);

	QPoint pos(-menu.sizeHint().width() + uiGeneralTab.tbConstants->width(), -menu.sizeHint().height());
	menu.exec(uiGeneralTab.tbConstants->mapToGlobal(pos));
}

void XYFitCurveDock::showFunctions() {
	QMenu menu;
	FunctionsWidget functions(&menu);
	connect(&functions, &FunctionsWidget::functionSelected, this, &XYFitCurveDock::insertFunction);
	connect(&functions, &FunctionsWidget::functionSelected, &menu, &QMenu::close);
	connect(&functions, &FunctionsWidget::canceled, &menu, &QMenu::close);

	auto* widgetAction = new QWidgetAction(this);
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
	vars << "x";	//TODO: generalize when we support other XYEquationCurve::EquationType
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
 * called when parameter names and/or start values for the model were changed
 * also called from parameter widget
 */
void XYFitCurveDock::parametersChanged(bool updateParameterWidget) {
	DEBUG("XYFitCurveDock::parametersChanged() m_initializing = " << m_initializing);

	//parameter names were (probably) changed -> set the new vars in ExpressionTextEdit teEquation
	QStringList vars{m_fitData.paramNames};
	vars << "x";	//TODO: generalize when we support other XYEquationCurve::EquationType
	uiGeneralTab.teEquation->setVariables(vars);

	if (m_initializing)
		return;

	if (updateParameterWidget)
		fitParametersWidget->setFitData(&m_fitData);

	enableRecalculate();
}
void XYFitCurveDock::parametersValid(bool valid) {
	DEBUG("XYFitCurveDock::parametersValid() valid = " << valid);
	m_parametersValid = valid;
}

void XYFitCurveDock::showOptions() {
	QMenu menu;
	FitOptionsWidget w(&menu, &m_fitData, m_fitCurve);
	connect(&w, &FitOptionsWidget::finished, &menu, &QMenu::close);
	connect(&w, &FitOptionsWidget::optionsChanged, this, &XYFitCurveDock::enableRecalculate);

	auto* widgetAction = new QWidgetAction(this);
	widgetAction->setDefaultWidget(&w);
	menu.addAction(widgetAction);
	menu.setTearOffEnabled(true);

	//menu.setWindowFlags(menu.windowFlags() & Qt::MSWindowsFixedSizeDialogHint);

	QPoint pos(-menu.sizeHint().width() + uiGeneralTab.pbOptions->width(), 0);
	menu.exec(uiGeneralTab.pbOptions->mapToGlobal(pos));
}

void XYFitCurveDock::insertFunction(const QString& functionName) const {
	uiGeneralTab.teEquation->insertPlainText(functionName + ExpressionParser::functionArgumentString(functionName, XYEquationCurve::EquationType::Cartesian));
}

void XYFitCurveDock::insertConstant(const QString& constantsName) const {
	uiGeneralTab.teEquation->insertPlainText(constantsName);
}

/*!
 * When a custom evaluate range is specified, set the plot range too.
 */
void XYFitCurveDock::setPlotXRange() {
	if (m_fitData.autoEvalRange || !m_curve)
		return;

	auto* plot = dynamic_cast<CartesianPlot*>(m_curve->parentAspect());
	if (plot) {
		Range<double> range{ m_fitData.evalRange };
		if (!range.isZero()) {
			range.extend(range.size() * 0.05);	// 5%
			plot->setXRange(range);
		}
	}
}

void XYFitCurveDock::recalculateClicked() {
	DEBUG("XYFitCurveDock::recalculateClicked()");
	QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
	m_fitData.degree = uiGeneralTab.sbDegree->value();
	if (m_fitData.modelCategory == nsl_fit_model_custom)
		updateParameterList();

	for (XYCurve* curve: m_curvesList)
		dynamic_cast<XYFitCurve*>(curve)->setFitData(m_fitData);

	m_fitCurve->recalculate();
	setPlotXRange();

	//update fitParametersWidget
	if (m_fitData.useResults) {
		DEBUG(" nr of param names = " << m_fitData.paramNames.size())
		DEBUG("	size of start values = " << m_fitData.paramStartValues.size())
		DEBUG("	size of param values = " << m_fitCurve->fitResult().paramValues.size())
		if (m_fitCurve->fitResult().paramValues.size() > 0) {	// may be 0 if fit fails
			for (int i = 0; i < m_fitData.paramNames.size(); i++)
				m_fitData.paramStartValues[i] = m_fitCurve->fitResult().paramValues.at(i);
			fitParametersWidget->setFitData(&m_fitData);
		} else {
			DEBUG(" WARNING: no fit result available!")
		}
	}

	this->showFitResult();
	uiGeneralTab.pbRecalculate->setEnabled(false);

	//show the warning/error message, if available
	const XYFitCurve::FitResult& fitResult = m_fitCurve->fitResult();
	const QString& status = fitResult.status;
	if (status != i18n("Success")) {
		emit info(i18n("Fit status: %1", fitResult.status));
		if (!m_messageWidget) {
			m_messageWidget = new KMessageWidget(this);
			uiGeneralTab.gridLayout_2->addWidget(m_messageWidget, 25, 3, 1, 4);
		}

		if (!fitResult.valid)
			m_messageWidget->setMessageType(KMessageWidget::Error);
		else
			m_messageWidget->setMessageType(KMessageWidget::Warning);
		m_messageWidget->setText(status);
        m_messageWidget->animatedShow();
	} else {
		if (m_messageWidget && m_messageWidget->isVisible())
			m_messageWidget->close();
	}

	QApplication::restoreOverrideCursor();
	DEBUG("XYFitCurveDock::recalculateClicked() DONE");
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

void XYFitCurveDock::enableRecalculate() {
	DEBUG("XYFitCurveDock::enableRecalculate()");
	if (m_initializing || !m_fitCurve)
		return;

	//no fitting possible without the x- and y-data
	bool hasSourceData = false;
	if (m_fitCurve->dataSourceType() == XYAnalysisCurve::DataSourceType::Spreadsheet) {
		auto* aspectX = static_cast<AbstractAspect*>(cbXDataColumn->currentModelIndex().internalPointer());
		auto* aspectY = static_cast<AbstractAspect*>(cbYDataColumn->currentModelIndex().internalPointer());
		hasSourceData = (aspectX && aspectY);
		if (aspectX) {
			cbXDataColumn->useCurrentIndexText(true);
			cbXDataColumn->setInvalid(false);
		}
		if (aspectY) {
			cbYDataColumn->useCurrentIndexText(true);
			cbYDataColumn->setInvalid(false);
		}
	} else {
		hasSourceData = (m_fitCurve->dataSourceCurve() != nullptr);
	}

	uiGeneralTab.pbRecalculate->setEnabled(hasSourceData && m_parametersValid);

	// PREVIEW as soon as recalculate is enabled (does not need source data)
	if (m_parametersValid && m_fitData.previewEnabled) {
		DEBUG("	EVALUATE WITH PREVIEW ENABLED");
		// use recent fit data
		m_fitCurve->setFitData(m_fitData);
		// calculate fit function
		m_fitCurve->evaluate(true);
		setPlotXRange();
	}
	else {
		DEBUG("	PREVIEW DISABLED");
	}
	DEBUG("XYFitCurveDock::enableRecalculate() DONE");
}

void XYFitCurveDock::resultCopy(bool copyAll) {
	QTableWidget* tw{nullptr};
	int currentTab = uiGeneralTab.twResults->currentIndex();
	if (currentTab == 0)
		tw = uiGeneralTab.twParameters;
	else if (currentTab == 1)
		tw = uiGeneralTab.twGoodness;
	else if (currentTab == 2)
		tw = uiGeneralTab.twLog;
	else
		return;

	QString str;
	QString rowStr;

	//copy the header of the parameters table if we copy everything in this table
	if (copyAll && tw == uiGeneralTab.twParameters) {
		for (int i = 1; i < tw->columnCount(); ++i)
			str += QLatin1Char('\t') + tw->horizontalHeaderItem(i)->text();
	}

	//copy the content of the table
	for (int i = 0; i < tw->rowCount(); ++i) {
		for (int j = 0; j < tw->columnCount(); ++j) {
			if (!tw->item(i, j))
				continue;
			if (!copyAll && !tw->item(i, j)->isSelected())
				continue;

			if (!rowStr.isEmpty())
				rowStr += QLatin1Char('\t');

			rowStr += tw->item(i, j)->text();
		}
		if (!rowStr.isEmpty()) {
			if (!str.isEmpty())
				str += QLatin1Char('\n');
			str += rowStr;
			rowStr.clear();
		}
	}

	QApplication::clipboard()->setText(str);
	DEBUG(STDSTRING(QApplication::clipboard()->text()));
}

void XYFitCurveDock::resultCopyAll() {
	resultCopy(true);
}

void XYFitCurveDock::resultParametersContextMenuRequest(QPoint pos) {
	auto* contextMenu = new QMenu;
	contextMenu->addAction(i18n("Copy Selection"), this, &XYFitCurveDock::resultCopy, QKeySequence::Copy);
	contextMenu->addAction(i18n("Copy All"), this, &XYFitCurveDock::resultCopyAll);
	contextMenu->exec(uiGeneralTab.twParameters->mapToGlobal(pos));
}
void XYFitCurveDock::resultGoodnessContextMenuRequest(QPoint pos) {
	auto* contextMenu = new QMenu;
	contextMenu->addAction(i18n("Copy Selection"), this, &XYFitCurveDock::resultCopy, QKeySequence::Copy);
	contextMenu->addAction(i18n("Copy All"), this, &XYFitCurveDock::resultCopyAll);
	contextMenu->exec(uiGeneralTab.twGoodness->mapToGlobal(pos));
}
void XYFitCurveDock::resultLogContextMenuRequest(QPoint pos) {
	auto* contextMenu = new QMenu;
	contextMenu->addAction(i18n("Copy Selection"), this, &XYFitCurveDock::resultCopy, QKeySequence::Copy);
	contextMenu->addAction(i18n("Copy All"), this, &XYFitCurveDock::resultCopyAll);
	contextMenu->exec(uiGeneralTab.twLog->mapToGlobal(pos));
}

/*!
 * show the result and details of the fit
 */
void XYFitCurveDock::showFitResult() {
	//clear the previous result
	uiGeneralTab.twParameters->setRowCount(0);
	for (int row = 0; row < uiGeneralTab.twGoodness->rowCount(); ++row)
		uiGeneralTab.twGoodness->item(row, 1)->setText(QString());
	for (int row = 0; row < uiGeneralTab.twLog->rowCount(); ++row)
		uiGeneralTab.twLog->item(row, 1)->setText(QString());

	const auto& fitResult = m_fitCurve->fitResult();

	if (!fitResult.available) {
		DEBUG(Q_FUNC_INFO << ", fit result not available");
		return;
	}

	// Log
	uiGeneralTab.twLog->item(0, 1)->setText(fitResult.status);

	if (!fitResult.valid) {
		DEBUG(Q_FUNC_INFO << ", fit result not valid");
		return;
	}

	SET_NUMBER_LOCALE
	// used confidence interval
	double confidenceInterval{m_fitData.confidenceInterval};
	uiGeneralTab.twParameters->horizontalHeaderItem(6)->setToolTip(i18n("%1 % lower confidence level", numberLocale.toString(confidenceInterval, 'g', 7)));
	uiGeneralTab.twParameters->horizontalHeaderItem(7)->setToolTip(i18n("%1 % upper confidence level", numberLocale.toString(confidenceInterval, 'g', 7)));

	// log
	uiGeneralTab.twLog->item(1, 1)->setText(numberLocale.toString(fitResult.iterations));
	uiGeneralTab.twLog->item(2, 1)->setText(numberLocale.toString(m_fitData.eps));
	if (fitResult.elapsedTime > 1000)
		uiGeneralTab.twLog->item(3, 1)->setText(numberLocale.toString(fitResult.elapsedTime/1000) + " s");
	else
		uiGeneralTab.twLog->item(3, 1)->setText(numberLocale.toString(fitResult.elapsedTime) + " ms");

	uiGeneralTab.twLog->item(4, 1)->setText(numberLocale.toString(fitResult.dof));
	uiGeneralTab.twLog->item(5, 1)->setText(numberLocale.toString(fitResult.paramValues.size()));
	uiGeneralTab.twLog->item(6, 1)->setText(m_fitData.fitRange.toString());

	const int np = m_fitData.paramNames.size();

	// correlation matrix
	QString sCorr;
	for (const auto &s : m_fitData.paramNamesUtf8)
		sCorr += '\t' + s;
	int index{0};
	DEBUG(Q_FUNC_INFO << ", correlation values size = " << fitResult.correlationMatrix.size())
	for (int i = 0; i < np; i++) {
		sCorr += '\n' + m_fitData.paramNamesUtf8.at(i);
		for (int j = 0; j <= i; j++)
			sCorr += '\t' + numberLocale.toString(fitResult.correlationMatrix.at(index++), 'f');
	}
	uiGeneralTab.twLog->item(7, 1)->setText(sCorr);

	// iterations
	QString sIter;
	for (const auto &s : m_fitData.paramNamesUtf8)
		sIter += s + '\t';
	sIter += UTF8_QSTRING("χ²");

	const QStringList iterations = fitResult.solverOutput.split(';');
	for (const auto &s : iterations)
		if (!s.isEmpty())
			sIter += '\n' + s;
	uiGeneralTab.twLog->item(8, 1)->setText(sIter);
	uiGeneralTab.twLog->resizeRowsToContents();

	// Parameters
	uiGeneralTab.twParameters->setRowCount(np);

	for (int i = 0; i < np; i++) {
		const double paramValue = fitResult.paramValues.at(i);
		const double errorValue = fitResult.errorValues.at(i);

		auto* item = new QTableWidgetItem(m_fitData.paramNamesUtf8.at(i));
		item->setBackground(QApplication::palette().color(QPalette::Window));
		uiGeneralTab.twParameters->setItem(i, 0, item);
		item = new QTableWidgetItem(numberLocale.toString(paramValue));
		uiGeneralTab.twParameters->setItem(i, 1, item);

		if (!m_fitData.paramFixed.at(i)) {
			if (!std::isnan(errorValue)) {
				item = new QTableWidgetItem(numberLocale.toString(errorValue));
				uiGeneralTab.twParameters->setItem(i, 2, item);
				item = new QTableWidgetItem(numberLocale.toString(100.*errorValue/std::abs(paramValue), 'g', 3));
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
				tdistValueString = numberLocale.toString(fitResult.tdist_tValues.at(i), 'g', 3);
			else
				tdistValueString = UTF8_QSTRING("∞");
			item = new QTableWidgetItem(tdistValueString);
			uiGeneralTab.twParameters->setItem(i, 4, item);

			// p values
			const double p = fitResult.tdist_pValues.at(i);
			item = new QTableWidgetItem(numberLocale.toString(p, 'g', 3));
			// color p values depending on value
			if (p > 0.05)
				item->setForeground(QBrush(QApplication::palette().color(QPalette::LinkVisited)));
			else if (p > 0.01)
				item->setForeground(QBrush(Qt::darkGreen));
			else if (p > 0.001)
				item->setForeground(QBrush(Qt::darkCyan));
			else if (p > 0.0001)
				item->setForeground(QBrush(QApplication::palette().color(QPalette::Link)));
			else
				item->setForeground(QBrush(QApplication::palette().color(QPalette::Highlight)));
			uiGeneralTab.twParameters->setItem(i, 5, item);

			// Conf. interval
			if (!std::isnan(errorValue)) {
				const double margin = fitResult.tdist_marginValues.at(i);
				//TODO: if (fitResult.tdist_tValues.at(i) > 1.e6)
				//	item = new QTableWidgetItem(i18n("too small"));

				item = new QTableWidgetItem(numberLocale.toString(paramValue - margin));
				uiGeneralTab.twParameters->setItem(i, 6, item);
				item = new QTableWidgetItem(numberLocale.toString(paramValue + margin));
				uiGeneralTab.twParameters->setItem(i, 7, item);
			}
		}
	}

	// Goodness of fit
	uiGeneralTab.twGoodness->item(0, 1)->setText(numberLocale.toString(fitResult.sse));

	if (fitResult.dof != 0) {
		uiGeneralTab.twGoodness->item(1, 1)->setText(numberLocale.toString(fitResult.rms));
		uiGeneralTab.twGoodness->item(2, 1)->setText(numberLocale.toString(fitResult.rsd));

		uiGeneralTab.twGoodness->item(3, 1)->setText(numberLocale.toString(fitResult.rsquare));
		uiGeneralTab.twGoodness->item(4, 1)->setText(numberLocale.toString(fitResult.rsquareAdj));

		// chi^2 and F test p-values
		uiGeneralTab.twGoodness->item(5, 1)->setText(numberLocale.toString(fitResult.chisq_p, 'g', 3));
		uiGeneralTab.twGoodness->item(6, 1)->setText(numberLocale.toString(fitResult.fdist_F, 'g', 3));
		uiGeneralTab.twGoodness->item(7, 1)->setText(numberLocale.toString(fitResult.fdist_p, 'g', 3));
		uiGeneralTab.twGoodness->item(9, 1)->setText(numberLocale.toString(fitResult.aic, 'g', 3));
		uiGeneralTab.twGoodness->item(10, 1)->setText(numberLocale.toString(fitResult.bic, 'g', 3));
	}

	uiGeneralTab.twGoodness->item(8, 1)->setText(numberLocale.toString(fitResult.mae));

	//resize the table headers to fit the new content
	uiGeneralTab.twLog->resizeColumnsToContents();
	uiGeneralTab.twParameters->resizeColumnsToContents();
	//twGoodness doesn't have any header -> resize sections
	uiGeneralTab.twGoodness->resizeColumnToContents(0);
	uiGeneralTab.twGoodness->resizeColumnToContents(1);

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
	else if (aspect->comment() != uiGeneralTab.teComment->text())
		uiGeneralTab.teComment->setText(aspect->comment());
	m_initializing = false;
}

void XYFitCurveDock::curveDataSourceTypeChanged(XYAnalysisCurve::DataSourceType type) {
	m_initializing = true;
	uiGeneralTab.cbDataSourceType->setCurrentIndex(static_cast<int>(type));
	m_initializing = false;
}

void XYFitCurveDock::curveDataSourceCurveChanged(const XYCurve* curve) {
	m_initializing = true;
	cbDataSourceCurve->setAspect(curve);
	m_initializing = false;
}

void XYFitCurveDock::curveXDataColumnChanged(const AbstractColumn* column) {
	m_initializing = true;
	cbXDataColumn->setColumn(column, m_fitCurve->xDataColumnPath());
	m_initializing = false;
}

void XYFitCurveDock::curveYDataColumnChanged(const AbstractColumn* column) {
	m_initializing = true;
	cbYDataColumn->setColumn(column, m_fitCurve->yDataColumnPath());
	m_initializing = false;
}

void XYFitCurveDock::curveXErrorColumnChanged(const AbstractColumn* column) {
	m_initializing = true;
	cbXErrorColumn->setColumn(column, m_fitCurve->xErrorColumnPath());
	m_initializing = false;
}

void XYFitCurveDock::curveYErrorColumnChanged(const AbstractColumn* column) {
	m_initializing = true;
	cbYErrorColumn->setColumn(column, m_fitCurve->yErrorColumnPath());
	m_initializing = false;
}

/*!
 * called when fit data of fit curve changes
 */
void XYFitCurveDock::curveFitDataChanged(const XYFitCurve::FitData& fitData) {
	m_initializing = true;
	m_fitData = fitData;

	if (m_fitData.modelCategory != nsl_fit_model_custom)
		uiGeneralTab.cbModel->setCurrentIndex(m_fitData.modelType);

	uiGeneralTab.sbDegree->setValue(m_fitData.degree);
	m_initializing = false;
}

void XYFitCurveDock::dataChanged() {
	this->enableRecalculate();
}

void XYFitCurveDock::curveVisibilityChanged(bool on) {
	m_initializing = true;
	uiGeneralTab.chkVisible->setChecked(on);
	m_initializing = false;
}
