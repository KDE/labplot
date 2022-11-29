/*
	File                 : XYCurveDock.cpp
	Project              : LabPlot
	Description          : widget for XYCurve properties
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2010-2022 Alexander Semke <alexander.semke@web.de>
	SPDX-FileCopyrightText: 2012-2022 Stefan Gerlach <stefan.gerlach@uni-konstanz.de>

	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "XYCurveDock.h"
#include "backend/core/AspectTreeModel.h"
#include "backend/core/Project.h"
#include "backend/core/column/Column.h"
#include "backend/core/datatypes/DateTime2StringFilter.h"
#include "backend/core/datatypes/Double2StringFilter.h"
#include "backend/worksheet/Worksheet.h"
#include "backend/worksheet/plots/cartesian/XYCurve.h"
#include "commonfrontend/widgets/TreeViewComboBox.h"
#include "kdefrontend/GuiTools.h"
#include "kdefrontend/TemplateHandler.h"
#include "kdefrontend/widgets/BackgroundWidget.h"
#include "kdefrontend/widgets/LineWidget.h"
#include "kdefrontend/widgets/SymbolWidget.h"

#include <QPainter>

#include <KConfig>
#include <KConfigGroup>
#include <KLocalizedString>
#include <KSharedConfig>

/*!
  \class XYCurveDock
  \brief  Provides a widget for editing the properties of the XYCurves (2D-curves) currently selected in the project explorer.

  If more than one curves are set, the properties of the first column are shown. The changes of the properties are applied to all curves.
  The exclusions are the name, the comment and the datasets (columns) of the curves  - these properties can only be changed if there is only one single curve.

  \ingroup kdefrontend
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

	// add formats for numeric values
	ui.cbValuesNumericFormat->addItem(i18n("Decimal"), QVariant('f'));
	ui.cbValuesNumericFormat->addItem(i18n("Scientific (e)"), QVariant('e'));
	ui.cbValuesNumericFormat->addItem(i18n("Scientific (E)"), QVariant('E'));
	ui.cbValuesNumericFormat->addItem(i18n("Automatic (e)"), QVariant('g'));
	ui.cbValuesNumericFormat->addItem(i18n("Automatic (E)"), QVariant('G'));

	// add format for date, time and datetime values
	for (const auto& s : AbstractColumn::dateTimeFormats())
		ui.cbValuesDateTimeFormat->addItem(s, QVariant(s));

	ui.cbValuesDateTimeFormat->setEditable(true);

	// Tab "Filling"
	auto* layout = static_cast<QHBoxLayout*>(ui.tabAreaFilling->layout());
	backgroundWidget = new BackgroundWidget(ui.tabAreaFilling);
	layout->insertWidget(0, backgroundWidget);

	// Tab "Error Bars"
	const KConfigGroup group = KSharedConfig::openConfig()->group(QStringLiteral("Settings_General"));
	if (group.readEntry("GUMTerms", false)) {
		ui.tabWidget->setTabText(ui.tabWidget->indexOf(ui.tabErrorBars), i18n("Uncertainty Bars"));
		ui.lErrorBarX->setText(i18n("X Uncertainty"));
		ui.lErrorBarY->setText(i18n("Y Uncertainty"));
	}

	gridLayout = qobject_cast<QGridLayout*>(ui.tabErrorBars->layout());

	cbXErrorPlusColumn = new TreeViewComboBox(ui.tabErrorBars);
	gridLayout->addWidget(cbXErrorPlusColumn, 2, 2, 1, 1);

	cbXErrorMinusColumn = new TreeViewComboBox(ui.tabErrorBars);
	gridLayout->addWidget(cbXErrorMinusColumn, 3, 2, 1, 1);

	cbYErrorPlusColumn = new TreeViewComboBox(ui.tabErrorBars);
	gridLayout->addWidget(cbYErrorPlusColumn, 7, 2, 1, 1);

	cbYErrorMinusColumn = new TreeViewComboBox(ui.tabErrorBars);
	gridLayout->addWidget(cbYErrorMinusColumn, 8, 2, 1, 1);

	errorBarsLineWidget = new LineWidget(ui.tabErrorBars);
	gridLayout->addWidget(errorBarsLineWidget, 11, 0, 1, 3);

	// Tab "Margin Plots"
	ui.cbRugOrientation->addItem(i18n("Vertical"));
	ui.cbRugOrientation->addItem(i18n("Horizontal"));
	ui.cbRugOrientation->addItem(i18n("Both"));

	// adjust layouts in the tabs
	for (int i = 0; i < ui.tabWidget->count(); ++i) {
		auto* layout = dynamic_cast<QGridLayout*>(ui.tabWidget->widget(i)->layout());
		if (!layout)
			continue;

		layout->setContentsMargins(2, 2, 2, 2);
		layout->setHorizontalSpacing(2);
		layout->setVerticalSpacing(2);
	}

	XYCurveDock::updateLocale();

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

	// Error bars
	connect(ui.cbXErrorType, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &XYCurveDock::xErrorTypeChanged);
	connect(cbXErrorPlusColumn, &TreeViewComboBox::currentModelIndexChanged, this, &XYCurveDock::xErrorPlusColumnChanged);
	connect(cbXErrorMinusColumn, &TreeViewComboBox::currentModelIndexChanged, this, &XYCurveDock::xErrorMinusColumnChanged);
	connect(ui.cbYErrorType, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &XYCurveDock::yErrorTypeChanged);
	connect(cbYErrorPlusColumn, &TreeViewComboBox::currentModelIndexChanged, this, &XYCurveDock::yErrorPlusColumnChanged);
	connect(cbYErrorMinusColumn, &TreeViewComboBox::currentModelIndexChanged, this, &XYCurveDock::yErrorMinusColumnChanged);

	// Margin Plots
	connect(ui.chkRugEnabled, &QCheckBox::toggled, this, &XYCurveDock::rugEnabledChanged);
	connect(ui.cbRugOrientation, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &XYCurveDock::rugOrientationChanged);
	connect(ui.sbRugLength, QOverload<double>::of(&NumberSpinBox::valueChanged), this, &XYCurveDock::rugLengthChanged);
	connect(ui.sbRugWidth, QOverload<double>::of(&NumberSpinBox::valueChanged), this, &XYCurveDock::rugWidthChanged);
	connect(ui.sbRugOffset, QOverload<double>::of(&NumberSpinBox::valueChanged), this, &XYCurveDock::rugOffsetChanged);

	// template handler
	auto* frame = new QFrame(this);
	hboxLayout = new QHBoxLayout(frame);
	hboxLayout->setContentsMargins(0, 11, 0, 11);

	auto* templateHandler = new TemplateHandler(this, TemplateHandler::ClassName::XYCurve);
	layout->addWidget(templateHandler);
	connect(templateHandler, &TemplateHandler::loadConfigRequested, this, &XYCurveDock::loadConfigFromTemplate);
	connect(templateHandler, &TemplateHandler::saveConfigRequested, this, &XYCurveDock::saveConfigAsTemplate);
	connect(templateHandler, &TemplateHandler::info, this, &XYCurveDock::info);

	ui.verticalLayout->addWidget(frame);

	retranslateUi();
	init();
}

XYCurveDock::~XYCurveDock() {
	if (m_aspectTreeModel)
		delete m_aspectTreeModel;
}

void XYCurveDock::setupGeneral() {
	auto* generalTab = new QWidget(ui.tabGeneral);
	uiGeneralTab.setupUi(generalTab);
	m_leName = uiGeneralTab.leName;
	m_teComment = uiGeneralTab.teComment;
	m_teComment->setFixedHeight(m_leName->height());

	auto* layout = new QHBoxLayout(ui.tabGeneral);
	layout->setMargin(0);
	layout->addWidget(generalTab);

	// Tab "General" (see xycurvedockgeneraltab.ui)
	auto* gridLayout = qobject_cast<QGridLayout*>(generalTab->layout());

	cbXColumn = new TreeViewComboBox(generalTab);
	cbXColumn->useCurrentIndexText(false);
	gridLayout->addWidget(cbXColumn, 4, 2, 1, 1);

	cbYColumn = new TreeViewComboBox(generalTab);
	cbYColumn->useCurrentIndexText(false);
	gridLayout->addWidget(cbYColumn, 5, 2, 1, 1);

	// General
	connect(uiGeneralTab.leName, &QLineEdit::textChanged, this, &XYCurveDock::nameChanged);
	connect(uiGeneralTab.teComment, &QTextEdit::textChanged, this, &XYCurveDock::commentChanged);
	connect(uiGeneralTab.chkLegendVisible, &QCheckBox::toggled, this, &XYCurveDock::legendVisibleChanged);
	connect(uiGeneralTab.chkVisible, &QCheckBox::clicked, this, &XYCurveDock::visibilityChanged);
	connect(cbXColumn, &TreeViewComboBox::currentModelIndexChanged, this, &XYCurveDock::xColumnChanged);
	connect(cbYColumn, &TreeViewComboBox::currentModelIndexChanged, this, &XYCurveDock::yColumnChanged);
	connect(uiGeneralTab.cbPlotRanges, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &XYCurveDock::plotRangeChanged);
}

void XYCurveDock::init() {
	m_initializing = true;

	// Line
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

	QPainter pa;
	// TODO size of the icon depending on the actual height of the combobox?
	int iconSize = 20;
	QPixmap pm(iconSize, iconSize);
	ui.cbLineType->setIconSize(QSize(iconSize, iconSize));

	QPen pen(Qt::SolidPattern, 0);
	const QColor& color = (palette().color(QPalette::Base).lightness() < 128) ? Qt::white : Qt::black;
	pen.setColor(color);
	pa.setPen(pen);

	// no line
	pm.fill(Qt::transparent);
	pa.begin(&pm);
	pa.setPen(pen);
	pa.setRenderHint(QPainter::Antialiasing);
	pa.drawEllipse(1, 1, 4, 4);
	pa.drawEllipse(15, 15, 4, 4);
	pa.end();
	ui.cbLineType->setItemIcon(0, pm);

	// line
	pm.fill(Qt::transparent);
	pa.begin(&pm);
	pa.setPen(pen);
	pa.setRenderHint(QPainter::Antialiasing);
	pa.drawEllipse(1, 1, 4, 4);
	pa.drawEllipse(15, 15, 4, 4);
	pa.drawLine(3, 3, 17, 17);
	pa.end();
	ui.cbLineType->setItemIcon(1, pm);

	pm.fill(Qt::transparent);
	pa.begin(&pm);
	pa.setPen(pen);
	pa.setRenderHint(QPainter::Antialiasing);
	pa.drawEllipse(1, 1, 4, 4);
	pa.drawEllipse(15, 15, 4, 4);
	pa.drawLine(3, 3, 17, 3);
	pa.drawLine(17, 3, 17, 17);
	pa.end();
	ui.cbLineType->setItemIcon(2, pm);

	pm.fill(Qt::transparent);
	pa.begin(&pm);
	pa.setPen(pen);
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

	m_initializing = false;

	// Values
	ui.cbValuesType->addItem(i18n("No Values"));
	ui.cbValuesType->addItem(QStringLiteral("x"));
	ui.cbValuesType->addItem(QStringLiteral("y"));
	ui.cbValuesType->addItem(QStringLiteral("x, y"));
	ui.cbValuesType->addItem(QStringLiteral("(x, y)"));
	ui.cbValuesType->addItem(i18n("Custom Column"));

	ui.cbValuesPosition->addItem(i18n("Above"));
	ui.cbValuesPosition->addItem(i18n("Below"));
	ui.cbValuesPosition->addItem(i18n("Left"));
	ui.cbValuesPosition->addItem(i18n("Right"));

	// Error Bars
	ui.cbXErrorType->addItem(i18n("No"));
	ui.cbXErrorType->addItem(i18n("Symmetric"));
	ui.cbXErrorType->addItem(i18n("Asymmetric"));

	ui.cbYErrorType->addItem(i18n("No"));
	ui.cbYErrorType->addItem(i18n("Symmetric"));
	ui.cbYErrorType->addItem(i18n("Asymmetric"));
}

void XYCurveDock::setModel() {
	m_aspectTreeModel->enablePlottableColumnsOnly(true);
	m_aspectTreeModel->enableShowPlotDesignation(true);

	QList<AspectType> list{AspectType::Folder,
						   AspectType::Workbook,
						   AspectType::Datapicker,
						   AspectType::DatapickerCurve,
						   AspectType::Spreadsheet,
						   AspectType::LiveDataSource,
						   AspectType::Column,
						   AspectType::Worksheet,
						   AspectType::CartesianPlot,
						   AspectType::XYFitCurve,
						   AspectType::XYSmoothCurve,
						   AspectType::CantorWorksheet};

	if (cbXColumn && cbYColumn) {
		cbXColumn->setTopLevelClasses(list);
		cbYColumn->setTopLevelClasses(list);
	}
	cbValuesColumn->setTopLevelClasses(list);
	cbXErrorMinusColumn->setTopLevelClasses(list);
	cbXErrorPlusColumn->setTopLevelClasses(list);
	cbYErrorMinusColumn->setTopLevelClasses(list);
	cbYErrorPlusColumn->setTopLevelClasses(list);

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
				AspectType::XYDataReductionCurve};
	else
		list = {AspectType::Column};

	m_aspectTreeModel->setSelectableAspects(list);

	if (cbXColumn && cbYColumn) {
		cbXColumn->setModel(m_aspectTreeModel);
		cbYColumn->setModel(m_aspectTreeModel);
	}

	cbXErrorMinusColumn->setModel(m_aspectTreeModel);
	cbXErrorPlusColumn->setModel(m_aspectTreeModel);
	cbYErrorMinusColumn->setModel(m_aspectTreeModel);
	cbYErrorPlusColumn->setModel(m_aspectTreeModel);

	// for value labels we need a dedicated model since we also want to allow
	// to select text columns and we don't want to call enablePlottableColumnsOnly().
	auto* valuesTreeModel = new AspectTreeModel(m_curve->project());
	valuesTreeModel->setSelectableAspects(list);
	cbValuesColumn->setModel(valuesTreeModel);

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

	// remove the tab "Error bars" for analysis curves
	if (!visible)
		ui.tabWidget->removeTab(5);
}

/*!
  sets the curves. The properties of the curves in the list \c list can be edited in this widget.
*/
void XYCurveDock::setCurves(QList<XYCurve*> list) {
	CONDITONAL_LOCK_RETURN;
	m_curvesList = list;
	m_curve = list.first();
	setAspects(list);
	Q_ASSERT(m_curve);
	m_aspectTreeModel = new AspectTreeModel(m_curve->project());
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
	QList<Line*> errorBarLines;
	for (auto* curve : curves) {
		symbols << curve->symbol();
		backgrounds << curve->background();
		lines << curve->line();
		dropLines << curve->dropLine();
		errorBarLines << curve->errorBarsLine();
	}

	symbolWidget->setSymbols(symbols);
	backgroundWidget->setBackgrounds(backgrounds);
	lineWidget->setLines(lines);
	dropLineWidget->setLines(dropLines);
	errorBarsLineWidget->setLines(errorBarLines);
}

