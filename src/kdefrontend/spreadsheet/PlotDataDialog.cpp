/*
	File                 : PlotDataDialog.cpp
	Project              : LabPlot
	Description          : Dialog for generating plots for the spreadsheet data
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2017-2023 Alexander Semke <alexander.semke@web.de>
	SPDX-FileCopyrightText: 2022 Stefan Gerlach <stefan.gerlach@uni.kn>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "PlotDataDialog.h"
#include "backend/core/AspectTreeModel.h"
#include "backend/core/Project.h"
#include "backend/core/Settings.h"
#include "backend/core/column/Column.h"
#include "backend/lib/Range.h"
#include "backend/spreadsheet/Spreadsheet.h"
#include "backend/worksheet/TextLabel.h"
#include "backend/worksheet/Worksheet.h"
#include "backend/worksheet/plots/cartesian/Axis.h"
#include "backend/worksheet/plots/cartesian/BarPlot.h"
#include "backend/worksheet/plots/cartesian/BoxPlot.h"
#include "backend/worksheet/plots/cartesian/CartesianPlot.h"
#include "backend/worksheet/plots/cartesian/Histogram.h"
#include "backend/worksheet/plots/cartesian/KDEPlot.h"
#include "backend/worksheet/plots/cartesian/LollipopPlot.h"
#include "backend/worksheet/plots/cartesian/QQPlot.h"
#include "backend/worksheet/plots/cartesian/XYAnalysisCurve.h"
#include "backend/worksheet/plots/cartesian/XYCurve.h"
#include "backend/worksheet/plots/cartesian/XYDataReductionCurve.h"
#include "backend/worksheet/plots/cartesian/XYDifferentiationCurve.h"
#include "backend/worksheet/plots/cartesian/XYFitCurve.h"
#include "backend/worksheet/plots/cartesian/XYFourierFilterCurve.h"
#include "backend/worksheet/plots/cartesian/XYIntegrationCurve.h"
#include "backend/worksheet/plots/cartesian/XYInterpolationCurve.h"
#include "backend/worksheet/plots/cartesian/XYSmoothCurve.h"
#include "commonfrontend/spreadsheet/SpreadsheetView.h"
#include "commonfrontend/widgets/TreeViewComboBox.h"

#ifdef HAVE_MQTT
#include "backend/datasources/MQTTTopic.h"
#endif

#include <QDialogButtonBox>
#include <QPushButton>
#include <QWindow>

#include <KConfigGroup>

#include <KWindowConfig>

#include "ui_plotdatawidget.h"

/*!
	\class PlotDataDialog
	\brief Dialog for generating plots for the spreadsheet data.

	\ingroup kdefrontend
 */
