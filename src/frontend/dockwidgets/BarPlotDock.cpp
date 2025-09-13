/*
	File                 : BarPlotDock.cpp
	Project              : LabPlot
	Description          : Dock widget for the bar plot
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2022-2025 Alexander Semke <alexander.semke@web.de>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "BarPlotDock.h"
#include "backend/core/AbstractColumn.h"
#include "backend/core/Settings.h"
#include "backend/lib/macros.h"
#include "frontend/widgets/TreeViewComboBox.h"
#include "frontend/TemplateHandler.h"
#include "frontend/widgets/BackgroundWidget.h"
#include "frontend/widgets/ErrorBarWidget.h"
#include "frontend/widgets/LineWidget.h"
#include "frontend/widgets/ValueWidget.h"

BarPlotDock::BarPlotDock(QWidget* parent)
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

	// filling
	auto* gridLayout = static_cast<QGridLayout*>(ui.tabBars->layout());
	backgroundWidget = new BackgroundWidget(ui.tabBars);
	gridLayout->addWidget(backgroundWidget, 5, 0, 1, 3);

	// border lines
	lineWidget = new LineWidget(ui.tabBars);
	gridLayout->addWidget(lineWidget, 8, 0, 1, 3);

	// Tab "Values"
	auto* hboxLayout = new QHBoxLayout(ui.tabValues);
	valueWidget = new ValueWidget(ui.tabValues);
	hboxLayout->addWidget(valueWidget);
	hboxLayout->setContentsMargins(2, 2, 2, 2);
	hboxLayout->setSpacing(2);

	// Tab "Error Bars"
	const KConfigGroup group = Settings::group(QStringLiteral("Settings_General"));
	if (group.readEntry(QStringLiteral("GUMTerms"), false))
		ui.tabWidget->setTabText(ui.tabWidget->indexOf(ui.tabErrorBars), i18n("Uncertainty Bars"));

	errorBarWidget = new ErrorBarWidget(ui.tabErrorBars);
	gridLayout = qobject_cast<QGridLayout*>(ui.tabErrorBars->layout());
	gridLayout->addWidget(errorBarWidget, 2, 0, 1, 3);

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
	connect(cbXColumn, &TreeViewComboBox::currentModelIndexChanged, this, &BarPlotDock::xColumnChanged);
	connect(ui.bRemoveXColumn, &QPushButton::clicked, this, &BarPlotDock::removeXColumn);
	connect(m_buttonNew, &QPushButton::clicked, this, &BarPlotDock::addDataColumn);
	connect(ui.cbType, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &BarPlotDock::typeChanged);
	connect(ui.cbOrientation, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &BarPlotDock::orientationChanged);

	// Tab "Bars"
	connect(ui.cbNumber, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &BarPlotDock::barNumberChanged);
	connect(ui.sbWidthFactor, QOverload<int>::of(&QSpinBox::valueChanged), this, &BarPlotDock::widthFactorChanged);

	// Tab "Error Bars"
	connect(ui.cbErrorBarsNumber, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &BarPlotDock::errorNumberChanged);

	// template handler
	auto* frame = new QFrame(this);
	auto* layout = new QHBoxLayout(frame);
	layout->setContentsMargins(0, 0, 0, 0);

	auto* templateHandler = new TemplateHandler(this, QLatin1String("BarPlot"));
	layout->addWidget(templateHandler);
	connect(templateHandler, &TemplateHandler::loadConfigRequested, this, &BarPlotDock::loadConfigFromTemplate);
	connect(templateHandler, &TemplateHandler::saveConfigRequested, this, &BarPlotDock::saveConfigAsTemplate);
	connect(templateHandler, &TemplateHandler::info, this, &BarPlotDock::info);

	ui.verticalLayout->addWidget(frame);
}

void BarPlotDock::setBarPlots(QList<BarPlot*> list) {
	CONDITIONAL_LOCK_RETURN;
	m_barPlots = list;
	m_barPlot = list.first();
	setAspects(list);

	Q_ASSERT(m_barPlot);
	setModel();

	// initialize widgets for common properties
	QList<Background*> backgrounds;
	QList<Line*> lines;
	QList<Value*> values;
	QList<ErrorBar*> errorBars;
	for (auto* plot : m_barPlots) {
		backgrounds << plot->backgroundAt(0);
		lines << plot->lineAt(0);
		values << plot->value();
		errorBars << plot->errorBarAt(0);
	}

	backgroundWidget->setBackgrounds(backgrounds);
	lineWidget->setLines(lines);
	valueWidget->setValues(values);
	errorBarWidget->setErrorBars(errorBars);

	// show the properties of the first plot
	ui.chkLegendVisible->setChecked(m_barPlot->legendVisible());
	ui.chkVisible->setChecked(m_barPlot->isVisible());
	cbXColumn->setAspect(m_barPlot->xColumn(), m_barPlot->xColumnPath());
	loadDataColumns();

	// load the remaining properties
	load();
	updatePlotRangeList();

	// SIGNALs/SLOTs
	// general
	connect(m_barPlot, &BarPlot::typeChanged, this, &BarPlotDock::plotTypeChanged);
	connect(m_barPlot, &BarPlot::xColumnChanged, this, &BarPlotDock::plotXColumnChanged);
	connect(m_barPlot, &BarPlot::dataColumnsChanged, this, &BarPlotDock::plotDataColumnsChanged);

	connect(m_barPlot, &BarPlot::widthFactorChanged, this, &BarPlotDock::plotWidthFactorChanged);
}

void BarPlotDock::setModel() {
	auto* model = aspectModel();
	model->enablePlottableColumnsOnly(true);
	model->enableShowPlotDesignation(true);
	model->setSelectableAspects({AspectType::Column});
	cbXColumn->setTopLevelClasses(TreeViewComboBox::plotColumnTopLevelClasses());
	cbXColumn->setModel(model);
	errorBarWidget->setModel(model);
}

void BarPlotDock::loadDataColumns() {
	// add the combobox for the first column, is always present
	if (m_dataComboBoxes.count() == 0)
		addDataColumn();

	int count = m_barPlot->dataColumns().count();
	ui.cbNumber->clear();
	ui.cbErrorBarsNumber->clear();

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
			m_dataComboBoxes.at(i)->setModel(model); // the model might have changed in-between, reset the current model
			m_dataComboBoxes.at(i)->setAspect(m_barPlot->dataColumns().at(i), m_barPlot->dataColumnPaths().at(i));
		}

		// show columns names in the combobox for the selection of the bar to be modified
		for (int i = 0; i < count; ++i)
			if (m_barPlot->dataColumns().at(i)) {
				const auto& name = m_barPlot->dataColumns().at(i)->name();
				ui.cbNumber->addItem(name);
				ui.cbErrorBarsNumber->addItem(name);
			}
	} else {
		// no data columns set in the box plot yet, we show the first combo box only and reset its model
		m_dataComboBoxes.first()->setModel(model);
		m_dataComboBoxes.first()->setAspect(nullptr);
		for (int i = 0; i < m_dataComboBoxes.count(); ++i)
			removeDataColumn();
	}

	// disable data column widgets if we're modifying more than one box plot at the same time
	bool enabled = (m_barPlots.count() == 1);
	m_buttonNew->setVisible(enabled);
	for (auto* cb : m_dataComboBoxes)
		cb->setEnabled(enabled);
	for (auto* b : m_removeButtons)
		b->setVisible(enabled);

	// select the first column after all of them were added to the combobox
	ui.cbNumber->setCurrentIndex(0);
	ui.cbErrorBarsNumber->setCurrentIndex(0);
}

void BarPlotDock::setDataColumns() const {
	int newCount = m_dataComboBoxes.count();
	int oldCount = m_barPlot->dataColumns().count();

	if (newCount > oldCount) {
		ui.cbNumber->addItem(QString::number(newCount));
		ui.cbErrorBarsNumber->addItem(QString::number(newCount));
	} else {
		if (newCount != 0) {
			ui.cbNumber->removeItem(ui.cbNumber->count() - 1);
			ui.cbErrorBarsNumber->removeItem(ui.cbErrorBarsNumber->count() - 1);
		}
	}

	QVector<const AbstractColumn*> columns;

	for (auto* cb : m_dataComboBoxes) {
		auto* aspect = cb->currentAspect();
		if (aspect && aspect->type() == AspectType::Column)
			columns << static_cast<AbstractColumn*>(aspect);
	}

	m_barPlot->setDataColumns(columns);
}

/*
 * updates the locale in the widgets. called when the application settings are changed.
 */