void XYCurveDock::initGeneralTab() {
	DEBUG(Q_FUNC_INFO);
	// if there is more than one curve in the list, disable the content in the tab "general"
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

	// show the properties of the first curve
	cbXColumn->setColumn(m_curve->xColumn(), m_curve->xColumnPath());
	cbYColumn->setColumn(m_curve->yColumn(), m_curve->yColumnPath());
	uiGeneralTab.chkLegendVisible->setChecked(m_curve->legendVisible());
	uiGeneralTab.chkVisible->setChecked(m_curve->isVisible());

	updatePlotRanges();

	// Slots
	connect(m_curve, &XYCurve::aspectDescriptionChanged, this, &XYCurveDock::curveDescriptionChanged);
	connect(m_curve, &XYCurve::xColumnChanged, this, &XYCurveDock::curveXColumnChanged);
	connect(m_curve, &XYCurve::yColumnChanged, this, &XYCurveDock::curveYColumnChanged);
	connect(m_curve, &WorksheetElement::plotRangeListChanged, this, &XYCurveDock::updatePlotRanges);
	connect(m_curve, &XYCurve::legendVisibleChanged, this, &XYCurveDock::curveLegendVisibleChanged);
	connect(m_curve, &WorksheetElement::visibleChanged, this, &XYCurveDock::curveVisibilityChanged);
	DEBUG(Q_FUNC_INFO << " DONE");
}

