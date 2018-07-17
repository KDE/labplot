/***************************************************************************
    File                 : SpreadsheetView.cpp
    Project              : LabPlot
    Description          : View class for Spreadsheet
    --------------------------------------------------------------------
    Copyright            : (C) 2011-2017 by Alexander Semke (alexander.semke@web.de)
						   (C) 2016      by Fabian Kristof (fkristofszabolcs@gmail.com)

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

#include "SpreadsheetView.h"
#include "backend/spreadsheet/SpreadsheetModel.h"
#include "backend/spreadsheet/Spreadsheet.h"
#include "commonfrontend/spreadsheet/SpreadsheetItemDelegate.h"
#include "commonfrontend/spreadsheet/SpreadsheetHeaderView.h"
#include "backend/datasources/filters/FITSFilter.h"
#include "backend/lib/macros.h"
#include "backend/lib/trace.h"
#include "backend/core/column/Column.h"
#include "backend/core/column/ColumnPrivate.h"
#include "backend/core/datatypes/SimpleCopyThroughFilter.h"
#include "backend/core/datatypes/Double2StringFilter.h"
#include "backend/core/datatypes/String2DoubleFilter.h"
#include "backend/core/datatypes/DateTime2StringFilter.h"
#include "backend/core/datatypes/String2DateTimeFilter.h"

#include <QKeyEvent>
#include <QClipboard>
#include <QInputDialog>
#include <QDate>
#include <QApplication>
#include <QMenu>
#include <QMimeData>
#include <QPainter>
#include <QPrinter>
#include <QPrintDialog>
#include <QPrintPreviewDialog>
#include <QTableView>
#include <QToolBar>
#include <QTextStream>
#include <QProcess>

#include <KConfigGroup>
#include <KLocalizedString>
#include <KSharedConfig>

#include "kdefrontend/spreadsheet/ExportSpreadsheetDialog.h"
#include "kdefrontend/spreadsheet/PlotDataDialog.h"
#include "kdefrontend/spreadsheet/DropValuesDialog.h"
#include "kdefrontend/spreadsheet/SortDialog.h"
#include "kdefrontend/spreadsheet/RandomValuesDialog.h"
#include "kdefrontend/spreadsheet/EquidistantValuesDialog.h"
#include "kdefrontend/spreadsheet/FunctionValuesDialog.h"
#include "kdefrontend/spreadsheet/StatisticsDialog.h"

#include <algorithm> //for std::reverse

/*!
	\class SpreadsheetView
	\brief View class for Spreadsheet

	\ingroup commonfrontend
 */
SpreadsheetView::SpreadsheetView(Spreadsheet* spreadsheet, bool readOnly) : QWidget(),
	m_tableView(new QTableView(this)),
	m_spreadsheet(spreadsheet),
	m_model( new SpreadsheetModel(spreadsheet) ),
	m_suppressSelectionChangedEvent(false),
	m_readOnly(readOnly),
	m_columnSetAsMenu(nullptr),
	m_columnGenerateDataMenu(nullptr),
	m_columnSortMenu(nullptr) {

	QHBoxLayout* layout = new QHBoxLayout(this);
	layout->setContentsMargins(0,0,0,0);
	layout->addWidget(m_tableView);
	if (m_readOnly)
		m_tableView->setEditTriggers(QTableView::NoEditTriggers);
	init();

	//resize the view to show alls columns and the first 50 rows.
	//no need to resize the view when the project is being opened,
	//all views will be resized to the stored values at the end
	if (!m_spreadsheet->isLoading()) {
		int w = m_tableView->verticalHeader()->width();
		int h = m_horizontalHeader->height();
		for (int i = 0; i < m_horizontalHeader->count(); ++i)
			w += m_horizontalHeader->sectionSize(i);

		if (m_tableView->verticalHeader()->count() > 50 || m_tableView->verticalHeader()->count() < 10)
			h += m_tableView->verticalHeader()->sectionSize(0)*50;
		else
			h += m_tableView->verticalHeader()->sectionSize(0)*m_tableView->verticalHeader()->count();

		resize(w+50, h);
	}

	KConfigGroup group = KSharedConfig::openConfig()->group(QLatin1String("Spreadsheet"));
	showComments(group.readEntry(QLatin1String("ShowComments"), false));
}

SpreadsheetView::~SpreadsheetView() {
	delete m_model;
}

void SpreadsheetView::init() {
	initActions();
	initMenus();

	m_tableView->setModel(m_model);
	m_tableView->setItemDelegate(new SpreadsheetItemDelegate(this));
	m_tableView->setSelectionMode(QAbstractItemView::ExtendedSelection);

	//horizontal header
	m_horizontalHeader = new SpreadsheetHeaderView(this);
	m_horizontalHeader->setSectionsClickable(true);
	m_horizontalHeader->setHighlightSections(true);
	m_tableView->setHorizontalHeader(m_horizontalHeader);
	m_horizontalHeader->setSectionsMovable(true);
	m_horizontalHeader->installEventFilter(this);

	resizeHeader();

	connect(m_horizontalHeader, SIGNAL(sectionMoved(int,int,int)), this, SLOT(handleHorizontalSectionMoved(int,int,int)));
	connect(m_horizontalHeader, SIGNAL(sectionDoubleClicked(int)), this, SLOT(handleHorizontalHeaderDoubleClicked(int)));
	connect(m_horizontalHeader, SIGNAL(sectionResized(int,int,int)), this, SLOT(handleHorizontalSectionResized(int,int,int)));
	connect(m_horizontalHeader, SIGNAL(sectionClicked(int)), this, SLOT(columnClicked(int)) );

	// vertical header
	QHeaderView* v_header = m_tableView->verticalHeader();
	v_header->setSectionResizeMode(QHeaderView::Fixed);
	v_header->setSectionsMovable(false);
	v_header->installEventFilter(this);

	setFocusPolicy(Qt::StrongFocus);
	setFocus();
	installEventFilter(this);
	connectActions();
	showComments(false);

	connect(m_model, SIGNAL(headerDataChanged(Qt::Orientation,int,int)), this,
	        SLOT(updateHeaderGeometry(Qt::Orientation,int,int)) );
	connect(m_model, SIGNAL(headerDataChanged(Qt::Orientation,int,int)), this,
	        SLOT(handleHeaderDataChanged(Qt::Orientation,int,int)) );
	connect(m_spreadsheet, SIGNAL(aspectAdded(const AbstractAspect*)),
	        this, SLOT(handleAspectAdded(const AbstractAspect*)));
	connect(m_spreadsheet, SIGNAL(aspectAboutToBeRemoved(const AbstractAspect*)),
	        this, SLOT(handleAspectAboutToBeRemoved(const AbstractAspect*)));
	connect(m_spreadsheet, SIGNAL(requestProjectContextMenu(QMenu*)), this, SLOT(createContextMenu(QMenu*)));

	for (auto* column: m_spreadsheet->children<Column>())
		connect(column, SIGNAL(requestProjectContextMenu(QMenu*)), this, SLOT(createColumnContextMenu(QMenu*)));

	//selection relevant connections
	QItemSelectionModel* sel_model = m_tableView->selectionModel();
	connect(sel_model, SIGNAL(currentColumnChanged(QModelIndex,QModelIndex)),
	        this, SLOT(currentColumnChanged(QModelIndex,QModelIndex)));
	connect(sel_model, SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
	        this, SLOT(selectionChanged(QItemSelection,QItemSelection)));
	connect(sel_model, SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
	        this, SLOT(selectionChanged(QItemSelection,QItemSelection)) );

	connect(m_spreadsheet, SIGNAL(columnSelected(int)), this, SLOT(selectColumn(int)) );
	connect(m_spreadsheet, SIGNAL(columnDeselected(int)), this, SLOT(deselectColumn(int)) );
}

/*!
	set the column sizes to the saved values or resize to content if no size was saved yet
*/
void SpreadsheetView::resizeHeader() {
	for (int i = 0; i < m_spreadsheet->children<Column>().size(); ++i) {
		Column* col = m_spreadsheet->child<Column>(i);
		if (col->width() == 0)
			m_tableView->resizeColumnToContents(i);
		else
			m_tableView->setColumnWidth(i, col->width());
	}
}

