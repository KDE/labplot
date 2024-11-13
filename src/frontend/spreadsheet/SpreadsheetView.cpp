/*
	File                 : SpreadsheetView.cpp
	Project              : LabPlot
	Description          : View class for Spreadsheet
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2011-2023 Alexander Semke <alexander.semke@web.de>
	SPDX-FileCopyrightText: 2016 Fabian Kristof <fkristofszabolcs@gmail.com>
	SPDX-FileCopyrightText: 2020-2023 Stefan Gerlach <stefan.gerlach@uni.kn>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "SpreadsheetView.h"
#include "SpreadsheetItemDelegate.h"
#include "backend/core/Project.h"
#include "backend/core/Settings.h"
#include "backend/core/datatypes/DateTime2StringFilter.h"
#include "backend/core/datatypes/Double2StringFilter.h"
#include "backend/datasources/filters/FITSFilter.h"
#include "backend/datasources/filters/McapFilter.h"
#include "backend/datasources/filters/XLSXFilter.h"
#include "backend/lib/hostprocess.h"
#include "backend/lib/macros.h"
#include "backend/lib/trace.h"
#include "backend/spreadsheet/StatisticsSpreadsheet.h"
#include "backend/worksheet/plots/cartesian/BoxPlot.h" //TODO: needed for the icon only, remove later once we have a breeze icon
#include "backend/worksheet/plots/cartesian/CartesianPlot.h"
#include "frontend/spreadsheet/SpreadsheetHeaderView.h"

#ifndef SDK
#include "frontend/spreadsheet/AddSubtractValueDialog.h"
#include "frontend/spreadsheet/DropValuesDialog.h"
#include "frontend/spreadsheet/EquidistantValuesDialog.h"
#include "frontend/spreadsheet/FlattenColumnsDialog.h"
#include "frontend/spreadsheet/FormattingHeatmapDialog.h"
#include "frontend/spreadsheet/FunctionValuesDialog.h"
#include "frontend/spreadsheet/RandomValuesDialog.h"
#include "frontend/spreadsheet/RescaleDialog.h"
#include "frontend/spreadsheet/SampleValuesDialog.h"
#include "frontend/spreadsheet/SearchReplaceWidget.h"
#include "frontend/spreadsheet/SortDialog.h"
#endif
#include "frontend/spreadsheet/ExportSpreadsheetDialog.h"
#include "frontend/spreadsheet/GoToDialog.h"
#include "frontend/spreadsheet/PlotDataDialog.h"
#include "frontend/spreadsheet/StatisticsDialog.h"

#ifdef Q_OS_MAC
#include "3rdparty/kdmactouchbar/src/kdmactouchbar.h"
#endif

#include <KConfigGroup>
#include <KLocalizedString>
#include <KMessageBox>

#include <QAbstractSlider>
#include <QActionGroup>
#include <QApplication>
#include <QClipboard>
#include <QDate>
#include <QFile>
#include <QInputDialog>
#include <QKeyEvent>
#include <QMenu>
#include <QMessageBox>
#include <QMimeData>
#include <QPainter>
#include <QPrintDialog>
#include <QPrintPreviewDialog>
#include <QPrinter>
#include <QProcess>
#include <QRandomGenerator>
#include <QRegularExpression>
#include <QScrollBar>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QTableView>
#include <QTextStream>
#include <QTimer>
#include <QToolBar>
#include <QUndoCommand>
#include <QVBoxLayout>

#include <algorithm> //for std::reverse

#include <gsl/gsl_const_cgs.h>
#include <gsl/gsl_math.h>

enum NormalizationMethod {
	DivideBySum,
	DivideByMin,
	DivideByMax,
	DivideByCount,
	DivideByMean,
	DivideByMedian,
	DivideByMode,
	DivideByRange,
	DivideBySD,
	DivideByMAD,
	DivideByIQR,
	ZScoreSD,
	ZScoreMAD,
	ZScoreIQR,
	Rescale
};

enum TukeyLadderPower { InverseSquared, Inverse, InverseSquareRoot, Log, SquareRoot, Squared, Cube };

/*!
	\class SpreadsheetView
	\brief View class for Spreadsheet

 \ingroup frontend
*/
SpreadsheetView::SpreadsheetView(Spreadsheet* spreadsheet, bool readOnly)
	: QWidget()
	, m_tableView(new QTableView(this))
	, m_spreadsheet(spreadsheet)
	, m_readOnly(readOnly) {
	auto* layout = new QVBoxLayout(this);
	layout->setContentsMargins(0, 0, 0, 0);

	layout->addWidget(m_tableView);
	if (m_readOnly)
		m_tableView->setEditTriggers(QTableView::NoEditTriggers);

	init();

	// resize the view to show alls columns and the first 10 rows.
	// no need to resize the view when the project is being opened,
	// all views will be resized to the stored values at the end
	if (!m_spreadsheet->isLoading()) {
		int w = m_tableView->verticalHeader()->width();
		int h = m_horizontalHeader->height();
		for (int i = 0; i < m_horizontalHeader->count(); ++i)
			w += m_horizontalHeader->sectionSize(i);

		if (m_tableView->verticalHeader()->count() <= 10)
			h += m_tableView->verticalHeader()->sectionSize(0) * m_tableView->verticalHeader()->count();
		else
			h += m_tableView->verticalHeader()->sectionSize(0) * 11;

		resize(w + 50, h);
	}
}

SpreadsheetView::~SpreadsheetView() {
}

void SpreadsheetView::init() {
	// create a new SpreadsheetModel if not available yet.
	// the creation of the model is done here since it's only required
	// for the view but its lifecycle is managed in Spreadsheet,
	// i.e. the deletion of the model is done in the destructor of Spreadsheet
	m_model = m_spreadsheet->model();
	if (!m_model)
		m_model = new SpreadsheetModel(m_spreadsheet);

	m_tableView->setModel(m_model);
	auto* delegate = new SpreadsheetItemDelegate(this);
	connect(delegate, &SpreadsheetItemDelegate::returnPressed, this, &SpreadsheetView::advanceCell);
	connect(delegate, &SpreadsheetItemDelegate::editorEntered, this, [=]() {
		m_editorEntered = true;
	});
	connect(delegate, &SpreadsheetItemDelegate::closeEditor, this, [=]() {
		m_editorEntered = false;
	});

	m_tableView->setItemDelegate(delegate);
	m_tableView->setSelectionMode(QAbstractItemView::ExtendedSelection);

	// horizontal header
	m_horizontalHeader = new SpreadsheetHeaderView(this);

	m_horizontalHeader->setSectionsClickable(true);
	m_horizontalHeader->setHighlightSections(true);
	m_tableView->setHorizontalHeader(m_horizontalHeader);
	m_horizontalHeader->setSectionsMovable(true);
	m_horizontalHeader->installEventFilter(this);
	m_tableView->setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);
	// 	m_tableView->installEventFilter(this);

	resizeHeader();

	connect(m_horizontalHeader, &SpreadsheetHeaderView::sectionMoved, this, &SpreadsheetView::handleHorizontalSectionMoved);
	connect(m_horizontalHeader, &SpreadsheetHeaderView::sectionDoubleClicked, this, &SpreadsheetView::handleHorizontalHeaderDoubleClicked);
	connect(m_horizontalHeader, &SpreadsheetHeaderView::sectionResized, this, &SpreadsheetView::handleHorizontalSectionResized);
	connect(m_horizontalHeader, &SpreadsheetHeaderView::sectionClicked, this, &SpreadsheetView::columnClicked);

	// vertical header
	auto* v_header = m_tableView->verticalHeader();
	v_header->setSectionResizeMode(QHeaderView::Fixed);
	v_header->setSectionsMovable(false);
	v_header->installEventFilter(this);
	m_tableView->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);

	setFocusPolicy(Qt::StrongFocus);
	setFocus();
	installEventFilter(this);

	// save and load the values from Spreadsheet
	KConfigGroup group = Settings::group(QStringLiteral("Spreadsheet"));
	showComments(group.readEntry(QLatin1String("ShowComments"), false));
	showSparkLines(group.readEntry(QLatin1String("ShowSparkLines"), false));

	connect(m_model, &SpreadsheetModel::headerDataChanged, this, &SpreadsheetView::updateHeaderGeometry);
	connect(m_model, &SpreadsheetModel::headerDataChanged, this, &SpreadsheetView::handleHeaderDataChanged);
	connect(m_spreadsheet, &Spreadsheet::aspectsInserted, this, &SpreadsheetView::handleAspectsAdded);
	connect(m_spreadsheet, &Spreadsheet::aspectsAboutToBeRemoved, this, &SpreadsheetView::handleAspectAboutToBeRemoved);
	connect(m_spreadsheet, &Spreadsheet::requestProjectContextMenu, this, &SpreadsheetView::createContextMenu);
	connect(m_spreadsheet, &Spreadsheet::manyAspectsAboutToBeInserted, [this] {
		m_suppressResize = true;
	});

	// react on sparkline toggled
	connect(m_horizontalHeader, &SpreadsheetHeaderView::sparklineToggled, this, [=] {
		for (int colIndex = 0; colIndex < m_spreadsheet->columnCount(); ++colIndex) {
			SpreadsheetSparkLinesHeaderModel::sparkLine(m_spreadsheet->column(colIndex));
			m_horizontalHeader->refresh();
			connect(m_spreadsheet->column(colIndex), &AbstractColumn::dataChanged, this, [=] {
				if (m_spreadsheet->isSparklineShown)
					SpreadsheetSparkLinesHeaderModel::sparkLine(m_spreadsheet->column(colIndex));
				m_horizontalHeader->refresh();
			});
		}
	});

	connect(m_spreadsheet, &Spreadsheet::columnCountChanged, this, [=] {
		// Disconnect existing connections before creating new ones
		for (int colIndex = 0; colIndex < m_spreadsheet->columnCount(); ++colIndex)
			disconnect(m_spreadsheet->column(colIndex), &AbstractColumn::dataChanged, this, nullptr);

		// Establish new connections
		for (int colIndex = 0; colIndex < m_spreadsheet->columnCount(); ++colIndex) {
			connect(m_spreadsheet->column(colIndex), &AbstractColumn::dataChanged, this, [=] {
				if (m_spreadsheet->isSparklineShown)
					SpreadsheetSparkLinesHeaderModel::sparkLine(m_spreadsheet->column(colIndex));
				m_horizontalHeader->refresh();
			});
		}
	});

	// selection related connections
	connect(m_tableView->selectionModel(), &QItemSelectionModel::selectionChanged, this, &SpreadsheetView::selectionChanged);
	connect(m_spreadsheet, &Spreadsheet::columnSelected, this, &SpreadsheetView::selectColumn);
	connect(m_spreadsheet, &Spreadsheet::columnDeselected, this, &SpreadsheetView::deselectColumn);
}

bool SpreadsheetView::isReadOnly() const {
	return m_readOnly;
}

/*!
	set the column sizes to the saved values or resize to content if no size was saved yet
*/
void SpreadsheetView::resizeHeader() {
	if (m_suppressResizeHeader)
		return;
	DEBUG(Q_FUNC_INFO)
	const auto& columns = m_spreadsheet->children<Column>();

	QFontMetrics fontMetrics(m_horizontalHeader->font());
	const auto* style = m_horizontalHeader->style();
	int headerOffset = style->pixelMetric(QStyle::PM_SmallIconSize, nullptr, m_horizontalHeader); // icon size
	headerOffset += 3 * style->pixelMetric(QStyle::PM_HeaderMargin, nullptr, m_horizontalHeader); // two margins plus the margin between icon and text

	int c = 0;
	for (auto col : columns) {
		if (col->width() == 0) {
			// No width was saved yet, resize to fit the content:
			// Calling m_tableView->resizeColumnToContents(i) is expensive since Qt needs to iterate over all values in the column
			// and to determine the maximal length which takes time the more rows we have in the spreadsheet. For a high number of
			// columns in the spreadsheet this operation can take significant amount of time, s. a. BUG: 455977.
			// To improve the performance, we check the width of the header texts only and resize the column widths to fit the header
			// widths. There will be cases where this is not a perfect fit but the user still can resize manually if needed.
			// Since this approach doesn't always lead to precise results (text width in other cells in the column can be bigger) and
			// since in many cases we deal with much lower number of columns during the import, we apply the more precise method with
			// resizeColumnToContents() if the number of columns is smaller than 50.
			if (columns.count() > 50) {
				int width = headerOffset + fontMetrics.horizontalAdvance(m_model->headerData(c, Qt::Horizontal, Qt::DisplayRole).toString());
				m_tableView->setColumnWidth(c, width);
			} else
				m_tableView->resizeColumnToContents(c);
		} else
			m_tableView->setColumnWidth(c, col->width());

		c++;
	}
}

void SpreadsheetView::setFocus() {
	m_tableView->setFocus();
}

void SpreadsheetView::setSuppressResizeHeader(bool suppress) {
	m_suppressResizeHeader = suppress;
}

void SpreadsheetView::resizeEvent(QResizeEvent* event) {
	QWidget::resizeEvent(event);
	if (m_frozenTableView)
		updateFrozenTableGeometry();
}

void SpreadsheetView::updateFrozenTableGeometry() {
	m_frozenTableView->setGeometry(m_tableView->verticalHeader()->width() + m_tableView->frameWidth(),
								   m_tableView->frameWidth(),
								   m_tableView->columnWidth(0),
								   m_tableView->viewport()->height() + m_tableView->horizontalHeader()->height());
}

