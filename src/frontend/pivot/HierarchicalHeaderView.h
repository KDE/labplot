/*
	File                 : HierarchicalHeaderView.h
	Project              : LabPlot
	Description          : Hierarchical header view
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2016 Lee Cho Kang <pzesseto@gmail.com>
	SPDX-FileCopyrightText: 2023-2025 Alexander Semke <alexander.semke@web.de>
	SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef HIERARCHICALHEADERVIEW_H
#define HIERARCHICALHEADERVIEW_H

#include <QHeaderView>
#include <QModelIndex>

#include <QAbstractTableModel>
#include <QHash>
#include <QPair>

enum eRbHeaderRole {
	COLUMN_SPAN_ROLE = Qt::UserRole + 1,
	ROW_SPAN_ROLE,
	COLUMN_SIZE_ROLE,
	ROW_SIZE_ROLE
};

class HierarchicalHeaderItem {

public:
	HierarchicalHeaderItem(HierarchicalHeaderItem* parent = nullptr);
	HierarchicalHeaderItem(int row, int column, HierarchicalHeaderItem* parent = nullptr);
	~HierarchicalHeaderItem();

	HierarchicalHeaderItem* insertChild(int row, int col);
	const HierarchicalHeaderItem* child(int row,int col) const;
	HierarchicalHeaderItem* child(int row,int col);

	void setData(const QVariant& data, int role);
	QVariant data(int role = Qt::UserRole + 1) const;

	int column() const;
	int row() const;
	HierarchicalHeaderItem* parent();
	void setText(const QString&);
	void clear();

private:
	int row_prop;
	int column_prop;
	HierarchicalHeaderItem* parent_item;
	QHash<QPair<int,int>,HierarchicalHeaderItem*> child_items;
	QHash<int,QVariant> role_datas;
};

/****************************************************************************************************
 *                              MODEL DECLARATIONS
 * *************************************************************************************************/
class HierarchicalHeaderModel : public QAbstractTableModel {
	Q_OBJECT

public:
	HierarchicalHeaderModel(QObject* parent = nullptr);
	HierarchicalHeaderModel(int rows, int cols, QObject* parent = nullptr);
	virtual ~HierarchicalHeaderModel();

public:
	void setRowCount(int);
	void setColumnCount(int);
	void setSpan(int row, int column, int rowSpanCount, int columnSpanCount);
	QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const override;
	int rowCount(const QModelIndex& parent = QModelIndex()) const override;
	int columnCount(const QModelIndex &parent = QModelIndex()) const override;
	QVariant data(const QModelIndex&, int role) const override;
	bool setData(const QModelIndex&, const QVariant& value, int role = Qt::EditRole) override;
	Qt::ItemFlags flags(const QModelIndex&) const override;
	void clear();

	void setBaseSectionSize(QSize);
	void setOrientation(Qt::Orientation);

private:
	int m_rowCount{0};
	int m_columnCount{0};
	HierarchicalHeaderItem* m_rootItem{nullptr};

	int *maxWidthArr{nullptr};
	QSize baseSectionSize;
	Qt::Orientation orientation{Qt::Horizontal};
};

/****************************************************************************************************
 *                              VIEW DECLARATIONS
 * *************************************************************************************************/
class HierarchicalHeaderView : public QHeaderView {
	Q_OBJECT

public:
	HierarchicalHeaderView(Qt::Orientation, QWidget* parent = nullptr);
	HierarchicalHeaderView(Qt::Orientation, int rows, int columns, QWidget* parent = nullptr);
	~HierarchicalHeaderView() override;

	void setRowHeight(int row, int height);
	void setColumnWidth(int col, int width);
	void setCellBackgroundColor(const QModelIndex&, const QColor&);
	void setCellForegroundColor(const QModelIndex&, const QColor&);

	QSize getBaseSectionSize() const;

	HierarchicalHeaderModel* hierarchicalModel() const;
	void setHierarchicalModel(HierarchicalHeaderModel*);

protected:
	void mousePressEvent(QMouseEvent*) override;
	QModelIndex indexAt(const QPoint&) const override;
	void paintSection(QPainter*, const QRect& rect, int logicalIndex) const override;
	QSize sectionSizeFromContents(int logicalIndex) const override;

	QModelIndex columnSpanIndex(const QModelIndex&) const;
	QModelIndex rowSpanIndex(const QModelIndex&) const;
	int columnSpanSize(int row, int from, int spanCount) const;
	int rowSpanSize(int column, int from, int spanCount) const;
	int getSectionRange(QModelIndex& index, int* beginSection, int* endSection) const;

protected Q_SLOTS:
	void onSectionResized(int logicalIndex,int oldSize,int newSize);

Q_SIGNALS:
	void sectionPressed(int from, int to);

private:
	HierarchicalHeaderModel* m_model{nullptr};
    QSize baseSectionSize;
};

#endif