void SpreadsheetView::initActions() {
	// selection related actions
	action_cut_selection = new QAction(QIcon::fromTheme("edit-cut"), i18n("Cu&t"), this);
	action_copy_selection = new QAction(QIcon::fromTheme("edit-copy"), i18n("&Copy"), this);
	action_paste_into_selection = new QAction(QIcon::fromTheme("edit-paste"), i18n("Past&e"), this);
	action_mask_selection = new QAction(QIcon::fromTheme("edit-node"), i18n("&Mask Selection"), this);
	action_unmask_selection = new QAction(QIcon::fromTheme("format-remove-node"), i18n("&Unmask Selection"), this);
	action_clear_selection = new QAction(QIcon::fromTheme("edit-clear"), i18n("Clea&r Selection"), this);
	action_select_all = new QAction(QIcon::fromTheme("edit-select-all"), i18n("Select All"), this);

// 	action_set_formula = new QAction(QIcon::fromTheme(""), i18n("Assign &Formula"), this);
// 	action_recalculate = new QAction(QIcon::fromTheme(""), i18n("Recalculate"), this);
	action_fill_sel_row_numbers = new QAction(QIcon::fromTheme(""), i18n("Row Numbers"), this);
	action_fill_row_numbers = new QAction(QIcon::fromTheme(""), i18n("Row Numbers"), this);
	action_fill_random = new QAction(QIcon::fromTheme(""), i18n("Uniform Random Values"), this);
	action_fill_random_nonuniform = new QAction(QIcon::fromTheme(""), i18n("Random Values"), this);
	action_fill_equidistant = new QAction(QIcon::fromTheme(""), i18n("Equidistant Values"), this);
	action_fill_function = new QAction(QIcon::fromTheme(""), i18n("Function Values"), this);
	action_fill_const = new QAction(QIcon::fromTheme(""), i18n("Const Values"), this);

	//spreadsheet related actions
	action_toggle_comments = new QAction(QIcon::fromTheme("document-properties"), i18n("Show Comments"), this);
	action_clear_spreadsheet = new QAction(QIcon::fromTheme("edit-clear"), i18n("Clear Spreadsheet"), this);
	action_clear_masks = new QAction(QIcon::fromTheme("format-remove-node"), i18n("Clear Masks"), this);
	action_sort_spreadsheet = new QAction(QIcon::fromTheme("view-sort-ascending"), i18n("&Sort Spreadsheet"), this);
	action_go_to_cell = new QAction(QIcon::fromTheme("go-jump"), i18n("&Go to Cell"), this);
	action_statistics_all_columns = new QAction(QIcon::fromTheme("view-statistics"), i18n("Statisti&cs"), this );

	// column related actions
	action_insert_column_left = new QAction(QIcon::fromTheme("edit-table-insert-column-left"), i18n("Insert Column Left"), this);
	action_insert_column_right = new QAction(QIcon::fromTheme("edit-table-insert-column-right"), i18n("Insert Column Right"), this);
	action_remove_columns = new QAction(QIcon::fromTheme("edit-table-delete-column"), i18n("Remove Selected Columns"), this);
	action_clear_columns = new QAction(QIcon::fromTheme("edit-clear"), i18n("Clear Selected Columns"), this);

	action_set_as_none = new QAction(i18n("None"), this);
	action_set_as_none->setData(AbstractColumn::NoDesignation);

	action_set_as_x = new QAction("X", this);
	action_set_as_x->setData(AbstractColumn::X);

	action_set_as_y = new QAction("Y", this);
	action_set_as_y->setData(AbstractColumn::Y);

	action_set_as_z = new QAction("Z", this);
	action_set_as_z->setData(AbstractColumn::Z);

	action_set_as_xerr = new QAction(i18n("X-error"), this);
	action_set_as_xerr->setData(AbstractColumn::XError);

	action_set_as_xerr_minus = new QAction(i18n("X-error minus"), this);
	action_set_as_xerr_minus->setData(AbstractColumn::XErrorMinus);

	action_set_as_xerr_plus = new QAction(i18n("X-error plus"), this);
	action_set_as_xerr_plus->setData(AbstractColumn::XErrorPlus);

	action_set_as_yerr = new QAction(i18n("Y-error"), this);
	action_set_as_yerr->setData(AbstractColumn::YError);

	action_set_as_yerr_minus = new QAction(i18n("Y-error minus"), this);
	action_set_as_yerr_minus->setData(AbstractColumn::YErrorMinus);

	action_set_as_yerr_plus = new QAction(i18n("Y-error plus"), this);
	action_set_as_yerr_plus->setData(AbstractColumn::YErrorPlus);

	action_reverse_columns = new QAction(QIcon::fromTheme(""), i18n("Reverse"), this);
	action_drop_values = new QAction(QIcon::fromTheme(""), i18n("Drop Values"), this);
	action_mask_values = new QAction(QIcon::fromTheme(""), i18n("Mask Values"), this);
// 	action_join_columns = new QAction(QIcon::fromTheme(""), i18n("Join"), this);
	action_normalize_columns = new QAction(QIcon::fromTheme(""), i18n("&Normalize"), this);
	action_normalize_selection = new QAction(QIcon::fromTheme(""), i18n("&Normalize Selection"), this);
	action_sort_columns = new QAction(QIcon::fromTheme(""), i18n("&Selected Columns"), this);
	action_sort_asc_column = new QAction(QIcon::fromTheme("view-sort-ascending"), i18n("&Ascending"), this);
	action_sort_desc_column = new QAction(QIcon::fromTheme("view-sort-descending"), i18n("&Descending"), this);
	action_statistics_columns = new QAction(QIcon::fromTheme("view-statistics"), i18n("Column Statisti&cs"), this);

	// row related actions
	action_insert_row_above = new QAction(QIcon::fromTheme("edit-table-insert-row-above") ,i18n("Insert Row Above"), this);
	action_insert_row_below = new QAction(QIcon::fromTheme("edit-table-insert-row-below"), i18n("Insert Row Below"), this);
	action_remove_rows = new QAction(QIcon::fromTheme("edit-table-delete-row"), i18n("Remo&ve Selected Rows"), this);
	action_clear_rows = new QAction(QIcon::fromTheme("edit-clear"), i18n("Clea&r Selected Rows"), this);
	action_statistics_rows = new QAction(QIcon::fromTheme("view-statistics"), i18n("Row Statisti&cs"), this);

	//plot data action
	action_plot_data = new QAction(QIcon::fromTheme("office-chart-line"), i18n("Plot Data"), this);

	//Analyze and plot menu actions
	//TODO: no own icons yet
	addDataOperationAction = new QAction(QIcon::fromTheme("labplot-xy-curve"), i18n("Data Operation"), this);
//	addDataOperationAction = new QAction(QIcon::fromTheme("labplot-xy-data-operation-curve"), i18n("Data Operation"), this);
	//TODO: enable when available
	addDataOperationAction->setEnabled(false);
	addDataReductionAction = new QAction(QIcon::fromTheme("labplot-xy-curve"), i18n("Reduce Data"), this);
//	addDataReductionAction = new QAction(QIcon::fromTheme("labplot-xy-data-reduction-curve"), i18n("Reduce Data"), this);
	addDataReductionAction->setData(PlotDataDialog::DataReduction);
	addDifferentiationAction = new QAction(QIcon::fromTheme("labplot-xy-curve"), i18n("Differentiate"), this);
//	addDifferentiationAction = new QAction(QIcon::fromTheme("labplot-xy-differentiation-curve"), i18n("Differentiate"), this);
	addDifferentiationAction->setData(PlotDataDialog::Differentiation);
	addIntegrationAction = new QAction(QIcon::fromTheme("labplot-xy-curve"), i18n("Integrate"), this);
//	addIntegrationAction = new QAction(QIcon::fromTheme("labplot-xy-integration-curve"), i18n("Integrate"), this);
	addIntegrationAction->setData(PlotDataDialog::Integration);
	addInterpolationAction = new QAction(QIcon::fromTheme("labplot-xy-interpolation-curve"), i18n("Interpolate"), this);
	addInterpolationAction->setData(PlotDataDialog::Interpolation);
	addSmoothAction = new QAction(QIcon::fromTheme("labplot-xy-smoothing-curve"), i18n("Smooth"), this);
	addSmoothAction->setData(PlotDataDialog::Smoothing);

	QAction* fitAction = new QAction(QIcon::fromTheme("labplot-xy-fit-curve"), i18n("Linear"), this);
	fitAction->setData(PlotDataDialog::FitLinear);
	addFitAction.append(fitAction);

	fitAction = new QAction(QIcon::fromTheme("labplot-xy-fit-curve"), i18n("Power"), this);
	fitAction->setData(PlotDataDialog::FitPower);
	addFitAction.append(fitAction);

	fitAction = new QAction(QIcon::fromTheme("labplot-xy-fit-curve"), i18n("Exponential (degree 1)"), this);
	fitAction->setData(PlotDataDialog::FitExp1);
	addFitAction.append(fitAction);

	fitAction = new QAction(QIcon::fromTheme("labplot-xy-fit-curve"), i18n("Exponential (degree 2)"), this);
	fitAction->setData(PlotDataDialog::FitExp2);
	addFitAction.append(fitAction);

	fitAction = new QAction(QIcon::fromTheme("labplot-xy-fit-curve"), i18n("Inverse Exponential"), this);
	fitAction->setData(PlotDataDialog::FitInvExp);
	addFitAction.append(fitAction);

	fitAction = new QAction(QIcon::fromTheme("labplot-xy-fit-curve"), i18n("Gauss"), this);
	fitAction->setData(PlotDataDialog::FitGauss);
	addFitAction.append(fitAction);

	fitAction = new QAction(QIcon::fromTheme("labplot-xy-fit-curve"), i18n("Cauchy-Lorentz"), this);
	fitAction->setData(PlotDataDialog::FitCauchyLorentz);
	addFitAction.append(fitAction);

	fitAction = new QAction(QIcon::fromTheme("labplot-xy-fit-curve"), i18n("Arc Tangent"), this);
	fitAction->setData(PlotDataDialog::FitTan);
	addFitAction.append(fitAction);

	fitAction = new QAction(QIcon::fromTheme("labplot-xy-fit-curve"), i18n("Hyperbolic Tangent"), this);
	fitAction->setData(PlotDataDialog::FitTanh);
	addFitAction.append(fitAction);

	fitAction = new QAction(QIcon::fromTheme("labplot-xy-fit-curve"), i18n("Error Function"), this);
	fitAction->setData(PlotDataDialog::FitErrFunc);
	addFitAction.append(fitAction);

	fitAction = new QAction(QIcon::fromTheme("labplot-xy-fit-curve"), i18n("Custom"), this);
	fitAction->setData(PlotDataDialog::FitCustom);
	addFitAction.append(fitAction);

	addFourierFilterAction = new QAction(QIcon::fromTheme("labplot-xy-fourier-filter-curve"), i18n("Fourier Filter"), this);
	addFourierFilterAction->setData(PlotDataDialog::FourierFilter);

	connect(addDataReductionAction, SIGNAL(triggered()), SLOT(plotData()));
	connect(addDifferentiationAction, SIGNAL(triggered()), SLOT(plotData()));
	connect(addIntegrationAction, SIGNAL(triggered()), SLOT(plotData()));
	connect(addInterpolationAction, SIGNAL(triggered()), SLOT(plotData()));
	connect(addSmoothAction, SIGNAL(triggered()), SLOT(plotData()));
	for (const auto& action: addFitAction)
		connect(action, SIGNAL(triggered()), SLOT(plotData()));
	connect(addFourierFilterAction, SIGNAL(triggered()), SLOT(plotData()));
}

void SpreadsheetView::initMenus() {
	//Selection menu
	m_selectionMenu = new QMenu(i18n("Selection"), this);
	QMenu* submenu = nullptr;

	if (!m_readOnly) {
		submenu= new QMenu(i18n("Fi&ll Selection With"), this);
		submenu->addAction(action_fill_sel_row_numbers);
		submenu->addAction(action_fill_const);
		m_selectionMenu->addMenu(submenu);
		m_selectionMenu->addSeparator();
		m_selectionMenu->addAction(action_cut_selection);
	}

	m_selectionMenu->addAction(action_copy_selection);

	if (!m_readOnly) {
		m_selectionMenu->addAction(action_paste_into_selection);
		m_selectionMenu->addAction(action_clear_selection);
		m_selectionMenu->addSeparator();
		m_selectionMenu->addAction(action_mask_selection);
		m_selectionMenu->addAction(action_unmask_selection);
		m_selectionMenu->addSeparator();
		m_selectionMenu->addAction(action_normalize_selection);
	}

	// Column menu
	m_columnMenu = new QMenu(this);
	m_columnMenu->addAction(action_plot_data);

	// Data manipulation sub-menu
	QMenu* dataManipulationMenu = new QMenu(i18n("Data Manipulation"));
	dataManipulationMenu->setIcon(QIcon::fromTheme("zoom-draw"));
	dataManipulationMenu->addAction(addDataOperationAction);
	dataManipulationMenu->addAction(addDataReductionAction);

	// Data fit sub-menu
	QMenu* dataFitMenu = new QMenu(i18n("Fit"));
	dataFitMenu->setIcon(QIcon::fromTheme("labplot-xy-fit-curve"));
	dataFitMenu->addAction(addFitAction.at(0));
	dataFitMenu->addAction(addFitAction.at(1));
	dataFitMenu->addAction(addFitAction.at(2));
	dataFitMenu->addAction(addFitAction.at(3));
	dataFitMenu->addAction(addFitAction.at(4));
	dataFitMenu->addSeparator();
	dataFitMenu->addAction(addFitAction.at(5));
	dataFitMenu->addAction(addFitAction.at(6));
	dataFitMenu->addSeparator();
	dataFitMenu->addAction(addFitAction.at(7));
	dataFitMenu->addAction(addFitAction.at(8));
	dataFitMenu->addAction(addFitAction.at(9));
	dataFitMenu->addSeparator();
	dataFitMenu->addAction(addFitAction.at(10));

	//analyze and plot data menu
	m_analyzePlotMenu = new QMenu(i18n("Analyze and Plot Data"));
	m_analyzePlotMenu->insertMenu(0, dataManipulationMenu);
	m_analyzePlotMenu->addSeparator();
	m_analyzePlotMenu->addAction(addDifferentiationAction);
	m_analyzePlotMenu->addAction(addIntegrationAction);
	m_analyzePlotMenu->addSeparator();
	m_analyzePlotMenu->addAction(addInterpolationAction);
	m_analyzePlotMenu->addAction(addSmoothAction);
	m_analyzePlotMenu->addAction(addFourierFilterAction);
	m_analyzePlotMenu->addSeparator();
	m_analyzePlotMenu->addMenu(dataFitMenu);
	m_columnMenu->addMenu(m_analyzePlotMenu);

	m_columnSetAsMenu = new QMenu(i18n("Set Column As"));
	m_columnMenu->addSeparator();
	m_columnSetAsMenu->addAction(action_set_as_x);
	m_columnSetAsMenu->addAction(action_set_as_y);
	m_columnSetAsMenu->addAction(action_set_as_z);
	m_columnSetAsMenu->addSeparator();
	m_columnSetAsMenu->addAction(action_set_as_xerr);
	m_columnSetAsMenu->addAction(action_set_as_xerr_minus);
	m_columnSetAsMenu->addAction(action_set_as_xerr_plus);
	m_columnSetAsMenu->addSeparator();
	m_columnSetAsMenu->addAction(action_set_as_yerr);
	m_columnSetAsMenu->addAction(action_set_as_yerr_minus);
	m_columnSetAsMenu->addAction(action_set_as_yerr_plus);
	m_columnSetAsMenu->addSeparator();
	m_columnSetAsMenu->addAction(action_set_as_none);
	m_columnMenu->addMenu(m_columnSetAsMenu);

	if (!m_readOnly) {
		m_columnGenerateDataMenu = new QMenu(i18n("Generate Data"), this);
		m_columnGenerateDataMenu->addAction(action_fill_row_numbers);
		m_columnGenerateDataMenu->addAction(action_fill_const);
		m_columnGenerateDataMenu->addAction(action_fill_equidistant);
		m_columnGenerateDataMenu->addAction(action_fill_random_nonuniform);
		m_columnGenerateDataMenu->addAction(action_fill_function);
		m_columnMenu->addSeparator();
		m_columnMenu->addMenu(m_columnGenerateDataMenu);
		m_columnMenu->addSeparator();

		m_columnMenu->addAction(action_reverse_columns);
		m_columnMenu->addAction(action_drop_values);
		m_columnMenu->addAction(action_mask_values);
		// 	m_columnMenu->addAction(action_join_columns);
		m_columnMenu->addAction(action_normalize_columns);

		m_columnSortMenu = new QMenu(i18n("Sort"), this);
		m_columnSortMenu->setIcon(QIcon::fromTheme("view-sort-ascending"));
		m_columnSortMenu->addAction(action_sort_asc_column);
		m_columnSortMenu->addAction(action_sort_desc_column);
		m_columnSortMenu->addAction(action_sort_columns);
		m_columnMenu->addSeparator();
		m_columnMenu->addMenu(m_columnSortMenu);
		m_columnMenu->addSeparator();

		m_columnMenu->addAction(action_insert_column_left);
		m_columnMenu->addAction(action_insert_column_right);
		m_columnMenu->addSeparator();
		m_columnMenu->addAction(action_remove_columns);
		m_columnMenu->addAction(action_clear_columns);
	}
	m_columnMenu->addSeparator();
	m_columnMenu->addAction(action_toggle_comments);
	m_columnMenu->addSeparator();

	m_columnMenu->addAction(action_statistics_columns);
	action_statistics_columns->setVisible(false);

	//Spreadsheet menu
	m_spreadsheetMenu = new QMenu(this);
	m_spreadsheetMenu->addAction(action_plot_data);
	m_spreadsheetMenu->addMenu(m_analyzePlotMenu);
	m_spreadsheetMenu->addSeparator();
	m_spreadsheetMenu->addMenu(m_selectionMenu);
	m_spreadsheetMenu->addSeparator();
	m_spreadsheetMenu->addAction(action_select_all);
	if (!m_readOnly) {
		m_spreadsheetMenu->addAction(action_clear_spreadsheet);
		m_spreadsheetMenu->addAction(action_clear_masks);
		m_spreadsheetMenu->addAction(action_sort_spreadsheet);
	}
	m_spreadsheetMenu->addSeparator();
	m_spreadsheetMenu->addAction(action_go_to_cell);
	m_spreadsheetMenu->addSeparator();
	m_spreadsheetMenu->addAction(action_toggle_comments);
	m_spreadsheetMenu->addSeparator();
	m_spreadsheetMenu->addAction(action_statistics_all_columns);
	action_statistics_all_columns->setVisible(true);

	//Row menu
	m_rowMenu = new QMenu(this);
	if (!m_readOnly) {
		submenu = new QMenu(i18n("Fi&ll Selection With"), this);
		submenu->addAction(action_fill_sel_row_numbers);
		submenu->addAction(action_fill_const);
		m_rowMenu->addMenu(submenu);
		m_rowMenu->addSeparator();

		m_rowMenu->addAction(action_insert_row_above);
		m_rowMenu->addAction(action_insert_row_below);
		m_rowMenu->addSeparator();

		m_rowMenu->addAction(action_remove_rows);
		m_rowMenu->addAction(action_clear_rows);
	}
	m_rowMenu->addSeparator();
	m_rowMenu->addAction(action_statistics_rows);
	action_statistics_rows->setVisible(false);
}