void SpreadsheetView::initActions() {
	// selection related actions
	action_cut_selection = new QAction(QIcon::fromTheme(QStringLiteral("edit-cut")), i18n("Cu&t"), this);
	action_copy_selection = new QAction(QIcon::fromTheme(QStringLiteral("edit-copy")), i18n("&Copy"), this);
	action_paste_into_selection = new QAction(QIcon::fromTheme(QStringLiteral("edit-paste")), i18n("Past&e"), this);
	action_mask_selection = new QAction(QIcon::fromTheme(QStringLiteral("edit-node")), i18n("&Mask"), this);
	action_unmask_selection = new QAction(QIcon::fromTheme(QStringLiteral("format-remove-node")), i18n("&Unmask"), this);
	action_reverse_selection = new QAction(QIcon::fromTheme(QStringLiteral("reverse")), i18n("&Reverse"), this);
	action_clear_selection = new QAction(QIcon::fromTheme(QStringLiteral("edit-clear")), i18n("Clea&r Content"), this);
	action_select_all = new QAction(QIcon::fromTheme(QStringLiteral("edit-select-all")), i18n("Select All"), this);

	// 	action_set_formula = new QAction(QIcon::fromTheme(QString()), i18n("Assign &Formula"), this);
	// 	action_recalculate = new QAction(QIcon::fromTheme(QString()), i18n("Recalculate"), this);
	// 	action_fill_sel_row_numbers = new QAction(QIcon::fromTheme(QString()), i18n("Row Numbers"), this);
	action_fill_row_numbers = new QAction(QIcon::fromTheme(QString()), i18n("Row Numbers"), this);
	action_fill_random = new QAction(QIcon::fromTheme(QString()), i18n("Uniform Random Values"), this);
	action_fill_random_nonuniform = new QAction(QIcon::fromTheme(QString()), i18n("Random Values"), this);
	action_fill_equidistant = new QAction(QIcon::fromTheme(QString()), i18n("Equidistant Values"), this);
	action_fill_function = new QAction(QIcon::fromTheme(QString()), i18n("Function Values"), this);
	action_fill_const = new QAction(QIcon::fromTheme(QString()), i18n("Const Values"), this);

	action_sample_values = new QAction(QIcon::fromTheme(QStringLiteral("view-list-details")), i18n("Sample Values"), this);
	action_flatten_columns = new QAction(QIcon::fromTheme(QStringLiteral("gnumeric-object-list")), i18n("Flatten Columns"), this);

	// spreadsheet related actions
	action_toggle_comments = new QAction(QIcon::fromTheme(QStringLiteral("document-properties")), i18n("Show Comments"), this);
	action_toggle_sparklines = new QAction(QIcon::fromTheme(QStringLiteral("office-chart-line")), i18n("Show SparkLines"), this);

	action_clear_spreadsheet = new QAction(QIcon::fromTheme(QStringLiteral("edit-clear")), i18n("Clear Spreadsheet"), this);
	action_clear_masks = new QAction(QIcon::fromTheme(QStringLiteral("format-remove-node")), i18n("Clear Masks"), this);
	action_go_to_cell = new QAction(QIcon::fromTheme(QStringLiteral("go-jump")), i18n("&Go to Cell..."), this);
	action_search = new QAction(QIcon::fromTheme(QStringLiteral("edit-find")), i18n("&Search"), this);
	action_search->setShortcut(QKeySequence::Find);
	action_search_replace = new QAction(QIcon::fromTheme(QStringLiteral("edit-find-replace")), i18n("&Replace"), this);
	action_search_replace->setShortcut(QKeySequence::Replace);
	action_statistics_all_columns = new QAction(QIcon::fromTheme(QStringLiteral("view-statistics")), i18n("Column Statistics..."), this);

	action_statistics_spreadsheet = new QAction(QIcon::fromTheme(QStringLiteral("view-statistics")), i18n("Column Statistics Spreadsheet"), this);
	action_statistics_spreadsheet->setCheckable(true);

	// column related actions
	action_insert_column_left = new QAction(QIcon::fromTheme(QStringLiteral("edit-table-insert-column-left")), i18n("Insert Column Left"), this);
	action_insert_column_right = new QAction(QIcon::fromTheme(QStringLiteral("edit-table-insert-column-right")), i18n("Insert Column Right"), this);
	action_insert_columns_left = new QAction(QIcon::fromTheme(QStringLiteral("edit-table-insert-column-left")), i18n("Insert Multiple Columns Left"), this);
	action_insert_columns_right = new QAction(QIcon::fromTheme(QStringLiteral("edit-table-insert-column-right")), i18n("Insert Multiple Columns Right"), this);
	action_remove_columns = new QAction(QIcon::fromTheme(QStringLiteral("edit-table-delete-column")), i18n("Delete Selected Column(s)"), this);
	action_clear_columns = new QAction(QIcon::fromTheme(QStringLiteral("edit-clear")), i18n("Clear Content"), this);
	action_freeze_columns = new QAction(i18n("Freeze Column"), this);

	// TODO: action collection?
	action_set_as_none = new QAction(AbstractColumn::plotDesignationString(AbstractColumn::PlotDesignation::NoDesignation, false), this);
	action_set_as_none->setData(static_cast<int>(AbstractColumn::PlotDesignation::NoDesignation));

	action_set_as_x = new QAction(AbstractColumn::plotDesignationString(AbstractColumn::PlotDesignation::X, false), this);
	action_set_as_x->setData(static_cast<int>(AbstractColumn::PlotDesignation::X));

	action_set_as_y = new QAction(AbstractColumn::plotDesignationString(AbstractColumn::PlotDesignation::Y, false), this);
	action_set_as_y->setData(static_cast<int>(AbstractColumn::PlotDesignation::Y));

	action_set_as_z = new QAction(AbstractColumn::plotDesignationString(AbstractColumn::PlotDesignation::Z, false), this);
	action_set_as_z->setData(static_cast<int>(AbstractColumn::PlotDesignation::Z));

	action_set_as_xerr = new QAction(AbstractColumn::plotDesignationString(AbstractColumn::PlotDesignation::XError, false), this);
	action_set_as_xerr->setData(static_cast<int>(AbstractColumn::PlotDesignation::XError));

	action_set_as_xerr_plus = new QAction(AbstractColumn::plotDesignationString(AbstractColumn::PlotDesignation::XErrorPlus, false), this);
	action_set_as_xerr_plus->setData(static_cast<int>(AbstractColumn::PlotDesignation::XErrorPlus));

	action_set_as_xerr_minus = new QAction(AbstractColumn::plotDesignationString(AbstractColumn::PlotDesignation::XErrorMinus, false), this);
	action_set_as_xerr_minus->setData(static_cast<int>(AbstractColumn::PlotDesignation::XErrorMinus));

	action_set_as_yerr = new QAction(AbstractColumn::plotDesignationString(AbstractColumn::PlotDesignation::YError, false), this);
	action_set_as_yerr->setData(static_cast<int>(AbstractColumn::PlotDesignation::YError));

	action_set_as_yerr_plus = new QAction(AbstractColumn::plotDesignationString(AbstractColumn::PlotDesignation::YErrorPlus, false), this);
	action_set_as_yerr_plus->setData(static_cast<int>(AbstractColumn::PlotDesignation::YErrorPlus));

	action_set_as_yerr_minus = new QAction(AbstractColumn::plotDesignationString(AbstractColumn::PlotDesignation::YErrorMinus, false), this);
	action_set_as_yerr_minus->setData(static_cast<int>(AbstractColumn::PlotDesignation::YErrorMinus));

	// data manipulation
#ifndef SDK
	action_add_value = new QAction(i18n("Add"), this);
	action_add_value->setData(AddSubtractValueDialog::Add);
	action_subtract_value = new QAction(i18n("Subtract"), this);
	action_subtract_value->setData(AddSubtractValueDialog::Subtract);
	action_multiply_value = new QAction(i18n("Multiply"), this);
	action_multiply_value->setData(AddSubtractValueDialog::Multiply);
	action_divide_value = new QAction(i18n("Divide"), this);
	action_divide_value->setData(AddSubtractValueDialog::Divide);
	action_drop_values = new QAction(QIcon::fromTheme(QStringLiteral("delete-table-row")), i18n("Drop Values"), this);
	action_mask_values = new QAction(QIcon::fromTheme(QStringLiteral("hide_table_row")), i18n("Mask Values"), this);

	action_reverse_columns = new QAction(QIcon::fromTheme(QStringLiteral("reverse")), i18n("Reverse"), this);
	// 	action_join_columns = new QAction(QIcon::fromTheme(QString()), i18n("Join"), this);

	// algorithms - baseline subtraction, outliar removal, etc.
	action_subtract_baseline = new QAction(i18n("Subtract Baseline"), this);
	action_subtract_baseline->setData(AddSubtractValueDialog::SubtractBaseline);
#endif

	// normalization
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

	normalizeAction = new QAction(i18n("(x-Mean)/SD"), normalizeColumnActionGroup);
	normalizeAction->setData(ZScoreSD);

	normalizeAction = new QAction(i18n("(x-Median)/MAD"), normalizeColumnActionGroup);
	normalizeAction->setData(ZScoreMAD);

	normalizeAction = new QAction(i18n("(x-Median)/IQR"), normalizeColumnActionGroup);
	normalizeAction->setData(ZScoreIQR);

	normalizeAction = new QAction(i18n("Rescale to [a, b]"), normalizeColumnActionGroup);
	normalizeAction->setData(Rescale);

	// 	action_normalize_selection = new QAction(QIcon::fromTheme(QString()), i18n("&Normalize Selection"), this);

	// Tukey's ladder of powers
	ladderOfPowersActionGroup = new QActionGroup(this);

	QAction* ladderAction = new QAction(QStringLiteral("x³"), ladderOfPowersActionGroup);
	ladderAction->setData(Cube);

	ladderAction = new QAction(QStringLiteral("x²"), ladderOfPowersActionGroup);
	ladderAction->setData(Squared);

	ladderAction = new QAction(QStringLiteral("√x"), ladderOfPowersActionGroup);
	ladderAction->setData(SquareRoot);

	ladderAction = new QAction(QLatin1String("log(x)"), ladderOfPowersActionGroup);
	ladderAction->setData(Log);

	ladderAction = new QAction(QStringLiteral("1/√x"), ladderOfPowersActionGroup);
	ladderAction->setData(InverseSquareRoot);

	ladderAction = new QAction(QLatin1String("1/x"), ladderOfPowersActionGroup);
	ladderAction->setData(Inverse);

	ladderAction = new QAction(QStringLiteral("1/x²"), ladderOfPowersActionGroup);
	ladderAction->setData(InverseSquared);

	// sort and statistics
	action_sort = new QAction(QIcon::fromTheme(QStringLiteral("view-sort")), i18n("&Sort..."), this);
	action_sort->setToolTip(i18n("Sort multiple columns together"));
	action_sort_asc = new QAction(QIcon::fromTheme(QStringLiteral("view-sort-ascending")), i18n("Sort &Ascending"), this);
	action_sort_asc->setToolTip(i18n("Sort the selected columns separately in ascending order"));
	action_sort_desc = new QAction(QIcon::fromTheme(QStringLiteral("view-sort-descending")), i18n("Sort &Descending"), this);
	action_sort_desc->setToolTip(i18n("Sort the selected columns separately in descending order"));
	action_statistics_columns = new QAction(QIcon::fromTheme(QStringLiteral("view-statistics")), i18n("Column Statistics..."), this);

	// conditional formatting
	action_formatting_heatmap = new QAction(QIcon::fromTheme(QStringLiteral("color-management")), i18n("Heatmap"), this);
	action_formatting_remove = new QAction(QIcon::fromTheme(QStringLiteral("edit-clear")), i18n("Delete"), this);

	// row related actions
	action_insert_row_above = new QAction(QIcon::fromTheme(QStringLiteral("edit-table-insert-row-above")), i18n("Insert Row Above"), this);
	action_insert_row_below = new QAction(QIcon::fromTheme(QStringLiteral("edit-table-insert-row-below")), i18n("Insert Row Below"), this);
	action_insert_rows_above = new QAction(QIcon::fromTheme(QStringLiteral("edit-table-insert-row-above")), i18n("Insert Multiple Rows Above"), this);
	action_insert_rows_below = new QAction(QIcon::fromTheme(QStringLiteral("edit-table-insert-row-below")), i18n("Insert Multiple Rows Below"), this);
	action_remove_rows = new QAction(QIcon::fromTheme(QStringLiteral("edit-table-delete-row")), i18n("Remo&ve Selected Row(s)"), this);
	action_remove_missing_value_rows = new QAction(QIcon::fromTheme(QStringLiteral("delete-table-row")), i18n("Delete Rows With Missing Values"), this);
	action_mask_missing_value_rows = new QAction(QIcon::fromTheme(QStringLiteral("hide_table_row")), i18n("Mask Rows With Missing Values"), this);
	action_statistics_rows = new QAction(QIcon::fromTheme(QStringLiteral("view-statistics")), i18n("Row Statisti&cs"), this);

	// Analyze and plot menu actions
	addAnalysisActionGroup = new QActionGroup(this);
	addDataReductionAction = new QAction(QIcon::fromTheme(QStringLiteral("labplot-xy-curve")), i18n("Reduce Data"), addAnalysisActionGroup);
	//	addDataReductionAction = new QAction(QIcon::fromTheme(QStringLiteral("labplot-xy-data-reduction-curve")), i18n("Reduce Data"), this);
	addDataReductionAction->setData(static_cast<int>(XYAnalysisCurve::AnalysisAction::DataReduction));
	addDifferentiationAction = new QAction(QIcon::fromTheme(QStringLiteral("labplot-xy-curve")), i18n("Differentiate"), addAnalysisActionGroup);
	//	addDifferentiationAction = new QAction(QIcon::fromTheme(QStringLiteral("labplot-xy-differentiation-curve")), i18n("Differentiate"), this);
	addDifferentiationAction->setData(static_cast<int>(XYAnalysisCurve::AnalysisAction::Differentiation));
	addIntegrationAction = new QAction(QIcon::fromTheme(QStringLiteral("labplot-xy-curve")), i18n("Integrate"), addAnalysisActionGroup);
	//	addIntegrationAction = new QAction(QIcon::fromTheme(QStringLiteral("labplot-xy-integration-curve")), i18n("Integrate"), this);
	addIntegrationAction->setData(static_cast<int>(XYAnalysisCurve::AnalysisAction::Integration));
	addInterpolationAction = new QAction(QIcon::fromTheme(QStringLiteral("labplot-xy-interpolation-curve")), i18n("Interpolate"), addAnalysisActionGroup);
	addInterpolationAction->setData(static_cast<int>(XYAnalysisCurve::AnalysisAction::Interpolation));
	addSmoothAction = new QAction(QIcon::fromTheme(QStringLiteral("labplot-xy-smoothing-curve")), i18n("Smooth"), addAnalysisActionGroup);
	addSmoothAction->setData(static_cast<int>(XYAnalysisCurve::AnalysisAction::Smoothing));
	addFourierFilterAction = new QAction(QIcon::fromTheme(QStringLiteral("labplot-xy-fourier-filter-curve")), i18n("Fourier Filter"), addAnalysisActionGroup);
	addFourierFilterAction->setData(static_cast<int>(XYAnalysisCurve::AnalysisAction::FourierFilter));

	// data fit actions
	addFitActionGroup = new QActionGroup(this);
	QAction* fitAction = new QAction(QIcon::fromTheme(QStringLiteral("labplot-xy-fit-curve")), i18n("Linear"), addFitActionGroup);
	fitAction->setData(static_cast<int>(XYAnalysisCurve::AnalysisAction::FitLinear));

	fitAction = new QAction(QIcon::fromTheme(QStringLiteral("labplot-xy-fit-curve")), i18n("Power"), addFitActionGroup);
	fitAction->setData(static_cast<int>(XYAnalysisCurve::AnalysisAction::FitPower));

	fitAction = new QAction(QIcon::fromTheme(QStringLiteral("labplot-xy-fit-curve")), i18n("Exponential (degree 1)"), addFitActionGroup);
	fitAction->setData(static_cast<int>(XYAnalysisCurve::AnalysisAction::FitExp1));

	fitAction = new QAction(QIcon::fromTheme(QStringLiteral("labplot-xy-fit-curve")), i18n("Exponential (degree 2)"), addFitActionGroup);
	fitAction->setData(static_cast<int>(XYAnalysisCurve::AnalysisAction::FitExp2));

	fitAction = new QAction(QIcon::fromTheme(QStringLiteral("labplot-xy-fit-curve")), i18n("Inverse Exponential"), addFitActionGroup);
	fitAction->setData(static_cast<int>(XYAnalysisCurve::AnalysisAction::FitInvExp));

	fitAction = new QAction(QIcon::fromTheme(QStringLiteral("labplot-xy-fit-curve")), i18n("Gauss"), addFitActionGroup);
	fitAction->setData(static_cast<int>(XYAnalysisCurve::AnalysisAction::FitGauss));

	fitAction = new QAction(QIcon::fromTheme(QStringLiteral("labplot-xy-fit-curve")), i18n("Cauchy-Lorentz"), addFitActionGroup);
	fitAction->setData(static_cast<int>(XYAnalysisCurve::AnalysisAction::FitCauchyLorentz));

	fitAction = new QAction(QIcon::fromTheme(QStringLiteral("labplot-xy-fit-curve")), i18n("Arc Tangent"), addFitActionGroup);
	fitAction->setData(static_cast<int>(XYAnalysisCurve::AnalysisAction::FitTan));

	fitAction = new QAction(QIcon::fromTheme(QStringLiteral("labplot-xy-fit-curve")), i18n("Hyperbolic Tangent"), addFitActionGroup);
	fitAction->setData(static_cast<int>(XYAnalysisCurve::AnalysisAction::FitTanh));

	fitAction = new QAction(QIcon::fromTheme(QStringLiteral("labplot-xy-fit-curve")), i18n("Error Function"), addFitActionGroup);
	fitAction->setData(static_cast<int>(XYAnalysisCurve::AnalysisAction::FitErrFunc));

	fitAction = new QAction(QIcon::fromTheme(QStringLiteral("labplot-xy-fit-curve")), i18n("Custom"), addFitActionGroup);
	fitAction->setData(static_cast<int>(XYAnalysisCurve::AnalysisAction::FitCustom));

	// distribution fit actions
	addDistributionFitActionGroup = new QActionGroup(this);
	fitAction = new QAction(QIcon::fromTheme(QStringLiteral("labplot-xy-fit-curve")), i18n("Gaussian (Normal)"), addDistributionFitActionGroup);
	fitAction->setData(static_cast<int>(nsl_sf_stats_gaussian));

	fitAction = new QAction(QIcon::fromTheme(QStringLiteral("labplot-xy-fit-curve")), i18n("Exponential"), addDistributionFitActionGroup);
	fitAction->setData(static_cast<int>(nsl_sf_stats_exponential));

	fitAction = new QAction(QIcon::fromTheme(QStringLiteral("labplot-xy-fit-curve")), i18n("Laplace"), addDistributionFitActionGroup);
	fitAction->setData(static_cast<int>(nsl_sf_stats_laplace));

	fitAction = new QAction(QIcon::fromTheme(QStringLiteral("labplot-xy-fit-curve")), i18n("Cauchy-Lorentz"), addDistributionFitActionGroup);
	fitAction->setData(static_cast<int>(nsl_sf_stats_cauchy_lorentz));

	fitAction = new QAction(QIcon::fromTheme(QStringLiteral("labplot-xy-fit-curve")), i18n("Log-normal"), addDistributionFitActionGroup);
	fitAction->setData(static_cast<int>(nsl_sf_stats_lognormal));

	fitAction = new QAction(QIcon::fromTheme(QStringLiteral("labplot-xy-fit-curve")), i18n("Poisson"), addDistributionFitActionGroup);
	fitAction->setData(static_cast<int>(nsl_sf_stats_poisson));

	fitAction = new QAction(QIcon::fromTheme(QStringLiteral("labplot-xy-fit-curve")), i18n("Binomial"), addDistributionFitActionGroup);
	fitAction->setData(static_cast<int>(nsl_sf_stats_binomial));

	connectActions();
}

void SpreadsheetView::initMenus() {
	initActions();

	// Selection menu
	m_selectionMenu = new QMenu(i18n("Selection"), this);
	m_selectionMenu->setIcon(QIcon::fromTheme(QStringLiteral("selection")));
	connect(m_selectionMenu, &QMenu::aboutToShow, this, &SpreadsheetView::checkSpreadsheetSelectionMenu);

	if (!m_readOnly) {
		// 		submenu = new QMenu(i18n("Fi&ll Selection With"), this);
		// 		submenu->setIcon(QIcon::fromTheme(QStringLiteral("select-rectangle")));
		// 		submenu->addAction(action_fill_sel_row_numbers);
		// 		submenu->addAction(action_fill_const);
		// 		m_selectionMenu->addMenu(submenu);
		// 		m_selectionMenu->addSeparator();
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
		m_selectionMenu->addAction(action_reverse_selection);
		// 		m_selectionMenu->addAction(action_normalize_selection);
	}

	// plot data menu
	plotDataActionGroup = new QActionGroup(this);
	connect(plotDataActionGroup, &QActionGroup::triggered, this, &SpreadsheetView::plotData);
	m_plotDataMenu = new QMenu(i18n("Plot Data"), this);
	PlotDataDialog::fillMenu(m_plotDataMenu, plotDataActionGroup);

	// conditional formatting
	m_formattingMenu = new QMenu(i18n("Conditional Formatting"), this);
	m_formattingMenu->addAction(action_formatting_heatmap);
	m_formattingMenu->addSeparator();
	m_formattingMenu->addAction(action_formatting_remove);

	// Column menu
	m_columnMenu = new QMenu(this);
	m_columnMenu->addMenu(m_plotDataMenu);

	// Data fit sub-menu
	QMenu* dataFitMenu = new QMenu(i18nc("Curve fitting", "Fit"), this);
	dataFitMenu->setIcon(QIcon::fromTheme(QStringLiteral("labplot-xy-fit-curve")));
	const auto& addFitActions = addFitActionGroup->actions();
	dataFitMenu->addAction(addFitActions.at(0));
	dataFitMenu->addAction(addFitActions.at(1));
	dataFitMenu->addAction(addFitActions.at(2));
	dataFitMenu->addAction(addFitActions.at(3));
	dataFitMenu->addAction(addFitActions.at(4));
	dataFitMenu->addSeparator();
	dataFitMenu->addAction(addFitActions.at(5));
	dataFitMenu->addAction(addFitActions.at(6));
	dataFitMenu->addSeparator();
	dataFitMenu->addAction(addFitActions.at(7));
	dataFitMenu->addAction(addFitActions.at(8));
	dataFitMenu->addAction(addFitActions.at(9));
	dataFitMenu->addSeparator();
	dataFitMenu->addAction(addFitActions.at(10));

	// distribution fit sub-menu
	QMenu* distributionFitMenu = new QMenu(i18nc("Curve fitting", "Fit Distribution"), this);
	distributionFitMenu->setIcon(QIcon::fromTheme(QStringLiteral("labplot-xy-fit-curve")));
	const auto& addDistributionFitActions = addDistributionFitActionGroup->actions();
	distributionFitMenu->addAction(addDistributionFitActions.at(0));
	distributionFitMenu->addAction(addDistributionFitActions.at(1));
	distributionFitMenu->addAction(addDistributionFitActions.at(2));
	distributionFitMenu->addAction(addDistributionFitActions.at(3));
	distributionFitMenu->addAction(addDistributionFitActions.at(4));
	dataFitMenu->addSeparator();
	distributionFitMenu->addAction(addDistributionFitActions.at(5));
	distributionFitMenu->addAction(addDistributionFitActions.at(6));
	dataFitMenu->addSeparator();

	// analyze and plot data menu
	m_analyzePlotMenu = new QMenu(i18n("Analyze and Plot Data"), this);
	m_analyzePlotMenu->addMenu(dataFitMenu);
	m_analyzePlotMenu->addMenu(distributionFitMenu);
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
	m_columnSetAsMenu->addAction(action_set_as_xerr_plus);
	m_columnSetAsMenu->addAction(action_set_as_xerr_minus);
	m_columnSetAsMenu->addSeparator();
	m_columnSetAsMenu->addAction(action_set_as_yerr);
	m_columnSetAsMenu->addAction(action_set_as_yerr_plus);
	m_columnSetAsMenu->addAction(action_set_as_yerr_minus);
	m_columnSetAsMenu->addSeparator();
	m_columnSetAsMenu->addAction(action_set_as_none);
	m_columnMenu->addMenu(m_columnSetAsMenu);

	if (!m_readOnly) {
		m_columnGenerateDataMenu = new QMenu(i18n("Generate Data"), this);
		m_columnGenerateDataMenu->addAction(action_fill_row_numbers);
		m_columnGenerateDataMenu->addAction(action_fill_const);
		m_columnGenerateDataMenu->addSeparator();
		m_columnGenerateDataMenu->addAction(action_fill_equidistant);
		m_columnGenerateDataMenu->addAction(action_fill_random_nonuniform);
		m_columnGenerateDataMenu->addAction(action_fill_function);
		m_columnGenerateDataMenu->addSeparator();
		m_columnGenerateDataMenu->addAction(action_sample_values);
		m_columnGenerateDataMenu->addAction(action_flatten_columns);

		m_columnMenu->addSeparator();
		m_columnMenu->addMenu(m_columnGenerateDataMenu);
		m_columnMenu->addSeparator();

		m_columnManipulateDataMenu = new QMenu(i18n("Manipulate Data"), this);
		m_columnManipulateDataMenu->addAction(action_add_value);
		m_columnManipulateDataMenu->addAction(action_subtract_value);
		m_columnManipulateDataMenu->addAction(action_multiply_value);
		m_columnManipulateDataMenu->addAction(action_divide_value);
		m_columnManipulateDataMenu->addSeparator();
		m_columnManipulateDataMenu->addAction(action_subtract_baseline);
		m_columnManipulateDataMenu->addSeparator();
		m_columnManipulateDataMenu->addAction(action_reverse_columns);
		m_columnManipulateDataMenu->addSeparator();
		m_columnManipulateDataMenu->addAction(action_drop_values);
		m_columnManipulateDataMenu->addAction(action_mask_values);
		m_columnManipulateDataMenu->addSeparator();
		// 	m_columnManipulateDataMenu->addAction(action_join_columns);

		// normalization menu with the following structure
		// Divide by Sum
		// Divide by Min
		// Divide by Max
		// Divide by Count
		//--------------
		// Divide by Mean
		// Divide by Median
		// Divide by Mode
		//---------------
		// Divide by Range
		// Divide by SD
		// Divide by MAD
		// Divide by IQR
		//--------------
		//(x-Mean)/SD
		//(x-Median)/MAD
		//(x-Median)/IQR
		//--------------
		// Rescale to [a, b]

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

		//"Ladder of powers" transformation
		m_columnLadderOfPowersMenu = new QMenu(i18n("Ladder of Powers"), this);
		m_columnLadderOfPowersMenu->addAction(ladderOfPowersActionGroup->actions().at(0));
		m_columnLadderOfPowersMenu->addAction(ladderOfPowersActionGroup->actions().at(1));
		m_columnLadderOfPowersMenu->addAction(ladderOfPowersActionGroup->actions().at(2));
		m_columnLadderOfPowersMenu->addAction(ladderOfPowersActionGroup->actions().at(3));
		m_columnLadderOfPowersMenu->addAction(ladderOfPowersActionGroup->actions().at(4));
		m_columnLadderOfPowersMenu->addAction(ladderOfPowersActionGroup->actions().at(5));
		m_columnLadderOfPowersMenu->addAction(ladderOfPowersActionGroup->actions().at(6));

		m_columnManipulateDataMenu->addSeparator();
		m_columnManipulateDataMenu->addMenu(m_columnLadderOfPowersMenu);

		m_columnMenu->addMenu(m_columnManipulateDataMenu);
		m_columnMenu->addSeparator();

		m_columnMenu->addMenu(m_selectionMenu);
		m_columnMenu->addSeparator();

		m_columnMenu->addAction(action_sort);
		m_columnMenu->addAction(action_sort_asc);
		m_columnMenu->addAction(action_sort_desc);
	}

	m_columnMenu->addSeparator();
	m_columnMenu->addMenu(m_formattingMenu);
	m_columnMenu->addSeparator();
	m_columnMenu->addAction(action_freeze_columns);
	m_columnMenu->addSeparator();
	m_columnMenu->addAction(action_statistics_columns);

	if (!m_readOnly) {
		m_columnMenu->addSeparator();
		m_columnMenu->addAction(action_insert_column_left);
		m_columnMenu->addAction(action_insert_column_right);
		m_columnMenu->addSeparator();
		m_columnMenu->addAction(action_insert_columns_left);
		m_columnMenu->addAction(action_insert_columns_right);
		m_columnMenu->addSeparator();
		m_columnMenu->addAction(action_remove_columns);
		m_columnMenu->addAction(action_clear_columns);
		m_columnMenu->addSeparator();
		m_columnMenu->addAction(action_remove_missing_value_rows);
		m_columnMenu->addAction(action_mask_missing_value_rows);
	}

	// Spreadsheet menu
	m_spreadsheetMenu = new QMenu(this);
	createContextMenu(m_spreadsheetMenu);

	// Row menu
	m_rowMenu = new QMenu(this);
	if (!m_readOnly) {
		// 		submenu = new QMenu(i18n("Fi&ll Selection With"), this);
		// 		submenu->addAction(action_fill_sel_row_numbers);
		// 		submenu->addAction(action_fill_const);
		// 		m_rowMenu->addMenu(submenu);
		// 		m_rowMenu->addSeparator();

		m_rowMenu->addMenu(m_selectionMenu);
		m_rowMenu->addSeparator();

		m_rowMenu->addAction(action_insert_row_above);
		m_rowMenu->addAction(action_insert_row_below);
		m_rowMenu->addSeparator();

		m_rowMenu->addAction(action_insert_rows_above);
		m_rowMenu->addAction(action_insert_rows_below);
		m_rowMenu->addSeparator();

		m_rowMenu->addAction(action_remove_rows);
		m_rowMenu->addSeparator();

		m_rowMenu->addAction(action_remove_missing_value_rows);
		m_rowMenu->addAction(action_mask_missing_value_rows);
	}

	m_rowMenu->addSeparator();
	m_rowMenu->addAction(action_statistics_rows);
}

