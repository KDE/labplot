/*
	File                 : XYCurveDock.cpp
	Project              : LabPlot
	Description          : widget for XYCurve properties
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2010-2025 Alexander Semke <alexander.semke@web.de>
	SPDX-FileCopyrightText: 2012-2022 Stefan Gerlach <stefan.gerlach@uni-konstanz.de>

	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "XYCurveDock.h"
#include "backend/core/Project.h"
#include "backend/core/Settings.h"
#include "backend/core/column/Column.h"
#include "backend/core/datatypes/DateTime2StringFilter.h"
#include "backend/core/datatypes/Double2StringFilter.h"
#include "backend/worksheet/Worksheet.h"
#include "backend/worksheet/plots/cartesian/XYCurve.h"
#include "frontend/GuiTools.h"
#include "frontend/TemplateHandler.h"
#include "frontend/widgets/BackgroundWidget.h"
#include "frontend/widgets/ErrorBarWidget.h"
#include "frontend/widgets/LineWidget.h"
#include "frontend/widgets/SymbolWidget.h"
#include "frontend/widgets/TreeViewComboBox.h"

#include <QPainter>

#include <KConfig>
#include <KConfigGroup>
#include <KLocalizedString>

/*!
  \class XYCurveDock
  \brief  Provides a widget for editing the properties of the XYCurves (2D-curves) currently selected in the project explorer.

  If more than one curves are set, the properties of the first column are shown. The changes of the properties are applied to all curves.
  The exclusions are the name, the comment and the datasets (columns) of the curves  - these properties can only be changed if there is only one single curve.

  \ingroup frontend
*/

XYCurveDock::XYCurveDock(QWidget* parent)
	: BaseDock(parent) {
	ui.setupUi(this);

	// Tab "Line"
	auto* gridLayout = qobject_cast<QGridLayout*>(ui.tabLine->layout());
	lineWidget = new LineWidget(ui.tabLine);
	gridLayout->addWidget(lineWidget, 5, 0, 1, 3);

	dropLineWidget = new LineWidget(ui.tabLine);
	gridLayout->addWidget(dropLineWidget, 8, 0, 1, 3);

	// Tab "Symbol"
	auto* hboxLayout = new QHBoxLayout(ui.tabSymbol);
	symbolWidget = new SymbolWidget(ui.tabSymbol);
	hboxLayout->addWidget(symbolWidget);
	hboxLayout->setContentsMargins(2, 2, 2, 2);
	hboxLayout->setSpacing(2);

	// Tab "Values"
	gridLayout = qobject_cast<QGridLayout*>(ui.tabValues->layout());
	cbValuesColumn = new TreeViewComboBox(ui.tabValues);
	gridLayout->addWidget(cbValuesColumn, 2, 2, 1, 1);

	// add format for date, time and datetime values
	for (const auto& s : AbstractColumn::dateTimeFormats())
		ui.cbValuesDateTimeFormat->addItem(s, QVariant(s));

	ui.cbValuesDateTimeFormat->setEditable(true);

	// Tab "Filling"
	auto* layout = static_cast<QHBoxLayout*>(ui.tabAreaFilling->layout());
	backgroundWidget = new BackgroundWidget(ui.tabAreaFilling);
	layout->insertWidget(0, backgroundWidget);

	// Tab "Error Bars"
	const KConfigGroup group = Settings::group(QStringLiteral("Settings_General"));
	if (group.readEntry(QStringLiteral("GUMTerms"), false))
		ui.tabWidget->setTabText(ui.tabWidget->indexOf(ui.tabErrorBars), i18n("Uncertainty Bars"));

	errorBarWidget = new ErrorBarWidget(ui.tabErrorBars);
	auto* vLayout = qobject_cast<QVBoxLayout*>(ui.tabErrorBars->layout());
	vLayout->insertWidget(0, errorBarWidget);

	// adjust layouts in the tabs
	for (int i = 0; i < ui.tabWidget->count(); ++i) {
		auto* layout = dynamic_cast<QGridLayout*>(ui.tabWidget->widget(i)->layout());
		if (!layout)
			continue;

		layout->setContentsMargins(2, 2, 2, 2);
		layout->setHorizontalSpacing(2);
		layout->setVerticalSpacing(2);
	}

	updateLocale();
	retranslateUi();
	init();

	// Slots

	// Lines
	connect(ui.cbLineType, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &XYCurveDock::lineTypeChanged);
	connect(ui.sbLineInterpolationPointsCount, QOverload<int>::of(&QSpinBox::valueChanged), this, &XYCurveDock::lineInterpolationPointsCountChanged);
	connect(ui.chkLineSkipGaps, &QCheckBox::clicked, this, &XYCurveDock::lineSkipGapsChanged);
	connect(ui.chkLineIncreasingXOnly, &QCheckBox::clicked, this, &XYCurveDock::lineIncreasingXOnlyChanged);

	// Values
	connect(ui.cbValuesType, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &XYCurveDock::valuesTypeChanged);
	connect(cbValuesColumn, &TreeViewComboBox::currentModelIndexChanged, this, &XYCurveDock::valuesColumnChanged);
	connect(ui.cbValuesPosition, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &XYCurveDock::valuesPositionChanged);
	connect(ui.sbValuesDistance, QOverload<double>::of(&NumberSpinBox::valueChanged), this, &XYCurveDock::valuesDistanceChanged);
	connect(ui.sbValuesRotation, QOverload<int>::of(&QSpinBox::valueChanged), this, &XYCurveDock::valuesRotationChanged);
	connect(ui.sbValuesOpacity, QOverload<int>::of(&QSpinBox::valueChanged), this, &XYCurveDock::valuesOpacityChanged);
	connect(ui.cbValuesNumericFormat, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &XYCurveDock::valuesNumericFormatChanged);
	connect(ui.sbValuesPrecision, QOverload<int>::of(&QSpinBox::valueChanged), this, &XYCurveDock::valuesPrecisionChanged);
	connect(ui.cbValuesDateTimeFormat, &QComboBox::currentTextChanged, this, &XYCurveDock::valuesDateTimeFormatChanged);
	connect(ui.leValuesPrefix, &QLineEdit::textChanged, this, &XYCurveDock::valuesPrefixChanged);
	connect(ui.leValuesSuffix, &QLineEdit::textChanged, this, &XYCurveDock::valuesSuffixChanged);
	connect(ui.kfrValuesFont, &KFontRequester::fontSelected, this, &XYCurveDock::valuesFontChanged);
	connect(ui.kcbValuesColor, &KColorButton::changed, this, &XYCurveDock::valuesColorChanged);

	// Margin Plots
	connect(ui.chkRugEnabled, &QCheckBox::toggled, this, &XYCurveDock::rugEnabledChanged);
	connect(ui.cbRugOrientation, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &XYCurveDock::rugOrientationChanged);
	connect(ui.sbRugLength, QOverload<double>::of(&NumberSpinBox::valueChanged), this, &XYCurveDock::rugLengthChanged);
	connect(ui.sbRugWidth, QOverload<double>::of(&NumberSpinBox::valueChanged), this, &XYCurveDock::rugWidthChanged);
	connect(ui.sbRugOffset, QOverload<double>::of(&NumberSpinBox::valueChanged), this, &XYCurveDock::rugOffsetChanged);

	// template handler
	auto* frame = new QFrame(this);
	layout = new QHBoxLayout(frame);
	layout->setContentsMargins(0, 0, 0, 0);

	auto* templateHandler = new TemplateHandler(this, QLatin1String("XYCurve"));
	layout->addWidget(templateHandler);
	connect(templateHandler, &TemplateHandler::loadConfigRequested, this, &XYCurveDock::loadConfigFromTemplate);
	connect(templateHandler, &TemplateHandler::saveConfigRequested, this, &XYCurveDock::saveConfigAsTemplate);
	connect(templateHandler, &TemplateHandler::info, this, &XYCurveDock::info);

	ui.verticalLayout->addWidget(frame);
}

