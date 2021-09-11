/*
    File                 : MatrixView.cpp
    Project              : LabPlot
    Description          : View class for Matrix
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2015-2021 Alexander Semke <alexander.semke@web.de>
    SPDX-FileCopyrightText: 2008-2009 Tilman Benkert <thzs@gmx.net>
    SPDX-License-Identifier: GPL-2.0-or-later
*/


#ifndef MATRIXVIEW_H
#define MATRIXVIEW_H

#include <QWidget>
#include <QLocale>

class Matrix;
class MatrixModel;

class QPrinter;
class QAction;
class QLabel;
class QMenu;
class QStackedWidget;
class QTableView;

class MatrixView : public QWidget {
    Q_OBJECT

public:
	explicit MatrixView(Matrix*);
	~MatrixView() override;

	MatrixModel* model() const;

	int selectedColumnCount(bool full = false) const;
	bool isColumnSelected(int col, bool full = false) const;
	int selectedRowCount(bool full = false) const;
	bool isRowSelected(int row, bool full = false) const;
	int firstSelectedColumn(bool full = false) const;
	int lastSelectedColumn(bool full = false) const;
	int firstSelectedRow(bool full = false) const;
	int lastSelectedRow(bool full = false) const;
	bool isCellSelected(int row, int col) const;
	void setCellSelected(int row, int col);
	void setCellsSelected(int first_row, int first_col, int last_row, int last_col);
	void getCurrentCell(int* row, int* col) const;

	void resizeHeaders();
	void adjustHeaders();
	void exportToFile(const QString& path, const QString& separator, QLocale::Language) const;
	void exportToLaTeX(const QString&, const bool verticalHeaders, const bool horizontalHeaders,
                           const bool latexHeaders, const bool gridLines,
                           const bool entire, const bool captions) const;
	void exportToFits(const QString& fileName, const int exportTo) const;

public slots:
	void createContextMenu(QMenu*);
	void print(QPrinter*) const;

private:
	void init();
	void initActions();
	void initMenus();
	void goToCell(int row, int col);
	void updateImage();

	bool eventFilter(QObject*, QEvent*) override;
	void keyPressEvent(QKeyEvent*) override;

	QStackedWidget* m_stackedWidget;
	QTableView* m_tableView;
	QLabel* m_imageLabel;
	Matrix* m_matrix;
	MatrixModel* m_model;
	QImage m_image;
	bool m_imageIsDirty{true};

	//Actions
	QAction* action_cut_selection;
	QAction* action_copy_selection;
	QAction* action_paste_into_selection;
	QAction* action_clear_selection;

	QAction* action_select_all;
	QAction* action_clear_matrix;
	QAction* action_go_to_cell;
	QAction* action_set_formula;
	QAction* action_recalculate;
	QAction* action_duplicate;
	QAction* action_transpose;
	QAction* action_mirror_vertically;
	QAction* action_mirror_horizontally;

	QAction* action_add_value;
	QAction* action_subtract_value;
	QAction* action_multiply_value;
	QAction* action_divide_value;

	QAction* action_header_format_1;
	QAction* action_header_format_2;
	QAction* action_header_format_3;

	QAction* action_insert_columns;
	QAction* action_remove_columns;
	QAction* action_clear_columns;
	QAction* action_add_columns;
	QAction* action_statistics_columns;

	QAction* action_insert_rows;
	QAction* action_remove_rows;
	QAction* action_clear_rows;
	QAction* action_add_rows;
	QAction* action_statistics_rows;

	QAction* action_data_view;
	QAction* action_image_view;
	QAction* action_fill_function;
	QAction* action_fill_const;

	//Menus
	QMenu* m_selectionMenu{nullptr};
	QMenu* m_columnMenu{nullptr};
	QMenu* m_rowMenu{nullptr};
	QMenu* m_matrixMenu{nullptr};
	QMenu* m_headerFormatMenu{nullptr};

private slots:
	void goToCell();
	void advanceCell();
	void handleHorizontalSectionResized(int logicalIndex, int oldSize, int newSize);
	void handleVerticalSectionResized(int logicalIndex, int oldSize, int newSize);

	void switchView(QAction*);
	void matrixDataChanged();

	void fillWithFunctionValues();
	void fillWithConstValues();

	void cutSelection();
	void copySelection();
	void pasteIntoSelection();
	void clearSelectedCells();

	void headerFormatChanged(QAction*);

	void modifyValues();

	void addColumns();
	void insertEmptyColumns();
	void removeSelectedColumns();
	void clearSelectedColumns();
	void addRows();
	void insertEmptyRows();
	void removeSelectedRows();
	void clearSelectedRows();

	void showColumnStatistics();
	void showRowStatistics();
};

#endif