void XYCurveDock::initTabs() {
	// if there are more than one curve in the list, disable the tab "general"
	if (m_curvesList.size() == 1) {
		cbValuesColumn->setColumn(m_curve->valuesColumn(), m_curve->valuesColumnPath());
		cbXErrorPlusColumn->setColumn(m_curve->xErrorPlusColumn(), m_curve->xErrorPlusColumnPath());
		cbXErrorMinusColumn->setColumn(m_curve->xErrorMinusColumn(), m_curve->xErrorMinusColumnPath());
		cbYErrorPlusColumn->setColumn(m_curve->yErrorPlusColumn(), m_curve->yErrorPlusColumnPath());
		cbYErrorMinusColumn->setColumn(m_curve->yErrorMinusColumn(), m_curve->yErrorMinusColumnPath());
	} else {
		cbValuesColumn->setCurrentModelIndex(QModelIndex());
		cbXErrorPlusColumn->setCurrentModelIndex(QModelIndex());
		cbXErrorMinusColumn->setCurrentModelIndex(QModelIndex());
		cbYErrorPlusColumn->setCurrentModelIndex(QModelIndex());
		cbYErrorMinusColumn->setCurrentModelIndex(QModelIndex());
	}

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

	//"Error bars"-Tab
	connect(m_curve, &XYCurve::xErrorTypeChanged, this, &XYCurveDock::curveXErrorTypeChanged);
	connect(m_curve, &XYCurve::xErrorPlusColumnChanged, this, &XYCurveDock::curveXErrorPlusColumnChanged);
	connect(m_curve, &XYCurve::xErrorMinusColumnChanged, this, &XYCurveDock::curveXErrorMinusColumnChanged);
	connect(m_curve, &XYCurve::yErrorTypeChanged, this, &XYCurveDock::curveYErrorTypeChanged);
	connect(m_curve, &XYCurve::yErrorPlusColumnChanged, this, &XYCurveDock::curveYErrorPlusColumnChanged);
	connect(m_curve, &XYCurve::yErrorMinusColumnChanged, this, &XYCurveDock::curveYErrorMinusColumnChanged);

	//"Margin Plots"-Tab
	connect(m_curve, &XYCurve::rugEnabledChanged, this, &XYCurveDock::curveRugEnabledChanged);
	connect(m_curve, &XYCurve::rugOrientationChanged, this, &XYCurveDock::curveRugOrientationChanged);
	connect(m_curve, &XYCurve::rugLengthChanged, this, &XYCurveDock::curveRugLengthChanged);
	connect(m_curve, &XYCurve::rugWidthChanged, this, &XYCurveDock::curveRugWidthChanged);
	connect(m_curve, &XYCurve::rugOffsetChanged, this, &XYCurveDock::curveRugOffsetChanged);
}

