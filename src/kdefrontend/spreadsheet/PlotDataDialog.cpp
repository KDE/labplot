/***************************************************************************
    File                 : PlotDataDialog.cpp
    Project              : LabPlot
    Description          : Dialog for generating plots for the spreadsheet data
    --------------------------------------------------------------------
    Copyright            : (C) 2017 by Alexander Semke (alexander.semke@web.de)

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
#include "backend/spreadsheet/Spreadsheet.h"
#include "backend/worksheet/plots/cartesian/XYCurve.h"
#include "backend/worksheet/plots/cartesian/CartesianPlot.h"
#include "backend/worksheet/Worksheet.h"
#include "commonfrontend/spreadsheet/SpreadsheetView.h"
#include "commonfrontend/widgets/TreeViewComboBox.h"

#include <QDialogButtonBox>
#include <QPushButton>

#include <KConfigGroup>
#include <KSharedConfig>
#include <KWindowConfig>

/*!
	\class PlotDataDialog
	\brief Dialog for generating plots for the spreadsheet data.

	\ingroup kdefrontend
 */
PlotDataDialog::PlotDataDialog(Spreadsheet* s, QWidget* parent, Qt::WFlags fl) : QDialog(parent, fl),
	m_spreadsheet(s),
	m_plotsModel(new AspectTreeModel(m_spreadsheet->project())),
	m_worksheetsModel(new AspectTreeModel(m_spreadsheet->project())) {

	setWindowTitle(i18n("Plot spreadsheet data"));
	setWindowIcon(QIcon::fromTheme("office-chart-line"));

	QWidget* mainWidget = new QWidget(this);
	ui.setupUi(mainWidget);

	QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
	okButton = buttonBox->button(QDialogButtonBox::Ok);
	okButton->setDefault(true);
	okButton->setToolTip(i18n("Plot the selected data"));
	okButton->setText(i18n("&Plot"));

	QVBoxLayout* layout = new QVBoxLayout(this);
	layout->addWidget(mainWidget);
	layout->addWidget(buttonBox);
	setLayout(layout);

	//create combox boxes for the existing plots and worksheets
	QGridLayout* gridLayout = dynamic_cast<QGridLayout*>(ui.gbPlotPlacement->layout());
	cbExistingPlots = new TreeViewComboBox(ui.gbPlotPlacement);
	cbExistingPlots->setMinimumWidth(250);//TODO: use proper sizeHint in TreeViewComboBox
	gridLayout->addWidget(cbExistingPlots, 0, 1, 1, 1);

	cbExistingWorksheets = new TreeViewComboBox(ui.gbPlotPlacement);
	cbExistingWorksheets->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred));
	gridLayout->addWidget(cbExistingWorksheets, 1, 1, 1, 1);
	
	QList<const char*>  list;
	list<<"Folder"<<"Worksheet"<<"CartesianPlot";
	cbExistingPlots->setTopLevelClasses(list);
	list.clear();
	list<<"CartesianPlot";
	m_plotsModel->setSelectableAspects(list);
	cbExistingPlots->setModel(m_plotsModel);

	list.clear();
	list<<"Folder"<<"Worksheet";
	cbExistingWorksheets->setTopLevelClasses(list);
	list.clear();
	list<<"Worksheet";
	m_worksheetsModel->setSelectableAspects(list);
	cbExistingWorksheets->setModel(m_worksheetsModel);

	//SIGNALs/SLOTs
	connect(buttonBox, SIGNAL(accepted()), this, SLOT(plot()));
	connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
	connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
	connect(ui.rbCurvePlacement1, SIGNAL(toggled(bool)), this, SLOT(curvePlacementChanged()));
	connect(ui.rbCurvePlacement2, SIGNAL(toggled(bool)), this, SLOT(curvePlacementChanged()));
	connect(ui.rbPlotPlacement1, SIGNAL(toggled(bool)), this, SLOT(plotPlacementChanged()));
	connect(ui.rbPlotPlacement2, SIGNAL(toggled(bool)), this, SLOT(plotPlacementChanged()));
	connect(ui.rbPlotPlacement3, SIGNAL(toggled(bool)), this, SLOT(plotPlacementChanged()));
	connect(cbExistingPlots, SIGNAL(currentModelIndexChanged(QModelIndex)), this, SLOT(checkOkButton()));
	connect(cbExistingWorksheets, SIGNAL(currentModelIndexChanged(QModelIndex)), this, SLOT(checkOkButton()));

	//restore saved settings if available
	const KConfigGroup conf(KSharedConfig::openConfig(), "PlotDataDialog");
	if (conf.exists()) {
		int index = conf.readEntry("CurvePlacement", 0);
		if (index == 2) ui.rbCurvePlacement2->setChecked(true);

		index = conf.readEntry("PlotPlacement", 0);
		if (index == 2) ui.rbPlotPlacement2->setChecked(true);
		if (index == 3) ui.rbPlotPlacement3->setChecked(true);
		plotPlacementChanged();

		KWindowConfig::restoreWindowSize(windowHandle(), conf);
	} else {
		resize( QSize(500,0).expandedTo(minimumSize()) );
	}

	this->processColumns();
}

