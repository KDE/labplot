/*
	File             : XYFitCurveDock.cpp
	Project          : LabPlot
	Description      : widget for editing properties of fit curves
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2014-2025 Alexander Semke <alexander.semke@web.de>
	SPDX-FileCopyrightText: 2016-2025 Stefan Gerlach <stefan.gerlach@uni.kn>

	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "XYFitCurveDock.h"
#include "backend/core/Project.h"
#include "backend/gsl/ExpressionParser.h"
#include "backend/worksheet/plots/cartesian/CartesianPlot.h"
#include "backend/worksheet/plots/cartesian/Histogram.h"
#include "frontend/GuiTools.h"
#include "frontend/widgets/ConstantsWidget.h"
#include "frontend/widgets/FitOptionsWidget.h"
#include "frontend/widgets/FitParametersWidget.h"
#include "frontend/widgets/FunctionsWidget.h"
#include "frontend/widgets/TreeViewComboBox.h"
#include "backend/core/Settings.h"

#include "backend/nsl/nsl_sf_stats.h"

#include <KFileWidget>
#include <KConfig>
#include <KConfigGroup>
#include <KLineEdit>
#include <KMessageWidget>
#include <KUrlComboBox>
#include <KPreviewWidgetBase>

#include <QClipboard>
#include <QFileDialog>
#include <QDialog>
#include <QDialogButtonBox>
#include <QPlainTextEdit>
#include <QMenu>
#include <QStandardItemModel>
#include <QStandardPaths>
#include <QWidgetAction>

/*!
  \class XYFitCurveDock
  \brief  Provides a widget for editing the properties of the XYFitCurves
		(2D-curves defined by a fit model) currently selected in
		the project explorer.

  If more than one curves are set, the properties of the first column are shown.
  The changes of the properties are applied to all curves.
  The exclusions are the name, the comment and the datasets (columns) of
  the curves  - these properties can only be changed if there is only one single curve.

  \ingroup frontend
*/

XYFitCurveDock::XYFitCurveDock(QWidget* parent)
	: XYCurveDock(parent) {
}

XYFitCurveDock::~XYFitCurveDock() {
	delete m_dataSourceModel;
}

/*!
 * 	set up "General" tab
 */
