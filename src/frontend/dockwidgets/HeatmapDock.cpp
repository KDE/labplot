/*
	File                 : HeatmapDock.cpp
	Project              : LabPlot
	Description          : Dock widget for the heatmap plot
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2023 Martin Marmsoler <martin.marmsoler@gmail.com>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "HeatmapDock.h"
#include "backend/core/AbstractColumn.h"
#include "backend/core/AspectTreeModel.h"
#include "backend/core/Project.h"
#include "backend/lib/macros.h"
#include "backend/matrix/Matrix.h"
#include "frontend/widgets/TreeViewComboBox.h"
#include "frontend/GuiTools.h"
#include "frontend/TemplateHandler.h"
#include "frontend/widgets/LineWidget.h"
#include "frontend/widgets/ValueWidget.h"
#include "frontend/colormaps/ColorMapsDialog.h"
#include "tools/ColorMapsManager.h"

#include <QPushButton>

#include <KConfig>
#include <KLocalizedString>

HeatmapDock::HeatmapDock(QWidget* parent)
	: BaseDock(parent)
	, cbXColumn(new TreeViewComboBox)
	, cbYColumn(new TreeViewComboBox)
	, cbMatrix(new TreeViewComboBox) {
	ui.setupUi(this);
	setPlotRangeCombobox(ui.cbPlotRanges);
	setBaseWidgets(ui.leName, ui.teComment);

	QSizePolicy sizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
	cbXColumn->setSizePolicy(sizePolicy);
	ui.hBoxXColumn->insertWidget(0, cbXColumn);

	cbXColumn->setSizePolicy(sizePolicy);
	ui.hBoxYColumn->insertWidget(0, cbYColumn);

	cbXColumn->setSizePolicy(sizePolicy);
	ui.hBoxMatrix->insertWidget(0, cbMatrix);

	ui.cbDataSource->addItem(i18n("Matrix"), (int)Heatmap::DataSource::Matrix);
	ui.cbDataSource->addItem(i18n("Spreadsheet"), (int)Heatmap::DataSource::Spreadsheet);
	dataSourceChanged();

	ui.bColorMap->setIcon(QIcon::fromTheme(QLatin1String("color-management")));
	ui.lColorMapPreview->setMaximumHeight(ui.bColorMap->height());

	updateLocale();
	retranslateUi();

	// SLOTS
	// Tab "General"
	connect(ui.leName, &QLineEdit::textChanged, this, &HeatmapDock::nameChanged);
	connect(ui.teComment, &QTextEdit::textChanged, this, &HeatmapDock::commentChanged);
	connect(ui.cbDataSource, &QComboBox::currentIndexChanged, this, &HeatmapDock::dataSourceChanged);
	connect(cbXColumn, &TreeViewComboBox::currentModelIndexChanged, this, &HeatmapDock::xColumnChanged);
	connect(cbYColumn, &TreeViewComboBox::currentModelIndexChanged, this, &HeatmapDock::yColumnChanged);
	connect(cbMatrix, &TreeViewComboBox::currentModelIndexChanged, this, &HeatmapDock::matrixChanged);

	connect(ui.cbAutomaticLimits, &QCheckBox::clicked, this, &HeatmapDock::automaticLimitsChanged);
	connect(ui.sbLimitsMin, QOverload<double>::of(&NumberSpinBox::valueChanged), this, &HeatmapDock::limitsMinChanged);
	connect(ui.sbLimitsMax, QOverload<double>::of(&NumberSpinBox::valueChanged), this, &HeatmapDock::limitsMaxChanged);
	connect(ui.cbSourceNumberBins, &QCheckBox::clicked, this, &HeatmapDock::sourceNumberBinsChanged);
	connect(ui.cbEqualNumberBins, &QCheckBox::clicked, this, &HeatmapDock::equalNumberBinsChanged);
	connect(ui.sbxNumberBins, &QSpinBox::valueChanged, this, &HeatmapDock::xNumBinsChanged);
	connect(ui.sbyNumberBins, &QSpinBox::valueChanged, this, &HeatmapDock::yNumBinsChanged);

	connect(ui.bColorMap, &QPushButton::clicked, this, &HeatmapDock::selectColorMap);
}

void HeatmapDock::checkBinSettings() {
	const auto sourceNumberBins = ui.cbSourceNumberBins->isChecked();

	ui.cbEqualNumberBins->setEnabled(!sourceNumberBins);
	ui.sbxNumberBins->setEnabled(!sourceNumberBins);
	ui.sbyNumberBins->setEnabled(!sourceNumberBins && !ui.cbEqualNumberBins->isChecked());
}

void HeatmapDock::retranslateUi() {
	CONDITIONAL_LOCK_RETURN;

	QString msg = i18n("Use the row/column count as number bins");
	ui.lSourceNumberBins->setToolTip(msg);
	ui.cbSourceNumberBins->setToolTip(msg);
	msg = i18n("Use equal number bins for x and y");
	ui.lEqualNumberBins->setToolTip(msg);
	ui.cbEqualNumberBins->setToolTip(msg);
}

void HeatmapDock::setPlots(QList<Heatmap*> list) {
	CONDITIONAL_LOCK_RETURN;
	if (m_plot)
		disconnect(m_plot, nullptr, this, nullptr);

	m_plots = list;
	m_plot = list.first();
	setAspects(list);
	Q_ASSERT(m_plot);
	m_aspectTreeModelColumn = new AspectTreeModel(m_plot->project());
	m_aspectTreeModelMatrix = new AspectTreeModel(m_plot->project());
	setModel();

	// if there is more than one point in the list, disable the comment and name widgets in "general"
	if (list.size() == 1) {
		ui.lName->setEnabled(true);
		ui.leName->setEnabled(true);
		ui.lComment->setEnabled(true);
		ui.teComment->setEnabled(true);
		ui.leName->setText(m_plot->name());
		ui.teComment->setText(m_plot->comment());
	} else {
		ui.lName->setEnabled(false);
		ui.leName->setEnabled(false);
		ui.lComment->setEnabled(false);
		ui.teComment->setEnabled(false);
		ui.leName->setText(QString());
		ui.teComment->setText(QString());
	}
	ui.leName->setStyleSheet(QString());
	ui.leName->setToolTip(QString());

	// show the properties of the first box plot
	ui.chkVisible->setChecked(m_plot->isVisible());
	ui.cbDataSource->setCurrentIndex(m_plot->dataSource() == Heatmap::DataSource::Matrix ? 0 : 1);
	cbXColumn->setAspect(m_plot->xColumn(), m_plot->xColumnPath());
	cbYColumn->setAspect(m_plot->yColumn(), m_plot->yColumnPath());
	cbMatrix->setAspect(m_plot->matrix(), m_plot->matrixPath());

	dataSourceWidgetAppearance(m_plot->dataSource());
	ui.cbEqualNumberBins->setChecked(m_plot->equalNumberBins());
	ui.cbSourceNumberBins->setChecked(m_plot->sourceNumberBins());
	ui.sbxNumberBins->setValue(m_plot->xNumberBins());
	ui.sbyNumberBins->setValue(m_plot->yNumberBins());

	ui.cbAutomaticLimits->setChecked(m_plot->automaticLimits());
	ui.sbLimitsMin->setEnabled(!m_plot->automaticLimits());
	ui.sbLimitsMax->setEnabled(!m_plot->automaticLimits());
	ui.sbLimitsMin->setValue(m_plot->formatMin());
	ui.sbLimitsMax->setValue(m_plot->formatMax());

	checkBinSettings();

	QPixmap pixmap;
	ColorMapsManager::render(pixmap, m_plot->format().colors, 80, 200);
	ui.lColorMapPreview->setPixmap(pixmap);

	updatePlotRangeList();
	updateLocale();
	retranslateUi();

	// SIGNALs/SLOTs
	connect(m_plot, &Heatmap::xColumnChanged, this, &HeatmapDock::plotXColumnChanged);
	connect(m_plot, &Heatmap::yColumnChanged, this, &HeatmapDock::plotYColumnChanged);
	connect(m_plot, &Heatmap::matrixChanged, this, &HeatmapDock::plotMatrixChanged);
	connect(m_plot, &Heatmap::equalNumberBinsChanged, this, &HeatmapDock::plotEqualNumberBinsChanged);
	connect(m_plot, &Heatmap::sourceNumberBinsChanged, this, &HeatmapDock::plotSourceNumberBinsChanged);
	connect(m_plot, &Heatmap::xNumberBinsChanged, this, &HeatmapDock::plotXNumBinsChanged);
	connect(m_plot, &Heatmap::yNumberBinsChanged, this, &HeatmapDock::plotYNumBinsChanged);
	connect(m_plot, &Heatmap::automaticLimitsChanged, this, &HeatmapDock::plotAutomaticLimitsChanged);
}

void HeatmapDock::updateLocale() {
	CONDITIONAL_LOCK_RETURN;

	BaseDock::updateLocale();

	ui.sbLimitsMin->setLocale(QLocale());
	ui.sbLimitsMax->setLocale(QLocale());
	ui.sbLimitsMin->setLocale(QLocale());
	ui.sbLimitsMax->setLocale(QLocale());
}

void HeatmapDock::setModel() {
	m_aspectTreeModelColumn->enablePlottableColumnsOnly(true);
	m_aspectTreeModelColumn->enableShowPlotDesignation(true);

	QList<AspectType> list{AspectType::Column};
	m_aspectTreeModelColumn->setSelectableAspects(list);
	cbXColumn->setTopLevelClasses(TreeViewComboBox::plotColumnTopLevelClasses());
	cbXColumn->setModel(m_aspectTreeModelColumn);

	cbYColumn->setTopLevelClasses(TreeViewComboBox::plotColumnTopLevelClasses());
	cbYColumn->setModel(m_aspectTreeModelColumn);

	list = {AspectType::Matrix};
	m_aspectTreeModelMatrix->setSelectableAspects(list);
	list = {AspectType::Folder,
			AspectType::Workbook,
			AspectType::Datapicker,
			AspectType::Matrix,
			AspectType::Notebook};

	cbMatrix->setTopLevelClasses(list);
	cbMatrix->setModel(m_aspectTreeModelMatrix);
}

void HeatmapDock::selectColorMap() {
	CONDITIONAL_LOCK_RETURN;

	auto* dlg = new ColorMapsDialog(this);
	if (dlg->exec() == QDialog::Accepted) {
		const auto& name = dlg->name();
		ui.lColorMapPreview->setPixmap(ColorMapsManager::instance()->previewPixmap(name));
		ui.lColorMapPreview->setFocus();

		for (auto* plot : m_plots) {
			auto f = plot->format();
			f.name = name;
			f.colors = ColorMapsManager::instance()->colors(name);
			plot->setFormat(f);
		}
	}
}

////**********************************************************
////******* SLOTs for changes triggered in HeatmapDock *******
////**********************************************************
////"General"-tab


void HeatmapDock::dataSourceWidgetAppearance(Heatmap::DataSource datasource) {
	bool matrix = datasource == Heatmap::DataSource::Matrix;

	ui.lMatrix->setVisible(matrix);
	cbMatrix->setVisible(matrix);

	ui.lSourceNumberBins->setVisible(matrix);
	ui.cbSourceNumberBins->setVisible(matrix);

	ui.lXColumn->setVisible(!matrix);
	cbXColumn->setVisible(!matrix);
	ui.lYColumn->setVisible(!matrix);
	cbYColumn->setVisible(!matrix);
}

void HeatmapDock::dataSourceChanged() {
	CONDITIONAL_LOCK_RETURN;

	const auto datasource = Heatmap::DataSource(ui.cbDataSource->currentData().toInt());
	dataSourceWidgetAppearance(datasource);

	for (auto* plot : m_plots)
		plot->setDataSource(datasource);
}

void HeatmapDock::xColumnChanged(const QModelIndex& index) {
	CONDITIONAL_LOCK_RETURN;

	auto* aspect = static_cast<AbstractAspect*>(index.internalPointer());
	AbstractColumn* column = nullptr;
	if (aspect) {
		column = dynamic_cast<AbstractColumn*>(aspect);
		Q_ASSERT(column);
	}

	for (auto* plot : m_plots)
		plot->setXColumn(column);
}

void HeatmapDock::yColumnChanged(const QModelIndex& index) {
	CONDITIONAL_LOCK_RETURN;

	auto* aspect = static_cast<AbstractAspect*>(index.internalPointer());
	AbstractColumn* column = nullptr;
	if (aspect) {
		column = dynamic_cast<AbstractColumn*>(aspect);
		Q_ASSERT(column);
	}

	for (auto* plot : m_plots)
		plot->setYColumn(column);
}

void HeatmapDock::matrixChanged(const QModelIndex& index) {
	CONDITIONAL_LOCK_RETURN;

	auto* aspect = static_cast<AbstractAspect*>(index.internalPointer());
	Matrix* matrix = nullptr;
	if (aspect) {
		matrix = dynamic_cast<Matrix*>(aspect);
		Q_ASSERT(matrix);
	}

	for (auto* plot : m_plots)
		plot->setMatrix(matrix);
}

void HeatmapDock::automaticLimitsChanged(bool automatic) {
	CONDITIONAL_LOCK_RETURN;

	ui.sbLimitsMin->setEnabled(!automatic);
	ui.sbLimitsMax->setEnabled(!automatic);

	for (auto* plot : m_plots)
		plot->setAutomaticLimits(automatic);
}

void HeatmapDock::limitsMinChanged(double value) {
	CONDITIONAL_RETURN_NO_LOCK;

	for (auto* plot : m_plots) {
		auto format = plot->format();
		format.min = value;
		plot->setFormat(format);
	}
}

void HeatmapDock::limitsMaxChanged(double value) {
	CONDITIONAL_RETURN_NO_LOCK;

	for (auto* plot : m_plots) {
		auto format = plot->format();
		format.max = value;
		plot->setFormat(format);
	}
}

void HeatmapDock::equalNumberBinsChanged(bool equal) {
	CONDITIONAL_LOCK_RETURN;
	for (auto* plot : m_plots)
		plot->setEqualNumberBins(equal);
	checkBinSettings();
}

void HeatmapDock::sourceNumberBinsChanged(bool useMatrixBins) {
	CONDITIONAL_LOCK_RETURN;
	for (auto* plot : m_plots)
		plot->setSourceNumberBins(useMatrixBins);
	checkBinSettings();
}

void HeatmapDock::xNumBinsChanged(int v) {
	CONDITIONAL_LOCK_RETURN;
	for (auto* plot : m_plots)
		plot->setXNumberBins(v);
}

void HeatmapDock::yNumBinsChanged(int v) {
	CONDITIONAL_LOCK_RETURN;
	for (auto* plot : m_plots)
		plot->setYNumberBins(v);
}

void HeatmapDock::visibilityChanged(bool visible) {
	CONDITIONAL_LOCK_RETURN;

	for (auto* plot : m_plots)
		plot->setVisible(visible);
}

////*************************************************************
////******* SLOTs for changes triggered in Heatmap ********
////*************************************************************
//// general

void HeatmapDock::plotEqualNumberBinsChanged(bool equal) {
	CONDITIONAL_LOCK_RETURN;
	ui.cbEqualNumberBins->setChecked(equal);
}

void HeatmapDock::plotSourceNumberBinsChanged(bool useMatrixBins) {
	CONDITIONAL_LOCK_RETURN;
	ui.cbSourceNumberBins->setChecked(useMatrixBins);
}

void HeatmapDock::plotXColumnChanged(const AbstractColumn* column) {
	CONDITIONAL_LOCK_RETURN;
	cbXColumn->setAspect(column, m_plot->xColumnPath());
}

void HeatmapDock::plotYColumnChanged(const AbstractColumn* column) {
	CONDITIONAL_LOCK_RETURN;
	cbYColumn->setAspect(column, m_plot->yColumnPath());
}

void HeatmapDock::plotMatrixChanged(const Matrix* matrix) {
	CONDITIONAL_LOCK_RETURN;
	cbMatrix->setAspect(matrix, m_plot->matrixPath());
}

void HeatmapDock::plotXNumBinsChanged(unsigned int v) {
	CONDITIONAL_LOCK_RETURN;
	ui.sbxNumberBins->setValue(v);
}

void HeatmapDock::plotYNumBinsChanged(unsigned int v) {
	CONDITIONAL_LOCK_RETURN;
	ui.sbyNumberBins->setValue(v);
}

void HeatmapDock::plotVisibilityChanged(bool visible) {
	CONDITIONAL_LOCK_RETURN;
	ui.chkVisible->setChecked(visible);
}

void HeatmapDock::plotAutomaticLimitsChanged(bool automatic) {
	CONDITIONAL_LOCK_RETURN;
	ui.cbAutomaticLimits->setChecked(automatic);
}

void HeatmapDock::plotLimitsMinChanged(double value) {
	CONDITIONAL_LOCK_RETURN;
	ui.sbLimitsMin->setValue(value);
}

void HeatmapDock::plotLimitsMaxChanged(double value) {
	CONDITIONAL_LOCK_RETURN;
	ui.sbLimitsMax->setValue(value);
}

////**********************************************************
////******************** SETTINGS ****************************
////**********************************************************
namespace {
const QLatin1String configName("Heatmap");
const QLatin1String configSourceNumberBins("SourceNumberBins");
const QLatin1String configEqualNumberBins("EqualNumberBins");
const QLatin1String configXNumberBins("XNumberBins");
const QLatin1String configYNumberBins("YNumberBins");
const QLatin1String configAutomaticLimits("AutomaticLimits");
const QLatin1String configLimitMin("LimitMin");
const QLatin1String configLimitMax("LimitMax");
}

void HeatmapDock::loadConfig(KConfig& config) {
	KConfigGroup group = config.group(configName);


	ui.cbSourceNumberBins->setChecked(group.readEntry(configSourceNumberBins, (int)m_plot->sourceNumberBins()));
	ui.cbEqualNumberBins->setChecked(group.readEntry(configEqualNumberBins, (int)m_plot->equalNumberBins()));
	ui.sbxNumberBins->setValue(group.readEntry(configXNumberBins, (int)m_plot->xNumberBins()));
	ui.sbyNumberBins->setValue(group.readEntry(configYNumberBins, (int)m_plot->yNumberBins()));
	ui.cbAutomaticLimits->setChecked(group.readEntry(configAutomaticLimits, (int)m_plot->automaticLimits()));
	ui.sbLimitsMin->setValue(group.readEntry(configLimitMin, (int)m_plot->format().min));
	ui.sbLimitsMax->setValue(group.readEntry(configLimitMax, (int)m_plot->format().max));

	checkBinSettings();
}

void HeatmapDock::loadConfigFromTemplate(KConfig& config) {
	// extract the name of the template from the file name
	QString name;
	int index = config.name().lastIndexOf(QLatin1String("/"));
	if (index != -1)
		name = config.name().right(config.name().size() - index - 1);
	else
		name = config.name();

	int size = m_plots.size();
	if (size > 1)
		m_plot->beginMacro(i18n("%1 heatmap plots: template \"%2\" loaded", size, name));
	else
		m_plot->beginMacro(i18n("%1: template \"%2\" loaded", m_plot->name(), name));

	this->loadConfig(config);

	m_plot->endMacro();
}

void HeatmapDock::saveConfigAsTemplate(KConfig& config) {
	KConfigGroup group = config.group(configName);

	group.writeEntry(configSourceNumberBins, (int)m_plot->sourceNumberBins());
	group.writeEntry(configEqualNumberBins, (int)m_plot->equalNumberBins());
	group.writeEntry(configXNumberBins, (int)m_plot->xNumberBins());
	group.writeEntry(configYNumberBins, (int)m_plot->yNumberBins());
	group.writeEntry(configAutomaticLimits, (int)m_plot->automaticLimits());
	group.writeEntry(configLimitMin, (int)m_plot->format().min);
	group.writeEntry(configLimitMax, (int)m_plot->format().max);

	config.sync();
}