void XYCurveDock::updateLocale() {
	SET_NUMBER_LOCALE
	ui.sbValuesDistance->setLocale(numberLocale);
	lineWidget->updateLocale();
	dropLineWidget->updateLocale();
	symbolWidget->updateLocale();
	errorBarsLineWidget->updateLocale();
}

void XYCurveDock::updatePlotRanges() {
	updatePlotRangeList(uiGeneralTab.cbPlotRanges);
}

//*************************************************************
//********** SLOTs for changes triggered in XYCurveDock ********
//*************************************************************
void XYCurveDock::retranslateUi() {
	ui.lLineSkipGaps->setToolTip(i18n("If checked, connect neighbour points with lines even if there are gaps (invalid or masked values) between them"));
	ui.chkLineSkipGaps->setToolTip(i18n("If checked, connect neighbour points with lines even if there are gaps (invalid or masked values) between them"));
	ui.lLineIncreasingXOnly->setToolTip(i18n("If checked, connect data points only for strictly increasing values of X"));
	ui.chkLineIncreasingXOnly->setToolTip(i18n("If checked, connect data points only for strictly increasing values of X"));
	// TODO:
	// 	uiGeneralTab.lName->setText(i18n("Name"));
	// 	uiGeneralTab.lComment->setText(i18n("Comment"));
	// 	uiGeneralTab.chkVisible->setText(i18n("Visible"));
	// 	uiGeneralTab.lXColumn->setText(i18n("x-data"));
	// 	uiGeneralTab.lYColumn->setText(i18n("y-data"));

	// TODO updatePenStyles, updateBrushStyles for all comboboxes
}

void XYCurveDock::xColumnChanged(const QModelIndex& index) {
	updateValuesWidgets();

	if (m_initializing)
		return;

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

	if (m_initializing)
		return;

	auto* aspect = static_cast<AbstractAspect*>(index.internalPointer());
	AbstractColumn* column = nullptr;
	if (aspect) {
		column = dynamic_cast<AbstractColumn*>(aspect);
		Q_ASSERT(column);
	}

	for (auto* curve : m_curvesList)
		curve->setYColumn(column);
}

void XYCurveDock::legendVisibleChanged(bool state) {
	if (m_initializing)
		return;

	for (auto* curve : m_curvesList)
		curve->setLegendVisible(state);
}

void XYCurveDock::visibilityChanged(bool state) {
	if (m_initializing)
		return;

	for (auto* curve : m_curvesList)
		curve->setVisible(state);
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

	if (m_initializing)
		return;

	for (auto* curve : m_curvesList)
		curve->setLineType(lineType);
}

void XYCurveDock::lineSkipGapsChanged(bool skip) {
	if (m_initializing)
		return;

	for (auto* curve : m_curvesList)
		curve->setLineSkipGaps(skip);
}

void XYCurveDock::lineIncreasingXOnlyChanged(bool incr) {
	if (m_initializing)
		return;

	for (auto* curve : m_curvesList)
		curve->setLineIncreasingXOnly(incr);
}

void XYCurveDock::lineInterpolationPointsCountChanged(int count) {
	if (m_initializing)
		return;

	for (auto* curve : m_curvesList)
		curve->setLineInterpolationPointsCount(count);
}

// Values-tab

/*!
  called when the type of the values (none, x, y, (x,y) etc.) was changed.
*/
void XYCurveDock::valuesTypeChanged(int index) {
	if (m_initializing)
		return;

	this->updateValuesWidgets();

	const auto type = XYCurve::ValuesType(index);
	for (auto* curve : m_curvesList)
		curve->setValuesType(type);
}

/*!
  called when the custom column for the values was changed.
*/
void XYCurveDock::valuesColumnChanged(const QModelIndex& index) {
	if (m_initializing)
		return;

	this->updateValuesWidgets();

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
	if (m_initializing)
		return;

	for (auto* curve : m_curvesList)
		curve->setValuesPosition(XYCurve::ValuesPosition(index));
}

void XYCurveDock::valuesDistanceChanged(double value) {
	if (m_initializing)
		return;

	for (auto* curve : m_curvesList)
		curve->setValuesDistance(Worksheet::convertToSceneUnits(value, Worksheet::Unit::Point));
}