void XYFitCurveDock::setupGeneral() {
	auto* generalTab = new QWidget(ui.tabGeneral);
	uiGeneralTab.setupUi(generalTab);
	setPlotRangeCombobox(uiGeneralTab.cbPlotRanges);
	setBaseWidgets(uiGeneralTab.leName, uiGeneralTab.teComment);
	setVisibilityWidgets(uiGeneralTab.chkVisible, uiGeneralTab.chkLegendVisible);

	auto* gridLayout = static_cast<QGridLayout*>(generalTab->layout());
	gridLayout->setContentsMargins(2, 2, 2, 2);
	gridLayout->setHorizontalSpacing(2);
	gridLayout->setVerticalSpacing(2);

	cbDataSourceCurve = new TreeViewComboBox(generalTab);
	gridLayout->addWidget(cbDataSourceCurve, 5, 2, 1, 2);

	cbXDataColumn = new TreeViewComboBox(generalTab);
	gridLayout->addWidget(cbXDataColumn, 6, 2, 1, 2);

	cbXErrorColumn = new TreeViewComboBox(generalTab);
	cbXErrorColumn->setEnabled(false);
	uiGeneralTab.hlXError->addWidget(cbXErrorColumn);

	cbYDataColumn = new TreeViewComboBox(generalTab);
	gridLayout->addWidget(cbYDataColumn, 7, 2, 1, 2);

	cbYErrorColumn = new TreeViewComboBox(generalTab);
	cbYErrorColumn->setEnabled(false);
	uiGeneralTab.hlYWeight->addWidget(cbYErrorColumn);

	// X/Y-Weight
	for (int i = 0; i < NSL_FIT_WEIGHT_TYPE_COUNT; i++) {
		uiGeneralTab.cbXWeight->addItem(QLatin1String(nsl_fit_weight_type_name[i]));
		uiGeneralTab.cbYWeight->addItem(QLatin1String(nsl_fit_weight_type_name[i]));
	}
	uiGeneralTab.cbXWeight->setCurrentIndex(nsl_fit_weight_no);
	uiGeneralTab.cbYWeight->setCurrentIndex(nsl_fit_weight_no);

	for (int i = 0; i < NSL_FIT_MODEL_CATEGORY_COUNT; i++)
		uiGeneralTab.cbCategory->addItem(QLatin1String(nsl_fit_model_category_name[i]));

	uiGeneralTab.teEquation->setMaximumHeight(uiGeneralTab.leName->sizeHint().height() * 2);

	fitParametersWidget = new FitParametersWidget(uiGeneralTab.frameParameters);
	auto* l = new QVBoxLayout();
	l->setContentsMargins(0, 0, 0, 0);
	l->addWidget(fitParametersWidget);
	uiGeneralTab.frameParameters->setLayout(l);

	uiGeneralTab.tbConstants->setIcon(QIcon::fromTheme(QStringLiteral("labplot-format-text-symbol")));
	uiGeneralTab.tbFunctions->setIcon(QIcon::fromTheme(QStringLiteral("preferences-desktop-font")));
	uiGeneralTab.pbRecalculate->setIcon(QIcon::fromTheme(QStringLiteral("run-build")));

	for (int i = 0; i < NSL_FIT_ALGORITHM_COUNT; i++)
		uiGeneralTab.cbAlgorithm->addItem(QLatin1String(nsl_fit_algorithm_name[i]));

	if (m_fitData.modelCategory != nsl_fit_model_distribution) { // disable ML
		const auto* model = qobject_cast<const QStandardItemModel*>(uiGeneralTab.cbAlgorithm->model());
		auto* item = model->item(nsl_fit_algorithm_ml);
		item->setFlags(item->flags() & ~(Qt::ItemIsSelectable | Qt::ItemIsEnabled));
	}

	// TODO: setting checked background color to unchecked color
	//	p = uiGeneralTab.tbData->palette();
	// QWidget::palette().color(QWidget::backgroundRole())
	// not working with 'transparent'
	//	p.setColor(QPalette::Base, Qt::transparent);
	//	uiGeneralTab.tbData->setPalette(p);
	// see https://forum.qt.io/topic/41325/solved-background-of-checked-qpushbutton-with-stylesheet/2
	// Styles not usable (here: text color not theme dependent). see https://forum.qt.io/topic/60546/qpushbutton-default-windows-style-sheet/9
	//	uiGeneralTab.tbData->setStyleSheet(QStringLiteral("QToolButton:checked{background-color: transparent;border: 3px transparent;padding: 3px;}"));

	//	uiGeneralTab.tbData->setAutoFillBackground(true);

	uiGeneralTab.twLog->setEditTriggers(QAbstractItemView::NoEditTriggers);
	uiGeneralTab.twParameters->setEditTriggers(QAbstractItemView::NoEditTriggers);
	uiGeneralTab.twGoodness->setEditTriggers(QAbstractItemView::NoEditTriggers);

	// don't allow word wrapping in the log-table for the multi-line iterations string
	uiGeneralTab.twLog->setWordWrap(false);

	// show all options per default
	showDataOptions(true);
	showFitOptions(true);
	showWeightsOptions(true);
	showParameters(true);
	showResults(true);

	// CTRL+C copies only the last cell in the selection, we want to copy the whole selection.
	// install event filters to handle CTRL+C key events.
	uiGeneralTab.twParameters->installEventFilter(this);
	uiGeneralTab.twGoodness->installEventFilter(this);
	uiGeneralTab.twLog->installEventFilter(this);

	// context menus
	uiGeneralTab.twParameters->setContextMenuPolicy(Qt::CustomContextMenu);
	uiGeneralTab.twGoodness->setContextMenuPolicy(Qt::CustomContextMenu);
	uiGeneralTab.twLog->setContextMenuPolicy(Qt::CustomContextMenu);
	connect(uiGeneralTab.twParameters, &QTableWidget::customContextMenuRequested, this, &XYFitCurveDock::resultParametersContextMenuRequest);
	connect(uiGeneralTab.twGoodness, &QTableWidget::customContextMenuRequested, this, &XYFitCurveDock::resultGoodnessContextMenuRequest);
	connect(uiGeneralTab.twLog, &QTableWidget::customContextMenuRequested, this, &XYFitCurveDock::resultLogContextMenuRequest);

	uiGeneralTab.twLog->horizontalHeader()->resizeSections(QHeaderView::ResizeToContents);
	uiGeneralTab.twGoodness->horizontalHeader()->resizeSections(QHeaderView::ResizeToContents);
	// append symbols
	uiGeneralTab.twGoodness->item(0, 0)->setText(uiGeneralTab.twGoodness->item(0, 0)->text() + UTF8_QSTRING(" (χ²)"));
	uiGeneralTab.twGoodness->item(1, 0)->setText(uiGeneralTab.twGoodness->item(1, 0)->text() + UTF8_QSTRING(" (χ²/dof)"));
	uiGeneralTab.twGoodness->item(3, 0)->setText(uiGeneralTab.twGoodness->item(3, 0)->text() + UTF8_QSTRING(" (R²)"));
	uiGeneralTab.twGoodness->item(4, 0)->setText(uiGeneralTab.twGoodness->item(4, 0)->text() + UTF8_QSTRING(" (R̄²)"));
	uiGeneralTab.twGoodness->item(5, 0)->setText(UTF8_QSTRING("χ²-") + i18n("Test") + UTF8_QSTRING(" (P > χ²)"));

	auto* layout = new QHBoxLayout(ui.tabGeneral);
	layout->setContentsMargins(0, 0, 0, 0);
	layout->addWidget(generalTab);

	updateLocale();
	retranslateUi();

	// Slots
	connect(uiGeneralTab.cbDataSourceType, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &XYFitCurveDock::dataSourceTypeChanged);
	connect(uiGeneralTab.tbWeights, &QPushButton::clicked, this, &XYFitCurveDock::showWeightsOptions);
	connect(uiGeneralTab.cbXWeight, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &XYFitCurveDock::xWeightChanged);
	connect(uiGeneralTab.cbYWeight, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &XYFitCurveDock::yWeightChanged);
	connect(uiGeneralTab.cbCategory, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &XYFitCurveDock::categoryChanged);
	connect(uiGeneralTab.cbModel, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &XYFitCurveDock::modelTypeChanged);
	connect(uiGeneralTab.sbDegree, QOverload<int>::of(&QSpinBox::valueChanged), this, &XYFitCurveDock::updateModelEquation);
	connect(uiGeneralTab.teEquation, &ExpressionTextEdit::expressionChanged, this, &XYFitCurveDock::expressionChanged);
	connect(uiGeneralTab.tbConstants, &QToolButton::clicked, this, &XYFitCurveDock::showConstants);
	connect(uiGeneralTab.tbFunctions, &QToolButton::clicked, this, &XYFitCurveDock::showFunctions);
	connect(uiGeneralTab.pbLoadFunction, &QPushButton::clicked, this, &XYFitCurveDock::loadFunction);
	connect(uiGeneralTab.pbSaveFunction, &QPushButton::clicked, this, &XYFitCurveDock::saveFunction);
	connect(uiGeneralTab.pbOptions, &QPushButton::clicked, this, &XYFitCurveDock::showOptions);
	connect(uiGeneralTab.cbAlgorithm, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &XYFitCurveDock::algorithmChanged);
	connect(uiGeneralTab.pbRecalculate, &QPushButton::clicked, this, &XYFitCurveDock::recalculateClicked);
	connect(uiGeneralTab.tbData, &QPushButton::clicked, this, &XYFitCurveDock::showDataOptions);
	connect(uiGeneralTab.tbFit, &QPushButton::clicked, this, &XYFitCurveDock::showFitOptions);
	connect(uiGeneralTab.tbParameters, &QPushButton::clicked, this, &XYFitCurveDock::showParameters);
	connect(uiGeneralTab.tbResults, &QPushButton::clicked, this, &XYFitCurveDock::showResults);
	connect(uiGeneralTab.cbPlotRanges, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &XYFitCurveDock::plotRangeChanged);

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
	uiGeneralTab.cbDataSourceType->setCurrentIndex(static_cast<int>(m_fitCurve->dataSourceType()));
	this->dataSourceTypeChanged(uiGeneralTab.cbDataSourceType->currentIndex());

	switch (m_fitCurve->dataSourceType()) {
	case XYAnalysisCurve::DataSourceType::Curve:
		cbDataSourceCurve->setAspect(m_fitCurve->dataSourceCurve());
		break;
	case XYAnalysisCurve::DataSourceType::Histogram:
		cbDataSourceCurve->setAspect(m_fitCurve->dataSourceHistogram());
		break;
	case XYAnalysisCurve::DataSourceType::Spreadsheet:
		cbDataSourceCurve->setAspect(nullptr);
	}

	cbXDataColumn->setAspect(m_fitCurve->xDataColumn(), m_fitCurve->xDataColumnPath());
	cbYDataColumn->setAspect(m_fitCurve->yDataColumn(), m_fitCurve->yDataColumnPath());
	cbXErrorColumn->setAspect(m_fitCurve->xErrorColumn(), m_fitCurve->xErrorColumnPath());
	cbYErrorColumn->setAspect(m_fitCurve->yErrorColumn(), m_fitCurve->yErrorColumnPath());

	int tmpModelType = m_fitData.modelType; // save type because it's reset when category changes
	if (m_fitData.modelCategory == nsl_fit_model_custom)
		uiGeneralTab.cbCategory->setCurrentIndex(uiGeneralTab.cbCategory->count() - 1);
	else
		uiGeneralTab.cbCategory->setCurrentIndex(m_fitData.modelCategory);
	categoryChanged(m_fitData.modelCategory); // fill model types

	m_fitData.modelType = tmpModelType;
	if (m_fitData.modelCategory != nsl_fit_model_custom)
		uiGeneralTab.cbModel->setCurrentIndex(m_fitData.modelType);

	uiGeneralTab.cbXWeight->setCurrentIndex(m_fitData.xWeightsType);
	uiGeneralTab.cbYWeight->setCurrentIndex(m_fitData.yWeightsType);
	uiGeneralTab.sbDegree->setValue(m_fitData.degree);
	DEBUG(Q_FUNC_INFO << ", model degree = " << m_fitData.degree);

	if (m_fitData.paramStartValues.size() > 0)
		DEBUG(Q_FUNC_INFO << ", start value 1 = " << m_fitData.paramStartValues.at(0));

	uiGeneralTab.cbAlgorithm->setCurrentIndex(m_fitData.algorithm);

	uiGeneralTab.chkLegendVisible->setChecked(m_curve->legendVisible());
	uiGeneralTab.chkVisible->setChecked(m_curve->isVisible());

	// Slots
	connect(m_fitCurve, &XYFitCurve::dataSourceTypeChanged, this, &XYFitCurveDock::curveDataSourceTypeChanged);
	connect(m_fitCurve, &XYFitCurve::dataSourceCurveChanged, this, &XYFitCurveDock::curveDataSourceCurveChanged);
	connect(m_fitCurve, &XYFitCurve::dataSourceHistogramChanged, this, &XYFitCurveDock::curveDataSourceHistogramChanged);
	connect(m_fitCurve, &XYFitCurve::xDataColumnChanged, this, &XYFitCurveDock::curveXDataColumnChanged);
	connect(m_fitCurve, &XYFitCurve::yDataColumnChanged, this, &XYFitCurveDock::curveYDataColumnChanged);
	connect(m_fitCurve, &XYFitCurve::xErrorColumnChanged, this, &XYFitCurveDock::curveXErrorColumnChanged);
	connect(m_fitCurve, &XYFitCurve::yErrorColumnChanged, this, &XYFitCurveDock::curveYErrorColumnChanged);
	connect(m_fitCurve, &XYFitCurve::fitDataChanged, this, &XYFitCurveDock::curveFitDataChanged);
	connect(m_fitCurve, &XYFitCurve::sourceDataChanged, this, &XYFitCurveDock::enableRecalculate);

	connect(fitParametersWidget, &FitParametersWidget::parametersChanged, this, &XYFitCurveDock::parametersChanged);
	connect(fitParametersWidget, &FitParametersWidget::parametersValid, this, &XYFitCurveDock::parametersValid);
}

void XYFitCurveDock::setModel() {
	QList<const AbstractAspect*> hiddenAspects;
	for (auto* curve : m_curvesList)
		hiddenAspects << curve;
	cbDataSourceCurve->setHiddenAspects(hiddenAspects);

	const auto& topLevelClasses = TreeViewComboBox::plotColumnTopLevelClasses();
	cbXDataColumn->setTopLevelClasses(topLevelClasses);
	cbYDataColumn->setTopLevelClasses(topLevelClasses);
	cbXErrorColumn->setTopLevelClasses(topLevelClasses);
	cbYErrorColumn->setTopLevelClasses(topLevelClasses);

	auto* model = aspectModel();
	model->setSelectableAspects({AspectType::Column});

	cbXDataColumn->setModel(model);
	cbYDataColumn->setModel(model);
	cbXErrorColumn->setModel(model);
	cbYErrorColumn->setModel(model);

	XYCurveDock::setModel();
}

