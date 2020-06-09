/***************************************************************************
    File                 : PlotDataDialog.cpp
    Project              : LabPlot
    Description          : Dialog for generating plots for the spreadsheet data
    --------------------------------------------------------------------
    Copyright            : (C) 2017-2019 by Alexander Semke (alexander.semke@web.de)

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *  This program is free software; you can redistribute it and/or modify   *
 *  it under the terms of the GNU General Public License as published by   *
 *  the Free Software Foundation; either version 2 of the License, or      *
 *  (at your option) any later version.                                    *
 *                                                                         *
 *  This program is distributed in the hope that it will be useful,        *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 *  GNU General Public License for more details.                           *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the Free Software           *
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor,                    *
 *   Boston, MA  02110-1301  USA                                           *
 *                                                                         *
 ***************************************************************************/

#include "PlotDataDialog.h"
#include "backend/core/AspectTreeModel.h"
#include "backend/core/Project.h"
#include "backend/core/column/Column.h"
#include "backend/datapicker/DatapickerCurve.h"
#include "backend/spreadsheet/Spreadsheet.h"
#include "backend/worksheet/plots/cartesian/Axis.h"
#include "backend/worksheet/plots/cartesian/XYAnalysisCurve.h"
#include "backend/worksheet/plots/cartesian/XYCurve.h"
#include "backend/worksheet/plots/cartesian/XYDataReductionCurve.h"
#include "backend/worksheet/plots/cartesian/XYDifferentiationCurve.h"
#include "backend/worksheet/plots/cartesian/XYIntegrationCurve.h"
#include "backend/worksheet/plots/cartesian/XYInterpolationCurve.h"
#include "backend/worksheet/plots/cartesian/XYSmoothCurve.h"
#include "backend/worksheet/plots/cartesian/XYFitCurve.h"
#include "backend/worksheet/plots/cartesian/XYFourierFilterCurve.h"

#ifdef HAVE_MQTT
#include "backend/datasources/MQTTTopic.h"
#endif

#include "backend/worksheet/plots/cartesian/CartesianPlot.h"
#include "backend/worksheet/Worksheet.h"
#include "backend/worksheet/TextLabel.h"
#include "commonfrontend/spreadsheet/SpreadsheetView.h"
#include "commonfrontend/widgets/TreeViewComboBox.h"

#include <QDialogButtonBox>
#include <QPushButton>
#include <QWindow>

#include <KConfigGroup>
#include <KSharedConfig>
#include <KWindowConfig>

#include "ui_plotdatawidget.h"

/*!
	\class PlotDataDialog
	\brief Dialog for generating plots for the spreadsheet data.

	\ingroup kdefrontend
 */