void SpreadsheetView::connectActions() {
	connect(action_cut_selection, SIGNAL(triggered()), this, SLOT(cutSelection()));
	connect(action_copy_selection, SIGNAL(triggered()), this, SLOT(copySelection()));
	connect(action_paste_into_selection, SIGNAL(triggered()), this, SLOT(pasteIntoSelection()));
	connect(action_mask_selection, SIGNAL(triggered()), this, SLOT(maskSelection()));
	connect(action_unmask_selection, SIGNAL(triggered()), this, SLOT(unmaskSelection()));

	connect(action_clear_selection, SIGNAL(triggered()), this, SLOT(clearSelectedCells()));
// 	connect(action_recalculate, SIGNAL(triggered()), this, SLOT(recalculateSelectedCells()));
	connect(action_fill_row_numbers, SIGNAL(triggered()), this, SLOT(fillWithRowNumbers()));
	connect(action_fill_sel_row_numbers, SIGNAL(triggered()), this, SLOT(fillSelectedCellsWithRowNumbers()));
// 	connect(action_fill_random, SIGNAL(triggered()), this, SLOT(fillSelectedCellsWithRandomNumbers()));
	connect(action_fill_random_nonuniform, SIGNAL(triggered()), this, SLOT(fillWithRandomValues()));
	connect(action_fill_equidistant, SIGNAL(triggered()), this, SLOT(fillWithEquidistantValues()));
	connect(action_fill_function, SIGNAL(triggered()), this, SLOT(fillWithFunctionValues()));
	connect(action_fill_const, SIGNAL(triggered()), this, SLOT(fillSelectedCellsWithConstValues()));
	connect(action_select_all, SIGNAL(triggered()), m_tableView, SLOT(selectAll()));
	connect(action_clear_spreadsheet, SIGNAL(triggered()), m_spreadsheet, SLOT(clear()));
	connect(action_clear_masks, SIGNAL(triggered()), m_spreadsheet, SLOT(clearMasks()));
	connect(action_sort_spreadsheet, SIGNAL(triggered()), this, SLOT(sortSpreadsheet()));
	connect(action_go_to_cell, SIGNAL(triggered()), this, SLOT(goToCell()));

	connect(action_insert_column_left, SIGNAL(triggered()), this, SLOT(insertColumnLeft()));
	connect(action_insert_column_right, SIGNAL(triggered()), this, SLOT(insertColumnRight()));
	connect(action_remove_columns, SIGNAL(triggered()), this, SLOT(removeSelectedColumns()));
	connect(action_clear_columns, SIGNAL(triggered()), this, SLOT(clearSelectedColumns()));
	connect(action_set_as_none, SIGNAL(triggered()), this, SLOT(setSelectionAs()));
	connect(action_set_as_x, SIGNAL(triggered()), this, SLOT(setSelectionAs()));
	connect(action_set_as_y, SIGNAL(triggered()), this, SLOT(setSelectionAs()));
	connect(action_set_as_z, SIGNAL(triggered()), this, SLOT(setSelectionAs()));
	connect(action_set_as_xerr, SIGNAL(triggered()), this, SLOT(setSelectionAs()));
	connect(action_set_as_xerr_minus, SIGNAL(triggered()), this, SLOT(setSelectionAs()));
	connect(action_set_as_xerr_plus, SIGNAL(triggered()), this, SLOT(setSelectionAs()));
	connect(action_set_as_yerr, SIGNAL(triggered()), this, SLOT(setSelectionAs()));
	connect(action_set_as_yerr_minus, SIGNAL(triggered()), this, SLOT(setSelectionAs()));
	connect(action_set_as_yerr_plus, SIGNAL(triggered()), this, SLOT(setSelectionAs()));
	connect(action_reverse_columns, SIGNAL(triggered()), this, SLOT(reverseColumns()));
	connect(action_drop_values, SIGNAL(triggered()), this, SLOT(dropColumnValues()));
	connect(action_mask_values, SIGNAL(triggered()), this, SLOT(maskColumnValues()));
// 	connect(action_join_columns, SIGNAL(triggered()), this, SLOT(joinColumns()));
	connect(action_normalize_columns, SIGNAL(triggered()), this, SLOT(normalizeSelectedColumns()));
	connect(action_normalize_selection, SIGNAL(triggered()), this, SLOT(normalizeSelection()));
	connect(action_sort_columns, SIGNAL(triggered()), this, SLOT(sortSelectedColumns()));
	connect(action_sort_asc_column, SIGNAL(triggered()), this, SLOT(sortColumnAscending()));
	connect(action_sort_desc_column, SIGNAL(triggered()), this, SLOT(sortColumnDescending()));
	connect(action_statistics_columns, SIGNAL(triggered()), this, SLOT(showColumnStatistics()));
	connect(action_statistics_all_columns, SIGNAL(triggered()), this, SLOT(showAllColumnsStatistics()));

	connect(action_insert_row_above, SIGNAL(triggered()), this, SLOT(insertRowAbove()));
	connect(action_insert_row_below, SIGNAL(triggered()), this, SLOT(insertRowBelow()));
	connect(action_remove_rows, SIGNAL(triggered()), this, SLOT(removeSelectedRows()));
	connect(action_clear_rows, SIGNAL(triggered()), this, SLOT(clearSelectedRows()));
	connect(action_statistics_rows, SIGNAL(triggered()), this, SLOT(showRowStatistics()));
	connect(action_toggle_comments, SIGNAL(triggered()), this, SLOT(toggleComments()));

	connect(action_plot_data, SIGNAL(triggered()), this, SLOT(plotData()));
}

void SpreadsheetView::fillToolBar(QToolBar* toolBar) {
	if (!m_readOnly) {
		toolBar->addAction(action_insert_row_above);
		toolBar->addAction(action_insert_row_below);
		toolBar->addAction(action_remove_rows);
	}
	toolBar->addAction(action_statistics_rows);
	toolBar->addSeparator();
	if (!m_readOnly) {
		toolBar->addAction(action_insert_column_left);
		toolBar->addAction(action_insert_column_right);
		toolBar->addAction(action_remove_columns);
	}

	toolBar->addAction(action_statistics_columns);
	if (!m_readOnly) {
		toolBar->addSeparator();
		toolBar->addAction(action_sort_asc_column);
		toolBar->addAction(action_sort_desc_column);
	}
}

/*!
 * Populates the menu \c menu with the spreadsheet and spreadsheet view relevant actions.
 * The menu is used
 *   - as the context menu in SpreadsheetView
 *   - as the "spreadsheet menu" in the main menu-bar (called form MainWin)
 *   - as a part of the spreadsheet context menu in project explorer
 */
void SpreadsheetView::createContextMenu(QMenu* menu) {
	Q_ASSERT(menu);

	checkSpreadsheetMenu();

	QAction* firstAction = 0;
	// if we're populating the context menu for the project explorer, then
	//there're already actions available there. Skip the first title-action
	//and insert the action at the beginning of the menu.
	if (menu->actions().size()>1)
		firstAction = menu->actions().at(1);

	if (m_spreadsheet->columnCount() > 0 && m_spreadsheet->rowCount() > 0) {
		menu->insertAction(firstAction, action_plot_data);
		menu->insertSeparator(firstAction);
	}
	menu->insertMenu(firstAction, m_selectionMenu);
	menu->insertSeparator(firstAction);
	menu->insertAction(firstAction, action_select_all);
	if (!m_readOnly) {
		menu->insertAction(firstAction, action_clear_spreadsheet);
		menu->insertAction(firstAction, action_clear_masks);
		menu->insertAction(firstAction, action_sort_spreadsheet);
		menu->insertSeparator(firstAction);
	}

	menu->insertAction(firstAction, action_go_to_cell);
	menu->insertSeparator(firstAction);
	menu->insertAction(firstAction, action_toggle_comments);
	menu->insertSeparator(firstAction);
	menu->insertAction(firstAction, action_statistics_all_columns);
	menu->insertSeparator(firstAction);
}

/*!
 * adds column specific actions in SpreadsheetView to the context menu shown in the project explorer.
 */
void SpreadsheetView::createColumnContextMenu(QMenu* menu) {
	const Column* column = dynamic_cast<Column*>(QObject::sender());
	if (!column)
		return; //should never happen, since the sender is always a Column

	const bool numeric = (column->columnMode() == AbstractColumn::Numeric) || (column->columnMode() == AbstractColumn::Integer);

	if (numeric) {
		QAction* firstAction = menu->actions().at(1);
		menu->insertMenu(firstAction, m_columnSetAsMenu);

		const bool hasValues = column->hasValues();

		if (!m_readOnly) {
			menu->insertSeparator(firstAction);
			menu->insertMenu(firstAction, m_columnGenerateDataMenu);

			menu->insertSeparator(firstAction);
			menu->insertAction(firstAction, action_reverse_columns);
			menu->insertAction(firstAction, action_drop_values);
			menu->insertAction(firstAction, action_mask_values);
			menu->insertAction(firstAction, action_normalize_columns);

			menu->insertSeparator(firstAction);
			menu->insertMenu(firstAction, m_columnSortMenu);
			action_sort_asc_column->setVisible(true);
			action_sort_desc_column->setVisible(true);
			action_sort_columns->setVisible(false);

			//in case no cells are available, deactivate the actions that only make sense in the presence of cells
			const bool hasCells = m_spreadsheet->rowCount() > 0;
			m_columnGenerateDataMenu->setEnabled(hasCells);

			//in case no valid numerical values are available, deactivate the actions that only make sense in the presence of values
			action_reverse_columns->setEnabled(hasValues);
			action_drop_values->setEnabled(hasValues);
			action_mask_values->setEnabled(hasValues);
			action_normalize_columns->setEnabled(hasValues);
			m_columnSortMenu->setEnabled(hasValues);
		}

		menu->insertSeparator(firstAction);
		menu->insertAction(firstAction, action_statistics_columns);
		action_statistics_columns->setEnabled(hasValues);
	}
}

//SLOTS
void SpreadsheetView::handleAspectAdded(const AbstractAspect* aspect) {
	const Column* col = dynamic_cast<const Column*>(aspect);
	if (!col || col->parentAspect() != m_spreadsheet)
		return;

	int index = m_spreadsheet->indexOfChild<Column>(col);
	if (col->width() == 0)
		m_tableView->resizeColumnToContents(index);
	else
		m_tableView->setColumnWidth(index, col->width());

	connect(col, SIGNAL(requestProjectContextMenu(QMenu*)), this, SLOT(createColumnContextMenu(QMenu*)));
}

void SpreadsheetView::handleAspectAboutToBeRemoved(const AbstractAspect* aspect) {
	const Column* col = dynamic_cast<const Column*>(aspect);
	if (!col || col->parentAspect() != m_spreadsheet)
		return;

	disconnect(col, 0, this, 0);
}

void SpreadsheetView::handleHorizontalSectionResized(int logicalIndex, int oldSize, int newSize) {
	Q_UNUSED(logicalIndex);
	Q_UNUSED(oldSize);

	//save the new size in the column
	Column* col = m_spreadsheet->child<Column>(logicalIndex);
	col->setWidth(newSize);
}

void SpreadsheetView::goToCell(int row, int col) {
	QModelIndex index = m_model->index(row, col);
	m_tableView->scrollTo(index);
	m_tableView->setCurrentIndex(index);
}

void SpreadsheetView::handleHorizontalSectionMoved(int index, int from, int to) {
	Q_UNUSED(index);

	static bool inside = false;
	if (inside) return;

	Q_ASSERT(index == from);

	inside = true;
	m_tableView->horizontalHeader()->moveSection(to, from);
	inside = false;
	m_spreadsheet->moveColumn(from, to);
}

//TODO Implement the "change of the column name"-mode  upon a double click
void SpreadsheetView::handleHorizontalHeaderDoubleClicked(int index) {
	Q_UNUSED(index);
}

/*!
  Returns whether comments are show currently or not.
*/
bool SpreadsheetView::areCommentsShown() const {
	return m_horizontalHeader->areCommentsShown();
}

/*!
  toggles the column comment in the horizontal header
*/
void SpreadsheetView::toggleComments() {
	showComments(!areCommentsShown());
	//TODO
	if (areCommentsShown())
		action_toggle_comments->setText(i18n("Hide Comments"));
	else
		action_toggle_comments->setText(i18n("Show Comments"));
}