void BarPlotDock::updateLocale() {
	lineWidget->updateLocale();
}

void BarPlotDock::retranslateUi() {
	CONDITIONAL_LOCK_RETURN;

	ui.cbType->clear();
	ui.cbType->addItem(i18n("Grouped"));
	ui.cbType->addItem(i18n("Stacked"));
	ui.cbType->addItem(i18n("Stacked 100%"));

	ui.cbOrientation->clear();
	ui.cbOrientation->addItem(i18n("Horizontal"));
	ui.cbOrientation->addItem(i18n("Vertical"));

	// tooltip texts
	QString msg = i18n("Select the data column for which the properties should be shown and edited");
	ui.lNumber->setToolTip(msg);
	ui.cbNumber->setToolTip(msg);
	ui.lErrorBarsNumber->setToolTip(msg);
	ui.cbErrorBarsNumber->setToolTip(msg);

	msg = i18n("Specify the factor in percent to control the width of the bar relative to its default value, applying to all bars");
	ui.lWidthFactor->setToolTip(msg);
	ui.sbWidthFactor->setToolTip(msg);
}

//**********************************************************
//******* SLOTs for changes triggered in BarPlotDock *******
//**********************************************************
//"General"-tab
void BarPlotDock::xColumnChanged(const QModelIndex& index) {
	auto aspect = static_cast<AbstractAspect*>(index.internalPointer());
	AbstractColumn* column(nullptr);
	if (aspect) {
		column = dynamic_cast<AbstractColumn*>(aspect);
		Q_ASSERT(column);
	}

	ui.bRemoveXColumn->setEnabled(column != nullptr);

	CONDITIONAL_LOCK_RETURN;

	for (auto* barPlot : m_barPlots)
		barPlot->setXColumn(column);
}