PlotDataDialog::PlotDataDialog(Spreadsheet* s, PlotType type, QWidget* parent)
	: QDialog(parent)
	, ui(new Ui::PlotDataWidget())
	, m_spreadsheet(s)
	, m_plotsModel(new AspectTreeModel(m_spreadsheet->project()))
	, m_worksheetsModel(new AspectTreeModel(m_spreadsheet->project()))
	, m_plotType(type) {
	setAttribute(Qt::WA_DeleteOnClose);
	setWindowTitle(i18nc("@title:window", "Plot Spreadsheet Data"));
	setWindowIcon(QIcon::fromTheme(QStringLiteral("office-chart-line")));

	auto* mainWidget = new QWidget(this);
	ui->setupUi(mainWidget);

	auto* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
	m_okButton = buttonBox->button(QDialogButtonBox::Ok);
	m_okButton->setDefault(true);
	m_okButton->setToolTip(i18n("Plot the selected data"));
	m_okButton->setText(i18n("&Plot"));

	auto* layout = new QVBoxLayout(this);
	layout->addWidget(mainWidget);
	layout->addWidget(buttonBox);
	setLayout(layout);

	// create combox boxes for the existing plots and worksheets
	auto* gridLayout = dynamic_cast<QGridLayout*>(ui->gbPlotPlacement->layout());
	cbExistingPlots = new TreeViewComboBox(ui->gbPlotPlacement);
	cbExistingPlots->setMinimumWidth(250); // TODO: use proper sizeHint in TreeViewComboBox
	gridLayout->addWidget(cbExistingPlots, 0, 1, 1, 1);

	cbExistingWorksheets = new TreeViewComboBox(ui->gbPlotPlacement);
	cbExistingWorksheets->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred));
	gridLayout->addWidget(cbExistingWorksheets, 1, 1, 1, 1);

	QList<AspectType> list{AspectType::Folder, AspectType::Worksheet, AspectType::CartesianPlot};
	cbExistingPlots->setTopLevelClasses(list);
	list = {AspectType::CartesianPlot};
	m_plotsModel->setSelectableAspects(list);
	cbExistingPlots->setModel(m_plotsModel);

	// select the first available plot, if available
	auto plots = m_spreadsheet->project()->children<CartesianPlot>(AbstractAspect::ChildIndexFlag::Recursive);
	if (!plots.isEmpty()) {
		const auto* plot = plots.first();
		cbExistingPlots->setCurrentModelIndex(m_plotsModel->modelIndexOfAspect(plot));
	}

	list = {AspectType::Folder, AspectType::Worksheet};
	cbExistingWorksheets->setTopLevelClasses(list);
	list = {AspectType::Worksheet};
	m_worksheetsModel->setSelectableAspects(list);
	cbExistingWorksheets->setModel(m_worksheetsModel);

	// select the first available worksheet, if available
	auto worksheets = m_spreadsheet->project()->children<Worksheet>(AbstractAspect::ChildIndexFlag::Recursive);
	if (!worksheets.isEmpty()) {
		const auto* worksheet = worksheets.first();
		cbExistingWorksheets->setCurrentModelIndex(m_worksheetsModel->modelIndexOfAspect(worksheet));
	}

	// in the grid layout of the scroll area we have on default one row for the x-column,
	// one row for the separating line and one line for the y-column.
	// set the height of this default content as the minimal size of the scroll area.
	gridLayout = dynamic_cast<QGridLayout*>(ui->scrollAreaColumns->widget()->layout());
	int height = 2 * ui->cbXColumn->height() + ui->line->height() + 2 * gridLayout->verticalSpacing() + gridLayout->contentsMargins().top()
		+ gridLayout->contentsMargins().bottom();
	ui->scrollAreaColumns->setMinimumSize(0, height);

	// hide the check box for creation of original data, only shown if analysis curves are to be created
	ui->spacer->changeSize(0, 0);
	ui->chkCreateDataCurve->hide();

	// SIGNALs/SLOTs
	connect(buttonBox, &QDialogButtonBox::accepted, this, [=]() {
		hide();
		plot();
	});
	connect(buttonBox, &QDialogButtonBox::rejected, this, &PlotDataDialog::reject);
	connect(buttonBox, &QDialogButtonBox::accepted, this, &PlotDataDialog::accept);
	connect(ui->rbCurvePlacementAllInOnePlotArea, &QRadioButton::toggled, this, &PlotDataDialog::curvePlacementChanged);
	connect(ui->rbCurvePlacementAllInOwnPlotArea, &QRadioButton::toggled, this, &PlotDataDialog::curvePlacementChanged);
	connect(ui->rbPlotPlacementExistingPlotArea, &QRadioButton::toggled, this, &PlotDataDialog::plotPlacementChanged);
	connect(ui->rbPlotPlacementExistingWorksheet, &QRadioButton::toggled, this, &PlotDataDialog::plotPlacementChanged);
	connect(ui->rbPlotPlacementNewWorksheet, &QRadioButton::toggled, this, &PlotDataDialog::plotPlacementChanged);
	connect(cbExistingPlots, &TreeViewComboBox::currentModelIndexChanged, this, &PlotDataDialog::checkOkButton);
	connect(cbExistingWorksheets, &TreeViewComboBox::currentModelIndexChanged, this, &PlotDataDialog::checkOkButton);

	// restore saved settings if available
	create(); // ensure there's a window created
	KConfigGroup conf = Settings::group(QStringLiteral("PlotDataDialog"));
	if (conf.exists()) {
		int index = conf.readEntry("CurvePlacement", 0);
		if (index == 2)
			ui->rbCurvePlacementAllInOwnPlotArea->setChecked(true);

		index = conf.readEntry("PlotPlacement", 0);
		if (index == 2)
			ui->rbPlotPlacementExistingWorksheet->setChecked(true);
		if (index == 3)
			ui->rbPlotPlacementNewWorksheet->setChecked(true);

		KWindowConfig::restoreWindowSize(windowHandle(), conf);
		resize(windowHandle()->size()); // workaround for QTBUG-40584
	} else
		resize(QSize(0, 0).expandedTo(minimumSize()));

	processColumns();
	plotPlacementChanged();
}

PlotDataDialog::~PlotDataDialog() {
	// save current settings
	KConfigGroup conf = Settings::group(QStringLiteral("PlotDataDialog"));
	int index = 0;
	if (ui->rbCurvePlacementAllInOnePlotArea->isChecked())
		index = 1;
	if (ui->rbCurvePlacementAllInOwnPlotArea->isChecked())
		index = 2;
	conf.writeEntry("CurvePlacement", index);

	if (ui->rbPlotPlacementExistingPlotArea->isChecked())
		index = 1;
	if (ui->rbPlotPlacementExistingWorksheet->isChecked())
		index = 2;
	if (ui->rbPlotPlacementNewWorksheet->isChecked())
		index = 3;
	conf.writeEntry("PlotPlacement", index);

	KWindowConfig::saveWindowSize(windowHandle(), conf);

	delete m_plotsModel;
	delete m_worksheetsModel;
}

void PlotDataDialog::setAnalysisAction(XYAnalysisCurve::AnalysisAction action) {
	m_analysisAction = action;
	m_analysisMode = true;
	ui->spacer->changeSize(0, 40);
	ui->chkCreateDataCurve->show();
}

