/***************************************************************************
    File                 : SpreadsheetView.h
    Project              : LabPlot
    Description          : View class for Spreadsheet
    --------------------------------------------------------------------
    Copyright            : (C) 2010-2015 by Alexander Semke (alexander.semke@web.de)

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

#ifndef SPREADSHEETVIEW_H
#define SPREADSHEETVIEW_H

#include <QWidget>

#include "backend/core/AbstractColumn.h"
#include "backend/lib/IntervalAttribute.h"

class Column;
class Spreadsheet;
class SpreadsheetModel;
class SpreadsheetItemDelegate;
class SpreadsheetHeaderView;
class AbstractAspect;
class QTableView;

class QPrinter;
class QMenu;
class QToolBar;
class QModelIndex;
class QItemSelection;

class SpreadsheetView : public QWidget {
	Q_OBJECT

public:
	explicit SpreadsheetView(Spreadsheet* spreadsheet, bool readOnly = false);
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
	void exportToFits(const QString &fileName, const int exportTo, const bool commentsAsUnits) const;

	QTableView* m_tableView;
	Spreadsheet* m_spreadsheet;
	SpreadsheetItemDelegate* m_delegate;
	SpreadsheetModel* m_model;
	SpreadsheetHeaderView* m_horizontalHeader;
	bool m_suppressSelectionChangedEvent;
	bool m_readOnly;
	bool eventFilter(QObject*, QEvent*) override;
	void checkSpreadsheetMenu();

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
	QAction* action_fill_sel_row_numbers;
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
	QAction* action_go_to_cell;
	QAction* action_statistics_all_columns;

	//column related actions
	QAction* action_insert_column_left;
	QAction* action_insert_column_right;
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
	QAction* action_drop_values;
	QAction* action_mask_values;
	QAction* action_join_columns;
	QAction* action_normalize_columns;
	QAction* action_normalize_selection;
	QAction* action_sort_columns;
	QAction* action_sort_asc_column;
	QAction* action_sort_desc_column;
	QAction* action_statistics_columns;

	//row related actions
	QAction* action_insert_row_above;
	QAction* action_insert_row_below;
	QAction* action_remove_rows;
	QAction* action_clear_rows;
	QAction* action_statistics_rows;

	//analysis and plot data menu actions
	QAction* action_plot_data_xycurve;
	QAction* action_plot_data_histogram;
	QAction* addDataOperationAction;
	QAction* addDataReductionAction;
	QAction* addDifferentiationAction;
	QAction* addIntegrationAction;
	QAction* addInterpolationAction;
	QAction* addSmoothAction;
	QVector<QAction*> addFitAction;
	QAction* addFourierFilterAction;

	//Menus
	QMenu* m_selectionMenu;
	QMenu* m_columnMenu;
	QMenu* m_columnSetAsMenu;
	QMenu* m_columnGenerateDataMenu;
	QMenu* m_columnManipulateDataMenu;
	QMenu* m_columnSortMenu;
	QMenu* m_rowMenu;
	QMenu* m_spreadsheetMenu;
	QMenu* m_plotDataMenu;
	QMenu* m_analyzePlotMenu;

public slots:
	void createContextMenu(QMenu*);
	void fillToolBar(QToolBar*);
	void print(QPrinter*) const;

private slots:
	void createColumnContextMenu(QMenu*);
	void goToCell(int row, int col);
	void toggleComments();
	void goToNextColumn();
	void goToPreviousColumn();
	void goToCell();
	void sortSpreadsheet();
	void sortDialog(QVector<Column*>);

	void cutSelection();
	void copySelection();
	void pasteIntoSelection();
	void clearSelectedCells();
	void maskSelection();
	void unmaskSelection();
// 		void recalculateSelectedCells();

	void plotData();

	void fillSelectedCellsWithRowNumbers();
	void fillWithRowNumbers();
	void fillSelectedCellsWithRandomNumbers();
	void fillWithRandomValues();
	void fillWithEquidistantValues();
	void fillWithFunctionValues();
	void fillSelectedCellsWithConstValues();

	void insertRowAbove();
	void insertRowBelow();
	void removeSelectedRows();
	void clearSelectedRows();

	void insertColumnLeft();
	void insertColumnRight();
	void removeSelectedColumns();
	void clearSelectedColumns();

	void addValue();
	void subtractValue();
	void reverseColumns();
	void dropColumnValues();
	void maskColumnValues();
	void joinColumns();
	void normalizeSelectedColumns();
	void normalizeSelection();

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
	void handleHeaderDataChanged(Qt::Orientation orientation, int first, int last);
	void currentColumnChanged(const QModelIndex& current, const QModelIndex & previous);
	void handleAspectAdded(const AbstractAspect*);
	void handleAspectAboutToBeRemoved(const AbstractAspect*);
	void updateHeaderGeometry(Qt::Orientation o, int first, int last);

	void selectColumn(int);
	void deselectColumn(int);
	void columnClicked(int);
	void selectionChanged(const QItemSelection& selected, const QItemSelection& deselected);
};

#endif