PlotDataDialog::~PlotDataDialog() {
	//save current settings
	KConfigGroup conf(KSharedConfig::openConfig(), "PlotDataDialog");
	int index = 0;
	if (ui.rbCurvePlacement1->isChecked()) index = 1;
	if (ui.rbCurvePlacement2->isChecked()) index = 2;
	conf.writeEntry("CurvePlacement", index);

	if (ui.rbPlotPlacement1->isChecked()) index = 1;
	if (ui.rbPlotPlacement2->isChecked()) index = 2;
	if (ui.rbPlotPlacement3->isChecked()) index = 3;
	conf.writeEntry("PlotPlacement", index);

	KWindowConfig::saveWindowSize(windowHandle(), conf);

	delete m_plotsModel;
	delete m_worksheetsModel;
}

void PlotDataDialog::processColumns() {
	//columns to plot
	SpreadsheetView* view = reinterpret_cast<SpreadsheetView*>(m_spreadsheet->view());
	m_columns = view->selectedColumns();

	//use all spreadsheet columns if no columns are selected
	if (!m_columns.size()) {
		m_columns = m_spreadsheet->children<Column>();
		
		//disable everything if the spreadsheet doesn't have any columns
		if (!m_columns.size()) {
			ui.gbData->setEnabled(false);
			ui.gbCurvePlacement->setEnabled(false);
			ui.gbPlotPlacement->setEnabled(false);
			return;
		}
	}

	m_columnComboBoxes << ui.cbXColumn;
	m_columnComboBoxes << ui.cbYColumn;

	//ui-widget only has one combobox for the y-data -> add additional comboboxes dynamically if required
	if (m_columns.size()>2) {
		QGridLayout* gridLayout = dynamic_cast<QGridLayout*>(ui.gbData->layout());
		for (int i = 2; i < m_columns.size(); ++i) {
			QLabel* label = new QLabel(i18n("Y-data"));
			QComboBox* comboBox = new QComboBox();
			gridLayout->addWidget(label, i+1, 0, 1, 1);
			gridLayout->addWidget(comboBox, i+1, 2, 1, 1);
			m_columnComboBoxes << comboBox;
		}
	} else {
		//two columns provided, only one curve is possible -> hide the curve placement options
		ui.gbCurvePlacement->hide();
		ui.gbPlotPlacement->setTitle(i18n("Add curve to"));
	}

	//determine the column names and the name of the first column having "X" as the plot designation
	QList<QString> columnNames;
	QString xColumnName;
	foreach(const Column* column, m_columns) {
		columnNames << column->name();
		if (xColumnName.isEmpty() && column->plotDesignation() == AbstractColumn::X)
			xColumnName = column->name();
	}

	//show all selected/available column names in the data comboboxes
	foreach(QComboBox* comboBox, m_columnComboBoxes)
		comboBox->addItems(columnNames);

	//show in the X-data combobox the first column having X as the plot designation
	ui.cbXColumn->setCurrentIndex(ui.cbXColumn->findText(xColumnName));
	
	//for the remaining columns, show the names in the comboboxes for the Y-data
	//TODO: handle columns with error-designations
	int yColumnIndex = 1; //the index of the first Y-data comboBox in m_columnComboBoxes
	foreach(const QString name, columnNames) {
		if (name != xColumnName) {
			QComboBox* comboBox = m_columnComboBoxes[yColumnIndex];
			comboBox->setCurrentIndex(comboBox->findText(name));
			yColumnIndex++;
		}
	}
}