PlotDataDialog::PlotDataDialog(Spreadsheet* s, PlotType type, QWidget* parent) : QDialog(parent),
	ui(new Ui::PlotDataWidget()),
	m_spreadsheet(s),
	m_plotsModel(new AspectTreeModel(m_spreadsheet->project())),
	m_worksheetsModel(new AspectTreeModel(m_spreadsheet->project())),
	m_plotType(type) {

	setAttribute(Qt::WA_DeleteOnClose);
	setWindowTitle(i18nc("@title:window", "Plot Spreadsheet Data"));
	setWindowIcon(QIcon::fromTheme("office-chart-line"));

	QWidget* mainWidget = new QWidget(this);
	ui->setupUi(mainWidget);

	QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
	m_okButton = buttonBox->button(QDialogButtonBox::Ok);
	m_okButton->setDefault(true);
	m_okButton->setToolTip(i18n("Plot the selected data"));
	m_okButton->setText(i18n("&Plot"));

	auto* layout = new QVBoxLayout(this);
	layout->addWidget(mainWidget);
	layout->addWidget(buttonBox);
	setLayout(layout);

	//create combox boxes for the existing plots and worksheets
	auto* gridLayout = dynamic_cast<QGridLayout*>(ui->gbPlotPlacement->layout());
	cbExistingPlots = new TreeViewComboBox(ui->gbPlotPlacement);
	cbExistingPlots->setMinimumWidth(250);//TODO: use proper sizeHint in TreeViewComboBox
	gridLayout->addWidget(cbExistingPlots, 0, 1, 1, 1);

	cbExistingWorksheets = new TreeViewComboBox(ui->gbPlotPlacement);
	cbExistingWorksheets->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred));
	gridLayout->addWidget(cbExistingWorksheets, 1, 1, 1, 1);

	QList<AspectType> list{AspectType::Folder, AspectType::Worksheet, AspectType::CartesianPlot};
	cbExistingPlots->setTopLevelClasses(list);
	list = {AspectType::CartesianPlot};
	m_plotsModel->setSelectableAspects(list);
	cbExistingPlots->setModel(m_plotsModel);

	//select the first available plot, if available
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

	//select the first available worksheet, if available
	auto worksheets = m_spreadsheet->project()->children<Worksheet>(AbstractAspect::ChildIndexFlag::Recursive);
	if (!worksheets.isEmpty()) {
		const auto* worksheet = worksheets.first();
		cbExistingWorksheets->setCurrentModelIndex(m_worksheetsModel->modelIndexOfAspect(worksheet));
	}

	//in the grid layout of the scroll area we have on default one row for the x-column,
	//one row for the separating line and one line for the y-column.
	//set the height of this default content as the minimal size of the scroll area.
	gridLayout = dynamic_cast<QGridLayout*>(ui->scrollAreaColumns->widget()->layout());
	int height = 2*ui->cbXColumn->height() + ui->line->height()
				+ 2*gridLayout->verticalSpacing()
				+ gridLayout->contentsMargins().top()
				+ gridLayout->contentsMargins().bottom();
	ui->scrollAreaColumns->setMinimumSize(0, height);

	//hide the check box for creation of original data, only shown if analysis curves are to be created
	ui->spacer->changeSize(0, 0);
	ui->chkCreateDataCurve->hide();

	//SIGNALs/SLOTs
	connect(buttonBox, &QDialogButtonBox::accepted, this, [=]() { hide();  plot(); });
	connect(buttonBox, &QDialogButtonBox::rejected, this, &PlotDataDialog::reject);
	connect(buttonBox, &QDialogButtonBox::accepted, this, &PlotDataDialog::accept);
	connect(ui->rbCurvePlacement1, &QRadioButton::toggled, this, &PlotDataDialog::curvePlacementChanged);
	connect(ui->rbCurvePlacement2, &QRadioButton::toggled, this, &PlotDataDialog::curvePlacementChanged);
	connect(ui->rbPlotPlacement1, &QRadioButton::toggled, this, &PlotDataDialog::plotPlacementChanged);
	connect(ui->rbPlotPlacement2, &QRadioButton::toggled, this, &PlotDataDialog::plotPlacementChanged);
	connect(ui->rbPlotPlacement3, &QRadioButton::toggled, this, &PlotDataDialog::plotPlacementChanged);
	connect(cbExistingPlots, &TreeViewComboBox::currentModelIndexChanged, this, &PlotDataDialog::checkOkButton);
	connect(cbExistingWorksheets, &TreeViewComboBox::currentModelIndexChanged, this, &PlotDataDialog::checkOkButton);

	//restore saved settings if available
	create(); // ensure there's a window created
	KConfigGroup conf(KSharedConfig::openConfig(), "PlotDataDialog");
	if (conf.exists()) {
		int index = conf.readEntry("CurvePlacement", 0);
		if (index == 2) ui->rbCurvePlacement2->setChecked(true);

		index = conf.readEntry("PlotPlacement", 0);
		if (index == 2) ui->rbPlotPlacement2->setChecked(true);
		if (index == 3) ui->rbPlotPlacement3->setChecked(true);

		KWindowConfig::restoreWindowSize(windowHandle(), conf);
		resize(windowHandle()->size()); // workaround for QTBUG-40584
	} else
		resize(QSize(0, 0).expandedTo(minimumSize()));

	processColumns();
	plotPlacementChanged();
}

