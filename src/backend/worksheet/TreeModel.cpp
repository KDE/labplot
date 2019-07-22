/***************************************************************************
File                 : TreeModel.cpp
Project              : LabPlot
Description 	     : This is an abstract treemodel which can be used by a treeview
--------------------------------------------------------------------
Copyright            : (C) 2019 Martin Marmsoler (martin.marmsoler@gmail.com)

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

#include "TreeModel.h"

//##########################################################
// TreeItem ###############################################
//##########################################################

TreeItem::TreeItem(const QVector<QVariant> &data, TreeItem *parent) :
	itemData(data),
	parentItem(parent) {
}

TreeItem::~TreeItem() {
	qDeleteAll(childItems);
}

TreeItem *TreeItem::child(int number) {
	return childItems.value(number);
}

int TreeItem::childCount() const {
	return childItems.count();
}

int TreeItem::childNumber() const {
	if (parentItem)
		return parentItem->childItems.indexOf(const_cast<TreeItem*>(this));

	return 0;
}

int TreeItem::columnCount() const {
	return itemData.count();
}

QVariant TreeItem::data(int column) const {
	return itemData.value(column);
}

QVariant TreeItem::backgroundColor() const {
	return m_backgroundColor;
}

bool TreeItem::insertChildren(int position, int count, int columns) {
	if (position < 0 || position > childItems.size())
		return false;

	for (int row = 0; row < count; ++row) {
		QVector<QVariant> data(columns);
		TreeItem *item = new TreeItem(data, this);
		childItems.insert(position, item);
	}

	return true;
}

bool TreeItem::insertColumns(int position, int columns) {
	if (position < 0 || position > itemData.size())
		return false;

	for (int column = 0; column < columns; ++column)
		itemData.insert(position, QVariant());

	foreach (TreeItem *child, childItems)
		child->insertColumns(position, columns);

	return true;
}

TreeItem *TreeItem::parent() {
	return parentItem;
}

bool TreeItem::removeChildren(int position, int count) {
	if (position < 0 || position + count > childItems.size())
		return false;

	for (int row = 0; row < count; ++row)
		delete childItems.takeAt(position);

	return true;
}

bool TreeItem::removeColumns(int position, int columns) {
	if (position < 0 || position + columns > itemData.size())
		return false;

	for (int column = 0; column < columns; ++column)
		itemData.remove(position);

	foreach (TreeItem *child, childItems)
		child->removeColumns(position, columns);

	return true;
}

bool TreeItem::setData(int column, const QVariant &value) {
	if (column < 0 || column >= itemData.size())
		return false;

	itemData[column] = value;
	return true;
}

bool TreeItem::setBackgroundColor(int column, const QVariant &value) {
	if (column < 0 || column >= itemData.size())
		return false;

	m_backgroundColor = value.value<QColor>();
	return true;
}

//##########################################################
// TreeModel ###############################################
//##########################################################

TreeModel::TreeModel(const QStringList &headers, QObject *parent)
	: QAbstractItemModel(parent) {
	QVector<QVariant> rootData;
	for (auto header : headers)
		rootData << header;

	rootItem = new TreeItem(rootData);
}

TreeModel::~TreeModel() {
	delete rootItem;
}

int TreeModel::columnCount(const QModelIndex & /* parent */) const {
	return rootItem->columnCount();
}

QVariant TreeModel::treeData(const int row, const int column, const QModelIndex& parent, const int role) {
	QModelIndex currentIndex = index(row, column, parent);
	return data(currentIndex, role);
}

QVariant TreeModel::data(const QModelIndex &index, int role) const {
	if (!index.isValid())
		return QVariant();

	if (role != Qt::DisplayRole && role != Qt::EditRole && role != Qt::BackgroundRole)
		return QVariant();

	TreeItem *item = getItem(index);

	if (role != Qt::BackgroundRole)
		return item->data(index.column());

	return item->backgroundColor();
}