void PlotDataDialog::plot() {
	if (ui.rbPlotPlacement1->isChecked()) {
		//add curves to an existing plot
		AbstractAspect* aspect = static_cast<AbstractAspect*>(cbExistingPlots->currentModelIndex().internalPointer());
		CartesianPlot* plot = dynamic_cast<CartesianPlot*>(aspect);
		plot->beginMacro( i18n("Plot data from %1", m_spreadsheet->name()) );
		addCurvesToPlot(plot);
		plot->endMacro();
	} else if (ui.rbPlotPlacement2->isChecked()) {
		//add curves to a new plot in an existing worksheet
		AbstractAspect* aspect = static_cast<AbstractAspect*>(cbExistingWorksheets->currentModelIndex().internalPointer());
		Worksheet* worksheet = dynamic_cast<Worksheet*>(aspect);
		worksheet->beginMacro( i18n("Plot data from %1", m_spreadsheet->name()) );

		if (ui.rbCurvePlacement1->isChecked()) {
			//all curves in one plot
			CartesianPlot* plot = new CartesianPlot( i18n("Plot data from %1", m_spreadsheet->name()) );
			plot->initDefault(CartesianPlot::FourAxes);
			worksheet->addChild(plot);
			addCurvesToPlot(plot);
		} else {
			//one plot per curve
			addCurvesToPlots(worksheet);
		}
		worksheet->endMacro();
	} else {
		//add curves to a new plot(s) in a new worksheet
		Project* project = m_spreadsheet->project();
		project->beginMacro( i18n("Plot data from %1", m_spreadsheet->name()) );
		Worksheet* worksheet = new Worksheet(0, i18n("Plot data from %1", m_spreadsheet->name()));
		project->addChild(worksheet);

		if (ui.rbCurvePlacement1->isChecked()) {
			//all curves in one plot
			CartesianPlot* plot = new CartesianPlot( i18n("Plot data from %1", m_spreadsheet->name()) );
			plot->initDefault(CartesianPlot::FourAxes);
			worksheet->addChild(plot);
			addCurvesToPlot(plot);
		} else {
			//one plot per curve
			addCurvesToPlots(worksheet);
		}
		project->endMacro();
	}
}

Column* PlotDataDialog::columnFromName(const QString& name) const {
	foreach(Column* column, m_columns) {
		if (column->name() == name)
			return column;
	}
	return 0;
}

void PlotDataDialog::addCurvesToPlot(CartesianPlot* plot) const {
	Column* xColumn = columnFromName(ui.cbXColumn->currentText());
	for (int i=1; i<m_columnComboBoxes.size(); ++i) {
		QComboBox* comboBox = m_columnComboBoxes[i];
		Column* yColumn = columnFromName(comboBox->currentText());
		XYCurve* curve = new XYCurve(comboBox->currentText());
		curve->setXColumn(xColumn);
		curve->setYColumn(yColumn);
		plot->addChild(curve);
	}
	plot->scaleAuto();
}

void PlotDataDialog::addCurvesToPlots(Worksheet* worksheet) const {
	Column* xColumn = columnFromName(ui.cbXColumn->currentText());
	for (int i=1; i<m_columnComboBoxes.size(); ++i) {
		QComboBox* comboBox = m_columnComboBoxes[i];
		const QString& name = comboBox->currentText();
		Column* yColumn = columnFromName(name);

		XYCurve* curve = new XYCurve(name);
		curve->setXColumn(xColumn);
		curve->setYColumn(yColumn);

		CartesianPlot* plot = new CartesianPlot(i18n("Plot %1", name));
		plot->initDefault(CartesianPlot::FourAxes);
		plot->addChild(curve);
		worksheet->addChild(plot);
		plot->scaleAuto();
	}
}

//################################################################
//########################## Slots ###############################
//################################################################
void PlotDataDialog::curvePlacementChanged() {
	if (ui.rbCurvePlacement1->isChecked()){
		ui.rbPlotPlacement1->setEnabled(true);
		ui.rbPlotPlacement2->setText(i18n("new plot in an existing worksheet"));
		ui.rbPlotPlacement3->setText(i18n("new plot in a new worksheet"));
	} else {
		ui.rbPlotPlacement1->setEnabled(false);
		if (ui.rbPlotPlacement1->isChecked())
			ui.rbPlotPlacement2->setChecked(true);
		ui.rbPlotPlacement2->setText(i18n("new plots in an existing worksheet"));
		ui.rbPlotPlacement3->setText(i18n("new plots in a new worksheet"));
	}
}

void PlotDataDialog::plotPlacementChanged() {
	if (ui.rbPlotPlacement1->isChecked()) {
		cbExistingPlots->setEnabled(true);
		cbExistingWorksheets->setEnabled(false);
	} else if (ui.rbPlotPlacement2->isChecked()) {
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
	if (ui.rbPlotPlacement1->isChecked()) {
		AbstractAspect* aspect = static_cast<AbstractAspect*>(cbExistingPlots->currentModelIndex().internalPointer());
		enable = (aspect!=NULL);
	} else if (ui.rbPlotPlacement2->isChecked()) {
		AbstractAspect* aspect = static_cast<AbstractAspect*>(cbExistingWorksheets->currentModelIndex().internalPointer());
		enable = (aspect!=NULL);
	} else {
		enable = true;
	}

	okButton->setEnabled(enable);
}