//! Shows (\c on=true) or hides (\c on=false) the column comments in the horizontal header
void SpreadsheetView::showComments(bool on) {
	m_horizontalHeader->showComments(on);
}

void SpreadsheetView::currentColumnChanged(const QModelIndex & current, const QModelIndex & previous) {
	Q_UNUSED(previous);
	int col = current.column();
	if (col < 0 || col >= m_spreadsheet->columnCount())
		return;
}

//TODO
void SpreadsheetView::handleHeaderDataChanged(Qt::Orientation orientation, int first, int last) {
	if (orientation != Qt::Horizontal) return;

	QItemSelectionModel * sel_model = m_tableView->selectionModel();

	int col = sel_model->currentIndex().column();
	if (col < first || col > last) return;
}

/*!
  Returns the number of selected columns.
  If \c full is \c true, this function only returns the number of fully selected columns.
*/
int SpreadsheetView::selectedColumnCount(bool full) const {
	int count = 0;
	const int cols = m_spreadsheet->columnCount();
	for (int i=0; i<cols; i++)
		if (isColumnSelected(i, full)) count++;
	return count;
}

/*!
  Returns the number of (at least partly) selected columns with the plot designation \param pd.
 */
int SpreadsheetView::selectedColumnCount(AbstractColumn::PlotDesignation pd) const{
	int count = 0;
	const int cols = m_spreadsheet->columnCount();
	for (int i=0; i<cols; i++)
		if ( isColumnSelected(i, false) && (m_spreadsheet->column(i)->plotDesignation() == pd) ) count++;

	return count;
}

/*!
  Returns \c true if column \param col is selected, otherwise returns \c false.
  If \param full is \c true, this function only returns true if the whole column is selected.
*/
bool SpreadsheetView::isColumnSelected(int col, bool full) const {
	if (full)
		return m_tableView->selectionModel()->isColumnSelected(col, QModelIndex());
	else
		return m_tableView->selectionModel()->columnIntersectsSelection(col, QModelIndex());
}

/*!
  Returns all selected columns.
  If \param full is true, this function only returns a column if the whole column is selected.
  */
QVector<Column*> SpreadsheetView::selectedColumns(bool full) const {
	QVector<Column*> columns;
	const int cols = m_spreadsheet->columnCount();
	for (int i=0; i<cols; i++)
		if (isColumnSelected(i, full)) columns << m_spreadsheet->column(i);

	return columns;
}

/*!
  Returns \c true if row \param row is selected; otherwise returns \c false
  If \param full is \c true, this function only returns \c true if the whole row is selected.
*/
bool SpreadsheetView::isRowSelected(int row, bool full) const {
	if (full)
		return m_tableView->selectionModel()->isRowSelected(row, QModelIndex());
	else
		return m_tableView->selectionModel()->rowIntersectsSelection(row, QModelIndex());
}

/*!
  Return the index of the first selected column.
  If \param full is \c true, this function only looks for fully selected columns.
*/
int SpreadsheetView::firstSelectedColumn(bool full) const {
	const int cols = m_spreadsheet->columnCount();
	for (int i=0; i<cols; i++) {
		if (isColumnSelected(i, full))
			return i;
	}
	return -1;
}

/*!
  Return the index of the last selected column.
  If \param full is \c true, this function only looks for fully selected columns.
  */
int SpreadsheetView::lastSelectedColumn(bool full) const {
	const int cols = m_spreadsheet->columnCount();
	for (int i=cols-1; i>=0; i--)
		if (isColumnSelected(i, full)) return i;

	return -2;
}

/*!
  Return the index of the first selected row.
  If \param full is \c true, this function only looks for fully selected rows.
  */
int SpreadsheetView::firstSelectedRow(bool full) const{
	QModelIndexList indexes;
	if (!full)
		indexes = m_tableView->selectionModel()->selectedIndexes();
	else
		indexes = m_tableView->selectionModel()->selectedRows();

	if (!indexes.empty())
		return indexes.first().row();
	else
		return -1;
}

/*!
  Return the index of the last selected row.
  If \param full is \c true, this function only looks for fully selected rows.
  */
int SpreadsheetView::lastSelectedRow(bool full) const {
	QModelIndexList indexes;
	if (!full)
		indexes = m_tableView->selectionModel()->selectedIndexes();
	else
		indexes = m_tableView->selectionModel()->selectedRows();

	if (!indexes.empty())
		return indexes.last().row();
	else
		return -2;
}

/*!
  Return whether a cell is selected
 */
bool SpreadsheetView::isCellSelected(int row, int col) const {
	if (row < 0 || col < 0 || row >= m_spreadsheet->rowCount() || col >= m_spreadsheet->columnCount())
		return false;

	return m_tableView->selectionModel()->isSelected(m_model->index(row, col));
}

/*!
  Get the complete set of selected rows.
 */
IntervalAttribute<bool> SpreadsheetView::selectedRows(bool full) const {
	IntervalAttribute<bool> result;
	const int rows = m_spreadsheet->rowCount();
	for (int i=0; i<rows; i++)
		if (isRowSelected(i, full))
			result.setValue(i, true);
	return result;
}

/*!
  Select/Deselect a cell.
 */
void SpreadsheetView::setCellSelected(int row, int col, bool select) {
	m_tableView->selectionModel()->select(m_model->index(row, col),
	                                      select ? QItemSelectionModel::Select : QItemSelectionModel::Deselect);
}

/*!
  Select/Deselect a range of cells.
 */
void SpreadsheetView::setCellsSelected(int first_row, int first_col, int last_row, int last_col, bool select) {
	QModelIndex top_left = m_model->index(first_row, first_col);
	QModelIndex bottom_right = m_model->index(last_row, last_col);
	m_tableView->selectionModel()->select(QItemSelection(top_left, bottom_right),
	                                      select ? QItemSelectionModel::SelectCurrent : QItemSelectionModel::Deselect);
}

/*!
  Determine the current cell (-1 if no cell is designated as the current).
 */
void SpreadsheetView::getCurrentCell(int* row, int* col) const {
	QModelIndex index = m_tableView->selectionModel()->currentIndex();
	if (index.isValid())	{
		*row = index.row();
		*col = index.column();
	} else {
		*row = -1;
		*col = -1;
	}
}

bool SpreadsheetView::eventFilter(QObject* watched, QEvent* event) {
	if (event->type() == QEvent::ContextMenu) {
		QContextMenuEvent* cm_event = static_cast<QContextMenuEvent*>(event);
		const QPoint global_pos = cm_event->globalPos();
		if (watched == m_tableView->verticalHeader()) {
			bool onlyNumeric = true;
			for (int i = 0; i < m_spreadsheet->columnCount(); ++i) {
				if (m_spreadsheet->column(i)->columnMode() != AbstractColumn::Numeric) {
					onlyNumeric = false;
					break;
				}
			}
			action_statistics_rows->setVisible(onlyNumeric);
			m_rowMenu->exec(global_pos);
		} else if (watched == m_horizontalHeader) {
			const int col = m_horizontalHeader->logicalIndexAt(cm_event->pos());
			if (!isColumnSelected(col, true)) {
				QItemSelectionModel* sel_model = m_tableView->selectionModel();
				sel_model->clearSelection();
				sel_model->select(QItemSelection(m_model->index(0, col, QModelIndex()),
												m_model->index(m_model->rowCount()-1, col, QModelIndex())),
								QItemSelectionModel::Select);
			}

			if (selectedColumns().size()==1) {
				action_sort_columns->setVisible(false);
				action_sort_asc_column->setVisible(true);
				action_sort_desc_column->setVisible(true);
			} else {
				action_sort_columns->setVisible(true);
				action_sort_asc_column->setVisible(false);
				action_sort_desc_column->setVisible(false);
			}

			//check whether we have non-numeric columns selected and deactivate actions for numeric columns
			bool numeric = true;
			for(const Column* col : selectedColumns()) {
				if ( !(col->columnMode() == AbstractColumn::Numeric || col->columnMode() == AbstractColumn::Integer) ) {
					numeric = false;
					break;
				}
			}

			action_plot_data->setEnabled(numeric);
			m_analyzePlotMenu->setEnabled(numeric);
			m_columnSetAsMenu->setEnabled(numeric);
			if (!m_readOnly) {
				m_columnGenerateDataMenu->setEnabled(numeric);
				action_reverse_columns->setEnabled(numeric);
				action_drop_values->setEnabled(numeric);
				action_mask_values->setEnabled(numeric);
				action_normalize_columns->setEnabled(numeric);
				m_columnSortMenu->setEnabled(numeric);
			}
			action_statistics_columns->setEnabled(numeric);

			if (numeric) {
				bool hasValues = false;
				for (const Column* col : selectedColumns()) {
					if (col->hasValues()) {
						hasValues = true;
						break;
					}
				}

				if (!m_readOnly) {
					//in case no cells are available, deactivate the actions that only make sense in the presence of cells
					const bool hasCells = m_spreadsheet->rowCount() > 0;
					m_columnGenerateDataMenu->setEnabled(hasCells);

					//in case no valid numerical values are available, deactivate the actions that only make sense in the presence of values
					action_reverse_columns->setEnabled(hasValues);
					action_drop_values->setEnabled(hasValues);
					action_mask_values->setEnabled(hasValues);
					action_normalize_columns->setEnabled(hasValues);
					m_columnSortMenu->setEnabled(hasValues);
				}

				action_statistics_columns->setEnabled(hasValues);
			}

			m_columnMenu->exec(global_pos);
		} else if (watched == this) {
			checkSpreadsheetMenu();
			m_spreadsheetMenu->exec(global_pos);
		}

		return true;
	} else if (event->type() == QEvent::KeyPress) {
		QKeyEvent* key_event = static_cast<QKeyEvent*>(event);
		if (key_event->matches(QKeySequence::Copy))
			copySelection();
		else if (key_event->matches(QKeySequence::Paste))
			pasteIntoSelection();
	}

	return QWidget::eventFilter(watched, event);
}

/*!
 * disables cell data relevant actions in the spreadsheet menu if there're no cells available
 */
void SpreadsheetView::checkSpreadsheetMenu() {
	const bool cellsAvail = m_spreadsheet->columnCount()>0 && m_spreadsheet->rowCount()>0;
	action_plot_data->setEnabled(cellsAvail);
	m_selectionMenu->setEnabled(cellsAvail);
	action_select_all->setEnabled(cellsAvail);
	action_clear_spreadsheet->setEnabled(cellsAvail);
	action_clear_masks->setEnabled(cellsAvail);
	action_sort_spreadsheet->setEnabled(cellsAvail);
	action_go_to_cell->setEnabled(cellsAvail);
	action_statistics_all_columns->setEnabled(cellsAvail);
}

bool SpreadsheetView::formulaModeActive() const {
	return m_model->formulaModeActive();
}

void SpreadsheetView::activateFormulaMode(bool on) {
	m_model->activateFormulaMode(on);
}

void SpreadsheetView::goToNextColumn() {
	if (m_spreadsheet->columnCount() == 0) return;

	QModelIndex idx = m_tableView->currentIndex();
	int col = idx.column()+1;
	if (col >= m_spreadsheet->columnCount())
		col = 0;

	m_tableView->setCurrentIndex(idx.sibling(idx.row(), col));
}

void SpreadsheetView::goToPreviousColumn() {
	if (m_spreadsheet->columnCount() == 0)
		return;

	QModelIndex idx = m_tableView->currentIndex();
	int col = idx.column()-1;
	if (col < 0)
		col = m_spreadsheet->columnCount()-1;

	m_tableView->setCurrentIndex(idx.sibling(idx.row(), col));
}

void SpreadsheetView::cutSelection() {
	int first = firstSelectedRow();
	if ( first < 0 )
		return;

	WAIT_CURSOR;
	m_spreadsheet->beginMacro(i18n("%1: cut selected cells", m_spreadsheet->name()));
	copySelection();
	clearSelectedCells();
	m_spreadsheet->endMacro();
	RESET_CURSOR;
}

void SpreadsheetView::copySelection() {
	PERFTRACE("copy selected cells");
	const int first_col = firstSelectedColumn();
	if (first_col == -1) return;
	const int last_col = lastSelectedColumn();
	if (last_col == -2) return;
	const int first_row = firstSelectedRow();
	if (first_row == -1)	return;
	const int last_row = lastSelectedRow();
	if (last_row == -2) return;
	const int cols = last_col - first_col + 1;
	const int rows = last_row - first_row + 1;

	WAIT_CURSOR;
	QString output_str;

	QVector<Column*> columns;
	QVector<char> formats;
	for (int c = 0; c < cols; c++) {
		Column* col = m_spreadsheet->column(first_col + c);
		columns << col;
		const Double2StringFilter* out_fltr = static_cast<Double2StringFilter*>(col->outputFilter());
		formats << out_fltr->numericFormat();
	}

	QLocale locale;
	for (int r = 0; r < rows; r++) {
		for (int c = 0; c < cols; c++) {
			const Column* col_ptr = columns.at(c);
			if (isCellSelected(first_row + r, first_col + c)) {
// 				if (formulaModeActive())
// 					output_str += col_ptr->formula(first_row + r);
// 				else
				if (col_ptr->columnMode() == AbstractColumn::Numeric)
					output_str += locale.toString(col_ptr->valueAt(first_row + r), formats.at(c), 16); // copy with max. precision
				else
					output_str += col_ptr->asStringColumn()->textAt(first_row + r);
			}
			if (c < cols-1)
				output_str += '\t';
		}
		if (r < rows-1)
			output_str += '\n';
	}

	QApplication::clipboard()->setText(output_str);
	RESET_CURSOR;
}