void BarPlotDock::removeXColumn() {
	cbXColumn->setAspect(nullptr);
	ui.bRemoveXColumn->setEnabled(false);
	for (auto* barPlot : m_barPlots)
		barPlot->setXColumn(nullptr);
}

void BarPlotDock::addDataColumn() {
	auto* cb = new TreeViewComboBox(this);
	cb->setTopLevelClasses(TreeViewComboBox::plotColumnTopLevelClasses());
	cb->setModel(aspectModel());
	connect(cb, &TreeViewComboBox::currentModelIndexChanged, this, &BarPlotDock::dataColumnChanged);

	const int index = m_dataComboBoxes.size();

	if (index == 0) {
		QSizePolicy sizePolicy1(QSizePolicy::Expanding, QSizePolicy::Preferred);
		sizePolicy1.setHorizontalStretch(0);
		sizePolicy1.setVerticalStretch(0);
		sizePolicy1.setHeightForWidth(cb->sizePolicy().hasHeightForWidth());
		cb->setSizePolicy(sizePolicy1);
	} else {
		auto* button = new QPushButton();
		button->setIcon(QIcon::fromTheme(QStringLiteral("list-remove")));
		connect(button, &QPushButton::clicked, this, &BarPlotDock::removeDataColumn);
		m_gridLayout->addWidget(button, index, 1, 1, 1);
		m_removeButtons << button;
	}

	m_gridLayout->addWidget(cb, index, 0, 1, 1);
	m_gridLayout->addWidget(m_buttonNew, index + 1, 1, 1, 1);

	m_dataComboBoxes << cb;
	ui.lDataColumn->setText(i18n("Columns:"));
}

void BarPlotDock::removeDataColumn() {
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
	if (!m_removeButtons.isEmpty())
		ui.lDataColumn->setText(i18n("Columns:"));
	else
		ui.lDataColumn->setText(i18n("Column:"));

	if (!m_initializing)
		setDataColumns();
}

void BarPlotDock::dataColumnChanged(const QModelIndex&) {
	CONDITIONAL_LOCK_RETURN;
	setDataColumns();
}

void BarPlotDock::typeChanged(int index) {
	CONDITIONAL_LOCK_RETURN;

	auto type = static_cast<BarPlot::Type>(index);
	for (auto* barPlot : m_barPlots)
		barPlot->setType(type);
}

void BarPlotDock::orientationChanged(int index) {
	CONDITIONAL_LOCK_RETURN;

	auto orientation = BarPlot::Orientation(index);
	for (auto* barPlot : m_barPlots)
		barPlot->setOrientation(orientation);
}

//"Bars"-tab
/*!
 * called when the current bar number was changed, shows the bar properties for the selected bar.
 */