PlotDataDialog::~PlotDataDialog() {
	//save current settings
	KConfigGroup conf(KSharedConfig::openConfig(), "PlotDataDialog");
	int index = 0;
	if (ui->rbCurvePlacement1->isChecked()) index = 1;
	if (ui->rbCurvePlacement2->isChecked()) index = 2;
	conf.writeEntry("CurvePlacement", index);

	if (ui->rbPlotPlacement1->isChecked()) index = 1;
	if (ui->rbPlotPlacement2->isChecked()) index = 2;
	if (ui->rbPlotPlacement3->isChecked()) index = 3;
	conf.writeEntry("PlotPlacement", index);

	KWindowConfig::saveWindowSize(windowHandle(), conf);

	delete m_plotsModel;
	delete m_worksheetsModel;
}

void PlotDataDialog::setAnalysisAction(AnalysisAction action) {
	m_analysisAction = action;
	m_analysisMode = true;
	ui->spacer->changeSize(0, 40);
	ui->chkCreateDataCurve->show();
}

void PlotDataDialog::processColumns() {
	//columns to plot
	auto* view = reinterpret_cast<SpreadsheetView*>(m_spreadsheet->view());
	QVector<Column*> selectedColumns = view->selectedColumns(true);

	//use all spreadsheet columns if no columns are selected
	if (selectedColumns.isEmpty())
		selectedColumns = m_spreadsheet->children<Column>();

	//skip error and non-plottable columns
	for (Column* col : selectedColumns) {
		if ((col->plotDesignation() == AbstractColumn::PlotDesignation::X || col->plotDesignation() == AbstractColumn::PlotDesignation::Y
			|| col->plotDesignation() == AbstractColumn::PlotDesignation::NoDesignation) && col->isPlottable())
			m_columns << col;
	}

	//disable everything if the spreadsheet doesn't have any columns to plot
	if (m_columns.isEmpty()) {
		ui->gbCurvePlacement->setEnabled(false);
		ui->gbPlotPlacement->setEnabled(false);
		return;
	}

	//determine the column names
	//and the name of the first column having "X" as the plot designation (relevant for xy-curves only)
	QStringList columnNames;
	QString xColumnName;
	for (const Column* column : m_columns) {
		columnNames << column->name();
		if (m_plotType == PlotType::XYCurve && xColumnName.isEmpty() && column->plotDesignation() == AbstractColumn::PlotDesignation::X)
			xColumnName = column->name();
	}

	if (m_plotType == PlotType::XYCurve && xColumnName.isEmpty()) {
		//no X-column was selected -> look for the first non-selected X-column left to the first selected column
		const int index = m_spreadsheet->indexOfChild<Column>(selectedColumns.first()) - 1;
		if (index >= 0) {
			for (int i = index; i >= 0; --i) {
				Column* column = m_spreadsheet->column(i);
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
		processColumnsForHistogram(columnNames);
		break;
	}

	//resize the scroll area to show five ComboBoxes at maximum without showing the scroll bars
	int size = m_columnComboBoxes.size() >= 5 ? 5 : m_columnComboBoxes.size();
	int height = size * ui->cbXColumn->height();
	auto* layout = dynamic_cast<QGridLayout*>(ui->scrollAreaColumns->widget()->layout());
	if (layout) {
		height += (size + 1)*layout->verticalSpacing();
		if (m_plotType == PlotType::XYCurve)
			height += layout->verticalSpacing(); //one more spacing for the separating line
	}
	ui->scrollAreaColumns->setMinimumSize(ui->scrollAreaColumns->width(), height);
}

void PlotDataDialog::processColumnsForXYCurve(const QStringList& columnNames, const QString& xColumnName) {
	m_columnComboBoxes << ui->cbXColumn;
	m_columnComboBoxes << ui->cbYColumn;

	//ui-widget only has one combobox for the y-data -> add additional comboboxes dynamically if required
	if (m_columns.size() > 2) {
		auto* gridLayout = dynamic_cast<QGridLayout*>(ui->scrollAreaColumns->widget()->layout());
		for (int i = 2; i < m_columns.size(); ++i) {
			QLabel* label = new QLabel(i18n("Y-data"));
			auto* comboBox = new QComboBox();
			gridLayout->addWidget(label, i+1, 0, 1, 1);
			gridLayout->addWidget(comboBox, i+1, 2, 1, 1);
			m_columnComboBoxes << comboBox;
		}
	} else {
		//two columns provided, only one curve is possible -> hide the curve placement options
		ui->rbCurvePlacement1->setChecked(true);
		ui->gbCurvePlacement->hide();
		ui->gbPlotPlacement->setTitle(i18n("Add Curve to"));
	}

	//show all selected/available column names in the data comboboxes
	for (QComboBox* const comboBox : m_columnComboBoxes)
		comboBox->addItems(columnNames);

	if (!xColumnName.isEmpty()) {
		//show in the X-data combobox the first column having X as the plot designation
		ui->cbXColumn->setCurrentIndex(ui->cbXColumn->findText(xColumnName));

		//for the remaining columns, show the names in the comboboxes for the Y-data
		//TODO: handle columns with error-designations
		int yColumnIndex = 1; //the index of the first Y-data comboBox in m_columnComboBoxes
		for (const QString& name : columnNames) {
			if (name != xColumnName) {
				QComboBox* comboBox = m_columnComboBoxes[yColumnIndex];
				comboBox->setCurrentIndex(comboBox->findText(name));
				yColumnIndex++;
			}
		}
	} else {
		//no column with "x plot designation" is selected, simply show all columns in the order they were selected.
		//first selected column will serve as the x-column.
		int yColumnIndex = 0;
		for (const QString& name : columnNames) {
			QComboBox* comboBox = m_columnComboBoxes[yColumnIndex];
			comboBox->setCurrentIndex(comboBox->findText(name));
			yColumnIndex++;
		}
	}
}

void PlotDataDialog::processColumnsForHistogram(const QStringList& columnNames) {
	ui->gbData->setTitle(i18n("Histogram Data"));
	ui->line->hide();
	ui->spacer->changeSize(0, 0);
	ui->chkCreateDataCurve->hide();

	//use the already available cbXColumn combo box
	ui->lXColumn->setText(i18n("Data"));
	m_columnComboBoxes << ui->cbXColumn;
	ui->cbXColumn->addItems(columnNames);
	ui->cbXColumn->setCurrentIndex(0);

	if (m_columns.size() == 1) {
		//one column provided, only one histogram is possible
		//-> hide the curve placement options and the scroll areas for further columns
		ui->lYColumn->hide();
		ui->cbYColumn->hide();
		ui->rbCurvePlacement1->setChecked(true);
		ui->gbCurvePlacement->hide();
		ui->gbPlotPlacement->setTitle(i18n("Add Histogram to"));
		ui->scrollAreaColumns->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	} else {
		ui->gbCurvePlacement->setTitle(i18n("Histogram Placement"));
		ui->rbCurvePlacement1->setText(i18n("All histograms in one plot"));
		ui->rbCurvePlacement2->setText(i18n("One plot per histogram"));
		ui->gbPlotPlacement->setTitle(i18n("Add Histograms to"));
		ui->scrollAreaColumns->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);

		//use the already available cbYColumn combo box
		ui->lYColumn->setText(i18n("Data"));
		m_columnComboBoxes << ui->cbYColumn;
		ui->cbYColumn->addItems(columnNames);
		ui->cbYColumn->setCurrentIndex(1);

		//add a ComboBox for every further column to be plotted
		auto* gridLayout = dynamic_cast<QGridLayout*>(ui->scrollAreaColumns->widget()->layout());
		for (int i = 2; i < m_columns.size(); ++i) {
			auto* label = new QLabel(i18n("Data"));
			auto* comboBox = new QComboBox();
			gridLayout->addWidget(label, i+1, 0, 1, 1);
			gridLayout->addWidget(comboBox, i+1, 2, 1, 1);
			comboBox->addItems(columnNames);
			comboBox->setCurrentIndex(i);
			m_columnComboBoxes << comboBox;
		}
	}
}

void PlotDataDialog::plot() {
	DEBUG("PlotDataDialog::plot()");
	WAIT_CURSOR;
	m_spreadsheet->project()->setSuppressAspectAddedSignal(true);
	m_lastAddedCurve = nullptr;

	if (ui->rbPlotPlacement1->isChecked()) {
		//add curves to an existing plot
		auto* aspect = static_cast<AbstractAspect*>(cbExistingPlots->currentModelIndex().internalPointer());
		auto* plot = static_cast<CartesianPlot*>(aspect);
		plot->beginMacro( i18n("Plot data from %1", m_spreadsheet->name()) );
		addCurvesToPlot(plot);
		plot->endMacro();
	} else if (ui->rbPlotPlacement2->isChecked()) {
		//add curves to a new plot in an existing worksheet
		auto* aspect = static_cast<AbstractAspect*>(cbExistingWorksheets->currentModelIndex().internalPointer());
		auto* worksheet = static_cast<Worksheet*>(aspect);
		worksheet->beginMacro( i18n("Plot data from %1", m_spreadsheet->name()) );

		if (ui->rbCurvePlacement1->isChecked()) {
			//all curves in one plot
			CartesianPlot* plot = new CartesianPlot( i18n("Plot data from %1", m_spreadsheet->name()) );
			plot->setType(CartesianPlot::Type::FourAxes);

			//set the axis titles before we add the plot to the worksheet
			//set the x-axis names
			const QString& xColumnName = ui->cbXColumn->currentText();
			for (auto* axis : plot->children<Axis>()) {
				if (axis->orientation() == Axis::Orientation::Horizontal) {
					axis->title()->setText(xColumnName);
					break;
				}
			}

			//if we only have one single y-column to plot, we can set the title of the y-axes
			if (m_columnComboBoxes.size() == 2) {
				const QString& yColumnName = m_columnComboBoxes[1]->currentText();
				for (auto* axis : plot->children<Axis>()) {
					if (axis->orientation() == Axis::Orientation::Vertical) {
						axis->title()->setText(yColumnName);
						break;
					}
				}
			}

			worksheet->addChild(plot);
			addCurvesToPlot(plot);
		} else {
			//one plot per curve
			addCurvesToPlots(worksheet);
		}
		worksheet->endMacro();
	} else {
		//add curves to a new plot(s) in a new worksheet
		AbstractAspect* parent = m_spreadsheet->parentAspect();
		if (parent->type() == AspectType::DatapickerCurve)
			parent = parent->parentAspect()->parentAspect();
		else if (parent->type() == AspectType::Workbook)
			parent = parent->parentAspect();
#ifdef HAVE_MQTT
		else if (dynamic_cast<MQTTTopic*>(m_spreadsheet))
			parent = m_spreadsheet->project();
#endif
		parent->beginMacro( i18n("Plot data from %1", m_spreadsheet->name()) );
		Worksheet* worksheet = new Worksheet(i18n("Plot data from %1", m_spreadsheet->name()));
		parent->addChild(worksheet);

		if (ui->rbCurvePlacement1->isChecked()) {
			//all curves in one plot
			CartesianPlot* plot = new CartesianPlot( i18n("Plot data from %1", m_spreadsheet->name()) );
			plot->setType(CartesianPlot::Type::FourAxes);

			//set the axis titles before we add the plot to the worksheet
			//set the x-axis names
			const QString& xColumnName = ui->cbXColumn->currentText();
			for (auto* axis : plot->children<Axis>()) {
				if (axis->orientation() == Axis::Orientation::Horizontal) {
					axis->title()->setText(xColumnName);
					break;
				}
			}

			//if we only have one single y-column to plot, we can set the title of the y-axes
			if (m_columnComboBoxes.size() == 2) {
				const QString& yColumnName = m_columnComboBoxes[1]->currentText();
				for (auto* axis : plot->children<Axis>()) {
					if (axis->orientation() == Axis::Orientation::Vertical) {
						axis->title()->setText(yColumnName);
						break;
					}
				}
			}

			worksheet->addChild(plot);
			addCurvesToPlot(plot);
		} else {
			//one plot per curve
			addCurvesToPlots(worksheet);
		}

		parent->endMacro();
	}

	//if more than two curves are produced, select the parent plot of the last added curve in the project explorer.
	//if a custom fit is being done, select the last created fit curve independent of the number of created curves.
	m_spreadsheet->project()->setSuppressAspectAddedSignal(false);
	if (m_lastAddedCurve) {
		QString path;
		if (!m_analysisMode) {
			if (m_columns.size() > 2)
				path = m_lastAddedCurve->parentAspect()->path();
			else
				path = m_lastAddedCurve->path();
		} else {
			if (m_analysisAction == AnalysisAction::FitCustom)
				path = m_lastAddedCurve->path();
			else {
				if (m_columns.size() > 2)
					path = m_lastAddedCurve->parentAspect()->path();
				else
					path = m_lastAddedCurve->path();
			}
		}

		emit m_spreadsheet->project()->requestNavigateTo(path);
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

			//if only one column was selected, allow to use this column for x and for y.
			//otherwise, don't assign xColumn to y
			if (m_columns.size() > 1 && yColumn == xColumn)
				continue;

			addCurve(name, xColumn, yColumn, plot);
		}
		break;
	}
	case PlotType::Histogram: {
		for (auto* comboBox : m_columnComboBoxes) {
			const QString& name = comboBox->currentText();
			Column* column = columnFromName(name);
			addHistogram(name, column, plot);
		}
		break;
	}
	}

	plot->dataChanged();
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

			CartesianPlot* plot = new CartesianPlot(i18n("Plot %1", name));
			plot->setType(CartesianPlot::Type::FourAxes);

			//set the axis names in the new plot
			bool xSet = false;
			bool ySet = false;
			for (auto* axis : plot->children<Axis>()) {
				if (axis->orientation() == Axis::Orientation::Horizontal && !xSet) {
					axis->title()->setText(xColumnName);
					xSet = true;
				} else if (axis->orientation() == Axis::Orientation::Vertical && !ySet) {
					axis->title()->setText(name);
					ySet = true;
				}
			}

			worksheet->addChild(plot);
			addCurve(name, xColumn, yColumn, plot);
			plot->scaleAuto();
		}
		break;
	}
	case PlotType::Histogram: {
		for (auto* comboBox : m_columnComboBoxes) {
			const QString& name = comboBox->currentText();
			Column* column = columnFromName(name);

			CartesianPlot* plot = new CartesianPlot(i18n("Plot %1", name));
			plot->setType(CartesianPlot::Type::FourAxes);

			//set the axis names in the new plot
			bool xSet = false;
			for (auto* axis : plot->children<Axis>()) {
				if (axis->orientation() == Axis::Orientation::Horizontal && !xSet) {
					axis->title()->setText(name);
					xSet = true;
				}
			}

			worksheet->addChild(plot);
			addHistogram(name, column, plot);
			plot->scaleAuto();
		}
	}
	}

	worksheet->setSuppressLayoutUpdate(false);
	worksheet->updateLayout();
}

