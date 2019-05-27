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

#include "HierarchicalHeaderView.h"
#include <QPainter>
#include <QStandardItem>
#include <QMouseEvent>

#include <QBrush>
#include <QVariant>
#include <qdrawutil.h>

#include <QDebug>

HierarchicalHeaderItem::HierarchicalHeaderItem(HierarchicalHeaderItem* parent):
	row_prop(0),column_prop(0),parent_item(parent) {
}

HierarchicalHeaderItem::HierarchicalHeaderItem(int arow, int acolumn, HierarchicalHeaderItem* parent):
	row_prop(arow),column_prop(acolumn),parent_item(parent) {
}

HierarchicalHeaderItem::~HierarchicalHeaderItem() = default;

HierarchicalHeaderItem* HierarchicalHeaderItem::insertChild(int row, int col) {
	HierarchicalHeaderItem* newChild = new HierarchicalHeaderItem(row,col,this);
	child_items.insert(QPair<int,int>(row,col),newChild);
	return newChild;
}

const HierarchicalHeaderItem* HierarchicalHeaderItem::child(int row,int col) const {
	QHash<QPair<int,int>,HierarchicalHeaderItem*>::const_iterator itr = child_items.find(QPair<int,int>(row,col));
	if (itr != child_items.end()) return itr.value();
    return nullptr;
}

HierarchicalHeaderItem* HierarchicalHeaderItem::child(int row,int col) {
	QHash<QPair<int,int>,HierarchicalHeaderItem*>::iterator itr = child_items.find(QPair<int,int>(row,col));
	if (itr != child_items.end()) return itr.value();
    return nullptr;
}

void HierarchicalHeaderItem::setText(const QString& text) {
	role_datas.insert(Qt::DisplayRole,text);
}

QVariant HierarchicalHeaderItem::data(int role) const {
	QHash<int,QVariant>::const_iterator itr = role_datas.find(role);
	if (itr != role_datas.end()) return itr.value();
	return QVariant();
}

void HierarchicalHeaderItem::setData(const QVariant& data, int role) {
	role_datas.insert(role,data);
}

int HierarchicalHeaderItem::column() const {
	return column_prop;
}

int HierarchicalHeaderItem::row() const {
	return row_prop;
}

HierarchicalHeaderItem* HierarchicalHeaderItem::parent() {
	return parent_item;
}

void HierarchicalHeaderItem::clear() {
	QList<HierarchicalHeaderItem*> items = child_items.values();
	foreach (HierarchicalHeaderItem* item, child_items) {
        if (item) {
            delete item;
        }
	}
	child_items.clear();
}

/**********************************************************************************************
 *                                    MODEL IMPLEMENTATION
 *
 * ********************************************************************************************/
HierarchicalHeaderModel::HierarchicalHeaderModel(QObject* parent) : QAbstractTableModel(parent),
	m_rootItem(new HierarchicalHeaderItem()) {
}
HierarchicalHeaderModel::HierarchicalHeaderModel(int rows, int cols, QObject* parent) :
	QAbstractTableModel(parent),m_rowCount(rows),m_columnCount(cols),m_rootItem(new HierarchicalHeaderItem()) {
    maxWidthArr = new int[m_columnCount];
       for(int col=0; col<m_columnCount; col++)
           maxWidthArr[col] = 50;
}

HierarchicalHeaderModel::~HierarchicalHeaderModel() {
	m_rootItem->clear();
	delete m_rootItem;
    delete maxWidthArr;
}

void HierarchicalHeaderModel::setBaseSectionSize(QSize size)
{

    baseSectionSize = size;

    if(orientation == Qt::Vertical){
        for (int row=0;row<m_rowCount;++row)
            for (int col=0;col<m_columnCount;++col)
            {
                baseSectionSize.setWidth(maxWidthArr[col]);
                this->setData(this->index(row,col),baseSectionSize,Qt::SizeHintRole);
            }
        return;
    }

    for (int row=0;row<m_rowCount;++row)
        for (int col=0;col<m_columnCount;++col)
        {
            this->setData(this->index(row,col),baseSectionSize,Qt::SizeHintRole);
        }
}

