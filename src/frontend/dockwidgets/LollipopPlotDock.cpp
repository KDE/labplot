/*
	File                 : LollipopPlotDock.cpp
	Project              : LabPlot
	Description          : Dock widget for the lolliplot plot
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2023-2025 Alexander Semke <alexander.semke@web.de>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "LollipopPlotDock.h"
#include "backend/core/AbstractColumn.h"
#include "backend/lib/macros.h"
#include "frontend/widgets/TreeViewComboBox.h"
#include "frontend/TemplateHandler.h"
#include "frontend/widgets/DataColumnsWidget.h"
#include "frontend/widgets/LineWidget.h"
#include "frontend/widgets/SymbolWidget.h"
#include "frontend/widgets/ValueWidget.h"

#include <KConfig>

LollipopPlotDock::LollipopPlotDock(QWidget* parent)
	: BaseDock(parent) {
	ui.setupUi(this);
	setPlotRangeCombobox(ui.cbPlotRanges);
	setBaseWidgets(ui.leName, ui.teComment);
	setVisibilityWidgets(ui.chkVisible, ui.chkLegendVisible);

	// Tab "General"
	// x-data
	cbXColumn = new TreeViewComboBox(ui.tabGeneral);
	QSizePolicy sizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
	cbXColumn->setSizePolicy(sizePolicy);
	static_cast<QVBoxLayout*>(ui.frameXColumn->layout())->insertWidget(0, cbXColumn);
	ui.bRemoveXColumn->setIcon(QIcon::fromTheme(QStringLiteral("edit-clear")));

	// y-data
	auto* gridLayout = static_cast<QGridLayout*>(ui.tabGeneral->layout());
	m_dataColumnsWidget = new DataColumnsWidget(this);
	gridLayout->addWidget(m_dataColumnsWidget, 5, 2, 1, 1);

	// Tab "Line"
	lineWidget = new LineWidget(ui.tabLine);
	gridLayout = qobject_cast<QGridLayout*>(ui.tabLine->layout());
	gridLayout->addWidget(lineWidget, 2, 0, 1, 3);

	// Tab "Symbol"
	symbolWidget = new SymbolWidget(ui.tabSymbol);
	gridLayout = qobject_cast<QGridLayout*>(ui.tabSymbol->layout());
	gridLayout->addWidget(symbolWidget, 2, 0, 1, 3);

	// Tab "Values"
	auto* hboxLayout = new QHBoxLayout(ui.tabValues);
	valueWidget = new ValueWidget(ui.tabValues);
	hboxLayout->addWidget(valueWidget);
	hboxLayout->setContentsMargins(2, 2, 2, 2);
	hboxLayout->setSpacing(2);

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

	// SLOTS
	// Tab "General"
	connect(cbXColumn, &TreeViewComboBox::currentModelIndexChanged, this, &LollipopPlotDock::xColumnChanged);
	connect(ui.bRemoveXColumn, &QPushButton::clicked, this, &LollipopPlotDock::removeXColumn);
	connect(m_dataColumnsWidget, &DataColumnsWidget::dataColumnsChanged, this, &LollipopPlotDock::dataColumnsChanged);
	connect(ui.cbOrientation, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &LollipopPlotDock::orientationChanged);

	// Tab "Line"
	connect(ui.cbNumberLine, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &LollipopPlotDock::currentBarLineChanged);

	// Tab "Symbol"
	connect(ui.cbNumberSymbol, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &LollipopPlotDock::currentBarSymbolChanged);

	// template handler
	auto* frame = new QFrame(this);
	auto* layout = new QHBoxLayout(frame);
	layout->setContentsMargins(0, 0, 0, 0);

	auto* templateHandler = new TemplateHandler(this, QLatin1String("LollipopPlot"));
	layout->addWidget(templateHandler);
	connect(templateHandler, &TemplateHandler::loadConfigRequested, this, &LollipopPlotDock::loadConfigFromTemplate);
	connect(templateHandler, &TemplateHandler::saveConfigRequested, this, &LollipopPlotDock::saveConfigAsTemplate);
	connect(templateHandler, &TemplateHandler::info, this, &LollipopPlotDock::info);

	ui.verticalLayout->addWidget(frame);
}

void LollipopPlotDock::setPlots(QList<LollipopPlot*> list) {
	CONDITIONAL_LOCK_RETURN;
	m_plots = list;
	m_plot = list.first();
	setAspects(list);
	Q_ASSERT(m_plot);
	setModel();

	// backgrounds
	QList<Line*> lines;
	QList<Symbol*> symbols;
	QList<Value*> values;
	for (auto* plot : m_plots) {
		lines << plot->lineAt(0);
		symbols << plot->symbolAt(0);
		values << plot->value();
	}

	lineWidget->setLines(lines);
	symbolWidget->setSymbols(symbols);
	valueWidget->setValues(values);

	// show the properties of the first box plot
	ui.chkLegendVisible->setChecked(m_plot->legendVisible());
	ui.chkVisible->setChecked(m_plot->isVisible());
	load();
	cbXColumn->setAspect(m_plot->xColumn(), m_plot->xColumnPath());
	loadDataColumns();

	updatePlotRangeList();

	// SIGNALs/SLOTs
	// general
	connect(m_plot, &LollipopPlot::xColumnChanged, this, &LollipopPlotDock::plotXColumnChanged);
	connect(m_plot, &LollipopPlot::dataColumnsChanged, this, &LollipopPlotDock::plotDataColumnsChanged);
}

void LollipopPlotDock::setModel() {
	auto* model = aspectModel();
	model->enablePlottableColumnsOnly(true);
	model->enableShowPlotDesignation(true);
	model->setSelectableAspects({AspectType::Column});
	cbXColumn->setTopLevelClasses(TreeViewComboBox::plotColumnTopLevelClasses());
	cbXColumn->setModel(model);
}

/*
 * updates the locale in the widgets. called when the application settings are changed.
 */
