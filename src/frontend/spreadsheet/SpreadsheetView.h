/*
	File                 : SpreadsheetView.h
	Project              : LabPlot
	Description          : View class for Spreadsheet
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2010-2025 Alexander Semke <alexander.semke@web.de>
	SPDX-FileCopyrightText: 2023 Stefan Gerlach <stefan.gerlach@uni.kn>
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
#ifndef SDK
class SearchReplaceWidget;
#endif
class Spreadsheet;
class SpreadsheetHeaderView;
class SpreadsheetModel;

class QActionGroup;
class QFrame;
class QItemSelection;
class QItemSelectionModel;
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
	friend class SpreadsheetFormulaTest;
	friend class ImportSqlDatabaseTest;

public:
	explicit SpreadsheetView(Spreadsheet*, bool readOnly = false);
	~SpreadsheetView() override;

	bool isReadOnly() const;
	void resizeHeader();
	void setFocus();
	void setSuppressResizeHeader(bool);

	void createContextMenu(QMenu*);
	void fillColumnContextMenu(QMenu*, Column*);
	void fillColumnsContextMenu(QMenu*);

	void showComments(bool on = true);
	void showSparklines(bool on = true);

	int selectedColumnCount(bool full = true) const;
	int selectedColumnCount(AbstractColumn::PlotDesignation) const;
	bool isColumnSelected(int col, bool full = false) const;
	QVector<Column*> selectedColumns(bool full = true) const;
	int firstSelectedColumn(bool full = false) const;
	int lastSelectedColumn(bool full = false) const;

	int selectedRowCount(bool full = true) const;
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
	void exportToFile(const QString&, bool, const QString&, QLocale::Language) const;
	void exportToLaTeX(const QString&, bool exportHeaders, bool gridLines, bool captions, bool latexHeaders, bool skipEmptyRows, bool exportEntire) const;
	void exportToFits(const QString& path, int exportTo, bool commentsAsUnits) const;
	void exportToXLSX(const QString& path, bool exportHeaders) const;
	void exportToMCAP(const QString& path, int compression_mode, int compression_level) const;
	bool hasValues(const QVector<Column*>);

	void insertColumnsLeft(int);
	void insertColumnsRight(int);

	void insertRowsAbove(int);
	void insertRowsBelow(int);

	void updateFrozenTableGeometry();

	QItemSelectionModel* selectionModel();

	QTableView* m_tableView{nullptr};
	QTableView* m_frozenTableView{nullptr};
	int m_selectedColumnFromContextMenu{-1};
	bool m_editorEntered{false};
	Spreadsheet* m_spreadsheet;
	SpreadsheetModel* m_model;
	SpreadsheetHeaderView* m_horizontalHeader;
#ifndef SDK
	SearchReplaceWidget* m_searchReplaceWidget{nullptr};
#endif
	bool m_suppressSelectionChangedEvent{false};
	bool m_readOnly;
	bool m_suppressResizeHeader{false};

	bool eventFilter(QObject*, QEvent*) override;
	void checkSpreadsheetMenu();
	void checkSpreadsheetSelectionMenu();
	void checkColumnMenus(const QVector<Column*>&);
	void showSearchReplace(bool replace);

	// selection related actions
	QAction* action_cut_selection{nullptr};
	QAction* action_copy_selection{nullptr};
	QAction* action_paste_into_selection{nullptr};
	QAction* action_mask_selection{nullptr};
	QAction* action_unmask_selection{nullptr};
	QAction* action_clear_selection{nullptr};
	QAction* action_reverse_selection{nullptr};
	QAction* action_fill_row_numbers{nullptr};
	QAction* action_fill_random{nullptr};
	QAction* action_fill_equidistant{nullptr};
	QAction* action_fill_random_nonuniform{nullptr};
	QAction* action_fill_const{nullptr};
	QAction* action_fill_function{nullptr};

	// spreadsheet related actions
	QAction* action_toggle_comments{nullptr};
	QAction* action_toggle_sparklines{nullptr};
	QAction* action_select_all{nullptr};
	QAction* action_clear_spreadsheet{nullptr};
	QAction* action_clear_masks{nullptr};
	QAction* action_formatting_heatmap{nullptr};
	QAction* action_formatting_remove{nullptr};
	QAction* action_go_to_cell{nullptr};
	QAction* action_search{nullptr};
	QAction* action_search_replace{nullptr};
	QAction* action_statistics_all_columns{nullptr};
	QAction* action_statistics_spreadsheet{nullptr};
	QAction* action_pivot_table{nullptr};

	// column related actions
	QAction* action_insert_column_left{nullptr};
	QAction* action_insert_column_right{nullptr};
	QAction* action_insert_columns_left{nullptr};
	QAction* action_insert_columns_right{nullptr};
	QAction* action_remove_columns{nullptr};
	QAction* action_clear_columns{nullptr};
	QAction* action_add_columns{nullptr};
	QAction* action_set_as_none{nullptr};
	QAction* action_set_as_x{nullptr};
	QAction* action_set_as_y{nullptr};
	QAction* action_set_as_z{nullptr};
	QAction* action_set_as_xerr{nullptr};
	QAction* action_set_as_xerr_plus{nullptr};
	QAction* action_set_as_xerr_minus{nullptr};
	QAction* action_set_as_yerr{nullptr};
	QAction* action_set_as_yerr_plus{nullptr};
	QAction* action_set_as_yerr_minus{nullptr};
	QAction* action_reverse_columns{nullptr};
	QAction* action_add_value{nullptr};
	QAction* action_subtract_value{nullptr};
	QAction* action_subtract_baseline{nullptr};
	QAction* action_multiply_value{nullptr};
	QAction* action_divide_value{nullptr};
	QAction* action_drop_values{nullptr};
	QAction* action_mask_values{nullptr};
	QAction* action_sample_values{nullptr};
	QAction* action_flatten_columns{nullptr};
	QAction* action_join_columns{nullptr};
	QActionGroup* normalizeColumnActionGroup{nullptr};
	QActionGroup* ladderOfPowersActionGroup{nullptr};
	QAction* action_sort{nullptr};
	QAction* action_sort_asc{nullptr};
	QAction* action_sort_desc{nullptr};
	QAction* action_statistics_columns{nullptr};
	QAction* action_freeze_columns{nullptr};

	// row related actions
	QAction* action_insert_row_above{nullptr};
	QAction* action_insert_row_below{nullptr};
	QAction* action_insert_rows_above{nullptr};
	QAction* action_insert_rows_below{nullptr};
	QAction* action_remove_rows{nullptr};
	QAction* action_remove_missing_value_rows{nullptr};
	QAction* action_mask_missing_value_rows{nullptr};
	QAction* action_statistics_rows{nullptr};

	// analysis and plot data menu actions
	QActionGroup* plotDataActionGroup{nullptr};
	QAction* addDataOperationAction{nullptr};
	QAction* addDataReductionAction{nullptr};
	QAction* addDifferentiationAction{nullptr};
	QAction* addIntegrationAction{nullptr};
	QAction* addInterpolationAction{nullptr};
	QAction* addSmoothAction{nullptr};
	QAction* addFourierFilterAction{nullptr};
	QActionGroup* addAnalysisActionGroup{nullptr};
	QActionGroup* addFitActionGroup{nullptr};
	QActionGroup* addDistributionFitActionGroup{nullptr};

	// Menus
	QMenu* m_selectionMenu{nullptr};
	QMenu* m_formattingMenu{nullptr};
	QMenu* m_columnMenu{nullptr};
	QMenu* m_columnSetAsMenu{nullptr};
	QMenu* m_columnGenerateDataMenu{nullptr};
	QMenu* m_columnManipulateDataMenu{nullptr};
	QMenu* m_columnNormalizeMenu{nullptr};
	QMenu* m_columnLadderOfPowersMenu{nullptr};
	QMenu* m_rowMenu{nullptr};
	QMenu* m_spreadsheetMenu{nullptr};
	QMenu* m_plotDataMenu{nullptr};
	QMenu* m_analyzePlotMenu{nullptr};

	bool m_suppressResize{false};

public Q_SLOTS:
	void handleAspectsAdded(int first, int last);

#ifdef HAVE_TOUCHBAR
	void fillTouchBar(KDMacTouchBar*);
#endif
	void print(QPrinter*) const;

	void pasteIntoSelection();

	void fillWithRowNumbers();
	void selectAll();
	void selectColumn(int);
	void deselectColumn(int);
	void goToCell(int row, int col);
	void selectCell(int row, int col);
	void clearSelection();

	// public slots that are also used in the toolbar
	void insertRowAbove();
	void insertRowBelow();
	void removeSelectedRows();
	void insertColumnLeft();
	void insertColumnRight();
	void removeSelectedColumns();
	void sortCustom();
	void sortAscending();
	void sortDescending();

private Q_SLOTS:
	void searchReplace();
	void toggleComments();
	void toggleSparklines();
	void goToNextColumn();
	void goToPreviousColumn();
	void goToCell();
	void createPivotTable();
	void formatHeatmap();
	void removeFormat();
	void cutSelection();
	void copySelection();
	void clearSelectedCells();
	void maskSelection();
	void unmaskSelection();
	void reverseSelection();

	void plotData(QAction*);
	void plotAnalysisData(QAction*);
	void plotDataDistributionFit(QAction*);

	void fillSelectedCellsWithRowNumbers();
	void fillSelectedCellsWithRandomNumbers();
	void fillWithRandomValues();
	void fillWithEquidistantValues();
	void fillWithFunctionValues();
	void fillSelectedCellsWithConstValues();

	void insertRowsAbove();
	void insertRowsBelow();

	void insertColumnsLeft();
	void insertColumnsRight();
	void clearSelectedColumns();
	void toggleFreezeColumn();

	void modifyValues();
	void reverseColumns();
	void dropColumnValues();
	void maskColumnValues();
	void sampleColumnValues();
	void flattenColumns();
	void normalizeSelectedColumns(QAction*);
	void powerTransformSelectedColumns(QAction*);

	void setSelectionAs();
	void activateFormulaMode(bool on);

	void showColumnStatistics(bool forAll = false);
	void showAllColumnsStatistics();
	void showRowStatistics();

	void handleHorizontalSectionResized(int logicalIndex, int oldSize, int newSize);
	void handleHorizontalSectionMoved(int index, int from, int to);
	void handleHorizontalHeaderDoubleClicked(int index);
	void handleHeaderDataChanged(Qt::Orientation, int first, int last);
	void handleAspectAboutToBeRemoved(int first, int last);
	void updateHeaderGeometry(Qt::Orientation, int first, int last);

	void columnClicked(int);
	void selectionChanged(const QItemSelection& selected, const QItemSelection& deselected);
	void advanceCell();
};

#endif