XYCurveDock::~XYCurveDock() {
	delete m_valuesModel;
}

void XYCurveDock::setupGeneral() {
	auto* generalTab = new QWidget(ui.tabGeneral);
	uiGeneralTab.setupUi(generalTab);
	setPlotRangeCombobox(uiGeneralTab.cbPlotRanges);
	setBaseWidgets(uiGeneralTab.leName, uiGeneralTab.teComment);
	setVisibilityWidgets(uiGeneralTab.chkVisible, uiGeneralTab.chkLegendVisible);

	auto* layout = new QHBoxLayout(ui.tabGeneral);
	layout->setContentsMargins(0, 0, 0, 0);
	layout->addWidget(generalTab);

	// Tab "General" (see xycurvedockgeneraltab.ui)
	auto* gridLayout = qobject_cast<QGridLayout*>(generalTab->layout());

	cbXColumn = new TreeViewComboBox(generalTab);
	gridLayout->addWidget(cbXColumn, 4, 2, 1, 1);

	cbYColumn = new TreeViewComboBox(generalTab);
	gridLayout->addWidget(cbYColumn, 5, 2, 1, 1);

	// General
	connect(cbXColumn, &TreeViewComboBox::currentModelIndexChanged, this, &XYCurveDock::xColumnChanged);
	connect(cbYColumn, &TreeViewComboBox::currentModelIndexChanged, this, &XYCurveDock::yColumnChanged);
}

void XYCurveDock::init() {
	QPainter pa;
	// TODO size of the icon depending on the actual height of the combobox?
	int iconSize = 20;
	QPixmap pm(iconSize, iconSize);
	ui.cbLineType->setIconSize(QSize(iconSize, iconSize));

	QPen pen(Qt::SolidPattern, 0);
	const QColor& color = GuiTools::isDarkMode() ? Qt::white : Qt::black;
	pen.setColor(color);

	// no line
	pm.fill(Qt::transparent);
	pa.begin(&pm);
	pa.setPen(pen);
	pa.setBrush(QBrush(pen.color()));
	pa.setRenderHint(QPainter::Antialiasing);
	pa.drawEllipse(1, 1, 4, 4);
	pa.drawEllipse(15, 15, 4, 4);
	pa.end();
	ui.cbLineType->setItemIcon(0, pm);

	// line
	pm.fill(Qt::transparent);
	pa.begin(&pm);
	pa.setPen(pen);
	pa.setBrush(QBrush(pen.color()));
	pa.setRenderHint(QPainter::Antialiasing);
	pa.drawEllipse(1, 1, 4, 4);
	pa.drawEllipse(15, 15, 4, 4);
	pa.drawLine(3, 3, 17, 17);
	pa.end();
	ui.cbLineType->setItemIcon(1, pm);

	// horizontal start
	pm.fill(Qt::transparent);
	pa.begin(&pm);
	pa.setPen(pen);
	pa.setBrush(QBrush(pen.color()));
	pa.setRenderHint(QPainter::Antialiasing);
	pa.drawEllipse(1, 1, 4, 4);
	pa.drawEllipse(15, 15, 4, 4);
	pa.drawLine(3, 3, 17, 3);
	pa.drawLine(17, 3, 17, 17);
	pa.end();
	ui.cbLineType->setItemIcon(2, pm);

	// vertical start
	pm.fill(Qt::transparent);
	pa.begin(&pm);
	pa.setPen(pen);
	pa.setBrush(QBrush(pen.color()));
	pa.setRenderHint(QPainter::Antialiasing);
	pa.drawEllipse(1, 1, 4, 4);
	pa.drawEllipse(15, 15, 4, 4);
	pa.drawLine(3, 3, 3, 17);
	pa.drawLine(3, 17, 17, 17);
	pa.end();
	ui.cbLineType->setItemIcon(3, pm);

	// horizontal midpoint
	pm.fill(Qt::transparent);
	pa.begin(&pm);
	pa.setPen(pen);
	pa.setBrush(QBrush(pen.color()));
	pa.setRenderHint(QPainter::Antialiasing);
	pa.drawEllipse(1, 1, 4, 4);
	pa.drawEllipse(15, 15, 4, 4);
	pa.drawLine(3, 3, 10, 3);
	pa.drawLine(10, 3, 10, 17);
	pa.drawLine(10, 17, 17, 17);
	pa.end();
	ui.cbLineType->setItemIcon(4, pm);

	// vertical midpoint
	pm.fill(Qt::transparent);
	pa.begin(&pm);
	pa.setPen(pen);
	pa.setBrush(QBrush(pen.color()));
	pa.setRenderHint(QPainter::Antialiasing);
	pa.drawEllipse(1, 1, 4, 4);
	pa.drawEllipse(15, 15, 4, 4);
	pa.drawLine(3, 3, 3, 10);
	pa.drawLine(3, 10, 17, 10);
	pa.drawLine(17, 10, 17, 17);
	pa.end();
	ui.cbLineType->setItemIcon(5, pm);

	// 2-segments
	pm.fill(Qt::transparent);
	pa.begin(&pm);
	pa.setPen(pen);
	pa.setBrush(QBrush(pen.color()));
	pa.setRenderHint(QPainter::Antialiasing);
	pa.drawEllipse(1, 1, 4, 4);
	pa.drawEllipse(8, 8, 4, 4);
	pa.drawEllipse(15, 15, 4, 4);
	pa.drawLine(3, 3, 10, 10);
	pa.end();
	ui.cbLineType->setItemIcon(6, pm);

	// 3-segments
	pm.fill(Qt::transparent);
	pa.begin(&pm);
	pa.setPen(pen);
	pa.setBrush(QBrush(pen.color()));
	pa.setRenderHint(QPainter::Antialiasing);
	pa.drawEllipse(1, 1, 4, 4);
	pa.drawEllipse(8, 8, 4, 4);
	pa.drawEllipse(15, 15, 4, 4);
	pa.drawLine(3, 3, 17, 17);
	pa.end();
	ui.cbLineType->setItemIcon(7, pm);

	// natural spline
	pm.fill(Qt::transparent);
	pa.begin(&pm);
	pa.setPen(pen);
	pa.setBrush(QBrush(pen.color()));
	pa.setRenderHint(QPainter::Antialiasing);
	pa.drawEllipse(1, 1, 4, 4);
	pa.drawEllipse(15, 15, 4, 4);
	pa.rotate(45);
	pa.drawArc(2 * sqrt(2), -4, 17 * sqrt(2), 20, 30 * 16, 120 * 16);
	pa.end();
	ui.cbLineType->setItemIcon(8, pm);
	ui.cbLineType->setItemIcon(9, pm);
	ui.cbLineType->setItemIcon(10, pm);
	ui.cbLineType->setItemIcon(11, pm);
}

