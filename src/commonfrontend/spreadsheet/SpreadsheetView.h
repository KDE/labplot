/*
    File                 : SpreadsheetView.h
    Project              : LabPlot
    Description          : View class for Spreadsheet
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2010-2022 Alexander Semke <alexander.semke@web.de>
    SPDX-License-Identifier: GPL-2.0-or-later
*/


#ifndef SPREADSHEETVIEW_H
#define SPREADSHEETVIEW_H

#include <QWidget>

#include "backend/core/AbstractColumn.h"
#include "backend/lib/IntervalAttribute.h"
#include <QLocale>

class AbstractAspect;
class Column;
class Spreadsheet;
class SpreadsheetHeaderView;
class SpreadsheetModel;

class QActionGroup;
class QFrame;
class QItemSelection;
class QLineEdit;
class QMenu;
class QPrinter;
class QModelIndex;
class QResizeEvent;
class QTableView;
class QToolBar;

#ifdef HAVE_TOUCHBAR
	class KDMacTouchBar;
#endif

class SpreadsheetView : public QWidget {
	Q_OBJECT

	friend class SpreadsheetTest;

public:
	explicit SpreadsheetView(Spreadsheet*, bool readOnly = false);
	~SpreadsheetView() override;

	void resizeHeader();

	void showComments(bool on = true);
	bool areCommentsShown() const;

	int selectedColumnCount(bool full = false) const;
	int selectedColumnCount(AbstractColumn::PlotDesignation) const;
	bool isColumnSelected(int col, bool full = false) const;
	QVector<Column*> selectedColumns(bool full = false) const;
	int firstSelectedColumn(bool full = false) const;
	int lastSelectedColumn(bool full = false) const;

	bool isRowSelected(int row, bool full = false) const;
	int firstSelectedRow(bool full = false) const;
	int lastSelectedRow(bool full = false) const;
	IntervalAttribute<bool> selectedRows(bool full = false) const;

	bool isCellSelected(int row, int col) const;
	void setCellSelected(int row, int col, bool select = true);
	void setCellsSelected(int first_row, int first_col, int last_row, int last_col, bool select = true);
	void getCurrentCell(int* row, int* col) const;

	bool exportView();
	bool printView();
	bool printPreview();

protected:
	void resizeEvent(QResizeEvent*) override;

private:
	void init();
	void initActions();
	void initMenus();
	void connectActions();
	bool formulaModeActive() const;
	void exportToFile(const QString&, const bool, const QString&, QLocale::Language) const;
	void exportToLaTeX(const QString&, const bool exportHeaders,
	                   const bool gridLines, const bool captions, const bool latexHeaders,
	                   const bool skipEmptyRows,const bool exportEntire) const;
	void exportToFits(const QString& path, const int exportTo, const bool commentsAsUnits) const;
	void exportToSQLite(const QString& path) const;
	int maxRowToExport() const;

	void insertColumnsLeft(int);
	void insertColumnsRight(int);

	void insertRowsAbove(int);
	void insertRowsBelow(int);

	void updateFrozenTableGeometry();

	QTableView* m_tableView{nullptr};
	QTableView* m_frozenTableView{nullptr};
	bool m_editorEntered{false};
	Spreadsheet* m_spreadsheet;
	SpreadsheetModel* m_model;
	SpreadsheetHeaderView* m_horizontalHeader;
	QFrame* m_frameSearch{nullptr};
	QLineEdit* m_leSearch{nullptr};
	bool m_suppressSelectionChangedEvent{false};
	bool m_readOnly;
	bool eventFilter(QObject*, QEvent*) override;
	void checkSpreadsheetMenu();
	void checkSpreadsheetSelectionMenu();
	void checkColumnMenus(bool numeric, bool datetime, bool hasValues);

	//selection related actions
	QAction* action_cut_selection;
	QAction* action_copy_selection;
	QAction* action_paste_into_selection;
	QAction* action_mask_selection;
	QAction* action_unmask_selection;
	QAction* action_clear_selection;
// 		QAction* action_set_formula;
// 		QAction* action_recalculate;
	QAction* action_fill_row_numbers;
	QAction* action_fill_random;
	QAction* action_fill_equidistant;
	QAction* action_fill_random_nonuniform;
	QAction* action_fill_const;
	QAction* action_fill_function;

	//spreadsheet related actions
	QAction* action_toggle_comments;
	QAction* action_select_all;
	QAction* action_clear_spreadsheet;
	QAction* action_clear_masks;
	QAction* action_sort_spreadsheet;
	QAction* action_formatting_heatmap;
	QAction* action_formatting_remove;
	QAction* action_go_to_cell;
	QAction* action_search;
	QAction* action_statistics_all_columns;