void XYCurveDock::valuesRotationChanged(int value) {
	if (m_initializing)
		return;

	for (auto* curve : m_curvesList)
		curve->setValuesRotationAngle(value);
}

void XYCurveDock::valuesOpacityChanged(int value) {
	if (m_initializing)
		return;

	qreal opacity = (float)value / 100.;
	for (auto* curve : m_curvesList)
		curve->setValuesOpacity(opacity);
}

void XYCurveDock::valuesNumericFormatChanged(int index) {
	if (m_initializing)
		return;

	char format = ui.cbValuesNumericFormat->itemData(index).toChar().toLatin1();
	for (auto* curve : m_curvesList)
		curve->setValuesNumericFormat(format);
}

void XYCurveDock::valuesDateTimeFormatChanged(const QString& format) {
	if (m_initializing)
		return;

	for (auto* curve : m_curvesList)
		curve->setValuesDateTimeFormat(format);
}

void XYCurveDock::valuesPrecisionChanged(int precision) {
	if (m_initializing)
		return;

	for (auto* curve : m_curvesList)
		curve->setValuesPrecision(precision);
}

void XYCurveDock::valuesPrefixChanged() {
	if (m_initializing)
		return;

	QString prefix = ui.leValuesPrefix->text();
	for (auto* curve : m_curvesList)
		curve->setValuesPrefix(prefix);
}

void XYCurveDock::valuesSuffixChanged() {
	if (m_initializing)
		return;

	QString suffix = ui.leValuesSuffix->text();
	for (auto* curve : m_curvesList)
		curve->setValuesSuffix(suffix);
}

void XYCurveDock::valuesFontChanged(const QFont& font) {
	if (m_initializing)
		return;

	QFont valuesFont = font;
	valuesFont.setPixelSize(Worksheet::convertToSceneUnits(font.pointSizeF(), Worksheet::Unit::Point));
	for (auto* curve : m_curvesList)
		curve->setValuesFont(valuesFont);
}

void XYCurveDock::valuesColorChanged(const QColor& color) {
	if (m_initializing)
		return;

	for (auto* curve : m_curvesList)
		curve->setValuesColor(color);
}

//"Error bars"-Tab
void XYCurveDock::xErrorTypeChanged(int index) const {
	if (index == 0) {
		// no error
		ui.lXErrorDataPlus->setVisible(false);
		cbXErrorPlusColumn->setVisible(false);
		ui.lXErrorDataMinus->setVisible(false);
		cbXErrorMinusColumn->setVisible(false);
	} else if (index == 1) {
		// symmetric error
		ui.lXErrorDataPlus->setVisible(true);
		cbXErrorPlusColumn->setVisible(true);
		ui.lXErrorDataMinus->setVisible(false);
		cbXErrorMinusColumn->setVisible(false);
		ui.lXErrorDataPlus->setText(i18n("Data, +-:"));
	} else if (index == 2) {
		// asymmetric error
		ui.lXErrorDataPlus->setVisible(true);
		cbXErrorPlusColumn->setVisible(true);
		ui.lXErrorDataMinus->setVisible(true);
		cbXErrorMinusColumn->setVisible(true);
		ui.lXErrorDataPlus->setText(i18n("Data, +:"));
	}

	bool b = (index != 0 || ui.cbYErrorType->currentIndex() != 0);
	ui.lErrorFormat->setVisible(b);
	errorBarsLineWidget->setVisible(b);

	if (m_initializing)
		return;

	for (auto* curve : m_curvesList)
		curve->setXErrorType(XYCurve::ErrorType(index));
}

void XYCurveDock::xErrorPlusColumnChanged(const QModelIndex& index) const {
	if (m_initializing)
		return;

	auto* aspect = static_cast<AbstractAspect*>(index.internalPointer());
	auto* column = dynamic_cast<AbstractColumn*>(aspect);
	Q_ASSERT(column);

	for (auto* curve : m_curvesList)
		curve->setXErrorPlusColumn(column);
}

void XYCurveDock::xErrorMinusColumnChanged(const QModelIndex& index) const {
	if (m_initializing)
		return;

	auto* aspect = static_cast<AbstractAspect*>(index.internalPointer());
	auto* column = dynamic_cast<AbstractColumn*>(aspect);
	Q_ASSERT(column);

	for (auto* curve : m_curvesList)
		curve->setXErrorMinusColumn(column);
}

void XYCurveDock::yErrorTypeChanged(int index) const {
	if (index == 0) {
		// no error
		ui.lYErrorDataPlus->setVisible(false);
		cbYErrorPlusColumn->setVisible(false);
		ui.lYErrorDataMinus->setVisible(false);
		cbYErrorMinusColumn->setVisible(false);
	} else if (index == 1) {
		// symmetric error
		ui.lYErrorDataPlus->setVisible(true);
		cbYErrorPlusColumn->setVisible(true);
		ui.lYErrorDataMinus->setVisible(false);
		cbYErrorMinusColumn->setVisible(false);
		ui.lYErrorDataPlus->setText(i18n("Data, +-:"));
	} else if (index == 2) {
		// asymmetric error
		ui.lYErrorDataPlus->setVisible(true);
		cbYErrorPlusColumn->setVisible(true);
		ui.lYErrorDataMinus->setVisible(true);
		cbYErrorMinusColumn->setVisible(true);
		ui.lYErrorDataPlus->setText(i18n("Data, +:"));
	}

	bool b = (index != 0 || ui.cbXErrorType->currentIndex() != 0);
	ui.lErrorFormat->setVisible(b);
	errorBarsLineWidget->setVisible(b);

	if (m_initializing)
		return;

	for (auto* curve : m_curvesList)
		curve->setYErrorType(XYCurve::ErrorType(index));
}