void BarPlotDock::barNumberChanged(int index) {
	if (index == -1)
		return;

	CONDITIONAL_LOCK_RETURN;

	QList<Background*> backgrounds;
	QList<Line*> lines;
	for (auto* plot : m_barPlots) {
		auto* background = plot->backgroundAt(index);
		if (background)
			backgrounds << background;

		auto* line = plot->lineAt(index);
		if (line)
			lines << line;
	}

	backgroundWidget->setBackgrounds(backgrounds);
	lineWidget->setLines(lines);
}

void BarPlotDock::widthFactorChanged(int value) {
	CONDITIONAL_LOCK_RETURN;

	double factor = (double)value / 100.;
	for (auto* barPlot : m_barPlots)
		barPlot->setWidthFactor(factor);
}

//"Error Bars"-tab
/*!
 * called when the current error bars number was changed, shows the bar properties for the selected bar.
 */
void BarPlotDock::errorNumberChanged(int index) {
	if (index == -1)
		return;

	CONDITIONAL_LOCK_RETURN;

	QList<ErrorBar*> errorBars;
	for (auto* plot : m_barPlots) {
		auto* errorBar = plot->errorBarAt(index);
		if (errorBar)
			errorBars << errorBar;
	}

	errorBarWidget->setErrorBars(errorBars);
}

//*************************************************************
//******* SLOTs for changes triggered in BarPlot ********
//*************************************************************
// general
void BarPlotDock::plotXColumnChanged(const AbstractColumn* column) {
	CONDITIONAL_LOCK_RETURN;
	cbXColumn->setAspect(column, m_barPlot->xColumnPath());
}
void BarPlotDock::plotDataColumnsChanged(const QVector<const AbstractColumn*>&) {
	CONDITIONAL_LOCK_RETURN;
	loadDataColumns();
}
void BarPlotDock::plotTypeChanged(BarPlot::Type type) {
	CONDITIONAL_LOCK_RETURN;
	ui.cbType->setCurrentIndex((int)type);
}
void BarPlotDock::plotOrientationChanged(BarPlot::Orientation orientation) {
	CONDITIONAL_LOCK_RETURN;
	ui.cbOrientation->setCurrentIndex((int)orientation);
}

// box
void BarPlotDock::plotWidthFactorChanged(double factor) {
	CONDITIONAL_LOCK_RETURN;
	ui.sbWidthFactor->setValue(round(factor * 100.));
}

//**********************************************************
//******************** SETTINGS ****************************
//**********************************************************
void BarPlotDock::load() {
	// general
	ui.cbType->setCurrentIndex((int)m_barPlot->type());
	ui.cbOrientation->setCurrentIndex((int)m_barPlot->orientation());

	// box
	ui.sbWidthFactor->setValue(round(m_barPlot->widthFactor() * 100.));
}

void BarPlotDock::loadConfig(KConfig& config) {
	KConfigGroup group = config.group(QStringLiteral("BarPlot"));

	// general
	ui.cbType->setCurrentIndex(group.readEntry(QStringLiteral("Type"), (int)m_barPlot->type()));
	ui.cbOrientation->setCurrentIndex(group.readEntry(QStringLiteral("Orientation"), (int)m_barPlot->orientation()));

	// box
	ui.sbWidthFactor->setValue(round(group.readEntry(QStringLiteral("WidthFactor"), m_barPlot->widthFactor()) * 100.));
	backgroundWidget->loadConfig(group);
	lineWidget->loadConfig(group);

	// values
	valueWidget->loadConfig(group);
}

void BarPlotDock::loadConfigFromTemplate(KConfig& config) {
	auto name = TemplateHandler::templateName(config);
	int size = m_barPlots.size();
	if (size > 1)
		m_barPlot->beginMacro(i18n("%1 bar plots: template \"%2\" loaded", size, name));
	else
		m_barPlot->beginMacro(i18n("%1: template \"%2\" loaded", m_barPlot->name(), name));

	this->loadConfig(config);

	m_barPlot->endMacro();
}

void BarPlotDock::saveConfigAsTemplate(KConfig& config) {
	KConfigGroup group = config.group(QStringLiteral("BarPlot"));

	// general
	group.writeEntry(QStringLiteral("Type"), ui.cbType->currentIndex());
	group.writeEntry(QStringLiteral("Orientation"), ui.cbOrientation->currentIndex());

	// box
	group.writeEntry(QStringLiteral("WidthFactor"), ui.sbWidthFactor->value() / 100.0);
	backgroundWidget->saveConfig(group);
	lineWidget->saveConfig(group);

	// values
	valueWidget->saveConfig(group);

	config.sync();
}
