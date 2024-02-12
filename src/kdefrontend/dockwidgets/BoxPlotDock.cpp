/*
	File                 : BoxPlotDock.cpp
	Project              : LabPlot
	Description          : Dock widget for the reference line on the plot
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2020-2024 Alexander Semke <alexander.semke@web.de>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "BoxPlotDock.h"
#include "backend/core/AbstractColumn.h"
#include "backend/lib/macros.h"
#include "backend/worksheet/Worksheet.h"
#include "commonfrontend/widgets/TreeViewComboBox.h"
#include "kdefrontend/GuiTools.h"
#include "kdefrontend/TemplateHandler.h"
#include "kdefrontend/widgets/BackgroundWidget.h"
#include "kdefrontend/widgets/LineWidget.h"
#include "kdefrontend/widgets/SymbolWidget.h"

#include <QPushButton>

#include <KConfig>
#include <KLocalizedString>

BoxPlotDock::BoxPlotDock(QWidget* parent)
	: BaseDock(parent) {
	ui.setupUi(this);
	setPlotRangeCombobox(ui.cbPlotRanges);
	setBaseWidgets(ui.leName, ui.teComment);
	setVisibilityWidgets(ui.chkVisible, ui.chkLegendVisible);

	// Tab "General"
	m_buttonNew = new QPushButton();
	m_buttonNew->setIcon(QIcon::fromTheme(QStringLiteral("list-add")));
	connect(m_buttonNew, &QPushButton::clicked, this, &BoxPlotDock::addDataColumn);

	m_gridLayout = new QGridLayout(ui.frameDataColumns);
	m_gridLayout->setContentsMargins(0, 0, 0, 0);
	m_gridLayout->setHorizontalSpacing(2);
	m_gridLayout->setVerticalSpacing(2);
	ui.frameDataColumns->setLayout(m_gridLayout);

	ui.cbWhiskersType->addItem(QStringLiteral("min/max"));
	ui.cbWhiskersType->addItem(QStringLiteral("Tukey"));
	ui.cbWhiskersType->addItem(QStringLiteral("mean ∓ k*SD"));
	ui.cbWhiskersType->addItem(QStringLiteral("median ∓ k*MAD"));
	ui.cbWhiskersType->addItem(i18n("10/90 percentiles"));
	ui.cbWhiskersType->addItem(i18n("5/95 percentiles"));
	ui.cbWhiskersType->addItem(i18n("1/99 percentiles"));

	ui.cbOrientation->addItem(i18n("Horizontal"));
	ui.cbOrientation->addItem(i18n("Vertical"));

	ui.cbOrdering->addItem(i18n("None"));
	ui.cbOrdering->addItem(i18n("By Median, Ascending"));
	ui.cbOrdering->addItem(i18n("By Median, Descending"));
	ui.cbOrdering->addItem(i18n("By Mean, Ascending"));
	ui.cbOrdering->addItem(i18n("By Mean, Descending"));

	QString msg = i18n("If multiple data sets are provided, define how they should be ordered or use 'None' to keep the original order.");
	ui.lOrdering->setToolTip(msg);
	ui.cbOrdering->setToolTip(msg);

	msg = i18n("If checked, the box width is made proportional to the square root of the number of data points.");
	ui.lVariableWidth->setToolTip(msg);
	ui.chkVariableWidth->setToolTip(msg);

	msg = i18n("Parameter controlling the range of the inner fences of the box plot.");
	ui.lWhiskersRangeParameter->setToolTip(msg);
	ui.leWhiskersRangeParameter->setToolTip(msg);

	// Tab "Box"
	msg = i18n("Select the data column for which the properties should be shown and edited");
	ui.lNumber->setToolTip(msg);
	ui.cbNumber->setToolTip(msg);

	msg = i18n("Specify the factor in percent to control the width of the box relative to its default value.");
	ui.lWidthFactor->setToolTip(msg);
	ui.sbWidthFactor->setToolTip(msg);

	// Tab "Box"
	auto* gridLayout = static_cast<QGridLayout*>(ui.tabBox->layout());
	backgroundWidget = new BackgroundWidget(ui.tabBox);
	gridLayout->addWidget(backgroundWidget, 5, 0, 1, 3);

	// lines
	borderLineWidget = new LineWidget(ui.tabBox);
	gridLayout->addWidget(borderLineWidget, 8, 0, 1, 3);

	medianLineWidget = new LineWidget(ui.tabBox);
	gridLayout->addWidget(medianLineWidget, 11, 0, 1, 3);

	// Tab "Markers"
	gridLayout = static_cast<QGridLayout*>(ui.tabSymbol->layout());
	symbolWidget = new SymbolWidget(ui.tabSymbol);
	gridLayout->addWidget(symbolWidget, 2, 0, 1, 3);

	// Tab "Whiskers"
	gridLayout = static_cast<QGridLayout*>(ui.tabWhiskers->layout());
	whiskersLineWidget = new LineWidget(ui.tabBox);
	gridLayout->addWidget(whiskersLineWidget, 1, 0, 1, 3);

	whiskersCapLineWidget = new LineWidget(ui.tabBox);
	gridLayout->addWidget(whiskersCapLineWidget, 5, 0, 1, 3);

	// adjust layouts in the tabs
	for (int i = 0; i < ui.tabWidget->count(); ++i) {
		auto* layout = dynamic_cast<QGridLayout*>(ui.tabWidget->widget(i)->layout());
		if (!layout)
			continue;

		layout->setContentsMargins(2, 2, 2, 2);
		layout->setHorizontalSpacing(2);
		layout->setVerticalSpacing(2);
	}

	// Validators
	ui.leWhiskersRangeParameter->setValidator(new QDoubleValidator(ui.leWhiskersRangeParameter));

	// SLOTS
	// Tab "General"
	connect(ui.cbOrdering, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &BoxPlotDock::orderingChanged);
	connect(ui.cbOrientation, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &BoxPlotDock::orientationChanged);
	connect(ui.chkVariableWidth, &QCheckBox::toggled, this, &BoxPlotDock::variableWidthChanged);
	connect(ui.chkNotches, &QCheckBox::toggled, this, &BoxPlotDock::notchesEnabledChanged);

	// Tab "Box"
	connect(ui.cbNumber, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &BoxPlotDock::currentBoxChanged);
	connect(ui.sbWidthFactor, QOverload<int>::of(&QSpinBox::valueChanged), this, &BoxPlotDock::widthFactorChanged);

	// Tab "Markers"
	connect(ui.rbMean, &QRadioButton::toggled, this, &BoxPlotDock::symbolCategoryChanged);
	connect(ui.rbMedian, &QRadioButton::toggled, this, &BoxPlotDock::symbolCategoryChanged);
	connect(ui.rbOutlier, &QRadioButton::toggled, this, &BoxPlotDock::symbolCategoryChanged);
	connect(ui.rbFarOut, &QRadioButton::toggled, this, &BoxPlotDock::symbolCategoryChanged);
	connect(ui.rbJitter, &QRadioButton::toggled, this, &BoxPlotDock::symbolCategoryChanged);
	connect(ui.rbWhiskerEnd, &QRadioButton::toggled, this, &BoxPlotDock::symbolCategoryChanged);
	connect(ui.chkJitteringEnabled, &QCheckBox::toggled, this, &BoxPlotDock::jitteringEnabledChanged);

	// Tab "Whiskers"
	connect(ui.cbWhiskersType, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &BoxPlotDock::whiskersTypeChanged);
	connect(ui.leWhiskersRangeParameter, &QLineEdit::textChanged, this, &BoxPlotDock::whiskersRangeParameterChanged);
	connect(ui.sbWhiskersCapSize, QOverload<double>::of(&NumberSpinBox::valueChanged), this, &BoxPlotDock::whiskersCapSizeChanged);

	// Margin Plots
	connect(ui.chkRugEnabled, &QCheckBox::toggled, this, &BoxPlotDock::rugEnabledChanged);
	connect(ui.sbRugLength, QOverload<double>::of(&NumberSpinBox::valueChanged), this, &BoxPlotDock::rugLengthChanged);
	connect(ui.sbRugWidth, QOverload<double>::of(&NumberSpinBox::valueChanged), this, &BoxPlotDock::rugWidthChanged);
	connect(ui.sbRugOffset, QOverload<double>::of(&NumberSpinBox::valueChanged), this, &BoxPlotDock::rugOffsetChanged);

	// template handler
	auto* frame = new QFrame(this);
	auto* layout = new QHBoxLayout(frame);
	layout->setContentsMargins(0, 11, 0, 11);

	auto* templateHandler = new TemplateHandler(this, QLatin1String("BoxPlot"));
	layout->addWidget(templateHandler);
	connect(templateHandler, &TemplateHandler::loadConfigRequested, this, &BoxPlotDock::loadConfigFromTemplate);
	connect(templateHandler, &TemplateHandler::saveConfigRequested, this, &BoxPlotDock::saveConfigAsTemplate);
	connect(templateHandler, &TemplateHandler::info, this, &BoxPlotDock::info);

	ui.verticalLayout->addWidget(frame);
}

void BoxPlotDock::setBoxPlots(QList<BoxPlot*> list) {
	CONDITIONAL_LOCK_RETURN;
	m_boxPlots = list;
	m_boxPlot = list.first();
	setAspects(list);
	Q_ASSERT(m_boxPlot);
	setModel();

	QList<Background*> backgrounds;
	QList<Line*> borderLines;
	QList<Line*> medianLines;
	QList<Line*> whiskersLines;
	QList<Line*> whiskersCapLines;
	for (auto* plot : m_boxPlots) {
		backgrounds << plot->backgroundAt(0);
		borderLines << plot->borderLineAt(0);
		medianLines << plot->medianLineAt(0);
		whiskersLines << plot->whiskersLine();
		whiskersCapLines << plot->whiskersCapLine();
	}
	backgroundWidget->setBackgrounds(backgrounds);
	borderLineWidget->setLines(borderLines);
	medianLineWidget->setLines(medianLines);
	whiskersLineWidget->setLines(whiskersLines);
	whiskersCapLineWidget->setLines(whiskersCapLines);

	// show the properties of the first box plot
	ui.chkLegendVisible->setChecked(m_boxPlot->legendVisible());
	ui.chkVisible->setChecked(m_boxPlot->isVisible());
	load();
	loadDataColumns();

	updatePlotRangeList();

	// set the current locale
	updateLocale();

	// SIGNALs/SLOTs
	// general
	connect(m_boxPlot, &BoxPlot::orientationChanged, this, &BoxPlotDock::plotOrientationChanged);
	connect(m_boxPlot, &BoxPlot::variableWidthChanged, this, &BoxPlotDock::plotVariableWidthChanged);
	connect(m_boxPlot, &BoxPlot::notchesEnabledChanged, this, &BoxPlotDock::plotNotchesEnabledChanged);
	connect(m_boxPlot, &BoxPlot::dataColumnsChanged, this, &BoxPlotDock::plotDataColumnsChanged);
	connect(m_boxPlot, &BoxPlot::widthFactorChanged, this, &BoxPlotDock::plotWidthFactorChanged);

	// symbols
	connect(m_boxPlot, &BoxPlot::jitteringEnabledChanged, this, &BoxPlotDock::plotJitteringEnabledChanged);

	// whiskers
	connect(m_boxPlot, &BoxPlot::whiskersTypeChanged, this, &BoxPlotDock::plotWhiskersTypeChanged);
	connect(m_boxPlot, &BoxPlot::whiskersRangeParameterChanged, this, &BoxPlotDock::plotWhiskersRangeParameterChanged);
	connect(m_boxPlot, &BoxPlot::whiskersCapSizeChanged, this, &BoxPlotDock::plotWhiskersCapSizeChanged);

	//"Margin Plots"-Tab
	connect(m_boxPlot, &BoxPlot::rugEnabledChanged, this, &BoxPlotDock::plotRugEnabledChanged);
	connect(m_boxPlot, &BoxPlot::rugLengthChanged, this, &BoxPlotDock::plotRugLengthChanged);
	connect(m_boxPlot, &BoxPlot::rugWidthChanged, this, &BoxPlotDock::plotRugWidthChanged);
	connect(m_boxPlot, &BoxPlot::rugOffsetChanged, this, &BoxPlotDock::plotRugOffsetChanged);
}

void BoxPlotDock::setModel() {
	auto* model = aspectModel();
	model->enablePlottableColumnsOnly(true);
	model->enableShowPlotDesignation(true);

	QList<AspectType> list{AspectType::Column};
	model->setSelectableAspects(list);
}

/*
 * updates the locale in the widgets. called when the application settins are changed.
 */