void PlotDataDialog::processColumns() {
	// columns to plot
	auto* view = reinterpret_cast<SpreadsheetView*>(m_spreadsheet->view());
	auto selectedColumns = view->selectedColumns(true);

	// use all spreadsheet columns if no columns are selected
	if (selectedColumns.isEmpty())
		selectedColumns = m_spreadsheet->children<Column>();

	// skip error and non-plottable columns
	for (Column* col : selectedColumns) {
		if ((col->plotDesignation() == AbstractColumn::PlotDesignation::X || col->plotDesignation() == AbstractColumn::PlotDesignation::Y
			 || col->plotDesignation() == AbstractColumn::PlotDesignation::NoDesignation)
			&& col->isPlottable())
			m_columns << col;
	}

	// disable everything if the spreadsheet doesn't have any columns to plot
	if (m_columns.isEmpty()) {
		ui->gbCurvePlacement->setEnabled(false);
		ui->gbPlotPlacement->setEnabled(false);
		return;
	}

	// determine the column names
	// and the name of the first column having "X" as the plot designation (relevant for xy-curves only)
	QStringList columnNames;
	QString xColumnName;
	for (const Column* column : m_columns) {
		columnNames << column->name();
		if (m_plotType == PlotType::XYCurve && xColumnName.isEmpty() && column->plotDesignation() == AbstractColumn::PlotDesignation::X)
			xColumnName = column->name();
	}

	if (m_plotType == PlotType::XYCurve && xColumnName.isEmpty()) {
		// no X-column was selected -> look for the first non-selected X-column left to the first selected column
		const auto& columns = m_spreadsheet->children<Column>();
		const int index = columns.indexOf(selectedColumns.first()) - 1;
		for (int i = index; i >= 0; --i) {
			auto* column = columns.at(i);
			if (column->plotDesignation() == AbstractColumn::PlotDesignation::X && column->isPlottable()) {
				xColumnName = column->name();
				m_columns.prepend(column);
				columnNames.prepend(xColumnName);
				break;
			}
		}

		if (xColumnName.isEmpty()) {
			// no X-column was found left to the first selected column -> look right to the last selected column
			const int index = columns.indexOf(selectedColumns.last()) + 1;
			for (int i = index; i < m_spreadsheet->columnCount(); ++i) {
				auto* column = columns.at(i);
				if (column->plotDesignation() == AbstractColumn::PlotDesignation::X && column->isPlottable()) {
					xColumnName = column->name();
					m_columns.prepend(column);
					columnNames.prepend(xColumnName);
					break;
				}
			}
		}
	}

	switch (m_plotType) {
	case PlotType::XYCurve:
		processColumnsForXYCurve(columnNames, xColumnName);
		break;
	case PlotType::Histogram:
	case PlotType::KDEPlot:
	case PlotType::QQPlot:
	case PlotType::BoxPlot:
	case PlotType::BarPlot:
	case PlotType::LollipopPlot:
		processColumnsForHistogram(columnNames);
		break;
	}

	// resize the scroll area to show five ComboBoxes at maximum without showing the scroll bars
	int size = m_columnComboBoxes.size() >= 5 ? 5 : m_columnComboBoxes.size();
	int height = size * ui->cbXColumn->height();
	auto* layout = dynamic_cast<QGridLayout*>(ui->scrollAreaColumns->widget()->layout());
	if (layout) {
		height += (size + 1) * layout->verticalSpacing();
		if (m_plotType == PlotType::XYCurve)
			height += layout->verticalSpacing(); // one more spacing for the separating line
	}
	ui->scrollAreaColumns->setMinimumSize(ui->scrollAreaColumns->width(), height);
}

void PlotDataDialog::processColumnsForXYCurve(const QStringList& columnNames, const QString& xColumnName) {
	m_columnComboBoxes << ui->cbXColumn;
	m_columnComboBoxes << ui->cbYColumn;

	// ui-widget only has one combobox for the y-data -> add additional comboboxes dynamically if required
	if (m_columns.size() > 2) {
		auto* gridLayout = dynamic_cast<QGridLayout*>(ui->scrollAreaColumns->widget()->layout());
		for (int i = 2; i < m_columns.size(); ++i) {
			QLabel* label = new QLabel(i18n("Y-data"));
			auto* comboBox = new QComboBox();
			gridLayout->addWidget(label, i + 1, 0, 1, 1);
			gridLayout->addWidget(comboBox, i + 1, 2, 1, 1);
			m_columnComboBoxes << comboBox;
		}
	} else {
		// two columns provided, only one curve is possible -> hide the curve placement options
		ui->rbCurvePlacementAllInOnePlotArea->setChecked(true);
		ui->gbCurvePlacement->hide();
		ui->gbPlotPlacement->setTitle(i18n("Add Plot to"));
	}

	// show all selected/available column names in the data comboboxes
	for (auto* const comboBox : m_columnComboBoxes)
		comboBox->addItems(columnNames);

	if (!xColumnName.isEmpty()) {
		// show in the X-data combobox the first column having X as the plot designation
		ui->cbXColumn->setCurrentIndex(ui->cbXColumn->findText(xColumnName));

		// for the remaining columns, show the names in the comboboxes for the Y-data
		// TODO: handle columns with error-designations
		int yColumnIndex = 1; // the index of the first Y-data comboBox in m_columnComboBoxes
		for (const QString& name : columnNames) {
			if (name != xColumnName) {
				QComboBox* comboBox = m_columnComboBoxes[yColumnIndex];
				comboBox->setCurrentIndex(comboBox->findText(name));
				yColumnIndex++;
			}
		}
	} else {
		// no column with "x plot designation" is selected, simply show all columns in the order they were selected.
		// first selected column will serve as the x-column.
		int yColumnIndex = 0;
		for (const QString& name : columnNames) {
			QComboBox* comboBox = m_columnComboBoxes[yColumnIndex];
			comboBox->setCurrentIndex(comboBox->findText(name));
			yColumnIndex++;
		}
	}
}

/*!
 * processes columns for cases where one single column
 * is required per "plot" (histogram, boxplot, etc.)
 * */