/*!
  sets the curves. The properties of the curves in the list \c list can be edited in this widget.
*/
void XYFitCurveDock::setCurves(QList<XYCurve*> list) {
	CONDITIONAL_LOCK_RETURN;
	m_curvesList = list;
	m_curve = list.first();
	setAspects(list);
	m_fitCurve = static_cast<XYFitCurve*>(m_curve);

	// we need a second model for data source comboboxes which will be dynamically
	// updated in the slot depending on the current type (spreadsheet, curve or histogram)
	// to allow to select the relevant aspects only
	m_dataSourceModel = new AspectTreeModel(m_curve->project());

	this->setModel();
	m_fitData = m_fitCurve->fitData();

	DEBUG(Q_FUNC_INFO << ", model type = " << m_fitData.modelType);
	DEBUG(Q_FUNC_INFO << ", model expression = " << STDSTRING(m_fitData.model));
	DEBUG(Q_FUNC_INFO << ", model degree = " << m_fitData.degree);
	DEBUG(Q_FUNC_INFO << ", # params = " << m_fitData.paramNames.size());
	DEBUG(Q_FUNC_INFO << ", # start values = " << m_fitData.paramStartValues.size());
	// for (auto startValue: m_fitData.paramStartValues)
	//	DEBUG("XYFitCurveDock::setCurves()	start value = " << startValue);

	fitParametersWidget->setFitData(&m_fitData);

	if (m_messageWidget && m_messageWidget->isVisible()) {
		DEBUG(Q_FUNC_INFO << ", close message")
		m_messageWidget->animatedHide();
	}

	initGeneralTab();
	initTabs();
	setSymbols(list);

	showFitResult();
	enableRecalculate();

	updatePlotRangeList();

	// init parameter list when not available
	if (m_fitData.paramStartValues.size() == 0)
		updateModelEquation();
}

bool XYFitCurveDock::eventFilter(QObject* obj, QEvent* event) {
	if (event->type() == QEvent::KeyPress && (obj == uiGeneralTab.twParameters || obj == uiGeneralTab.twGoodness || obj == uiGeneralTab.twLog)) {
		auto* key_event = static_cast<QKeyEvent*>(event);
		if (key_event->matches(QKeySequence::Copy)) {
			resultCopy();
			return true;
		}
	}
	return QWidget::eventFilter(obj, event);
}

void XYFitCurveDock::checkDataColumns() {
	if (m_initializing)
		return;

	if (!m_messageWidget) {
		m_messageWidget = new KMessageWidget(this);
		uiGeneralTab.gridLayout_2->addWidget(m_messageWidget, 23, 2, 1, 2);
	}
	switch (m_fitCurve->dataSourceType()) {
	case XYAnalysisCurve::DataSourceType::Spreadsheet: {
		auto xColumn = m_fitCurve->xDataColumn();
		auto yColumn = m_fitCurve->yDataColumn();
		if (!xColumn && !yColumn)
			m_messageWidget->setText(i18n("No X and Y column specified!"));
		else if (!xColumn)
			m_messageWidget->setText(i18n("No X column specified!"));
		else if (!yColumn)
			m_messageWidget->setText(i18n("No Y column specified!"));
		else if (xColumn->availableRowCount(1) == 0)
			m_messageWidget->setText(i18n("No X data available!"));
		else if (yColumn->availableRowCount(1) == 0)
			m_messageWidget->setText(i18n("No Y data available!"));

		if (!xColumn || !yColumn || xColumn->availableRowCount(1) == 0 || yColumn->availableRowCount(1) == 0)
			m_messageWidget->animatedShow();
		break;
	}
	case XYAnalysisCurve::DataSourceType::Curve:
		// TODO: check for enough data
		m_messageWidget->animatedHide();
		break;
	case XYAnalysisCurve::DataSourceType::Histogram:
		// TODO: check for enough data
		m_messageWidget->animatedHide();
		break;
	}
}

/*
 * updates the locale in the widgets. called when the application settings are changed.
 */
void XYFitCurveDock::updateLocale() {
	if (m_fitCurve) {
		fitParametersWidget->setFitData(&m_fitData);
		showFitResult();
	}
}

void XYFitCurveDock::retranslateUi() {
	CONDITIONAL_LOCK_RETURN;

	uiGeneralTab.cbDataSourceType->clear();
	uiGeneralTab.cbDataSourceType->addItem(i18n("Spreadsheet"));
	uiGeneralTab.cbDataSourceType->addItem(i18n("XY-Curve"));
	uiGeneralTab.cbDataSourceType->addItem(i18n("Histogram"));

	// header labels
	QStringList labels{QString(), i18n("Value"), i18n("Uncertainty"), i18n("Uncertainty, %"), i18n("t statistic"), QLatin1String("P > |t|"), i18n("Lower"), i18n("Upper")};
	uiGeneralTab.twParameters->setHorizontalHeaderLabels(labels);

	fitParametersWidget->retranslateUi();

	// retranslate fit results, if available
	if (m_fitCurve)
		showFitResult();
}

//*************************************************************
//******* SLOTs for changes triggered in XYFitCurveDock *******
//*************************************************************
void XYFitCurveDock::dataSourceTypeChanged(int index) {
	DEBUG(Q_FUNC_INFO << ", m_initializing = " << m_initializing)
	const auto type = static_cast<XYAnalysisCurve::DataSourceType>(index);
	DEBUG(Q_FUNC_INFO << ", source type = " << ENUM_TO_STRING(XYAnalysisCurve, DataSourceType, type))
	if (type == XYAnalysisCurve::DataSourceType::Spreadsheet) {
		uiGeneralTab.cbCategory->setEnabled(true);
		cbDataSourceCurve->setTopLevelClasses(TreeViewComboBox::plotColumnTopLevelClasses());

		// when the dock is initialized, this functions is called before setModel(),
		// we need this nullptr check
		if (m_dataSourceModel) {
			m_dataSourceModel->setSelectableAspects({AspectType::Column});

			// TODO: why do we need to reset the model here and below again to get the combobox updated?
			cbDataSourceCurve->setModel(m_dataSourceModel);
		}
	} else { // curve or histogram
		if (type == XYAnalysisCurve::DataSourceType::Curve) {
			uiGeneralTab.cbCategory->setEnabled(true);
			uiGeneralTab.lDataSourceCurve->setText(i18n("Curve:"));

			QList<AspectType> list{AspectType::Folder,
								   AspectType::Datapicker,
								   AspectType::Worksheet,
								   AspectType::CartesianPlot,
								   AspectType::XYCurve,
								   AspectType::XYAnalysisCurve,
								   AspectType::XYEquationCurve,
								   AspectType::XYFunctionCurve};
			cbDataSourceCurve->setTopLevelClasses(list);

			if (m_dataSourceModel) {
				list = {AspectType::XYCurve, AspectType::XYAnalysisCurve, AspectType::XYEquationCurve, AspectType::XYFunctionCurve};
				m_dataSourceModel->setSelectableAspects(list);
				cbDataSourceCurve->setModel(m_dataSourceModel);
				cbDataSourceCurve->setAspect(m_fitCurve->dataSourceCurve());
			}
		} else { // histogram
			uiGeneralTab.cbCategory->setEnabled(false);
			uiGeneralTab.cbCategory->setCurrentIndex(3); // select "statistics (distributions);
			uiGeneralTab.lDataSourceCurve->setText(i18n("Histogram:"));

			QList<AspectType> list{AspectType::Folder, AspectType::Worksheet, AspectType::CartesianPlot, AspectType::Histogram};
			cbDataSourceCurve->setTopLevelClasses(list);

			if (m_dataSourceModel) {
				list = {AspectType::Histogram};
				m_dataSourceModel->setSelectableAspects(list);
				cbDataSourceCurve->setModel(m_dataSourceModel);
				cbDataSourceCurve->setAspect(m_fitCurve->dataSourceHistogram());
			}
		}
	}

	showDataOptions(uiGeneralTab.tbData->isChecked()); // show/hide data source widgets for the current type

	enableRecalculate();

	CONDITIONAL_LOCK_RETURN;

	for (auto* curve : m_curvesList)
		static_cast<XYFitCurve*>(curve)->setDataSourceType(type);
}

void XYFitCurveDock::dataSourceCurveChanged(const QModelIndex& index) {
	DEBUG(Q_FUNC_INFO << ", m_initializing = " << m_initializing)
	CONDITIONAL_RETURN_NO_LOCK;

	auto* aspect = static_cast<AbstractAspect*>(index.internalPointer());

	const auto type = (XYAnalysisCurve::DataSourceType)uiGeneralTab.cbDataSourceType->currentIndex();
	if (type == XYAnalysisCurve::DataSourceType::Curve) {
		auto* dataSourceCurve = static_cast<XYCurve*>(aspect);
		for (auto* curve : m_curvesList)
			static_cast<XYFitCurve*>(curve)->setDataSourceCurve(dataSourceCurve);
	} else {
		auto* dataSourceHist = static_cast<Histogram*>(aspect);
		for (auto* curve : m_curvesList)
			static_cast<XYFitCurve*>(curve)->setDataSourceHistogram(dataSourceHist);
	}
}