void BoxPlotDock::updateLocale() {
	ui.leWhiskersRangeParameter->setLocale(QLocale());
	borderLineWidget->updateLocale();
	medianLineWidget->updateLocale();
	whiskersLineWidget->updateLocale();
	whiskersCapLineWidget->updateLocale();
}

void BoxPlotDock::loadDataColumns() {
	// add the combobox for the first column, is always present
	if (m_dataComboBoxes.count() == 0)
		addDataColumn();

	int count = m_boxPlot->dataColumns().count();
	ui.cbNumber->clear();

	if (count != 0) {
		// box plot has already data columns, make sure we have the proper number of comboboxes
		int diff = count - m_dataComboBoxes.count();
		if (diff > 0) {
			for (int i = 0; i < diff; ++i)
				addDataColumn();
		} else if (diff < 0) {
			for (int i = diff; i != 0; ++i)
				removeDataColumn();
		}

		// show the columns in the comboboxes
		auto* model = aspectModel();
		for (int i = 0; i < count; ++i) {
			m_dataComboBoxes.at(i)->setModel(model); // the model might have changed in-between, reset the current model
			m_dataComboBoxes.at(i)->setAspect(m_boxPlot->dataColumns().at(i));
		}

		// show columns names in the combobox for the selection of the box to be modified
		for (int i = 0; i < count; ++i)
			if (m_boxPlot->dataColumns().at(i))
				ui.cbNumber->addItem(m_boxPlot->dataColumns().at(i)->name());
	} else {
		// no data columns set in the box plot yet, we show the first combo box only
		m_dataComboBoxes.first()->setAspect(nullptr);
		for (int i = 1; i < m_dataComboBoxes.count(); ++i)
			removeDataColumn();
	}

	// disable data column widgets if we're modifying more than one box plot at the same time
	bool enabled = (m_boxPlots.count() == 1);
	m_buttonNew->setVisible(enabled);
	for (auto* cb : m_dataComboBoxes)
		cb->setEnabled(enabled);
	for (auto* b : m_removeButtons)
		b->setVisible(enabled);

	// select the first column after all of them were added to the combobox
	ui.cbNumber->setCurrentIndex(0);
}