void XYCurveDock::yErrorPlusColumnChanged(const QModelIndex& index) const {
	if (m_initializing)
		return;

	auto* aspect = static_cast<AbstractAspect*>(index.internalPointer());
	auto* column = dynamic_cast<AbstractColumn*>(aspect);
	Q_ASSERT(column);

	for (auto* curve : m_curvesList)
		curve->setYErrorPlusColumn(column);
}

void XYCurveDock::yErrorMinusColumnChanged(const QModelIndex& index) const {
	if (m_initializing)
		return;

	auto* aspect = static_cast<AbstractAspect*>(index.internalPointer());
	auto* column = dynamic_cast<AbstractColumn*>(aspect);
	Q_ASSERT(column);

	for (auto* curve : m_curvesList)
		curve->setYErrorMinusColumn(column);
}

//"Margin Plots"-Tab
void XYCurveDock::rugEnabledChanged(bool state) const {
	if (m_initializing)
		return;

	for (auto* curve : qAsConst(m_curvesList))
		curve->setRugEnabled(state);
}

void XYCurveDock::rugOrientationChanged(int index) const {
	if (m_initializing)
		return;

	auto orientation = static_cast<WorksheetElement::Orientation>(index);
	for (auto* curve : qAsConst(m_curvesList))
		curve->setRugOrientation(orientation);
}

void XYCurveDock::rugLengthChanged(double value) const {
	if (m_initializing)
		return;

	const double length = Worksheet::convertToSceneUnits(value, Worksheet::Unit::Point);
	for (auto* curve : qAsConst(m_curvesList))
		curve->setRugLength(length);
}

void XYCurveDock::rugWidthChanged(double value) const {
	if (m_initializing)
		return;

	const double width = Worksheet::convertToSceneUnits(value, Worksheet::Unit::Point);
	for (auto* curve : qAsConst(m_curvesList))
		curve->setRugWidth(width);
}

void XYCurveDock::rugOffsetChanged(double value) const {
	if (m_initializing)
		return;

	const double offset = Worksheet::convertToSceneUnits(value, Worksheet::Unit::Point);
	for (auto* curve : qAsConst(m_curvesList))
		curve->setRugOffset(offset);
}

//*************************************************************
//*********** SLOTs for changes triggered in XYCurve **********
//*************************************************************
// General-Tab
void XYCurveDock::curveDescriptionChanged(const AbstractAspect* aspect) {
	if (m_curve != aspect)
		return;

	CONDITONAL_LOCK_RETURN;
	if (aspect->name() != uiGeneralTab.leName->text())
		uiGeneralTab.leName->setText(aspect->name());
	else if (aspect->comment() != uiGeneralTab.teComment->text())
		uiGeneralTab.teComment->setText(aspect->comment());
}

void XYCurveDock::curveXColumnChanged(const AbstractColumn* column) {
	CONDITONAL_LOCK_RETURN;
	cbXColumn->setColumn(column, m_curve->xColumnPath());
	updateValuesWidgets();
}

void XYCurveDock::curveYColumnChanged(const AbstractColumn* column) {
	CONDITONAL_LOCK_RETURN;
	cbYColumn->setColumn(column, m_curve->yColumnPath());
	updateValuesWidgets();
}
void XYCurveDock::curveLegendVisibleChanged(bool on) {
	CONDITONAL_LOCK_RETURN;
	uiGeneralTab.chkLegendVisible->setChecked(on);
}
void XYCurveDock::curveVisibilityChanged(bool on) {
	CONDITONAL_LOCK_RETURN;
	uiGeneralTab.chkVisible->setChecked(on);
}

// Line-Tab
void XYCurveDock::curveLineTypeChanged(XYCurve::LineType type) {
	CONDITONAL_LOCK_RETURN;
	ui.cbLineType->setCurrentIndex((int)type);
}
void XYCurveDock::curveLineSkipGapsChanged(bool skip) {
	CONDITONAL_LOCK_RETURN;
	ui.chkLineSkipGaps->setChecked(skip);
}
void XYCurveDock::curveLineIncreasingXOnlyChanged(bool incr) {
	CONDITONAL_LOCK_RETURN;
	ui.chkLineIncreasingXOnly->setChecked(incr);
}
void XYCurveDock::curveLineInterpolationPointsCountChanged(int count) {
	CONDITONAL_LOCK_RETURN;
	ui.sbLineInterpolationPointsCount->setValue(count);
}

// Values-Tab
void XYCurveDock::curveValuesTypeChanged(XYCurve::ValuesType type) {
	CONDITONAL_LOCK_RETURN;
	ui.cbValuesType->setCurrentIndex((int)type);
}
void XYCurveDock::curveValuesColumnChanged(const AbstractColumn* column) {
	CONDITONAL_LOCK_RETURN;
	cbValuesColumn->setColumn(column, m_curve->valuesColumnPath());
}
void XYCurveDock::curveValuesPositionChanged(XYCurve::ValuesPosition position) {
	CONDITONAL_LOCK_RETURN;
	ui.cbValuesPosition->setCurrentIndex((int)position);
}
void XYCurveDock::curveValuesDistanceChanged(qreal distance) {
	CONDITONAL_LOCK_RETURN;
	ui.sbValuesDistance->setValue(Worksheet::convertFromSceneUnits(distance, Worksheet::Unit::Point));
}
void XYCurveDock::curveValuesRotationAngleChanged(qreal angle) {
	CONDITONAL_LOCK_RETURN;
	ui.sbValuesRotation->setValue(angle);
}
void XYCurveDock::curveValuesNumericFormatChanged(char format) {
	CONDITONAL_LOCK_RETURN;
	ui.cbValuesNumericFormat->setCurrentIndex(ui.cbValuesNumericFormat->findData(format));
}
void XYCurveDock::curveValuesPrecisionChanged(int precision) {
	CONDITONAL_LOCK_RETURN;
	ui.sbValuesPrecision->setValue(precision);
}
void XYCurveDock::curveValuesDateTimeFormatChanged(const QString& format) {
	CONDITONAL_LOCK_RETURN;
	ui.cbValuesDateTimeFormat->setCurrentText(format);
}
void XYCurveDock::curveValuesOpacityChanged(qreal opacity) {
	CONDITONAL_LOCK_RETURN;
	ui.sbValuesOpacity->setValue(round(opacity * 100.0));
}
void XYCurveDock::curveValuesPrefixChanged(const QString& prefix) {
	CONDITONAL_LOCK_RETURN;
	ui.leValuesPrefix->setText(prefix);
}
void XYCurveDock::curveValuesSuffixChanged(const QString& suffix) {
	CONDITONAL_LOCK_RETURN;
	ui.leValuesSuffix->setText(suffix);
}
void XYCurveDock::curveValuesFontChanged(QFont font) {
	CONDITONAL_LOCK_RETURN;
	font.setPointSizeF(round(Worksheet::convertFromSceneUnits(font.pixelSize(), Worksheet::Unit::Point)));
	ui.kfrValuesFont->setFont(font);
}
void XYCurveDock::curveValuesColorChanged(QColor color) {
	CONDITONAL_LOCK_RETURN;
	ui.kcbValuesColor->setColor(color);
}