void XYCurveDock::setModel() {
	auto* model = aspectModel();
	model->enablePlottableColumnsOnly(true);
	model->enableShowPlotDesignation(true);

	auto list = TreeViewComboBox::plotColumnTopLevelClasses();
	if (cbXColumn && cbYColumn) {
		cbXColumn->setTopLevelClasses(list);
		cbYColumn->setTopLevelClasses(list);
	}
	cbValuesColumn->setTopLevelClasses(list);

	if (m_curve->inherits(AspectType::XYAnalysisCurve))
		// the model is used in the combobox for curve data sources -> allow to also select analysis curves
		list = {AspectType::Column,
				AspectType::XYCurve,
				AspectType::XYFitCurve,
				AspectType::XYIntegrationCurve,
				AspectType::XYInterpolationCurve,
				AspectType::XYSmoothCurve,
				AspectType::XYFourierFilterCurve,
				AspectType::XYFourierTransformCurve,
				AspectType::XYConvolutionCurve,
				AspectType::XYCorrelationCurve,
				AspectType::XYDataReductionCurve,
				AspectType::XYEquationCurve,
				AspectType::XYFunctionCurve};
	else
		list = {AspectType::Column};

	model->setSelectableAspects(list);

	if (cbXColumn && cbYColumn) {
		cbXColumn->setModel(model);
		cbYColumn->setModel(model);
	}

	// for value labels we need a dedicated model since we also want to allow
	// to select text columns and we don't want to call enablePlottableColumnsOnly().
	if (!m_valuesModel) {
		m_valuesModel = new AspectTreeModel(m_curve->project());
		m_valuesModel->setSelectableAspects(list);
	}
	cbValuesColumn->setModel(m_valuesModel);

	errorBarWidget->setModel(model);

	// this function is called after the dock widget is initialized and the curves are set.
	// so, we use this function to finalize the initialization even though it's not related
	// to the actual set of the model (could also be solved by a new class XYAnalysisiCurveDock).

	// hide property widgets that are not relevant for analysis curves
	bool visible = (m_curve->type() == AspectType::XYCurve);
	ui.lLineType->setVisible(visible);
	ui.cbLineType->setVisible(visible);
	ui.lLineSkipGaps->setVisible(visible);
	ui.chkLineSkipGaps->setVisible(visible);
	ui.lLineIncreasingXOnly->setVisible(visible);
	ui.chkLineIncreasingXOnly->setVisible(visible);

	// if it's not a xy-curve, it's an analysis curve and the line widget is always enables since we always draw the line
	if (!visible)
		lineWidget->setEnabled(true);

	// remove the tab "Error bars" for analysis curves
	if (!visible)
		ui.tabWidget->removeTab(5);
}

/*!
  sets the curves. The properties of the curves in the list \c list can be edited in this widget.
*/
void XYCurveDock::setCurves(QList<XYCurve*> list) {
	CONDITIONAL_LOCK_RETURN;
	m_curvesList = list;
	m_curve = list.first();
	setAspects(list);
	Q_ASSERT(m_curve);
	setModel();
	initGeneralTab();
	initTabs();
	setSymbols(list);
}

void XYCurveDock::setSymbols(QList<XYCurve*> curves) {
	// symbols
	QList<Symbol*> symbols;
	QList<Background*> backgrounds;
	QList<Line*> lines;
	QList<Line*> dropLines;
	QList<ErrorBar*> errorBars;
	for (auto* curve : curves) {
		symbols << curve->symbol();
		backgrounds << curve->background();
		lines << curve->line();
		dropLines << curve->dropLine();
		errorBars << curve->errorBar();
	}

	symbolWidget->setSymbols(symbols);
	backgroundWidget->setBackgrounds(backgrounds);
	lineWidget->setLines(lines);
	dropLineWidget->setLines(dropLines);
	errorBarWidget->setErrorBars(errorBars);
}

void XYCurveDock::initGeneralTab() {
	// show the properties of the first curve
	cbXColumn->setAspect(m_curve->xColumn(), m_curve->xColumnPath());
	cbYColumn->setAspect(m_curve->yColumn(), m_curve->yColumnPath());
	uiGeneralTab.chkLegendVisible->setChecked(m_curve->legendVisible());
	uiGeneralTab.chkVisible->setChecked(m_curve->isVisible());

	updatePlotRangeList();

	// Slots
	connect(m_curve, &XYCurve::xColumnChanged, this, &XYCurveDock::curveXColumnChanged);
	connect(m_curve, &XYCurve::yColumnChanged, this, &XYCurveDock::curveYColumnChanged);
}