void XYFitCurveDock::xDataColumnChanged(const QModelIndex& index) {
	CONDITIONAL_LOCK_RETURN;

	auto* aspect = static_cast<AbstractAspect*>(index.internalPointer());
	auto* column = dynamic_cast<AbstractColumn*>(aspect);

	for (auto* curve : m_curvesList)
		static_cast<XYFitCurve*>(curve)->setXDataColumn(column);

	// set model dependent start values from new data
	DEBUG(Q_FUNC_INFO)
	static_cast<XYFitCurve*>(m_curve)->initStartValues(m_fitData);
	// udpate parameter widget
	fitParametersWidget->setFitData(&m_fitData);
	enableRecalculate(); // update preview
	showFitResult(); // show result of preview

	// update model limits depending on number of points
	modelTypeChanged(uiGeneralTab.cbModel->currentIndex());

	cbXDataColumn->setInvalid(false);
}

void XYFitCurveDock::yDataColumnChanged(const QModelIndex& index) {
	CONDITIONAL_LOCK_RETURN;

	auto* aspect = static_cast<AbstractAspect*>(index.internalPointer());
	auto* column = dynamic_cast<AbstractColumn*>(aspect);

	for (auto* curve : m_curvesList)
		static_cast<XYFitCurve*>(curve)->setYDataColumn(column);

	// set model dependent start values from new data
	DEBUG(Q_FUNC_INFO)
	static_cast<XYFitCurve*>(m_curve)->initStartValues(m_fitData);
	// update parameter widget
	fitParametersWidget->setFitData(&m_fitData);
	enableRecalculate(); // update preview
	showFitResult(); // show result of preview

	cbYDataColumn->setInvalid(false);
}

void XYFitCurveDock::xErrorColumnChanged(const QModelIndex& index) {
	CONDITIONAL_LOCK_RETURN;

	auto* aspect = static_cast<AbstractAspect*>(index.internalPointer());
	auto* column = dynamic_cast<AbstractColumn*>(aspect);

	for (auto* curve : m_curvesList)
		static_cast<XYFitCurve*>(curve)->setXErrorColumn(column);

	cbXErrorColumn->setInvalid(false);
}

void XYFitCurveDock::yErrorColumnChanged(const QModelIndex& index) {
	CONDITIONAL_LOCK_RETURN;

	auto* aspect = static_cast<AbstractAspect*>(index.internalPointer());
	auto* column = dynamic_cast<AbstractColumn*>(aspect);

	for (auto* curve : m_curvesList)
		static_cast<XYFitCurve*>(curve)->setYErrorColumn(column);

	cbYErrorColumn->setInvalid(false);
}

///////////////////////// fold/unfold options //////////////////////////////////////////////////

void XYFitCurveDock::showDataOptions(bool checked) {
	if (checked) {
		uiGeneralTab.tbData->setIcon(QIcon::fromTheme(QStringLiteral("arrow-down")));
		uiGeneralTab.lDataSourceType->show();
		uiGeneralTab.cbDataSourceType->show();

		// show/hide data source widgets depending on the current type
		const auto type = static_cast<XYAnalysisCurve::DataSourceType>(uiGeneralTab.cbDataSourceType->currentIndex());
		if (type == XYAnalysisCurve::DataSourceType::Spreadsheet) {
			uiGeneralTab.lDataSourceCurve->hide();
			cbDataSourceCurve->hide();
			uiGeneralTab.lXColumn->show();
			cbXDataColumn->show();
			uiGeneralTab.lYColumn->show();
			cbYDataColumn->show();
		} else { // curve or histogram
			uiGeneralTab.lDataSourceCurve->show();
			cbDataSourceCurve->show();
			uiGeneralTab.lXColumn->hide();
			cbXDataColumn->hide();
			uiGeneralTab.lYColumn->hide();
			cbYDataColumn->hide();
		}
	} else {
		uiGeneralTab.tbData->setIcon(QIcon::fromTheme(QStringLiteral("arrow-right")));
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
		uiGeneralTab.tbWeights->setIcon(QIcon::fromTheme(QStringLiteral("arrow-down")));
		uiGeneralTab.lXWeight->show();
		uiGeneralTab.cbXWeight->show();
		uiGeneralTab.lXErrorCol->show();
		cbXErrorColumn->show();
		uiGeneralTab.lYWeight->show();
		uiGeneralTab.cbYWeight->show();
		uiGeneralTab.lYErrorCol->show();
		cbYErrorColumn->show();
	} else {
		uiGeneralTab.tbWeights->setIcon(QIcon::fromTheme(QStringLiteral("arrow-right")));
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
		uiGeneralTab.tbFit->setIcon(QIcon::fromTheme(QStringLiteral("arrow-down")));
		uiGeneralTab.lCategory->show();
		uiGeneralTab.cbCategory->show();
		uiGeneralTab.lModel->show();
		uiGeneralTab.cbModel->show();
		uiGeneralTab.lEquation->show();

		CONDITIONAL_LOCK_RETURN; // do not change start parameter
		modelTypeChanged(uiGeneralTab.cbModel->currentIndex());
	} else {
		uiGeneralTab.tbFit->setIcon(QIcon::fromTheme(QStringLiteral("arrow-right")));
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
		uiGeneralTab.tbParameters->setIcon(QIcon::fromTheme(QStringLiteral("arrow-down")));
		uiGeneralTab.frameParameters->show();
	} else {
		uiGeneralTab.tbParameters->setIcon(QIcon::fromTheme(QStringLiteral("arrow-right")));
		uiGeneralTab.frameParameters->hide();
	}
}