//"Error bars"-Tab
void XYCurveDock::curveXErrorTypeChanged(XYCurve::ErrorType type) {
	CONDITONAL_LOCK_RETURN;
	ui.cbXErrorType->setCurrentIndex((int)type);
}
void XYCurveDock::curveXErrorPlusColumnChanged(const AbstractColumn* column) {
	CONDITONAL_LOCK_RETURN;
	cbXErrorPlusColumn->setColumn(column, m_curve->xErrorPlusColumnPath());
}
void XYCurveDock::curveXErrorMinusColumnChanged(const AbstractColumn* column) {
	CONDITONAL_LOCK_RETURN;
	cbXErrorMinusColumn->setColumn(column, m_curve->xErrorMinusColumnPath());
}
void XYCurveDock::curveYErrorTypeChanged(XYCurve::ErrorType type) {
	CONDITONAL_LOCK_RETURN;
	ui.cbYErrorType->setCurrentIndex((int)type);
}
void XYCurveDock::curveYErrorPlusColumnChanged(const AbstractColumn* column) {
	CONDITONAL_LOCK_RETURN;
	cbYErrorPlusColumn->setColumn(column, m_curve->yErrorPlusColumnPath());
}
void XYCurveDock::curveYErrorMinusColumnChanged(const AbstractColumn* column) {
	CONDITONAL_LOCK_RETURN;
	cbYErrorMinusColumn->setColumn(column, m_curve->yErrorMinusColumnPath());
}

//"Margin Plot"-Tab
void XYCurveDock::curveRugEnabledChanged(bool status) {
	CONDITONAL_LOCK_RETURN;
	ui.chkRugEnabled->setChecked(status);
}
void XYCurveDock::curveRugOrientationChanged(WorksheetElement::Orientation orientation) {
	CONDITONAL_LOCK_RETURN;
	ui.cbRugOrientation->setCurrentIndex(static_cast<int>(orientation));
}
void XYCurveDock::curveRugLengthChanged(double value) {
	CONDITONAL_LOCK_RETURN;
	ui.sbRugLength->setValue(Worksheet::convertFromSceneUnits(value, Worksheet::Unit::Point));
}
void XYCurveDock::curveRugWidthChanged(double value) {
	CONDITONAL_LOCK_RETURN;
	ui.sbRugWidth->setValue(Worksheet::convertFromSceneUnits(value, Worksheet::Unit::Point));
}
void XYCurveDock::curveRugOffsetChanged(double value) {
	CONDITONAL_LOCK_RETURN;
	ui.sbRugOffset->setValue(Worksheet::convertFromSceneUnits(value, Worksheet::Unit::Point));
}

//*************************************************************
//************************* Settings **************************
//*************************************************************
void XYCurveDock::load() {
	// General
	// This data is read in XYCurveDock::setCurves().

	// Line
	ui.cbLineType->setCurrentIndex((int)m_curve->lineType());
	ui.chkLineSkipGaps->setChecked(m_curve->lineSkipGaps());
	ui.sbLineInterpolationPointsCount->setValue(m_curve->lineInterpolationPointsCount());

	// Values
	ui.cbValuesType->setCurrentIndex((int)m_curve->valuesType());
	ui.cbValuesPosition->setCurrentIndex((int)m_curve->valuesPosition());
	ui.sbValuesDistance->setValue(Worksheet::convertFromSceneUnits(m_curve->valuesDistance(), Worksheet::Unit::Point));
	ui.sbValuesRotation->setValue(m_curve->valuesRotationAngle());
	ui.sbValuesOpacity->setValue(round(m_curve->valuesOpacity() * 100.0));
	ui.sbValuesPrecision->setValue(m_curve->valuesPrecision());
	ui.cbValuesNumericFormat->setCurrentIndex(ui.cbValuesNumericFormat->findData(m_curve->valuesNumericFormat()));
	ui.cbValuesDateTimeFormat->setCurrentText(m_curve->valuesDateTimeFormat());
	ui.leValuesPrefix->setText(m_curve->valuesPrefix());
	ui.leValuesSuffix->setText(m_curve->valuesSuffix());
	QFont valuesFont = m_curve->valuesFont();
	valuesFont.setPointSizeF(round(Worksheet::convertFromSceneUnits(valuesFont.pixelSize(), Worksheet::Unit::Point)));
	ui.kfrValuesFont->setFont(valuesFont);
	ui.kcbValuesColor->setColor(m_curve->valuesColor());
	this->updateValuesWidgets();

	// Error bars
	ui.cbXErrorType->setCurrentIndex((int)m_curve->xErrorType());
	ui.cbYErrorType->setCurrentIndex((int)m_curve->yErrorType());

	// Margin plots
	ui.chkRugEnabled->setChecked(m_curve->rugEnabled());
	ui.cbRugOrientation->setCurrentIndex(static_cast<int>(m_curve->rugOrientation()));
	ui.sbRugWidth->setValue(Worksheet::convertFromSceneUnits(m_curve->rugWidth(), Worksheet::Unit::Point));
	ui.sbRugLength->setValue(Worksheet::convertFromSceneUnits(m_curve->rugLength(), Worksheet::Unit::Point));
	ui.sbRugOffset->setValue(Worksheet::convertFromSceneUnits(m_curve->rugOffset(), Worksheet::Unit::Point));
}