void XYCurveDock::initTabs() {
	// if there are more than one curve in the list, disable the tab "general"
	if (m_curvesList.size() == 1)
		cbValuesColumn->setAspect(m_curve->valuesColumn(), m_curve->valuesColumnPath());
	else
		cbValuesColumn->setCurrentModelIndex(QModelIndex());

	// show the properties of the first curve
	load();

	// Slots

	// Line-Tab
	connect(m_curve, &XYCurve::lineTypeChanged, this, &XYCurveDock::curveLineTypeChanged);
	connect(m_curve, &XYCurve::lineSkipGapsChanged, this, &XYCurveDock::curveLineSkipGapsChanged);
	connect(m_curve, &XYCurve::lineIncreasingXOnlyChanged, this, &XYCurveDock::curveLineIncreasingXOnlyChanged);
	connect(m_curve, &XYCurve::lineInterpolationPointsCountChanged, this, &XYCurveDock::curveLineInterpolationPointsCountChanged);

	// Values-Tab
	connect(m_curve, &XYCurve::valuesTypeChanged, this, &XYCurveDock::curveValuesTypeChanged);
	connect(m_curve, &XYCurve::valuesColumnChanged, this, &XYCurveDock::curveValuesColumnChanged);
	connect(m_curve, &XYCurve::valuesPositionChanged, this, &XYCurveDock::curveValuesPositionChanged);
	connect(m_curve, &XYCurve::valuesDistanceChanged, this, &XYCurveDock::curveValuesDistanceChanged);
	connect(m_curve, &XYCurve::valuesOpacityChanged, this, &XYCurveDock::curveValuesOpacityChanged);
	connect(m_curve, &XYCurve::valuesRotationAngleChanged, this, &XYCurveDock::curveValuesRotationAngleChanged);
	connect(m_curve, &XYCurve::valuesNumericFormatChanged, this, &XYCurveDock::curveValuesNumericFormatChanged);
	connect(m_curve, &XYCurve::valuesPrecisionChanged, this, &XYCurveDock::curveValuesPrecisionChanged);
	connect(m_curve, &XYCurve::valuesDateTimeFormatChanged, this, &XYCurveDock::curveValuesDateTimeFormatChanged);
	connect(m_curve, &XYCurve::valuesPrefixChanged, this, &XYCurveDock::curveValuesPrefixChanged);
	connect(m_curve, &XYCurve::valuesSuffixChanged, this, &XYCurveDock::curveValuesSuffixChanged);
	connect(m_curve, &XYCurve::valuesFontChanged, this, &XYCurveDock::curveValuesFontChanged);
	connect(m_curve, &XYCurve::valuesColorChanged, this, &XYCurveDock::curveValuesColorChanged);

	//"Margin Plots"-Tab
	connect(m_curve, &XYCurve::rugEnabledChanged, this, &XYCurveDock::curveRugEnabledChanged);
	connect(m_curve, &XYCurve::rugOrientationChanged, this, &XYCurveDock::curveRugOrientationChanged);
	connect(m_curve, &XYCurve::rugLengthChanged, this, &XYCurveDock::curveRugLengthChanged);
	connect(m_curve, &XYCurve::rugWidthChanged, this, &XYCurveDock::curveRugWidthChanged);
	connect(m_curve, &XYCurve::rugOffsetChanged, this, &XYCurveDock::curveRugOffsetChanged);
}

void XYCurveDock::updateLocale() {
	ui.sbValuesDistance->setLocale(QLocale());
	lineWidget->updateLocale();
	dropLineWidget->updateLocale();
	symbolWidget->updateLocale();
	errorBarWidget->updateLocale();
}

//*************************************************************
//********** SLOTs for changes triggered in XYCurveDock ********
//*************************************************************
void XYCurveDock::retranslateUi() {
	CONDITIONAL_LOCK_RETURN;

	// Line
	ui.cbLineType->clear();
	ui.cbLineType->addItem(i18n("None"));
	ui.cbLineType->addItem(i18n("Line"));
	ui.cbLineType->addItem(i18n("Horiz. Start"));
	ui.cbLineType->addItem(i18n("Vert. Start"));
	ui.cbLineType->addItem(i18n("Horiz. Midpoint"));
	ui.cbLineType->addItem(i18n("Vert. Midpoint"));
	ui.cbLineType->addItem(i18n("2-segments"));
	ui.cbLineType->addItem(i18n("3-segments"));
	ui.cbLineType->addItem(i18n("Cubic Spline (Natural)"));
	ui.cbLineType->addItem(i18n("Cubic Spline (Periodic)"));
	ui.cbLineType->addItem(i18n("Akima-spline (Natural)"));
	ui.cbLineType->addItem(i18n("Akima-spline (Periodic)"));


	// formats for numeric values
	ui.cbValuesNumericFormat->clear();
	ui.cbValuesNumericFormat->addItem(i18n("Decimal"), QVariant('f'));
	ui.cbValuesNumericFormat->addItem(i18n("Scientific (e)"), QVariant('e'));
	ui.cbValuesNumericFormat->addItem(i18n("Scientific (E)"), QVariant('E'));
	ui.cbValuesNumericFormat->addItem(i18n("Automatic (e)"), QVariant('g'));
	ui.cbValuesNumericFormat->addItem(i18n("Automatic (E)"), QVariant('G'));

	// Values
	ui.cbValuesType->clear();
	ui.cbValuesType->addItem(i18n("No Values"));
	ui.cbValuesType->addItem(QStringLiteral("x"));
	ui.cbValuesType->addItem(QStringLiteral("y"));
	ui.cbValuesType->addItem(QStringLiteral("x, y"));
	ui.cbValuesType->addItem(QStringLiteral("(x, y)"));
	ui.cbValuesType->addItem(i18n("Custom Column"));

	ui.cbValuesPosition->clear();
	ui.cbValuesPosition->addItem(i18n("Above"));
	ui.cbValuesPosition->addItem(i18n("Below"));
	ui.cbValuesPosition->addItem(i18n("Left"));
	ui.cbValuesPosition->addItem(i18n("Right"));

	// Margin Plots
	ui.cbRugOrientation->clear();
	ui.cbRugOrientation->addItem(i18n("Vertical"));
	ui.cbRugOrientation->addItem(i18n("Horizontal"));
	ui.cbRugOrientation->addItem(i18n("Both"));

	// tooltip texts
	ui.lLineSkipGaps->setToolTip(i18n("If checked, connect neighbour points with lines even if there are gaps (invalid or masked values) between them"));
	ui.chkLineSkipGaps->setToolTip(i18n("If checked, connect neighbour points with lines even if there are gaps (invalid or masked values) between them"));
	ui.lLineIncreasingXOnly->setToolTip(i18n("If checked, connect data points only for strictly increasing values of X"));
	ui.chkLineIncreasingXOnly->setToolTip(i18n("If checked, connect data points only for strictly increasing values of X"));

	// TODO updatePenStyles, updateBrushStyles for all comboboxes
}