void HierarchicalHeaderModel::setOrientation(Qt::Orientation orient)
{
    orientation = orient;
}

QModelIndex HierarchicalHeaderModel::index(int row, int column, const QModelIndex & parent) const {
	if (!hasIndex(row,column,parent)) return QModelIndex();

	HierarchicalHeaderItem* parentItem;
	if (!parent.isValid()) parentItem = m_rootItem; // parent item is always the m_rootItem on table model
	else parentItem = static_cast<HierarchicalHeaderItem*>(parent.internalPointer()); // no effect

	HierarchicalHeaderItem* childItem = parentItem->child(row,column);
	if (!childItem) childItem = parentItem->insertChild(row,column);
	return createIndex(row,column,childItem);

}

void HierarchicalHeaderModel::setRowCount(int count) {
	m_rowCount = count;

// 		if (horizontalHeaderModel->rowCount() > 1)
// 			horizontalHeaderModel->removeRows(1, horizontalHeaderModel->rowCount() - 1);
// 		else if (horizontalHeaderModel->rowCount() < 1)
// 			horizontalHeaderModel->insertRows(0, 1);
}

void HierarchicalHeaderModel::setColumnCount(int count) {
	m_columnCount = count;

    maxWidthArr = new int[m_columnCount];
    for(int col=0; col<m_columnCount; col++)
       maxWidthArr[col] = 50;
}

void HierarchicalHeaderModel::setSpan(int row, int column, int rowSpanCount, int columnSpanCount) {
	QModelIndex index = this->index(row, column);
	if (rowSpanCount > 0)
		setData(index, rowSpanCount, ROW_SPAN_ROLE);
	if (columnSpanCount)
		setData(index, columnSpanCount, COLUMN_SPAN_ROLE);
}

int HierarchicalHeaderModel::rowCount(const QModelIndex& parent) const {
	Q_UNUSED(parent);
	return m_rowCount;
}

int HierarchicalHeaderModel::columnCount(const QModelIndex& parent) const {
	Q_UNUSED(parent);
	return m_columnCount;
}

QVariant HierarchicalHeaderModel::data(const QModelIndex& index, int role) const {
	if (!index.isValid())
		return QVariant();

	if (index.row() >= m_rowCount || index.row() < 0 || index.column() >= m_columnCount || index.column() < 0)
		return QVariant();

	HierarchicalHeaderItem* item = static_cast<HierarchicalHeaderItem*>(index.internalPointer());

	return item->data(role);
}

bool HierarchicalHeaderModel::setData(const QModelIndex & index, const QVariant & value, int role) {
	if (index.isValid()) {
		HierarchicalHeaderItem* item = static_cast<HierarchicalHeaderItem*>(index.internalPointer());
		if (role == COLUMN_SPAN_ROLE) {
			int col = index.column();
			int span = value.toInt();
			if (span > 0) { // span size should be more than 1, else nothing to do
				if (col+span-1 >= m_columnCount) // span size should be less than whole columns,
					span = m_columnCount-col;
				item->setData(span,COLUMN_SPAN_ROLE);
			}
		} else if (role == ROW_SPAN_ROLE) {
			int row = index.row();
			int span = value.toInt();
			if (span > 0) { // span size should be more than 1, else nothing to do
				if (row+span-1 >= m_rowCount)
					span = m_rowCount-row;
				item->setData(span,ROW_SPAN_ROLE);
			}
        }
        else if (role == Qt::DisplayRole || role == Qt::EditRole){
            item->setData(value, role);
            if(orientation == Qt::Vertical)
            {
                int width = value.toString().length()*10;
                int col = index.column();
                if(width > maxWidthArr[col])
                    maxWidthArr[col] = width;
            }
         }
        else
			item->setData(value,role);

		return true;
	}
	return false;
}

Qt::ItemFlags HierarchicalHeaderModel::flags(const QModelIndex &index) const {
	if (!index.isValid())
		return Qt::ItemIsEnabled;
	return QAbstractTableModel::flags(index) | Qt::ItemIsEditable;
}

