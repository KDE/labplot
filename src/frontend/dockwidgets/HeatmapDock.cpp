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
#include "frontend/widgets/SymbolWidget.h"
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

	// Tab "General"

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
	connect(ui.sbxNumberBins, &QSpinBox::valueChanged, this, &HeatmapDock::xNumBinsChanged);
	connect(ui.sbyNumberBins, &QSpinBox::valueChanged, this, &HeatmapDock::yNumBinsChanged);

	connect(ui.bColorMap, &QPushButton::clicked, this, &HeatmapDock::selectColorMap);
}

void HeatmapDock::retranslateUi() {
}

void HeatmapDock::setPlots(QList<Heatmap*> list) {
	CONDITIONAL_LOCK_RETURN;
	if (m_plot)
		disconnect(m_plot, nullptr, this, nullptr);

	m_plots = list;
	m_plot = list.first();
	setAspects(list);
	Q_ASSERT(m_plot);
	// TODO: if setPlots will be called multiple times, we are loosing the object created before
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
	// load();
	ui.cbDataSource->setCurrentIndex(m_plot->dataSource() == Heatmap::DataSource::Matrix ? 0 : 1);
	cbXColumn->setAspect(m_plot->xColumn(), m_plot->xColumnPath());
	cbYColumn->setAspect(m_plot->yColumn(), m_plot->yColumnPath());
	cbMatrix->setAspect(m_plot->matrix(), m_plot->matrixPath());
	// loadDataColumns();

	ui.sbxNumberBins->setValue(m_plot->xNumBins());
	ui.sbyNumberBins->setValue(m_plot->yNumBins());

	ui.cbAutomaticLimits->setChecked(m_plot->automaticLimits());
	ui.sbLimitsMin->setEnabled(!m_plot->automaticLimits());
	ui.sbLimitsMax->setEnabled(!m_plot->automaticLimits());
	ui.sbLimitsMin->setValue(m_plot->formatMin());
	ui.sbLimitsMax->setValue(m_plot->formatMax());

	QPixmap pixmap;
	ColorMapsManager::render(pixmap, m_plot->format().colors, 80, 200);
	ui.lColorMapPreview->setPixmap(pixmap);

	updatePlotRangeList();

	// set the current locale
	//	updateLocale();

	// SIGNALs/SLOTs
	// general

	connect(m_plot, &Heatmap::xColumnChanged, this, &HeatmapDock::plotXColumnChanged);
	connect(m_plot, &Heatmap::yColumnChanged, this, &HeatmapDock::plotYColumnChanged);
	connect(m_plot, &Heatmap::matrixChanged, this, &HeatmapDock::plotMatrixChanged);
	connect(m_plot, &Heatmap::xNumBinsChanged, this, &HeatmapDock::plotXNumBinsChanged);
	connect(m_plot, &Heatmap::yNumBinsChanged, this, &HeatmapDock::plotYNumBinsChanged);
	connect(m_plot, &Heatmap::automaticLimitsChanged, this, &HeatmapDock::plotAutomaticLimitsChanged);
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

// void LollipopPlotDock::loadDataColumns() {
//	// add the combobox for the first column, is always present
//	if (m_dataComboBoxes.count() == 0)
//		addDataColumn();

//	int count = m_plot->dataColumns().count();
//	ui.cbNumberLine->clear();
//	ui.cbNumberSymbol->clear();

//	if (count != 0) {
//		// box plot has already data columns, make sure we have the proper number of comboboxes
//		int diff = count - m_dataComboBoxes.count();
//		if (diff > 0) {
//			for (int i = 0; i < diff; ++i)
//				addDataColumn();
//		} else if (diff < 0) {
//			for (int i = diff; i != 0; ++i)
//				removeDataColumn();
//		}

//		// show the columns in the comboboxes
//		for (int i = 0; i < count; ++i)
//			m_dataComboBoxes.at(i)->setAspect(m_plot->dataColumns().at(i));

//		// show columns names in the combobox for the selection of the bar to be modified
//		for (int i = 0; i < count; ++i)
//			if (m_plot->dataColumns().at(i)) {
//				ui.cbNumberLine->addItem(m_plot->dataColumns().at(i)->name());
//				ui.cbNumberSymbol->addItem(m_plot->dataColumns().at(i)->name());
//			}
//	} else {
//		// no data columns set in the box plot yet, we show the first combo box only
//		m_dataComboBoxes.first()->setAspect(nullptr);
//		for (int i = 0; i < m_dataComboBoxes.count(); ++i)
//			removeDataColumn();
//	}

//	// disable data column widgets if we're modifying more than one box plot at the same time
//	bool enabled = (m_plots.count() == 1);
//	m_buttonNew->setVisible(enabled);
//	for (auto* cb : m_dataComboBoxes)
//		cb->setEnabled(enabled);
//	for (auto* b : m_removeButtons)
//		b->setVisible(enabled);

//	// select the first column after all of them were added to the combobox
//	ui.cbNumberLine->setCurrentIndex(0);
//	ui.cbNumberSymbol->setCurrentIndex(0);
//}

// void LollipopPlotDock::setDataColumns() const {
//	int newCount = m_dataComboBoxes.count();
//	int oldCount = m_plot->dataColumns().count();

//	if (newCount > oldCount) {
//		ui.cbNumberLine->addItem(QString::number(newCount));
//		ui.cbNumberSymbol->addItem(QString::number(newCount));
//	} else {
//		if (newCount != 0) {
//			ui.cbNumberLine->removeItem(ui.cbNumberLine->count() - 1);
//			ui.cbNumberSymbol->removeItem(ui.cbNumberSymbol->count() - 1);
//		}
//	}

//	QVector<const AbstractColumn*> columns;

//	for (auto* cb : m_dataComboBoxes) {
//		auto* aspect = cb->currentAspect();
//		if (aspect && aspect->type() == AspectType::Column)
//			columns << static_cast<AbstractColumn*>(aspect);
//	}

//	m_plot->setDataColumns(columns);
//}

void HeatmapDock::selectColorMap() {
	CONDITIONAL_LOCK_RETURN;

	auto* dlg = new ColorMapsDialog(this);
	if (dlg->exec() == QDialog::Accepted) {
		const auto& name = dlg->name();
		ui.lColorMapPreview->setPixmap(dlg->previewPixmap());
		const auto& colors = dlg->colors(); // fetch the colors _after_ the preview pixmap was fetched to get the proper colors from the color manager
		ui.lColorMapPreview->setFocus();

		QPixmap pixmap;
		ColorMapsManager::instance()->render(pixmap, name);
		ui.lColorMapPreview->setPixmap(pixmap);

		for (auto* plot : m_plots) {
			auto f = plot->format();
			f.name = name;
			f.colors = colors;
			plot->setFormat(f);
		}
	}
}

////**********************************************************
////******* SLOTs for changes triggered in HeatmapDock *******
////**********************************************************
////"General"-tab

void HeatmapDock::dataSourceChanged() {
	CONDITIONAL_LOCK_RETURN;

	const auto datasource = Heatmap::DataSource(ui.cbDataSource->currentData().toInt());
	bool matrix = datasource == Heatmap::DataSource::Matrix;

	ui.lMatrix->setVisible(matrix);
	cbMatrix->setVisible(matrix);

	ui.lXColumn->setVisible(!matrix);
	cbXColumn->setVisible(!matrix);
	ui.lYColumn->setVisible(!matrix);
	cbYColumn->setVisible(!matrix);

	for (auto* plot : m_plots)
		plot->setDataSource(datasource);
}

void HeatmapDock::xColumnChanged(const QModelIndex& index) {
	// updateValuesWidgets();

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
	// updateValuesWidgets();

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
	// updateValuesWidgets();

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

void HeatmapDock::xNumBinsChanged(int v) {
	CONDITIONAL_LOCK_RETURN;
	for (auto* plot : m_plots)
		plot->setXNumBins(v);
}

void HeatmapDock::yNumBinsChanged(int v) {
	CONDITIONAL_LOCK_RETURN;
	for (auto* plot : m_plots)
		plot->setYNumBins(v);
}

void HeatmapDock::visibilityChanged(bool visible) {
	CONDITIONAL_LOCK_RETURN;

	for (auto* plot : m_plots)
		plot->setVisible(visible);
}

// void LollipopPlotDock::xColumnChanged(const QModelIndex& index) {
//	auto aspect = static_cast<AbstractAspect*>(index.internalPointer());
//	AbstractColumn* column(nullptr);
//	if (aspect) {
//		column = dynamic_cast<AbstractColumn*>(aspect);
//		Q_ASSERT(column);
//	}

//	ui.bRemoveXColumn->setEnabled(column != nullptr);

//	CONDITIONAL_LOCK_RETURN;

//	for (auto* plot : m_plots)
//		plot->setXColumn(column);
//}

// void LollipopPlotDock::removeXColumn() {
//	cbXColumn->setAspect(nullptr);
//	ui.bRemoveXColumn->setEnabled(false);
//	for (auto* plot : m_plots)
//		plot->setXColumn(nullptr);
// }

// void LollipopPlotDock::addDataColumn() {
//	auto* cb = new TreeViewComboBox;

//	static const QList<AspectType> list{AspectType::Folder,
//										AspectType::Workbook,
//										AspectType::Datapicker,
//										AspectType::DatapickerCurve,
//										AspectType::Spreadsheet,
//										AspectType::LiveDataSource,
//										AspectType::Column,
//										AspectType::Worksheet,
//										AspectType::CartesianPlot,
//										AspectType::XYFitCurve,
//										AspectType::XYSmoothCurve,
//										AspectType::CantorWorksheet};
//	cb->setTopLevelClasses(list);
//	cb->setModel(m_aspectTreeModel);
//	connect(cb, &TreeViewComboBox::currentModelIndexChanged, this, &LollipopPlotDock::dataColumnChanged);

//	int index = m_dataComboBoxes.size();

//	if (index == 0) {
//		QSizePolicy sizePolicy1(QSizePolicy::Expanding, QSizePolicy::Preferred);
//		sizePolicy1.setHorizontalStretch(0);
//		sizePolicy1.setVerticalStretch(0);
//		sizePolicy1.setHeightForWidth(cb->sizePolicy().hasHeightForWidth());
//		cb->setSizePolicy(sizePolicy1);
//	} else {
//		auto* button = new QPushButton();
//		button->setIcon(QIcon::fromTheme(QStringLiteral("list-remove")));
//		connect(button, &QPushButton::clicked, this, &LollipopPlotDock::removeDataColumn);
//		m_gridLayout->addWidget(button, index, 1, 1, 1);
//		m_removeButtons << button;
//	}

//	m_gridLayout->addWidget(cb, index, 0, 1, 1);
//	m_gridLayout->addWidget(m_buttonNew, index + 1, 1, 1, 1);

//	m_dataComboBoxes << cb;
//	ui.lDataColumn->setText(i18n("Columns:"));
//}

// void LollipopPlotDock::removeDataColumn() {
//	auto* sender = static_cast<QPushButton*>(QObject::sender());
//	if (sender) {
//		// remove button was clicked, determin which one and
//		// delete it together with the corresponding combobox
//		for (int i = 0; i < m_removeButtons.count(); ++i) {
//			if (sender == m_removeButtons.at(i)) {
//				delete m_dataComboBoxes.takeAt(i + 1);
//				delete m_removeButtons.takeAt(i);
//			}
//		}
//	} else {
//		// no sender is available, the function is being called directly in loadDataColumns().
//		// delete the last remove button together with the corresponding combobox
//		int index = m_removeButtons.count() - 1;
//		if (index >= 0) {
//			delete m_dataComboBoxes.takeAt(index + 1);
//			delete m_removeButtons.takeAt(index);
//		}
//	}

//	// TODO
//	if (!m_removeButtons.isEmpty()) {
//		ui.lDataColumn->setText(i18n("Columns:"));
//	} else {
//		ui.lDataColumn->setText(i18n("Column:"));
//	}

//	if (!m_initializing)
//		setDataColumns();
//}

// void LollipopPlotDock::dataColumnChanged(const QModelIndex&) {
//	CONDITIONAL_LOCK_RETURN;

//	setDataColumns();
//}

// void LollipopPlotDock::orientationChanged(int index) {
//	CONDITIONAL_LOCK_RETURN;

//	auto orientation = LollipopPlot::Orientation(index);
//	for (auto* plot : m_plots)
//		plot->setOrientation(orientation);
//}

// void LollipopPlotDock::visibilityChanged(bool state) {
//	CONDITIONAL_LOCK_RETURN;

//	for (auto* plot : m_plots)
//		plot->setVisible(state);
//}

////"Line"-tab
///*!
// * called when the current bar number was changed, shows the line properties for the selected bar.
// */
// void LollipopPlotDock::currentBarLineChanged(int index) {
//	if (index == -1)
//		return;

//	CONDITIONAL_LOCK_RETURN;

//	QList<Line*> lines;
//	for (auto* plot : m_plots) {
//		auto* line = plot->lineAt(index);
//		if (line)
//			lines << line;
//	}

//	lineWidget->setLines(lines);
//}

////"Symbol"-tab
///*!
// * called when the current bar number was changed, shows the symbol properties for the selected bar.
// */
// void LollipopPlotDock::currentBarSymbolChanged(int index) {
//	if (index == -1)
//		return;

//	CONDITIONAL_LOCK_RETURN;

//	QList<Symbol*> symbols;
//	for (auto* plot : m_plots) {
//		auto* symbol = plot->symbolAt(index);
//		if (symbol)
//			symbols << symbol;
//	}

//	symbolWidget->setSymbols(symbols);
//}

////*************************************************************
////******* SLOTs for changes triggered in Lollipop ********
////*************************************************************
//// general
///
void HeatmapDock::plotXColumnChanged(const AbstractColumn* column) {
	// updateValuesWidgets();
	CONDITIONAL_LOCK_RETURN;
	cbXColumn->setAspect(column, m_plot->xColumnPath());
}

void HeatmapDock::plotYColumnChanged(const AbstractColumn* column) {
	// updateValuesWidgets();
	CONDITIONAL_LOCK_RETURN;
	cbYColumn->setAspect(column, m_plot->yColumnPath());
}

void HeatmapDock::plotMatrixChanged(const Matrix* matrix) {
	// updateValuesWidgets();
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
// void LollipopPlotDock::plotXColumnChanged(const AbstractColumn* column) {
//	CONDITIONAL_LOCK_RETURN;
//	cbXColumn->setColumn(column, m_plot->xColumnPath());
// }
// void LollipopPlotDock::plotDataColumnsChanged(const QVector<const AbstractColumn*>&) {
//	CONDITIONAL_LOCK_RETURN;
//	loadDataColumns();
// }
// void LollipopPlotDock::plotOrientationChanged(LollipopPlot::Orientation orientation) {
//	CONDITIONAL_LOCK_RETURN;
//	ui.cbOrientation->setCurrentIndex((int)orientation);
// }
// void LollipopPlotDock::plotVisibilityChanged(bool on) {
//	CONDITIONAL_LOCK_RETURN;
//	ui.chkVisible->setChecked(on);
// }

////**********************************************************
////******************** SETTINGS ****************************
////**********************************************************
// void LollipopPlotDock::load() {
//	// general
//	ui.cbOrientation->setCurrentIndex((int)m_plot->orientation());
// }

namespace {
const QLatin1String configName("Heatmap");
}

void HeatmapDock::loadConfig(KConfig& config) {
	KConfigGroup group = config.group(configName);

	// general
	// ui.cbOrientation->setCurrentIndex(group.readEntry("Orientation", (int)m_plot->orientation()));
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

	// general
	// group.writeEntry("Orientation", ui.cbOrientation->currentIndex());

	config.sync();
}