void SpreadsheetView::connectActions() {
	connect(action_cut_selection, &QAction::triggered, this, &SpreadsheetView::cutSelection);
	connect(action_copy_selection, &QAction::triggered, this, &SpreadsheetView::copySelection);
	connect(action_paste_into_selection, &QAction::triggered, this, &SpreadsheetView::pasteIntoSelection);
	connect(action_mask_selection, &QAction::triggered, this, &SpreadsheetView::maskSelection);
	connect(action_unmask_selection, &QAction::triggered, this, &SpreadsheetView::unmaskSelection);
	connect(action_reverse_selection, &QAction::triggered, this, &SpreadsheetView::reverseSelection);

	connect(action_clear_selection, &QAction::triggered, this, &SpreadsheetView::clearSelectedCells);
	// 	connect(action_recalculate, &QAction::triggered, this, &SpreadsheetView::recalculateSelectedCells);
	connect(action_fill_row_numbers, &QAction::triggered, this, &SpreadsheetView::fillWithRowNumbers);
	// 	connect(action_fill_sel_row_numbers, &QAction::triggered, this, &SpreadsheetView::fillSelectedCellsWithRowNumbers);
	// 	connect(action_fill_random, &QAction::triggered, this, &SpreadsheetView::fillSelectedCellsWithRandomNumbers);
	connect(action_fill_random_nonuniform, &QAction::triggered, this, &SpreadsheetView::fillWithRandomValues);
	connect(action_fill_equidistant, &QAction::triggered, this, &SpreadsheetView::fillWithEquidistantValues);
	connect(action_fill_function, &QAction::triggered, this, &SpreadsheetView::fillWithFunctionValues);
	connect(action_fill_const, &QAction::triggered, this, &SpreadsheetView::fillSelectedCellsWithConstValues);
	connect(action_select_all, &QAction::triggered, this, &SpreadsheetView::selectAll);
	connect(action_clear_spreadsheet, &QAction::triggered, m_spreadsheet, QOverload<>::of(&Spreadsheet::clear));
	connect(action_clear_masks, &QAction::triggered, m_spreadsheet, &Spreadsheet::clearMasks);
	connect(action_go_to_cell, &QAction::triggered, this, static_cast<void (SpreadsheetView::*)()>(&SpreadsheetView::goToCell));
	connect(action_search, &QAction::triggered, this, &SpreadsheetView::searchReplace);
	connect(action_search_replace, &QAction::triggered, this, &SpreadsheetView::searchReplace);

	connect(action_insert_column_left, &QAction::triggered, this, &SpreadsheetView::insertColumnLeft);
	connect(action_insert_column_right, &QAction::triggered, this, &SpreadsheetView::insertColumnRight);
	connect(action_insert_columns_left, &QAction::triggered, this, static_cast<void (SpreadsheetView::*)()>(&SpreadsheetView::insertColumnsLeft));
	connect(action_insert_columns_right, &QAction::triggered, this, static_cast<void (SpreadsheetView::*)()>(&SpreadsheetView::insertColumnsRight));
	connect(action_remove_columns, &QAction::triggered, this, &SpreadsheetView::removeSelectedColumns);
	connect(action_clear_columns, &QAction::triggered, this, &SpreadsheetView::clearSelectedColumns);
	connect(action_freeze_columns, &QAction::triggered, this, &SpreadsheetView::toggleFreezeColumn);
	connect(action_set_as_none, &QAction::triggered, this, &SpreadsheetView::setSelectionAs);
	connect(action_set_as_x, &QAction::triggered, this, &SpreadsheetView::setSelectionAs);
	connect(action_set_as_y, &QAction::triggered, this, &SpreadsheetView::setSelectionAs);
	connect(action_set_as_z, &QAction::triggered, this, &SpreadsheetView::setSelectionAs);
	connect(action_set_as_xerr, &QAction::triggered, this, &SpreadsheetView::setSelectionAs);
	connect(action_set_as_xerr_plus, &QAction::triggered, this, &SpreadsheetView::setSelectionAs);
	connect(action_set_as_xerr_minus, &QAction::triggered, this, &SpreadsheetView::setSelectionAs);
	connect(action_set_as_yerr, &QAction::triggered, this, &SpreadsheetView::setSelectionAs);
	connect(action_set_as_yerr_plus, &QAction::triggered, this, &SpreadsheetView::setSelectionAs);
	connect(action_set_as_yerr_minus, &QAction::triggered, this, &SpreadsheetView::setSelectionAs);

	// data manipulation
	connect(action_add_value, &QAction::triggered, this, &SpreadsheetView::modifyValues);
	connect(action_subtract_value, &QAction::triggered, this, &SpreadsheetView::modifyValues);
	connect(action_multiply_value, &QAction::triggered, this, &SpreadsheetView::modifyValues);
	connect(action_divide_value, &QAction::triggered, this, &SpreadsheetView::modifyValues);
	connect(action_reverse_columns, &QAction::triggered, this, &SpreadsheetView::reverseColumns);
	connect(action_drop_values, &QAction::triggered, this, &SpreadsheetView::dropColumnValues);
	connect(action_mask_values, &QAction::triggered, this, &SpreadsheetView::maskColumnValues);
	connect(action_sample_values, &QAction::triggered, this, &SpreadsheetView::sampleColumnValues);
	connect(action_flatten_columns, &QAction::triggered, this, &SpreadsheetView::flattenColumns);

	// algorithms
	connect(action_subtract_baseline, &QAction::triggered, this, &SpreadsheetView::modifyValues);

	// 	connect(action_join_columns, &QAction::triggered, this, &SpreadsheetView::joinColumns);
	connect(normalizeColumnActionGroup, &QActionGroup::triggered, this, &SpreadsheetView::normalizeSelectedColumns);
	connect(ladderOfPowersActionGroup, &QActionGroup::triggered, this, &SpreadsheetView::powerTransformSelectedColumns);
	// 	connect(action_normalize_selection, &QAction::triggered, this, &SpreadsheetView::normalizeSelection);

	// sort
	connect(action_sort, &QAction::triggered, this, &SpreadsheetView::sortCustom);
	connect(action_sort_asc, &QAction::triggered, this, &SpreadsheetView::sortAscending);
	connect(action_sort_desc, &QAction::triggered, this, &SpreadsheetView::sortDescending);

	// conditional formatting
	connect(action_formatting_heatmap, &QAction::triggered, this, &SpreadsheetView::formatHeatmap);
	connect(action_formatting_remove, &QAction::triggered, this, &SpreadsheetView::removeFormat);

	// statistics
	connect(action_statistics_columns, &QAction::triggered, this, &SpreadsheetView::showColumnStatistics);
	connect(action_statistics_all_columns, &QAction::triggered, this, &SpreadsheetView::showAllColumnsStatistics);
	connect(action_statistics_spreadsheet, &QAction::toggled, m_spreadsheet, &Spreadsheet::toggleStatisticsSpreadsheet);

	// rows
	connect(action_insert_row_above, &QAction::triggered, this, &SpreadsheetView::insertRowAbove);
	connect(action_insert_row_below, &QAction::triggered, this, &SpreadsheetView::insertRowBelow);
	connect(action_insert_rows_above, &QAction::triggered, this, static_cast<void (SpreadsheetView::*)()>(&SpreadsheetView::insertRowsAbove));
	connect(action_insert_rows_below, &QAction::triggered, this, static_cast<void (SpreadsheetView::*)()>(&SpreadsheetView::insertRowsBelow));
	connect(action_remove_rows, &QAction::triggered, this, &SpreadsheetView::removeSelectedRows);
	connect(action_remove_missing_value_rows, &QAction::triggered, m_spreadsheet, &Spreadsheet::removeEmptyRows);
	connect(action_mask_missing_value_rows, &QAction::triggered, m_spreadsheet, &Spreadsheet::maskEmptyRows);
	connect(action_statistics_rows, &QAction::triggered, this, &SpreadsheetView::showRowStatistics);
	connect(action_toggle_comments, &QAction::triggered, this, &SpreadsheetView::toggleComments);

	connect(action_toggle_sparklines, &QAction::triggered, this, &SpreadsheetView::toggleSparkLines);

	// analysis
	connect(addAnalysisActionGroup, &QActionGroup::triggered, this, &SpreadsheetView::plotAnalysisData);
	connect(addFitActionGroup, &QActionGroup::triggered, this, &SpreadsheetView::plotAnalysisData);
	connect(addDistributionFitActionGroup, &QActionGroup::triggered, this, &SpreadsheetView::plotDataDistributionFit);
}

#ifdef HAVE_TOUCHBAR
void SpreadsheetView::fillTouchBar(KDMacTouchBar* touchBar) {
	// touchBar->addAction(action_insert_column_right);
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

	if (!m_selectionMenu)
		initMenus();

	checkSpreadsheetMenu();

	QAction* firstAction = nullptr;
	// if we're populating the context menu for the project explorer, then
	// there're already actions available there. Skip the first title-action
	// and insert the action at the beginning of the menu.
	if (menu->actions().size() > 1)
		firstAction = menu->actions().at(1);

	if (m_spreadsheet->columnCount() > 0 && m_spreadsheet->rowCount() > 0) {
		menu->insertMenu(firstAction, m_plotDataMenu);
		menu->insertMenu(firstAction, m_analyzePlotMenu);
		menu->insertSeparator(firstAction);
	}
	menu->insertMenu(firstAction, m_selectionMenu);
	menu->insertSeparator(firstAction);
	menu->insertAction(firstAction, action_select_all);
	if (!m_readOnly) {
		menu->insertAction(firstAction, action_clear_spreadsheet);
		menu->insertAction(firstAction, action_clear_masks);
		menu->insertAction(firstAction, action_sort);
		menu->insertSeparator(firstAction);
	}

	menu->insertMenu(firstAction, m_formattingMenu);
	menu->insertSeparator(firstAction);
	menu->insertAction(firstAction, action_go_to_cell);
	menu->insertAction(firstAction, action_search);
	if (!m_readOnly)
		menu->insertAction(firstAction, action_search_replace);
	menu->insertSeparator(firstAction);
	menu->insertAction(firstAction, action_toggle_comments);
	menu->insertAction(firstAction, action_toggle_sparklines);
	menu->insertSeparator(firstAction);
	menu->insertAction(firstAction, action_statistics_spreadsheet);
	menu->insertSeparator(firstAction);
	menu->insertAction(firstAction, action_statistics_all_columns);
	menu->insertSeparator(firstAction);
}

/*!
 * adds column specific actions in SpreadsheetView to the context menu shown in the project explorer.
 */
void SpreadsheetView::fillColumnContextMenu(QMenu* menu, Column* column) {
	if (!column)
		return; // should never happen, since the sender is always a Column

	if (!m_selectionMenu)
		initMenus();

	const bool numeric = column->isNumeric();
	const bool datetime = (column->columnMode() == AbstractColumn::ColumnMode::DateTime);

	QAction* firstAction = menu->actions().at(1);
	menu->insertMenu(firstAction, m_plotDataMenu);
	menu->insertMenu(firstAction, m_analyzePlotMenu);
	menu->insertSeparator(firstAction);

	if (numeric)
		menu->insertMenu(firstAction, m_columnSetAsMenu);

	if (!m_readOnly) {
		if (numeric) {
			menu->insertSeparator(firstAction);
			menu->insertMenu(firstAction, m_columnGenerateDataMenu);
			menu->insertSeparator(firstAction);
		}

		if (numeric || datetime) {
			menu->insertMenu(firstAction, m_columnManipulateDataMenu);
			menu->insertSeparator(firstAction);
		}

		menu->insertAction(firstAction, action_sort);
		menu->insertAction(firstAction, action_sort_asc);
		menu->insertAction(firstAction, action_sort_desc);
	}

	menu->insertSeparator(firstAction);
	menu->insertMenu(firstAction, m_formattingMenu);
	menu->insertSeparator(firstAction);
	menu->insertAction(firstAction, action_freeze_columns);
	menu->insertSeparator(firstAction);
	menu->insertAction(firstAction, action_statistics_columns);

	checkColumnMenus(QVector<Column*>{column});
}

// SLOTS
void SpreadsheetView::handleAspectsAdded(int first, int last) {
	if (m_suppressResize) {
		m_suppressResize = false;
		return;
	}
	PERFTRACE(QLatin1String(Q_FUNC_INFO));

	const auto& children = m_spreadsheet->children<Column>();

	for (int i = first; i <= last; i++) {
		const auto* col = children.at(i);
		// TODO: this makes it slow!
		if (col->width() == 0)
			m_tableView->resizeColumnToContents(i);
		else
			m_tableView->setColumnWidth(i, col->width());
	}

	goToCell(0, last);
}

void SpreadsheetView::handleAspectAboutToBeRemoved(int first, int last) {
	const auto& children = m_spreadsheet->children<Column>();
	if (first < 0 || first >= children.count() || last >= children.count() || first > last)
		return;

	for (int i = first; i <= last; i++)
		disconnect(children.at(i), nullptr, this, nullptr);
}

void SpreadsheetView::handleHorizontalSectionResized(int logicalIndex, int /* oldSize */, int newSize) {
	// save the new size in the column
	Column* col = m_spreadsheet->child<Column>(logicalIndex);
	col->setWidth(newSize);

	if (m_frozenTableView && logicalIndex == 0) {
		m_frozenTableView->setColumnWidth(0, newSize);
		updateFrozenTableGeometry();
	}
}

void SpreadsheetView::goToCell(int row, int col) {
	QModelIndex index = m_model->index(row, col);
	m_tableView->scrollTo(index);
	m_tableView->setCurrentIndex(index);
}

void SpreadsheetView::selectCell(int row, int col) {
	m_tableView->selectionModel()->select(m_model->index(row, col), QItemSelectionModel::Select);
}

void SpreadsheetView::clearSelection() {
	m_tableView->selectionModel()->clear();
}

void SpreadsheetView::handleHorizontalSectionMoved(int index, int from, int to) {
	static bool inside = false;
	if (inside)
		return;

	Q_ASSERT(index == from);

	inside = true;
	m_tableView->horizontalHeader()->moveSection(to, from);
	inside = false;
	m_spreadsheet->moveColumn(from, to);
}

// TODO Implement the "change of the column name"-mode upon a double click
void SpreadsheetView::handleHorizontalHeaderDoubleClicked(int /*index*/) {
}

/*!
  Returns whether comments are shown currently or not
*/
bool SpreadsheetView::areCommentsShown() const {
	return m_horizontalHeader->areCommentsShown();
}

/*!
  Returns whether spark lines are shown currently or not
*/
bool SpreadsheetView::areSparkLinesShown() const {
	return m_horizontalHeader->areSparkLinesShown();
}
/*!
  toggles the column comment in the horizontal header
*/
void SpreadsheetView::toggleComments() {
	showComments(!areCommentsShown());
}
/*!
  toggles the column spark line in the horizontal header
*/
void SpreadsheetView::toggleSparkLines() {
	showSparkLines(!areSparkLinesShown());
}

//! Shows (\c on=true) or hides (\c on=false) the column comments in the horizontal header
void SpreadsheetView::showComments(bool on) {
	m_horizontalHeader->showComments(on);
	if (action_toggle_comments) {
		if (on)
			action_toggle_comments->setText(i18n("Hide Comments"));
		else
			action_toggle_comments->setText(i18n("Show Comments"));
	}
}

//! Shows (\c on=true) or hides (\c on=false) the column sparkline in the horizontal header
void SpreadsheetView::showSparkLines(bool on) {
	m_horizontalHeader->showSparkLines(on);
	if (action_toggle_sparklines) {
		if (on)
			action_toggle_sparklines->setText(i18n("Hide Sparklines"));
		else
			action_toggle_sparklines->setText(i18n("Show Sparklines"));
	}
}

void SpreadsheetView::handleHeaderDataChanged(Qt::Orientation orientation, int first, int last) {
	if (orientation != Qt::Horizontal)
		return;

	for (int index = first; index <= last; ++index)
		m_tableView->resizeColumnToContents(index);
}

/*!
 * return the selection model of the tree view, private function wrapper to be able
 * to unit-test the selection without exposing the whole internal table view.
 */
QItemSelectionModel* SpreadsheetView::selectionModel() {
	return m_tableView->selectionModel();
}