void XYCurveDock::xColumnChanged(const QModelIndex& index) {
	updateValuesWidgets();

	CONDITIONAL_LOCK_RETURN;

	auto* aspect = static_cast<AbstractAspect*>(index.internalPointer());
	AbstractColumn* column = nullptr;
	if (aspect) {
		column = dynamic_cast<AbstractColumn*>(aspect);
		Q_ASSERT(column);
	}

	for (auto* curve : m_curvesList)
		curve->setXColumn(column);
}

void XYCurveDock::yColumnChanged(const QModelIndex& index) {
	updateValuesWidgets();

	CONDITIONAL_LOCK_RETURN;

	auto* aspect = static_cast<AbstractAspect*>(index.internalPointer());
	AbstractColumn* column = nullptr;
	if (aspect) {
		column = dynamic_cast<AbstractColumn*>(aspect);
		Q_ASSERT(column);
	}

	for (auto* curve : m_curvesList)
		curve->setYColumn(column);
}

// "Line"-tab
void XYCurveDock::lineTypeChanged(int index) {
	const auto lineType = XYCurve::LineType(index);

	if (lineType == XYCurve::LineType::NoLine) {
		ui.chkLineSkipGaps->setEnabled(false);
		lineWidget->setEnabled(false);
		ui.lLineInterpolationPointsCount->hide();
		ui.sbLineInterpolationPointsCount->hide();
	} else {
		ui.chkLineSkipGaps->setEnabled(true);
		lineWidget->setEnabled(true);

		if (lineType == XYCurve::LineType::SplineCubicNatural || lineType == XYCurve::LineType::SplineCubicPeriodic
			|| lineType == XYCurve::LineType::SplineAkimaNatural || lineType == XYCurve::LineType::SplineAkimaPeriodic) {
			ui.lLineInterpolationPointsCount->show();
			ui.sbLineInterpolationPointsCount->show();
			ui.lLineSkipGaps->hide();
			ui.chkLineSkipGaps->hide();
		} else {
			ui.lLineInterpolationPointsCount->hide();
			ui.sbLineInterpolationPointsCount->hide();
			ui.lLineSkipGaps->show();
			ui.chkLineSkipGaps->show();
		}
	}

	CONDITIONAL_LOCK_RETURN;

	for (auto* curve : m_curvesList)
		curve->setLineType(lineType);
}

void XYCurveDock::lineSkipGapsChanged(bool skip) {
	CONDITIONAL_LOCK_RETURN;

	for (auto* curve : m_curvesList)
		curve->setLineSkipGaps(skip);
}

void XYCurveDock::lineIncreasingXOnlyChanged(bool incr) {
	CONDITIONAL_LOCK_RETURN;

	for (auto* curve : m_curvesList)
		curve->setLineIncreasingXOnly(incr);
}

void XYCurveDock::lineInterpolationPointsCountChanged(int count) {
	CONDITIONAL_LOCK_RETURN;

	for (auto* curve : m_curvesList)
		curve->setLineInterpolationPointsCount(count);
}

// Values-tab

/*!
  called when the type of the values (none, x, y, (x,y) etc.) was changed.
*/
void XYCurveDock::valuesTypeChanged(int index) {
	this->updateValuesWidgets();

	CONDITIONAL_LOCK_RETURN;

	const auto type = XYCurve::ValuesType(index);
	for (auto* curve : m_curvesList)
		curve->setValuesType(type);
}

/*!
  called when the custom column for the values was changed.
*/
void XYCurveDock::valuesColumnChanged(const QModelIndex& index) {
	this->updateValuesWidgets();

	CONDITIONAL_LOCK_RETURN;

	auto* column = static_cast<Column*>(index.internalPointer());
	for (auto* curve : m_curvesList)
		curve->setValuesColumn(column);
}

/*!
  shows the formatting properties of the column \c column.
  Called, when a new column for the values was selected - either by changing the type of the values (none, x, y, etc.) or
  by selecting a new custom column for the values.
*/

/*!
  depending on the currently selected values column type (column mode) updates the widgets for the values column format,
  shows/hides the allowed widgets, fills the corresponding combobox with the possible entries.
  Called when the values column was changed.
*/
void XYCurveDock::updateValuesWidgets() {
	const auto type{XYCurve::ValuesType(ui.cbValuesType->currentIndex())};
	bool showValues{type != XYCurve::ValuesType::NoValues};

	ui.cbValuesPosition->setEnabled(showValues);
	ui.sbValuesDistance->setEnabled(showValues);
	ui.sbValuesRotation->setEnabled(showValues);
	ui.sbValuesOpacity->setEnabled(showValues);
	ui.kfrValuesFont->setEnabled(showValues);
	ui.kcbValuesColor->setEnabled(showValues);

	bool hasInteger = false;
	bool hasNumeric = false;
	bool hasDateTime = false;

	if (type == XYCurve::ValuesType::CustomColumn) {
		ui.lValuesColumn->show();
		cbValuesColumn->show();

		auto* column = static_cast<Column*>(cbValuesColumn->currentModelIndex().internalPointer());
		if (column) {
			if (column->columnMode() == AbstractColumn::ColumnMode::Double)
				hasNumeric = true;
			else if (column->columnMode() == AbstractColumn::ColumnMode::Integer || column->columnMode() == AbstractColumn::ColumnMode::BigInt)
				hasInteger = true;
			else if (column->columnMode() == AbstractColumn::ColumnMode::DateTime)
				hasDateTime = true;
		}
	} else {
		ui.lValuesColumn->hide();
		cbValuesColumn->hide();

		const AbstractColumn* xColumn = nullptr;
		const AbstractColumn* yColumn = nullptr;
		switch (type) {
		case XYCurve::ValuesType::NoValues:
			break;
		case XYCurve::ValuesType::X:
			xColumn = m_curve->xColumn();
			break;
		case XYCurve::ValuesType::Y:
			yColumn = m_curve->yColumn();
			break;
		case XYCurve::ValuesType::XY:
		case XYCurve::ValuesType::XYBracketed:
			xColumn = m_curve->xColumn();
			yColumn = m_curve->yColumn();
			break;
		case XYCurve::ValuesType::CustomColumn:
			break;
		}

		hasInteger = (xColumn && (xColumn->columnMode() == AbstractColumn::ColumnMode::Integer || xColumn->columnMode() == AbstractColumn::ColumnMode::BigInt))
			|| (yColumn && (yColumn->columnMode() == AbstractColumn::ColumnMode::Integer || yColumn->columnMode() == AbstractColumn::ColumnMode::BigInt));

		hasNumeric = (xColumn && xColumn->columnMode() == AbstractColumn::ColumnMode::Double)
			|| (yColumn && yColumn->columnMode() == AbstractColumn::ColumnMode::Double);

		hasDateTime = (xColumn && xColumn->columnMode() == AbstractColumn::ColumnMode::DateTime)
			|| (yColumn && yColumn->columnMode() == AbstractColumn::ColumnMode::DateTime);
	}

	// hide all the format related widgets first and
	// then show only what is required depending of the column mode(s)
	ui.lValuesFormat->hide();
	ui.lValuesNumericFormat->hide();
	ui.cbValuesNumericFormat->hide();
	ui.lValuesPrecision->hide();
	ui.sbValuesPrecision->hide();
	ui.lValuesDateTimeFormat->hide();
	ui.cbValuesDateTimeFormat->hide();

	if (hasNumeric || hasInteger) {
		ui.lValuesFormat->show();
		ui.lValuesNumericFormat->show();
		ui.cbValuesNumericFormat->show();
	}

	// precision is only available for Numeric
	if (hasNumeric) {
		ui.lValuesPrecision->show();
		ui.sbValuesPrecision->show();
	}

	if (hasDateTime) {
		ui.lValuesFormat->show();
		ui.lValuesDateTimeFormat->show();
		ui.cbValuesDateTimeFormat->show();
	}
}