void HierarchicalHeaderModel::clear() {
//    setRowCount(0);
//    setColumnCount(0);
//    m_rootItem->clear();
}

/**********************************************************************************************
 *                                    VIEW IMPLEMENTATION
 *
 * ********************************************************************************************/
HierarchicalHeaderView::HierarchicalHeaderView(Qt::Orientation orientation, int rows, int columns, QWidget* parent):
	QHeaderView(orientation,parent) {
	QSize baseSectionSize;
	if (orientation == Qt::Horizontal) {
		baseSectionSize.setWidth(defaultSectionSize());
		baseSectionSize.setHeight(20);
	} else {
		baseSectionSize.setWidth(50);
		baseSectionSize.setHeight(defaultSectionSize());
	}

	// create header model
	HierarchicalHeaderModel* headerModel = new HierarchicalHeaderModel(rows,columns);

	// set default size of item
	for (int row=0; row<rows; ++row)
		for (int col=0; col<columns; ++col)
			headerModel->setData(headerModel->index(row,col),baseSectionSize,Qt::SizeHintRole);

	setModel(headerModel);

	connect(this, SIGNAL(sectionResized(int, int, int)), this, SLOT(onSectionResized(int,int,int)));
}

HierarchicalHeaderView::HierarchicalHeaderView(Qt::Orientation orientation, QWidget* parent):
	QHeaderView(orientation,parent) {
	if (orientation == Qt::Horizontal) {
		baseSectionSize.setWidth(defaultSectionSize());
		baseSectionSize.setHeight(20);
	} else {
		baseSectionSize.setWidth(50);
		baseSectionSize.setHeight(defaultSectionSize());
	}

	m_model = new HierarchicalHeaderModel();
	setModel(m_model);

	connect(this, SIGNAL(sectionResized(int, int, int)), this, SLOT(onSectionResized(int,int,int)));
}

HierarchicalHeaderView::~HierarchicalHeaderView() = default;


QSize HierarchicalHeaderView::getBaseSectionSize() const
{
    return baseSectionSize;
}

void HierarchicalHeaderView::setNewModel(HierarchicalHeaderModel* model)
{
    m_model = model;
    setModel(m_model);
}

HierarchicalHeaderModel* HierarchicalHeaderView::hierarchicalModel() const {
	return m_model;
}



void HierarchicalHeaderView::setRowHeight(int row, int rowHeight) {
	const int cols = m_model->columnCount();
	for (int col = 0; col < cols; ++col) {
		QSize size = m_model->index(row,col).data(Qt::SizeHintRole).toSize();
		size.setHeight(rowHeight);
		m_model->setData(m_model->index(row, col), size, Qt::SizeHintRole);
	}

	if (orientation() == Qt::Vertical)
		resizeSection(row, rowHeight);
}

void HierarchicalHeaderView::setColumnWidth(int col, int colWidth) {
	const int rows = m_model->rowCount();
	for (int row = 0; row < rows; ++row) {
		QSize size = m_model->index(row,col).data(Qt::SizeHintRole).toSize();
		size.setWidth(colWidth);
		m_model->setData(m_model->index(row,col), size, Qt::SizeHintRole);
	}

	if (orientation() == Qt::Horizontal)
		resizeSection(col, colWidth);
}

void HierarchicalHeaderView::setCellBackgroundColor(const QModelIndex& index, const QColor& color) {
	m_model->setData(index, color, Qt::BackgroundRole);
}

void HierarchicalHeaderView::setCellForegroundColor(const QModelIndex& index, const QColor& color) {
	m_model->setData(index, color, Qt::ForegroundRole);
}

