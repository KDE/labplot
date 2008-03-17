//LabPlot: TableModel.h

#ifndef TABLEMODEL_H
#define TABLEMODEL_H

#include <QList>
#include <QString>
#include <QAbstractTableModel>

class TableModel : public QAbstractTableModel
{
public:
	TableModel(QObject *parent=0);
	void setRowCount(int c);
	void setColumnCount(int c);
	int rowCount(const QModelIndex &parent = QModelIndex()) const;
	int columnCount(const QModelIndex &parent = QModelIndex()) const;
	bool insertRows(int row, int count, const QModelIndex &parent = QModelIndex());
	bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex());
	bool insertColumns(int col, int count, const QModelIndex &parent = QModelIndex());
	bool removeColumns(int col, int count, const QModelIndex &parent = QModelIndex());
	QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
	bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);
	QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
	bool setHeaderData(int section, Qt::Orientation orientation, const QVariant &value, int role = Qt::EditRole);

	QModelIndex index(int row, int column, const QModelIndex & = QModelIndex()) const { return createIndex(row, column); }
	QModelIndex parent(const QModelIndex &) const { return QModelIndex(); }
	Qt::ItemFlags flags(const QModelIndex &index) const;
private:
	QList< QList<QString> > table;
	QList< QString> header;
};

#endif // TABLEMODEL_H
