/*
    File                 : TreeModel.h
    Project              : LabPlot
    Description 	     : This is an abstract treemodel which can be used by a treeview
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2019 Martin Marmsoler <martin.marmsoler@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef TREEMODEL_H
#define TREEMODEL_H

#include <QAbstractItemModel>
#include <QColor>

/*!
 * \brief The TreeItem class
 * Item in the treemodel
 */
class TreeItem {
public:
	explicit TreeItem(const QVector<QVariant>& data, TreeItem* parent = 0);
	~TreeItem();

	TreeItem* child(int number);
	int childCount() const;
	int columnCount() const;
	QVariant data(int column) const;
	QVariant backgroundColor() const;
	bool insertChildren(int position, int count, int columns);
	bool insertColumns(int position, int columns);
	TreeItem *parent();
	bool removeChildren(int position, int count);
	bool removeColumns(int position, int columns);
	int childNumber() const;
	bool setData(int column, const QVariant& value);
	bool setBackgroundColor(int column, const QVariant& value);

private:
    QList<TreeItem*> childItems;
    QVector<QVariant> itemData;
	QColor m_backgroundColor{QColor(0,0,0,0)};
    TreeItem *parentItem{nullptr};
};
/*!
 * \brief The TreeModel class
 * This is an abstract treemodel which can be used by a treeview
 */
class TreeModel : public QAbstractItemModel {
    Q_OBJECT

public:
	explicit TreeModel(const QStringList& headers, QObject* parent = nullptr);
	~TreeModel();
	QVariant treeData(const int row, const int column, const QModelIndex& parent = QModelIndex(), const int role = Qt::EditRole);
	QVariant data(const QModelIndex&, int role) const override;
	QVariant headerData(int section, Qt::Orientation orientation,
						int role = Qt::DisplayRole) const override;

	QModelIndex index(int row, int column,
						const QModelIndex& parent = QModelIndex()) const override;
	QModelIndex parent(const QModelIndex&) const override;

	int rowCount(const QModelIndex& parent = QModelIndex()) const override;
	int columnCount(const QModelIndex& parent = QModelIndex()) const override;

	Qt::ItemFlags flags(const QModelIndex&) const override;
	bool setTreeData(const QVariant& data, const int row, const int column,
						const QModelIndex& parent = QModelIndex(), int role = Qt::EditRole);
	bool setData(const QModelIndex&, const QVariant& value,
					int role = Qt::EditRole) override;
	int compareStrings(const QString& value, const int row, const int column, const QModelIndex& parent = QModelIndex());
	bool setHeaderData(int section, Qt::Orientation orientation,
						const QVariant& value, int role = Qt::EditRole) override;

	bool insertColumns(int position, int columns,
						const QModelIndex& parent = QModelIndex()) override;
	bool removeColumns(int position, int columns,
						const QModelIndex& parent = QModelIndex()) override;
	bool insertRows(int position, int rows,
					const QModelIndex& parent = QModelIndex()) override;
	bool removeRows(int position, int rows,
					const QModelIndex& parent = QModelIndex()) override;

private:
	TreeItem* getItem(const QModelIndex&) const;
	TreeItem* rootItem{nullptr};
};

#endif // TREEMODEL_H