/*!
 * helper function that does the actual creation of the curve and adding it as child to the \c plot.
 */
void PlotDataDialog::addCurve(const QString& name, Column* xColumn, Column* yColumn, CartesianPlot* plot) {
	DEBUG("PlotDataDialog::addCurve()");
	if (!m_analysisMode) {
		auto* curve = new XYCurve(name);
		curve->suppressRetransform(true);
		curve->setXColumn(xColumn);
		curve->setYColumn(yColumn);
		curve->suppressRetransform(false);
		plot->addChild(curve);
		m_lastAddedCurve = curve;
	} else {
		bool createDataCurve = ui->chkCreateDataCurve->isChecked();
		XYCurve* curve = nullptr;
		if (createDataCurve) {
			curve = new XYCurve(name);
			curve->suppressRetransform(true);
			curve->setXColumn(xColumn);
			curve->setYColumn(yColumn);
			curve->suppressRetransform(false);
			plot->addChild(curve);
			m_lastAddedCurve = curve;
		}

		XYAnalysisCurve* analysisCurve = nullptr;
		switch (m_analysisAction) {
			case AnalysisAction::DataReduction:
				analysisCurve = new XYDataReductionCurve(i18n("Reduction of '%1'", name));
				break;
			case AnalysisAction::Differentiation:
				analysisCurve = new XYDifferentiationCurve(i18n("Derivative of '%1'", name));
				break;
			case AnalysisAction::Integration:
				analysisCurve = new XYIntegrationCurve(i18n("Integral of '%1'", name));
				break;
			case AnalysisAction::Interpolation:
				analysisCurve = new XYInterpolationCurve(i18n("Interpolation of '%1'", name));
				break;
			case AnalysisAction::Smoothing:
				analysisCurve = new XYSmoothCurve(i18n("Smoothing of '%1'", name));
				break;
			case AnalysisAction::FitLinear:
			case AnalysisAction::FitPower:
			case AnalysisAction::FitExp1:
			case AnalysisAction::FitExp2:
			case AnalysisAction::FitInvExp:
			case AnalysisAction::FitGauss:
			case AnalysisAction::FitCauchyLorentz:
			case AnalysisAction::FitTan:
			case AnalysisAction::FitTanh:
			case AnalysisAction::FitErrFunc:
			case AnalysisAction::FitCustom:
				analysisCurve = new XYFitCurve(i18n("Fit to '%1'", name));
				static_cast<XYFitCurve*>(analysisCurve)->initFitData(m_analysisAction);
				static_cast<XYFitCurve*>(analysisCurve)->initStartValues(curve);
				break;
			case AnalysisAction::FourierFilter:
				analysisCurve = new XYFourierFilterCurve(i18n("Fourier Filter of '%1'", name));
				break;
		}

		if (analysisCurve != nullptr) {
			analysisCurve->suppressRetransform(true);
			analysisCurve->setXDataColumn(xColumn);
			analysisCurve->setYDataColumn(yColumn);
			if (m_analysisAction != AnalysisAction::FitCustom) //no custom fit-model set yet, no need to recalculate
				analysisCurve->recalculate();
			analysisCurve->suppressRetransform(false);
			plot->addChild(analysisCurve);
			m_lastAddedCurve = analysisCurve;
		}
	}
}