void HierarchicalHeaderView::mousePressEvent(QMouseEvent* event) {
	QHeaderView::mousePressEvent(event);
	QPoint pos = event->pos();
	QModelIndex index = indexAt(pos);
	const int OTN = orientation();
	if (index.isValid()) {
		int beginSection = -1;
		int endSection   = -1;
		int numbers = 0;
		numbers = getSectionRange(index,&beginSection,&endSection);
		if (numbers > 0) {
			emit sectionPressed(beginSection,endSection);
			return;
		} else {
			const int LEVEL_CNT = (OTN == Qt::Horizontal)?m_model->rowCount():m_model->columnCount();
			int logicalIdx = (OTN == Qt::Horizontal)?index.column():index.row();
			int curLevel   = (OTN == Qt::Horizontal)?index.row():index.column();
			for (int i=0; i<LEVEL_CNT; ++i) {
				QModelIndex cellIndex = (OTN == Qt::Horizontal)?m_model->index(i,logicalIdx):m_model->index(logicalIdx,i);
				numbers = getSectionRange(cellIndex,&beginSection,&endSection);
				if (numbers > 0) {
					if (beginSection <= logicalIdx && logicalIdx <= endSection) {
						int beginLevel = (OTN == Qt::Horizontal)?cellIndex.row():cellIndex.column();
						QVariant levelSpanCnt = cellIndex.data((OTN == Qt::Horizontal)?ROW_SPAN_ROLE:COLUMN_SPAN_ROLE);
						if (!levelSpanCnt.isValid())
							continue;
						int endLevel   = beginLevel + levelSpanCnt.toInt()-1;
						if (beginLevel <= curLevel && curLevel <= endLevel) {
							emit sectionPressed(beginSection,endSection);
							break;
						}
					}
				}
			}
		}
	}
}

QModelIndex HierarchicalHeaderView::indexAt(const QPoint& pos) const {
	const int OTN = orientation();
	const int ROWS = m_model->rowCount();
	const int COLS = m_model->columnCount();
	int logicalIdx = logicalIndexAt(pos);

	if (OTN == Qt::Horizontal) {
		int dY=0;
		for (int row=0; row<ROWS; ++row) {
			QModelIndex cellIndex = m_model->index(row,logicalIdx);
			dY += cellIndex.data(Qt::SizeHintRole).toSize().height();
			if (pos.y() <= dY) return cellIndex;
		}
	} else {
		int dX=0;
		for (int col=0; col<COLS; ++col) {
			QModelIndex cellIndex = m_model->index(logicalIdx,col);
			dX += cellIndex.data(Qt::SizeHintRole).toSize().width();
			if (pos.x() <= dX) return cellIndex;
		}
	}

	return QModelIndex();
}