void XYCurveDock::valuesPositionChanged(int index) {
	CONDITIONAL_LOCK_RETURN;

	for (auto* curve : m_curvesList)
		curve->setValuesPosition(XYCurve::ValuesPosition(index));
}

void XYCurveDock::valuesDistanceChanged(double value) {
	CONDITIONAL_RETURN_NO_LOCK;

	for (auto* curve : m_curvesList)
		curve->setValuesDistance(Worksheet::convertToSceneUnits(value, Worksheet::Unit::Point));
}

void XYCurveDock::valuesRotationChanged(int value) {
	CONDITIONAL_LOCK_RETURN;

	for (auto* curve : m_curvesList)
		curve->setValuesRotationAngle(value);
}

void XYCurveDock::valuesOpacityChanged(int value) {
	CONDITIONAL_LOCK_RETURN;

	qreal opacity = (float)value / 100.;
	for (auto* curve : m_curvesList)
		curve->setValuesOpacity(opacity);
}

void XYCurveDock::valuesNumericFormatChanged(int index) {
	CONDITIONAL_LOCK_RETURN;

	char format = ui.cbValuesNumericFormat->itemData(index).toChar().toLatin1();
	for (auto* curve : m_curvesList)
		curve->setValuesNumericFormat(format);
}

void XYCurveDock::valuesDateTimeFormatChanged(const QString& format) {
	CONDITIONAL_LOCK_RETURN;

	for (auto* curve : m_curvesList)
		curve->setValuesDateTimeFormat(format);
}

void XYCurveDock::valuesPrecisionChanged(int precision) {
	CONDITIONAL_LOCK_RETURN;

	for (auto* curve : m_curvesList)
		curve->setValuesPrecision(precision);
}

void XYCurveDock::valuesPrefixChanged() {
	CONDITIONAL_LOCK_RETURN;

	QString prefix = ui.leValuesPrefix->text();
	for (auto* curve : m_curvesList)
		curve->setValuesPrefix(prefix);
}

void XYCurveDock::valuesSuffixChanged() {
	CONDITIONAL_LOCK_RETURN;

	QString suffix = ui.leValuesSuffix->text();
	for (auto* curve : m_curvesList)
		curve->setValuesSuffix(suffix);
}

void XYCurveDock::valuesFontChanged(const QFont& font) {
	CONDITIONAL_LOCK_RETURN;

	QFont valuesFont = font;
	valuesFont.setPointSizeF(Worksheet::convertToSceneUnits(font.pointSizeF(), Worksheet::Unit::Point));
	for (auto* curve : m_curvesList)
		curve->setValuesFont(valuesFont);
}

void XYCurveDock::valuesColorChanged(const QColor& color) {
	CONDITIONAL_LOCK_RETURN;

	for (auto* curve : m_curvesList)
		curve->setValuesColor(color);
}

//"Margin Plots"-Tab
void XYCurveDock::rugEnabledChanged(bool state) {
	CONDITIONAL_LOCK_RETURN;

	for (auto* curve : std::as_const(m_curvesList))
		curve->setRugEnabled(state);
}

void XYCurveDock::rugOrientationChanged(int index) {
	CONDITIONAL_LOCK_RETURN;

	auto orientation = static_cast<WorksheetElement::Orientation>(index);
	for (auto* curve : std::as_const(m_curvesList))
		curve->setRugOrientation(orientation);
}

void XYCurveDock::rugLengthChanged(double value) {
	CONDITIONAL_RETURN_NO_LOCK;

	const double length = Worksheet::convertToSceneUnits(value, Worksheet::Unit::Point);
	for (auto* curve : std::as_const(m_curvesList))
		curve->setRugLength(length);
}

void XYCurveDock::rugWidthChanged(double value) {
	CONDITIONAL_RETURN_NO_LOCK;

	const double width = Worksheet::convertToSceneUnits(value, Worksheet::Unit::Point);
	for (auto* curve : std::as_const(m_curvesList))
		curve->setRugWidth(width);
}

void XYCurveDock::rugOffsetChanged(double value) {
	CONDITIONAL_RETURN_NO_LOCK;

	const double offset = Worksheet::convertToSceneUnits(value, Worksheet::Unit::Point);
	for (auto* curve : std::as_const(m_curvesList))
		curve->setRugOffset(offset);
}

//*************************************************************
//*********** SLOTs for changes triggered in XYCurve **********
//*************************************************************
// General-Tab
void XYCurveDock::curveDescriptionChanged(const AbstractAspect* aspect) {
	if (m_curve != aspect)
		return;

	CONDITIONAL_LOCK_RETURN;
	if (aspect->name() != uiGeneralTab.leName->text())
		uiGeneralTab.leName->setText(aspect->name());
	else if (aspect->comment() != uiGeneralTab.teComment->text())
		uiGeneralTab.teComment->setText(aspect->comment());
}

void XYCurveDock::curveXColumnChanged(const AbstractColumn* column) {
	updateValuesWidgets();
	CONDITIONAL_LOCK_RETURN;
	cbXColumn->setAspect(column, m_curve->xColumnPath());
}

void XYCurveDock::curveYColumnChanged(const AbstractColumn* column) {
	updateValuesWidgets();
	CONDITIONAL_LOCK_RETURN;
	cbYColumn->setAspect(column, m_curve->yColumnPath());
}