void PlotDataDialog::processColumnsForHistogram(const QStringList& columnNames) {
	ui->line->hide();
	ui->spacer->changeSize(0, 0);
	ui->chkCreateDataCurve->hide();

	// use the already available cbXColumn combo box
	ui->lXColumn->setText(i18n("Data"));
	m_columnComboBoxes << ui->cbXColumn;
	ui->cbXColumn->addItems(columnNames);
	ui->cbXColumn->setCurrentIndex(0);

	if (m_columns.size() == 1) {
		// one column provided, only one histogram is possible
		//-> hide the curve placement options and the scroll areas for further columns
		ui->lYColumn->hide();
		ui->cbYColumn->hide();
		ui->rbCurvePlacementAllInOnePlotArea->setChecked(true);
		ui->gbCurvePlacement->hide();
		ui->gbPlotPlacement->setTitle(i18n("Add Plot to"));
		ui->scrollAreaColumns->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	} else {
		ui->gbPlotPlacement->setTitle(i18n("Add Plots to"));
		ui->scrollAreaColumns->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);

		// use the already available cbYColumn combo box
		ui->lYColumn->setText(i18n("Data"));
		m_columnComboBoxes << ui->cbYColumn;
		ui->cbYColumn->addItems(columnNames);
		ui->cbYColumn->setCurrentIndex(1);

		// add a ComboBox for every further column to be plotted
		auto* gridLayout = dynamic_cast<QGridLayout*>(ui->scrollAreaColumns->widget()->layout());
		for (int i = 2; i < m_columns.size(); ++i) {
			auto* label = new QLabel(i18n("Data"));
			auto* comboBox = new QComboBox();
			gridLayout->addWidget(label, i + 1, 0, 1, 1);
			gridLayout->addWidget(comboBox, i + 1, 2, 1, 1);
			comboBox->addItems(columnNames);
			comboBox->setCurrentIndex(i);
			m_columnComboBoxes << comboBox;
		}
	}
}

void PlotDataDialog::plot() {
	WAIT_CURSOR;
	m_spreadsheet->project()->setSuppressAspectAddedSignal(true);
	m_lastAddedCurve = nullptr;

	if (ui->rbPlotPlacementExistingPlotArea->isChecked()) {
		// add curves to an existing plot
		auto* aspect = static_cast<AbstractAspect*>(cbExistingPlots->currentModelIndex().internalPointer());
		auto* plot = static_cast<CartesianPlot*>(aspect);
		plot->beginMacro(i18n("Plot Area - %1", m_spreadsheet->name()));
		addCurvesToPlot(plot);
		plot->endMacro();
	} else if (ui->rbPlotPlacementExistingWorksheet->isChecked()) {
		// add curves to a new plot in an existing worksheet
		auto* aspect = static_cast<AbstractAspect*>(cbExistingWorksheets->currentModelIndex().internalPointer());
		auto* worksheet = static_cast<Worksheet*>(aspect);
		worksheet->beginMacro(i18n("Worksheet - %1", m_spreadsheet->name()));

		if (ui->rbCurvePlacementAllInOnePlotArea->isChecked()) {
			// all curves in one plot
			auto* plot = new CartesianPlot(i18n("Plot Area - %1", m_spreadsheet->name()));
			plot->setType(CartesianPlot::Type::FourAxes);
			worksheet->addChild(plot);
			if (m_columnComboBoxes.count() == 2)
				setAxesColumnLabels(plot, m_columnComboBoxes.at(1)->currentText());

			addCurvesToPlot(plot);
			setAxesTitles(plot);
		} else {
			// one plot per curve
			addCurvesToPlots(worksheet);
		}
		worksheet->endMacro();
	} else {
		// add curves to a new plot(s) in a new worksheet
		auto* parent = m_spreadsheet->parentAspect();
		if (parent->type() == AspectType::DatapickerCurve)
			parent = parent->parentAspect()->parentAspect();
		else if (parent->type() == AspectType::Workbook)
			parent = parent->parentAspect();
#ifdef HAVE_MQTT
		else if (dynamic_cast<MQTTTopic*>(m_spreadsheet))
			parent = m_spreadsheet->project();
#endif
		parent->beginMacro(i18n("Plot data from %1", m_spreadsheet->name()));
		auto* worksheet = new Worksheet(i18n("Worksheet - %1", m_spreadsheet->name()));
		parent->addChild(worksheet);

		if (ui->rbCurvePlacementAllInOnePlotArea->isChecked()) {
			// all curves in one plot
			auto* plot = new CartesianPlot(i18n("Plot Area - %1", m_spreadsheet->name()));
			plot->setType(CartesianPlot::Type::FourAxes);
			worksheet->addChild(plot);
			if (m_columnComboBoxes.count() == 2)
				setAxesColumnLabels(plot, m_columnComboBoxes.at(1)->currentText());
			addCurvesToPlot(plot);
			setAxesTitles(plot);
		} else {
			// one plot per curve
			addCurvesToPlots(worksheet);
		}

		// when creating a new worksheet with many plots it can happen the place available for the data in the plot
		// is small and the result created here doesn't look nice. To avoid this and as a quick a dirty workaround,
		// we increase the size of the worksheet so the plots have a certain minimal size.
		// TODO:
		// A more sophisticated and better logic would require further adjustments for properties like plot area
		// paddings, the font size of axis ticks and title labels, etc. Also, this logic should be applied maybe if
		// we add plots to an already created worksheet.
		adjustWorksheetSize(worksheet);

		parent->endMacro();
	}

	// if new curves are created via this dialog, select the parent plot of the last added curve in the project explorer.
	// if a custom fit is being done, select the last created fit curve independent of the number of created curves.
	m_spreadsheet->project()->setSuppressAspectAddedSignal(false);
	if (m_lastAddedCurve) {
		QString path;
		if (!m_analysisMode) {
			path = m_lastAddedCurve->parentAspect()->path();
		} else {
			if (m_analysisAction == XYAnalysisCurve::AnalysisAction::FitCustom)
				path = m_lastAddedCurve->path();
			else
				path = m_lastAddedCurve->parentAspect()->path();
		}

		Q_EMIT m_spreadsheet->project()->requestNavigateTo(path);
	}
	RESET_CURSOR;
}