/*!
  Returns the number of selected columns.
  If \c full is \c true, this function only returns the number of fully selected columns.
*/
int SpreadsheetView::selectedColumnCount(bool full) const {
	int count = 0;
	const int cols = m_spreadsheet->columnCount();
	for (int i = 0; i < cols; i++)
		if (isColumnSelected(i, full))
			count++;
	return count;
}

int SpreadsheetView::selectedRowCount(bool full) const {
	if (full)
		return m_tableView->selectionModel()->selectedRows().count();
	const auto& indexes = m_tableView->selectionModel()->selectedIndexes();
	QSet<int> set;
	for (auto& index : indexes)
		set.insert(index.row());

	return set.count();
}

/*!
  Returns the number of (at least partly) selected columns with the plot designation \param pd .
 */
int SpreadsheetView::selectedColumnCount(AbstractColumn::PlotDesignation pd) const {
	int count = 0;
	const int cols = m_spreadsheet->columnCount();
	for (int i = 0; i < cols; i++)
		if (isColumnSelected(i, false) && (m_spreadsheet->column(i)->plotDesignation() == pd))
			count++;

	return count;
}

/*!
  Returns \c true if column \param col is selected, otherwise returns \c false.
  If \param full is \c true, this function only returns true if the whole column is selected.
*/
bool SpreadsheetView::isColumnSelected(int col, bool full) const {
	if (full)
		return m_tableView->selectionModel()->isColumnSelected(col);
	else
		return m_tableView->selectionModel()->columnIntersectsSelection(col);
}

/*!
  Returns all selected columns.
  If \param full is true, this function only returns a column if the whole column is selected.
  */
QVector<Column*> SpreadsheetView::selectedColumns(bool full) const {
	QVector<Column*> columns;
	const int cols = m_spreadsheet->columnCount();
	for (int i = 0; i < cols; i++)
		if (isColumnSelected(i, full))
			columns << m_spreadsheet->column(i);

	return columns;
}

/*!
  Returns \c true if row \param row is selected; otherwise returns \c false
  If \param full is \c true, this function only returns \c true if the whole row is selected.
*/
bool SpreadsheetView::isRowSelected(int row, bool full) const {
	if (full)
		return m_tableView->selectionModel()->isRowSelected(row);
	else
		return m_tableView->selectionModel()->rowIntersectsSelection(row);
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
		if (isColumnSelected(i, full))
			return i;

	return -2;
}

/*!
  Return the index of the first selected row.
  If \param full is \c true, this function only looks for fully selected rows.
  */
int SpreadsheetView::firstSelectedRow(bool full) const {
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
	m_tableView->selectionModel()->select(m_model->index(row, col), select ? QItemSelectionModel::Select : QItemSelectionModel::Deselect);
}

/*!
  Select/Deselect a range of cells.
 */
void SpreadsheetView::setCellsSelected(int first_row, int first_col, int last_row, int last_col, bool select) {
	QModelIndex top_left = m_model->index(first_row, first_col);
	QModelIndex bottom_right = m_model->index(last_row, last_col);
	m_tableView->selectionModel()->select(QItemSelection(top_left, bottom_right), select ? QItemSelectionModel::SelectCurrent : QItemSelectionModel::Deselect);
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
		// initialize all menus and actions if not done yet
		if (!m_plotDataMenu)
			initMenus();

		auto* cm_event = static_cast<QContextMenuEvent*>(event);
		const QPoint global_pos = cm_event->globalPos();

		if (watched == m_tableView->verticalHeader()) {
			bool hasValues = false;
			const auto& rows = m_tableView->selectionModel()->selectedRows();
			for (int i = 0; i < rows.count(); ++i) {
				int row = rows.at(i).row();

				for (int j = 0; j < m_spreadsheet->columnCount(); ++j) {
					if (m_spreadsheet->column(j)->hasValueAt(row)) {
						hasValues = true;
						break;
					}
				}

				if (hasValues)
					break;
			}

			m_selectionMenu->setEnabled(hasValues);
			action_statistics_rows->setEnabled(hasValues);
			m_rowMenu->exec(global_pos);
		} else if ((watched == m_horizontalHeader) || (m_frozenTableView && watched == m_frozenTableView->horizontalHeader()) || !selectedColumns().isEmpty()) {
			// if the horizontal header was clicked, select the column under the cursor if not selected yet
			if (watched == m_horizontalHeader) {
				const int col = m_horizontalHeader->logicalIndexAt(cm_event->pos());
				if (!isColumnSelected(col, true)) {
					auto* sel_model = m_tableView->selectionModel();
					sel_model->clearSelection();
					QItemSelection selection(m_model->index(0, col, QModelIndex()), m_model->index(m_model->rowCount() - 1, col, QModelIndex()));
					sel_model->select(selection, QItemSelectionModel::Select);
				}
			}

			checkColumnMenus(selectedColumns());
			m_columnMenu->exec(global_pos);
		} else if (watched == this) {
			// the cursor position is in one of the cells and no full columns are selected,
			// show the global spreadsheet context menu in this case
			// TODO: deactivate the "selection" menu if there are no values in the selected cells
			checkSpreadsheetMenu();
			m_spreadsheetMenu->exec(global_pos);
		}

		return true;
	} else if (event->type() == QEvent::KeyPress) {
		auto* key_event = static_cast<QKeyEvent*>(event);
		if (key_event->matches(QKeySequence::Copy))
			copySelection();
		else if (key_event->matches(QKeySequence::Paste)) {
			if (!m_readOnly)
				pasteIntoSelection();
		} else if (key_event->key() == Qt::Key_Backspace || key_event->matches(QKeySequence::Delete))
			clearSelectedCells();
		else if (key_event->key() == Qt::Key_Return || key_event->key() == Qt::Key_Enter) {
			// only advance for return pressed events in the table,
			// ignore it in the search field that has its own handling for it
			if (watched == m_tableView)
				advanceCell();
		} else if (key_event->key() == Qt::Key_Insert) {
			if (!m_editorEntered) {
				if (lastSelectedColumn(true) >= 0)
					insertColumnRight();
				else
					insertRowBelow();
			}
		} else if (key_event->key() == Qt::Key_Left) {
			// adjusted example from
			// https://doc.qt.io/qt-5/qtwidgets-itemviews-frozencolumn-example.html
			// 			QModelIndex current = m_tableView->currentIndex();
			// 			if (current.column() > 0) {
			// 				const QRect& rect = m_tableView->visualRect(current);
			// 				auto* scrollBar = m_tableView->horizontalScrollBar();
			// 				if (rect.topLeft().x() < m_frozenTableView->columnWidth(0)) {
			// 					const int newValue = scrollBar->value() + rect.topLeft().x() - m_frozenTableView->columnWidth(0);
			// 					scrollBar->setValue(newValue);
			// 				}
			// 			}
		} else if (key_event->matches(QKeySequence::Find))
			showSearchReplace(/* replace */ false);
		else if (key_event->matches(QKeySequence::Replace))
			showSearchReplace(/* replace */ true);
#ifndef SDK
		else if (key_event->key() == Qt::Key_Escape && m_searchReplaceWidget && m_searchReplaceWidget->isVisible())
			m_searchReplaceWidget->hide();
#endif
		else if (key_event->matches(QKeySequence::Cut))
			cutSelection();
		else if (key_event->matches(QKeySequence::SelectAll)) // TODO: doesn't work
			selectAll();
	}

	return QWidget::eventFilter(watched, event);
}

/*!
	Advance current cell after [Return] or [Enter] was pressed
*/
void SpreadsheetView::advanceCell() {
	const QModelIndex& idx = m_tableView->currentIndex();
	const int row = idx.row();
	const int col = idx.column();
	if (row + 1 == m_spreadsheet->rowCount())
		m_spreadsheet->setRowCount(m_spreadsheet->rowCount() + 1);

	m_tableView->setCurrentIndex(idx.sibling(row + 1, col));
}

/*!
 * disables cell data relevant actions in the spreadsheet menu if there're no cells available
 */
void SpreadsheetView::checkSpreadsheetMenu() {
	if (!m_plotDataMenu)
		initMenus();

	const auto& columns = m_spreadsheet->children<Column>();

	const bool cellsAvail = m_spreadsheet->columnCount() > 0 && m_spreadsheet->rowCount() > 0;
	bool hasValues = this->hasValues(columns);
	m_plotDataMenu->setEnabled(hasValues);
	m_selectionMenu->setEnabled(hasValues);
	action_select_all->setEnabled(hasValues);
	action_clear_spreadsheet->setEnabled(hasValues);
	action_statistics_all_columns->setEnabled(hasValues);
	action_go_to_cell->setEnabled(cellsAvail);

	// deactivate the "Clear masks" action if there are no masked cells
	bool hasMasked = false;
	for (auto* column : columns) {
		if (column->maskedIntervals().size() > 0) {
			hasMasked = true;
			break;
		}
	}

	action_clear_masks->setVisible(hasMasked);

	// deactivate the "Remove format" action if no columns are formatted
	bool hasFormat = false;
	for (auto* column : columns) {
		if (column->hasHeatmapFormat()) {
			hasFormat = true;
			break;
		}
	}

	action_formatting_remove->setVisible(hasFormat);

	bool hasStatisticsSpreadsheet = (m_spreadsheet->children<StatisticsSpreadsheet>().size() == 1);
	action_statistics_spreadsheet->setChecked(hasStatisticsSpreadsheet);

	if (areCommentsShown())
		action_toggle_comments->setText(i18n("Hide Comments"));
	else
		action_toggle_comments->setText(i18n("Show Comments"));

	if (areSparkLinesShown())
		action_toggle_sparklines->setText(i18n("Hide Sparklines"));
	else
		action_toggle_sparklines->setText(i18n("Show Sparklines"));
}

void SpreadsheetView::checkSpreadsheetSelectionMenu() {
	// deactivate mask/unmask actions for the selection
	// if there are no unmasked/masked cells in the current selection
	const QModelIndexList& indexes = m_tableView->selectionModel()->selectedIndexes();
	bool hasMasked = false;
	bool hasUnmasked = false;
	for (auto& index : std::as_const(indexes)) {
		int row = index.row();
		int col = index.column();
		const auto* column = m_spreadsheet->column(col);
		// TODO: the null pointer check shouldn't be actually required here
		// but when deleting the columns the selection model in the view
		// and the aspect model sometimes get out of sync and we crash...
		if (!column)
			break;

		if (!hasMasked && column->isMasked(row))
			hasMasked = true;

		if (!hasUnmasked && !column->isMasked(row))
			hasUnmasked = true;

		if (hasMasked && hasUnmasked)
			break;
	}

	action_mask_selection->setEnabled(hasUnmasked);
	action_unmask_selection->setEnabled(hasMasked);
}

void SpreadsheetView::checkColumnMenus(const QVector<Column*>& columns) {
	bool numeric = true;
	bool plottable = true;
	bool datetime = false;
	bool text = false;
	bool hasValues = false;
	bool hasFormat = false;
	bool hasEnoughValues = false; // enough for statistics (> 1)

	for (const auto* col : columns) {
		if (!col->isNumeric()) {
			datetime = (col->columnMode() == AbstractColumn::ColumnMode::DateTime);
			if (!datetime)
				plottable = false;

			numeric = false;
			break;
		}
	}

	for (const auto* col : columns) {
		if (col->columnMode() == AbstractColumn::ColumnMode::Text) {
			text = true;
			break;
		}
	}

	for (const auto* col : columns) {
		if (col->hasValues()) {
			hasValues = true;
			break;
		}
	}

	for (const auto* col : columns) {
		if (col->availableRowCount() > 1) {
			hasEnoughValues = true;
			break;
		}
	}

	for (const auto* col : columns) {
		if (col->hasHeatmapFormat()) {
			hasFormat = true;
			break;
		}
	}

	if (isColumnSelected(0, true)) {
		action_freeze_columns->setVisible(true);
		if (m_frozenTableView) {
			if (!m_frozenTableView->isVisible()) {
				action_freeze_columns->setText(i18n("Freeze Column"));
				action_insert_column_left->setEnabled(true);
				action_insert_columns_left->setEnabled(true);
			} else {
				action_freeze_columns->setText(i18n("Unfreeze Column"));
				action_insert_column_left->setEnabled(false);
				action_insert_columns_left->setEnabled(false);
			}
		}
	} else
		action_freeze_columns->setVisible(false);

	m_plotDataMenu->setEnabled(plottable && hasValues);
	m_analyzePlotMenu->setEnabled(numeric && hasValues);
	m_columnSetAsMenu->setEnabled(numeric);
	action_statistics_columns->setEnabled(hasEnoughValues);
	action_clear_columns->setEnabled(hasValues);
	m_selectionMenu->setEnabled(hasValues);
	m_formattingMenu->setEnabled(hasValues);
	action_formatting_remove->setVisible(hasFormat);

	if (m_readOnly)
		return;

	// generate data is only possible for numeric columns and if there are cells available
	const bool hasCells = m_spreadsheet->rowCount() > 0;
	m_columnGenerateDataMenu->setEnabled(hasCells);
	action_fill_row_numbers->setEnabled(numeric);
	action_fill_const->setEnabled(numeric);
	action_fill_equidistant->setEnabled(numeric || datetime);
	action_fill_random_nonuniform->setEnabled(numeric);
	action_fill_function->setEnabled(numeric);
	action_sample_values->setEnabled(hasValues);
	action_flatten_columns->setEnabled(hasValues);

	// manipulate data is only possible for numeric and datetime and if there values.
	// datetime has only "add/subtract value", everything else is deactivated
	m_columnManipulateDataMenu->setEnabled((numeric || datetime || text) && hasValues);
	action_add_value->setEnabled(numeric || datetime);
	action_subtract_value->setEnabled(numeric || datetime);
	action_subtract_baseline->setEnabled(numeric);
	action_multiply_value->setEnabled(numeric);
	action_divide_value->setEnabled(numeric);
	action_reverse_columns->setEnabled(numeric);
	action_drop_values->setEnabled(numeric || text || datetime);
	action_mask_values->setEnabled(numeric || text || datetime);
	m_columnNormalizeMenu->setEnabled(numeric);
	m_columnLadderOfPowersMenu->setEnabled(numeric);
}

bool SpreadsheetView::formulaModeActive() const {
	return m_model->formulaModeActive();
}

void SpreadsheetView::activateFormulaMode(bool on) {
	m_model->activateFormulaMode(on);
}

void SpreadsheetView::goToNextColumn() {
	if (m_spreadsheet->columnCount() == 0)
		return;

	QModelIndex idx = m_tableView->currentIndex();
	int col = idx.column() + 1;
	if (col >= m_spreadsheet->columnCount())
		col = 0;

	m_tableView->setCurrentIndex(idx.sibling(idx.row(), col));
}

void SpreadsheetView::goToPreviousColumn() {
	if (m_spreadsheet->columnCount() == 0)
		return;

	QModelIndex idx = m_tableView->currentIndex();
	int col = idx.column() - 1;
	if (col < 0)
		col = m_spreadsheet->columnCount() - 1;

	m_tableView->setCurrentIndex(idx.sibling(idx.row(), col));
}

void SpreadsheetView::cutSelection() {
	if (firstSelectedRow() < 0)
		return;

	WAIT_CURSOR;
	m_spreadsheet->beginMacro(i18n("%1: cut selected cells", m_spreadsheet->name()));
	copySelection();
	clearSelectedCells();
	m_spreadsheet->endMacro();
	RESET_CURSOR;
}

