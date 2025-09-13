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
#include "frontend/GuiTools.h"
#include "frontend/TemplateHandler.h"
#include "frontend/widgets/LineWidget.h"
#include "frontend/widgets/SymbolWidget.h"
#include "frontend/widgets/ValueWidget.h"

#include <QPushButton>

#include <KConfig>
#include <KLocalizedString>

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
	m_buttonNew = new QPushButton();
	m_buttonNew->setIcon(QIcon::fromTheme(QStringLiteral("list-add")));

	m_gridLayout = new QGridLayout(ui.frameDataColumns);
	m_gridLayout->setContentsMargins(0, 0, 0, 0);
	m_gridLayout->setHorizontalSpacing(2);
	m_gridLayout->setVerticalSpacing(2);
	ui.frameDataColumns->setLayout(m_gridLayout);

	// Tab "Line"
	lineWidget = new LineWidget(ui.tabLine);
	auto* gridLayout = qobject_cast<QGridLayout*>(ui.tabLine->layout());
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
	connect(m_buttonNew, &QPushButton::clicked, this, &LollipopPlotDock::addDataColumn);
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

void LollipopPlotDock::loadDataColumns() {
	// add the combobox for the first column, is always present
	if (m_dataComboBoxes.count() == 0)
		addDataColumn();

	int count = m_plot->dataColumns().count();
	ui.cbNumberLine->clear();
	ui.cbNumberSymbol->clear();

	auto* model = aspectModel();
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
		for (int i = 0; i < count; ++i) {
			m_dataComboBoxes.at(i)->setModel(model); // the model might have changed in-between, re-set the current model
			m_dataComboBoxes.at(i)->setAspect(m_plot->dataColumns().at(i), m_plot->dataColumnPaths().at(i));
		}

		// show columns names in the combobox for the selection of the bar to be modified
		for (int i = 0; i < count; ++i)
			if (m_plot->dataColumns().at(i)) {
				ui.cbNumberLine->addItem(m_plot->dataColumns().at(i)->name());
				ui.cbNumberSymbol->addItem(m_plot->dataColumns().at(i)->name());
			}
	} else {
		// no data columns set in the box plot yet, we show the first combo box only and reset its model
		m_dataComboBoxes.first()->setModel(model);
		m_dataComboBoxes.first()->setAspect(nullptr);
		for (int i = 0; i < m_dataComboBoxes.count(); ++i)
			removeDataColumn();
	}

	// disable data column widgets if we're modifying more than one box plot at the same time
	bool enabled = (m_plots.count() == 1);
	m_buttonNew->setVisible(enabled);
	for (auto* cb : m_dataComboBoxes)
		cb->setEnabled(enabled);
	for (auto* b : m_removeButtons)
		b->setVisible(enabled);

	// select the first column after all of them were added to the combobox
	ui.cbNumberLine->setCurrentIndex(0);
	ui.cbNumberSymbol->setCurrentIndex(0);
}

void LollipopPlotDock::setDataColumns() const {
	int newCount = m_dataComboBoxes.count();
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

	QVector<const AbstractColumn*> columns;

	for (auto* cb : m_dataComboBoxes) {
		auto* aspect = cb->currentAspect();
		if (aspect && aspect->type() == AspectType::Column)
			columns << static_cast<AbstractColumn*>(aspect);
	}

	m_plot->setDataColumns(columns);
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

void LollipopPlotDock::addDataColumn() {
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
										AspectType::Notebook};
	cb->setTopLevelClasses(list);
	cb->setModel(aspectModel());
	connect(cb, &TreeViewComboBox::currentModelIndexChanged, this, &LollipopPlotDock::dataColumnChanged);

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
		connect(button, &QPushButton::clicked, this, &LollipopPlotDock::removeDataColumn);
		m_gridLayout->addWidget(button, index, 1, 1, 1);
		m_removeButtons << button;
	}

	m_gridLayout->addWidget(cb, index, 0, 1, 1);
	m_gridLayout->addWidget(m_buttonNew, index + 1, 1, 1, 1);

	m_dataComboBoxes << cb;
	ui.lDataColumn->setText(i18n("Columns:"));
}

void LollipopPlotDock::removeDataColumn() {
	auto* sender = static_cast<QPushButton*>(QObject::sender());
	if (sender) {
		// remove button was clicked, determine which one and
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

	// TODO
	if (!m_removeButtons.isEmpty()) {
		ui.lDataColumn->setText(i18n("Columns:"));
	} else {
		ui.lDataColumn->setText(i18n("Column:"));
	}

	if (!m_initializing)
		setDataColumns();
}

void LollipopPlotDock::dataColumnChanged(const QModelIndex&) {
	CONDITIONAL_LOCK_RETURN;

	setDataColumns();
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