Column* PlotDataDialog::columnFromName(const QString& name) const {
	for (auto* column : m_columns) {
		if (column->name() == name)
			return column;
	}
	return nullptr;
}

/*!
 * * for the selected columns in this dialog, creates a curve in the already existing plot \c plot.
 */
void PlotDataDialog::addCurvesToPlot(CartesianPlot* plot) {
	QApplication::processEvents(QEventLoop::AllEvents, 100);
	switch (m_plotType) {
	case PlotType::XYCurve: {
		Column* xColumn = columnFromName(ui->cbXColumn->currentText());
		for (auto* comboBox : m_columnComboBoxes) {
			const QString& name = comboBox->currentText();
			Column* yColumn = columnFromName(name);

			// if only one column was selected, allow to use this column for x and for y.
			// otherwise, don't assign xColumn to y
			if (yColumn == xColumn) {
				if (m_columns.size() == 1) {
					addCurve(name, xColumn, yColumn, plot);
					break;
				} else
					continue;
			}

			addCurve(name, xColumn, yColumn, plot);
		}
		break;
	}
	case PlotType::Histogram:
	case PlotType::KDEPlot:
	case PlotType::QQPlot: {
		for (auto* comboBox : m_columnComboBoxes) {
			const QString& name = comboBox->currentText();
			Column* column = columnFromName(name);
			addSingleSourceColumnPlot(column, plot);
		}
		break;
	}
	case PlotType::BoxPlot:
	case PlotType::BarPlot:
	case PlotType::LollipopPlot: {
		QVector<const AbstractColumn*> columns;
		for (auto* comboBox : m_columnComboBoxes)
			columns << columnFromName(comboBox->currentText());

		addMultiSourceColumnsPlot(columns, plot);
		break;
	}
	}

	// TODO: check if it really needs to update all ranges
	plot->dataChanged(-1, -1);
}

/*!
 * for the selected columns in this dialog, creates a plot and a curve in the already existing worksheet \c worksheet.
 */
void PlotDataDialog::addCurvesToPlots(Worksheet* worksheet) {
	QApplication::processEvents(QEventLoop::AllEvents, 100);
	worksheet->setSuppressLayoutUpdate(true);

	switch (m_plotType) {
	case PlotType::XYCurve: {
		const QString& xColumnName = ui->cbXColumn->currentText();
		Column* xColumn = columnFromName(xColumnName);
		for (auto* comboBox : m_columnComboBoxes) {
			const QString& name = comboBox->currentText();
			Column* yColumn = columnFromName(name);
			if (yColumn == xColumn)
				continue;

			auto* plot = new CartesianPlot(i18n("Plot Area %1", name));
			plot->setType(CartesianPlot::Type::FourAxes);
			worksheet->addChild(plot);
			setAxesColumnLabels(plot, yColumn);
			addCurve(name, xColumn, yColumn, plot);
			plot->scaleAuto(-1, -1);
			plot->retransform();
			setAxesTitles(plot, name);
		}
		break;
	}
	case PlotType::Histogram:
	case PlotType::KDEPlot:
	case PlotType::QQPlot: {
		for (auto* comboBox : m_columnComboBoxes) {
			const QString& name = comboBox->currentText();
			Column* column = columnFromName(name);

			auto* plot = new CartesianPlot(i18n("Plot Area %1", name));
			plot->setType(CartesianPlot::Type::FourAxes);
			setAxesTitles(plot, name);
			worksheet->addChild(plot);
			addSingleSourceColumnPlot(column, plot);
			plot->scaleAuto(-1, -1);
			plot->retransform();
		}
		break;
	}
	case PlotType::BoxPlot:
	case PlotType::BarPlot:
	case PlotType::LollipopPlot: {
		for (auto* comboBox : m_columnComboBoxes) {
			const QString& name = comboBox->currentText();
			Column* column = columnFromName(name);

			auto* plot = new CartesianPlot(i18n("Plot Area %1", name));
			plot->setType(CartesianPlot::Type::FourAxes);
			worksheet->addChild(plot);
			addMultiSourceColumnsPlot(QVector<const AbstractColumn*>{column}, plot);
			plot->scaleAuto(-1, -1);
			plot->retransform();
			setAxesTitles(plot, name);
		}
		break;
	}
	}

	worksheet->setSuppressLayoutUpdate(false);
	worksheet->updateLayout();
}

/*!
 * helper function that does the actual creation of the curve and adding it as child to the \c plot.
 */