void BoxPlotDock::setDataColumns() const {
	QVector<const AbstractColumn*> columns;

	for (auto* cb : m_dataComboBoxes) {
		auto* aspect = cb->currentAspect();
		if (aspect && aspect->type() == AspectType::Column)
			columns << static_cast<AbstractColumn*>(aspect);
	}

	m_boxPlot->setDataColumns(columns);
}

//**********************************************************
//*** SLOTs for changes triggered in BoxPlotDock *****
//**********************************************************
void BoxPlotDock::addDataColumn() {
	auto* cb = new TreeViewComboBox(this);

	static const QList<AspectType> list{AspectType::Folder,
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
	cb->setTopLevelClasses(list);
	cb->setModel(aspectModel());
	connect(cb, &TreeViewComboBox::currentModelIndexChanged, this, &BoxPlotDock::dataColumnChanged);

	int index = m_dataComboBoxes.size();

	if (index == 0) {
		QSizePolicy sizePolicy1(QSizePolicy::Expanding, QSizePolicy::Preferred);
		sizePolicy1.setHorizontalStretch(0);
		sizePolicy1.setVerticalStretch(0);
		sizePolicy1.setHeightForWidth(cb->sizePolicy().hasHeightForWidth());
		cb->setSizePolicy(sizePolicy1);
	} else {
		auto* button = new QPushButton();
		button->setIcon(QIcon::fromTheme(QStringLiteral("list-remove")));
		connect(button, &QPushButton::clicked, this, &BoxPlotDock::removeDataColumn);
		m_gridLayout->addWidget(button, index, 1, 1, 1);
		m_removeButtons << button;

		ui.lOrdering->setEnabled(true);
		ui.cbOrdering->setEnabled(true);
	}

	m_gridLayout->addWidget(cb, index, 0, 1, 1);
	m_gridLayout->addWidget(m_buttonNew, index + 1, 1, 1, 1);

	m_dataComboBoxes << cb;
	ui.lDataColumn->setText(i18n("Columns:"));
}

void BoxPlotDock::removeDataColumn() {
	auto* sender = static_cast<QPushButton*>(QObject::sender());
	if (sender) {
		// remove button was clicked, determin which one and
		// delete it together with the corresponding combobox
		for (int i = 0; i < m_removeButtons.count(); ++i) {
			if (sender == m_removeButtons.at(i)) {
				delete m_dataComboBoxes.takeAt(i + 1);
				delete m_removeButtons.takeAt(i);
			}
		}
	} else {
		// no sender is available, the function is being called directly in loadDataColumns().
		// delete the last remove button together with the corresponding combobox
		int index = m_removeButtons.count() - 1;
		if (index >= 0) {
			delete m_dataComboBoxes.takeAt(index + 1);
			delete m_removeButtons.takeAt(index);
		}
	}

	if (!m_removeButtons.isEmpty()) {
		ui.lDataColumn->setText(i18n("Columns:"));
		ui.lOrdering->setEnabled(true);
		ui.cbOrdering->setEnabled(true);
	} else {
		ui.lDataColumn->setText(i18n("Column:"));
		ui.lOrdering->setEnabled(false);
		ui.cbOrdering->setEnabled(false);
	}

	if (!m_initializing)
		setDataColumns();
}

void BoxPlotDock::dataColumnChanged(const QModelIndex&) {
	CONDITIONAL_LOCK_RETURN;

	setDataColumns();
}

void BoxPlotDock::orderingChanged(int index) {
	CONDITIONAL_LOCK_RETURN;

	auto ordering = static_cast<BoxPlot::Ordering>(index);
	for (auto* boxPlot : m_boxPlots)
		boxPlot->setOrdering(ordering);
}

void BoxPlotDock::orientationChanged(int index) {
	CONDITIONAL_LOCK_RETURN;

	auto orientation = BoxPlot::Orientation(index);
	for (auto* boxPlot : m_boxPlots)
		boxPlot->setOrientation(orientation);
}

void BoxPlotDock::variableWidthChanged(bool state) {
	CONDITIONAL_LOCK_RETURN;

	for (auto* boxPlot : m_boxPlots)
		boxPlot->setVariableWidth(state);
}

void BoxPlotDock::notchesEnabledChanged(bool state) {
	CONDITIONAL_LOCK_RETURN;

	for (auto* boxPlot : m_boxPlots)
		boxPlot->setNotchesEnabled(state);
}

//"Box"-tab
/*!
 * called when the current box number was changed, shows the box properties for the selected box.
 */
void BoxPlotDock::currentBoxChanged(int index) {
	if (index == -1)
		return;

	CONDITIONAL_LOCK_RETURN;

	QList<Background*> backgrounds;
	QList<Line*> borderLines;
	QList<Line*> medianLines;
	for (auto* plot : m_boxPlots) {
		auto* background = plot->backgroundAt(index);
		if (background)
			backgrounds << background;

		auto* line = plot->borderLineAt(index);
		if (line)
			borderLines << line;

		line = plot->medianLineAt(index);
		if (line)
			medianLines << line;
	}

	backgroundWidget->setBackgrounds(backgrounds);
	borderLineWidget->setLines(borderLines);
	medianLineWidget->setLines(medianLines);
}

void BoxPlotDock::widthFactorChanged(int value) {
	CONDITIONAL_LOCK_RETURN;

	double factor = (double)value / 100.;
	for (auto* boxPlot : m_boxPlots)
		boxPlot->setWidthFactor(factor);
}

// markers
void BoxPlotDock::symbolCategoryChanged() {
	QList<Symbol*> symbols;

	for (auto* plot : m_boxPlots) {
		if (ui.rbMean->isChecked())
			symbols << plot->symbolMean();
		else if (ui.rbMedian->isChecked())
			symbols << plot->symbolMedian();
		else if (ui.rbOutlier->isChecked())
			symbols << plot->symbolOutlier();
		else if (ui.rbFarOut->isChecked())
			symbols << plot->symbolFarOut();
		else if (ui.rbJitter->isChecked())
			symbols << plot->symbolData();
		else if (ui.rbWhiskerEnd->isChecked())
			symbols << plot->symbolWhiskerEnd();
	}

	symbolWidget->setSymbols(symbols);
}

void BoxPlotDock::jitteringEnabledChanged(bool state) {
	CONDITIONAL_LOCK_RETURN;

	for (auto* boxPlot : m_boxPlots)
		boxPlot->setJitteringEnabled(state);
}

// whiskers
void BoxPlotDock::whiskersTypeChanged(int index) {
	auto type = BoxPlot::WhiskersType(index);
	ui.rbOutlier->setEnabled(type != BoxPlot::WhiskersType::MinMax);
	ui.rbFarOut->setEnabled(type == BoxPlot::WhiskersType::IQR);

	// range parameter 'k' only available for IQR(=Tukey), SD and MAD
	bool visible = (type == BoxPlot::WhiskersType::IQR) || (type == BoxPlot::WhiskersType::SD) || (type == BoxPlot::WhiskersType::MAD);
	ui.lWhiskersRangeParameter->setVisible(visible);
	ui.leWhiskersRangeParameter->setVisible(visible);

	CONDITIONAL_LOCK_RETURN;

	for (auto* boxPlot : m_boxPlots)
		boxPlot->setWhiskersType(type);
}

void BoxPlotDock::whiskersRangeParameterChanged(const QString& text) {
	CONDITIONAL_LOCK_RETURN;

	bool ok;
	double value{QLocale().toDouble(text, &ok)};
	if (!ok)
		return;

	for (auto* boxPlot : m_boxPlots)
		boxPlot->setWhiskersRangeParameter(value);
}

// whiskers cap
void BoxPlotDock::whiskersCapSizeChanged(double value) const {
	CONDITIONAL_RETURN_NO_LOCK;

	float size = Worksheet::convertToSceneUnits(value, Worksheet::Unit::Point);
	for (auto* boxPlot : m_boxPlots)
		boxPlot->setWhiskersCapSize(size);
}

//"Margin Plots"-Tab
void BoxPlotDock::rugEnabledChanged(bool state) {
	CONDITIONAL_LOCK_RETURN;

	for (auto* curve : qAsConst(m_boxPlots))
		curve->setRugEnabled(state);
}

void BoxPlotDock::rugLengthChanged(double value) const {
	CONDITIONAL_RETURN_NO_LOCK;

	const double length = Worksheet::convertToSceneUnits(value, Worksheet::Unit::Point);
	for (auto* curve : qAsConst(m_boxPlots))
		curve->setRugLength(length);
}

void BoxPlotDock::rugWidthChanged(double value) const {
	CONDITIONAL_RETURN_NO_LOCK;

	const double width = Worksheet::convertToSceneUnits(value, Worksheet::Unit::Point);
	for (auto* curve : qAsConst(m_boxPlots))
		curve->setRugWidth(width);
}

void BoxPlotDock::rugOffsetChanged(double value) const {
	CONDITIONAL_RETURN_NO_LOCK;

	const double offset = Worksheet::convertToSceneUnits(value, Worksheet::Unit::Point);
	for (auto* curve : qAsConst(m_boxPlots))
		curve->setRugOffset(offset);
}

//*************************************************************
//******* SLOTs for changes triggered in BoxPlot ********
//*************************************************************
// general
void BoxPlotDock::plotDescriptionChanged(const AbstractAspect* aspect) {
	if (m_boxPlot != aspect)
		return;

	CONDITIONAL_LOCK_RETURN;
	if (aspect->name() != ui.leName->text())
		ui.leName->setText(aspect->name());
	else if (aspect->comment() != ui.teComment->text())
		ui.teComment->setText(aspect->comment());
}
void BoxPlotDock::plotDataColumnsChanged(const QVector<const AbstractColumn*>&) {
	CONDITIONAL_LOCK_RETURN;
	loadDataColumns();
}
void BoxPlotDock::plotOrderingChanged(BoxPlot::Ordering ordering) {
	CONDITIONAL_LOCK_RETURN;
	ui.cbOrdering->setCurrentIndex((int)ordering);
}
void BoxPlotDock::plotOrientationChanged(BoxPlot::Orientation orientation) {
	CONDITIONAL_LOCK_RETURN;
	ui.cbOrientation->setCurrentIndex((int)orientation);
}
void BoxPlotDock::plotVariableWidthChanged(bool on) {
	CONDITIONAL_LOCK_RETURN;
	ui.chkVariableWidth->setChecked(on);
}
void BoxPlotDock::plotNotchesEnabledChanged(bool on) {
	CONDITIONAL_LOCK_RETURN;
	ui.chkNotches->setChecked(on);
}

// box
void BoxPlotDock::plotWidthFactorChanged(double factor) {
	CONDITIONAL_LOCK_RETURN;
	ui.sbWidthFactor->setValue(round(factor * 100));
}

// symbols
void BoxPlotDock::plotJitteringEnabledChanged(bool status) {
	CONDITIONAL_LOCK_RETURN;
	ui.chkJitteringEnabled->setChecked(status);
}

// whiskers
void BoxPlotDock::plotWhiskersTypeChanged(BoxPlot::WhiskersType type) {
	CONDITIONAL_LOCK_RETURN;
	ui.cbWhiskersType->setCurrentIndex((int)type);
}
void BoxPlotDock::plotWhiskersRangeParameterChanged(double value) {
	CONDITIONAL_LOCK_RETURN;
	ui.leWhiskersRangeParameter->setText(QLocale().toString(value));
}

// whiskers cap
void BoxPlotDock::plotWhiskersCapSizeChanged(double size) {
	CONDITIONAL_LOCK_RETURN;
	ui.sbWhiskersCapSize->setValue(Worksheet::convertFromSceneUnits(size, Worksheet::Unit::Point));
}

//"Margin Plot"-Tab
void BoxPlotDock::plotRugEnabledChanged(bool status) {
	CONDITIONAL_LOCK_RETURN;
	ui.chkRugEnabled->setChecked(status);
}
void BoxPlotDock::plotRugLengthChanged(double value) {
	CONDITIONAL_LOCK_RETURN;
	ui.sbRugLength->setValue(Worksheet::convertFromSceneUnits(value, Worksheet::Unit::Point));
}
void BoxPlotDock::plotRugWidthChanged(double value) {
	CONDITIONAL_LOCK_RETURN;
	ui.sbRugWidth->setValue(Worksheet::convertFromSceneUnits(value, Worksheet::Unit::Point));
}
void BoxPlotDock::plotRugOffsetChanged(double value) {
	CONDITIONAL_LOCK_RETURN;
	ui.sbRugOffset->setValue(Worksheet::convertFromSceneUnits(value, Worksheet::Unit::Point));
}

//**********************************************************
//******************** SETTINGS ****************************
//**********************************************************
void BoxPlotDock::load() {
	// general
	ui.cbOrdering->setCurrentIndex((int)m_boxPlot->ordering());
	ui.cbOrientation->setCurrentIndex((int)m_boxPlot->orientation());
	ui.chkVariableWidth->setChecked(m_boxPlot->variableWidth());
	ui.chkNotches->setChecked(m_boxPlot->notchesEnabled());

	// box
	ui.sbWidthFactor->setValue(round(m_boxPlot->widthFactor()) * 100);

	// symbols
	symbolCategoryChanged();
	ui.chkJitteringEnabled->setChecked(m_boxPlot->jitteringEnabled());

	// whiskers
	ui.cbWhiskersType->setCurrentIndex((int)m_boxPlot->whiskersType());
	ui.leWhiskersRangeParameter->setText(QLocale().toString(m_boxPlot->whiskersRangeParameter()));

	// whiskers cap
	ui.sbWhiskersCapSize->setValue(Worksheet::convertFromSceneUnits(m_boxPlot->whiskersCapSize(), Worksheet::Unit::Point));

	// Margin plots
	ui.chkRugEnabled->setChecked(m_boxPlot->rugEnabled());
	ui.sbRugWidth->setValue(Worksheet::convertFromSceneUnits(m_boxPlot->rugWidth(), Worksheet::Unit::Point));
	ui.sbRugLength->setValue(Worksheet::convertFromSceneUnits(m_boxPlot->rugLength(), Worksheet::Unit::Point));
	ui.sbRugOffset->setValue(Worksheet::convertFromSceneUnits(m_boxPlot->rugOffset(), Worksheet::Unit::Point));
}

void BoxPlotDock::loadConfig(KConfig& config) {
	KConfigGroup group = config.group(QStringLiteral("BoxPlot"));

	// general
	ui.cbOrdering->setCurrentIndex(group.readEntry(QStringLiteral("Ordering"), (int)m_boxPlot->ordering()));
	ui.cbOrientation->setCurrentIndex(group.readEntry(QStringLiteral("Orientation"), (int)m_boxPlot->orientation()));
	ui.chkVariableWidth->setChecked(group.readEntry(QStringLiteral("QStringLiteral(VariableWidth"), m_boxPlot->variableWidth()));
	ui.chkNotches->setChecked(group.readEntry(QStringLiteral("NotchesEnabled"), m_boxPlot->notchesEnabled()));

	// box
	ui.sbWidthFactor->setValue(round(group.readEntry(QStringLiteral("WidthFactor"), m_boxPlot->widthFactor()) * 100));
	backgroundWidget->loadConfig(group);
	borderLineWidget->loadConfig(group);
	medianLineWidget->loadConfig(group);

	// symbols
	symbolCategoryChanged();
	ui.chkJitteringEnabled->setChecked(group.readEntry(QStringLiteral("JitteringEnabled"), m_boxPlot->jitteringEnabled()));

	// whiskers
	ui.cbWhiskersType->setCurrentIndex(group.readEntry(QStringLiteral("WhiskersType"), (int)m_boxPlot->whiskersType()));
	ui.leWhiskersRangeParameter->setText(QLocale().toString(m_boxPlot->whiskersRangeParameter()));
	whiskersLineWidget->loadConfig(group);

	// whiskers cap
	ui.sbWhiskersCapSize->setValue(
		Worksheet::convertFromSceneUnits(group.readEntry(QStringLiteral("WhiskersCapSize"), m_boxPlot->whiskersCapSize()), Worksheet::Unit::Point));
	whiskersCapLineWidget->loadConfig(group);

	// Margin plots
	ui.chkRugEnabled->setChecked(m_boxPlot->rugEnabled());
	ui.sbRugWidth->setValue(Worksheet::convertFromSceneUnits(m_boxPlot->rugWidth(), Worksheet::Unit::Point));
	ui.sbRugLength->setValue(Worksheet::convertFromSceneUnits(m_boxPlot->rugLength(), Worksheet::Unit::Point));
	ui.sbRugOffset->setValue(Worksheet::convertFromSceneUnits(m_boxPlot->rugOffset(), Worksheet::Unit::Point));
}

void BoxPlotDock::loadConfigFromTemplate(KConfig& config) {
	auto name = TemplateHandler::templateName(config);
	int size = m_boxPlots.size();
	if (size > 1)
		m_boxPlot->beginMacro(i18n("%1 box plots: template \"%2\" loaded", size, name));
	else
		m_boxPlot->beginMacro(i18n("%1: template \"%2\" loaded", m_boxPlot->name(), name));

	this->loadConfig(config);

	m_boxPlot->endMacro();
}

void BoxPlotDock::saveConfigAsTemplate(KConfig& config) {
	KConfigGroup group = config.group(QStringLiteral("BoxPlot"));

	// general
	group.writeEntry(QStringLiteral("Ordering"), ui.cbOrdering->currentIndex());
	group.writeEntry(QStringLiteral("Orientation"), ui.cbOrientation->currentIndex());
	group.writeEntry(QStringLiteral("VariableWidth"), ui.chkVariableWidth->isChecked());
	group.writeEntry(QStringLiteral("NotchesEnabled"), ui.chkNotches->isChecked());

	// box
	group.writeEntry(QStringLiteral("WidthFactor"), ui.sbWidthFactor->value() / 100.0);
	backgroundWidget->saveConfig(group);
	borderLineWidget->saveConfig(group);
	medianLineWidget->saveConfig(group);

	// symbols
	// TODO: save symbol properties for outliers, etc.?
	group.writeEntry(QStringLiteral("JitteringEnabled"), ui.chkJitteringEnabled->isChecked());

	// whiskers
	group.writeEntry(QStringLiteral("WhiskersType"), ui.cbWhiskersType->currentIndex());
	group.writeEntry(QStringLiteral("WhiskersRangeParameter"), QLocale().toDouble(ui.leWhiskersRangeParameter->text()));
	whiskersLineWidget->saveConfig(group);

	// whiskers cap
	group.writeEntry(QStringLiteral("WhiskersCapSize"), Worksheet::convertToSceneUnits(ui.sbWhiskersCapSize->value(), Worksheet::Unit::Point));
	whiskersCapLineWidget->saveConfig(group);

	config.sync();
}