Qt::ItemFlags TreeModel::flags(const QModelIndex &index) const {
	if (!index.isValid())
		return nullptr;

	return Qt::ItemIsEditable | QAbstractItemModel::flags(index);
}

TreeItem *TreeModel::getItem(const QModelIndex &index) const {
	if (index.isValid()) {
		TreeItem *item = static_cast<TreeItem*>(index.internalPointer());
		if (item)
			return item;
	}
	return rootItem;
}

QVariant TreeModel::headerData(int section, Qt::Orientation orientation, int role) const {
	if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
		return rootItem->data(section);

	return QVariant();
}

QModelIndex TreeModel::index(int row, int column, const QModelIndex &parent) const {
	if (parent.isValid() && parent.column() != 0)
		return QModelIndex();

	TreeItem *parentItem = getItem(parent);

	TreeItem *childItem = parentItem->child(row);
	if (childItem)
		return createIndex(row, column, childItem);
	else
		return QModelIndex();
}

bool TreeModel::insertColumns(int position, int columns, const QModelIndex &parent) {
	bool success;

	beginInsertColumns(parent, position, position + columns - 1);
	success = rootItem->insertColumns(position, columns);
	endInsertColumns();

	return success;
}

bool TreeModel::insertRows(int position, int rows, const QModelIndex &parent) {
	TreeItem *parentItem = getItem(parent);
	bool success;

	beginInsertRows(parent, position, position + rows - 1);
	success = parentItem->insertChildren(position, rows, rootItem->columnCount());
	endInsertRows();

	return success;
}

QModelIndex TreeModel::parent(const QModelIndex &index) const {
	if (!index.isValid())
		return QModelIndex();

	TreeItem *childItem = getItem(index);
	TreeItem *parentItem = childItem->parent();

	if (parentItem == rootItem)
		return QModelIndex();

	return createIndex(parentItem->childNumber(), 0, parentItem);
}

bool TreeModel::removeColumns(int position, int columns, const QModelIndex &parent) {
	bool success;

	beginRemoveColumns(parent, position, position + columns - 1);
	success = rootItem->removeColumns(position, columns);
	endRemoveColumns();

	if (rootItem->columnCount() == 0)
		removeRows(0, rowCount());

	return success;
}

bool TreeModel::removeRows(int position, int rows, const QModelIndex &parent) {
	TreeItem *parentItem = getItem(parent);
	bool success = true;

	beginRemoveRows(parent, position, position + rows - 1);
	success = parentItem->removeChildren(position, rows);
	endRemoveRows();

	return success;
}

int TreeModel::rowCount(const QModelIndex &parent) const {
	TreeItem *parentItem = getItem(parent);

	return parentItem->childCount();
}

bool TreeModel::setTreeData(const QVariant data, const int row, const int column, const QModelIndex &parent, int role) {
	QModelIndex curveIndex = index(row, column, parent);
	return setData(curveIndex, data, role);
}

bool TreeModel::setData(const QModelIndex &index, const QVariant &value, int role) {

	if (role == Qt::EditRole || role == Qt::DisplayRole) {
		TreeItem *item = getItem(index);
		bool result = item->setData(index.column(), value);

		if (result)
			emit dataChanged(index, index);

		return result;
	} else if (role == Qt::BackgroundRole) {
		TreeItem *item = getItem(index);
		bool result = item->setBackgroundColor(index.column(), value);

		if (result)
			emit dataChanged(index, index);
	}

	return false;
}

bool TreeModel::setHeaderData(int section, Qt::Orientation orientation, const QVariant &value, int role) {
	if (role != Qt::EditRole && role != Qt::DisplayRole && orientation != Qt::Horizontal)
		return false;

	bool result = rootItem->setData(section, value);

	if (result)
		emit headerDataChanged(orientation, section, section);

	return result;
}

int TreeModel::compareStrings(const QString value, const int row, const int column, const QModelIndex &parent) {
	QModelIndex plotIndex = index(row, column, parent);
	return plotIndex.data().toString().compare(value);
}
