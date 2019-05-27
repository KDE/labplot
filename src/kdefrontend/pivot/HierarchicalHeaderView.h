/* Copyright 2016 Lee Cho Kang.
* email: pzesseto@gmail.com
* This file is part of the HierarchicalHeaderView.
*
* The HierarchicalHeaderView is free software: you can redistribute it
* and/or modify it under the terms of the GNU General Public License as
* published by the Free Software Foundation, either version 3 of the
* License, or (at your option) any later version.
*
* The HierarchicalHeaderView is distributed in the hope that it will be
* useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
* Public License for more details.
*
* You should have received a copy of the GNU General Public License along
* with the HierarchicalHeaderView. If not, see http://www.gnu.org/licenses/.
*/
/*
 * HierarchicalHeaderView.h
 *  Created on: 2016. 6. 13.
 */

#ifndef HIERARCHICALHEADERVIEW_H
#define HIERARCHICALHEADERVIEW_H

#include <QHeaderView>
#include <QModelIndex>


#include <QAbstractTableModel>
#include <QHash>
#include <QPair>

enum eRbHeaderRole {
    COLUMN_SPAN_ROLE = Qt::UserRole+1,
    ROW_SPAN_ROLE,
    COLUMN_SIZE_ROLE,
    ROW_SIZE_ROLE,
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
	QVariant data(int role=Qt::UserRole+1) const;

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
 *
 *                              MODEL DECLARATIONS
 * *************************************************************************************************/

class HierarchicalHeaderModel: public QAbstractTableModel {
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
	int rowCount(const QModelIndex &parent=QModelIndex()) const override;
	int columnCount(const QModelIndex &parent=QModelIndex()) const override;
	QVariant data(const QModelIndex &index, int role) const override;
	bool setData(const QModelIndex & index, const QVariant & value, int role = Qt::EditRole) override;
	Qt::ItemFlags flags(const QModelIndex &index) const override;
	void clear();

    void setBaseSectionSize(QSize size);
    void setOrientation(Qt::Orientation orient);
private:
	int m_rowCount{0};
	int m_columnCount{0};
	HierarchicalHeaderItem* m_rootItem{nullptr};

    int *maxWidthArr;
    QSize baseSectionSize;
    Qt::Orientation orientation;
};

/****************************************************************************************************
 *
 *                              VIEW DECLARATIONS
 * *************************************************************************************************/

class HierarchicalHeaderView : public QHeaderView {
	Q_OBJECT
public:
	HierarchicalHeaderView(Qt::Orientation orientation, QWidget* parent = nullptr);
	HierarchicalHeaderView(Qt::Orientation orientation, int rows, int columns, QWidget* parent = nullptr);
	~HierarchicalHeaderView() override;

	HierarchicalHeaderModel* hierarchicalModel() const;
	void setRowHeight(int row, int height);
	void setColumnWidth(int col, int width);
	void setCellBackgroundColor(const QModelIndex&, const QColor&);
	void setCellForegroundColor(const QModelIndex&, const QColor&);

    QSize getBaseSectionSize() const;
    void setNewModel(HierarchicalHeaderModel* model);

protected:
	void mousePressEvent(QMouseEvent*) override;
	QModelIndex indexAt(const QPoint&) const override;
	void paintSection(QPainter* painter, const QRect& rect, int logicalIndex) const override;
	QSize sectionSizeFromContents(int logicalIndex) const override;

	QModelIndex columnSpanIndex(const QModelIndex&) const;
	QModelIndex rowSpanIndex(const QModelIndex&) const;
	int columnSpanSize(int row, int from, int spanCount) const;
	int rowSpanSize(int column, int from, int spanCount) const;
	int getSectionRange(QModelIndex& index, int* beginSection, int* endSection) const;

protected slots:
	void onSectionResized(int logicalIndex,int oldSize,int newSize);

signals:
	void sectionPressed(int from, int to);

private:
	HierarchicalHeaderModel* m_model{nullptr};
    QSize baseSectionSize;
};

#endif