void SpreadsheetView::pasteIntoSelection() {
	if (m_spreadsheet->columnCount() < 1 || m_spreadsheet->rowCount() < 1)
		return;

	const QMimeData* mime_data = QApplication::clipboard()->mimeData();
	if (!mime_data->hasFormat("text/plain"))
		return;

	PERFTRACE("paste selected cells");
	WAIT_CURSOR;
	m_spreadsheet->beginMacro(i18n("%1: paste from clipboard", m_spreadsheet->name()));

	int first_col = firstSelectedColumn();
	int last_col = lastSelectedColumn();
	int first_row = firstSelectedRow();
	int last_row = lastSelectedRow();
	int input_row_count = 0;
	int input_col_count = 0;

	QString input_str = QString(mime_data->data("text/plain")).trimmed();
	QVector<QStringList> cellTexts;
	QStringList input_rows(input_str.split('\n'));
	input_row_count = input_rows.count();
	input_col_count = 0;
	for (int i=0; i<input_row_count; i++) {
		cellTexts.append(input_rows.at(i).split(QRegExp("\\s+")));
		if (cellTexts.at(i).count() > input_col_count)
			input_col_count = cellTexts.at(i).count();
	}

	if ( (first_col == -1 || first_row == -1) || (last_row == first_row && last_col == first_col) ) {
		// if the is no selection or only one cell selected, the
		// selection will be expanded to the needed size from the current cell
		int current_row, current_col;
		getCurrentCell(&current_row, &current_col);
		if (current_row == -1) current_row = 0;
		if (current_col == -1) current_col = 0;
		setCellSelected(current_row, current_col);
		first_col = current_col;
		first_row = current_row;
		last_row = first_row + input_row_count -1;
		last_col = first_col + input_col_count -1;

		//add columns if necessary
		const int columnCount = m_spreadsheet->columnCount();
		if (last_col >= columnCount) {
			for (int c = 0; c < last_col - (columnCount - 1); ++c) {
				const int curCol = columnCount - 1 + c;
				//first non-empty value in the column to paste determines the column mode/type of the new column to be added
				QString nonEmptyValue;
				for (int r = 0; r<cellTexts.size(); ++r) {
					if (!cellTexts.at(r).at(curCol).isEmpty()) {
						nonEmptyValue = cellTexts.at(r).at(curCol);
						break;
					}
				}
				const AbstractColumn::ColumnMode mode = AbstractFileFilter::columnMode(nonEmptyValue,
														QLatin1String("yyyy-dd-MM hh:mm:ss:zzz"), QLocale::AnyLanguage);
				Column* new_col = new Column(QString::number(curCol), mode);
				new_col->setPlotDesignation(AbstractColumn::Y);
				new_col->insertRows(0, m_spreadsheet->rowCount());
				m_spreadsheet->addChild(new_col);
			}
		}

		//add rows if necessary
		if (last_row >= m_spreadsheet->rowCount())
			m_spreadsheet->appendRows(last_row + 1 - m_spreadsheet->rowCount());

		// select the rectangle to be pasted in
		setCellsSelected(first_row, first_col, last_row, last_col);
	}

	const int rows = last_row - first_row + 1;
	const int cols = last_col - first_col + 1;
	QLocale locale;

	for (int c = 0; c < cols && c < input_col_count; c++) {
		Column* col = m_spreadsheet->column(first_col + c);
		col->setSuppressDataChangedSignal(true);
		if (col->columnMode() == AbstractColumn::Numeric) {
			if (rows == m_spreadsheet->rowCount()) {
				QVector<double> new_data(rows);
				for (int r = 0; r < rows; ++r)
					new_data[r] = locale.toDouble(cellTexts.at(r).at(c));
				col->replaceValues(0, new_data);
			} else {
				for (int r = 0; r < rows && r < input_row_count; r++) {
					if ( isCellSelected(first_row + r, first_col + c) && (c < cellTexts.at(r).count()) ) {
						if (!cellTexts.at(r).at(c).isEmpty())
							col->setValueAt(first_row + r, locale.toDouble(cellTexts.at(r).at(c)));
						else
							col->setValueAt(first_row + r, NAN);
					}
				}
			}
		} else {
			for (int r = 0; r < rows && r < input_row_count; r++) {
				if (isCellSelected(first_row + r, first_col + c) && (c < cellTexts.at(r).count()) ) {
// 					if (formulaModeActive())
// 						col->setFormula(first_row + r, cellTexts.at(r).at(c));
// 					else
					col->asStringColumn()->setTextAt(first_row + r, cellTexts.at(r).at(c));
				}
			}
		}

		col->setSuppressDataChangedSignal(false);
		col->setChanged();
	}

	m_spreadsheet->endMacro();
	RESET_CURSOR;
}

void SpreadsheetView::maskSelection() {
	int first = firstSelectedRow();
	if ( first < 0 ) return;
	int last = lastSelectedRow();

	WAIT_CURSOR;
	m_spreadsheet->beginMacro(i18n("%1: mask selected cells", m_spreadsheet->name()));
	for (auto* column : selectedColumns()) {
		int col = m_spreadsheet->indexOfChild<Column>(column);
		for (int row=first; row<=last; row++)
			if (isCellSelected(row, col)) column->setMasked(row);
	}
	m_spreadsheet->endMacro();
	RESET_CURSOR;
}

void SpreadsheetView::unmaskSelection() {
	int first = firstSelectedRow();
	if ( first < 0 ) return;
	int last = lastSelectedRow();

	WAIT_CURSOR;
	m_spreadsheet->beginMacro(i18n("%1: unmask selected cells", m_spreadsheet->name()));
	for (auto* column : selectedColumns()) {
		int col = m_spreadsheet->indexOfChild<Column>(column);
		for (int row=first; row<=last; row++)
			if (isCellSelected(row, col)) column->setMasked(row, false);
	}
	m_spreadsheet->endMacro();
	RESET_CURSOR;
}

void SpreadsheetView::plotData() {
	PlotDataDialog* dlg = new PlotDataDialog(m_spreadsheet);
	const QObject* sender = QObject::sender();
	if (sender != action_plot_data) {
		PlotDataDialog::AnalysisAction action = (PlotDataDialog::AnalysisAction)dynamic_cast<const QAction*>(sender)->data().toInt();
		dlg->setAnalysisAction(action);
	}
	dlg->exec();
}

void SpreadsheetView::fillSelectedCellsWithRowNumbers() {
	if (selectedColumnCount() < 1) return;
	int first = firstSelectedRow();
	if ( first < 0 ) return;
	int last = lastSelectedRow();

	WAIT_CURSOR;
	m_spreadsheet->beginMacro(i18n("%1: fill cells with row numbers", m_spreadsheet->name()));
	for (auto* col_ptr: selectedColumns()) {
		int col = m_spreadsheet->indexOfChild<Column>(col_ptr);
		col_ptr->setSuppressDataChangedSignal(true);
		switch (col_ptr->columnMode()) {
		case AbstractColumn::Numeric: {
				QVector<double> results(last-first+1);
				for (int row = first; row <= last; row++)
					if (isCellSelected(row, col))
						results[row-first] = row + 1;
					else
						results[row-first] = col_ptr->valueAt(row);
				col_ptr->replaceValues(first, results);
				break;
			}
		case AbstractColumn::Integer: {
				QVector<int> results(last-first+1);
				for (int row = first; row <= last; row++)
					if (isCellSelected(row, col))
						results[row-first] = row + 1;
					else
						results[row-first] = col_ptr->integerAt(row);
				col_ptr->replaceInteger(first, results);
				break;
			}
		case AbstractColumn::Text: {
				QVector<QString> results;
				for (int row = first; row <= last; row++)
					if (isCellSelected(row, col))
						results << QString::number(row+1);
					else
						results << col_ptr->textAt(row);
				col_ptr->replaceTexts(first, results);
				break;
			}
		//TODO: handle other modes
		case AbstractColumn::DateTime:
		case AbstractColumn::Month:
		case AbstractColumn::Day:
			break;
		}

		col_ptr->setSuppressDataChangedSignal(false);
		col_ptr->setChanged();
	}
	m_spreadsheet->endMacro();
	RESET_CURSOR;
}

void SpreadsheetView::fillWithRowNumbers() {
	if (selectedColumnCount() < 1) return;

	WAIT_CURSOR;
	m_spreadsheet->beginMacro(i18np("%1: fill column with row numbers",
	                                "%1: fill columns with row numbers",
	                                m_spreadsheet->name(),
	                                selectedColumnCount()));

	const int rows = m_spreadsheet->rowCount();

	QVector<double> double_data(rows);
	QVector<int> int_data(rows);
	for (int i = 0; i < rows; ++i)
		double_data[i] = int_data[i] = i+1;

	for (auto* col: selectedColumns()) {
		switch (col->columnMode()) {
		case AbstractColumn::Numeric:
			col->replaceValues(0, double_data);
			break;
		case AbstractColumn::Integer:
			col->replaceInteger(0, int_data);
			break;
		case AbstractColumn::Text:
		case AbstractColumn::DateTime:
		case AbstractColumn::Day:
		case AbstractColumn::Month:
			break;
		}
	}

	m_spreadsheet->endMacro();
	RESET_CURSOR;
}

//TODO: this function is not used currently.
void SpreadsheetView::fillSelectedCellsWithRandomNumbers() {
	if (selectedColumnCount() < 1) return;
	int first = firstSelectedRow();
	int last = lastSelectedRow();
	if (first < 0) return;

	WAIT_CURSOR;
	m_spreadsheet->beginMacro(i18n("%1: fill cells with random values", m_spreadsheet->name()));
	qsrand(QTime::currentTime().msec());
	for (auto* col_ptr: selectedColumns()) {
		int col = m_spreadsheet->indexOfChild<Column>(col_ptr);
		col_ptr->setSuppressDataChangedSignal(true);
		switch (col_ptr->columnMode()) {
		case AbstractColumn::Numeric: {
				QVector<double> results(last-first+1);
				for (int row = first; row <= last; row++)
					if (isCellSelected(row, col))
						results[row-first] = double(qrand())/double(RAND_MAX);
					else
						results[row-first] = col_ptr->valueAt(row);
				col_ptr->replaceValues(first, results);
				break;
			}
		case AbstractColumn::Integer: {
				QVector<int> results(last-first+1);
				for (int row = first; row <= last; row++)
					if (isCellSelected(row, col))
						results[row-first] = qrand();
					else
						results[row-first] = col_ptr->integerAt(row);
				col_ptr->replaceInteger(first, results);
				break;
			}
		case AbstractColumn::Text: {
				QVector<QString> results;
				for (int row = first; row <= last; row++)
					if (isCellSelected(row, col))
						results << QString::number(double(qrand())/double(RAND_MAX));
					else
						results << col_ptr->textAt(row);
				col_ptr->replaceTexts(first, results);
				break;
			}
		case AbstractColumn::DateTime:
		case AbstractColumn::Month:
		case AbstractColumn::Day: {
				QVector<QDateTime> results;
				QDate earliestDate(1,1,1);
				QDate latestDate(2999,12,31);
				QTime midnight(0,0,0,0);
				for (int row = first; row <= last; row++)
					if (isCellSelected(row, col))
						results << QDateTime( earliestDate.addDays(((double)qrand())*((double)earliestDate.daysTo(latestDate))/((double)RAND_MAX)), midnight.addMSecs(((qint64)qrand())*1000*60*60*24/RAND_MAX));
					else
						results << col_ptr->dateTimeAt(row);
				col_ptr->replaceDateTimes(first, results);
				break;
			}
		}

		col_ptr->setSuppressDataChangedSignal(false);
		col_ptr->setChanged();
	}
	m_spreadsheet->endMacro();
	RESET_CURSOR;
}

void SpreadsheetView::fillWithRandomValues() {
	if (selectedColumnCount() < 1) return;
	RandomValuesDialog* dlg = new RandomValuesDialog(m_spreadsheet);
	dlg->setColumns(selectedColumns());
	dlg->exec();
}

void SpreadsheetView::fillWithEquidistantValues() {
	if (selectedColumnCount() < 1) return;
	EquidistantValuesDialog* dlg = new EquidistantValuesDialog(m_spreadsheet);
	dlg->setColumns(selectedColumns());
	dlg->exec();
}

void SpreadsheetView::fillWithFunctionValues() {
	if (selectedColumnCount() < 1) return;
	FunctionValuesDialog* dlg = new FunctionValuesDialog(m_spreadsheet);
	dlg->setColumns(selectedColumns());
	dlg->exec();
}

