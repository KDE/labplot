/***************************************************************************
    File                 : SpreadsheetView.cpp
    Project              : LabPlot
    Description          : View class for Spreadsheet
    --------------------------------------------------------------------
    Copyright            : (C) 2011-2020 by Alexander Semke (alexander.semke@web.de)
    Copyright            : (C) 2016      by Fabian Kristof (fkristofszabolcs@gmail.com)
    Copyright            : (C) 2020 by Stefan Gerlach (stefan.gerlach@uni.kn)

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
#include "backend/worksheet/plots/cartesian/CartesianPlot.h"
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
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QTableView>
#include <QToolBar>
#include <QTextStream>
#include <QProcess>
#include <QRegularExpression>

#include <KConfigGroup>
#include <KLocalizedString>
#include <KMessageBox>
#include <KSharedConfig>

#include "kdefrontend/spreadsheet/ExportSpreadsheetDialog.h"
#include "kdefrontend/spreadsheet/PlotDataDialog.h"
#include "kdefrontend/spreadsheet/AddSubtractValueDialog.h"
#include "kdefrontend/spreadsheet/DropValuesDialog.h"
#include "kdefrontend/spreadsheet/RescaleDialog.h"
#include "kdefrontend/spreadsheet/SortDialog.h"
#include "kdefrontend/spreadsheet/RandomValuesDialog.h"
#include "kdefrontend/spreadsheet/EquidistantValuesDialog.h"
#include "kdefrontend/spreadsheet/FunctionValuesDialog.h"
#include "kdefrontend/spreadsheet/StatisticsDialog.h"

#include <algorithm> //for std::reverse

#ifdef Q_OS_MAC
#include "3rdparty/kdmactouchbar/src/kdmactouchbar.h"
#endif

enum NormalizationMethod {DivideBySum, DivideByMin, DivideByMax, DivideByCount,
						DivideByMean, DivideByMedian, DivideByMode, DivideByRange,
						DivideBySD, DivideByMAD, DivideByIQR,
						ZScoreSD, ZScoreMAD, ZScoreIQR,
						Rescale};
/*!
	\class SpreadsheetView
	\brief View class for Spreadsheet

	\ingroup commonfrontend
 */