// Line-Tab
void XYCurveDock::curveLineTypeChanged(XYCurve::LineType type) {
	CONDITIONAL_LOCK_RETURN;
	ui.cbLineType->setCurrentIndex((int)type);
}
void XYCurveDock::curveLineSkipGapsChanged(bool skip) {
	CONDITIONAL_LOCK_RETURN;
	ui.chkLineSkipGaps->setChecked(skip);
}
void XYCurveDock::curveLineIncreasingXOnlyChanged(bool incr) {
	CONDITIONAL_LOCK_RETURN;
	ui.chkLineIncreasingXOnly->setChecked(incr);
}
void XYCurveDock::curveLineInterpolationPointsCountChanged(int count) {
	CONDITIONAL_LOCK_RETURN;
	ui.sbLineInterpolationPointsCount->setValue(count);
}

// Values-Tab
void XYCurveDock::curveValuesTypeChanged(XYCurve::ValuesType type) {
	CONDITIONAL_LOCK_RETURN;
	ui.cbValuesType->setCurrentIndex((int)type);
}
void XYCurveDock::curveValuesColumnChanged(const AbstractColumn* column) {
	CONDITIONAL_LOCK_RETURN;
	cbValuesColumn->setAspect(column, m_curve->valuesColumnPath());
}
void XYCurveDock::curveValuesPositionChanged(XYCurve::ValuesPosition position) {
	CONDITIONAL_LOCK_RETURN;
	ui.cbValuesPosition->setCurrentIndex((int)position);
}
void XYCurveDock::curveValuesDistanceChanged(qreal distance) {
	CONDITIONAL_LOCK_RETURN;
	ui.sbValuesDistance->setValue(Worksheet::convertFromSceneUnits(distance, Worksheet::Unit::Point));
}
void XYCurveDock::curveValuesRotationAngleChanged(qreal angle) {
	CONDITIONAL_LOCK_RETURN;
	ui.sbValuesRotation->setValue(angle);
}
void XYCurveDock::curveValuesNumericFormatChanged(char format) {
	CONDITIONAL_LOCK_RETURN;
	ui.cbValuesNumericFormat->setCurrentIndex(ui.cbValuesNumericFormat->findData(format));
}
void XYCurveDock::curveValuesPrecisionChanged(int precision) {
	CONDITIONAL_LOCK_RETURN;
	ui.sbValuesPrecision->setValue(precision);
}
void XYCurveDock::curveValuesDateTimeFormatChanged(const QString& format) {
	CONDITIONAL_LOCK_RETURN;
	ui.cbValuesDateTimeFormat->setCurrentText(format);
}
void XYCurveDock::curveValuesOpacityChanged(qreal opacity) {
	CONDITIONAL_LOCK_RETURN;
	ui.sbValuesOpacity->setValue(round(opacity * 100.0));
}
void XYCurveDock::curveValuesPrefixChanged(const QString& prefix) {
	CONDITIONAL_LOCK_RETURN;
	ui.leValuesPrefix->setText(prefix);
}
void XYCurveDock::curveValuesSuffixChanged(const QString& suffix) {
	CONDITIONAL_LOCK_RETURN;
	ui.leValuesSuffix->setText(suffix);
}
void XYCurveDock::curveValuesFontChanged(QFont font) {
	CONDITIONAL_LOCK_RETURN;
	font.setPointSizeF(std::round(Worksheet::convertFromSceneUnits(font.pointSizeF(), Worksheet::Unit::Point)));
	ui.kfrValuesFont->setFont(font);
}
void XYCurveDock::curveValuesColorChanged(QColor color) {
	CONDITIONAL_LOCK_RETURN;
	ui.kcbValuesColor->setColor(color);
}

//"Margin Plot"-Tab
void XYCurveDock::curveRugEnabledChanged(bool status) {
	CONDITIONAL_LOCK_RETURN;
	ui.chkRugEnabled->setChecked(status);
}
void XYCurveDock::curveRugOrientationChanged(WorksheetElement::Orientation orientation) {
	CONDITIONAL_LOCK_RETURN;
	ui.cbRugOrientation->setCurrentIndex(static_cast<int>(orientation));
}
void XYCurveDock::curveRugLengthChanged(double value) {
	CONDITIONAL_LOCK_RETURN;
	ui.sbRugLength->setValue(Worksheet::convertFromSceneUnits(value, Worksheet::Unit::Point));
}
void XYCurveDock::curveRugWidthChanged(double value) {
	CONDITIONAL_LOCK_RETURN;
	ui.sbRugWidth->setValue(Worksheet::convertFromSceneUnits(value, Worksheet::Unit::Point));
}
void XYCurveDock::curveRugOffsetChanged(double value) {
	CONDITIONAL_LOCK_RETURN;
	ui.sbRugOffset->setValue(Worksheet::convertFromSceneUnits(value, Worksheet::Unit::Point));
}

//*************************************************************
//************************* Settings **************************
//*************************************************************
void XYCurveDock::load() {
	// General
	// This data is read in XYCurveDock::setCurves().

	// Line
	bool xyCurve = (m_curve->type() == AspectType::XYCurve);
	if (xyCurve) { // options available for XYCurve only and not for analysis curves
		ui.cbLineType->setCurrentIndex((int)m_curve->lineType());
		ui.chkLineSkipGaps->setChecked(m_curve->lineSkipGaps());
		ui.sbLineInterpolationPointsCount->setValue(m_curve->lineInterpolationPointsCount());
	}

	// Values
	ui.cbValuesType->setCurrentIndex((int)m_curve->valuesType());
	ui.cbValuesPosition->setCurrentIndex((int)m_curve->valuesPosition());
	ui.sbValuesDistance->setValue(Worksheet::convertFromSceneUnits(m_curve->valuesDistance(), Worksheet::Unit::Point));
	ui.sbValuesRotation->setValue(m_curve->valuesRotationAngle());
	ui.sbValuesOpacity->setValue(std::round(m_curve->valuesOpacity() * 100.0));
	ui.sbValuesPrecision->setValue(m_curve->valuesPrecision());
	ui.cbValuesNumericFormat->setCurrentIndex(ui.cbValuesNumericFormat->findData(m_curve->valuesNumericFormat()));
	ui.cbValuesDateTimeFormat->setCurrentText(m_curve->valuesDateTimeFormat());
	ui.leValuesPrefix->setText(m_curve->valuesPrefix());
	ui.leValuesSuffix->setText(m_curve->valuesSuffix());
	QFont valuesFont = m_curve->valuesFont();
	valuesFont.setPointSizeF(std::round(Worksheet::convertFromSceneUnits(valuesFont.pointSizeF(), Worksheet::Unit::Point)));
	ui.kfrValuesFont->setFont(valuesFont);
	ui.kcbValuesColor->setColor(m_curve->valuesColor());
	this->updateValuesWidgets();

	// Margin plots
	ui.chkRugEnabled->setChecked(m_curve->rugEnabled());
	ui.cbRugOrientation->setCurrentIndex(static_cast<int>(m_curve->rugOrientation()));
	ui.sbRugWidth->setValue(Worksheet::convertFromSceneUnits(m_curve->rugWidth(), Worksheet::Unit::Point));
	ui.sbRugLength->setValue(Worksheet::convertFromSceneUnits(m_curve->rugLength(), Worksheet::Unit::Point));
	ui.sbRugOffset->setValue(Worksheet::convertFromSceneUnits(m_curve->rugOffset(), Worksheet::Unit::Point));
}