void SpreadsheetView::fillSelectedCellsWithConstValues() {
	if (selectedColumnCount() < 1) return;
	int first = firstSelectedRow();
	int last = lastSelectedRow();
	if (first < 0)
		return;

	bool doubleOk = false;
	bool intOk = false;
	bool stringOk = false;
	double doubleValue = 0;
	int intValue = 0;
	QString stringValue;

	m_spreadsheet->beginMacro(i18n("%1: fill cells with const values", m_spreadsheet->name()));
	for (auto* col_ptr: selectedColumns()) {
		int col = m_spreadsheet->indexOfChild<Column>(col_ptr);
		col_ptr->setSuppressDataChangedSignal(true);
		switch (col_ptr->columnMode()) {
		case AbstractColumn::Numeric:
			if (!doubleOk)
				doubleValue = QInputDialog::getDouble(this, i18n("Fill the selection with constant value"),
				                                      i18n("Value"), 0, -2147483647, 2147483647, 6, &doubleOk);
			if (doubleOk) {
				WAIT_CURSOR;
				QVector<double> results(last-first+1);
				for (int row = first; row <= last; row++) {
					if (isCellSelected(row, col))
						results[row-first] = doubleValue;
					else
						results[row-first] = col_ptr->valueAt(row);
				}
				col_ptr->replaceValues(first, results);
				RESET_CURSOR;
			}
			break;
		case AbstractColumn::Integer:
			if (!intOk)
				intValue = QInputDialog::getInt(this, i18n("Fill the selection with constant value"),
				                                i18n("Value"), 0, -2147483647, 2147483647, 1, &intOk);
			if (intOk) {
				WAIT_CURSOR;
				QVector<int> results(last-first+1);
				for (int row = first; row <= last; row++) {
					if (isCellSelected(row, col))
						results[row-first] = intValue;
					else
						results[row-first] = col_ptr->integerAt(row);
				}
				col_ptr->replaceInteger(first, results);
				RESET_CURSOR;
			}
			break;
		case AbstractColumn::Text:
			if (!stringOk)
				stringValue = QInputDialog::getText(this, i18n("Fill the selection with constant value"),
				                                    i18n("Value"), QLineEdit::Normal, 0, &stringOk);
			if (stringOk && !stringValue.isEmpty()) {
				WAIT_CURSOR;
				QVector<QString> results;
				for (int row = first; row <= last; row++) {
					if (isCellSelected(row, col))
						results << stringValue;
					else
						results << col_ptr->textAt(row);
				}
				col_ptr->replaceTexts(first, results);
				RESET_CURSOR;
			}
			break;
		//TODO: handle other modes
		case AbstractColumn::DateTime:
		case AbstractColumn::Month:
		case AbstractColumn::Day:
			break;
		}

		col_ptr->setSuppressDataChangedSignal(false);
		col_ptr->setChanged();
	}
	m_spreadsheet->endMacro();
}

/*!
    Open the sort dialog for all columns.
*/
void SpreadsheetView::sortSpreadsheet() {
	sortDialog(m_spreadsheet->children<Column>());
}

/*!
  Insert an empty column left to the firt selected column
*/
void SpreadsheetView::insertColumnLeft() {
	WAIT_CURSOR;
	m_spreadsheet->beginMacro(i18n("%1: insert empty column", m_spreadsheet->name()));

	Column* newCol = new Column("1", AbstractColumn::Numeric);
	newCol->setPlotDesignation(AbstractColumn::Y);
	const int first = firstSelectedColumn();

	if (first >= 0) {
		//determine the first selected column
		Column* firstCol = m_spreadsheet->child<Column>(first);

		//resize the new column and insert it before the first selected column
		newCol->insertRows(0, m_spreadsheet->rowCount());
		m_spreadsheet->insertChildBefore(newCol, firstCol);
	} else {
		if (m_spreadsheet->columnCount()>0) {
			//columns available but no columns selected -> prepend the new column at the very beginning
			Column* firstCol = m_spreadsheet->child<Column>(0);
			newCol->insertRows(0, m_spreadsheet->rowCount());
			m_spreadsheet->insertChildBefore(newCol, firstCol);
		} else {
			//no columns available anymore -> resize the spreadsheet and the new column to the default size
			KConfigGroup group = KSharedConfig::openConfig()->group(QLatin1String("Spreadsheet"));
			const int rows = group.readEntry(QLatin1String("RowCount"), 100);
			m_spreadsheet->setRowCount(rows);
			newCol->insertRows(0, rows);

			//add/append a new column
			m_spreadsheet->addChild(newCol);
		}
	}

	m_spreadsheet->endMacro();
	RESET_CURSOR;
}

/*!
  Insert an empty column right to the last selected column
*/
void SpreadsheetView::insertColumnRight() {
	WAIT_CURSOR;
	m_spreadsheet->beginMacro(i18n("%1: insert empty column", m_spreadsheet->name()));

	Column* newCol = new Column("1", AbstractColumn::Numeric);
	newCol->setPlotDesignation(AbstractColumn::Y);
	const int last = lastSelectedColumn();

	if (last >= 0) {
		newCol->insertRows(0, m_spreadsheet->rowCount());
		if (last < m_spreadsheet->columnCount() - 1) {
			//determine the column next to the last selected column
			Column* nextCol = m_spreadsheet->child<Column>(last + 1);

			//insert the new column before the column next to the last selected column
			m_spreadsheet->insertChildBefore(newCol, nextCol);
		} else {
			//last column selected, no next column available -> add/append a new column
			m_spreadsheet->addChild(newCol);
		}
	} else {
		if (m_spreadsheet->columnCount()>0) {
			//columns available but no columns selected -> append the new column at the very end
			newCol->insertRows(0, m_spreadsheet->rowCount());
			m_spreadsheet->addChild(newCol);
		} else {
			//no columns available anymore -> resize the spreadsheet and the new column to the default size
			KConfigGroup group = KSharedConfig::openConfig()->group(QLatin1String("Spreadsheet"));
			const int rows = group.readEntry(QLatin1String("RowCount"), 100);
			newCol->insertRows(0, rows);
			m_spreadsheet->setRowCount(rows);

			 //add/append a new column
			m_spreadsheet->addChild(newCol);
		}
	}

	m_spreadsheet->endMacro();
	RESET_CURSOR;
}

void SpreadsheetView::removeSelectedColumns() {
	WAIT_CURSOR;
	m_spreadsheet->beginMacro(i18n("%1: remove selected columns", m_spreadsheet->name()));

	for (auto* column : selectedColumns())
		m_spreadsheet->removeChild(column);

	m_spreadsheet->endMacro();
	RESET_CURSOR;
}

void SpreadsheetView::clearSelectedColumns() {
	WAIT_CURSOR;
	m_spreadsheet->beginMacro(i18n("%1: clear selected columns", m_spreadsheet->name()));

	if (formulaModeActive()) {
		for (auto* col: selectedColumns()) {
			col->setSuppressDataChangedSignal(true);
			col->clearFormulas();
			col->setSuppressDataChangedSignal(false);
			col->setChanged();
		}
	} else {
		for (auto* col: selectedColumns()) {
			col->setSuppressDataChangedSignal(true);
			col->clear();
			col->setSuppressDataChangedSignal(false);
			col->setChanged();
		}
	}

	m_spreadsheet->endMacro();
	RESET_CURSOR;
}

void SpreadsheetView::setSelectionAs() {
	QVector<Column*> columns = selectedColumns();
	if (!columns.size())
		return;

	m_spreadsheet->beginMacro(i18n("%1: set plot designation", m_spreadsheet->name()));

	QAction* action = dynamic_cast<QAction*>(QObject::sender());
	if (!action)
		return;

	AbstractColumn::PlotDesignation pd = (AbstractColumn::PlotDesignation)action->data().toInt();
	for (auto* col: columns)
		col->setPlotDesignation(pd);

	m_spreadsheet->endMacro();
}

void SpreadsheetView::reverseColumns() {
	WAIT_CURSOR;
	QVector<Column*> cols = selectedColumns();
	m_spreadsheet->beginMacro(i18np("%1: reverse column", "%1: reverse columns",
	                                m_spreadsheet->name(), cols.size()));
	for (auto* col: cols) {
		if (col->columnMode() != AbstractColumn::Numeric)
			continue;

		QVector<double>* data = static_cast<QVector<double>* >(col->data());
		QVector<double> new_data(*data);
		std::reverse(new_data.begin(), new_data.end());
		col->replaceValues(0, new_data);
	}
	m_spreadsheet->endMacro();
	RESET_CURSOR;
}

void SpreadsheetView::dropColumnValues() {
	if (selectedColumnCount() < 1) return;
	DropValuesDialog* dlg = new DropValuesDialog(m_spreadsheet);
	dlg->setColumns(selectedColumns());
	dlg->exec();
}

void SpreadsheetView::maskColumnValues() {
	if (selectedColumnCount() < 1) return;
	DropValuesDialog* dlg = new DropValuesDialog(m_spreadsheet, true);
	dlg->setColumns(selectedColumns());
	dlg->exec();
}

void SpreadsheetView::joinColumns() {
	//TODO
}

void SpreadsheetView::normalizeSelectedColumns() {
	WAIT_CURSOR;
	m_spreadsheet->beginMacro(i18n("%1: normalize columns", m_spreadsheet->name()));
	for (auto* col : selectedColumns()) {
		if (col->columnMode() == AbstractColumn::Numeric) {
			col->setSuppressDataChangedSignal(true);
			double max = col->maximum();
			if (max != 0.0) {// avoid division by zero
				for (int row=0; row<col->rowCount(); row++)
					col->setValueAt(row, col->valueAt(row) / max);
			}
			col->setSuppressDataChangedSignal(false);
			col->setChanged();
		}
	}
	m_spreadsheet->endMacro();
	RESET_CURSOR;
}

void SpreadsheetView::normalizeSelection() {
	WAIT_CURSOR;
	m_spreadsheet->beginMacro(i18n("%1: normalize selection", m_spreadsheet->name()));
	double max = 0.0;
	for (int col = firstSelectedColumn(); col <= lastSelectedColumn(); col++)
		if (m_spreadsheet->column(col)->columnMode() == AbstractColumn::Numeric)
			for (int row = 0; row < m_spreadsheet->rowCount(); row++) {
				if (isCellSelected(row, col) && m_spreadsheet->column(col)->valueAt(row) > max)
					max = m_spreadsheet->column(col)->valueAt(row);
			}

	if (max != 0.0) { // avoid division by zero
		//TODO setSuppressDataChangedSignal
		for (int col = firstSelectedColumn(); col <= lastSelectedColumn(); col++)
			if (m_spreadsheet->column(col)->columnMode() == AbstractColumn::Numeric)
				for (int row = 0; row < m_spreadsheet->rowCount(); row++) {
					if (isCellSelected(row, col))
						m_spreadsheet->column(col)->setValueAt(row, m_spreadsheet->column(col)->valueAt(row) / max);
				}
	}
	m_spreadsheet->endMacro();
	RESET_CURSOR;
}

void SpreadsheetView::sortSelectedColumns() {
	sortDialog(selectedColumns());
}


void SpreadsheetView::showAllColumnsStatistics() {
	showColumnStatistics(true);
}

void SpreadsheetView::showColumnStatistics(bool forAll) {
	QString dlgTitle(m_spreadsheet->name() + " column statistics");
	StatisticsDialog* dlg = new StatisticsDialog(dlgTitle);
	QVector<Column*> columns;

	if (!forAll)
		dlg->setColumns(selectedColumns());
	else if (forAll) {
		for (int col = 0; col < m_spreadsheet->columnCount(); ++col) {
			if (m_spreadsheet->column(col)->columnMode() == AbstractColumn::Numeric)
				columns << m_spreadsheet->column(col);
		}
		dlg->setColumns(columns);
	}
	if (dlg->exec() == QDialog::Accepted) {
		if (forAll)
			columns.clear();
	}
}

void SpreadsheetView::showRowStatistics() {
	QString dlgTitle(m_spreadsheet->name() + " row statistics");
	StatisticsDialog* dlg = new StatisticsDialog(dlgTitle);

	QVector<Column*> columns;
	for (int i = 0; i < m_spreadsheet->rowCount(); ++i) {
		if (isRowSelected(i)) {
			QVector<double> rowValues;
			for (int j = 0; j < m_spreadsheet->columnCount(); ++j)
				rowValues << m_spreadsheet->column(j)->valueAt(i);
			columns << new Column(QString::number(i+1), rowValues);
		}
	}
	dlg->setColumns(columns);

	if (dlg->exec() == QDialog::Accepted) {
		qDeleteAll(columns);
		columns.clear();
	}
}

/*!
  Insert an empty row above(=before) the first selected row
*/
void SpreadsheetView::insertRowAbove() {
	int first = firstSelectedRow();
	if (first < 0)
		return;

	WAIT_CURSOR;
	m_spreadsheet->beginMacro(i18n("%1: insert empty rows", m_spreadsheet->name()));
	m_spreadsheet->insertRows(first, 1);
	m_spreadsheet->endMacro();
	RESET_CURSOR;
}

/*!
  Insert an empty row below the last selected row
*/
void SpreadsheetView::insertRowBelow() {
	int last = lastSelectedRow();
	if (last < 0)
		return;

	WAIT_CURSOR;
	m_spreadsheet->beginMacro(i18n("%1: insert empty rows", m_spreadsheet->name()));

	if (last < m_spreadsheet->rowCount() -1)
		m_spreadsheet->insertRows(last + 1, 1); //insert before the next to the last selected row
	else
		m_spreadsheet->appendRow(); //append one row at the end

	m_spreadsheet->endMacro();
	RESET_CURSOR;
}

void SpreadsheetView::removeSelectedRows() {
	if (firstSelectedRow() < 0) return;

	WAIT_CURSOR;
	m_spreadsheet->beginMacro(i18n("%1: remove selected rows", m_spreadsheet->name()));
	//TODO setSuppressDataChangedSignal
	for (const auto& i: selectedRows().intervals())
		m_spreadsheet->removeRows(i.start(), i.size());
	m_spreadsheet->endMacro();
	RESET_CURSOR;
}