void HierarchicalHeaderView::paintSection(QPainter* painter, const QRect& rect, int logicalIdx) const {
	const int OTN = orientation();
	const int LEVEL_CNT = (OTN == Qt::Horizontal)?m_model->rowCount():m_model->columnCount();
	for (int i=0; i<LEVEL_CNT; ++i) {
		QModelIndex cellIndex = (OTN == Qt::Horizontal)?m_model->index(i,logicalIdx):m_model->index(logicalIdx,i);
		QSize cellSize=cellIndex.data(Qt::SizeHintRole).toSize();
		QRect sectionRect(rect);

		// set position of the cell
		if (OTN == Qt::Horizontal)
			sectionRect.setTop(rowSpanSize(logicalIdx,0,i)); // distance from 0 to i-1 rows
		else
			sectionRect.setLeft(columnSpanSize(logicalIdx,0,i));

		sectionRect.setSize(cellSize);

		// check up span column or row
		QModelIndex colSpanIdx = columnSpanIndex(cellIndex);
		QModelIndex rowSpanIdx = rowSpanIndex(cellIndex);
		if (colSpanIdx.isValid()) {
			int colSpanFrom = colSpanIdx.column();
			int colSpanCnt  = colSpanIdx.data(COLUMN_SPAN_ROLE).toInt();
			int colSpanTo   = colSpanFrom+colSpanCnt-1;
			int colSpan     = columnSpanSize(cellIndex.row(),colSpanFrom,colSpanCnt);
			if (OTN == Qt::Horizontal)
				sectionRect.setLeft(sectionViewportPosition(colSpanFrom));
			else {
				sectionRect.setLeft(columnSpanSize(logicalIdx,0,colSpanFrom));
				i = colSpanTo;
			}

			sectionRect.setWidth(colSpan);

			// check up  if the column span index has row span
			QVariant subRowSpanData = colSpanIdx.data(ROW_SPAN_ROLE);
			if (subRowSpanData.isValid()) {
				int subRowSpanFrom = colSpanIdx.row();
				int subRowSpanCnt  = subRowSpanData.toInt();
				int subRowSpanTo   = subRowSpanFrom+subRowSpanCnt-1;
				int subRowSpan     = rowSpanSize(colSpanFrom,subRowSpanFrom,subRowSpanCnt);
				if (OTN == Qt::Vertical)
					sectionRect.setTop(sectionViewportPosition(subRowSpanFrom));
				else {
					sectionRect.setTop(rowSpanSize(colSpanFrom,0,subRowSpanFrom));
					i = subRowSpanTo;
				}
				sectionRect.setHeight(subRowSpan);
			}
			cellIndex=colSpanIdx;
		}
		if (rowSpanIdx.isValid()) {
			int rowSpanFrom = rowSpanIdx.row();
			int rowSpanCnt  = rowSpanIdx.data(ROW_SPAN_ROLE).toInt();
			int rowSpanTo   = rowSpanFrom+rowSpanCnt-1;
			int rowSpan     = rowSpanSize(cellIndex.column(),rowSpanFrom,rowSpanCnt);
			if (OTN == Qt::Vertical)
				sectionRect.setTop(sectionViewportPosition(rowSpanFrom));
			else {
				sectionRect.setTop(rowSpanSize(logicalIdx,0,rowSpanFrom));
				i = rowSpanTo;
			}
			sectionRect.setHeight(rowSpan);

			// check up if the row span index has column span
			QVariant subColSpanData = rowSpanIdx.data(COLUMN_SPAN_ROLE);
			if (subColSpanData.isValid()) {
				int subColSpanFrom = rowSpanIdx.column();
				int subColSpanCnt  = subColSpanData.toInt();
				int subColSpanTo   = subColSpanFrom+subColSpanCnt-1;
				int subColSpan     = columnSpanSize(rowSpanFrom,subColSpanFrom,subColSpanCnt);
				if (OTN == Qt::Horizontal)
					sectionRect.setLeft(sectionViewportPosition(subColSpanFrom));
				else {
					sectionRect.setLeft(columnSpanSize(rowSpanFrom,0,subColSpanFrom));
					i = subColSpanTo;
				}
				sectionRect.setWidth(subColSpan);
			}
			cellIndex=rowSpanIdx;
		}

		// draw section with style
		QStyleOptionHeader sectionStyle;
		initStyleOption(&sectionStyle);
		sectionStyle.textAlignment = Qt::AlignCenter;
		sectionStyle.iconAlignment = Qt::AlignVCenter;
		sectionStyle.section = logicalIdx;
		sectionStyle.text = cellIndex.data(Qt::DisplayRole).toString();
		sectionStyle.rect = sectionRect;

		// file background or foreground color of the cell
		QVariant bg = cellIndex.data(Qt::BackgroundRole);
		QVariant fg = cellIndex.data(Qt::ForegroundRole);
		if (bg.canConvert<QBrush>()) {
			QBrush bgBrush = bg.value<QBrush>();
			sectionStyle.palette.setBrush(QPalette::Button, bgBrush);
			sectionStyle.palette.setBrush(QPalette::Window, bgBrush);
		}
		if (fg.canConvert<QBrush>())
			sectionStyle.palette.setBrush(QPalette::ButtonText, fg.value<QBrush>());

		painter->save();
		qDrawShadePanel(painter,sectionStyle.rect,sectionStyle.palette,false,1,&sectionStyle.palette.brush(QPalette::Button));
		style()->drawControl(QStyle::CE_HeaderLabel, &sectionStyle, painter);
		painter->restore();
	}
}