void XYFitCurveDock::showResults(bool checked) {
	if (checked) {
		uiGeneralTab.tbResults->setIcon(QIcon::fromTheme(QStringLiteral("arrow-down")));
		uiGeneralTab.twResults->show();
	} else {
		uiGeneralTab.tbResults->setIcon(QIcon::fromTheme(QStringLiteral("arrow-right")));
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
	if (index >= NSL_FIT_MODEL_CATEGORY_COUNT - 1) {	// custom category
		DEBUG(Q_FUNC_INFO << ", category = \"nsl_fit_model_custom\"");
		uiGeneralTab.pbSaveFunction->setEnabled(true);
	} else {
		DEBUG(Q_FUNC_INFO << ", category = \"" << nsl_fit_model_category_name[index] << "\"");
		uiGeneralTab.pbSaveFunction->setEnabled(false);
	}

	bool hasChanged = true;
	// nothing has changed when ...
	if (m_fitData.modelCategory == (nsl_fit_model_category)index
		|| (m_fitData.modelCategory == nsl_fit_model_custom && index == uiGeneralTab.cbCategory->count() - 1))
		hasChanged = false;
	DEBUG(Q_FUNC_INFO << ", has changed: " << hasChanged)

	if (uiGeneralTab.cbCategory->currentIndex() == uiGeneralTab.cbCategory->count() - 1)
		m_fitData.modelCategory = nsl_fit_model_custom;
	else
		m_fitData.modelCategory = (nsl_fit_model_category)index;
	uiGeneralTab.lModel->setText(i18n("Model:"));
	uiGeneralTab.cbModel->clear();
	uiGeneralTab.cbModel->setToolTip(QLatin1String(""));

	// enable algorithm selection only for distributions
	if (m_fitData.modelCategory == nsl_fit_model_distribution) {
		uiGeneralTab.lAlgorithm->show();
		uiGeneralTab.cbAlgorithm->show();
	} else {
		uiGeneralTab.lAlgorithm->hide();
		uiGeneralTab.cbAlgorithm->hide();
	}

	switch (m_fitData.modelCategory) {
	case nsl_fit_model_basic:
		for (int i = 0; i < NSL_FIT_MODEL_BASIC_COUNT; i++)
			uiGeneralTab.cbModel->addItem(QLatin1String(nsl_fit_model_basic_name[i]));
		break;
	case nsl_fit_model_peak: {
		for (int i = 0; i < NSL_FIT_MODEL_PEAK_COUNT; i++)
			uiGeneralTab.cbModel->addItem(QLatin1String(nsl_fit_model_peak_name[i]));
#if defined(_MSC_VER)
		// disable voigt model
		const auto* model = qobject_cast<const QStandardItemModel*>(uiGeneralTab.cbModel->model());
		auto* item = model->item(nsl_fit_model_voigt);
		item->setFlags(item->flags() & ~(Qt::ItemIsSelectable | Qt::ItemIsEnabled));
#endif
		break;
	}
	case nsl_fit_model_growth:
		for (int i = 0; i < NSL_FIT_MODEL_GROWTH_COUNT; i++)
			uiGeneralTab.cbModel->addItem(QLatin1String(nsl_fit_model_growth_name[i]));
		break;
	case nsl_fit_model_distribution: {
		for (int i = 0; i < NSL_SF_STATS_DISTRIBUTION_COUNT; i++)
			uiGeneralTab.cbModel->addItem(QLatin1String(nsl_sf_stats_distribution_name[i]));

		// not-used items are disabled here
		const auto* model = qobject_cast<const QStandardItemModel*>(uiGeneralTab.cbModel->model());

		for (int i = 1; i < NSL_SF_STATS_DISTRIBUTION_COUNT; i++) {
			if (m_fitData.algorithm == nsl_fit_algorithm_ml) {
				if (!nsl_sf_stats_distribution_supports_ML((nsl_sf_stats_distribution)i)) {
					auto* item = model->item(i);
					item->setFlags(item->flags() & ~(Qt::ItemIsSelectable | Qt::ItemIsEnabled));
				}
			} else { // LM
				// unused distributions
				if (i == nsl_sf_stats_levy_alpha_stable || i == nsl_sf_stats_levy_skew_alpha_stable || i == nsl_sf_stats_bernoulli) {
					auto* item = model->item(i);
					item->setFlags(item->flags() & ~(Qt::ItemIsSelectable | Qt::ItemIsEnabled));
				}
			}
		}
		break;
	}
	case nsl_fit_model_custom:
		uiGeneralTab.lModel->setText(i18n("Description:"));
		uiGeneralTab.cbModel->addItem(i18n("User defined model"));
	}

	if (hasChanged) {
		DEBUG(Q_FUNC_INFO << ", Resetting model")
		// show the fit-model for the currently selected default (first) fit-model
		uiGeneralTab.cbModel->setCurrentIndex(0);
		uiGeneralTab.sbDegree->setValue(1);
		// when model type does not change, call it here
		updateModelEquation();
	}

	// update algorithm list
	const auto* model = qobject_cast<const QStandardItemModel*>(uiGeneralTab.cbAlgorithm->model());
	auto* item = model->item(nsl_fit_algorithm_ml);
	// enable ML item only for distributions
	if (m_fitData.modelCategory == nsl_fit_model_distribution) {
		item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
	} else {
		uiGeneralTab.cbAlgorithm->setCurrentIndex(nsl_fit_algorithm_lm);
		item->setFlags(item->flags() & ~(Qt::ItemIsSelectable | Qt::ItemIsEnabled));
	}

	enableRecalculate();
}

/*!
 * called when the fit model type (depends on category) was changed.
 * Updates the model type dependent widgets in the general-tab and calls \c updateModelEquation() to update the preview pixmap.
 */
void XYFitCurveDock::modelTypeChanged(int index) {
	DEBUG(Q_FUNC_INFO << ", type = " << index << ", initializing = " << m_initializing << ", current type = " << m_fitData.modelType);
	// leave if no selection
	if (index == -1)
		return;

	bool custom = false;
	if (m_fitData.modelCategory == nsl_fit_model_custom)
		custom = true;
	uiGeneralTab.teEquation->setReadOnly(!custom);
	uiGeneralTab.tbFunctions->setVisible(custom);
	uiGeneralTab.tbConstants->setVisible(custom);

	// default settings
	uiGeneralTab.lDegree->setText(i18n("Degree:"));
	if (m_fitData.modelType != index)
		uiGeneralTab.sbDegree->setValue(1);

	const AbstractColumn* xColumn = nullptr;
	if (m_fitCurve->dataSourceType() == XYAnalysisCurve::DataSourceType::Spreadsheet) {
		DEBUG(Q_FUNC_INFO << ", data source: Spreadsheet")
		// auto* aspect = static_cast<AbstractAspect*>(cbXDataColumn->currentModelIndex().internalPointer());
		// xColumn = dynamic_cast<AbstractColumn*>(aspect);
		xColumn = m_fitCurve->xDataColumn();
	} else {
		DEBUG(Q_FUNC_INFO << ", data source: Curve or Histogram")
		if (m_fitCurve->dataSourceCurve())
			xColumn = m_fitCurve->dataSourceCurve()->xColumn();
	}
	// with no xColumn: show all models (assume 100 data points)
	const int availableRowCount = xColumn ? xColumn->availableRowCount(100) : 100;
	DEBUG(Q_FUNC_INFO << ", available row count = " << availableRowCount)

	bool disableFit = false;
	switch (m_fitData.modelCategory) {
	case nsl_fit_model_basic:
		switch (index) {
		case nsl_fit_model_polynomial:
			uiGeneralTab.lDegree->setVisible(true);
			uiGeneralTab.sbDegree->setVisible(true);
			if (availableRowCount > 1)
				uiGeneralTab.sbDegree->setMaximum(std::min(availableRowCount - 1, 10));
			else
				uiGeneralTab.sbDegree->setMaximum(1);
			break;
		case nsl_fit_model_fourier:
			if (availableRowCount < 4) { // too few data points
				uiGeneralTab.lDegree->setVisible(false);
				uiGeneralTab.sbDegree->setVisible(false);
				disableFit = true;
			} else {
				uiGeneralTab.lDegree->setVisible(true);
				uiGeneralTab.sbDegree->setVisible(true);
				uiGeneralTab.sbDegree->setMaximum(std::min(availableRowCount / 2 - 1, 10));
			}
			break;
		case nsl_fit_model_power:
			uiGeneralTab.lDegree->setVisible(true);
			uiGeneralTab.sbDegree->setVisible(true);
			uiGeneralTab.sbDegree->setMaximum(2);
			// TODO: limit degree depending on availableRowCount
			break;
		case nsl_fit_model_exponential:
			uiGeneralTab.lDegree->setVisible(true);
			uiGeneralTab.sbDegree->setVisible(true);
			uiGeneralTab.sbDegree->setMaximum(10);
			// TODO: limit degree depending on availableRowCount
			break;
		default:
			uiGeneralTab.lDegree->setVisible(false);
			uiGeneralTab.sbDegree->setVisible(false);
		}
		break;
	case nsl_fit_model_peak: // all models support multiple peaks
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

	if (m_fitData.modelCategory == nsl_fit_model_distribution) {
		// enable ML only for supported distros
		const auto* model = qobject_cast<const QStandardItemModel*>(uiGeneralTab.cbAlgorithm->model());
		auto* item = model->item(nsl_fit_algorithm_ml);

		if (nsl_sf_stats_distribution_supports_ML((nsl_sf_stats_distribution)index))
			item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
		else { // switch to LM
			item->setFlags(item->flags() & ~(Qt::ItemIsSelectable | Qt::ItemIsEnabled));
			uiGeneralTab.cbAlgorithm->setCurrentIndex(nsl_fit_algorithm_lm);
		}
	}

	if (!m_initializing)
		m_fitData.modelType = index;

	updateModelEquation();
	// TODO: update parameter list?
	//  updateParameterList();

	if (disableFit)
		uiGeneralTab.pbRecalculate->setEnabled(false);

	DEBUG(Q_FUNC_INFO << ", DONE")
}

/*!
 * Called when the model type or the degree of the model were changed.
 * Show the preview pixmap of the fit model expression for the current model category and type.
 */
void XYFitCurveDock::updateModelEquation() {
	if (m_fitData.modelCategory == nsl_fit_model_custom) {
		DEBUG(Q_FUNC_INFO << ", category = nsl_fit_model_custom, type = " << m_fitData.modelType);
	} else {
		DEBUG(Q_FUNC_INFO << ", category = " << nsl_fit_model_category_name[m_fitData.modelCategory] << ", type = " << m_fitData.modelType);
	}

	// this function can also be called when the value for the degree was changed -> update the fit data structure
	int degree = uiGeneralTab.sbDegree->value();
	if (!m_initializing) {
		m_fitData.degree = degree;
		XYFitCurve::initFitData(m_fitData);
		// set model dependent start values from curve data
		// invalidate result
		m_fitCurve->clearFitResult();
		static_cast<XYFitCurve*>(m_curve)->initStartValues(m_fitData);
		// udpate parameter widget
		fitParametersWidget->setFitData(&m_fitData);
		if (m_messageWidget && m_messageWidget->isVisible()) {
			DEBUG(Q_FUNC_INFO << ", close message")
			m_messageWidget->animatedHide();
		}

		if (m_fitData.previewEnabled)
			showFitResult(); // show result of preview
	}

	// variables/parameter that are known
	QStringList vars = {QStringLiteral("x")};
	vars << m_fitData.paramNames;
	uiGeneralTab.teEquation->setVariables(vars);

	// set formula picture
	uiGeneralTab.lEquation->setText(QStringLiteral("f(x) ="));
	QString file;
	switch (m_fitData.modelCategory) {
	case nsl_fit_model_basic: {
		// formula pic depends on degree
		QString numSuffix = QString::number(degree);
		if (degree > 4)
			numSuffix = QLatin1Char('4');
		if ((nsl_fit_model_type_basic)m_fitData.modelType == nsl_fit_model_power && degree > 2)
			numSuffix = QLatin1Char('2');
		file = QStandardPaths::locate(QStandardPaths::AppDataLocation,
									  QStringLiteral("pics/fit_models/") + QLatin1String(nsl_fit_model_basic_pic_name[m_fitData.modelType]) + numSuffix
										  + QStringLiteral(".pdf"));
		break;
	}
	case nsl_fit_model_peak: {
		// formula pic depends on number of peaks
		QString numSuffix = QString::number(degree);
		if (degree > 4)
			numSuffix = QLatin1Char('4');
		file = QStandardPaths::locate(QStandardPaths::AppDataLocation,
									  QStringLiteral("pics/fit_models/") + QLatin1String(nsl_fit_model_peak_pic_name[m_fitData.modelType]) + numSuffix
										  + QStringLiteral(".pdf"));
		break;
	}
	case nsl_fit_model_growth:
		file = QStandardPaths::locate(QStandardPaths::AppDataLocation,
									  QStringLiteral("pics/fit_models/") + QLatin1String(nsl_fit_model_growth_pic_name[m_fitData.modelType])
										  + QStringLiteral(".pdf"));
		break;
	case nsl_fit_model_distribution:
		file = QStandardPaths::locate(QStandardPaths::AppDataLocation,
									  QStringLiteral("pics/gsl_distributions/") + QLatin1String(nsl_sf_stats_distribution_pic_name[m_fitData.modelType])
										  + QStringLiteral(".pdf"));
		// change label
		if (m_fitData.modelType == nsl_sf_stats_poisson)
			uiGeneralTab.lEquation->setText(QStringLiteral("f(k)/A ="));
		else
			uiGeneralTab.lEquation->setText(QStringLiteral("f(x)/A ="));
		break;
	case nsl_fit_model_custom:
		uiGeneralTab.lFuncPic->hide();
		uiGeneralTab.teEquation->show();
		uiGeneralTab.teEquation->setPlainText(m_fitData.model);
	}

	if (m_fitData.modelCategory != nsl_fit_model_custom) {
		QImage image = GuiTools::importPDFFile(file);

		// use system palette for background
		if (GuiTools::isDarkMode()) {
			// invert image if in dark mode
			image.invertPixels();

			for (int i = 0; i < image.size().width(); i++)
				for (int j = 0; j < image.size().height(); j++)
					if (qGray(image.pixel(i, j)) < 64) // 0-255: 0-64 covers all dark pixel
						image.setPixel(QPoint(i, j), palette().color(QPalette::Base).rgb());
		} else {
			for (int i = 0; i < image.size().width(); i++)
				for (int j = 0; j < image.size().height(); j++)
					if (qGray(image.pixel(i, j)) > 192) // 0-255: 224-255 covers all light pixel
						image.setPixel(QPoint(i, j), palette().color(QPalette::Base).rgb());
		}

		if (image.isNull()) {
			DEBUG(Q_FUNC_INFO << ", WARNING: model image is null!")
			uiGeneralTab.lEquation->hide();
			uiGeneralTab.lFuncPic->hide();
		} else {
			// use light/dark background in the preview label
			QPalette p;
			p.setColor(QPalette::Window, palette().color(QPalette::Base));
			uiGeneralTab.lFuncPic->setAutoFillBackground(true);
			uiGeneralTab.lFuncPic->setPalette(p);

			uiGeneralTab.lFuncPic->setPixmap(QPixmap::fromImage(image));
			uiGeneralTab.lFuncPic->show();
		}
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

void XYFitCurveDock::insertFunction(const QString& functionName) const {
	uiGeneralTab.teEquation->insertPlainText(functionName + ExpressionParser::functionArgumentString(functionName, XYEquationCurve::EquationType::Cartesian));
}

void XYFitCurveDock::insertConstant(const QString& constantsName) const {
	uiGeneralTab.teEquation->insertPlainText(constantsName);
}

void XYFitCurveDock::algorithmChanged(int index) {
	m_fitData.algorithm = (nsl_fit_algorithm)index;

	enableRecalculate();
}

/*!
 * Update parameter by parsing expression
 * Only called for custom fit model
 */
void XYFitCurveDock::updateParameterList() {
	DEBUG(Q_FUNC_INFO);
	// use current model function
	m_fitData.model = uiGeneralTab.teEquation->toPlainText();

	ExpressionParser* parser = ExpressionParser::getInstance();
	QStringList vars; // variables that are known
	vars << QStringLiteral("x"); // TODO: generalize when we support other XYEquationCurve::EquationType
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
	DEBUG(Q_FUNC_INFO << ", m_initializing = " << m_initializing);

	// parameter names were (probably) changed -> set the new vars in ExpressionTextEdit teEquation
	QStringList vars{m_fitData.paramNames};
	vars << QStringLiteral("x"); // TODO: generalize when we support other XYEquationCurve::EquationType
	uiGeneralTab.teEquation->setVariables(vars);

	CONDITIONAL_RETURN_NO_LOCK

	if (updateParameterWidget)
		fitParametersWidget->setFitData(&m_fitData);

	enableRecalculate();
	DEBUG(Q_FUNC_INFO << " DONE")
}
void XYFitCurveDock::parametersValid(bool valid) {
	DEBUG(Q_FUNC_INFO << ", valid = " << valid);
	m_parametersValid = valid;
}

// TextPreview for FileWidget
class TextPreview : public KPreviewWidgetBase {
public:
	explicit TextPreview(QWidget *parent = nullptr)
		: KPreviewWidgetBase(parent) {
        textEdit = new QPlainTextEdit(this);
        textEdit->setReadOnly(true);
        textEdit->setWordWrapMode(QTextOption::WrapAnywhere);

        auto* layout = new QVBoxLayout(this);
        layout->addWidget(textEdit);
        layout->setContentsMargins(0, 0, 0, 0);
        setLayout(layout);
    }

    void showPreview(const QUrl &url) override {
        textEdit->clear();
        if (!url.isLocalFile()) return;

        QFile file(url.toLocalFile());
        if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QTextStream in(&file);
            QString text = in.read(1000);  // Limit size for preview
            textEdit->setPlainText(text);
        }
    }

    void clearPreview() override {
        textEdit->clear();
    }

private:
    QPlainTextEdit* textEdit;
};

void XYFitCurveDock::loadFunction() {
	//easy alternative: const QString& fileName = QFileDialog::getOpenFileName(this, i18nc("@title:window", "Select file to load function definition"), dir, filter);

	QDialog dialog;
	dialog.setWindowTitle(i18n("Select file to load function definition"));
	auto* layout = new QVBoxLayout(&dialog);

	// use last open dir from MainWin (project dir)
	KConfigGroup mainGroup = Settings::group(QStringLiteral("MainWin"));
	const QString& dir = mainGroup.readEntry("LastOpenDir", "");

	//using KFileWidget to add custom widgets
	auto* fileWidget = new KFileWidget(QUrl(dir), &dialog);
	fileWidget->setOperationMode(KFileWidget::Opening);
	fileWidget->setMode(KFile::File);

	// preview
	auto* preview = new TextPreview();
	fileWidget->setPreviewWidget(preview);

	auto filterList = QList<KFileFilter>();
	filterList << KFileFilter(i18n("LabPlot Function Definition"), {QLatin1String("*.lfd"), QLatin1String("*.LFD")}, {});
	fileWidget->setFilters(filterList);

	fileWidget->okButton()->show();
	fileWidget->okButton()->setEnabled(false);
	fileWidget->cancelButton()->show();
	QObject::connect(fileWidget->okButton(), &QPushButton::clicked, &dialog, &QDialog::accept);
	QObject::connect(fileWidget, &KFileWidget::selectionChanged, &dialog, [=]() {
		QString fileName = fileWidget->locationEdit()->currentText();
		auto currentDir = fileWidget->baseUrl().toLocalFile();
		fileName.prepend(currentDir);
		if (QFile::exists(fileName))
			fileWidget->okButton()->setEnabled(true);
	});
	QObject::connect(fileWidget->cancelButton(), &QPushButton::clicked, &dialog, &QDialog::reject);
	layout->addWidget(fileWidget);

	if (dialog.exec() == QDialog::Accepted) {
		QString fileName = fileWidget->locationEdit()->currentText();
		auto currentDir = fileWidget->baseUrl().toLocalFile();
		fileName.prepend(currentDir);

		//load config from file if accepted
		QDEBUG(Q_FUNC_INFO << ", load function from file" << fileName)

		KConfig config(fileName);
		auto general = config.group(QLatin1String("General"));
		m_fitData.model = general.readEntry("Function", "");
		// switch to custom model
		uiGeneralTab.cbCategory->setCurrentIndex(uiGeneralTab.cbCategory->count() - 1);

		auto description = general.readEntry("Description", "");
		auto comment = general.readEntry("Comment", "");
		QDEBUG("Description:" << description)
		QDEBUG("Comment:" << comment)
		if (!description.isEmpty()) {
			uiGeneralTab.cbModel->clear();
			uiGeneralTab.cbModel->addItem(description);
		}
		if (!comment.isEmpty())
			uiGeneralTab.cbModel->setToolTip(comment);
	}
}

void XYFitCurveDock::saveFunction() {
	QDialog dialog;
	dialog.setWindowTitle(i18n("Select file to save function definition"));
	auto* layout = new QVBoxLayout(&dialog);

	// use last open dir from MainWin (project dir)
	KConfigGroup mainGroup = Settings::group(QStringLiteral("MainWin"));
	const QString& dir = mainGroup.readEntry("LastOpenDir", "");

	//using KFileWidget to add custom widgets
	auto* fileWidget = new KFileWidget(QUrl(dir), &dialog);
	fileWidget->setOperationMode(KFileWidget::Saving);
	fileWidget->setMode(KFile::File);
	// preview
	auto* preview = new TextPreview();
	fileWidget->setPreviewWidget(preview);

	auto filterList = QList<KFileFilter>();
	filterList << KFileFilter(i18n("LabPlot Function Definition"), {QLatin1String("*.lfd"), QLatin1String("*.LFD")}, {});
	fileWidget->setFilters(filterList);

	fileWidget->okButton()->show();
	fileWidget->okButton()->setEnabled(false);
	fileWidget->cancelButton()->show();
	QObject::connect(fileWidget->okButton(), &QPushButton::clicked, &dialog, &QDialog::accept);
	QObject::connect(fileWidget->cancelButton(), &QPushButton::clicked, &dialog, &QDialog::reject);
	layout->addWidget(fileWidget);

	// custom widgets
	auto* lDescription = new QLabel(i18n("Description:"));
	auto* leDescription = new KLineEdit(uiGeneralTab.cbModel->currentText());
	auto* lComment = new QLabel(i18n("Comment:"));
	auto* leComment = new KLineEdit(uiGeneralTab.cbModel->toolTip());

	// update description and comment when selection changes
	connect(fileWidget, &KFileWidget::fileHighlighted, this, [=]() {
		QString fileName = fileWidget->locationEdit()->currentText();
		auto currentDir = fileWidget->baseUrl().toLocalFile();
		fileName.prepend(currentDir);
		QDEBUG(Q_FUNC_INFO << ", file selected:" << fileName)
		if (QFile::exists(fileName)) {
			KConfig config(fileName);
			auto group = config.group(QLatin1String("General"));
			const QString& description = group.readEntry("Description", "");
			const QString& comment = group.readEntry("Comment", "");
			if (!description.isEmpty())
				leDescription->setText(description);
			if (!comment.isEmpty())
				leComment->setText(comment);
		}
	});

	auto* grid = new QGridLayout;
	grid->addWidget(lDescription, 0, 0);
	grid->addWidget(leDescription, 0, 1);
	grid->addWidget(lComment, 1, 0);
	grid->addWidget(leComment, 1, 1);
	layout->addLayout(grid);

	dialog.adjustSize();
	if (dialog.exec() == QDialog::Accepted) {
		fileWidget->slotOk();

		QString fileName = fileWidget->selectedFile();
		if (fileName.isEmpty()) {	// if entered directly and not selected (also happens when selected!)
			// DEBUG(Q_FUNC_INFO << ", no file selected")
			fileName = fileWidget->locationEdit()->currentText();
			auto* cbExtension = fileWidget->findChild<QCheckBox*>();
			if (cbExtension) {
				bool checked = cbExtension->isChecked();
				if (checked && ! (fileName.endsWith(QLatin1String(".lfd")) || fileName.endsWith(QLatin1String(".LFD"))))
							fileName.append(QLatin1String(".lfd"));
			}
			// add current folder
			auto currentDir = fileWidget->baseUrl().toLocalFile();
			fileName.prepend(currentDir);
		}
		// save current model (with description and comment)
		// FORMAT: LFD - LabPlot Function Definition
		KConfig config(fileName);	// selected lfd file
		auto group = config.group(QLatin1String("General"));
		auto description = leDescription->text();
		auto comment = leComment->text();
		group.writeEntry("Function", m_fitData.model);	// model function
		group.writeEntry("Description", description);
		group.writeEntry("Comment", comment);
		config.sync();
		QDEBUG(Q_FUNC_INFO << ", saved function to" << fileName)

		// set description and comment in Dock (even when empty)
		uiGeneralTab.cbModel->clear();
		uiGeneralTab.cbModel->addItem(description);
		uiGeneralTab.cbModel->setToolTip(comment);
	}
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

	// menu.setWindowFlags(menu.windowFlags() & Qt::MSWindowsFixedSizeDialogHint);

	QPoint pos(-menu.sizeHint().width() + uiGeneralTab.pbOptions->width(), 0);
	menu.exec(uiGeneralTab.pbOptions->mapToGlobal(pos));
}

/*!
 * When a custom evaluate range is specified, set the plot range too.
 */
/*void XYFitCurveDock::setPlotXRange() {
	if (m_fitData.autoEvalRange || !m_curve)
		return;

	auto* plot = dynamic_cast<CartesianPlot*>(m_curve->parentAspect());
	if (plot) {
		Range<double> range{ m_fitData.evalRange };
		if (!range.isZero())
			plot->setXRange(range);
	}
}*/

void XYFitCurveDock::recalculateClicked() {
	DEBUG(Q_FUNC_INFO);
	m_fitData.degree = uiGeneralTab.sbDegree->value();
	if (m_fitData.modelCategory == nsl_fit_model_custom)
		updateParameterList();

	for (XYCurve* curve : m_curvesList)
		static_cast<XYFitCurve*>(curve)->setFitData(m_fitData);

	m_fitCurve->recalculate();
	// setPlotXRange();

	if (m_fitData.useResults) { // update fitParametersWidget
		DEBUG(Q_FUNC_INFO << ", nr of param names = " << m_fitData.paramNames.size())
		DEBUG(Q_FUNC_INFO << ", size of start values = " << m_fitData.paramStartValues.size())
		DEBUG(Q_FUNC_INFO << ", size of param values = " << m_fitCurve->fitResult().paramValues.size())
		if (m_fitCurve->fitResult().paramValues.size() > 0) { // may be 0 if fit fails
			for (int i = 0; i < m_fitData.paramNames.size(); i++)
				m_fitData.paramStartValues[i] = m_fitCurve->fitResult().paramValues.at(i);
			fitParametersWidget->setFitData(&m_fitData);
		} else {
			DEBUG(Q_FUNC_INFO << ", WARNING: no fit result available!")
		}
	}

	this->showFitResult();
	uiGeneralTab.pbRecalculate->setEnabled(false);

	// show the warning/error message, if available
	const auto& fitResult = m_fitCurve->fitResult();
	const QString& status = fitResult.status;
	if (status != i18n("Success")) {
		Q_EMIT info(i18nc("Curve fitting", "Fit status: %1", fitResult.status));
		checkDataColumns();

		if (!fitResult.valid)
			m_messageWidget->setMessageType(KMessageWidget::Error);
		else
			m_messageWidget->setMessageType(KMessageWidget::Warning);

		if (!status.isEmpty())
			m_messageWidget->setText(status);
		else // only reason to get here
			m_messageWidget->setText(i18n("No data source available!"));
		m_messageWidget->animatedShow();
	} else {
		if (m_messageWidget && m_messageWidget->isVisible()) {
			DEBUG(Q_FUNC_INFO << ", close message")
			m_messageWidget->animatedHide();
		}
	}

	DEBUG(Q_FUNC_INFO << " DONE");
}

void XYFitCurveDock::expressionChanged() {
	DEBUG(Q_FUNC_INFO);
	CONDITIONAL_RETURN_NO_LOCK;

	// update parameter list for custom model
	if (m_fitData.modelCategory == nsl_fit_model_custom)
		updateParameterList();

	enableRecalculate();
}

void XYFitCurveDock::enableRecalculate() {
	DEBUG(Q_FUNC_INFO << ", m_initializing = " << m_initializing);
	if (m_initializing || !m_fitCurve)
		return;

	// no fitting possible without the x- and y-data
	bool hasSourceData = false;
	//	auto type = m_fitCurve->dataSourceType();
	switch (m_fitCurve->dataSourceType()) {
	case XYAnalysisCurve::DataSourceType::Spreadsheet: {
		auto* aspectX = static_cast<AbstractAspect*>(cbXDataColumn->currentModelIndex().internalPointer());
		auto* aspectY = static_cast<AbstractAspect*>(cbYDataColumn->currentModelIndex().internalPointer());
		hasSourceData = (aspectX && aspectY);
		if (aspectX)
			cbXDataColumn->setInvalid(false);
		if (aspectY)
			cbYDataColumn->setInvalid(false);
		break;
	}
	case XYAnalysisCurve::DataSourceType::Curve:
		hasSourceData = (m_fitCurve->dataSourceCurve() != nullptr);
		break;
	case XYAnalysisCurve::DataSourceType::Histogram:
		hasSourceData = (m_fitCurve->dataSourceHistogram() != nullptr);
	}

	DEBUG(Q_FUNC_INFO << ", hasSourceData = " << hasSourceData << ", m_parametersValid = " << m_parametersValid)
	uiGeneralTab.pbRecalculate->setEnabled(hasSourceData && m_parametersValid);

	// PREVIEW as soon as recalculate is enabled (does not need source data)
	if (m_parametersValid && m_fitData.previewEnabled) {
		DEBUG("	EVALUATE WITH PREVIEW ENABLED");
		// use recent fit data
		m_fitCurve->setFitData(m_fitData);
		// calculate fit function
		m_fitCurve->evaluate(true);
		// setPlotXRange();
	} else {
		DEBUG("	PREVIEW DISABLED");
	}
	DEBUG(Q_FUNC_INFO << " DONE");
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

	// copy the header of the parameters table if we copy everything in this table
	if (copyAll && tw == uiGeneralTab.twParameters) {
		for (int i = 1; i < tw->columnCount(); ++i)
			str += QLatin1Char('\t') + tw->horizontalHeaderItem(i)->text();
	}

	// copy the content of the table
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
	auto* contextMenu = new QMenu(this);
	contextMenu->addAction(i18n("Copy Selection"), this, &XYFitCurveDock::resultCopy, QKeySequence::Copy);
	contextMenu->addAction(i18n("Copy All"), this, &XYFitCurveDock::resultCopyAll);
	contextMenu->exec(uiGeneralTab.twParameters->mapToGlobal(pos));
}
void XYFitCurveDock::resultGoodnessContextMenuRequest(QPoint pos) {
	auto* contextMenu = new QMenu(this);
	contextMenu->addAction(i18n("Copy Selection"), this, &XYFitCurveDock::resultCopy, QKeySequence::Copy);
	contextMenu->addAction(i18n("Copy All"), this, &XYFitCurveDock::resultCopyAll);
	contextMenu->exec(uiGeneralTab.twGoodness->mapToGlobal(pos));
}
void XYFitCurveDock::resultLogContextMenuRequest(QPoint pos) {
	auto* contextMenu = new QMenu(this);
	contextMenu->addAction(i18n("Copy Selection"), this, &XYFitCurveDock::resultCopy, QKeySequence::Copy);
	contextMenu->addAction(i18n("Copy All"), this, &XYFitCurveDock::resultCopyAll);
	contextMenu->exec(uiGeneralTab.twLog->mapToGlobal(pos));
}

/*!
 * show the result and details of the fit
 */
void XYFitCurveDock::showFitResult() {
	DEBUG(Q_FUNC_INFO)
	// clear the previous result
	uiGeneralTab.twParameters->setRowCount(0);
	for (int row = 0; row < uiGeneralTab.twGoodness->rowCount(); ++row)
		uiGeneralTab.twGoodness->item(row, 1)->setText(QString());
	for (int row = 0; row < uiGeneralTab.twLog->rowCount(); ++row)
		uiGeneralTab.twLog->item(row, 1)->setText(QString());

	const auto& fitResult = m_fitCurve->fitResult();

	if (!fitResult.available) {
		DEBUG(Q_FUNC_INFO << ", fit result not available")
		checkDataColumns();
		return;
	}

	// Log
	uiGeneralTab.twLog->item(0, 1)->setText(fitResult.status);

	if (!fitResult.valid) {
		DEBUG(Q_FUNC_INFO << ", fit result not valid");
		checkDataColumns();
		return;
	} else if (m_messageWidget && m_messageWidget->isVisible()) {
		DEBUG(Q_FUNC_INFO << ", close message")
		m_messageWidget->animatedHide();
	}

	const auto numberLocale = QLocale();
	// used confidence interval
	double confidenceInterval{m_fitData.confidenceInterval};
	uiGeneralTab.twParameters->horizontalHeaderItem(6)->setToolTip(i18n("%1 % lower confidence level", numberLocale.toString(confidenceInterval, 'g', 7)));
	uiGeneralTab.twParameters->horizontalHeaderItem(7)->setToolTip(i18n("%1 % upper confidence level", numberLocale.toString(confidenceInterval, 'g', 7)));

	// log
	uiGeneralTab.twLog->item(1, 1)->setText(numberLocale.toString(fitResult.iterations));
	uiGeneralTab.twLog->item(2, 1)->setText(numberLocale.toString(m_fitData.eps));
	if (fitResult.elapsedTime > 1000)
		uiGeneralTab.twLog->item(3, 1)->setText(numberLocale.toString(fitResult.elapsedTime / 1000) + QStringLiteral(" s"));
	else
		uiGeneralTab.twLog->item(3, 1)->setText(numberLocale.toString(fitResult.elapsedTime) + QStringLiteral(" ms"));

	uiGeneralTab.twLog->item(4, 1)->setText(numberLocale.toString(fitResult.dof));
	uiGeneralTab.twLog->item(5, 1)->setText(numberLocale.toString(fitResult.paramValues.size()));
	uiGeneralTab.twLog->item(6, 1)->setText(m_fitData.fitRange.toString());

	const int np = m_fitData.paramNames.size();

	// correlation matrix
	QString sCorr;
	for (const auto& s : m_fitData.paramNamesUtf8)
		sCorr += QLatin1Char('\t') + s;
	DEBUG(Q_FUNC_INFO << ", correlation matrix size = " << fitResult.correlationMatrix.size())
	if (fitResult.correlationMatrix.size() < np * (np + 1) / 2)
		return;

	int index = 0;
	for (int i = 0; i < np; i++) {
		sCorr += QLatin1Char('\n') + m_fitData.paramNamesUtf8.at(i);
		for (int j = 0; j <= i; j++)
			sCorr += QLatin1Char('\t') + numberLocale.toString(fitResult.correlationMatrix.at(index++), 'f');
	}
	uiGeneralTab.twLog->item(7, 1)->setText(sCorr);

	// iterations
	QString sIter;
	for (const auto& s : m_fitData.paramNamesUtf8)
		sIter += s + QLatin1Char('\t');
	sIter += UTF8_QSTRING("χ²");

	const QStringList iterations = fitResult.solverOutput.split(QLatin1Char(';'));
	for (const auto& s : iterations)
		if (!s.isEmpty())
			sIter += QLatin1Char('\n') + s;
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
				item = new QTableWidgetItem(numberLocale.toString(100. * errorValue / std::abs(paramValue), 'g', 3));
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
				const double marginLow = fitResult.marginValues.at(i);
				// TODO: if (fitResult.tdist_tValues.at(i) > 1.e6)
				//	item = new QTableWidgetItem(i18n("too small"));
				double marginHigh = marginLow;
				if (fitResult.marginValues.size() >= i && fitResult.marginValues.at(i) != 0.)
					marginHigh = fitResult.marginValues.at(i);

				item = new QTableWidgetItem(numberLocale.toString(paramValue - marginLow));
				uiGeneralTab.twParameters->setItem(i, 6, item);
				item = new QTableWidgetItem(numberLocale.toString(paramValue + marginHigh));
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

	// resize the table headers to fit the new content
	uiGeneralTab.twLog->resizeColumnsToContents();
	uiGeneralTab.twParameters->resizeColumnsToContents();
	// twGoodness doesn't have any header -> resize sections
	uiGeneralTab.twGoodness->resizeColumnToContents(0);
	uiGeneralTab.twGoodness->resizeColumnToContents(1);

	// enable the "recalculate"-button if the source data was changed since the last fit
	uiGeneralTab.pbRecalculate->setEnabled(m_fitCurve->isSourceDataChangedSinceLastRecalc());
}

//*************************************************************
//*********** SLOTs for changes triggered in XYCurve **********
//*************************************************************
// General-Tab
void XYFitCurveDock::curveDataSourceTypeChanged(XYAnalysisCurve::DataSourceType type) {
	CONDITIONAL_LOCK_RETURN;
	uiGeneralTab.cbDataSourceType->setCurrentIndex(static_cast<int>(type));
}

void XYFitCurveDock::curveDataSourceCurveChanged(const XYCurve* curve) {
	CONDITIONAL_LOCK_RETURN;
	cbDataSourceCurve->setAspect(curve);
}

void XYFitCurveDock::curveDataSourceHistogramChanged(const Histogram* hist) {
	CONDITIONAL_LOCK_RETURN;
	cbDataSourceCurve->setAspect(hist);
}

void XYFitCurveDock::curveXDataColumnChanged(const AbstractColumn* column) {
	CONDITIONAL_LOCK_RETURN;
	cbXDataColumn->setAspect(column, m_fitCurve->xDataColumnPath());
}

void XYFitCurveDock::curveYDataColumnChanged(const AbstractColumn* column) {
	CONDITIONAL_LOCK_RETURN;
	cbYDataColumn->setAspect(column, m_fitCurve->yDataColumnPath());
}

void XYFitCurveDock::curveXErrorColumnChanged(const AbstractColumn* column) {
	CONDITIONAL_LOCK_RETURN;
	cbXErrorColumn->setAspect(column, m_fitCurve->xErrorColumnPath());
}

void XYFitCurveDock::curveYErrorColumnChanged(const AbstractColumn* column) {
	CONDITIONAL_LOCK_RETURN;
	cbYErrorColumn->setAspect(column, m_fitCurve->yErrorColumnPath());
}

/*!
 * called when fit data of fit curve changes
 */
void XYFitCurveDock::curveFitDataChanged(const XYFitCurve::FitData& fitData) {
	CONDITIONAL_LOCK_RETURN;
	m_fitData = fitData;

	if (m_fitData.modelCategory != nsl_fit_model_custom)
		uiGeneralTab.cbModel->setCurrentIndex(m_fitData.modelType);

	uiGeneralTab.sbDegree->setValue(m_fitData.degree);
}

void XYFitCurveDock::dataChanged() {
	this->enableRecalculate();
}