void PlotDataDialog::addCurve(const QString& name, Column* xColumn, Column* yColumn, CartesianPlot* plot) {
	if (!m_analysisMode) {
		auto* curve = new XYCurve(name);
		curve->setSuppressRetransform(true);
		curve->setXColumn(xColumn);
		curve->setYColumn(yColumn);
		curve->setSuppressRetransform(false);
		plot->addChild(curve);
		m_lastAddedCurve = curve;
	} else {
		bool createDataCurve = ui->chkCreateDataCurve->isChecked();
		XYCurve* curve = nullptr;
		if (createDataCurve) {
			curve = new XYCurve(name);
			curve->setSuppressRetransform(true);
			curve->setXColumn(xColumn);
			curve->setYColumn(yColumn);
			curve->setSuppressRetransform(false);
			plot->addChild(curve);
			m_lastAddedCurve = curve;
		}

		XYAnalysisCurve* analysisCurve = nullptr;
		switch (m_analysisAction) {
		case XYAnalysisCurve::AnalysisAction::DataReduction:
			analysisCurve = new XYDataReductionCurve(i18n("Reduction of '%1'", name));
			break;
		case XYAnalysisCurve::AnalysisAction::Differentiation:
			analysisCurve = new XYDifferentiationCurve(i18n("Derivative of '%1'", name));
			break;
		case XYAnalysisCurve::AnalysisAction::Integration:
			analysisCurve = new XYIntegrationCurve(i18n("Integral of '%1'", name));
			break;
		case XYAnalysisCurve::AnalysisAction::Interpolation:
			analysisCurve = new XYInterpolationCurve(i18n("Interpolation of '%1'", name));
			break;
		case XYAnalysisCurve::AnalysisAction::Smoothing:
			analysisCurve = new XYSmoothCurve(i18n("Smoothing of '%1'", name));
			break;
		case XYAnalysisCurve::AnalysisAction::FitLinear:
		case XYAnalysisCurve::AnalysisAction::FitPower:
		case XYAnalysisCurve::AnalysisAction::FitExp1:
		case XYAnalysisCurve::AnalysisAction::FitExp2:
		case XYAnalysisCurve::AnalysisAction::FitInvExp:
		case XYAnalysisCurve::AnalysisAction::FitGauss:
		case XYAnalysisCurve::AnalysisAction::FitCauchyLorentz:
		case XYAnalysisCurve::AnalysisAction::FitTan:
		case XYAnalysisCurve::AnalysisAction::FitTanh:
		case XYAnalysisCurve::AnalysisAction::FitErrFunc:
		case XYAnalysisCurve::AnalysisAction::FitCustom:
			analysisCurve = new XYFitCurve(i18n("Fit to '%1'", name));
			static_cast<XYFitCurve*>(analysisCurve)->initFitData(m_analysisAction);
			static_cast<XYFitCurve*>(analysisCurve)->initStartValues(curve);
			break;
		case XYAnalysisCurve::AnalysisAction::FourierFilter:
			analysisCurve = new XYFourierFilterCurve(i18n("Fourier Filter of '%1'", name));
			break;
		}

		if (analysisCurve != nullptr) {
			analysisCurve->setSuppressRetransform(true);
			analysisCurve->setXDataColumn(xColumn);
			analysisCurve->setYDataColumn(yColumn);
			if (m_analysisAction != XYAnalysisCurve::AnalysisAction::FitCustom) // no custom fit-model set yet, no need to recalculate
				analysisCurve->recalculate();
			analysisCurve->setSuppressRetransform(false);
			plot->addChild(analysisCurve);
			m_lastAddedCurve = analysisCurve;
		}
	}
}

void PlotDataDialog::addSingleSourceColumnPlot(const Column* column, CartesianPlot* plotArea) {
	const QString& name = column->name();
	QApplication::processEvents(QEventLoop::AllEvents, 100);
	Plot* plot{nullptr};
	if (m_plotType == PlotType::Histogram) {
		auto* hist = new Histogram(name);
		hist->setDataColumn(column);
		plot = hist;
	} else if (m_plotType == PlotType::KDEPlot) {
		auto* kdePlot = new KDEPlot(name);
		kdePlot->setDataColumn(column);
		plot = kdePlot;
	} else if (m_plotType == PlotType::QQPlot) {
		auto* qqPlot = new QQPlot(name);
		qqPlot->setDataColumn(column);
		plot = qqPlot;
	}

	if (plot) {
		plotArea->addChild(plot);
		m_lastAddedCurve = plot;
	}
}

void PlotDataDialog::addMultiSourceColumnsPlot(const QVector<const AbstractColumn*>& columns, CartesianPlot* plotArea) {
	QString name;
	if (columns.size() > 1) {
		if (m_plotType == PlotType::BoxPlot)
			name = i18n("Box Plot");
		else if (m_plotType == PlotType::BarPlot)
			name = i18n("Bar Plot");
		else if (m_plotType == PlotType::LollipopPlot)
			name = i18n("Bar Plot");
	} else
		name = columns.constFirst()->name();

	QApplication::processEvents(QEventLoop::AllEvents, 100);
	Plot* plot{nullptr};
	if (m_plotType == PlotType::BoxPlot) {
		auto* boxPlot = new BoxPlot(name);
		boxPlot->setDataColumns(columns);
		plot = boxPlot;
	} else if (m_plotType == PlotType::BarPlot) {
		auto* barPlot = new BarPlot(name);
		barPlot->setDataColumns(columns);
		plot = barPlot;
	} else if (m_plotType == PlotType::LollipopPlot) {
		auto* lollipopPlot = new LollipopPlot(name);
		lollipopPlot->setDataColumns(columns);
		plot = lollipopPlot;
	}

	if (plot) {
		plotArea->addChild(plot);
		m_lastAddedCurve = plot;
	}
}