	//column related actions
	QAction* action_insert_column_left;
	QAction* action_insert_column_right;
	QAction* action_insert_columns_left;
	QAction* action_insert_columns_right;
	QAction* action_remove_columns;
	QAction* action_clear_columns;
	QAction* action_add_columns;
	QAction* action_set_as_none;
	QAction* action_set_as_x;
	QAction* action_set_as_y;
	QAction* action_set_as_z;
	QAction* action_set_as_xerr;
	QAction* action_set_as_xerr_plus;
	QAction* action_set_as_xerr_minus;
	QAction* action_set_as_yerr;
	QAction* action_set_as_yerr_plus;
	QAction* action_set_as_yerr_minus;
	QAction* action_reverse_columns;
	QAction* action_add_value;
	QAction* action_subtract_value;
	QAction* action_multiply_value;
	QAction* action_divide_value;
	QAction* action_drop_values;
	QAction* action_mask_values;
	QAction* action_join_columns;
	QActionGroup* normalizeColumnActionGroup;
	QActionGroup* ladderOfPowersActionGroup;
	QAction* action_sort_columns;
	QAction* action_sort_asc_column;
	QAction* action_sort_desc_column;
	QAction* action_statistics_columns;
	QAction* action_freeze_columns;

	//row related actions
	QAction* action_insert_row_above;
	QAction* action_insert_row_below;
	QAction* action_insert_rows_above;
	QAction* action_insert_rows_below;
	QAction* action_remove_rows;
	QAction* action_clear_rows;
	QAction* action_statistics_rows;

	//analysis and plot data menu actions
	QAction* action_plot_data_xycurve;
	QAction* action_plot_data_histogram;
	QAction* action_plot_data_boxplot;
	QAction* addDataOperationAction;
	QAction* addDataReductionAction;
	QAction* addDifferentiationAction;
	QAction* addIntegrationAction;
	QAction* addInterpolationAction;
	QAction* addSmoothAction;
	QVector<QAction*> addFitAction;
	QAction* addFourierFilterAction;

	//Menus
	QMenu* m_selectionMenu{nullptr};
	QMenu* m_formattingMenu{nullptr};
	QMenu* m_columnMenu{nullptr};
	QMenu* m_columnSetAsMenu{nullptr};
	QMenu* m_columnGenerateDataMenu{nullptr};
	QMenu* m_columnManipulateDataMenu;
	QMenu* m_columnNormalizeMenu{nullptr};
	QMenu* m_columnLadderOfPowersMenu{nullptr};
	QMenu* m_columnSortMenu{nullptr};
	QMenu* m_rowMenu{nullptr};
	QMenu* m_spreadsheetMenu{nullptr};
	QMenu* m_plotDataMenu{nullptr};
	QMenu* m_analyzePlotMenu{nullptr};

public Q_SLOTS:
	void createContextMenu(QMenu*);
	void fillToolBar(QToolBar*);
#ifdef HAVE_TOUCHBAR
	void fillTouchBar(KDMacTouchBar*);
#endif
	void print(QPrinter*) const;
	void pasteIntoSelection();

private Q_SLOTS:
	void createColumnContextMenu(QMenu*);
	void goToCell(int row, int col);
	void showSearch();
	void toggleComments();
	void goToNextColumn();
	void goToPreviousColumn();
	void goToCell();
	void sortSpreadsheet();
	void sortDialog(const QVector<Column*>&);
	void searchTextChanged(const QString&);
	void searchReturnPressed();
	void formatHeatmap();
	void removeFormat();

	void cutSelection();
	void copySelection();
	void clearSelectedCells();
	void maskSelection();
	void unmaskSelection();
// 		void recalculateSelectedCells();

	void plotData();
	void plotAnalysisData();

	void fillSelectedCellsWithRowNumbers();
	void fillWithRowNumbers();
	void fillSelectedCellsWithRandomNumbers();
	void fillWithRandomValues();
	void fillWithEquidistantValues();
	void fillWithFunctionValues();
	void fillSelectedCellsWithConstValues();

	void insertRowAbove();
	void insertRowBelow();
	void insertRowsAbove();
	void insertRowsBelow();
	void removeSelectedRows();
	void clearSelectedRows();

	void insertColumnLeft();
	void insertColumnRight();
	void insertColumnsLeft();
	void insertColumnsRight();
	void removeSelectedColumns();
	void clearSelectedColumns();
	void toggleFreezeColumn();

	void modifyValues();
	void reverseColumns();
	void dropColumnValues();
	void maskColumnValues();
// 	void joinColumns();
	void normalizeSelectedColumns(QAction*);
	void powerTransformSelectedColumns(QAction*);

	void sortSelectedColumns();
	void sortColumnAscending();
	void sortColumnDescending();

	void setSelectionAs();

	void activateFormulaMode(bool on);

	void showColumnStatistics(bool forAll = false);
	void showAllColumnsStatistics();
	void showRowStatistics();

	void handleHorizontalSectionResized(int logicalIndex, int oldSize, int newSize);
	void handleHorizontalSectionMoved(int index, int from, int to);
	void handleHorizontalHeaderDoubleClicked(int index);
	void handleHeaderDataChanged(Qt::Orientation, int first, int last);
	void currentColumnChanged(const QModelIndex& current, const QModelIndex & previous);
	void handleAspectAdded(const AbstractAspect*);
	void handleAspectAboutToBeRemoved(const AbstractAspect*);
	void updateHeaderGeometry(Qt::Orientation, int first, int last);

	void selectColumn(int);
	void deselectColumn(int);
	void columnClicked(int);
	void selectionChanged(const QItemSelection& selected, const QItemSelection& deselected);
	void advanceCell();
};

#endif