void SpreadsheetView::clearSelectedRows() {
	if (firstSelectedRow() < 0) return;

	WAIT_CURSOR;
	m_spreadsheet->beginMacro(i18n("%1: clear selected rows", m_spreadsheet->name()));
	for (auto* col: selectedColumns()) {
		col->setSuppressDataChangedSignal(true);
		if (formulaModeActive()) {
			for (const auto& i: selectedRows().intervals())
				col->setFormula(i, "");
		} else {
			for (const auto& i: selectedRows().intervals()) {
				if (i.end() == col->rowCount()-1)
					col->removeRows(i.start(), i.size());
				else {
					QVector<QString> empties;
					for (int j = 0; j < i.size(); j++)
						empties << QString();
					col->asStringColumn()->replaceTexts(i.start(), empties);
				}
			}
		}

		col->setSuppressDataChangedSignal(false);
		col->setChanged();
	}
	m_spreadsheet->endMacro();
	RESET_CURSOR;

	//selected rows were deleted but the view selection is still in place -> reset the selection in the view
	m_tableView->clearSelection();
}

void SpreadsheetView::clearSelectedCells() {
	int first = firstSelectedRow();
	int last = lastSelectedRow();
	if (first < 0) return;

	WAIT_CURSOR;
	m_spreadsheet->beginMacro(i18n("%1: clear selected cells", m_spreadsheet->name()));
	for (auto* column : selectedColumns()) {
		column->setSuppressDataChangedSignal(true);
		if (formulaModeActive()) {
			int col = m_spreadsheet->indexOfChild<Column>(column);
			for (int row = last; row >= first; row--)
				if (isCellSelected(row, col))
					column->setFormula(row, "");
		} else {
			int col = m_spreadsheet->indexOfChild<Column>(column);
			for (int row = last; row >= first; row--)
				if (isCellSelected(row, col)) {
					if (row < column->rowCount())
						column->asStringColumn()->setTextAt(row, QString());
				}
		}
		column->setSuppressDataChangedSignal(false);
		column->setChanged();
	}
	m_spreadsheet->endMacro();
	RESET_CURSOR;
}

void SpreadsheetView::goToCell() {
	bool ok;

	int col = QInputDialog::getInt(0, i18n("Go to Cell"), i18n("Enter column"), 1, 1, m_spreadsheet->columnCount(), 1, &ok);
	if (!ok) return;

	int row = QInputDialog::getInt(0, i18n("Go to Cell"), i18n("Enter row"), 1, 1, m_spreadsheet->rowCount(), 1, &ok);
	if (!ok) return;

	goToCell(row-1, col-1);
}

//! Open the sort dialog for the given columns
void SpreadsheetView::sortDialog(QVector<Column*> cols) {
	if (cols.isEmpty()) return;

	for (auto* col: cols)
		col->setSuppressDataChangedSignal(true);

	SortDialog* dlg = new SortDialog();
	connect(dlg, SIGNAL(sort(Column*,QVector<Column*>,bool)), m_spreadsheet, SLOT(sortColumns(Column*,QVector<Column*>,bool)));
	dlg->setColumns(cols);
	int rc = dlg->exec();

	for (auto* col: cols) {
		col->setSuppressDataChangedSignal(false);
		if (rc == QDialog::Accepted)
			col->setChanged();
	}
}

void SpreadsheetView::sortColumnAscending() {
	QVector<Column*> cols = selectedColumns();
	for (auto* col : cols)
		col->setSuppressDataChangedSignal(true);
	m_spreadsheet->sortColumns(cols.first(), cols, true);
	for (auto* col: cols) {
		col->setSuppressDataChangedSignal(false);
		col->setChanged();
	}
}

void SpreadsheetView::sortColumnDescending() {
	QVector<Column*> cols = selectedColumns();
	for (auto* col : cols)
		col->setSuppressDataChangedSignal(true);
	m_spreadsheet->sortColumns(cols.first(), cols, false);
	for (auto* col: cols) {
		col->setSuppressDataChangedSignal(false);
		col->setChanged();
	}
}

/*!
  Cause a repaint of the header.
*/
void SpreadsheetView::updateHeaderGeometry(Qt::Orientation o, int first, int last) {
	Q_UNUSED(first)
	Q_UNUSED(last)
	//TODO
	if (o != Qt::Horizontal) return;
	m_tableView->horizontalHeader()->setStretchLastSection(true);  // ugly hack (flaw in Qt? Does anyone know a better way?)
	m_tableView->horizontalHeader()->updateGeometry();
	m_tableView->horizontalHeader()->setStretchLastSection(false); // ugly hack part 2
}

/*!
  selects the column \c column in the speadsheet view .
*/
void SpreadsheetView::selectColumn(int column) {
	QItemSelection selection(m_model->index(0, column), m_model->index(m_spreadsheet->rowCount()-1, column) );
	m_suppressSelectionChangedEvent = true;
	m_tableView->selectionModel()->select(selection, QItemSelectionModel::Select);
	m_suppressSelectionChangedEvent = false;
}

/*!
  deselects the column \c column in the speadsheet view .
*/
void SpreadsheetView::deselectColumn(int column) {
	QItemSelection selection(m_model->index(0, column), m_model->index(m_spreadsheet->rowCount()-1, column) );
	m_suppressSelectionChangedEvent = true;
	m_tableView->selectionModel()->select(selection, QItemSelectionModel::Deselect);
	m_suppressSelectionChangedEvent = false;
}

/*!
  called when a column in the speadsheet view was clicked (click in the header).
  Propagates the selection of the column to the \c Spreadsheet object
  (a click in the header always selects the column).
*/
void SpreadsheetView::columnClicked(int column) {
	m_spreadsheet->setColumnSelectedInView(column, true);
}

/*!
  called on selections changes. Propagates the selection/deselection of columns to the \c Spreadsheet object.
*/
void SpreadsheetView::selectionChanged(const QItemSelection &selected, const QItemSelection &deselected) {
	Q_UNUSED(selected);
	Q_UNUSED(deselected);

	if (m_suppressSelectionChangedEvent)
		return;

	QItemSelectionModel* selModel = m_tableView->selectionModel();
	for (int i=0; i<m_spreadsheet->columnCount(); i++)
		m_spreadsheet->setColumnSelectedInView(i, selModel->isColumnSelected(i, QModelIndex()));
}

bool SpreadsheetView::exportView() {
	ExportSpreadsheetDialog* dlg = new ExportSpreadsheetDialog(this);
	dlg->setFileName(m_spreadsheet->name());

	dlg->setExportTo(QStringList() << i18n("FITS image") << i18n("FITS table"));
	for (int i = 0; i < m_spreadsheet->columnCount(); ++i) {
		if (m_spreadsheet->column(i)->columnMode() != AbstractColumn::Numeric) {
			dlg->setExportToImage(false);
			break;
		}
	}
	if (selectedColumnCount() == 0)
		dlg->setExportSelection(false);

	bool ret;
	if ((ret = dlg->exec()) == QDialog::Accepted) {
		const QString path = dlg->path();
		const bool exportHeader = dlg->exportHeader();
		WAIT_CURSOR;
		if (dlg->format() == ExportSpreadsheetDialog::LaTeX) {
			const bool exportLatexHeader = dlg->exportLatexHeader();
			const bool gridLines = dlg->gridLines();
			const bool captions = dlg->captions();
			const bool skipEmptyRows =dlg->skipEmptyRows();
			const bool exportEntire = dlg->entireSpreadheet();
			exportToLaTeX(path, exportHeader, gridLines, captions,
			                    exportLatexHeader, skipEmptyRows, exportEntire);
		} else if (dlg->format() == ExportSpreadsheetDialog::FITS) {
			const int exportTo = dlg->exportToFits();
			const bool commentsAsUnits = dlg->commentsAsUnitsFits();
			exportToFits(path, exportTo, commentsAsUnits);
		} else {
			const QString separator = dlg->separator();
			exportToFile(path, exportHeader, separator);
		}
		RESET_CURSOR;
	}
	delete dlg;

	return ret;
}

bool SpreadsheetView::printView() {
	QPrinter printer;
	QPrintDialog* dlg = new QPrintDialog(&printer, this);
	dlg->setWindowTitle(i18nc("@title:window", "Print Spreadsheet"));

	bool ret;
	if ((ret = dlg->exec()) == QDialog::Accepted) {
		print(&printer);
	}
	delete dlg;
	return ret;
}

bool SpreadsheetView::printPreview() {
	QPrintPreviewDialog* dlg = new QPrintPreviewDialog(this);
	connect(dlg, &QPrintPreviewDialog::paintRequested, this, &SpreadsheetView::print);
	return dlg->exec();
}

/*!
  prints the complete spreadsheet to \c printer.
 */
void SpreadsheetView::print(QPrinter* printer) const {
	WAIT_CURSOR;
	QPainter painter (printer);

	const int dpiy = printer->logicalDpiY();
	const int margin = (int) ( (1/2.54)*dpiy ); // 1 cm margins

	QHeaderView *hHeader = m_tableView->horizontalHeader();
	QHeaderView *vHeader = m_tableView->verticalHeader();

	const int rows = m_spreadsheet->rowCount();
	const int cols = m_spreadsheet->columnCount();
	int height = margin;
	int i;
	const int vertHeaderWidth = vHeader->width();
	int right = margin + vertHeaderWidth;

	int columnsPerTable = 0;
	int headerStringWidth = 0;
	int firstRowStringWidth = 0;
	bool tablesNeeded = false;
	for (int col = 0; col < cols; ++col) {
		headerStringWidth += m_tableView->columnWidth(col);
		firstRowStringWidth += m_spreadsheet->column(col)->asStringColumn()->textAt(0).length();
		if ((headerStringWidth >= printer->pageRect().width() -2*margin) ||
		        (firstRowStringWidth >= printer->pageRect().width() - 2*margin)) {
			tablesNeeded = true;
			break;
		}
		columnsPerTable++;
	}

	int tablesCount = (columnsPerTable != 0) ? cols/columnsPerTable : 0;
	const int remainingColumns = (columnsPerTable != 0) ? cols % columnsPerTable : cols;

	if (!tablesNeeded) {
		tablesCount = 1;
		columnsPerTable = cols;
	}

	if (remainingColumns > 0)
		tablesCount++;
	//Paint the horizontal header first
	for (int table = 0; table < tablesCount; ++table) {
		right = margin + vertHeaderWidth;

		painter.setFont(hHeader->font());
		QString headerString = m_tableView->model()->headerData(0, Qt::Horizontal).toString();
		QRect br;
		br = painter.boundingRect(br, Qt::AlignCenter, headerString);
		QRect tr(br);
		if (table != 0)
			height += tr.height();
		painter.drawLine(right, height, right, height+br.height());

		int w;
		i = table * columnsPerTable;
		int toI = table * columnsPerTable + columnsPerTable;
		if ((remainingColumns > 0) && (table == tablesCount-1)) {
			i = (tablesCount-1)*columnsPerTable;
			toI = (tablesCount-1)* columnsPerTable + remainingColumns;
		}

		for (; i<toI; ++i) {
			headerString = m_tableView->model()->headerData(i, Qt::Horizontal).toString();
			w = m_tableView->columnWidth(i);
			tr.setTopLeft(QPoint(right,height));
			tr.setWidth(w);
			tr.setHeight(br.height());

			painter.drawText(tr, Qt::AlignCenter, headerString);
			right += w;
			painter.drawLine(right, height, right, height+tr.height());

		}

		painter.drawLine(margin + vertHeaderWidth, height, right-1, height);//first horizontal line
		height += tr.height();
		painter.drawLine(margin, height, right-1, height);

		// print table values
		QString cellText;
		for (i=0; i<rows; ++i) {
			right = margin;
			cellText = m_tableView->model()->headerData(i, Qt::Vertical).toString()+'\t';
			tr = painter.boundingRect(tr, Qt::AlignCenter, cellText);
			painter.drawLine(right, height, right, height+tr.height());

			br.setTopLeft(QPoint(right,height));
			br.setWidth(vertHeaderWidth);
			br.setHeight(tr.height());
			painter.drawText(br, Qt::AlignCenter, cellText);
			right += vertHeaderWidth;
			painter.drawLine(right, height, right, height+tr.height());
			int j = table * columnsPerTable;
			int toJ = table * columnsPerTable + columnsPerTable;
			if ((remainingColumns > 0) && (table == tablesCount-1)) {
				j = (tablesCount-1)*columnsPerTable;
				toJ = (tablesCount-1)* columnsPerTable + remainingColumns;
			}
			for (; j< toJ; j++) {
				int w = m_tableView->columnWidth(j);
				cellText = m_spreadsheet->column(j)->isValid(i) ? m_spreadsheet->text(i,j)+'\t':
				           QLatin1String("- \t");
				tr = painter.boundingRect(tr,Qt::AlignCenter,cellText);
				br.setTopLeft(QPoint(right,height));
				br.setWidth(w);
				br.setHeight(tr.height());
				painter.drawText(br, Qt::AlignCenter, cellText);
				right += w;
				painter.drawLine(right, height, right, height+tr.height());

			}
			height += br.height();
			painter.drawLine(margin, height, right-1, height);

			if (height >= printer->height()-margin ) {
				printer->newPage();
				height = margin;
				painter.drawLine(margin, height, right, height);
			}
		}
	}
	RESET_CURSOR;
}