void LollipopPlotDock::updateLocale() {
	lineWidget->updateLocale();
}

void LollipopPlotDock::retranslateUi() {
	CONDITIONAL_LOCK_RETURN;

	ui.cbOrientation->clear();
	ui.cbOrientation->addItem(i18n("Horizontal"));
	ui.cbOrientation->addItem(i18n("Vertical"));

	QString msg = i18n("Select the data column for which the properties should be shown and edited");
	ui.lNumberLine->setToolTip(msg);
	ui.cbNumberLine->setToolTip(msg);
	ui.lNumberSymbol->setToolTip(msg);
	ui.cbNumberSymbol->setToolTip(msg);
}

//**********************************************************
//******* SLOTs for changes triggered in LollipopPlotDock *******
//**********************************************************
//"General"-tab
void LollipopPlotDock::xColumnChanged(const QModelIndex& index) {
	auto aspect = static_cast<AbstractAspect*>(index.internalPointer());
	AbstractColumn* column(nullptr);
	if (aspect) {
		column = dynamic_cast<AbstractColumn*>(aspect);
		Q_ASSERT(column);
	}

	ui.bRemoveXColumn->setEnabled(column != nullptr);

	CONDITIONAL_LOCK_RETURN;

	for (auto* plot : m_plots)
		plot->setXColumn(column);
}

void LollipopPlotDock::removeXColumn() {
	cbXColumn->setAspect(nullptr);
	ui.bRemoveXColumn->setEnabled(false);
	for (auto* plot : m_plots)
		plot->setXColumn(nullptr);
}

void LollipopPlotDock::dataColumnsChanged(QVector<const AbstractColumn*> columns) {
	int newCount = columns.count();
	int oldCount = m_plot->dataColumns().count();

	if (newCount > oldCount) {
		ui.cbNumberLine->addItem(QString::number(newCount));
		ui.cbNumberSymbol->addItem(QString::number(newCount));
	} else {
		if (newCount != 0) {
			ui.cbNumberLine->removeItem(ui.cbNumberLine->count() - 1);
			ui.cbNumberSymbol->removeItem(ui.cbNumberSymbol->count() - 1);
		}
	}

	CONDITIONAL_LOCK_RETURN;
	m_plot->setDataColumns(columns);
}

void LollipopPlotDock::orientationChanged(int index) {
	CONDITIONAL_LOCK_RETURN;

	auto orientation = LollipopPlot::Orientation(index);
	for (auto* plot : m_plots)
		plot->setOrientation(orientation);
}

