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
	int rowCount(const QModelIndex &parent = QModelIndex()) const;
	int columnCount(const QModelIndex &parent = QModelIndex()) const;
	QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
	bool setData(const QModelIndex &index, const QVariant &value,int role = Qt::EditRole);
//	QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;

//	void setRowCount(int c);
//	void setColumnCount(int c) { }
Qt::ItemFlags flags(const QModelIndex &index) const;
//	insertRows();
//	removeRows();
//	insertColumns();
//	removeColumns();
private:
	QList< QList<QString> > table;
};

#endif // TABLEMODEL_H