void SpreadsheetView::copySelection() {
	PERFTRACE(QStringLiteral("copy selected cells"));
	const int first_col = firstSelectedColumn();
	if (first_col == -1)
		return;
	const int last_col = lastSelectedColumn();
	if (last_col == -2)
		return;
	const int first_row = firstSelectedRow();
	if (first_row == -1)
		return;
	const int last_row = lastSelectedRow();
	if (last_row == -2)
		return;
	const int cols = last_col - first_col + 1;
	const int rows = last_row - first_row + 1;

	WAIT_CURSOR;
	QString output_str;

	QVector<Column*> columns;
	QVector<char> formats;
	for (int c = 0; c < cols; c++) {
		Column* col = m_spreadsheet->column(first_col + c);
		columns << col;
		const auto* outFilter = static_cast<Double2StringFilter*>(col->outputFilter());
		formats << outFilter->numericFormat();
	}

	const auto numberLocale = QLocale();
	for (int r = 0; r < rows; r++) {
		for (int c = 0; c < cols; c++) {
			const Column* col_ptr = columns.at(c);
			if (isCellSelected(first_row + r, first_col + c)) {
				// 				if (formulaModeActive())
				// 					output_str += col_ptr->formula(first_row + r);
				// 				else
				if (col_ptr->columnMode() == AbstractColumn::ColumnMode::Double)
					output_str += numberLocale.toString(col_ptr->valueAt(first_row + r), formats.at(c), 16); // copy with max. precision
				else if (col_ptr->columnMode() == AbstractColumn::ColumnMode::Integer || col_ptr->columnMode() == AbstractColumn::ColumnMode::BigInt)
					output_str += numberLocale.toString(col_ptr->valueAt(first_row + r));
				else
					output_str += col_ptr->asStringColumn()->textAt(first_row + r);
			}
			if (c < cols - 1)
				output_str += QLatin1Char('\t');
		}
		if (r < rows - 1)
			output_str += QLatin1Char('\n');
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
	if (!mime_data->hasFormat(QStringLiteral("text/plain")))
		return;

	PERFTRACE(QStringLiteral("paste selected cells"));
	WAIT_CURSOR;
	m_spreadsheet->beginMacro(i18n("%1: paste from clipboard", m_spreadsheet->name()));

	int first_col = firstSelectedColumn();
	int last_col = lastSelectedColumn();
	int first_row = firstSelectedRow();
	int last_row = lastSelectedRow();
	int input_row_count = 0;
	int input_col_count = 0;
	QVector<QStringList> cellTexts;

	try {
		QString input_str = QString::fromUtf8(mime_data->data(QStringLiteral("text/plain"))).trimmed();
		QString separator;
		if (input_str.indexOf(QLatin1String("\r\n")) != -1)
			separator = QLatin1String("\r\n");
		else
			separator = QLatin1Char('\n');

		QStringList input_rows(input_str.split(separator));
		input_str.clear(); // not needed anymore, release memory
		input_row_count = input_rows.count();
		input_col_count = 0;
		bool hasTabs = false;
		if (input_row_count > 0 && input_rows.constFirst().indexOf(QLatin1Char('\t')) != -1)
			hasTabs = true;

		const auto numberLocale = QLocale();
		// TEST ' ' as group separator:
		// numberLocale = QLocale(QLocale::French, QLocale::France);
		const KConfigGroup group = Settings::group(QStringLiteral("Settings_General"));
		for (int i = 0; i < input_row_count; i++) {
			if (hasTabs)
				cellTexts.append(input_rows.at(i).split(QLatin1Char('\t')));
			else if (numberLocale.groupSeparator().trimmed().isEmpty()
					 && !(numberLocale.numberOptions() & QLocale::OmitGroupSeparator)) // locale with ' ' as group separator && omit group separator not set
				cellTexts.append(input_rows.at(i).split(QRegularExpression(QStringLiteral("\\s\\s")), (Qt::SplitBehavior)0x1)); // split with two spaces
			else
				cellTexts.append(input_rows.at(i).split(QRegularExpression(QStringLiteral("\\s+"))));

			if (cellTexts.at(i).count() > input_col_count)
				input_col_count = cellTexts.at(i).count();
		}

		input_rows.clear(); // not needed anymore, release memory

		// when pasting DateTime data in the format 'yyyy-MM-dd hh:mm:ss' and similar, it get's split above because of the space separator.
		// here we check whether we have such a situation and merge the first two columns to get the proper value,
		// for example "2018-03-21 10:00:00" and not "2018-03-21" and "10:00:00"
		if (!cellTexts.isEmpty() && cellTexts.constFirst().size() > 1) {
			const auto& firstCell = cellTexts.constFirst().at(0);
			const auto& secondCell = cellTexts.constFirst().at(1);
			QString dateTimeFormat; // empty string, we'll auto-detect the format of the data
			const auto firstMode = AbstractFileFilter::columnMode(firstCell, dateTimeFormat, numberLocale);
			dateTimeFormat.clear();
			const auto secondMode = AbstractFileFilter::columnMode(secondCell, dateTimeFormat, numberLocale);

			// if both first columns are DateTime, check whether the combination of them is also DateTime
			if (firstMode == AbstractColumn::ColumnMode::DateTime && secondMode == AbstractColumn::ColumnMode::DateTime) {
				dateTimeFormat.clear();
				const QString newCell = firstCell + QLatin1Char(' ') + secondCell;
				const auto newMode = AbstractFileFilter::columnMode(newCell, dateTimeFormat, numberLocale);
				if (newMode == AbstractColumn::ColumnMode::DateTime) {
					// merge the first two colums
					for (auto& row : cellTexts) {
						row[1] = row.at(0) + QLatin1Char(' ') + row.at(1);
						row.takeFirst();
					}
					--input_col_count;
				}
			}
		}

		// 	bool localeDetermined = false;

		// expand the current selection to the needed size if
		// 1. there is no selection
		// 2. only one cell selected
		// 3. the whole column is selected (the use clicked on the header)
		// Also, set the proper column mode if the target column doesn't have any values yet
		// and set the proper column mode if the column is empty
		if ((first_col == -1 || first_row == -1) || (last_row == first_row && last_col == first_col)
			|| (first_row == 0 && last_row == m_spreadsheet->rowCount() - 1)) {
			int current_row, current_col;
			getCurrentCell(&current_row, &current_col);
			if (current_row == -1)
				current_row = 0;
			if (current_col == -1)
				current_col = 0;
			setCellSelected(current_row, current_col);
			first_col = current_col;
			first_row = current_row;
			last_row = first_row + input_row_count - 1;
			last_col = first_col + input_col_count - 1;
			const int columnCount = m_spreadsheet->columnCount();
			// if the target columns that are already available don't have any values yet,
			// convert their mode to the mode of the data to be pasted
			for (int c = first_col; c <= last_col && c < columnCount; ++c) {
				Column* col = m_spreadsheet->column(c);
				if (col->hasValues())
					continue;

				// first non-empty value in the column to paste determines the column mode/type of the new column to be added
				const int curCol = c - first_col;
				QString nonEmptyValue;
				for (auto& r : cellTexts) {
					if (curCol < r.count() && !r.at(curCol).isEmpty()) {
						nonEmptyValue = r.at(curCol);
						break;
					}
				}

				// 			if (!localeDetermined)
				// 				localeDetermined = determineLocale(nonEmptyValue, locale);

				QString dateTimeFormat; // empty string, we'll auto-detect the format of the data
				const auto mode = AbstractFileFilter::columnMode(nonEmptyValue, dateTimeFormat, numberLocale);
				col->setColumnMode(mode);
				if (mode == AbstractColumn::ColumnMode::DateTime) {
					auto* filter = static_cast<DateTime2StringFilter*>(col->outputFilter());
					filter->setFormat(dateTimeFormat);
				}
			}

			// add columns if necessary
			if (last_col >= columnCount) {
				for (int c = 0; c < last_col - (columnCount - 1); ++c) {
					const int curCol = columnCount - first_col + c;
					// first non-empty value in the column to paste determines the column mode/type of the new column to be added
					QString nonEmptyValue;
					for (auto& r : cellTexts) {
						if (curCol < r.count() && !r.at(curCol).isEmpty()) {
							nonEmptyValue = r.at(curCol);
							break;
						}
					}

					// 				if (!localeDetermined)
					// 					localeDetermined = determineLocale(nonEmptyValue, locale);

					QString dateTimeFormat; // empty string, we'll auto-detect the format of the data
					const auto mode = AbstractFileFilter::columnMode(nonEmptyValue, dateTimeFormat, numberLocale);
					Column* new_col = new Column(QString::number(curCol), mode);
					if (mode == AbstractColumn::ColumnMode::DateTime) {
						auto* filter = static_cast<DateTime2StringFilter*>(new_col->outputFilter());
						filter->setFormat(dateTimeFormat);
					}
					new_col->setPlotDesignation(AbstractColumn::PlotDesignation::Y);
					new_col->insertRows(0, m_spreadsheet->rowCount());
					m_spreadsheet->addChild(new_col);
				}
			}

			// add rows if necessary
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
			if (col->columnMode() == AbstractColumn::ColumnMode::Double) {
				if (rows == m_spreadsheet->rowCount() && rows <= cellTexts.size()) {
					QVector<double> new_data(rows);
					for (int r = 0; r < rows; ++r) {
						if (c < cellTexts.at(r).count())
							new_data[r] = numberLocale.toDouble(cellTexts.at(r).at(c));
					}
					col->setValues(new_data);
				} else {
					for (int r = 0; r < rows && r < input_row_count; r++) {
						if (isCellSelected(first_row + r, first_col + c) && (c < cellTexts.at(r).count())) {
							if (!cellTexts.at(r).at(c).isEmpty())
								col->setValueAt(first_row + r, numberLocale.toDouble(cellTexts.at(r).at(c)));
							else
								col->setValueAt(first_row + r, std::numeric_limits<double>::quiet_NaN());
						}
					}
				}
			} else if (col->columnMode() == AbstractColumn::ColumnMode::Integer) {
				if (rows == m_spreadsheet->rowCount() && rows <= cellTexts.size()) {
					QVector<int> new_data(rows);
					for (int r = 0; r < rows; ++r) {
						if (c < cellTexts.at(r).count())
							new_data[r] = numberLocale.toInt(cellTexts.at(r).at(c));
					}
					col->setIntegers(new_data);
				} else {
					for (int r = 0; r < rows && r < input_row_count; r++) {
						if (isCellSelected(first_row + r, first_col + c) && (c < cellTexts.at(r).count())) {
							if (!cellTexts.at(r).at(c).isEmpty())
								col->setIntegerAt(first_row + r, numberLocale.toInt(cellTexts.at(r).at(c)));
							else
								col->setIntegerAt(first_row + r, 0);
						}
					}
				}
			} else if (col->columnMode() == AbstractColumn::ColumnMode::BigInt) {
				if (rows == m_spreadsheet->rowCount() && rows <= cellTexts.size()) {
					QVector<qint64> new_data(rows);
					for (int r = 0; r < rows; ++r)
						new_data[r] = numberLocale.toLongLong(cellTexts.at(r).at(c));
					col->setBigInts(new_data);
				} else {
					for (int r = 0; r < rows && r < input_row_count; r++) {
						if (isCellSelected(first_row + r, first_col + c) && (c < cellTexts.at(r).count())) {
							if (!cellTexts.at(r).at(c).isEmpty())
								col->setBigIntAt(first_row + r, numberLocale.toLongLong(cellTexts.at(r).at(c)));
							else
								col->setBigIntAt(first_row + r, 0);
						}
					}
				}
			} else {
				for (int r = 0; r < rows && r < input_row_count; r++) {
					if (isCellSelected(first_row + r, first_col + c) && (c < cellTexts.at(r).count())) {
						// 					if (formulaModeActive())
						// 						col->setFormula(first_row + r, cellTexts.at(r).at(c));
						// 					else
						col->asStringColumn()->setTextAt(first_row + r, cellTexts.at(r).at(c));
					}
				}
			}

			col->setSuppressDataChangedSignal(false);
			col->setChanged();
		} // end of for-loop
	} catch (std::bad_alloc&) {
		cellTexts.clear();
		m_spreadsheet->endMacro();
		RESET_CURSOR;
		KMessageBox::error(this, i18n("Not enough memory to finalize this operation."));
	}

	m_spreadsheet->endMacro();
	RESET_CURSOR;
}

void SpreadsheetView::maskSelection() {
	int first = firstSelectedRow();
	if (first < 0)
		return;
	int last = lastSelectedRow();

	WAIT_CURSOR;
	m_spreadsheet->beginMacro(i18n("%1: mask selected cells", m_spreadsheet->name()));

	const auto& columns = selectedColumns(false);
	QVector<CartesianPlot*> plots;

	// determine the dependent plots
	for (auto* column : columns)
		column->addUsedInPlots(plots);

	// suppress retransform in the dependent plots
	for (auto* plot : plots)
		plot->setSuppressRetransform(true);

	// mask the selected cells
	for (auto* column : columns) {
		int col = m_spreadsheet->indexOfChild<Column>(column);
		for (int row = first; row <= last; row++)
			if (isCellSelected(row, col))
				column->setMasked(row);
	}

	// retransform the dependent plots
	for (auto* plot : plots) {
		plot->setSuppressRetransform(false);
		// TODO: check which ranges must be updated
		plot->dataChanged(-1, -1);
	}

	// some cells were masked, enable the "clear masks" action
	action_clear_masks->setEnabled(true);

	m_spreadsheet->endMacro();
	RESET_CURSOR;
}

void SpreadsheetView::unmaskSelection() {
	int first = firstSelectedRow();
	if (first < 0)
		return;
	int last = lastSelectedRow();

	WAIT_CURSOR;
	m_spreadsheet->beginMacro(i18n("%1: unmask selected cells", m_spreadsheet->name()));

	const auto& columns = selectedColumns(false);
	QVector<CartesianPlot*> plots;

	// determine the dependent plots
	for (auto* column : columns)
		column->addUsedInPlots(plots);

	// suppress retransform in the dependent plots
	for (auto* plot : plots)
		plot->setSuppressRetransform(true);

	// unmask the selected cells
	for (auto* column : columns) {
		int col = m_spreadsheet->indexOfChild<Column>(column);
		for (int row = first; row <= last; row++)
			if (isCellSelected(row, col))
				column->setMasked(row, false);
	}

	// retransform the dependent plots
	for (auto* plot : plots) {
		plot->setSuppressRetransform(false);
		// TODO: check which ranges must be updated
		plot->dataChanged(-1, -1);
	}

	m_spreadsheet->endMacro();
	RESET_CURSOR;
}

void SpreadsheetView::reverseSelection() {
	auto* selectionModel = m_tableView->selectionModel();
	const auto& indexes = selectionModel->selectedIndexes();
	if (indexes.isEmpty())
		return;

	// check if complete rows only are selected, reverse the selection for rows only in this case
	bool rowsOnly = true;
	QVector<int> selectedRows;
	for (const auto& index : indexes) {
		const int row = index.row();
		if (selectionModel->isRowSelected(row)) {
			if (selectedRows.indexOf(row) == -1)
				selectedRows << row;
		} else {
			rowsOnly = false;
			break;
		}
	}

	if (rowsOnly) {
		// clear the current selection
		m_suppressSelectionChangedEvent = true;
		selectionModel->clearSelection();

		// temporarily switch to the "multi selection" mode and select the rows, that were not selected before
		m_tableView->setSelectionMode(QAbstractItemView::MultiSelection);
		for (int row = 0; row < m_spreadsheet->rowCount(); ++row) {
			if (selectedRows.indexOf(row) == -1)
				m_tableView->selectRow(row);
		}
		m_tableView->setSelectionMode(QAbstractItemView::ExtendedSelection); // switch back to the extended selection

		m_suppressSelectionChangedEvent = false;
		selectionChanged(QItemSelection(), QItemSelection());
		return;
	}

	// check if complete columns are selected only, reverse the selection for columns only in this case
	bool columnsOnly = true;
	QVector<int> selectedColumns;
	for (const auto& index : indexes) {
		const int col = index.column();
		if (selectionModel->isColumnSelected(col)) {
			if (selectedColumns.indexOf(col) == -1)
				selectedColumns << col;
		} else {
			columnsOnly = false;
			break;
		}
	}

	if (columnsOnly) {
		// clear the current selection
		m_suppressSelectionChangedEvent = true;
		selectionModel->clearSelection();

		// temporarily switch to the "multi selection" mode and select the columns, that were not selected before
		m_tableView->setSelectionMode(QAbstractItemView::MultiSelection);
		for (int col = 0; col < m_spreadsheet->columnCount(); ++col) {
			if (selectedColumns.indexOf(col) == -1)
				m_tableView->selectColumn(col);
		}
		m_tableView->setSelectionMode(QAbstractItemView::ExtendedSelection); // switch back to the extended selection

		m_suppressSelectionChangedEvent = false;
		selectionChanged(QItemSelection(), QItemSelection());
		return;
	}

	// multiple cells are selected, reverse the selection for all cells in the spreadsheet.
	// select all cells first, after this deselect the cells that were selected before.
	m_suppressSelectionChangedEvent = true;
	selectAll();

	m_tableView->setSelectionMode(QAbstractItemView::SingleSelection); // temporarily switch to single selection
	for (const auto& index : indexes)
		selectionModel->select(index, QItemSelectionModel::Deselect);
	m_tableView->setSelectionMode(QAbstractItemView::ExtendedSelection); // switch back to the extended selection

	m_suppressSelectionChangedEvent = false;
	selectionChanged(QItemSelection(), QItemSelection());
}

void SpreadsheetView::plotData(QAction* action) {
	auto type = static_cast<PlotDataDialog::PlotType>(action->data().toInt());
	auto* dlg = new PlotDataDialog(m_spreadsheet, type);

	// use all spreadsheet columns if no columns are selected
	auto columns = selectedColumns(true);
	if (columns.isEmpty())
		columns = m_spreadsheet->children<Column>();

	dlg->setSelectedColumns(columns);
	dlg->exec();
}

void SpreadsheetView::plotAnalysisData(QAction* action) {
	auto* dlg = new PlotDataDialog(m_spreadsheet, PlotDataDialog::PlotType::XYCurve);
	const auto analysisAction = static_cast<XYAnalysisCurve::AnalysisAction>(action->data().toInt());
	dlg->setAnalysisAction(analysisAction);

	// use all spreadsheet columns if no columns are selected
	auto columns = selectedColumns(true);
	if (columns.isEmpty())
		columns = m_spreadsheet->children<Column>();
	dlg->setSelectedColumns(columns);

	dlg->exec();
}

void SpreadsheetView::plotDataDistributionFit(QAction* action) {
	auto* dlg = new PlotDataDialog(m_spreadsheet, PlotDataDialog::PlotType::DistributionFit);
	const auto distribution = static_cast<nsl_sf_stats_distribution>(action->data().toInt());
	dlg->setFitDistribution(distribution);

	// use all spreadsheet columns if no columns are selected
	auto columns = selectedColumns(true);
	if (columns.isEmpty())
		columns = m_spreadsheet->children<Column>();
	dlg->setSelectedColumns(columns);

	dlg->exec();
}

void SpreadsheetView::fillSelectedCellsWithRowNumbers() {
	const auto& columns = selectedColumns(false);
	if (columns.isEmpty())
		return;

	int first = firstSelectedRow();
	if (first < 0)
		return;
	int last = lastSelectedRow();

	WAIT_CURSOR;
	m_spreadsheet->beginMacro(i18n("%1: fill cells with row numbers", m_spreadsheet->name()));
	for (auto* col_ptr : columns) {
		int col = m_spreadsheet->indexOfChild<Column>(col_ptr);
		col_ptr->setSuppressDataChangedSignal(true);
		switch (col_ptr->columnMode()) {
		case AbstractColumn::ColumnMode::Double: {
			QVector<double> results(last - first + 1);
			for (int row = first; row <= last; row++)
				if (isCellSelected(row, col))
					results[row - first] = row + 1;
				else
					results[row - first] = col_ptr->valueAt(row);
			col_ptr->replaceValues(first, results);
			break;
		}
		case AbstractColumn::ColumnMode::Integer: {
			QVector<int> results(last - first + 1);
			for (int row = first; row <= last; row++)
				if (isCellSelected(row, col))
					results[row - first] = row + 1;
				else
					results[row - first] = col_ptr->integerAt(row);
			col_ptr->replaceInteger(first, results);
			break;
		}
		case AbstractColumn::ColumnMode::BigInt: {
			QVector<qint64> results(last - first + 1);
			for (int row = first; row <= last; row++)
				if (isCellSelected(row, col))
					results[row - first] = row + 1;
				else
					results[row - first] = col_ptr->bigIntAt(row);
			col_ptr->replaceBigInt(first, results);
			break;
		}
		case AbstractColumn::ColumnMode::Text: {
			QVector<QString> results;
			for (int row = first; row <= last; row++)
				if (isCellSelected(row, col))
					results << QString::number(row + 1);
				else
					results << col_ptr->textAt(row);
			col_ptr->replaceTexts(first, results);
			break;
		}
		// TODO: handle other modes
		case AbstractColumn::ColumnMode::DateTime:
		case AbstractColumn::ColumnMode::Month:
		case AbstractColumn::ColumnMode::Day:
			break;
		}

		col_ptr->setSuppressDataChangedSignal(false);
		col_ptr->setChanged();
	}
	m_spreadsheet->endMacro();
	RESET_CURSOR;
}

void SpreadsheetView::fillWithRowNumbers() {
	const auto& columns = selectedColumns();
	if (columns.isEmpty())
		return;

	WAIT_CURSOR;
	m_spreadsheet->beginMacro(i18np("%1: fill column with row numbers", "%1: fill columns with row numbers", m_spreadsheet->name(), columns.count()));

	const int rows = m_spreadsheet->rowCount();

	QVector<int> int_data(rows);
	for (int i = 0; i < rows; ++i)
		int_data[i] = i + 1;

	for (auto* col : columns) {
		col->clearFormula(); // clear the potentially available column formula

		switch (col->columnMode()) {
		case AbstractColumn::ColumnMode::Integer:
			col->replaceInteger(0, int_data);
			break;
		case AbstractColumn::ColumnMode::Double:
		case AbstractColumn::ColumnMode::BigInt:
			col->setColumnMode(AbstractColumn::ColumnMode::Integer);
			col->replaceInteger(0, int_data);
			break;
		case AbstractColumn::ColumnMode::Text:
		case AbstractColumn::ColumnMode::DateTime:
		case AbstractColumn::ColumnMode::Day:
		case AbstractColumn::ColumnMode::Month:
			break;
		}
	}

	m_spreadsheet->endMacro();
	RESET_CURSOR;
}

// TODO: this function is not used currently.
void SpreadsheetView::fillSelectedCellsWithRandomNumbers() {
	const auto& columns = selectedColumns(false);
	if (columns.isEmpty())
		return;

	int first = firstSelectedRow();
	int last = lastSelectedRow();
	if (first < 0)
		return;

	WAIT_CURSOR;
	m_spreadsheet->beginMacro(i18n("%1: fill cells with random values", m_spreadsheet->name()));
	QRandomGenerator rng(QTime::currentTime().msec());
	for (auto* col_ptr : columns) {
		int col = m_spreadsheet->indexOfChild<Column>(col_ptr);
		col_ptr->setSuppressDataChangedSignal(true);
		switch (col_ptr->columnMode()) {
		case AbstractColumn::ColumnMode::Double: {
			QVector<double> results(last - first + 1);
			for (int row = first; row <= last; row++)
				if (isCellSelected(row, col))
					results[row - first] = rng.generateDouble();
				else
					results[row - first] = col_ptr->valueAt(row);
			col_ptr->replaceValues(first, results);
			break;
		}
		case AbstractColumn::ColumnMode::Integer: {
			QVector<int> results(last - first + 1);
			for (int row = first; row <= last; row++)
				if (isCellSelected(row, col))
					results[row - first] = QRandomGenerator::global()->generate();
				else
					results[row - first] = col_ptr->integerAt(row);
			col_ptr->replaceInteger(first, results);
			break;
		}
		case AbstractColumn::ColumnMode::BigInt: {
			QVector<qint64> results(last - first + 1);
			for (int row = first; row <= last; row++)
				if (isCellSelected(row, col))
					results[row - first] = QRandomGenerator::global()->generate64();
				else
					results[row - first] = col_ptr->bigIntAt(row);
			col_ptr->replaceBigInt(first, results);
			break;
		}
		case AbstractColumn::ColumnMode::Text: {
			QVector<QString> results;
			for (int row = first; row <= last; row++)
				if (isCellSelected(row, col))
					results[row - first] = QString::number(QRandomGenerator::global()->generateDouble());
				else
					results << col_ptr->textAt(row);
			col_ptr->replaceTexts(first, results);
			break;
		}
		case AbstractColumn::ColumnMode::DateTime:
		case AbstractColumn::ColumnMode::Month:
		case AbstractColumn::ColumnMode::Day: {
			QVector<QDateTime> results;
			QDate earliestDate(1, 1, 1);
			QDate latestDate(2999, 12, 31);
			QTime midnight(0, 0, 0, 0);
			for (int row = first; row <= last; row++)
				if (isCellSelected(row, col))
					results << QDateTime(earliestDate.addDays((QRandomGenerator::global()->generateDouble()) * ((double)earliestDate.daysTo(latestDate))),
										 midnight.addMSecs((QRandomGenerator::global()->generateDouble()) * 1000 * 60 * 60 * 24));
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
#ifndef SDK
	const auto& columns = selectedColumns();
	if (columns.isEmpty())
		return;

	auto* dlg = new RandomValuesDialog(m_spreadsheet);
	dlg->setColumns(columns);
	dlg->exec();
#endif
}

void SpreadsheetView::fillWithEquidistantValues() {
#ifndef SDK
	const auto& columns = selectedColumns();
	if (columns.isEmpty())
		return;

	auto* dlg = new EquidistantValuesDialog(m_spreadsheet);
	dlg->setColumns(columns);
	dlg->exec();
#endif
}

void SpreadsheetView::fillWithFunctionValues() {
#ifndef SDK
	const auto& columns = selectedColumns();
	if (columns.isEmpty())
		return;

	auto* dlg = new FunctionValuesDialog(m_spreadsheet);
	dlg->setColumns(columns);
	dlg->exec();
#endif
}

void SpreadsheetView::fillSelectedCellsWithConstValues() {
	const auto& columns = selectedColumns(false);
	if (columns.isEmpty())
		return;

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
	for (auto* col_ptr : columns) {
		int col = m_spreadsheet->indexOfChild<Column>(col_ptr);
		col_ptr->setSuppressDataChangedSignal(true);
		col_ptr->clearFormula(); // clear the potentially available column formula

		switch (col_ptr->columnMode()) {
		case AbstractColumn::ColumnMode::Double:
			if (!doubleOk)
				doubleValue = QInputDialog::getDouble(this,
													  i18n("Fill the selection with constant value"),
													  i18n("Value"),
													  0,
													  -std::numeric_limits<double>::max(),
													  std::numeric_limits<double>::max(),
													  6,
													  &doubleOk);
			if (doubleOk) {
				WAIT_CURSOR;
				QVector<double> results(last - first + 1);
				for (int row = first; row <= last; row++) {
					if (isCellSelected(row, col))
						results[row - first] = doubleValue;
					else
						results[row - first] = col_ptr->valueAt(row);
				}
				col_ptr->replaceValues(first, results);
				RESET_CURSOR;
			}
			break;
		case AbstractColumn::ColumnMode::Integer:
			if (!intOk)
				intValue = QInputDialog::getInt(this, i18n("Fill the selection with constant value"), i18n("Value"), 0, -2147483647, 2147483647, 1, &intOk);
			if (intOk) {
				WAIT_CURSOR;
				QVector<int> results(last - first + 1);
				for (int row = first; row <= last; row++) {
					if (isCellSelected(row, col))
						results[row - first] = intValue;
					else
						results[row - first] = col_ptr->integerAt(row);
				}
				col_ptr->replaceInteger(first, results);
				RESET_CURSOR;
			}
			break;
		case AbstractColumn::ColumnMode::BigInt:
			// TODO: getBigInt()
			if (!bigIntOk)
				bigIntValue =
					QInputDialog::getInt(this, i18n("Fill the selection with constant value"), i18n("Value"), 0, -2147483647, 2147483647, 1, &bigIntOk);
			if (bigIntOk) {
				WAIT_CURSOR;
				QVector<qint64> results(last - first + 1);
				for (int row = first; row <= last; row++) {
					if (isCellSelected(row, col))
						results[row - first] = bigIntValue;
					else
						results[row - first] = col_ptr->bigIntAt(row);
				}
				col_ptr->replaceBigInt(first, results);
				RESET_CURSOR;
			}
			break;
		case AbstractColumn::ColumnMode::Text:
			if (!stringOk)
				stringValue =
					QInputDialog::getText(this, i18n("Fill the selection with constant value"), i18n("Value"), QLineEdit::Normal, QString(), &stringOk);
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
		// TODO: handle other modes
		case AbstractColumn::ColumnMode::DateTime:
		case AbstractColumn::ColumnMode::Month:
		case AbstractColumn::ColumnMode::Day:
			break;
		}

		col_ptr->setSuppressDataChangedSignal(false);
		col_ptr->setChanged();
	}
	m_spreadsheet->endMacro();
}

void SpreadsheetView::formatHeatmap() {
#ifndef SDK
	auto columns = selectedColumns();
	if (columns.isEmpty())
		columns = m_spreadsheet->children<Column>();

	if (columns.isEmpty())
		return;

	auto* dlg = new FormattingHeatmapDialog(m_spreadsheet);
	dlg->setColumns(columns);
	if (dlg->exec() == QDialog::Accepted) {
		const auto& format = dlg->format();
		int count = columns.count();
		if (count > 1)
			m_spreadsheet->beginMacro(i18n("%1: set heatmap format", m_spreadsheet->name()));
		for (auto* col : columns)
			col->setHeatmapFormat(format);
		if (count > 1)
			m_spreadsheet->endMacro();
	}

	delete dlg;
#endif
}

void SpreadsheetView::removeFormat() {
	auto columns = selectedColumns();
	if (columns.isEmpty())
		columns = m_spreadsheet->children<Column>();

	int count = columns.count();
	if (count > 1)
		m_spreadsheet->beginMacro(i18n("%1: remove heatmap format", m_spreadsheet->name()));
	for (auto* col : columns)
		col->removeFormat();
	if (count > 1)
		m_spreadsheet->endMacro();
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
	int count = QInputDialog::getInt(nullptr,
									 i18n("Insert empty columns"),
									 i18n("Enter the number of columns to insert"),
									 1 /*value*/,
									 1 /*min*/,
									 1000 /*max*/,
									 1 /*step*/,
									 &ok);
	if (!ok)
		return;

	insertColumnsLeft(count);
}

/*!
 * private helper function doing the actual insertion of columns to the left
 */
void SpreadsheetView::insertColumnsLeft(int count) {
	const int first = firstSelectedColumn();
	m_spreadsheet->insertColumns(first, count);
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
	int count = QInputDialog::getInt(nullptr,
									 i18n("Insert empty columns"),
									 i18n("Enter the number of columns to insert"),
									 1 /*value*/,
									 1 /*min*/,
									 1000 /*max*/,
									 1 /*step*/,
									 &ok);
	if (!ok)
		return;

	insertColumnsRight(count);
}

/*!
 * private helper function doing the actual insertion of columns to the right
 */
// TODO: check whether this function can be rewritten to use Spreadsheet::insertColumns()
void SpreadsheetView::insertColumnsRight(int count) {
	WAIT_CURSOR;
	m_spreadsheet->beginMacro(i18np("%1: insert empty column", "%1: insert empty columns", m_spreadsheet->name(), count));

	const int last = lastSelectedColumn();
	const int cols = m_spreadsheet->columnCount();
	if (last >= 0) {
		if (last < m_spreadsheet->columnCount() - 1) {
			// determine the column next to the last selected column
			Column* nextCol = m_spreadsheet->child<Column>(last + 1);

			for (int i = 0; i < count; ++i) {
				Column* newCol = new Column(QString::number(cols + i + 1), AbstractColumn::ColumnMode::Double);
				newCol->setPlotDesignation(AbstractColumn::PlotDesignation::Y);
				newCol->insertRows(0, m_spreadsheet->rowCount());

				// insert the new column before the column next to the last selected column
				m_spreadsheet->insertChildBefore(newCol, nextCol);
			}
		} else {
			for (int i = 0; i < count; ++i) {
				Column* newCol = new Column(QString::number(cols + i + 1), AbstractColumn::ColumnMode::Double);
				newCol->setPlotDesignation(AbstractColumn::PlotDesignation::Y);
				newCol->insertRows(0, m_spreadsheet->rowCount());

				// last column selected, no next column available -> add/append a new column
				if (!m_spreadsheet->statisticsSpreadsheet())
					m_spreadsheet->addChild(newCol);
				else
					m_spreadsheet->insertChildBefore(newCol, m_spreadsheet->statisticsSpreadsheet());
			}
		}
	} else {
		if (m_spreadsheet->columnCount() > 0) {
			for (int i = 0; i < count; ++i) {
				Column* newCol = new Column(QString::number(cols + i + 1), AbstractColumn::ColumnMode::Double);
				newCol->setPlotDesignation(AbstractColumn::PlotDesignation::Y);
				newCol->insertRows(0, m_spreadsheet->rowCount());

				// columns available but no columns selected -> append the new column at the very end
				if (!m_spreadsheet->statisticsSpreadsheet())
					m_spreadsheet->addChild(newCol);
				else
					m_spreadsheet->insertChildBefore(newCol, m_spreadsheet->statisticsSpreadsheet());
			}
		} else {
			// no columns available anymore -> resize the spreadsheet and the new column to the default size
			KConfigGroup group = Settings::group(QStringLiteral("Spreadsheet"));
			const int rows = group.readEntry(QLatin1String("RowCount"), 100);
			m_spreadsheet->setRowCount(rows);

			for (int i = 0; i < count; ++i) {
				Column* newCol = new Column(QString::number(cols + i + 1), AbstractColumn::ColumnMode::Double);
				(i == 0) ? newCol->setPlotDesignation(AbstractColumn::PlotDesignation::X) : newCol->setPlotDesignation(AbstractColumn::PlotDesignation::Y);
				newCol->insertRows(0, rows);

				// add/append a new column
				if (!m_spreadsheet->statisticsSpreadsheet())
					m_spreadsheet->addChild(newCol);
				else
					m_spreadsheet->insertChildBefore(newCol, m_spreadsheet->statisticsSpreadsheet());
			}
		}
	}
	Q_EMIT m_spreadsheet->emitColumnCountChanged();

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
	m_spreadsheet->clear(selectedColumns());
}

void SpreadsheetView::toggleFreezeColumn() {
	if (!m_frozenTableView) {
		m_frozenTableView = new QTableView(this);
		m_frozenTableView->setModel(m_model);

		auto* delegate = new SpreadsheetItemDelegate(this);
		connect(delegate, &SpreadsheetItemDelegate::returnPressed, this, &SpreadsheetView::advanceCell);
		connect(delegate, &SpreadsheetItemDelegate::editorEntered, this, [=]() {
			m_editorEntered = true;
		});
		connect(delegate, &SpreadsheetItemDelegate::closeEditor, this, [=]() {
			m_editorEntered = false;
		});

		m_frozenTableView->setItemDelegate(delegate);
		m_frozenTableView->setSelectionMode(QAbstractItemView::ExtendedSelection);

		m_frozenTableView->setFocusPolicy(Qt::NoFocus);
		m_frozenTableView->verticalHeader()->hide();
		m_frozenTableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed);
		m_frozenTableView->horizontalHeader()->installEventFilter(this);
		m_tableView->viewport()->stackUnder(m_frozenTableView);

		// 	m_frozenTableView->setStyleSheet("QTableView { border: none;"
		// 								"background-color: #8EDE21;"
		// 								"selection-background-color: #999}");
		m_frozenTableView->setSelectionModel(m_tableView->selectionModel());
		for (int col = 1; col < m_model->columnCount(); ++col)
			m_frozenTableView->setColumnHidden(col, true);

		m_frozenTableView->setColumnWidth(0, m_tableView->columnWidth(0));

		m_frozenTableView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
		m_frozenTableView->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
		m_frozenTableView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

		updateFrozenTableGeometry();

		// synchronize the sroll bars across the main and the frozen views
		connect(m_frozenTableView->verticalScrollBar(), &QAbstractSlider::valueChanged, m_tableView->verticalScrollBar(), &QAbstractSlider::setValue);
		connect(m_tableView->verticalScrollBar(), &QAbstractSlider::valueChanged, m_frozenTableView->verticalScrollBar(), &QAbstractSlider::setValue);
	}

	m_frozenTableView->setVisible(!m_frozenTableView->isVisible());
}

void SpreadsheetView::setSelectionAs() {
	const auto& columns = selectedColumns();
	if (!columns.size())
		return;

	m_spreadsheet->beginMacro(i18n("%1: set plot designation", m_spreadsheet->name()));

	auto* action = dynamic_cast<QAction*>(QObject::sender());
	if (!action)
		return;

	auto pd = (AbstractColumn::PlotDesignation)action->data().toInt();
	for (auto* col : columns)
		col->setPlotDesignation(pd);

	m_spreadsheet->endMacro();
}

/*!
 * add, subtract, multiply, divide
 */
void SpreadsheetView::modifyValues() {
	const auto& columns = selectedColumns();
	if (columns.isEmpty())
		return;

	const QAction* action = dynamic_cast<const QAction*>(QObject::sender());
	auto op = (AddSubtractValueDialog::Operation)action->data().toInt();
	auto* dlg = new AddSubtractValueDialog(m_spreadsheet, columns, op);
	dlg->exec();
}

void SpreadsheetView::reverseColumns() {
	const auto& columns = selectedColumns();
	if (columns.isEmpty())
		return;

	WAIT_CURSOR;
	m_spreadsheet->beginMacro(i18np("%1: reverse column", "%1: reverse columns", m_spreadsheet->name(), columns.size()));
	for (auto* col : columns) {
		if (col->columnMode() == AbstractColumn::ColumnMode::Double) {
			// determine the last row containing a valid value,
			// ignore all following empty rows when doing the reverse
			auto* data = static_cast<QVector<double>*>(col->data());
			QVector<double> new_data(*data);
			auto itEnd = new_data.begin();
			auto it = new_data.begin();
			while (it != new_data.end()) {
				if (!std::isnan(*it))
					itEnd = it;
				++it;
			}
			++itEnd;

			std::reverse(new_data.begin(), itEnd);
			col->setValues(new_data);
		} else if (col->columnMode() == AbstractColumn::ColumnMode::Integer) {
			auto* data = static_cast<QVector<int>*>(col->data());
			QVector<int> new_data(*data);
			std::reverse(new_data.begin(), new_data.end());
			col->setIntegers(new_data);
		} else if (col->columnMode() == AbstractColumn::ColumnMode::BigInt) {
			auto* data = static_cast<QVector<qint64>*>(col->data());
			QVector<qint64> new_data(*data);
			std::reverse(new_data.begin(), new_data.end());
			col->setBigInts(new_data);
		}
	}
	m_spreadsheet->endMacro();
	RESET_CURSOR;
}

void SpreadsheetView::dropColumnValues() {
	const auto& columns = selectedColumns();
	if (columns.isEmpty())
		return;

	auto* dlg = new DropValuesDialog(m_spreadsheet);
	dlg->setColumns(columns);
	dlg->exec();
}

void SpreadsheetView::maskColumnValues() {
	const auto& columns = selectedColumns();
	if (columns.isEmpty())
		return;

	auto* dlg = new DropValuesDialog(m_spreadsheet, true);
	dlg->setColumns(columns);
	dlg->exec();
}

void SpreadsheetView::sampleColumnValues() {
	const auto& columns = selectedColumns();
	if (columns.isEmpty())
		return;

	auto* dlg = new SampleValuesDialog(m_spreadsheet, this);
	dlg->setColumns(columns);
	dlg->exec();
}

void SpreadsheetView::flattenColumns() {
	const auto& columns = selectedColumns();
	if (columns.isEmpty())
		return;

	// ensure all selected columns have the same column mode
	auto mode = columns.at(0)->columnMode();
	for (auto* col : columns) {
		if (col->columnMode() != mode) {
			KMessageBox::error(this, i18n("The selected columns have different data types and cannot be flattened because of this. "));
			return;
		}
	}

	auto* dlg = new FlattenColumnsDialog(m_spreadsheet, this);
	dlg->setColumns(columns);
	dlg->exec();
}

// void SpreadsheetView::joinColumns() {
// 	//TODO
// }

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
	auto* message = "Normalization of the column <i>%1</i> was not possible because of %2.";
	m_spreadsheet->beginMacro(i18n("%1: normalize columns", m_spreadsheet->name()));

	for (auto* col : columns) {
		if (col->columnMode() != AbstractColumn::ColumnMode::Double && col->columnMode() != AbstractColumn::ColumnMode::Integer
			&& col->columnMode() != AbstractColumn::ColumnMode::BigInt)
			continue;

		if (col->columnMode() == AbstractColumn::ColumnMode::Integer || col->columnMode() == AbstractColumn::ColumnMode::BigInt)
			col->setColumnMode(AbstractColumn::ColumnMode::Double);

		auto* data = static_cast<QVector<double>*>(col->data());
		QVector<double> new_data(col->rowCount());

		switch (method) {
		case DivideBySum: {
			double sum = std::accumulate(data->begin(), data->end(), 0);
			if (sum != 0.0) {
				for (int i = 0; i < col->rowCount(); ++i)
					new_data[i] = data->operator[](i) / sum;
			} else {
				messages << i18n(message, col->name(), i18n("Sum = 0"));
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
				messages << i18n(message, col->name(), i18n("Min = 0"));
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
				messages << i18n(message, col->name(), i18n("Max = 0"));
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
				messages << i18n(message, col->name(), i18n("Count = 0"));
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
				messages << i18n(message, col->name(), i18n("Mean = 0"));
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
				messages << i18n(message, col->name(), i18n("Median = 0"));
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
					messages << i18n(message, col->name(), i18n("Mode = 0"));
				else
					messages << i18n(message, col->name(), i18n("'Mode not defined'"));
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
				messages << i18n(message, col->name(), i18n("Range = 0"));
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
				messages << i18n(message, col->name(), i18n("SD = 0"));
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
				messages << i18n(message, col->name(), i18n("MAD = 0"));
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
				messages << i18n(message, col->name(), i18n("IQR = 0"));
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
				messages << i18n(message, col->name(), i18n("SD = 0"));
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
				messages << i18n(message, col->name(), i18n("MAD = 0"));
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
				messages << i18n(message, col->name(), i18n("IQR = 0"));
				continue;
			}
			break;
		}
		case Rescale: {
			double min = col->statistics().minimum;
			double max = col->statistics().maximum;
			if (max - min != 0.0) {
				for (int i = 0; i < col->rowCount(); ++i)
					new_data[i] = rescaleIntervalMin + (data->operator[](i) - min) / (max - min) * (rescaleIntervalMax - rescaleIntervalMin);
			} else {
				messages << i18n(message, col->name(), i18n("Max - Min = 0"));
				continue;
			}
			break;
		}
		}

		col->setValues(new_data);
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

void SpreadsheetView::powerTransformSelectedColumns(QAction* action) {
	auto columns = selectedColumns();
	if (columns.isEmpty())
		return;

	auto power = static_cast<TukeyLadderPower>(action->data().toInt());

	WAIT_CURSOR;
	m_spreadsheet->beginMacro(i18n("%1: power transform columns", m_spreadsheet->name()));

	for (auto* col : columns) {
		if (col->columnMode() != AbstractColumn::ColumnMode::Double && col->columnMode() != AbstractColumn::ColumnMode::Integer
			&& col->columnMode() != AbstractColumn::ColumnMode::BigInt)
			continue;

		if (col->columnMode() == AbstractColumn::ColumnMode::Integer || col->columnMode() == AbstractColumn::ColumnMode::BigInt)
			col->setColumnMode(AbstractColumn::ColumnMode::Double);

		auto* data = static_cast<QVector<double>*>(col->data());
		QVector<double> new_data(col->rowCount());

		switch (power) {
		case InverseSquared: {
			for (int i = 0; i < col->rowCount(); ++i) {
				double x = data->operator[](i);
				if (x != 0.0)
					new_data[i] = 1 / gsl_pow_2(x);
				else
					new_data[i] = NAN;
			}
			break;
		}
		case Inverse: {
			for (int i = 0; i < col->rowCount(); ++i) {
				double x = data->operator[](i);
				if (x != 0.0)
					new_data[i] = 1 / x;
				else
					new_data[i] = NAN;
			}
			break;
		}
		case InverseSquareRoot: {
			for (int i = 0; i < col->rowCount(); ++i) {
				double x = data->operator[](i);
				if (x >= 0.0)
					new_data[i] = 1 / std::sqrt(x);
				else
					new_data[i] = NAN;
			}
			break;
		}
		case Log: {
			for (int i = 0; i < col->rowCount(); ++i) {
				double x = data->operator[](i);
				if (x >= 0.0)
					new_data[i] = log10(x);
				else
					new_data[i] = NAN;
			}
			break;
		}
		case SquareRoot: {
			for (int i = 0; i < col->rowCount(); ++i) {
				double x = data->operator[](i);
				if (x >= 0.0)
					new_data[i] = std::sqrt(x);
				else
					new_data[i] = NAN;
			}
			break;
		}
		case Squared: {
			for (int i = 0; i < col->rowCount(); ++i) {
				double x = data->operator[](i);
				new_data[i] = gsl_pow_2(x);
			}
			break;
		}
		case Cube: {
			for (int i = 0; i < col->rowCount(); ++i) {
				double x = data->operator[](i);
				new_data[i] = gsl_pow_3(x);
			}
			break;
		}
		}

		col->setValues(new_data);
	}

	m_spreadsheet->endMacro();
	RESET_CURSOR;
}

// TODO: either make this complete (support all column modes and normalization methods) or remove this code completely.
/*
void SpreadsheetView::normalizeSelection() {
	WAIT_CURSOR;
	m_spreadsheet->beginMacro(i18n("%1: normalize selection", m_spreadsheet->name()));
	double max = 0.0;
	for (int col = firstSelectedColumn(); col <= lastSelectedColumn(); col++)
		if (m_spreadsheet->column(col)->columnMode() == AbstractColumn::ColumnMode::Double)
			for (int row = 0; row < m_spreadsheet->rowCount(); row++) {
				if (isCellSelected(row, col) && m_spreadsheet->column(col)->valueAt(row) > max)
					max = m_spreadsheet->column(col)->valueAt(row);
			}

	if (max != 0.0) { // avoid division by zero
		// TODO setSuppressDataChangedSignal
		for (int col = firstSelectedColumn(); col <= lastSelectedColumn(); col++)
			if (m_spreadsheet->column(col)->columnMode() == AbstractColumn::ColumnMode::Double)
				for (int row = 0; row < m_spreadsheet->rowCount(); row++) {
					if (isCellSelected(row, col))
						m_spreadsheet->column(col)->setValueAt(row, m_spreadsheet->column(col)->valueAt(row) / max);
				}
	}
	m_spreadsheet->endMacro();
	RESET_CURSOR;
}*/

void SpreadsheetView::showAllColumnsStatistics() {
	showColumnStatistics(true);
}

void SpreadsheetView::showColumnStatistics(bool forAll) {
	QString dlgTitle(i18n("%1: column statistics", m_spreadsheet->name()));
	QVector<Column*> columns;

	// Column statistics can be shown for:
	// * all columns in the spreadsheet - called via the action_statistics_all_columns or when one single cell is selected (=no selection)
	// * all selected columns in the speadsheet - called via the context menu of the header
	// * selected column cells - called via the context menu in the spreadsheet with multiple selected cells
	const auto& indexes = m_tableView->selectionModel()->selectedIndexes();
	if (forAll || indexes.size() <= 1) {
		for (int col = 0; col < m_spreadsheet->columnCount(); ++col) {
			auto* column = m_spreadsheet->column(col);
			if (column->hasValues())
				columns << column;
		}
	} else {
		columns = selectedColumns(true);

		// if no columns are fully selected, copy the selected cells into new Columns which will be processes in the statistics dialog
		if (columns.isEmpty()) {
			const auto& children = m_spreadsheet->children<Column>();
			Column* targetColumn{nullptr};
			QMap<int, int> columnMappings; // key = child column index in the spreadsheet, value = column index in the vector of new colums
			QMap<int, int> rowMappings; // key = child column index in the spreadsheet, value = last row index
			for (const auto& index : indexes) {
				int col = index.column();
				int row = index.row();
				const auto* sourceColumn = children.at(col);

				if (columnMappings.contains(col))
					targetColumn = columns.at(columnMappings[col]);
				else {
					targetColumn = new Column(i18n("Selection in %1", sourceColumn->name()), sourceColumn->columnMode());
					columns << targetColumn;
					columnMappings[col] = columns.count() - 1;
					rowMappings[col] = 0;
				}

				int targetRow = rowMappings[col];

				switch (targetColumn->columnMode()) {
				case AbstractColumn::ColumnMode::Double:
					targetColumn->setValueAt(targetRow, sourceColumn->valueAt(row));
					break;
				case AbstractColumn::ColumnMode::Integer:
					targetColumn->setIntegerAt(targetRow, sourceColumn->integerAt(row));
					break;
				case AbstractColumn::ColumnMode::BigInt:
					targetColumn->setBigIntAt(targetRow, sourceColumn->bigIntAt(row));
					break;
				case AbstractColumn::ColumnMode::Text:
					targetColumn->setTextAt(targetRow, sourceColumn->textAt(row));
					break;
				case AbstractColumn::ColumnMode::DateTime:
				case AbstractColumn::ColumnMode::Month:
				case AbstractColumn::ColumnMode::Day:
					targetColumn->setDateTimeAt(targetRow, sourceColumn->dateTimeAt(row));
					break;
				}

				rowMappings[col] = targetRow + 1;
			}
		}
	}

	auto* dlg = new StatisticsDialog(dlgTitle, columns);
	dlg->setModal(true);
	dlg->show();
	QApplication::processEvents(QEventLoop::AllEvents, 0);
	QTimer::singleShot(0, this, [=]() {
		dlg->showStatistics();
	});
}

void SpreadsheetView::showRowStatistics() {
	QString dlgTitle(i18n("%1: row statistics", m_spreadsheet->name()));

	QVector<Column*> columns;
	const auto& rows = m_tableView->selectionModel()->selectedRows();

	for (int i = 0; i < rows.count(); ++i) {
		int row = rows.at(i).row();
		QVector<double> rowValues;
		for (int j = 0; j < m_spreadsheet->columnCount(); ++j)
			rowValues << m_spreadsheet->column(j)->valueAt(row);
		columns << new Column(i18n("Row %1", row + 1), rowValues);
	}

	auto* dlg = new StatisticsDialog(dlgTitle, columns);
	dlg->showStatistics();

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
	int count = QInputDialog::getInt(nullptr,
									 i18n("Insert multiple rows"),
									 i18n("Enter the number of rows to insert"),
									 1 /*value*/,
									 1 /*min*/,
									 1000000 /*max*/,
									 1 /*step*/,
									 &ok);
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
	m_spreadsheet->beginMacro(i18np("%1: insert empty row", "%1: insert empty rows", m_spreadsheet->name(), count));
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
	int count = QInputDialog::getInt(nullptr,
									 i18n("Insert multiple rows"),
									 i18n("Enter the number of rows to insert"),
									 1 /*value*/,
									 1 /*min*/,
									 1000000 /*max*/,
									 1 /*step*/,
									 &ok);
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
	m_spreadsheet->beginMacro(i18np("%1: insert empty row", "%1: insert empty rows", m_spreadsheet->name(), count));

	if (last < m_spreadsheet->rowCount() - 1)
		m_spreadsheet->insertRows(last + 1, count); // insert before the next to the last selected row
	else
		m_spreadsheet->appendRows(count); // append new rows at the end

	m_spreadsheet->endMacro();
	RESET_CURSOR;
}

void SpreadsheetView::removeSelectedRows() {
	if (firstSelectedRow() < 0)
		return;

	WAIT_CURSOR;
	m_spreadsheet->beginMacro(i18n("%1: remove selected rows", m_spreadsheet->name()));
	// TODO setSuppressDataChangedSignal
	for (const auto& i : selectedRows().intervals())
		m_spreadsheet->removeRows(i.start(), i.size());
	m_spreadsheet->endMacro();
	RESET_CURSOR;
}

void SpreadsheetView::clearSelectedCells() {
	// don't try to clear values if the selected cells don't have any values at all
	bool empty = true;

	const auto& columns = m_spreadsheet->children<Column>();
	const auto& indexes = m_tableView->selectionModel()->selectedIndexes();
	for (const auto& index : indexes) {
		if (columns.at(index.column())->isValid(index.row())) {
			empty = false;
			break;
		}
	}

	if (empty)
		return;

	WAIT_CURSOR;
	m_spreadsheet->beginMacro(i18n("%1: clear selected cells", m_spreadsheet->name()));
	for (auto* column : columns) {
		column->setSuppressDataChangedSignal(true);
		// 		if (formulaModeActive()) {
		// 			int col = m_spreadsheet->indexOfChild<Column>(column);
		// 			for (int row = last; row >= first; row--)
		// 				if (isCellSelected(row, col))
		// 					column->setFormula(row, QString());
		// 		} else {
		int index = m_spreadsheet->indexOfChild<Column>(column);
		if (isColumnSelected(index, true)) {
			// if the whole column is selected, clear directly instead of looping over the rows
			column->clear();
		} else {
			for (const auto& index : indexes)
				columns.at(index.column())->asStringColumn()->setTextAt(index.row(), QString());
		}

		column->setSuppressDataChangedSignal(false);
		column->setChanged();
	}
	m_spreadsheet->endMacro();
	RESET_CURSOR;
}

void SpreadsheetView::goToCell() {
	auto* dlg = new GoToDialog(this);
	if (dlg->exec() == QDialog::Accepted) {
		int row = dlg->row();
		if (row < 1)
			row = 1;
		if (row > m_spreadsheet->rowCount())
			row = m_spreadsheet->rowCount();

		int col = dlg->column();
		if (col < 1)
			col = 1;
		if (col > m_spreadsheet->columnCount())
			col = m_spreadsheet->columnCount();

		goToCell(row - 1, col - 1);
	}
	delete dlg;
}

void SpreadsheetView::searchReplace() {
	const auto* action = dynamic_cast<const QAction*>(QObject::sender());
	if (action == action_search_replace)
		showSearchReplace(true); // search&replace mode
	else
		showSearchReplace(false); // search mode
}

void SpreadsheetView::showSearchReplace(bool replace) {
	if (!m_searchReplaceWidget) {
		m_searchReplaceWidget = new SearchReplaceWidget(m_spreadsheet, this);
		static_cast<QVBoxLayout*>(this->layout())->addWidget(m_searchReplaceWidget);
	}

	m_searchReplaceWidget->setReplaceEnabled(replace);

	const auto& indexes = m_tableView->selectionModel()->selectedIndexes();
	if (!indexes.isEmpty()) {
		const auto& firstIndex = indexes.constFirst();
		const auto* column = m_spreadsheet->column(firstIndex.column());
		const int row = firstIndex.row();
		m_searchReplaceWidget->setInitialPattern(column->columnMode(), column->asStringColumn()->textAt(row));
	}

	m_searchReplaceWidget->show();

	// interrupt the event loop to show/render the widget first and set the focus
	// after this otherwise the focus in the search field is not set
	QTimer::singleShot(0, this, [=]() {
		m_searchReplaceWidget->setFocus();
	});
}

/*!
 * sorts the selected columns separately of each other in ascending order.
 */
void SpreadsheetView::sortAscending() {
	const auto& cols = selectedColumns();
	if (cols.isEmpty() || !hasValues(cols))
		return;

	for (auto* col : cols)
		col->setSuppressDataChangedSignal(true);
	m_spreadsheet->sortColumns(nullptr, cols, true);
	for (auto* col : cols) {
		col->setSuppressDataChangedSignal(false);
		col->setChanged();
	}
}

/*!
 * sorts the selected columns separately of each other in descending order.
 */
void SpreadsheetView::sortDescending() {
	const auto& cols = selectedColumns();
	if (cols.isEmpty() || !hasValues(cols))
		return;

	for (auto* col : cols)
		col->setSuppressDataChangedSignal(true);
	m_spreadsheet->sortColumns(nullptr, cols, false);
	for (auto* col : cols) {
		col->setSuppressDataChangedSignal(false);
		col->setChanged();
	}
}

/*!
 * custom sort of all or selected columns in the spreadsheet together by allowing
 * to specify the "sort by"-column(s) in the SortDialog.
 */
void SpreadsheetView::sortCustom() {
	const auto& cols = selectedColumns();
	QVector<Column*> columnsToSort;
	Column* leadingColumn{nullptr};

	bool sortAll{true};
	if (cols.size() == 0)
		// no columns selected, the function is being called from the spreadsheet context menu,
		// sort all columns
		columnsToSort = m_spreadsheet->children<Column>();
	else if (cols.size() == 1) {
		// one single column selected, sort the whole spreadsheet (all columns)
		// with the current selected column being the leading column
		columnsToSort = m_spreadsheet->children<Column>();
		leadingColumn = cols.constFirst();
	} else {
		columnsToSort = cols;
		sortAll = (cols.size() == m_spreadsheet->columnCount());
	}

	// don't do any sort if there are no values yet in the columns
	if (!hasValues(columnsToSort))
		return;

	for (auto* col : columnsToSort)
		col->setSuppressDataChangedSignal(true);

	auto* dlg = new SortDialog(this, sortAll);
	connect(dlg, &SortDialog::sort, m_spreadsheet, &Spreadsheet::sortColumns);
	dlg->setColumns(columnsToSort, leadingColumn);

	int rc = dlg->exec();

	for (auto* col : columnsToSort) {
		col->setSuppressDataChangedSignal(false);
		if (rc == QDialog::Accepted)
			col->setChanged();
	}
}

bool SpreadsheetView::hasValues(const QVector<Column*> cols) {
	bool hasValues = false;
	for (const auto* col : cols) {
		if (col->hasValues()) {
			hasValues = true;
			break;
		}
	}

	return hasValues;
}

/*!
  Cause a repaint of the header.
*/
void SpreadsheetView::updateHeaderGeometry(Qt::Orientation o, int first, int last) {
	// TODO
	if (o != Qt::Horizontal || last < first)
		return;
	m_tableView->horizontalHeader()->setStretchLastSection(true); // ugly hack (flaw in Qt? Does anyone know a better way?)
	m_tableView->horizontalHeader()->updateGeometry();
	m_tableView->horizontalHeader()->setStretchLastSection(false); // ugly hack part 2
	// Update the geometry for the sparkline header
	m_horizontalHeader->m_sparkLineSlave->setStretchLastSection(true);
	m_horizontalHeader->m_sparkLineSlave->updateGeometry();
	m_horizontalHeader->m_sparkLineSlave->setStretchLastSection(false);
	// Update the geometry for the comment header
	m_horizontalHeader->m_commentSlave->setStretchLastSection(true);
	m_horizontalHeader->m_commentSlave->updateGeometry();
	m_horizontalHeader->m_commentSlave->setStretchLastSection(false);
}

void SpreadsheetView::selectAll() {
	// HACK: m_tableView->selectAll() doesn't work for some reasons anymore, we need to create the selection manually
	QItemSelection itemSelection;
	itemSelection.select(m_model->index(0, 0), m_model->index(m_model->rowCount() - 1, m_model->columnCount() - 1));
	m_tableView->selectionModel()->select(itemSelection, QItemSelectionModel::Select);
}

/*!
  selects the column \c column in the speadsheet view .
*/
void SpreadsheetView::selectColumn(int column) {
	const auto& index = m_model->index(0, column);
	m_tableView->scrollTo(index);
	QItemSelection selection(index, m_model->index(m_spreadsheet->rowCount() - 1, column));
	m_suppressSelectionChangedEvent = true;
	m_tableView->selectionModel()->select(selection, QItemSelectionModel::Select);
	m_suppressSelectionChangedEvent = false;
}

/*!
  deselects the column \c column in the speadsheet view .
*/
void SpreadsheetView::deselectColumn(int column) {
	QItemSelection selection(m_model->index(0, column), m_model->index(m_spreadsheet->rowCount() - 1, column));
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
void SpreadsheetView::selectionChanged(const QItemSelection& /*selected*/, const QItemSelection& /*deselected*/) {
	if (m_suppressSelectionChangedEvent)
		return;

	PERFTRACE(QLatin1String(Q_FUNC_INFO));

	// determine the columns that were fully selected or deselected in the spreadsheet to also select/deselect them in the project explorer
	const auto* selModel = m_tableView->selectionModel();
	int fullySelectedCount = 0; // the number of fully selected columns
	bool notFullySelected = false; // check if we have columns that are not fully selected (some cells selected only)
	for (int col = 0; col < m_spreadsheet->columnCount(); ++col) {
		const bool selected = selModel->isColumnSelected(col);
		m_spreadsheet->setColumnSelectedInView(col, selected);
		if (selected)
			++fullySelectedCount;
		else if (!notFullySelected) //
			notFullySelected = selModel->columnIntersectsSelection(col);
	}

	// determine the number of selected cells, columns, missing and masked values in the current selection and show this information in the status bar.
	// Note, calling selModel->selectedIndexes() is very expensive for a high number of rows/cells in the spreadsheet and when whole columns are being
	// selected by clicking on the spreadsheet header. To avoid calling this expensive functions, handle the case when only full columns were selected:
	if (fullySelectedCount > 0 && !notFullySelected && m_spreadsheet->rowCount() > 10000) {
		Q_EMIT m_spreadsheet->statusInfo(i18n("Selected 1: %1 rows, %2 columns", m_spreadsheet->rowCount(), fullySelectedCount));
		return;
	}

	const auto& indexes = selModel->selectedIndexes();
	if (indexes.empty() || indexes.count() == 1) {
		Q_EMIT m_spreadsheet->statusInfo(QString());
		return;
	}
	else if (indexes.count() > 10000) { // more than 10k selected cells, skip the expensive logic below to check for maskes values, etc.
		Q_EMIT m_spreadsheet->statusInfo(i18n("Selected: %1 cells", indexes.count()));
		return;
	}

	const auto& columns = m_spreadsheet->children<Column>();
	int maskedValuesCount = 0;
	int missingValuesCount = 0;
	for (const auto& index : indexes) {
		const int col = index.column();
		const int row = index.row();
		const auto& column = columns.at(col);
		if (!column->isValid(row))
			missingValuesCount++;
		if (column->isMasked(row))
			maskedValuesCount++;
	}

	const int selectedCellsCount = indexes.count();
	const QPair<int, int> selectedRowCol = qMakePair(selectedRowCount(false), selectedColumnCount(false));
	QString selectedRowsText = i18np("row", "rows", selectedRowCol.first);
	QString selectedColumnsText = i18np("column", "columns", selectedRowCol.second);
	QString selectedCellsText = i18n("cells");
	QString maskedValuesText = (maskedValuesCount == 1) ? i18n("masked value") : (!maskedValuesCount) ? QString() : i18n("masked values");
	QString missingValuesText = (missingValuesCount == 1) ? i18n("missing value") : (!missingValuesCount) ? QString() : i18n("missing values");
	QString maskedValuesCountText = (!maskedValuesCount) ? QString() : i18n(" , ") + i18n("%1", maskedValuesCount);
	QString missingValuesCountText = (!missingValuesCount) ? QString() : i18n(", ") + i18n("%1", missingValuesCount);
	QString resultString;

	if (selectedCellsCount == selectedRowCol.first * selectedRowCol.second)
		resultString = i18n("Selected: %1 %2, %3 %4%5 %6 %7 %8",
							selectedRowCol.first,
							selectedRowsText,
							selectedRowCol.second,
							selectedColumnsText,
							maskedValuesCountText,
							maskedValuesText,
							missingValuesCountText,
							missingValuesText);
	else
		resultString = i18n("Selected: %1 %2%3 %4 %5 %6",
							selectedCellsCount,
							selectedCellsText,
							maskedValuesCountText,
							maskedValuesText,
							missingValuesCountText,
							missingValuesText);

	Q_EMIT m_spreadsheet->statusInfo(resultString);
}

bool SpreadsheetView::exportView() {
	QDEBUG(Q_FUNC_INFO)
	auto* dlg = new ExportSpreadsheetDialog(this);
	dlg->setProjectFileName(m_spreadsheet->project()->fileName());
	dlg->setFileName(m_spreadsheet->name());
#ifdef HAVE_FITS
	dlg->setExportTo(QStringList() << i18n("FITS image") << i18n("FITS table"));
#endif
	for (int i = 0; i < m_spreadsheet->columnCount(); ++i) {
		if (m_spreadsheet->column(i)->columnMode() != AbstractColumn::ColumnMode::Double) {
			dlg->setExportToImage(false);
			break;
		}
	}
	if (selectedColumnCount(false /* partial selection */) == 0)
		dlg->setExportSelection(false);
	bool ret;
	if ((ret = dlg->exec()) == QDialog::Accepted) {
		const QString path = dlg->path();
		const bool exportHeader = dlg->exportHeader();
		WAIT_CURSOR;
		switch (dlg->format()) {
		case ExportSpreadsheetDialog::Format::ASCII: {
			const QString separator = dlg->separator();
			const QLocale::Language format = dlg->numberFormat();
			exportToFile(path, exportHeader, separator, format);
			break;
		}
		case ExportSpreadsheetDialog::Format::LaTeX: {
			const bool exportLatexHeader = dlg->exportLatexHeader();
			const bool gridLines = dlg->gridLines();
			const bool captions = dlg->captions();
			const bool skipEmptyRows = dlg->skipEmptyRows();
			const bool exportEntire = dlg->entireSpreadheet();
			exportToLaTeX(path, exportHeader, gridLines, captions, exportLatexHeader, skipEmptyRows, exportEntire);
			break;
		}
		case ExportSpreadsheetDialog::Format::FITS: {
#ifdef HAVE_FITS
			const int exportTo = dlg->exportToFits();
			const bool commentsAsUnits = dlg->commentsAsUnitsFits();
			exportToFits(path, exportTo, commentsAsUnits);
#endif
			break;
		}
		case ExportSpreadsheetDialog::Format::XLSX: {
			exportToXLSX(path, exportHeader);
			break;
		}
		case ExportSpreadsheetDialog::Format::SQLite: {
			exportToSQLite(path);
			break;
		}
		case ExportSpreadsheetDialog::Format::MCAP: {
			std::pair<int, int> compressionSettings = dlg->getMcapSettings();
			exportToMCAP(path, compressionSettings.first, compressionSettings.second);
			break;
		}
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
	auto* dlg = new QPrintPreviewDialog(this);
	connect(dlg, &QPrintPreviewDialog::paintRequested, this, &SpreadsheetView::print);
	return dlg->exec();
}

/*!
  prints the complete spreadsheet to \c printer.
 */
void SpreadsheetView::print(QPrinter* printer) const {
	WAIT_CURSOR;
	QPainter painter(printer);

	const int dpiy = printer->logicalDpiY();
	const int margin = (int)((1 / GSL_CONST_CGS_INCH) * dpiy); // 1 cm margins

	QHeaderView* hHeader = m_tableView->horizontalHeader();
	QHeaderView* vHeader = m_tableView->verticalHeader();

	const int rows = m_spreadsheet->rowCount();
	const int cols = m_spreadsheet->columnCount();
	int height = margin;
	const int vertHeaderWidth = vHeader->width();
	const int width = printer->pageLayout().paintRectPixels(printer->resolution()).width() - 2 * margin;

	int columnsPerTable = 0;
	int headerStringWidth = 0;
	int firstRowStringWidth = 0;
	bool tablesNeeded = false;
	for (int col = 0; col < cols; ++col) {
		headerStringWidth += m_tableView->columnWidth(col);
		firstRowStringWidth += m_spreadsheet->column(col)->asStringColumn()->textAt(0).length();
		if ((headerStringWidth >= width) || (firstRowStringWidth >= width)) {
			tablesNeeded = true;
			break;
		}
		columnsPerTable++;
	}

	int tablesCount = (columnsPerTable != 0) ? cols / columnsPerTable : 0;
	const int remainingColumns = (columnsPerTable != 0) ? cols % columnsPerTable : cols;

	if (!tablesNeeded) {
		tablesCount = 1;
		columnsPerTable = cols;
	}

	if (remainingColumns > 0)
		tablesCount++;
	// Paint the horizontal header first
	for (int table = 0; table < tablesCount; ++table) {
		int right = margin + vertHeaderWidth;

		painter.setFont(hHeader->font());
		QString headerString = m_tableView->model()->headerData(0, Qt::Horizontal).toString();
		QRect br;
		br = painter.boundingRect(br, Qt::AlignCenter, headerString);
		QRect tr(br);
		if (table != 0)
			height += tr.height();
		painter.drawLine(right, height, right, height + br.height());

		int i = table * columnsPerTable;
		int toI = table * columnsPerTable + columnsPerTable;
		if ((remainingColumns > 0) && (table == tablesCount - 1)) {
			i = (tablesCount - 1) * columnsPerTable;
			toI = (tablesCount - 1) * columnsPerTable + remainingColumns;
		}

		for (; i < toI; ++i) {
			headerString = m_tableView->model()->headerData(i, Qt::Horizontal).toString();
			const int w = m_tableView->columnWidth(i);
			tr.setTopLeft(QPoint(right, height));
			tr.setWidth(w);
			tr.setHeight(br.height());

			painter.drawText(tr, Qt::AlignCenter, headerString);
			right += w;
			painter.drawLine(right, height, right, height + tr.height());
		}

		painter.drawLine(margin + vertHeaderWidth, height, right - 1, height); // first horizontal line
		height += tr.height();
		painter.drawLine(margin, height, right - 1, height);

		// print table values
		for (i = 0; i < rows; ++i) {
			right = margin;
			QString cellText = m_tableView->model()->headerData(i, Qt::Vertical).toString() + QLatin1Char('\t');
			tr = painter.boundingRect(tr, Qt::AlignCenter, cellText);
			painter.drawLine(right, height, right, height + tr.height());

			br.setTopLeft(QPoint(right, height));
			br.setWidth(vertHeaderWidth);
			br.setHeight(tr.height());
			painter.drawText(br, Qt::AlignCenter, cellText);
			right += vertHeaderWidth;
			painter.drawLine(right, height, right, height + tr.height());
			int j = table * columnsPerTable;
			int toJ = table * columnsPerTable + columnsPerTable;
			if ((remainingColumns > 0) && (table == tablesCount - 1)) {
				j = (tablesCount - 1) * columnsPerTable;
				toJ = (tablesCount - 1) * columnsPerTable + remainingColumns;
			}
			for (; j < toJ; j++) {
				int w = m_tableView->columnWidth(j);
				cellText = m_spreadsheet->column(j)->isValid(i) ? m_spreadsheet->text(i, j) + QStringLiteral("\t") : QStringLiteral("- \t");
				tr = painter.boundingRect(tr, Qt::AlignCenter, cellText);
				br.setTopLeft(QPoint(right, height));
				br.setWidth(w);
				br.setHeight(tr.height());
				painter.drawText(br, Qt::AlignCenter, cellText);
				right += w;
				painter.drawLine(right, height, right, height + tr.height());
			}
			height += br.height();
			painter.drawLine(margin, height, right - 1, height);

			if (height >= printer->height() - margin) {
				printer->newPage();
				height = margin;
				painter.drawLine(margin, height, right, height);
			}
		}
	}
	RESET_CURSOR;
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
		auto mode = col->columnMode();
		if (mode == AbstractColumn::ColumnMode::Double) {
			for (int i = 0; i < m_spreadsheet->rowCount(); ++i) {
				if (!std::isnan(col->valueAt(i)) && i > maxRow)
					maxRow = i;
			}
		}
		if (mode == AbstractColumn::ColumnMode::Integer || mode == AbstractColumn::ColumnMode::BigInt) {
			// TODO:
			// integer column found. Since empty integer cells are equal to 0
			// at the moment, we need to export the whole column.
			// this logic needs to be adjusted once we're able to descriminate
			// between empty and 0 values for integer columns
			maxRow = m_spreadsheet->rowCount() - 1;
			break;
		} else if (mode == AbstractColumn::ColumnMode::DateTime) {
			for (int i = 0; i < m_spreadsheet->rowCount(); ++i) {
				if (col->dateTimeAt(i).isValid() && i > maxRow)
					maxRow = i;
			}
		} else if (mode == AbstractColumn::ColumnMode::Text) {
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
	if (!file.open(QFile::WriteOnly | QFile::Truncate)) {
		RESET_CURSOR;
		QMessageBox::critical(nullptr, i18n("Failed to export"), i18n("Failed to write to '%1'. Please check the path.", path));
		return;
	}

	PERFTRACE(QStringLiteral("export spreadsheet to file"));
	QTextStream out(&file);

	int maxRow = maxRowToExport();
	if (maxRow < 0)
		return;

	const int cols = m_spreadsheet->columnCount();
	QString sep = separator;
	sep = sep.replace(QLatin1String("TAB"), QLatin1String("\t"), Qt::CaseInsensitive);
	sep = sep.replace(QLatin1String("SPACE"), QLatin1String(" "), Qt::CaseInsensitive);

	// export header (column names)
	if (exportHeader) {
		for (int j = 0; j < cols; ++j) {
			out << '"' << m_spreadsheet->column(j)->name() << '"';
			if (j != cols - 1)
				out << sep;
		}
		out << '\n';
	}

	// export values
	QLocale locale(language);
	for (int i = 0; i <= maxRow; ++i) {
		for (int j = 0; j < cols; ++j) {
			Column* col = m_spreadsheet->column(j);
			if (col->columnMode() == AbstractColumn::ColumnMode::Double) {
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

void SpreadsheetView::exportToLaTeX(const QString& path,
									const bool exportHeaders,
									const bool gridLines,
									const bool captions,
									const bool latexHeaders,
									const bool skipEmptyRows,
									const bool exportEntire) const {
	QFile file(path);
	if (!file.open(QFile::WriteOnly | QFile::Truncate)) {
		RESET_CURSOR;
		QMessageBox::critical(nullptr, i18n("Failed to export"), i18n("Failed to write to '%1'. Please check the path.", path));
		return;
	}

	QList<Column*> toExport;
	int cols;
	int totalRowCount = 0;
	if (exportEntire) {
		cols = const_cast<SpreadsheetView*>(this)->m_spreadsheet->columnCount();
		totalRowCount = m_spreadsheet->rowCount();
		for (int col = 0; col < cols; ++col)
			toExport << m_spreadsheet->column(col);
	} else {
		cols = const_cast<SpreadsheetView*>(this)->selectedColumnCount(false /* partial selection */);
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

	const int tablesCount = (columnsPerTable != 0) ? cols / columnsPerTable : 0;
	const int remainingColumns = (columnsPerTable != 0) ? cols % columnsPerTable : cols;

	bool columnsSeparating = (cols > columnsPerTable);
	QTextStream out(&file);

	const QString latexFullPath = safeExecutableName(QStringLiteral("latex"));
	if (latexFullPath.isEmpty()) {
		DEBUG(Q_FUNC_INFO << ", WARNING: latex not found!")
		return;
	}

	QProcess tex;
	startHostProcess(tex, latexFullPath, QStringList() << QStringLiteral("--version"), QProcess::ReadOnly);
	tex.waitForFinished(500);
	QString texVersionOutput = QLatin1String(tex.readAllStandardOutput());
	texVersionOutput = texVersionOutput.split(QLatin1Char('\n'))[0];

	int yearidx = -1;
	for (int i = texVersionOutput.size() - 1; i >= 0; --i) {
		if (texVersionOutput.at(i) == QLatin1Char('2')) {
			yearidx = i;
			break;
		}
	}

	if (texVersionOutput.at(yearidx + 1) == QLatin1Char('/'))
		yearidx -= 3;

	bool ok;
	texVersionOutput.mid(yearidx, 4).toInt(&ok);
	int version = -1;
	if (ok)
		version = texVersionOutput.mid(yearidx, 4).toInt(&ok);

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

	QString endTabularTable(QStringLiteral("\\end{tabular} \n \\end{table} \n"));
	QString tableCaption(QStringLiteral("\\caption{") + m_spreadsheet->name() + QStringLiteral("} \n"));
	QString beginTable(QStringLiteral("\\begin{table}[ht] \n"));

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
				textable << (gridLines ? QLatin1String(" c |") : QLatin1String(" c "));
			textable << QLatin1String("} \n");
			if (gridLines)
				textable << QLatin1String("\\hline \n");

			if (exportHeaders) {
				if (latexHeaders)
					textable << QLatin1String("\\rowcolor{HeaderBgColor} \n");
				for (int col = table * columnsPerTable; col < (table * columnsPerTable) + columnsPerTable; ++col) {
					textable << toExport.at(col)->name();
					if (col != ((table * columnsPerTable) + columnsPerTable) - 1)
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
				for (int col = table * columnsPerTable; col < (table * columnsPerTable) + columnsPerTable; ++col) {
					if (toExport.at(col)->isValid(row)) {
						notEmpty = true;
						values << toExport.at(col)->asStringColumn()->textAt(row);
					} else
						values << QLatin1String("-");
					if (col != ((table * columnsPerTable) + columnsPerTable) - 1)
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
						out << QLatin1String("\\clearpage \n");

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

		// new table for the remaining columns
		QStringList remainingTable;
		remainingTable << beginTable;
		if (captions)
			remainingTable << tableCaption;
		remainingTable << QLatin1String("\\centering \n");
		remainingTable << QLatin1String("\\begin{tabular}{") << (gridLines ? QStringLiteral("|") : QString());
		for (int c = 0; c < remainingColumns; ++c)
			remainingTable << (gridLines ? QLatin1String(" c |") : QLatin1String(" c "));
		remainingTable << QLatin1String("} \n");
		if (gridLines)
			remainingTable << QLatin1String("\\hline \n");
		if (exportHeaders) {
			if (latexHeaders)
				remainingTable << QLatin1String("\\rowcolor{HeaderBgColor} \n");
			for (int col = 0; col < remainingColumns; ++col) {
				remainingTable << toExport.at(col + (tablesCount * columnsPerTable))->name();
				if (col != remainingColumns - 1)
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
			for (int col = 0; col < remainingColumns; ++col) {
				if (toExport.at(col + (tablesCount * columnsPerTable))->isValid(row)) {
					notEmpty = true;
					values << toExport.at(col + (tablesCount * columnsPerTable))->asStringColumn()->textAt(row);
				} else
					values << QLatin1String("-");
				if (col != remainingColumns - 1)
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
			textable << (gridLines ? QLatin1String(" c |") : QLatin1String(" c "));
		textable << QLatin1String("} \n");
		if (gridLines)
			textable << QLatin1String("\\hline \n");
		if (exportHeaders) {
			if (latexHeaders)
				textable << QLatin1String("\\rowcolor{HeaderBgColor} \n");
			for (int col = 0; col < cols; ++col) {
				textable << toExport.at(col)->name();
				if (col != cols - 1)
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

			for (int col = 0; col < cols; ++col) {
				if (toExport.at(col)->isValid(row)) {
					notEmpty = true;
					values << toExport.at(col)->asStringColumn()->textAt(row);
				} else
					values << QStringLiteral("-");
				if (col != cols - 1)
					values << QStringLiteral(" & ");
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
					out << QLatin1String("\\clearpage \n");
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

void SpreadsheetView::exportToFits(const QString& fileName, const int exportTo, const bool commentsAsUnits) const {
	auto* filter = new FITSFilter;

	filter->setExportTo(exportTo);
	filter->setCommentsAsUnits(commentsAsUnits);
	filter->write(fileName, m_spreadsheet);

	delete filter;
}

void SpreadsheetView::exportToMCAP(const QString& fileName, int compressionMode, int compressionLevel) const {
	auto* filter = new McapFilter;
	filter->writeWithOptions(fileName, m_spreadsheet, compressionMode, compressionLevel);

	delete filter;
}

void SpreadsheetView::exportToXLSX(const QString& fileName, const bool exportHeader) const {
	auto* filter = new XLSXFilter;

	DEBUG("EXPORT HEADER = " << exportHeader)
	filter->setColumnNamesAsFirstRow(exportHeader);
	filter->write(fileName, m_spreadsheet);

	delete filter;
}

void SpreadsheetView::exportToSQLite(const QString& path) const {
	QFile file(path);
	if (!file.open(QFile::WriteOnly | QFile::Truncate))
		return;

	PERFTRACE(QStringLiteral("export spreadsheet to SQLite database"));
	QApplication::processEvents(QEventLoop::AllEvents, 0);

	// create database
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

	// create table
	const int cols = m_spreadsheet->columnCount();
	QString query = QLatin1String("create table ") + m_spreadsheet->name() + QLatin1String(" (");
	for (int i = 0; i < cols; ++i) {
		Column* col = m_spreadsheet->column(i);
		if (i != 0)
			query += QLatin1String(", ");

		query += QLatin1String("\"") + col->name() + QLatin1String("\" ");
		switch (col->columnMode()) {
		case AbstractColumn::ColumnMode::Double:
			query += QLatin1String("REAL");
			break;
		case AbstractColumn::ColumnMode::Integer:
		case AbstractColumn::ColumnMode::BigInt:
			query += QLatin1String("INTEGER");
			break;
		case AbstractColumn::ColumnMode::Text:
		case AbstractColumn::ColumnMode::Month:
		case AbstractColumn::ColumnMode::Day:
		case AbstractColumn::ColumnMode::DateTime:
			query += QLatin1String("TEXT");
			break;
		}
	}
	query += QLatin1Char(')');
	QSqlQuery q;
	if (!q.exec(query)) {
		RESET_CURSOR;
		KMessageBox::error(nullptr, i18n("Failed to create table in the SQLite database %1.", path) + QLatin1Char('\n') + q.lastError().databaseText());
		db.close();
		return;
	}

	int maxRow = maxRowToExport();
	if (maxRow < 0) {
		db.close();
		return;
	}

	// create bulk insert statement in batches of 10k rows
	{
		PERFTRACE(QStringLiteral("Insert the data"));
		q.exec(QLatin1String("BEGIN TRANSACTION;"));

		// create the first part of the INSERT-statement without the values
		QString insertQuery = QStringLiteral("INSERT INTO '") + m_spreadsheet->name() + QStringLiteral("' (");
		for (int i = 0; i < cols; ++i) {
			if (i != 0)
				insertQuery += QLatin1String(", ");
			insertQuery += QLatin1Char('\'') + m_spreadsheet->column(i)->name() + QLatin1Char('\'');
		}
		insertQuery += QLatin1String(") VALUES ");

		// add values in chunks of 10k row
		int chunkSize = 10000;
		int chunks = std::ceil((double)maxRow / chunkSize);
		for (int chunk = 0; chunk < chunks; ++chunk) {
			query = insertQuery;
			for (int i = 0; i < chunkSize; ++i) {
				int row = chunk * chunkSize + i;
				if (row > maxRow)
					break;

				if (i != 0)
					query += QLatin1String(",");

				query += QLatin1Char('(');
				for (int j = 0; j < cols; ++j) {
					auto* col = m_spreadsheet->column(j);
					if (j != 0)
						query += QLatin1String(", ");

					query += QLatin1Char('\'') + col->asStringColumn()->textAt(row) + QLatin1Char('\'');
				}
				query += QLatin1String(")");
			}
			query += QLatin1Char(';');

			// insert values for the current chunk of data
			if (!q.exec(query)) {
				RESET_CURSOR;
				KMessageBox::error(nullptr, i18n("Failed to insert values into the table."));
				QDEBUG(Q_FUNC_INFO << ", bulk insert error " << q.lastError().databaseText());
				db.close();
				return;
			}
		}

	} // end of perf-trace scope

	// commit the transaction and close the database
	q.exec(QLatin1String("COMMIT TRANSACTION;"));
	db.close();
}