SpreadsheetView::SpreadsheetView(Spreadsheet* spreadsheet, bool readOnly) : QWidget(),
	m_tableView(new QTableView(this)),
	m_spreadsheet(spreadsheet),
	m_model(new SpreadsheetModel(spreadsheet)),
	m_readOnly(readOnly) {

	auto* layout = new QHBoxLayout(this);
	layout->setContentsMargins(0,0,0,0);
	layout->addWidget(m_tableView);
	if (m_readOnly)
		m_tableView->setEditTriggers(QTableView::NoEditTriggers);
	init();

	//resize the view to show alls columns and the first 10 rows.
	//no need to resize the view when the project is being opened,
	//all views will be resized to the stored values at the end
	if (!m_spreadsheet->isLoading()) {
		int w = m_tableView->verticalHeader()->width();
		int h = m_horizontalHeader->height();
		for (int i = 0; i < m_horizontalHeader->count(); ++i)
			w += m_horizontalHeader->sectionSize(i);

		if (m_tableView->verticalHeader()->count() <= 10)
			h += m_tableView->verticalHeader()->sectionSize(0)*m_tableView->verticalHeader()->count();
		else
			h += m_tableView->verticalHeader()->sectionSize(0)*11;

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

	connect(m_horizontalHeader, &SpreadsheetHeaderView::sectionMoved, this, &SpreadsheetView::handleHorizontalSectionMoved);
	connect(m_horizontalHeader, &SpreadsheetHeaderView::sectionDoubleClicked, this, &SpreadsheetView::handleHorizontalHeaderDoubleClicked);
	connect(m_horizontalHeader, &SpreadsheetHeaderView::sectionResized, this, &SpreadsheetView::handleHorizontalSectionResized);
	connect(m_horizontalHeader, &SpreadsheetHeaderView::sectionClicked, this, &SpreadsheetView::columnClicked);

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

	connect(m_model, &SpreadsheetModel::headerDataChanged, this, &SpreadsheetView::handleHeaderDataChanged);
	connect(m_spreadsheet, &Spreadsheet::aspectAdded, this, &SpreadsheetView::handleAspectAdded);
	connect(m_spreadsheet, &Spreadsheet::aspectAboutToBeRemoved,this, &SpreadsheetView::handleAspectAboutToBeRemoved);
	connect(m_spreadsheet, &Spreadsheet::requestProjectContextMenu, this, &SpreadsheetView::createContextMenu);

	for (auto* column : m_spreadsheet->children<Column>())
		connect(column, &Column::requestProjectContextMenu, this, &SpreadsheetView::createColumnContextMenu);

	//selection relevant connections
	QItemSelectionModel* sel_model = m_tableView->selectionModel();
	connect(sel_model, &QItemSelectionModel::currentColumnChanged, this, &SpreadsheetView::currentColumnChanged);
	connect(sel_model, &QItemSelectionModel::selectionChanged, this, &SpreadsheetView::selectionChanged);
	connect(sel_model, &QItemSelectionModel::selectionChanged, this, &SpreadsheetView::selectionChanged);

	connect(m_spreadsheet, &Spreadsheet::columnSelected, this, &SpreadsheetView::selectColumn);
	connect(m_spreadsheet, &Spreadsheet::columnDeselected, this, &SpreadsheetView::deselectColumn);
}

/*!
	set the column sizes to the saved values or resize to content if no size was saved yet
*/
void SpreadsheetView::resizeHeader() {
	DEBUG("SpreadsheetView::resizeHeader()");
	const auto columns = m_spreadsheet->children<Column>();
	for (int i = 0; i < columns.size(); ++i) {
		const Column* col = columns.at(i);
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

// 	action_set_formula = new QAction(QIcon::fromTheme(QString()), i18n("Assign &Formula"), this);
// 	action_recalculate = new QAction(QIcon::fromTheme(QString()), i18n("Recalculate"), this);
	action_fill_sel_row_numbers = new QAction(QIcon::fromTheme(QString()), i18n("Row Numbers"), this);
	action_fill_row_numbers = new QAction(QIcon::fromTheme(QString()), i18n("Row Numbers"), this);
	action_fill_random = new QAction(QIcon::fromTheme(QString()), i18n("Uniform Random Values"), this);
	action_fill_random_nonuniform = new QAction(QIcon::fromTheme(QString()), i18n("Random Values"), this);
	action_fill_equidistant = new QAction(QIcon::fromTheme(QString()), i18n("Equidistant Values"), this);
	action_fill_function = new QAction(QIcon::fromTheme(QString()), i18n("Function Values"), this);
	action_fill_const = new QAction(QIcon::fromTheme(QString()), i18n("Const Values"), this);

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
	action_insert_columns_left = new QAction(QIcon::fromTheme("edit-table-insert-column-left"), i18n("Insert Multiple Columns Left"), this);
	action_insert_columns_right = new QAction(QIcon::fromTheme("edit-table-insert-column-right"), i18n("Insert Multiple Columns Right"), this);
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

	//data manipulation
	action_add_value = new QAction(i18n("Add Value"), this);
	action_add_value->setData(AddSubtractValueDialog::Add);
	action_subtract_value = new QAction(i18n("Subtract Value"), this);
	action_subtract_value->setData(AddSubtractValueDialog::Subtract);
	action_multiply_value = new QAction(i18n("Multiply by Value"), this);
	action_multiply_value->setData(AddSubtractValueDialog::Multiply);
	action_divide_value = new QAction(i18n("Divide by Value"), this);
	action_divide_value->setData(AddSubtractValueDialog::Divide);
	action_drop_values = new QAction(QIcon::fromTheme(QString()), i18n("Drop Values"), this);
	action_mask_values = new QAction(QIcon::fromTheme(QString()), i18n("Mask Values"), this);
	action_reverse_columns = new QAction(QIcon::fromTheme(QString()), i18n("Reverse"), this);
// 	action_join_columns = new QAction(QIcon::fromTheme(QString()), i18n("Join"), this);

	normalizeColumnActionGroup = new QActionGroup(this);
	QAction* normalizeAction = new QAction(i18n("Divide by Sum"), normalizeColumnActionGroup);
	normalizeAction->setData(DivideBySum);

	normalizeAction = new QAction(i18n("Divide by Min"), normalizeColumnActionGroup);
	normalizeAction->setData(DivideByMin);

	normalizeAction = new QAction(i18n("Divide by Max"), normalizeColumnActionGroup);
	normalizeAction->setData(DivideByMax);

	normalizeAction = new QAction(i18n("Divide by Count"), normalizeColumnActionGroup);
	normalizeAction->setData(DivideByCount);

	normalizeAction = new QAction(i18n("Divide by Mean"), normalizeColumnActionGroup);
	normalizeAction->setData(DivideByMean);

	normalizeAction = new QAction(i18n("Divide by Median"), normalizeColumnActionGroup);
	normalizeAction->setData(DivideByMedian);

	normalizeAction = new QAction(i18n("Divide by Mode"), normalizeColumnActionGroup);
	normalizeAction->setData(DivideByMode);

	normalizeAction = new QAction(i18n("Divide by Range"), normalizeColumnActionGroup);
	normalizeAction->setData(DivideByRange);

	normalizeAction = new QAction(i18n("Divide by SD"), normalizeColumnActionGroup);
	normalizeAction->setData(DivideBySD);

	normalizeAction = new QAction(i18n("Divide by MAD"), normalizeColumnActionGroup);
	normalizeAction->setData(DivideByMAD);

	normalizeAction = new QAction(i18n("Divide by IQR"), normalizeColumnActionGroup);
	normalizeAction->setData(DivideByIQR);

	normalizeAction = new QAction(QLatin1String("(x-Mean)/SD"), normalizeColumnActionGroup);
	normalizeAction->setData(ZScoreSD);

	normalizeAction = new QAction(QLatin1String("(x-Median)/MAD"), normalizeColumnActionGroup);
	normalizeAction->setData(ZScoreMAD);

	normalizeAction = new QAction(QLatin1String("(x-Median)/IQR"), normalizeColumnActionGroup);
	normalizeAction->setData(ZScoreIQR);

	normalizeAction = new QAction(QLatin1String("Rescale to [a, b]"), normalizeColumnActionGroup);
	normalizeAction->setData(Rescale);

	action_normalize_selection = new QAction(QIcon::fromTheme(QString()), i18n("&Normalize Selection"), this);

	//sort and statistics
	action_sort_columns = new QAction(QIcon::fromTheme(QString()), i18n("&Selected Columns"), this);
	action_sort_asc_column = new QAction(QIcon::fromTheme("view-sort-ascending"), i18n("&Ascending"), this);
	action_sort_desc_column = new QAction(QIcon::fromTheme("view-sort-descending"), i18n("&Descending"), this);
	action_statistics_columns = new QAction(QIcon::fromTheme("view-statistics"), i18n("Column Statisti&cs"), this);

	// row related actions
	action_insert_row_above = new QAction(QIcon::fromTheme("edit-table-insert-row-above") ,i18n("Insert Row Above"), this);
	action_insert_row_below = new QAction(QIcon::fromTheme("edit-table-insert-row-below"), i18n("Insert Row Below"), this);
	action_insert_rows_above = new QAction(QIcon::fromTheme("edit-table-insert-row-above") ,i18n("Insert Multiple Rows Above"), this);
	action_insert_rows_below = new QAction(QIcon::fromTheme("edit-table-insert-row-below"), i18n("Insert Multiple Rows Below"), this);
	action_remove_rows = new QAction(QIcon::fromTheme("edit-table-delete-row"), i18n("Remo&ve Selected Rows"), this);
	action_clear_rows = new QAction(QIcon::fromTheme("edit-clear"), i18n("Clea&r Selected Rows"), this);
	action_statistics_rows = new QAction(QIcon::fromTheme("view-statistics"), i18n("Row Statisti&cs"), this);

	//plot data action
	action_plot_data_xycurve = new QAction(QIcon::fromTheme("labplot-xy-curve"), i18n("xy-Curve"), this);
	action_plot_data_xycurve->setData(PlotDataDialog::PlotXYCurve);
	action_plot_data_histogram = new QAction(QIcon::fromTheme("view-object-histogram-linear"), i18n("Histogram"), this);
	action_plot_data_histogram->setData(PlotDataDialog::PlotHistogram);

	//Analyze and plot menu actions
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
}

void SpreadsheetView::initMenus() {
	//Selection menu
	m_selectionMenu = new QMenu(i18n("Selection"), this);
	m_selectionMenu->setIcon(QIcon::fromTheme("selection"));
	QMenu* submenu = nullptr;

	if (!m_readOnly) {
		submenu = new QMenu(i18n("Fi&ll Selection With"), this);
		submenu->setIcon(QIcon::fromTheme("select-rectangle"));
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

	//plot data menu
	m_plotDataMenu = new QMenu(i18n("Plot Data"), this);
	m_plotDataMenu->addAction(action_plot_data_xycurve);
	m_plotDataMenu->addAction(action_plot_data_histogram);

	// Column menu
	m_columnMenu = new QMenu(this);
	m_columnMenu->addMenu(m_plotDataMenu);

	// Data fit sub-menu
	QMenu* dataFitMenu = new QMenu(i18n("Fit"), this);
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
	m_analyzePlotMenu = new QMenu(i18n("Analyze and Plot Data"), this);
	m_analyzePlotMenu->addMenu(dataFitMenu);
	m_analyzePlotMenu->addSeparator();
	m_analyzePlotMenu->addAction(addDifferentiationAction);
	m_analyzePlotMenu->addAction(addIntegrationAction);
	m_analyzePlotMenu->addSeparator();
	m_analyzePlotMenu->addAction(addInterpolationAction);
	m_analyzePlotMenu->addAction(addSmoothAction);
	m_analyzePlotMenu->addSeparator();
	m_analyzePlotMenu->addAction(addFourierFilterAction);
	m_analyzePlotMenu->addSeparator();
	m_analyzePlotMenu->addAction(addDataReductionAction);
	m_columnMenu->addMenu(m_analyzePlotMenu);

	m_columnSetAsMenu = new QMenu(i18n("Set Column As"), this);
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

		m_columnManipulateDataMenu = new QMenu(i18n("Manipulate Data"), this);
		m_columnManipulateDataMenu->addAction(action_add_value);
		m_columnManipulateDataMenu->addAction(action_subtract_value);
		m_columnManipulateDataMenu->addAction(action_multiply_value);
		m_columnManipulateDataMenu->addAction(action_divide_value);
		m_columnManipulateDataMenu->addSeparator();
		m_columnManipulateDataMenu->addAction(action_reverse_columns);
		m_columnManipulateDataMenu->addSeparator();
		m_columnManipulateDataMenu->addAction(action_drop_values);
		m_columnManipulateDataMenu->addAction(action_mask_values);
		m_columnManipulateDataMenu->addSeparator();
		// 	m_columnManipulateDataMenu->addAction(action_join_columns);

		//normalization menu with the following structure
		//Divide by Sum
		//Divide by Min
		//Divide by Max
		//Divide by Count
		//--------------
		//Divide by Mean
		//Divide by Median
		//Divide by Mode
		//---------------
		//Divide by Range
		//Divide by SD
		//Divide by MAD
		//Divide by IQR
		//--------------
		//(x-Mean)/SD
		//(x-Median)/MAD
		//(x-Median)/IQR
		//--------------
		//Rescale to [a, b]

		m_columnNormalizeMenu = new QMenu(i18n("Normalize"), this);
		m_columnNormalizeMenu->addAction(normalizeColumnActionGroup->actions().at(0));
		m_columnNormalizeMenu->addAction(normalizeColumnActionGroup->actions().at(1));
		m_columnNormalizeMenu->addAction(normalizeColumnActionGroup->actions().at(2));
		m_columnNormalizeMenu->addAction(normalizeColumnActionGroup->actions().at(3));
		m_columnNormalizeMenu->addSeparator();
		m_columnNormalizeMenu->addAction(normalizeColumnActionGroup->actions().at(4));
		m_columnNormalizeMenu->addAction(normalizeColumnActionGroup->actions().at(5));
		m_columnNormalizeMenu->addAction(normalizeColumnActionGroup->actions().at(6));
		m_columnNormalizeMenu->addSeparator();
		m_columnNormalizeMenu->addAction(normalizeColumnActionGroup->actions().at(7));
		m_columnNormalizeMenu->addAction(normalizeColumnActionGroup->actions().at(8));
		m_columnNormalizeMenu->addAction(normalizeColumnActionGroup->actions().at(9));
		m_columnNormalizeMenu->addAction(normalizeColumnActionGroup->actions().at(10));
		m_columnNormalizeMenu->addSeparator();
		m_columnNormalizeMenu->addAction(normalizeColumnActionGroup->actions().at(11));
		m_columnNormalizeMenu->addAction(normalizeColumnActionGroup->actions().at(12));
		m_columnNormalizeMenu->addAction(normalizeColumnActionGroup->actions().at(13));
		m_columnNormalizeMenu->addSeparator();
		m_columnNormalizeMenu->addAction(normalizeColumnActionGroup->actions().at(14));
		m_columnManipulateDataMenu->addMenu(m_columnNormalizeMenu);

		m_columnMenu->addMenu(m_columnManipulateDataMenu);
		m_columnMenu->addSeparator();

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
		m_columnMenu->addAction(action_insert_columns_left);
		m_columnMenu->addAction(action_insert_columns_right);
		m_columnMenu->addSeparator();
		m_columnMenu->addAction(action_remove_columns);
		m_columnMenu->addAction(action_clear_columns);
	}
	m_columnMenu->addSeparator();
	m_columnMenu->addAction(action_toggle_comments);
	m_columnMenu->addSeparator();

	m_columnMenu->addAction(action_statistics_columns);

	//Spreadsheet menu
	m_spreadsheetMenu = new QMenu(this);
	m_spreadsheetMenu->addMenu(m_plotDataMenu);
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

		m_rowMenu->addAction(action_insert_rows_above);
		m_rowMenu->addAction(action_insert_rows_below);
		m_rowMenu->addSeparator();

		m_rowMenu->addAction(action_remove_rows);
		m_rowMenu->addAction(action_clear_rows);
	}
	m_rowMenu->addSeparator();
	m_rowMenu->addAction(action_statistics_rows);
	action_statistics_rows->setVisible(false);
}

void SpreadsheetView::connectActions() {
	connect(action_cut_selection, &QAction::triggered, this, &SpreadsheetView::cutSelection);
	connect(action_copy_selection, &QAction::triggered, this, &SpreadsheetView::copySelection);
	connect(action_paste_into_selection, &QAction::triggered, this, &SpreadsheetView::pasteIntoSelection);
	connect(action_mask_selection, &QAction::triggered, this, &SpreadsheetView::maskSelection);
	connect(action_unmask_selection, &QAction::triggered, this, &SpreadsheetView::unmaskSelection);

	connect(action_clear_selection, &QAction::triggered, this, &SpreadsheetView::clearSelectedCells);
// 	connect(action_recalculate, &QAction::triggered, this, &SpreadsheetView::recalculateSelectedCells);
	connect(action_fill_row_numbers, &QAction::triggered, this, &SpreadsheetView::fillWithRowNumbers);
	connect(action_fill_sel_row_numbers, &QAction::triggered, this, &SpreadsheetView::fillSelectedCellsWithRowNumbers);
// 	connect(action_fill_random, &QAction::triggered, this, &SpreadsheetView::fillSelectedCellsWithRandomNumbers);
	connect(action_fill_random_nonuniform, &QAction::triggered, this, &SpreadsheetView::fillWithRandomValues);
	connect(action_fill_equidistant, &QAction::triggered, this, &SpreadsheetView::fillWithEquidistantValues);
	connect(action_fill_function, &QAction::triggered, this, &SpreadsheetView::fillWithFunctionValues);
	connect(action_fill_const, &QAction::triggered, this, &SpreadsheetView::fillSelectedCellsWithConstValues);
	connect(action_select_all, &QAction::triggered, m_tableView, &QTableView::selectAll);
	connect(action_clear_spreadsheet, &QAction::triggered, m_spreadsheet, &Spreadsheet::clear);
	connect(action_clear_masks, &QAction::triggered, m_spreadsheet, &Spreadsheet::clearMasks);
	connect(action_sort_spreadsheet, &QAction::triggered, this, &SpreadsheetView::sortSpreadsheet);
	connect(action_go_to_cell, &QAction::triggered, this,
			static_cast<void (SpreadsheetView::*)()>(&SpreadsheetView::goToCell));

	connect(action_insert_column_left, &QAction::triggered, this, &SpreadsheetView::insertColumnLeft);
	connect(action_insert_column_right, &QAction::triggered, this, &SpreadsheetView::insertColumnRight);
	connect(action_insert_columns_left, &QAction::triggered, this, static_cast<void (SpreadsheetView::*)()>(&SpreadsheetView::insertColumnsLeft));
	connect(action_insert_columns_right, &QAction::triggered, this, static_cast<void (SpreadsheetView::*)()>(&SpreadsheetView::insertColumnsRight));
	connect(action_remove_columns, &QAction::triggered, this, &SpreadsheetView::removeSelectedColumns);
	connect(action_clear_columns, &QAction::triggered, this, &SpreadsheetView::clearSelectedColumns);
	connect(action_set_as_none, &QAction::triggered, this, &SpreadsheetView::setSelectionAs);
	connect(action_set_as_x, &QAction::triggered, this, &SpreadsheetView::setSelectionAs);
	connect(action_set_as_y, &QAction::triggered, this, &SpreadsheetView::setSelectionAs);
	connect(action_set_as_z, &QAction::triggered, this, &SpreadsheetView::setSelectionAs);
	connect(action_set_as_xerr, &QAction::triggered, this, &SpreadsheetView::setSelectionAs);
	connect(action_set_as_xerr_minus, &QAction::triggered, this, &SpreadsheetView::setSelectionAs);
	connect(action_set_as_xerr_plus, &QAction::triggered, this, &SpreadsheetView::setSelectionAs);
	connect(action_set_as_yerr, &QAction::triggered, this, &SpreadsheetView::setSelectionAs);
	connect(action_set_as_yerr_minus, &QAction::triggered, this, &SpreadsheetView::setSelectionAs);
	connect(action_set_as_yerr_plus, &QAction::triggered, this, &SpreadsheetView::setSelectionAs);

	//data manipulation
	connect(action_add_value, &QAction::triggered, this, &SpreadsheetView::modifyValues);
	connect(action_subtract_value, &QAction::triggered, this, &SpreadsheetView::modifyValues);
	connect(action_multiply_value, &QAction::triggered, this, &SpreadsheetView::modifyValues);
	connect(action_divide_value, &QAction::triggered, this, &SpreadsheetView::modifyValues);
	connect(action_reverse_columns, &QAction::triggered, this, &SpreadsheetView::reverseColumns);
	connect(action_drop_values, &QAction::triggered, this, &SpreadsheetView::dropColumnValues);
	connect(action_mask_values, &QAction::triggered, this, &SpreadsheetView::maskColumnValues);
// 	connect(action_join_columns, &QAction::triggered, this, &SpreadsheetView::joinColumns);
	connect(normalizeColumnActionGroup, &QActionGroup::triggered, this, &SpreadsheetView::normalizeSelectedColumns);
	connect(action_normalize_selection, &QAction::triggered, this, &SpreadsheetView::normalizeSelection);

	//sort
	connect(action_sort_columns, &QAction::triggered, this, &SpreadsheetView::sortSelectedColumns);
	connect(action_sort_asc_column, &QAction::triggered, this, &SpreadsheetView::sortColumnAscending);
	connect(action_sort_desc_column, &QAction::triggered, this, &SpreadsheetView::sortColumnDescending);

	//statistics
	connect(action_statistics_columns, &QAction::triggered, this, &SpreadsheetView::showColumnStatistics);
	connect(action_statistics_all_columns, &QAction::triggered, this, &SpreadsheetView::showAllColumnsStatistics);

	connect(action_insert_row_above, &QAction::triggered, this, &SpreadsheetView::insertRowAbove);
	connect(action_insert_row_below, &QAction::triggered, this, &SpreadsheetView::insertRowBelow);
	connect(action_insert_rows_above, &QAction::triggered, this, static_cast<void (SpreadsheetView::*)()>(&SpreadsheetView::insertRowsAbove));
	connect(action_insert_rows_below, &QAction::triggered, this, static_cast<void (SpreadsheetView::*)()>(&SpreadsheetView::insertRowsBelow));
	connect(action_remove_rows, &QAction::triggered, this, &SpreadsheetView::removeSelectedRows);
	connect(action_clear_rows, &QAction::triggered, this, &SpreadsheetView::clearSelectedRows);
	connect(action_statistics_rows, &QAction::triggered, this, &SpreadsheetView::showRowStatistics);
	connect(action_toggle_comments, &QAction::triggered, this, &SpreadsheetView::toggleComments);

	connect(action_plot_data_xycurve, &QAction::triggered, this, &SpreadsheetView::plotData);
	connect(action_plot_data_histogram, &QAction::triggered, this, &SpreadsheetView::plotData);
	connect(addDataReductionAction, &QAction::triggered, this, &SpreadsheetView::plotData);
	connect(addDifferentiationAction, &QAction::triggered, this, &SpreadsheetView::plotData);
	connect(addIntegrationAction, &QAction::triggered, this, &SpreadsheetView::plotData);
	connect(addInterpolationAction, &QAction::triggered, this, &SpreadsheetView::plotData);
	connect(addSmoothAction, &QAction::triggered, this, &SpreadsheetView::plotData);
	for (const auto& action : addFitAction)
		connect(action, &QAction::triggered, this, &SpreadsheetView::plotData);
	connect(addFourierFilterAction, &QAction::triggered,this, &SpreadsheetView::plotData);
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

#ifdef Q_OS_MAC
void SpreadsheetView::fillTouchBar(KDMacTouchBar* touchBar){
	//touchBar->addAction(action_insert_column_right);
}
#endif

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

	QAction* firstAction = nullptr;
	// if we're populating the context menu for the project explorer, then
	//there're already actions available there. Skip the first title-action
	//and insert the action at the beginning of the menu.
	if (menu->actions().size()>1)
		firstAction = menu->actions().at(1);

	if (m_spreadsheet->columnCount() > 0 && m_spreadsheet->rowCount() > 0) {
		menu->insertMenu(firstAction, m_plotDataMenu);
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

	QAction* firstAction = menu->actions().at(1);
	//TODO: add these menus and synchronize the behavior with the context menu creation
	//on the spreadsheet header in eventFilter(),
// 		menu->insertMenu(firstAction, m_plotDataMenu);
// 		menu->insertMenu(firstAction, m_analyzePlotMenu);
// 		menu->insertSeparator(firstAction);

	const bool hasValues = column->hasValues();
	const bool numeric = column->isNumeric();

	if (numeric)
		menu->insertMenu(firstAction, m_columnSetAsMenu);

	if (!m_readOnly) {
		if (numeric) {
			menu->insertSeparator(firstAction);
			menu->insertMenu(firstAction, m_columnGenerateDataMenu);
			menu->insertSeparator(firstAction);
			menu->insertMenu(firstAction, m_columnManipulateDataMenu);
			menu->insertSeparator(firstAction);
		}

		menu->insertMenu(firstAction, m_columnSortMenu);
		action_sort_asc_column->setVisible(true);
		action_sort_desc_column->setVisible(true);
		action_sort_columns->setVisible(false);

		//in case no cells are available, deactivate the actions that only make sense in the presence of cells
		const bool hasCells = m_spreadsheet->rowCount() > 0;
		m_columnGenerateDataMenu->setEnabled(numeric && hasCells);

		//in case no valid numerical values are available, deactivate the actions that only make sense in the presence of values
		m_columnManipulateDataMenu->setEnabled(numeric && hasValues);
		m_columnSortMenu->setEnabled(hasValues);
	}

	menu->insertSeparator(firstAction);
	menu->insertAction(firstAction, action_statistics_columns);
	action_statistics_columns->setEnabled(numeric && hasValues);
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

	goToCell(0, index);
	connect(col, &Column::requestProjectContextMenu, this, &SpreadsheetView::createColumnContextMenu);
}

void SpreadsheetView::handleAspectAboutToBeRemoved(const AbstractAspect* aspect) {
	const Column* col = dynamic_cast<const Column*>(aspect);
	if (!col || col->parentAspect() != m_spreadsheet)
		return;

	disconnect(col, nullptr, this, nullptr);
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

//TODO Implement the "change of the column name"-mode upon a double click
void SpreadsheetView::handleHorizontalHeaderDoubleClicked(int index) {
	Q_UNUSED(index);
}

/*!
  Returns whether comments are shown currently or not
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
	for (int i = 0; i < cols; i++)
		if (isColumnSelected(i, full)) count++;
	return count;
}

/*!
  Returns the number of (at least partly) selected columns with the plot designation \param pd .
 */
int SpreadsheetView::selectedColumnCount(AbstractColumn::PlotDesignation pd) const{
	int count = 0;
	const int cols = m_spreadsheet->columnCount();
	for (int i = 0; i < cols; i++)
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
	for (int i = 0; i < cols; i++)
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
	for (int i = 0; i < cols; i++) {
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
	for (int i = cols - 1; i >= 0; i--)
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
	for (int i = 0; i < rows; i++)
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
	if (index.isValid()) {
		*row = index.row();
		*col = index.column();
	} else {
		*row = -1;
		*col = -1;
	}
}

bool SpreadsheetView::eventFilter(QObject* watched, QEvent* event) {
	if (event->type() == QEvent::ContextMenu) {
		auto* cm_event = static_cast<QContextMenuEvent*>(event);
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

			if (selectedColumns().size() == 1) {
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
			bool plottable = true;
			bool datetime = false;
			bool hasValues = false;
			for (const Column* col : selectedColumns()) {
				if ( !(col->columnMode() == AbstractColumn::Numeric || col->columnMode() == AbstractColumn::Integer ||
							col->columnMode() == AbstractColumn::BigInt) ) {
					datetime = (col->columnMode() == AbstractColumn::DateTime);
					if (!datetime)
						plottable = false;

					numeric = false;
					break;
				}
			}

			for (const Column* col : selectedColumns()) {
				if (col->hasValues()) {
					hasValues = true;
					break;
				}
			}

			m_plotDataMenu->setEnabled(plottable);
			m_analyzePlotMenu->setEnabled(numeric);
			m_columnSetAsMenu->setEnabled(numeric);
			action_statistics_columns->setEnabled(numeric && hasValues);

			if (!m_readOnly) {
				m_columnGenerateDataMenu->setEnabled(numeric);
				m_columnManipulateDataMenu->setEnabled(numeric || datetime);
				m_columnSortMenu->setEnabled(numeric);

				//in case no cells are available, deactivate the actions that only make sense in the presence of cells
				const bool hasCells = m_spreadsheet->rowCount() > 0;
				m_columnGenerateDataMenu->setEnabled(numeric && hasCells);

				//in case no valid numerical values are available, deactivate the actions that only make sense in the presence of values
				m_columnManipulateDataMenu->setEnabled(numeric && hasValues);
				m_columnSortMenu->setEnabled(hasValues);
			}

			m_columnMenu->exec(global_pos);
		} else if (watched == this) {
			checkSpreadsheetMenu();
			m_spreadsheetMenu->exec(global_pos);
		}

		return true;
	} else if (event->type() == QEvent::KeyPress) {
		auto* key_event = static_cast<QKeyEvent*>(event);
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
	m_plotDataMenu->setEnabled(cellsAvail);
	m_selectionMenu->setEnabled(cellsAvail);
	action_select_all->setEnabled(cellsAvail);
	action_clear_spreadsheet->setEnabled(cellsAvail);
	action_clear_masks->setEnabled(cellsAvail);
	action_sort_spreadsheet->setEnabled(cellsAvail);
	action_go_to_cell->setEnabled(cellsAvail);
	action_statistics_all_columns->setEnabled(cellsAvail);

	//deactivate mask/unmask actions if there are no unmasked/masked cells
	//in the current selection
	QModelIndexList indexes = m_tableView->selectionModel()->selectedIndexes();
	bool hasMasked = false;
	bool hasUnmasked = false;
	for (auto index : indexes) {
		int row = index.row();
		int col = index.column();
		const auto* column = m_spreadsheet->column(col);
		//TODO: the null pointer check shouldn't be actually required here
		//but when deleting the columns the selection model in the view
		//and the aspect model sometimes get out of sync and we crash...
		if (column && column->isMasked(row)) {
			hasMasked = true;
			break;
		}
	}

	for (auto index : indexes) {
		int row = index.row();
		int col = index.column();
		const auto* column = m_spreadsheet->column(col);
		if (column && !column->isMasked(row)) {
			hasUnmasked = true;
			break;
		}
	}

	action_mask_selection->setEnabled(hasUnmasked);
	action_unmask_selection->setEnabled(hasMasked);
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
				else if (col_ptr->columnMode() == AbstractColumn::Integer || col_ptr->columnMode() == AbstractColumn::BigInt)
					output_str += QString::number(col_ptr->valueAt(first_row + r));
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
/*
bool determineLocale(const QString& value, QLocale& locale) {
	int pointIndex = value.indexOf(QLatin1Char('.'));
	int commaIndex = value.indexOf(QLatin1Char('.'));
	if (pointIndex != -1 && commaIndex != -1) {

	}
	return false;
}*/

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
	QString separator;
	if (input_str.indexOf(QLatin1String("\r\n")) != -1)
		separator = QLatin1String("\r\n");
	else
		separator = QLatin1Char('\n');

	QStringList input_rows(input_str.split(separator));
	input_row_count = input_rows.count();
	input_col_count = 0;
	bool hasTabs = false;
	if (input_row_count > 0 && input_rows.constFirst().indexOf(QLatin1Char('\t')) != -1)
		hasTabs = true;

	for (int i = 0; i < input_row_count; i++) {
		if (hasTabs)
			cellTexts.append(input_rows.at(i).split(QLatin1Char('\t')));
		else
			cellTexts.append(input_rows.at(i).split(QRegularExpression(QStringLiteral("\\s+"))));
		if (cellTexts.at(i).count() > input_col_count)
			input_col_count = cellTexts.at(i).count();
	}

	QLocale locale;
// 	bool localeDetermined = false;

	if ( (first_col == -1 || first_row == -1) || (last_row == first_row && last_col == first_col) ) {
		// if there is no selection or only one cell selected, the
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
		const int columnCount = m_spreadsheet->columnCount();
		//if the target columns that are already available don't have any values yet,
		//convert their mode to the mode of the data to be pasted
		for (int c = first_col; c <= last_col && c < columnCount; ++c) {
			Column* col = m_spreadsheet->column(c);
			if (col->hasValues() )
				continue;

			//first non-empty value in the column to paste determines the column mode/type of the new column to be added
			const int curCol = c - first_col;
			QString nonEmptyValue;
			for (auto r : cellTexts) {
				if (curCol < r.count() && !r.at(curCol).isEmpty()) {
					nonEmptyValue = r.at(curCol);
					break;
				}
			}

// 			if (!localeDetermined)
// 				localeDetermined = determineLocale(nonEmptyValue, locale);

			const AbstractColumn::ColumnMode mode = AbstractFileFilter::columnMode(nonEmptyValue,
													QLatin1String("yyyy-dd-MM hh:mm:ss:zzz"));
			col->setColumnMode(mode);
		}

		//add columns if necessary
		if (last_col >= columnCount) {
			for (int c = 0; c < last_col - (columnCount - 1); ++c) {
				const int curCol = columnCount - first_col + c;
				//first non-empty value in the column to paste determines the column mode/type of the new column to be added
				QString nonEmptyValue;
				for (auto r : cellTexts) {
					if (curCol < r.count() && !r.at(curCol).isEmpty()) {
						nonEmptyValue = r.at(curCol);
						break;
					}
				}

// 				if (!localeDetermined)
// 					localeDetermined = determineLocale(nonEmptyValue, locale);

				const AbstractColumn::ColumnMode mode = AbstractFileFilter::columnMode(nonEmptyValue,
														QLatin1String("yyyy-dd-MM hh:mm:ss:zzz"));
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
	for (int c = 0; c < cols && c < input_col_count; c++) {
		Column* col = m_spreadsheet->column(first_col + c);
		col->setSuppressDataChangedSignal(true);
		if (col->columnMode() == AbstractColumn::Numeric) {
			if (rows == m_spreadsheet->rowCount() && rows <= cellTexts.size()) {
				QVector<double> new_data(rows);
				for (int r = 0; r < rows; ++r) {
					if (c < cellTexts.at(r).count())
						new_data[r] = locale.toDouble(cellTexts.at(r).at(c));
				}
				col->replaceValues(0, new_data);
			} else {
				for (int r = 0; r < rows && r < input_row_count; r++) {
					if ( isCellSelected(first_row + r, first_col + c) && (c < cellTexts.at(r).count()) ) {
						if (!cellTexts.at(r).at(c).isEmpty())
							col->setValueAt(first_row + r, locale.toDouble(cellTexts.at(r).at(c)));
						else
							col->setValueAt(first_row + r, std::numeric_limits<double>::quiet_NaN());
					}
				}
			}
		} else if (col->columnMode() == AbstractColumn::Integer) {
			if (rows == m_spreadsheet->rowCount() && rows <= cellTexts.size()) {
				QVector<int> new_data(rows);
				for (int r = 0; r < rows; ++r) {
					if (c < cellTexts.at(r).count())
						new_data[r] = locale.toInt(cellTexts.at(r).at(c));
				}
				col->replaceInteger(0, new_data);
			} else {
				for (int r = 0; r < rows && r < input_row_count; r++) {
					if ( isCellSelected(first_row + r, first_col + c) && (c < cellTexts.at(r).count()) ) {
						if (!cellTexts.at(r).at(c).isEmpty())
							col->setIntegerAt(first_row + r, locale.toInt(cellTexts.at(r).at(c)));
						else
							col->setIntegerAt(first_row + r, 0);
					}
				}
			}
		} else if (col->columnMode() == AbstractColumn::BigInt) {
			if (rows == m_spreadsheet->rowCount() && rows <= cellTexts.size()) {
				QVector<qint64> new_data(rows);
				for (int r = 0; r < rows; ++r)
					new_data[r] = locale.toLongLong(cellTexts.at(r).at(c));
				col->replaceBigInt(0, new_data);
			} else {
				for (int r = 0; r < rows && r < input_row_count; r++) {
					if ( isCellSelected(first_row + r, first_col + c) && (c < cellTexts.at(r).count()) ) {
						if (!cellTexts.at(r).at(c).isEmpty())
							col->setBigIntAt(first_row + r, locale.toLongLong(cellTexts.at(r).at(c)));
						else
							col->setBigIntAt(first_row + r, 0);
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
	if (first < 0) return;
	int last = lastSelectedRow();

	WAIT_CURSOR;
	m_spreadsheet->beginMacro(i18n("%1: mask selected cells", m_spreadsheet->name()));

	QVector<CartesianPlot*> plots;
	//determine the dependent plots
	for (auto* column : selectedColumns())
		column->addUsedInPlots(plots);

	//suppress retransform in the dependent plots
	for (auto* plot : plots)
		plot->setSuppressDataChangedSignal(true);

	//mask the selected cells
	for (auto* column : selectedColumns()) {
		int col = m_spreadsheet->indexOfChild<Column>(column);
		for (int row = first; row <= last; row++)
			if (isCellSelected(row, col)) column->setMasked(row);
	}

	//retransform the dependent plots
	for (auto* plot : plots) {
		plot->setSuppressDataChangedSignal(false);
		plot->dataChanged();
	}

	m_spreadsheet->endMacro();
	RESET_CURSOR;
}

void SpreadsheetView::unmaskSelection() {
	int first = firstSelectedRow();
	if (first < 0) return;
	int last = lastSelectedRow();

	WAIT_CURSOR;
	m_spreadsheet->beginMacro(i18n("%1: unmask selected cells", m_spreadsheet->name()));

	QVector<CartesianPlot*> plots;
	//determine the dependent plots
	for (auto* column : selectedColumns())
		column->addUsedInPlots(plots);

	//suppress retransform in the dependent plots
	for (auto* plot : plots)
		plot->setSuppressDataChangedSignal(true);

	//unmask the selected cells
	for (auto* column : selectedColumns()) {
		int col = m_spreadsheet->indexOfChild<Column>(column);
		for (int row = first; row <= last; row++)
			if (isCellSelected(row, col)) column->setMasked(row, false);
	}

	//retransform the dependent plots
	for (auto* plot : plots) {
		plot->setSuppressDataChangedSignal(false);
		plot->dataChanged();
	}

	m_spreadsheet->endMacro();
	RESET_CURSOR;
}

void SpreadsheetView::plotData() {
	const QAction* action = dynamic_cast<const QAction*>(QObject::sender());
	PlotDataDialog::PlotType type = PlotDataDialog::PlotXYCurve;
	if (action == action_plot_data_xycurve || action == action_plot_data_histogram)
		type = (PlotDataDialog::PlotType)action->data().toInt();

	auto* dlg = new PlotDataDialog(m_spreadsheet, type);

	if (action != action_plot_data_xycurve && action != action_plot_data_histogram) {
		PlotDataDialog::AnalysisAction type = (PlotDataDialog::AnalysisAction)action->data().toInt();
		dlg->setAnalysisAction(type);
	}

	dlg->exec();
}

void SpreadsheetView::fillSelectedCellsWithRowNumbers() {
	if (selectedColumnCount() < 1) return;
	int first = firstSelectedRow();
	if (first < 0) return;
	int last = lastSelectedRow();

	WAIT_CURSOR;
	m_spreadsheet->beginMacro(i18n("%1: fill cells with row numbers", m_spreadsheet->name()));
	for (auto* col_ptr : selectedColumns()) {
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
		case AbstractColumn::BigInt: {
			QVector<qint64> results(last-first+1);
			for (int row = first; row <= last; row++)
				if (isCellSelected(row, col))
					results[row-first] = row + 1;
				else
					results[row-first] = col_ptr->bigIntAt(row);
			col_ptr->replaceBigInt(first, results);
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

	QVector<int> int_data(rows);
	for (int i = 0; i < rows; ++i)
		int_data[i] = i + 1;

	for (auto* col : selectedColumns()) {
		switch (col->columnMode()) {
		case AbstractColumn::Integer:
			col->replaceInteger(0, int_data);
			break;
		case AbstractColumn::Numeric:
		case AbstractColumn::BigInt:
			col->setColumnMode(AbstractColumn::Integer);
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
	for (auto* col_ptr : selectedColumns()) {
		int col = m_spreadsheet->indexOfChild<Column>(col_ptr);
		col_ptr->setSuppressDataChangedSignal(true);
		switch (col_ptr->columnMode()) {
		case AbstractColumn::Numeric: {
				//TODO qrand() is obsolete. Use QRandomGenerator instead
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
				//TODO qrand() is obsolete. Use QRandomGenerator instead
				QVector<int> results(last-first+1);
				for (int row = first; row <= last; row++)
					if (isCellSelected(row, col))
						results[row-first] = qrand();
					else
						results[row-first] = col_ptr->integerAt(row);
				col_ptr->replaceInteger(first, results);
				break;
			}
		case AbstractColumn::BigInt: {
				//TODO qrand() is obsolete. Use QRandomGenerator instead
				QVector<qint64> results(last-first+1);
				for (int row = first; row <= last; row++)
					if (isCellSelected(row, col))
						results[row-first] = qrand();
					else
						results[row-first] = col_ptr->bigIntAt(row);
				col_ptr->replaceBigInt(first, results);
				break;
			}
		case AbstractColumn::Text: {
				//TODO qrand() is obsolete. Use QRandomGenerator instead
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
				//TODO qrand() is obsolete. Use QRandomGenerator instead
				QVector<QDateTime> results;
				QDate earliestDate(1, 1, 1);
				QDate latestDate(2999, 12, 31);
				QTime midnight(0, 0, 0, 0);
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
	auto* dlg = new RandomValuesDialog(m_spreadsheet);
	dlg->setColumns(selectedColumns());
	dlg->exec();
}

void SpreadsheetView::fillWithEquidistantValues() {
	if (selectedColumnCount() < 1) return;
	auto* dlg = new EquidistantValuesDialog(m_spreadsheet);
	dlg->setColumns(selectedColumns());
	dlg->exec();
}

void SpreadsheetView::fillWithFunctionValues() {
	if (selectedColumnCount() < 1) return;
	auto* dlg = new FunctionValuesDialog(m_spreadsheet);
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
	bool bigIntOk = false;
	bool stringOk = false;
	double doubleValue = 0;
	int intValue = 0;
	qint64 bigIntValue = 0;
	QString stringValue;

	m_spreadsheet->beginMacro(i18n("%1: fill cells with const values", m_spreadsheet->name()));
	for (auto* col_ptr : selectedColumns()) {
		int col = m_spreadsheet->indexOfChild<Column>(col_ptr);
		col_ptr->setSuppressDataChangedSignal(true);
		switch (col_ptr->columnMode()) {
		case AbstractColumn::Numeric:
			if (!doubleOk)
				doubleValue = QInputDialog::getDouble(this, i18n("Fill the selection with constant value"),
					i18n("Value"), 0, -std::numeric_limits<double>::max(), std::numeric_limits<double>::max(), 6, &doubleOk);
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
		case AbstractColumn::BigInt:
			//TODO: getBigInt()
			if (!bigIntOk)
				bigIntValue = QInputDialog::getInt(this, i18n("Fill the selection with constant value"),
					i18n("Value"), 0, -2147483647, 2147483647, 1, &bigIntOk);
			if (bigIntOk) {
				WAIT_CURSOR;
				QVector<qint64> results(last-first+1);
				for (int row = first; row <= last; row++) {
					if (isCellSelected(row, col))
						results[row-first] = bigIntValue;
					else
						results[row-first] = col_ptr->bigIntAt(row);
				}
				col_ptr->replaceBigInt(first, results);
				RESET_CURSOR;
			}
			break;
		case AbstractColumn::Text:
			if (!stringOk)
				stringValue = QInputDialog::getText(this, i18n("Fill the selection with constant value"),
					i18n("Value"), QLineEdit::Normal, nullptr, &stringOk);
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
  Insert an empty column left to the first selected column
*/
void SpreadsheetView::insertColumnLeft() {
	insertColumnsLeft(1);
}

/*!
  Insert multiple empty columns left to the firt selected column
*/
void SpreadsheetView::insertColumnsLeft() {
	bool ok = false;
	int count = QInputDialog::getInt(nullptr, i18n("Insert empty columns"), i18n("Enter the number of columns to insert"), 1/*value*/, 1/*min*/, 1000/*max*/, 1/*step*/, &ok);
	if (!ok)
		return;

	insertColumnsLeft(count);
}

/*!
 * private helper function doing the actual insertion of columns to the left
 */
void SpreadsheetView::insertColumnsLeft(int count) {
	WAIT_CURSOR;
	m_spreadsheet->beginMacro(i18np("%1: insert empty column",
									"%1: insert empty columns",
								 m_spreadsheet->name(),
								 count
							));

	const int first = firstSelectedColumn();

	if (first >= 0) {
		//determine the first selected column
		Column* firstCol = m_spreadsheet->child<Column>(first);

		for (int i = 0; i < count; ++i) {
			Column* newCol = new Column(QString::number(i + 1), AbstractColumn::Numeric);
			newCol->setPlotDesignation(AbstractColumn::Y);

			//resize the new column and insert it before the first selected column
			newCol->insertRows(0, m_spreadsheet->rowCount());
			m_spreadsheet->insertChildBefore(newCol, firstCol);
		}
	} else {
		if (m_spreadsheet->columnCount()>0) {
			//columns available but no columns selected -> prepend the new column at the very beginning
			Column* firstCol = m_spreadsheet->child<Column>(0);

			for (int i = 0; i < count; ++i) {
				Column* newCol = new Column(QString::number(i + 1), AbstractColumn::Numeric);
				newCol->setPlotDesignation(AbstractColumn::Y);
				newCol->insertRows(0, m_spreadsheet->rowCount());
				m_spreadsheet->insertChildBefore(newCol, firstCol);
			}
		} else {
			//no columns available anymore -> resize the spreadsheet and the new column to the default size
			KConfigGroup group = KSharedConfig::openConfig()->group(QLatin1String("Spreadsheet"));
			const int rows = group.readEntry(QLatin1String("RowCount"), 100);
			m_spreadsheet->setRowCount(rows);

			for (int i = 0; i < count; ++i) {
				Column* newCol = new Column(QString::number(i + 1), AbstractColumn::Numeric);
				(i == 0) ? newCol->setPlotDesignation(AbstractColumn::X) : newCol->setPlotDesignation(AbstractColumn::Y);
				newCol->insertRows(0, rows);

				//add/append a new column
				m_spreadsheet->addChild(newCol);
			}
		}
	}

	m_spreadsheet->endMacro();
	RESET_CURSOR;
}

/*!
  Insert an empty column right to the last selected column
*/
void SpreadsheetView::insertColumnRight() {
	insertColumnsRight(1);
}

/*!
  Insert multiple empty columns right to the last selected column
*/
void SpreadsheetView::insertColumnsRight() {
	bool ok = false;
	int count = QInputDialog::getInt(nullptr, i18n("Insert empty columns"), i18n("Enter the number of columns to insert"), 1/*value*/, 1/*min*/, 1000/*max*/, 1/*step*/, &ok);
	if (!ok)
		return;

	insertColumnsRight(count);
}

/*!
 * private helper function doing the actual insertion of columns to the right
 */
void SpreadsheetView::insertColumnsRight(int count) {
	WAIT_CURSOR;
	m_spreadsheet->beginMacro(i18np("%1: insert empty column",
									"%1: insert empty columns",
									m_spreadsheet->name(),
									count
							));

	const int last = lastSelectedColumn();

	if (last >= 0) {
		if (last < m_spreadsheet->columnCount() - 1) {
			//determine the column next to the last selected column
			Column* nextCol = m_spreadsheet->child<Column>(last + 1);

			for (int i = 0; i < count; ++i) {
				Column* newCol = new Column(QString::number(i+1), AbstractColumn::Numeric);
				newCol->setPlotDesignation(AbstractColumn::Y);
				newCol->insertRows(0, m_spreadsheet->rowCount());

				//insert the new column before the column next to the last selected column
				m_spreadsheet->insertChildBefore(newCol, nextCol);
			}
		} else {
			for (int i = 0; i < count; ++i) {
				Column* newCol = new Column(QString::number(i+1), AbstractColumn::Numeric);
				newCol->setPlotDesignation(AbstractColumn::Y);
				newCol->insertRows(0, m_spreadsheet->rowCount());

				//last column selected, no next column available -> add/append a new column
				m_spreadsheet->addChild(newCol);
			}
		}
	} else {
		if (m_spreadsheet->columnCount()>0) {
			for (int i = 0; i < count; ++i) {
				Column* newCol = new Column(QString::number(i+1), AbstractColumn::Numeric);
				newCol->setPlotDesignation(AbstractColumn::Y);
				newCol->insertRows(0, m_spreadsheet->rowCount());

				//columns available but no columns selected -> append the new column at the very end
				m_spreadsheet->addChild(newCol);
			}
		} else {
			//no columns available anymore -> resize the spreadsheet and the new column to the default size
			KConfigGroup group = KSharedConfig::openConfig()->group(QLatin1String("Spreadsheet"));
			const int rows = group.readEntry(QLatin1String("RowCount"), 100);
			m_spreadsheet->setRowCount(rows);

			for (int i = 0; i < count; ++i) {
				Column* newCol = new Column(QString::number(i+1), AbstractColumn::Numeric);
				(i == 0) ? newCol->setPlotDesignation(AbstractColumn::X) : newCol->setPlotDesignation(AbstractColumn::Y);
				newCol->insertRows(0, rows);

				//add/append a new column
				m_spreadsheet->addChild(newCol);
			}
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
		for (auto* col : selectedColumns()) {
			col->setSuppressDataChangedSignal(true);
			col->clearFormulas();
			col->setSuppressDataChangedSignal(false);
			col->setChanged();
		}
	} else {
		for (auto* col : selectedColumns()) {
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
	for (auto* col : columns)
		col->setPlotDesignation(pd);

	m_spreadsheet->endMacro();
}

/*!
 * add, subtract, multiply, divide
 */
void SpreadsheetView::modifyValues() {
	if (selectedColumnCount() < 1)
		return;

	const QAction* action = dynamic_cast<const QAction*>(QObject::sender());
	AddSubtractValueDialog::Operation op = (AddSubtractValueDialog::Operation)action->data().toInt();
	auto* dlg = new AddSubtractValueDialog(m_spreadsheet, op);
	dlg->setColumns(selectedColumns());
	dlg->exec();
}

void SpreadsheetView::reverseColumns() {
	WAIT_CURSOR;
	QVector<Column*> cols = selectedColumns();
	m_spreadsheet->beginMacro(i18np("%1: reverse column", "%1: reverse columns",
		m_spreadsheet->name(), cols.size()));
	for (auto* col : cols) {
		if (col->columnMode() != AbstractColumn::Numeric)
			continue;

		auto* data = static_cast<QVector<double>* >(col->data());
		QVector<double> new_data(*data);
		std::reverse(new_data.begin(), new_data.end());
		col->replaceValues(0, new_data);
	}
	m_spreadsheet->endMacro();
	RESET_CURSOR;
}

void SpreadsheetView::dropColumnValues() {
	if (selectedColumnCount() < 1) return;
	auto* dlg = new DropValuesDialog(m_spreadsheet);
	dlg->setColumns(selectedColumns());
	dlg->exec();
}

void SpreadsheetView::maskColumnValues() {
	if (selectedColumnCount() < 1) return;
	auto* dlg = new DropValuesDialog(m_spreadsheet, true);
	dlg->setColumns(selectedColumns());
	dlg->exec();
}

void SpreadsheetView::joinColumns() {
	//TODO
}

void SpreadsheetView::normalizeSelectedColumns(QAction* action) {
	auto columns = selectedColumns();
	if (columns.isEmpty())
		return;

	auto method = static_cast<NormalizationMethod>(action->data().toInt());

	double rescaleIntervalMin = 0.0;
	double rescaleIntervalMax = 0.0;
	if (method == Rescale) {
		auto* dlg = new RescaleDialog(this);
		dlg->setColumns(columns);
		int rc = dlg->exec();
		if (rc != QDialog::Accepted)
			return;

		rescaleIntervalMin = dlg->min();
		rescaleIntervalMax = dlg->max();
		delete dlg;
	}

	WAIT_CURSOR;
	QStringList messages;
	QString message = i18n("Normalization of the column <i>%1</i> was not possible because of %2.");
	m_spreadsheet->beginMacro(i18n("%1: normalize columns", m_spreadsheet->name()));

	for (auto* col : columns) {
		if (col->columnMode() != AbstractColumn::Numeric
			&& col->columnMode() != AbstractColumn::Integer
			&& col->columnMode() != AbstractColumn::BigInt)
			continue;

		if (col->columnMode() == AbstractColumn::Integer
			|| col->columnMode() == AbstractColumn::BigInt)
			col->setColumnMode(AbstractColumn::Numeric);

		auto* data = static_cast<QVector<double>* >(col->data());
		QVector<double> new_data(col->rowCount());

		switch (method) {
		case DivideBySum: {
			double sum = std::accumulate(data->begin(), data->end(), 0);
			if (sum != 0.0) {
				for (int i = 0; i < col->rowCount(); ++i)
					new_data[i] = data->operator[](i) / sum;
			} else {
				messages << message.arg(col->name()).arg(QLatin1String("Sum = 0"));
				continue;
			}
			break;
		}
		case DivideByMin: {
			double min = col->minimum();
			if (min != 0.0) {
				for (int i = 0; i < col->rowCount(); ++i)
					new_data[i] = data->operator[](i) / min;
			} else {
				messages << message.arg(col->name()).arg(QLatin1String("Min = 0"));
				continue;
			}
			break;
		}
		case DivideByMax: {
			double max = col->maximum();
			if (max != 0.0) {
				for (int i = 0; i < col->rowCount(); ++i)
					new_data[i] = data->operator[](i) / max;
			} else {
				messages << message.arg(col->name()).arg(QLatin1String("Max = 0"));
				continue;
			}
			break;
		}
		case DivideByCount: {
			int count = data->size();
			if (count != 0.0) {
				for (int i = 0; i < col->rowCount(); ++i)
					new_data[i] = data->operator[](i) / count;
			} else {
				messages << message.arg(col->name()).arg(QLatin1String("Count = 0"));
				continue;
			}
			break;
		}
		case DivideByMean: {
			double mean = col->statistics().arithmeticMean;
			if (mean != 0.0) {
				for (int i = 0; i < col->rowCount(); ++i)
					new_data[i] = data->operator[](i) / mean;
			} else {
				messages << message.arg(col->name()).arg(QLatin1String("Mean = 0"));
				continue;
			}
			break;
		}
		case DivideByMedian: {
			double median = col->statistics().median;
			if (median != 0.0) {
				for (int i = 0; i < col->rowCount(); ++i)
					new_data[i] = data->operator[](i) / median;
			} else {
				messages << message.arg(col->name()).arg(QLatin1String("Median = 0"));
				continue;
			}
			break;
		}
		case DivideByMode: {
			double mode = col->statistics().mode;
			if (mode != 0.0 && !std::isnan(mode)) {
				for (int i = 0; i < col->rowCount(); ++i)
					new_data[i] = data->operator[](i) / mode;
			} else {
				if (mode == 0.0)
					messages << message.arg(col->name()).arg(QLatin1String("Mode = 0"));
				else
					messages << message.arg(col->name()).arg(i18n("'Mode not defined'"));
				continue;
			}
			break;
		}
		case DivideByRange: {
			double range = col->statistics().maximum - col->statistics().minimum;
			if (range != 0.0) {
				for (int i = 0; i < col->rowCount(); ++i)
					new_data[i] = data->operator[](i) / range;
			} else {
				messages << message.arg(col->name()).arg(QLatin1String("Range = 0"));
				continue;
			}
			break;
		}
		case DivideBySD: {
			double std = col->statistics().standardDeviation;
			if (std != 0.0) {
				for (int i = 0; i < col->rowCount(); ++i)
					new_data[i] = data->operator[](i) / std;
			} else {
				messages << message.arg(col->name()).arg(QLatin1String("SD = 0"));
				continue;
			}
			break;
		}
		case DivideByMAD: {
			double mad = col->statistics().medianDeviation;
			if (mad != 0.0) {
				for (int i = 0; i < col->rowCount(); ++i)
					new_data[i] = data->operator[](i) / mad;
			} else {
				messages << message.arg(col->name()).arg(QLatin1String("MAD = 0"));
				continue;
			}
			break;
		}
		case DivideByIQR: {
			double iqr = col->statistics().iqr;
			if (iqr != 0.0) {
				for (int i = 0; i < col->rowCount(); ++i)
					new_data[i] = data->operator[](i) / iqr;
			} else {
				messages << message.arg(col->name()).arg(QLatin1String("IQR = 0"));
				continue;
			}
			break;
		}
		case ZScoreSD: {
			double mean = col->statistics().arithmeticMean;
			double std = col->statistics().standardDeviation;
			if (std != 0.0) {
				for (int i = 0; i < col->rowCount(); ++i)
					new_data[i] = (data->operator[](i) - mean) / std;
			} else {
				messages << message.arg(col->name()).arg(QLatin1String("SD = 0"));
				continue;
			}
			break;
		}
		case ZScoreMAD: {
			double median = col->statistics().median;
			double mad = col->statistics().medianDeviation;
			if (mad != 0.0) {
				for (int i = 0; i < col->rowCount(); ++i)
					new_data[i] = (data->operator[](i) - median) / mad;
			} else {
				messages << message.arg(col->name()).arg(QLatin1String("MAD = 0"));
				continue;
			}
			break;
		}
		case ZScoreIQR: {
			double median = col->statistics().median;
			double iqr = col->statistics().thirdQuartile - col->statistics().firstQuartile;
			if (iqr != 0.0) {
				for (int i = 0; i < col->rowCount(); ++i)
					new_data[i] = (data->operator[](i) - median) / iqr;
			} else {
				messages << message.arg(col->name()).arg(QLatin1String("IQR = 0"));
				continue;
			}
			break;
		}
		case Rescale: {
			double min = col->statistics().minimum;
			double max = col->statistics().maximum;
			if (max - min != 0.0) {
				for (int i = 0; i < col->rowCount(); ++i)
					new_data[i] = rescaleIntervalMin + (data->operator[](i) - min)/(max - min)*(rescaleIntervalMax - rescaleIntervalMin);
			} else {
				messages << message.arg(col->name()).arg(QLatin1String("Max - Min = 0"));
				continue;
			}
			break;
		}
		}

		col->replaceValues(0, new_data);
	}
	m_spreadsheet->endMacro();
	RESET_CURSOR;

	if (!messages.isEmpty()) {
		QString info;
		for (const QString& message : messages) {
			if (!info.isEmpty())
				info += QLatin1String("<br><br>");
			info += message;
		}
		QMessageBox::warning(this, i18n("Normalization not possible"), info);
	}
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
	auto* dlg = new StatisticsDialog(dlgTitle);
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
	auto* dlg = new StatisticsDialog(dlgTitle);

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
	insertRowsAbove(1);
}

/*!
  Insert multiple empty rows above(=before) the first selected row
*/
void SpreadsheetView::insertRowsAbove() {
	bool ok = false;
	int count = QInputDialog::getInt(nullptr, i18n("Insert multiple rows"), i18n("Enter the number of rows to insert"), 1/*value*/, 1/*min*/, 1000000/*max*/, 1/*step*/, &ok);
	if (ok)
		insertRowsAbove(count);
}

/*!
 * private helper function doing the actual insertion of rows above
 */
void SpreadsheetView::insertRowsAbove(int count) {
	int first = firstSelectedRow();
	if (first < 0)
		return;

	WAIT_CURSOR;
	m_spreadsheet->beginMacro(i18np("%1: insert empty row",
									"%1: insert empty rows",
									m_spreadsheet->name(),
									count
							));
	m_spreadsheet->insertRows(first, count);
	m_spreadsheet->endMacro();
	RESET_CURSOR;
}

/*!
  Insert an empty row below the last selected row
*/
void SpreadsheetView::insertRowBelow() {
	insertRowsBelow(1);
}

/*!
  Insert an empty row below the last selected row
*/
void SpreadsheetView::insertRowsBelow() {
	bool ok = false;
	int count = QInputDialog::getInt(nullptr, i18n("Insert multiple rows"), i18n("Enter the number of rows to insert"), 1/*value*/, 1/*min*/, 1000000/*max*/, 1/*step*/, &ok);
	if (ok)
		insertRowsBelow(count);
}

/*!
 * private helper function doing the actual insertion of rows below
 */
void SpreadsheetView::insertRowsBelow(int count) {
	int last = lastSelectedRow();
	if (last < 0)
		return;

	WAIT_CURSOR;
	m_spreadsheet->beginMacro(i18np("%1: insert empty row",
								   "%1: insert empty rows",
									m_spreadsheet->name(),
									count
							));

	if (last < m_spreadsheet->rowCount() - 1)
		m_spreadsheet->insertRows(last + 1, count); //insert before the next to the last selected row
	else
		m_spreadsheet->appendRows(count); //append new rows at the end

	m_spreadsheet->endMacro();
	RESET_CURSOR;
}

void SpreadsheetView::removeSelectedRows() {
	if (firstSelectedRow() < 0) return;

	WAIT_CURSOR;
	m_spreadsheet->beginMacro(i18n("%1: remove selected rows", m_spreadsheet->name()));
	//TODO setSuppressDataChangedSignal
	for (const auto& i : selectedRows().intervals())
		m_spreadsheet->removeRows(i.start(), i.size());
	m_spreadsheet->endMacro();
	RESET_CURSOR;
}

void SpreadsheetView::clearSelectedRows() {
	if (firstSelectedRow() < 0) return;

	WAIT_CURSOR;
	m_spreadsheet->beginMacro(i18n("%1: clear selected rows", m_spreadsheet->name()));
	for (auto* col : selectedColumns()) {
		col->setSuppressDataChangedSignal(true);
		if (formulaModeActive()) {
			for (const auto& i : selectedRows().intervals())
				col->setFormula(i, QString());
		} else {
			for (const auto& i : selectedRows().intervals()) {
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

	//don't try to clear values if the selected cells don't have any values at all
	bool empty = true;
	for (auto* column : selectedColumns()) {
		for (int row = last; row >= first; row--) {
			if (column->isValid(row)) {
				empty = false;
				break;
			}
		}
		if (!empty)
			break;
	}

	if (empty)
		return;

	WAIT_CURSOR;
	m_spreadsheet->beginMacro(i18n("%1: clear selected cells", m_spreadsheet->name()));
	for (auto* column : selectedColumns()) {
		column->setSuppressDataChangedSignal(true);
		if (formulaModeActive()) {
			int col = m_spreadsheet->indexOfChild<Column>(column);
			for (int row = last; row >= first; row--)
				if (isCellSelected(row, col))
					column->setFormula(row, QString());
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

	int col = QInputDialog::getInt(nullptr, i18n("Go to Cell"), i18n("Enter column"), 1, 1, m_spreadsheet->columnCount(), 1, &ok);
	if (!ok) return;

	int row = QInputDialog::getInt(nullptr, i18n("Go to Cell"), i18n("Enter row"), 1, 1, m_spreadsheet->rowCount(), 1, &ok);
	if (!ok) return;

	goToCell(row-1, col-1);
}

//! Open the sort dialog for the given columns
void SpreadsheetView::sortDialog(const QVector<Column*>& cols) {
	if (cols.isEmpty()) return;

	for (auto* col : cols)
		col->setSuppressDataChangedSignal(true);

	auto* dlg = new SortDialog();
	connect(dlg, SIGNAL(sort(Column*,QVector<Column*>,bool)), m_spreadsheet, SLOT(sortColumns(Column*,QVector<Column*>,bool)));
	dlg->setColumns(cols);
	int rc = dlg->exec();

	for (auto* col : cols) {
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
	for (auto* col : cols) {
		col->setSuppressDataChangedSignal(false);
		col->setChanged();
	}
}

void SpreadsheetView::sortColumnDescending() {
	QVector<Column*> cols = selectedColumns();
	for (auto* col : cols)
		col->setSuppressDataChangedSignal(true);
	m_spreadsheet->sortColumns(cols.first(), cols, false);
	for (auto* col : cols) {
		col->setSuppressDataChangedSignal(false);
		col->setChanged();
	}
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
	for (int i = 0; i < m_spreadsheet->columnCount(); i++)
		m_spreadsheet->setColumnSelectedInView(i, selModel->isColumnSelected(i, QModelIndex()));
}

bool SpreadsheetView::exportView() {
	auto* dlg = new ExportSpreadsheetDialog(this);
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
		switch (dlg->format()) {
		case ExportSpreadsheetDialog::ASCII: {
			const QString separator = dlg->separator();
			const QLocale::Language format = dlg->numberFormat();
			exportToFile(path, exportHeader, separator, format);
			break;
		}
		case ExportSpreadsheetDialog::Binary:
			break;
		case ExportSpreadsheetDialog::LaTeX: {
			const bool exportLatexHeader = dlg->exportLatexHeader();
			const bool gridLines = dlg->gridLines();
			const bool captions = dlg->captions();
			const bool skipEmptyRows = dlg->skipEmptyRows();
			const bool exportEntire = dlg->entireSpreadheet();
			exportToLaTeX(path, exportHeader, gridLines, captions,
				exportLatexHeader, skipEmptyRows, exportEntire);
			break;
		}
		case ExportSpreadsheetDialog::FITS: {
			const int exportTo = dlg->exportToFits();
			const bool commentsAsUnits = dlg->commentsAsUnitsFits();
			exportToFits(path, exportTo, commentsAsUnits);
			break;
		}
		case ExportSpreadsheetDialog::SQLite:
			exportToSQLite(path);
			break;
		}
		RESET_CURSOR;
	}
	delete dlg;

	return ret;
}

bool SpreadsheetView::printView() {
	QPrinter printer;
	auto* dlg = new QPrintDialog(&printer, this);
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
	const int vertHeaderWidth = vHeader->width();

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
		int right = margin + vertHeaderWidth;

		painter.setFont(hHeader->font());
		QString headerString = m_tableView->model()->headerData(0, Qt::Horizontal).toString();
		QRect br;
		br = painter.boundingRect(br, Qt::AlignCenter, headerString);
		QRect tr(br);
		if (table != 0)
			height += tr.height();
		painter.drawLine(right, height, right, height+br.height());

		int i = table * columnsPerTable;
		int toI = table * columnsPerTable + columnsPerTable;
		if ((remainingColumns > 0) && (table == tablesCount-1)) {
			i = (tablesCount-1)*columnsPerTable;
			toI = (tablesCount-1)* columnsPerTable + remainingColumns;
		}

		for (; i<toI; ++i) {
			headerString = m_tableView->model()->headerData(i, Qt::Horizontal).toString();
			const int w = m_tableView->columnWidth(i);
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
		for (i = 0; i < rows; ++i) {
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
			for (; j < toJ; j++) {
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

			if (height >= printer->height() - margin) {
				printer->newPage();
				height = margin;
				painter.drawLine(margin, height, right, height);
			}
		}
	}
	RESET_CURSOR;
}

void SpreadsheetView::registerShortcuts() {
	action_clear_selection->setShortcut(QKeySequence::Delete);
}

void SpreadsheetView::unregisterShortcuts() {
	action_clear_selection->setShortcut(QKeySequence());
}

/*!
 * the spreadsheet can have empty rows at the end full with NaNs.
 * for the export we only need to export valid (non-empty) rows.
 * this functions determines the maximal row to export, or -1
 * if the spreadsheet doesn't have any data yet.
*/
int SpreadsheetView::maxRowToExport() const {
	int maxRow = -1;
	for (int j = 0; j < m_spreadsheet->columnCount(); ++j) {
		Column* col = m_spreadsheet->column(j);
		if (col->columnMode() == AbstractColumn::Numeric) {
			for (int i = 0; i < m_spreadsheet->rowCount(); ++i) {
				if (!std::isnan(col->valueAt(i)) && i > maxRow)
					maxRow = i;
			}
		} if (col->columnMode() == AbstractColumn::Integer) {
			//TODO:
			//integer column found. Since empty integer cells are equal to 0
			//at the moment, we need to export the whole column.
			//this logic needs to be adjusted once we're able to descriminate
			//between empty and 0 values for integer columns
			maxRow = m_spreadsheet->rowCount();
			break;
		} else if (col->columnMode() == AbstractColumn::DateTime) {
			for (int i = 0; i < m_spreadsheet->rowCount(); ++i) {
				if (col->dateTimeAt(i).isValid() && i > maxRow)
					maxRow = i;
			}
		} else if (col->columnMode() == AbstractColumn::Text) {
			for (int i = 0; i < m_spreadsheet->rowCount(); ++i) {
				if (!col->textAt(i).isEmpty() && i > maxRow)
					maxRow = i;
			}
		}
	}

	return maxRow;
}

void SpreadsheetView::exportToFile(const QString& path, const bool exportHeader, const QString& separator, QLocale::Language language) const {
	QFile file(path);
	if (!file.open(QFile::WriteOnly | QFile::Truncate))
		return;

	PERFTRACE("export spreadsheet to file");
	QTextStream out(&file);

	int maxRow = maxRowToExport();
	if (maxRow < 0)
		return;

	const int cols = m_spreadsheet->columnCount();
	QString sep = separator;
	sep = sep.replace(QLatin1String("TAB"), QLatin1String("\t"), Qt::CaseInsensitive);
	sep = sep.replace(QLatin1String("SPACE"), QLatin1String(" "), Qt::CaseInsensitive);

	//export header (column names)
	if (exportHeader) {
		for (int j = 0; j < cols; ++j) {
			out << '"' << m_spreadsheet->column(j)->name() <<'"';
			if (j != cols - 1)
				out << sep;
		}
		out << '\n';
	}

	//export values
	QLocale locale(language);
	for (int i = 0; i <= maxRow; ++i) {
		for (int j = 0; j < cols; ++j) {
			Column* col = m_spreadsheet->column(j);
			if (col->columnMode() == AbstractColumn::Numeric) {
				const Double2StringFilter* out_fltr = static_cast<Double2StringFilter*>(col->outputFilter());
				out << locale.toString(col->valueAt(i), out_fltr->numericFormat(), 16); // export with max. precision
			} else
				out << col->asStringColumn()->textAt(i);

			if (j != cols - 1)
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
		yearidx -= 3;

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
			textable << QLatin1String("\\begin{tabular}{") << (gridLines ? QStringLiteral("|") : QString());
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
		remainingTable << QLatin1String("\\begin{tabular}{") << (gridLines ? QStringLiteral("|") : QString());
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
		textable << QLatin1String("\\begin{tabular}{") << (gridLines ? QStringLiteral("|") : QString());
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
	auto* filter = new FITSFilter;

	filter->setExportTo(exportTo);
	filter->setCommentsAsUnits(commentsAsUnits);
	filter->write(fileName, m_spreadsheet);

	delete filter;
}

void SpreadsheetView::exportToSQLite(const QString& path) const {
	QFile file(path);
	if (!file.open(QFile::WriteOnly | QFile::Truncate))
		return;

	PERFTRACE("export spreadsheet to SQLite database");
	QApplication::processEvents(QEventLoop::AllEvents, 0);

	//create database
	const QStringList& drivers = QSqlDatabase::drivers();
	QString driver;
	if (drivers.contains(QLatin1String("QSQLITE3")))
		driver = QLatin1String("QSQLITE3");
	else
		driver = QLatin1String("QSQLITE");

	QSqlDatabase db = QSqlDatabase::addDatabase(driver);
	db.setDatabaseName(path);
	if (!db.open()) {
		RESET_CURSOR;
		KMessageBox::error(nullptr, i18n("Couldn't create the SQLite database %1.", path));
	}

	//create table
	const int cols = m_spreadsheet->columnCount();
	QString query = QLatin1String("create table ") + m_spreadsheet->name() + QLatin1String(" (");
	for (int i = 0; i < cols; ++i) {
		Column* col = m_spreadsheet->column(i);
		if (i != 0)
			query += QLatin1String(", ");

		query += QLatin1String("\"") + col->name() + QLatin1String("\" ");
		switch (col->columnMode()) {
		case AbstractColumn::Numeric:
			query += QLatin1String("REAL");
			break;
		case AbstractColumn::Integer:
		case AbstractColumn::BigInt:
			query += QLatin1String("INTEGER");
			break;
		case AbstractColumn::Text:
		case AbstractColumn::Month:
		case AbstractColumn::Day:
		case AbstractColumn::DateTime:
			query += QLatin1String("TEXT");
			break;
		}
	}
	query += QLatin1Char(')');
	QSqlQuery q;
	if (!q.exec(query)) {
		RESET_CURSOR;
		KMessageBox::error(nullptr, i18n("Failed to create table in the SQLite database %1.", path) + '\n' + q.lastError().databaseText());
		db.close();
		return;
	}


	int maxRow = maxRowToExport();
	if (maxRow < 0) {
		db.close();
		return;
	}

	//create bulk insert statement
	{
	PERFTRACE("Create the bulk insert statement");
	q.exec(QLatin1String("BEGIN TRANSACTION;"));
	query = "INSERT INTO '" + m_spreadsheet->name() + "' (";
	for (int i = 0; i < cols; ++i) {
		if (i != 0)
			query += QLatin1String(", ");
		query += QLatin1Char('\'') + m_spreadsheet->column(i)->name() + QLatin1Char('\'');
	}
	query += QLatin1String(") VALUES ");

	for (int i = 0; i <= maxRow; ++i) {
		if (i != 0)
			query += QLatin1String(",");

		query += QLatin1Char('(');
		for (int j = 0; j < cols; ++j) {
			Column* col = m_spreadsheet->column(j);
			if (j != 0)
				query += QLatin1String(", ");

			query += QLatin1Char('\'') + col->asStringColumn()->textAt(i) + QLatin1Char('\'');
		}
		query += QLatin1String(")");
	}
	query += QLatin1Char(';');
	}

	//insert values
	if (!q.exec(query)) {
		RESET_CURSOR;
		KMessageBox::error(nullptr, i18n("Failed to insert values into the table."));
		QDEBUG("bulk insert error " << q.lastError().databaseText());
	} else
		q.exec(QLatin1String("COMMIT TRANSACTION;"));

	//close the database
	db.close();
}