void PlotDataDialog::addHistogram(const QString& name, Column* column, CartesianPlot* plot) {
	auto* hist = new Histogram(name);
	plot->addChild(hist);
// 	hist->suppressRetransform(true);
	hist->setDataColumn(column);
// 	hist->suppressRetransform(false);
	m_lastAddedCurve = hist;
}

//################################################################
//########################## Slots ###############################
//################################################################
void PlotDataDialog::curvePlacementChanged() {
	if (ui->rbCurvePlacement1->isChecked()) {
		ui->rbPlotPlacement1->setEnabled(true);
		ui->rbPlotPlacement2->setText(i18n("new plot in an existing worksheet"));
		ui->rbPlotPlacement3->setText(i18n("new plot in a new worksheet"));
	} else {
		ui->rbPlotPlacement1->setEnabled(false);
		if (ui->rbPlotPlacement1->isChecked())
			ui->rbPlotPlacement2->setChecked(true);
		ui->rbPlotPlacement2->setText(i18n("new plots in an existing worksheet"));
		ui->rbPlotPlacement3->setText(i18n("new plots in a new worksheet"));
	}
}

void PlotDataDialog::plotPlacementChanged() {
	if (ui->rbPlotPlacement1->isChecked()) {
		cbExistingPlots->setEnabled(true);
		cbExistingWorksheets->setEnabled(false);
	} else if (ui->rbPlotPlacement2->isChecked()) {
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
	if ( (m_plotType == PlotType::XYCurve && (ui->cbXColumn->currentIndex() == -1 || ui->cbYColumn->currentIndex() == -1)) ||
		(m_plotType == PlotType::Histogram && ui->cbXColumn->currentIndex() == -1) )
		msg = i18n("No data selected to plot.");
	else if (ui->rbPlotPlacement1->isChecked()) {
		AbstractAspect* aspect = static_cast<AbstractAspect*>(cbExistingPlots->currentModelIndex().internalPointer());
		enable = (aspect != nullptr);
		if (!enable)
			msg = i18n("An already existing plot has to be selected.");
	} else if (ui->rbPlotPlacement2->isChecked()) {
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