void XYCurveDock::loadConfigFromTemplate(KConfig& config) {
	auto name = TemplateHandler::templateName(config);
	int size = m_curvesList.size();
	if (size > 1)
		m_curve->beginMacro(i18n("%1 xy-curves: template \"%2\" loaded", size, name));
	else
		m_curve->beginMacro(i18n("%1: template \"%2\" loaded", m_curve->name(), name));

	this->loadConfig(config);

	m_curve->endMacro();
}

void XYCurveDock::loadConfig(KConfig& config) {
	KConfigGroup group = config.group(QStringLiteral("XYCurve"));

	// General
	// we don't load/save the settings in the general-tab, since they are not style related.
	// It doesn't make sense to load/save them in the template.
	// This data is read in XYCurveDock::setCurves().

	// Line
	bool xyCurve = (m_curve->type() == AspectType::XYCurve);
	if (xyCurve) {
		ui.cbLineType->setCurrentIndex(group.readEntry(QStringLiteral("LineType"), (int)m_curve->lineType()));
		ui.chkLineSkipGaps->setChecked(group.readEntry(QStringLiteral("LineSkipGaps"), m_curve->lineSkipGaps()));
		ui.sbLineInterpolationPointsCount->setValue(group.readEntry(QStringLiteral("LineInterpolationPointsCount"), m_curve->lineInterpolationPointsCount()));
	}
	lineWidget->loadConfig(group);
	dropLineWidget->loadConfig(group);

	// Symbols
	symbolWidget->loadConfig(group);

	// Values
	ui.cbValuesType->setCurrentIndex(group.readEntry(QStringLiteral("ValuesType"), (int)m_curve->valuesType()));
	ui.cbValuesPosition->setCurrentIndex(group.readEntry(QStringLiteral("ValuesPosition"), (int)m_curve->valuesPosition()));
	ui.sbValuesDistance->setValue(
		Worksheet::convertFromSceneUnits(group.readEntry(QStringLiteral("ValuesDistance"), m_curve->valuesDistance()), Worksheet::Unit::Point));
	ui.sbValuesRotation->setValue(group.readEntry(QStringLiteral("ValuesRotation"), m_curve->valuesRotationAngle()));
	ui.sbValuesOpacity->setValue(std::round(group.readEntry(QStringLiteral("ValuesOpacity"), m_curve->valuesOpacity()) * 100.0));
	ui.leValuesPrefix->setText(group.readEntry(QStringLiteral("ValuesPrefix"), m_curve->valuesPrefix()));
	ui.leValuesSuffix->setText(group.readEntry(QStringLiteral("ValuesSuffix"), m_curve->valuesSuffix()));
	QFont valuesFont = m_curve->valuesFont();
	valuesFont.setPointSizeF(std::round(Worksheet::convertFromSceneUnits(valuesFont.pointSizeF(), Worksheet::Unit::Point)));
	ui.kfrValuesFont->setFont(group.readEntry(QStringLiteral("ValuesFont"), valuesFont));
	ui.kcbValuesColor->setColor(group.readEntry(QStringLiteral("ValuesColor"), m_curve->valuesColor()));

	// Filling
	backgroundWidget->loadConfig(group);
}

void XYCurveDock::saveConfigAsTemplate(KConfig& config) {
	KConfigGroup group = config.group(QStringLiteral("XYCurve"));

	// General
	// we don't load/save the settings in the general-tab, since they are not style related.
	// It doesn't make sense to load/save them in the template.

	bool xyCurve = (m_curve->type() == AspectType::XYCurve);
	if (xyCurve) {
		group.writeEntry(QStringLiteral("LineType"), ui.cbLineType->currentIndex());
		group.writeEntry(QStringLiteral("LineSkipGaps"), ui.chkLineSkipGaps->isChecked());
		group.writeEntry(QStringLiteral("LineInterpolationPointsCount"), ui.sbLineInterpolationPointsCount->value());
	}

	lineWidget->saveConfig(group);
	dropLineWidget->saveConfig(group);

	// Symbols
	symbolWidget->saveConfig(group);

	// Values
	group.writeEntry(QStringLiteral("ValuesType"), ui.cbValuesType->currentIndex());
	group.writeEntry(QStringLiteral("ValuesPosition"), ui.cbValuesPosition->currentIndex());
	group.writeEntry(QStringLiteral("ValuesDistance"), Worksheet::convertToSceneUnits(ui.sbValuesDistance->value(), Worksheet::Unit::Point));
	group.writeEntry(QStringLiteral("ValuesRotation"), ui.sbValuesRotation->value());
	group.writeEntry(QStringLiteral("ValuesOpacity"), ui.sbValuesOpacity->value() / 100.0);
	group.writeEntry(QStringLiteral("valuesNumericFormat"), ui.cbValuesNumericFormat->currentText());
	group.writeEntry(QStringLiteral("valuesPrecision"), ui.sbValuesPrecision->value());
	group.writeEntry(QStringLiteral("valuesDateTimeFormat"), ui.cbValuesDateTimeFormat->currentText());
	group.writeEntry(QStringLiteral("ValuesPrefix"), ui.leValuesPrefix->text());
	group.writeEntry(QStringLiteral("ValuesSuffix"), ui.leValuesSuffix->text());
	group.writeEntry(QStringLiteral("ValuesFont"), ui.kfrValuesFont->font());
	group.writeEntry(QStringLiteral("ValuesColor"), ui.kcbValuesColor->color());

	// Filling
	backgroundWidget->saveConfig(group);

	// Error bars
	if (xyCurve)
		errorBarWidget->saveConfig(group);

	config.sync();
}