void XYCurveDock::loadConfigFromTemplate(KConfig& config) {
	// extract the name of the template from the file name
	QString name;
	int index = config.name().lastIndexOf(QLatin1String("/"));
	if (index != -1)
		name = config.name().right(config.name().size() - index - 1);
	else
		name = config.name();

	int size = m_curvesList.size();
	if (size > 1)
		m_curve->beginMacro(i18n("%1 xy-curves: template \"%2\" loaded", size, name));
	else
		m_curve->beginMacro(i18n("%1: template \"%2\" loaded", m_curve->name(), name));

	this->loadConfig(config);

	m_curve->endMacro();
}

void XYCurveDock::loadConfig(KConfig& config) {
	KConfigGroup group = config.group("XYCurve");

	// General
	// we don't load/save the settings in the general-tab, since they are not style related.
	// It doesn't make sense to load/save them in the template.
	// This data is read in XYCurveDock::setCurves().

	// Line
	ui.cbLineType->setCurrentIndex(group.readEntry("LineType", (int)m_curve->lineType()));
	ui.chkLineSkipGaps->setChecked(group.readEntry("LineSkipGaps", m_curve->lineSkipGaps()));
	ui.sbLineInterpolationPointsCount->setValue(group.readEntry("LineInterpolationPointsCount", m_curve->lineInterpolationPointsCount()));
	lineWidget->loadConfig(group);
	dropLineWidget->loadConfig(group);

	// Symbols
	symbolWidget->loadConfig(group);

	// Values
	ui.cbValuesType->setCurrentIndex(group.readEntry("ValuesType", (int)m_curve->valuesType()));
	ui.cbValuesPosition->setCurrentIndex(group.readEntry("ValuesPosition", (int)m_curve->valuesPosition()));
	ui.sbValuesDistance->setValue(Worksheet::convertFromSceneUnits(group.readEntry("ValuesDistance", m_curve->valuesDistance()), Worksheet::Unit::Point));
	ui.sbValuesRotation->setValue(group.readEntry("ValuesRotation", m_curve->valuesRotationAngle()));
	ui.sbValuesOpacity->setValue(round(group.readEntry("ValuesOpacity", m_curve->valuesOpacity()) * 100.0));
	ui.leValuesPrefix->setText(group.readEntry("ValuesPrefix", m_curve->valuesPrefix()));
	ui.leValuesSuffix->setText(group.readEntry("ValuesSuffix", m_curve->valuesSuffix()));
	QFont valuesFont = m_curve->valuesFont();
	valuesFont.setPointSizeF(round(Worksheet::convertFromSceneUnits(valuesFont.pixelSize(), Worksheet::Unit::Point)));
	ui.kfrValuesFont->setFont(group.readEntry("ValuesFont", valuesFont));
	ui.kcbValuesColor->setColor(group.readEntry("ValuesColor", m_curve->valuesColor()));

	// Filling
	backgroundWidget->loadConfig(group);

	// Error bars
	ui.cbXErrorType->setCurrentIndex(group.readEntry("XErrorType", (int)m_curve->xErrorType()));
	ui.cbYErrorType->setCurrentIndex(group.readEntry("YErrorType", (int)m_curve->yErrorType()));
	errorBarsLineWidget->loadConfig(group);
}

void XYCurveDock::saveConfigAsTemplate(KConfig& config) {
	KConfigGroup group = config.group("XYCurve");

	// General
	// we don't load/save the settings in the general-tab, since they are not style related.
	// It doesn't make sense to load/save them in the template.

	group.writeEntry("LineType", ui.cbLineType->currentIndex());
	group.writeEntry("LineSkipGaps", ui.chkLineSkipGaps->isChecked());
	group.writeEntry("LineInterpolationPointsCount", ui.sbLineInterpolationPointsCount->value());
	lineWidget->saveConfig(group);
	dropLineWidget->saveConfig(group);

	// Symbols
	symbolWidget->saveConfig(group);

	// Values
	group.writeEntry("ValuesType", ui.cbValuesType->currentIndex());
	group.writeEntry("ValuesPosition", ui.cbValuesPosition->currentIndex());
	group.writeEntry("ValuesDistance", Worksheet::convertToSceneUnits(ui.sbValuesDistance->value(), Worksheet::Unit::Point));
	group.writeEntry("ValuesRotation", ui.sbValuesRotation->value());
	group.writeEntry("ValuesOpacity", ui.sbValuesOpacity->value() / 100.0);
	group.writeEntry("valuesNumericFormat", ui.cbValuesNumericFormat->currentText());
	group.writeEntry("valuesPrecision", ui.sbValuesPrecision->value());
	group.writeEntry("valuesDateTimeFormat", ui.cbValuesDateTimeFormat->currentText());
	group.writeEntry("ValuesPrefix", ui.leValuesPrefix->text());
	group.writeEntry("ValuesSuffix", ui.leValuesSuffix->text());
	group.writeEntry("ValuesFont", ui.kfrValuesFont->font());
	group.writeEntry("ValuesColor", ui.kcbValuesColor->color());

	// Filling
	backgroundWidget->saveConfig(group);

	// Error bars
	group.writeEntry("XErrorType", ui.cbXErrorType->currentIndex());
	group.writeEntry("YErrorType", ui.cbYErrorType->currentIndex());
	errorBarsLineWidget->saveConfig(group);

	config.sync();
}