void PlotDataDialog::adjustWorksheetSize(Worksheet* worksheet) const {
	// adjust the sizes
	const auto layout = worksheet->layout();
	const auto plots = worksheet->children<CartesianPlot>();
	const int count = plots.size();
	const double minSize = 4.0;
	switch (layout) {
	case Worksheet::Layout::NoLayout:
	case Worksheet::Layout::VerticalLayout: {
		if (layout == Worksheet::Layout::NoLayout)
			worksheet->setLayout(Worksheet::Layout::VerticalLayout);

		const auto plot = plots.constFirst();
		double height = Worksheet::convertFromSceneUnits(plot->rect().height(), Worksheet::Unit::Centimeter);
		if (height < 4.) {
			double newHeight = worksheet->layoutTopMargin() + worksheet->layoutBottomMargin() + (count - 1) * worksheet->layoutHorizontalSpacing()
				+ count * Worksheet::convertToSceneUnits(minSize, Worksheet::Unit::Centimeter);
			QRectF newRect = worksheet->pageRect();
			newRect.setHeight(round(newHeight));
			worksheet->setPageRect(newRect);
		}
		break;
	}
	case Worksheet::Layout::HorizontalLayout: {
		const auto plot = plots.constFirst();
		double width = Worksheet::convertFromSceneUnits(plot->rect().width(), Worksheet::Unit::Centimeter);
		if (width < 4.) {
			double newWidth = worksheet->layoutLeftMargin() + worksheet->layoutRightMargin() + (count - 1) * worksheet->layoutVerticalSpacing()
				+ count * Worksheet::convertToSceneUnits(minSize, Worksheet::Unit::Centimeter);
			QRectF newRect = worksheet->pageRect();
			newRect.setWidth(round(newWidth));
			worksheet->setPageRect(newRect);
		}
		break;
	}
	case Worksheet::Layout::GridLayout: {
		const auto plot = plots.constFirst();
		double width = Worksheet::convertFromSceneUnits(plot->rect().width(), Worksheet::Unit::Centimeter);
		double height = Worksheet::convertFromSceneUnits(plot->rect().height(), Worksheet::Unit::Centimeter);
		if (width < 4. || height < 4.) {
			QRectF newRect = worksheet->pageRect();
			if (height < 4.) {
				double newHeight = worksheet->layoutTopMargin() + worksheet->layoutBottomMargin() + (count - 1) * worksheet->layoutHorizontalSpacing()
					+ count * Worksheet::convertToSceneUnits(minSize, Worksheet::Unit::Centimeter);
				newRect.setHeight(round(newHeight));
			} else {
				double newWidth = worksheet->layoutLeftMargin() + worksheet->layoutRightMargin() + (count - 1) * worksheet->layoutVerticalSpacing()
					+ count * Worksheet::convertToSceneUnits(minSize, Worksheet::Unit::Centimeter);
				newRect.setWidth(round(newWidth));
			}
			worksheet->setPageRect(newRect);
		}
		break;
	}
	}
}

void PlotDataDialog::setAxesColumnLabels(CartesianPlot* plot, const Column* column) {
	// Use value labels if possible
	if (column && column->valueLabelsInitialized()) {
		auto axes = plot->children(AspectType::Axis);
		for (auto a : axes) {
			auto axis = static_cast<Axis*>(a);
			if (axis->orientation() == Axis::Orientation::Vertical) {
				// Use first vertical axis
				axis->setMajorTicksType(Axis::TicksType::ColumnLabels);
				axis->setMajorTicksColumn(column);
				break;
			}
		}
	}
}

void PlotDataDialog::setAxesColumnLabels(CartesianPlot* plot, const QString& columnName) {
	Column* column = columnFromName(columnName);
	setAxesColumnLabels(plot, column);
}