QSize HierarchicalHeaderView::sectionSizeFromContents(int logicalIndex) const {
	const int OTN = orientation();
	const int LEVEL_CNT = (OTN == Qt::Horizontal)?m_model->rowCount():m_model->columnCount();

	QSize siz = QHeaderView::sectionSizeFromContents(logicalIndex);
	for (int i=0; i<LEVEL_CNT; ++i) {
		QModelIndex cellIndex = (OTN == Qt::Horizontal)?m_model->index(i,logicalIndex):m_model->index(logicalIndex,i);
		QModelIndex colSpanIdx = columnSpanIndex(cellIndex);
		QModelIndex rowSpanIdx = rowSpanIndex(cellIndex);
		siz=cellIndex.data(Qt::SizeHintRole).toSize();

		if (colSpanIdx.isValid()) {
			int colSpanFrom = colSpanIdx.column();
			int colSpanCnt     = colSpanIdx.data(COLUMN_SPAN_ROLE).toInt();
			int colSpanTo   = colSpanFrom + colSpanCnt -1;
			siz.setWidth(columnSpanSize(colSpanIdx.row(),colSpanFrom,colSpanCnt));
			if (OTN == Qt::Vertical) i = colSpanTo;
		}
		if (rowSpanIdx.isValid()) {
			int rowSpanFrom = rowSpanIdx.row();
			int rowSpanCnt  = rowSpanIdx.data(ROW_SPAN_ROLE).toInt();
			int rowSpanTo   = rowSpanFrom + rowSpanCnt-1;
			siz.setHeight(rowSpanSize(rowSpanIdx.column(),rowSpanFrom,rowSpanCnt));
			if (OTN == Qt::Horizontal) i = rowSpanTo;
		}
	}
	return siz;
}

QModelIndex HierarchicalHeaderView::columnSpanIndex(const QModelIndex& currentIdx) const {
	const int curRow = currentIdx.row();
	const int curCol = currentIdx.column();
	int i = curCol;
	while (i >= 0) {
		QModelIndex spanIndex = m_model->index(curRow,i);
		QVariant span   = spanIndex.data(COLUMN_SPAN_ROLE);
		if (span.isValid() && spanIndex.column()+span.toInt()-1 >= curCol)
			return spanIndex;
		i--;
	}
	return QModelIndex();
}

QModelIndex HierarchicalHeaderView::rowSpanIndex(const QModelIndex& currentIdx) const {
	const int curRow = currentIdx.row();
	const int curCol = currentIdx.column();
	int i = curRow;
	while (i >= 0) {
		QModelIndex spanIndex = m_model->index(i,curCol);
		QVariant span   = spanIndex.data(ROW_SPAN_ROLE);
		if (span.isValid() && spanIndex.row()+span.toInt()-1 >= curRow)
			return spanIndex;
		i--;
	}
	return QModelIndex();
}

int HierarchicalHeaderView::columnSpanSize(int row, int from, int spanCount) const {
	int span = 0;
	for (int i=from; i<from+spanCount; ++i) {
		QSize cellSize = m_model->index(row,i).data(Qt::SizeHintRole).toSize();
		span += cellSize.width();
	}
	return span;
}


int HierarchicalHeaderView::rowSpanSize(int column, int from, int spanCount) const {
	int span = 0;
	for (int i=from; i<from+spanCount; ++i) {
		QSize cellSize = m_model->index(i,column).data(Qt::SizeHintRole).toSize();
		span += cellSize.height();
	}
	return span;
}

/**
 * @return section numbers
 */