//"Line"-tab
/*!
 * called when the current bar number was changed, shows the line properties for the selected bar.
 */
void LollipopPlotDock::currentBarLineChanged(int index) {
	if (index == -1)
		return;

	CONDITIONAL_LOCK_RETURN;

	QList<Line*> lines;
	for (auto* plot : m_plots) {
		auto* line = plot->lineAt(index);
		if (line)
			lines << line;
	}

	lineWidget->setLines(lines);
}

//"Symbol"-tab
/*!
 * called when the current bar number was changed, shows the symbol properties for the selected bar.
 */
void LollipopPlotDock::currentBarSymbolChanged(int index) {
	if (index == -1)
		return;

	CONDITIONAL_LOCK_RETURN;

	QList<Symbol*> symbols;
	for (auto* plot : m_plots) {
		auto* symbol = plot->symbolAt(index);
		if (symbol)
			symbols << symbol;
	}

	symbolWidget->setSymbols(symbols);
}

//*************************************************************
//******* SLOTs for changes triggered in Lollipop ********
//*************************************************************
// general
void LollipopPlotDock::plotXColumnChanged(const AbstractColumn* column) {
	CONDITIONAL_LOCK_RETURN;
	cbXColumn->setAspect(column, m_plot->xColumnPath());
}
void LollipopPlotDock::plotDataColumnsChanged(const QVector<const AbstractColumn*>&) {
	CONDITIONAL_LOCK_RETURN;
	loadDataColumns();
}
void LollipopPlotDock::plotOrientationChanged(LollipopPlot::Orientation orientation) {
	CONDITIONAL_LOCK_RETURN;
	ui.cbOrientation->setCurrentIndex((int)orientation);
}

//**********************************************************
//******************** SETTINGS ****************************
//**********************************************************
void LollipopPlotDock::load() {
	// general
	ui.cbOrientation->setCurrentIndex((int)m_plot->orientation());
	loadDataColumns();
}

void LollipopPlotDock::loadDataColumns() {
	// show columns names in the combobox for the selection of the bar to be modified
	ui.cbNumberLine->clear();
	ui.cbNumberSymbol->clear();
	for (int i = 0; i < m_plot->dataColumns().count(); ++i) {
		if (m_plot->dataColumns().at(i)) {
			ui.cbNumberLine->addItem(m_plot->dataColumns().at(i)->name());
			ui.cbNumberSymbol->addItem(m_plot->dataColumns().at(i)->name());
		}
	}

	// select the first column after all of them were added to the combobox
	ui.cbNumberLine->setCurrentIndex(0);
	ui.cbNumberSymbol->setCurrentIndex(0);

	// show the data columns
	m_dataColumnsWidget->setDataColumns(m_plot->dataColumns(), m_plot->dataColumnPaths(), aspectModel());

	// disable data column widgets if we're modifying more than one plot at the same time
	bool enabled = (m_plots.count() == 1);
	m_dataColumnsWidget->setEnabled(enabled);
}

void LollipopPlotDock::loadConfig(KConfig& config) {
	KConfigGroup group = config.group(QStringLiteral("Lollipop"));

	// general
	ui.cbOrientation->setCurrentIndex(group.readEntry(QStringLiteral("Orientation"), (int)m_plot->orientation()));

	lineWidget->loadConfig(group);
	symbolWidget->loadConfig(group);
	valueWidget->loadConfig(group);
}

void LollipopPlotDock::loadConfigFromTemplate(KConfig& config) {
	auto name = TemplateHandler::templateName(config);
	int size = m_plots.size();
	if (size > 1)
		m_plot->beginMacro(i18n("%1 lollipop plots: template \"%2\" loaded", size, name));
	else
		m_plot->beginMacro(i18n("%1: template \"%2\" loaded", m_plot->name(), name));

	this->loadConfig(config);

	m_plot->endMacro();
}

void LollipopPlotDock::saveConfigAsTemplate(KConfig& config) {
	KConfigGroup group = config.group(QStringLiteral("Lollipop"));

	// general
	group.writeEntry(QStringLiteral("Orientation"), ui.cbOrientation->currentIndex());

	lineWidget->saveConfig(group);
	symbolWidget->saveConfig(group);
	valueWidget->saveConfig(group);

	config.sync();
}
