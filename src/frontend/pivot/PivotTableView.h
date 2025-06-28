/***************************************************************************
	File                 : PivotTableView.h
	Project              : LabPlot
	Description          : View class for PivotTable
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2019-2025 Alexander Semke <alexander.semke@web.de>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef PIVOTTABLEVIEW_H
#define PIVOTTABLEVIEW_H

#include <QLocale>
#include <QWidget>

class PivotTable;
class HierarchicalHeaderView;
class QMenu;
class QPrinter;
class QTableView;

class PivotTableView : public QWidget {
	Q_OBJECT

public:
	explicit PivotTableView(PivotTable*, bool readOnly = false);
	~PivotTableView() override;

	bool exportView();
	bool printView();
	bool printPreview();

private:
	void init();
	void initActions();
	void initMenus();
	void connectActions();

	void exportToFile(const QString&, const bool, const QString&, QLocale::Language) const;
	void exportToLaTeX(const QString&, const bool exportHeaders,
	                   const bool gridLines, const bool captions, const bool latexHeaders,
	                   const bool skipEmptyRows,const bool exportEntire) const;

	PivotTable* m_pivotTable{nullptr};
	QTableView* m_tableView{nullptr};
	HierarchicalHeaderView* m_horizontalHeaderView{nullptr};
	HierarchicalHeaderView* m_verticalHeaderView{nullptr};
	bool m_readOnly{false};

public Q_SLOTS:
	void createContextMenu(QMenu*);
	void print(QPrinter*) const;
	void changed();

private Q_SLOTS:
	void goToCell();
	void goToCell(int row, int col);
};

#endif