int HierarchicalHeaderView::getSectionRange(QModelIndex& index, int* beginSection, int* endSection) const {
	// check up section range from the index
	QModelIndex colSpanIdx = columnSpanIndex(index);
	QModelIndex rowSpanIdx = rowSpanIndex(index);

	if (colSpanIdx.isValid()) {
		int colSpanFrom = colSpanIdx.column();
		int colSpanCnt  = colSpanIdx.data(COLUMN_SPAN_ROLE).toInt();
		int colSpanTo   = colSpanFrom+colSpanCnt-1;
		if (orientation() == Qt::Horizontal) {
			*beginSection = colSpanFrom;
			*endSection   = colSpanTo;
			index = colSpanIdx;
			return colSpanCnt;
		} else {
			// check up  if the column span index has row span
			QVariant subRowSpanData = colSpanIdx.data(ROW_SPAN_ROLE);
			if (subRowSpanData.isValid()) {
				int subRowSpanFrom = colSpanIdx.row();
				int subRowSpanCnt  = subRowSpanData.toInt();
				int subRowSpanTo   = subRowSpanFrom+subRowSpanCnt-1;
				*beginSection = subRowSpanFrom;
				*endSection   = subRowSpanTo;
				index = colSpanIdx;
				return subRowSpanCnt;
			}
		}
	}

	if (rowSpanIdx.isValid()) {
		int rowSpanFrom = rowSpanIdx.row();
		int rowSpanCnt  = rowSpanIdx.data(ROW_SPAN_ROLE).toInt();
		int rowSpanTo   = rowSpanFrom+rowSpanCnt-1;
		if (orientation() == Qt::Vertical) {
			*beginSection = rowSpanFrom;
			*endSection   = rowSpanTo;
			index = rowSpanIdx;
			return rowSpanCnt;
		} else {
			// check up if the row span index has column span
			QVariant subColSpanData = rowSpanIdx.data(COLUMN_SPAN_ROLE);
			if (subColSpanData.isValid()) {
				int subColSpanFrom = rowSpanIdx.column();
				int subColSpanCnt  = subColSpanData.toInt();
				int subColSpanTo   = subColSpanFrom+subColSpanCnt-1;
				*beginSection = subColSpanFrom;
				*endSection   = subColSpanTo;
				index = rowSpanIdx;
				return subColSpanCnt;
			}
		}
	}

	return 0;
}

void HierarchicalHeaderView::onSectionResized(int logicalIndex,int oldSize,int newSize) {
    Q_UNUSED(oldSize);

    if (!m_model)
		return;

	const int OTN = orientation();
	const int LEVEL_CNT = (OTN == Qt::Horizontal)?m_model->rowCount():m_model->columnCount();
	int pos = sectionViewportPosition(logicalIndex);
	int xx  = (OTN == Qt::Horizontal)?pos:0;
	int yy  = (OTN == Qt::Horizontal)?0:pos;
	QRect sectionRect(xx,yy,0,0);
	for (int i=0; i<LEVEL_CNT; ++i) {
		QModelIndex cellIndex = (OTN == Qt::Horizontal)?m_model->index(i,logicalIndex):m_model->index(logicalIndex,i);
		QSize cellSize=cellIndex.data(Qt::SizeHintRole).toSize();
		// set position of cell
		if (OTN == Qt::Horizontal) {
			sectionRect.setTop(rowSpanSize(logicalIndex,0,i));
			cellSize.setWidth(newSize);
		} else {
			sectionRect.setLeft(columnSpanSize(logicalIndex,0,i));
			cellSize.setHeight(newSize);
		}
		m_model->setData(cellIndex,cellSize,Qt::SizeHintRole);

		QModelIndex colSpanIdx = columnSpanIndex(cellIndex);
		QModelIndex rowSpanIdx = rowSpanIndex(cellIndex);

		if (colSpanIdx.isValid()) {
			int colSpanFrom = colSpanIdx.column();
			if (OTN == Qt::Horizontal)
				sectionRect.setLeft(sectionViewportPosition(colSpanFrom));
			else
				sectionRect.setLeft(columnSpanSize(logicalIndex,0,colSpanFrom));

		}
		if (rowSpanIdx.isValid()) {
			int rowSpanFrom = rowSpanIdx.row();
			if (OTN == Qt::Vertical)
				sectionRect.setTop(sectionViewportPosition(rowSpanFrom));
			else
				sectionRect.setTop(rowSpanSize(logicalIndex,0,rowSpanFrom));
		}
		QRect rToUpdate(sectionRect);
		rToUpdate.setWidth(viewport()->width()-sectionRect.left());
		rToUpdate.setHeight(viewport()->height()-sectionRect.top());
		viewport()->update(rToUpdate.normalized());
	}
}