void SpreadsheetView::exportToFile(const QString& path, const bool exportHeader, const QString& separator) const {
	QFile file(path);
	if (!file.open(QFile::WriteOnly | QFile::Truncate))
		return;

	PERFTRACE("export spreadsheet to file");
	QTextStream out(&file);
	const int cols = m_spreadsheet->columnCount();

	QString sep = separator;
	sep = sep.replace(QLatin1String("TAB"), QLatin1String("\t"), Qt::CaseInsensitive);
	sep = sep.replace(QLatin1String("SPACE"), QLatin1String(" "), Qt::CaseInsensitive);

	//export header (column names)
	if (exportHeader) {
		for (int j = 0; j < cols; ++j) {
			out << m_spreadsheet->column(j)->name();
			if (j != cols-1)
				out << sep;
		}
		out << '\n';
	}

	//export values
	for (int i = 0; i < m_spreadsheet->rowCount(); ++i) {
		for (int j = 0; j < cols; ++j) {
			out << m_spreadsheet->column(j)->asStringColumn()->textAt(i);
			if (j != cols-1)
				out << sep;
		}
		out << '\n';
	}
}

void SpreadsheetView::exportToLaTeX(const QString & path, const bool exportHeaders,
                                    const bool gridLines, const bool captions, const bool latexHeaders,
                                    const bool skipEmptyRows, const bool exportEntire) const {
	QFile file(path);
	if (!file.open(QFile::WriteOnly | QFile::Truncate))
		return;

	QList<Column*> toExport;
	int cols;
	int totalRowCount = 0;
	if (exportEntire) {
		cols = const_cast<SpreadsheetView*>(this)->m_spreadsheet->columnCount();
		totalRowCount = m_spreadsheet->rowCount();
		for (int col = 0; col < cols; ++col)
			toExport << m_spreadsheet->column(col);
	} else {
		cols = const_cast<SpreadsheetView*>(this)->selectedColumnCount();
		const int firtsSelectedCol = const_cast<SpreadsheetView*>(this)->firstSelectedColumn();
		bool rowsCalculated = false;
		for (int col = firtsSelectedCol; col < firtsSelectedCol + cols; ++col) {
			QVector<QString> textData;
			for (int row = 0; row < m_spreadsheet->rowCount(); ++row) {
				if (const_cast<SpreadsheetView*>(this)->isRowSelected(row)) {
					textData << m_spreadsheet->column(col)->asStringColumn()->textAt(row);
					if (!rowsCalculated)
						totalRowCount++;
				}
			}
			if (!rowsCalculated)
				rowsCalculated = true;
			Column* column = new Column(m_spreadsheet->column(col)->name(), textData);
			toExport << column;
		}
	}
	int columnsStringSize = 0;
	int columnsPerTable = 0;

	for (int i = 0; i < cols; ++i) {
		int maxSize = -1;
		for (int j = 0; j < toExport.at(i)->asStringColumn()->rowCount(); ++j) {
			if (toExport.at(i)->asStringColumn()->textAt(j).size() > maxSize)
				maxSize = toExport.at(i)->asStringColumn()->textAt(j).size();
		}
		columnsStringSize += maxSize;
		if (!toExport.at(i)->isValid(0))
			columnsStringSize += 3;
		if (columnsStringSize > 65)
			break;
		++columnsPerTable;
	}

	const int tablesCount = (columnsPerTable != 0) ? cols/columnsPerTable : 0;
	const int remainingColumns = (columnsPerTable != 0) ? cols % columnsPerTable : cols;

	bool columnsSeparating = (cols > columnsPerTable);
	QTextStream out(&file);

	QProcess tex;
	tex.start("latex", QStringList() << "--version", QProcess::ReadOnly);
	tex.waitForFinished(500);
	QString texVersionOutput = QString(tex.readAllStandardOutput());
	texVersionOutput = texVersionOutput.split('\n')[0];

	int yearidx = -1;
	for (int i = texVersionOutput.size() - 1; i >= 0; --i) {
		if (texVersionOutput.at(i) == QChar('2')) {
			yearidx = i;
			break;
		}
	}

	if (texVersionOutput.at(yearidx+1) == QChar('/'))
		yearidx-=3;

	bool ok;
	texVersionOutput.midRef(yearidx, 4).toInt(&ok);
	int version = -1;
	if (ok)
		version = texVersionOutput.midRef(yearidx, 4).toInt(&ok);

	if (latexHeaders) {
		out << QLatin1String("\\documentclass[11pt,a4paper]{article} \n");
		out << QLatin1String("\\usepackage{geometry} \n");
		out << QLatin1String("\\usepackage{xcolor,colortbl} \n");
		if (version >= 2015)
			out << QLatin1String("\\extrafloats{1280} \n");
		out << QLatin1String("\\definecolor{HeaderBgColor}{rgb}{0.81,0.81,0.81} \n");
		out << QLatin1String("\\geometry{ \n");
		out << QLatin1String("a4paper, \n");
		out << QLatin1String("total={170mm,257mm}, \n");
		out << QLatin1String("left=10mm, \n");
		out << QLatin1String("top=10mm } \n");

		out << QLatin1String("\\begin{document} \n");
		out << QLatin1String("\\title{LabPlot Spreadsheet Export to \\LaTeX{} } \n");
		out << QLatin1String("\\author{LabPlot} \n");
		out << QLatin1String("\\date{\\today} \n");
	}

	QString endTabularTable ("\\end{tabular} \n \\end{table} \n");
	QString tableCaption ("\\caption{"+ m_spreadsheet->name()+ "} \n");
	QString beginTable ("\\begin{table}[ht] \n");

	int rowCount = 0;
	const int maxRows = 45;
	bool captionRemoved = false;
	if (columnsSeparating) {
		QVector<int> emptyRowIndices;
		for (int table = 0; table < tablesCount; ++table) {
			QStringList textable;
			captionRemoved = false;
			textable << beginTable;

			if (captions)
				textable << tableCaption;
			textable << QLatin1String("\\centering \n");
			textable << QLatin1String("\\begin{tabular}{") << (gridLines ?QLatin1String("|") : QLatin1String(""));
			for (int i = 0; i < columnsPerTable; ++i)
				textable << ( gridLines ? QLatin1String(" c |") : QLatin1String(" c ") );
			textable << QLatin1String("} \n");
			if (gridLines)
				textable << QLatin1String("\\hline \n");

			if (exportHeaders) {
				if (latexHeaders)
					textable << QLatin1String("\\rowcolor{HeaderBgColor} \n");
				for (int col = table*columnsPerTable; col < (table * columnsPerTable) + columnsPerTable; ++col) {
					textable << toExport.at(col)->name();
					if (col != ((table * columnsPerTable)+ columnsPerTable)-1)
						textable << QLatin1String(" & ");
				}
				textable << QLatin1String("\\\\ \n");
				if (gridLines)
					textable << QLatin1String("\\hline \n");
			}
			for (const auto& s : textable)
				out << s;

			QStringList values;
			for (int row = 0; row < totalRowCount; ++row) {
				values.clear();
				bool notEmpty = false;
				for (int col = table*columnsPerTable; col < (table * columnsPerTable) + columnsPerTable; ++col ) {
					if (toExport.at(col)->isValid(row)) {
						notEmpty = true;
						values << toExport.at(col)->asStringColumn()->textAt(row);
					} else
						values << QLatin1String("-");
					if (col != ((table * columnsPerTable)+ columnsPerTable)-1)
						values << QLatin1String(" & ");
				}
				if (!notEmpty && skipEmptyRows) {
					if (!emptyRowIndices.contains(row))
						emptyRowIndices << row;
				}
				if (emptyRowIndices.contains(row) && notEmpty)
					emptyRowIndices.remove(emptyRowIndices.indexOf(row));

				if (notEmpty || !skipEmptyRows) {
					for (const auto& s : values)
						out << s;
					out << QLatin1String("\\\\ \n");
					if (gridLines)
						out << QLatin1String("\\hline \n");
					rowCount++;
					if (rowCount == maxRows) {
						out << endTabularTable;
						out << QLatin1String("\\newpage \n");

						if (captions)
							if (!captionRemoved)
								textable.removeAt(1);
						for (const auto& s : textable)
							out << s;
						rowCount = 0;
						if (!captionRemoved)
							captionRemoved = true;
					}
				}
			}
			out << endTabularTable;
		}

		//new table for the remaining columns
		QStringList remainingTable;
		remainingTable << beginTable;
		if (captions)
			remainingTable << tableCaption;
		remainingTable << QLatin1String("\\centering \n");
		remainingTable << QLatin1String("\\begin{tabular}{") <<  (gridLines ? QLatin1String("|"):QLatin1String(""));
		for (int c = 0; c < remainingColumns; ++c)
			remainingTable << ( gridLines ? QLatin1String(" c |") : QLatin1String(" c ") );
		remainingTable << QLatin1String("} \n");
		if (gridLines)
			remainingTable << QLatin1String("\\hline \n");
		if (exportHeaders) {
			if (latexHeaders)
				remainingTable << QLatin1String("\\rowcolor{HeaderBgColor} \n");
			for (int col = 0; col < remainingColumns; ++col) {
				remainingTable << toExport.at(col + (tablesCount * columnsPerTable))->name();
				if (col != remainingColumns-1)
					remainingTable << QLatin1String(" & ");
			}
			remainingTable << QLatin1String("\\\\ \n");
			if (gridLines)
				remainingTable << QLatin1String("\\hline \n");
		}

		for (const auto& s : remainingTable)
			out << s;

		QStringList values;
		captionRemoved = false;
		for (int row = 0; row < totalRowCount; ++row) {
			values.clear();
			bool notEmpty = false;
			for (int col = 0; col < remainingColumns; ++col ) {
				if (toExport.at(col + (tablesCount * columnsPerTable))->isValid(row)) {
					notEmpty = true;
					values << toExport.at(col + (tablesCount * columnsPerTable))->asStringColumn()->textAt(row);
				} else
					values << QLatin1String("-");
				if (col != remainingColumns-1)
					values << QLatin1String(" & ");
			}
			if (!emptyRowIndices.contains(row) && !notEmpty)
				notEmpty = true;
			if (notEmpty || !skipEmptyRows) {
				for (const auto& s : values)
					out << s;
				out << QLatin1String("\\\\ \n");
				if (gridLines)
					out << QLatin1String("\\hline \n");
				rowCount++;
				if (rowCount == maxRows) {
					out << endTabularTable;
					out << QLatin1String("\\pagebreak[4] \n");
					if (captions)
						if (!captionRemoved)
							remainingTable.removeAt(1);
					for (const auto& s : remainingTable)
						out << s;
					rowCount = 0;
					if (!captionRemoved)
						captionRemoved = true;
				}
			}
		}
		out << endTabularTable;
	} else {
		QStringList textable;
		textable << beginTable;
		if (captions)
			textable << tableCaption;
		textable << QLatin1String("\\centering \n");
		textable << QLatin1String("\\begin{tabular}{") << (gridLines ? QLatin1String("|"):QLatin1String(""));
		for (int c = 0; c < cols; ++c)
			textable << ( gridLines ? QLatin1String(" c |") : QLatin1String(" c ") );
		textable << QLatin1String("} \n");
		if (gridLines)
			textable << QLatin1String("\\hline \n");
		if (exportHeaders) {
			if (latexHeaders)
				textable << QLatin1String("\\rowcolor{HeaderBgColor} \n");
			for (int col = 0; col < cols; ++col) {
				textable << toExport.at(col)->name();
				if (col != cols-1)
					textable << QLatin1String(" & ");
			}
			textable << QLatin1String("\\\\ \n");
			if (gridLines)
				textable << QLatin1String("\\hline \n");
		}

		for (const auto& s : textable)
			out << s;
		QStringList values;
		captionRemoved = false;
		for (int row = 0; row < totalRowCount; ++row) {
			values.clear();
			bool notEmpty = false;

			for (int col = 0; col < cols; ++col ) {
				if (toExport.at(col)->isValid(row)) {
					notEmpty = true;
					values << toExport.at(col)->asStringColumn()->textAt(row);
				} else
					values << "-";
				if (col != cols-1)
					values << " & ";
			}

			if (notEmpty || !skipEmptyRows) {
				for (const auto& s : values)
					out << s;
				out << QLatin1String("\\\\ \n");
				if (gridLines)
					out << QLatin1String("\\hline \n");
				rowCount++;
				if (rowCount == maxRows) {
					out << endTabularTable;
					out << QLatin1String("\\newpage \n");
					if (captions)
						if (!captionRemoved)
							textable.removeAt(1);
					for (const auto& s : textable)
						out << s;
					rowCount = 0;
					if (!captionRemoved)
						captionRemoved = true;
				}
			}
		}
		out << endTabularTable;
	}
	if (latexHeaders)
		out << QLatin1String("\\end{document} \n");

	if (!exportEntire) {
		qDeleteAll(toExport);
		toExport.clear();
	} else
		toExport.clear();
}

void SpreadsheetView::exportToFits(const QString &fileName, const int exportTo, const bool commentsAsUnits) const {
	FITSFilter* filter = new FITSFilter;

	filter->setExportTo(exportTo);
	filter->setCommentsAsUnits(commentsAsUnits);
	filter->write(fileName, m_spreadsheet);

	delete filter;
}