void PlotDataDialog::setAxesTitles(CartesianPlot* plot, const QString& name) const {
	DEBUG(Q_FUNC_INFO)
	const auto& axes = plot->children<Axis>();
	switch (m_plotType) {
	case PlotType::XYCurve: {
		// x-axis title
		const QString& xColumnName = ui->cbXColumn->currentText();
		// DEBUG(Q_FUNC_INFO << ", x column name = " << STDSTRING(xColumnName))

		for (auto* axis : axes) {
			if (axis->orientation() == Axis::Orientation::Horizontal) {
				axis->title()->setText(xColumnName);
				break;
			}
		}

		// y-axis title
		for (auto* axis : axes) {
			if (axis->orientation() == Axis::Orientation::Vertical) {
				if (!name.isEmpty()) {
					// multiple columns are plotted with "one curve per plot",
					// the function is called with the column name.
					// use it for the x-axis title
					axis->title()->setText(name);
				} else if (m_columnComboBoxes.size() == 2) {
					// if we only have one single y-column to plot, we can set the title of the y-axes
					const QString& yColumnName = m_columnComboBoxes[1]->currentText();
					axis->title()->setText(yColumnName);
				}

				break;
			}
		}
		break;
	}
	case PlotType::Histogram:
	case PlotType::KDEPlot: {
		// x-axis title
		for (auto* axis : axes) {
			if (axis->orientation() == Axis::Orientation::Horizontal) {
				if (!name.isEmpty()) {
					// multiple columns are plotted with "one curve per plot",
					// the function is called with the column name.
					// use it for the x-axis title
					axis->title()->setText(name);
				} else if (m_columnComboBoxes.size() == 1) {
					const QString& yColumnName = m_columnComboBoxes.constFirst()->currentText();
					axis->title()->setText(yColumnName);
				}

				break;
			}
		}

		// y-axis title
		for (auto* axis : axes) {
			if (axis->orientation() == Axis::Orientation::Vertical) {
				if (m_plotType == PlotType::Histogram)
					axis->title()->setText(i18n("Frequency"));
				else
					axis->title()->setText(i18n("Density"));
				break;
			}
		}
		break;
	}
	case PlotType::QQPlot: {
		// x-axis title
		for (auto* axis : axes) {
			if (axis->orientation() == Axis::Orientation::Horizontal) {
				axis->title()->setText(i18n("Theoretical Quantiles"));
				break;
			}
		}

		// y-axis title
		for (auto* axis : axes) {
			if (axis->orientation() == Axis::Orientation::Vertical) {
				axis->title()->setText(i18n("Sample Quantiles"));
				break;
			}
		}
		break;
	}
	case PlotType::BoxPlot: {
		auto* boxPlot = static_cast<BoxPlot*>(m_lastAddedCurve);
		auto orientation = boxPlot->orientation();

		// number of box plots per plot:
		int count = 1; // 1 if we create a separate plot for every box plot
		if (ui->rbCurvePlacementAllInOnePlotArea->isChecked())
			count = m_columnComboBoxes.count(); // all box plots in one single plot

		// set the range of the plot and the number of ticks manually.
		// the n-th box plot is positioned at x=n and has the width=0.5 in logical coordinatates.
		// manually set the range to (0.5, n+0.5) and ajdust the number of ticks starting at 0.5
		// to make sure we have every tick precisely under the middle of the box plot
		const auto xIndex = plot->coordinateSystem(boxPlot->coordinateSystemIndex())->index(Dimension::X);
		const auto yIndex = plot->coordinateSystem(boxPlot->coordinateSystemIndex())->index(Dimension::Y);
		Range<double> range(0.5, count + 0.5);
		if (orientation == WorksheetElement::Orientation::Vertical)
			plot->setRange(Dimension::X, xIndex, range);
		else
			plot->setRange(Dimension::Y, yIndex, range);

		for (auto* axis : axes) {
			if (axis->orientation() != orientation) {
				axis->setMajorTicksNumber(count + 1);
				axis->setMajorTickStartOffset(0.5);
				axis->setMinorTicksNumber(0);
			}
			axis->title()->setText(QString()); // no title
		}

		// y-axis title
		for (auto* axis : axes) {
			if (axis->orientation() == Axis::Orientation::Vertical) {
				if (!name.isEmpty()) {
					// multiple columns are plotted with "one curve per plot",
					// the function is called with the column name.
					// use it for the x-axis title
					axis->title()->setText(name);
				} else if (count == 1) {
					const QString& yColumnName = m_columnComboBoxes.constFirst()->currentText();
					axis->title()->setText(yColumnName);
				}
				break;
			}
		}
		break;
	}
	case PlotType::BarPlot:
	case PlotType::LollipopPlot: {
		WorksheetElement::Orientation orientation;
		if (m_plotType == PlotType::BarPlot) {
			auto* barPlot = static_cast<BarPlot*>(m_lastAddedCurve);
			orientation = barPlot->orientation();
		} else {
			auto* lollipopPlot = static_cast<LollipopPlot*>(m_lastAddedCurve);
			orientation = lollipopPlot->orientation();
		}

		plot->setNiceExtend(false);

		for (auto* axis : axes) {
			if (axis->orientation() != orientation) {
				axis->setMajorTicksType(Axis::TicksType::Spacing);
				axis->setMajorTickStartOffset(0.5);
				axis->setMajorTicksSpacing(1.);
				axis->setLabelsPosition(Axis::LabelsPosition::NoLabels);
				axis->setMinorTicksDirection(Axis::noTicks);
			}
			axis->title()->setText(QString()); // no title
		}
	}
	}
}

// ################################################################
// ########################## Slots ###############################
// ################################################################
void PlotDataDialog::curvePlacementChanged() {
	if (ui->rbCurvePlacementAllInOnePlotArea->isChecked()) {
		ui->rbPlotPlacementExistingPlotArea->setEnabled(true);
	} else {
		ui->rbPlotPlacementExistingPlotArea->setEnabled(false);
		if (ui->rbPlotPlacementExistingPlotArea->isChecked())
			ui->rbPlotPlacementExistingWorksheet->setChecked(true);
	}
}

void PlotDataDialog::plotPlacementChanged() {
	if (ui->rbPlotPlacementExistingPlotArea->isChecked()) {
		cbExistingPlots->setEnabled(true);
		cbExistingWorksheets->setEnabled(false);
	} else if (ui->rbPlotPlacementExistingWorksheet->isChecked()) {
		cbExistingPlots->setEnabled(false);
		cbExistingWorksheets->setEnabled(true);
	} else {
		cbExistingPlots->setEnabled(false);
		cbExistingWorksheets->setEnabled(false);
	}

	checkOkButton();
}

void PlotDataDialog::checkOkButton() {
	bool enable = false;
	QString msg;
	if ((m_plotType == PlotType::XYCurve && (ui->cbXColumn->currentIndex() == -1 || ui->cbYColumn->currentIndex() == -1))
		|| (m_plotType == PlotType::Histogram && ui->cbXColumn->currentIndex() == -1))
		msg = i18n("No data selected to plot.");
	else if (ui->rbPlotPlacementExistingPlotArea->isChecked()) {
		AbstractAspect* aspect = static_cast<AbstractAspect*>(cbExistingPlots->currentModelIndex().internalPointer());
		enable = (aspect != nullptr);
		if (!enable)
			msg = i18n("An already existing plot area has to be selected.");
	} else if (ui->rbPlotPlacementExistingWorksheet->isChecked()) {
		AbstractAspect* aspect = static_cast<AbstractAspect*>(cbExistingWorksheets->currentModelIndex().internalPointer());
		enable = (aspect != nullptr);
		if (!enable)
			msg = i18n("An already existing worksheet has to be selected.");
	} else
		enable = true;

	m_okButton->setEnabled(enable);
	if (enable)
		m_okButton->setToolTip(i18n("Close the dialog and plot the data."));
	else
		m_okButton->setToolTip(msg);
}
