//LabPlot: TableModel.cc

#include <KDebug>
#include "TableModel.h"

TableModel::TableModel(QObject *parent)
	: QAbstractTableModel(parent)
{
	for(int i=0;i<2;i++) {
		QList<QString> list;
		table.append(list);
		for(int j=0;j<100;j++)
			table[i].append(QString::number(j)+'/'+QString::number(i));
	}
}

int TableModel::rowCount(const QModelIndex &) const {
	kdDebug()<<"TableModel::rowCount()"<<endl;
	if(columnCount()>0)
		return table[0].size();
	return 0;
}

int TableModel::columnCount(const QModelIndex &) const {
	kdDebug()<<"TableModel::columnCount()"<<endl;
	return table.size();
}

/*void TableModel::setRowCount(int c) {
	kdDebug()<<"TableModel::setRowCount()"<<endl;
	for(int i=0;i<columnCount();i++) {
		kdDebug()<<"OK"<<endl;
		for(int j=rowCount()-1;j<c;j++) {
			kdDebug()<<"col/row="<<i<<j<<endl;
			table[i].append(QString::number(j)+'/'+QString::number(i));
		}
	}
}
*/

QVariant TableModel::data(const QModelIndex &index, int role) const {
     if (!index.isValid() || role != Qt::DisplayRole)
         return QVariant();
     return table[index.column()][index.row()];
}

bool TableModel::setData(const QModelIndex &index, const QVariant &value, int role) {
     if (index.isValid() && role == Qt::EditRole) {
	table[index.column()].insert(index.row(),value.toString());
         emit dataChanged(index, index);
         return true;
     }
     return false;
}

Qt::ItemFlags TableModel::flags(const QModelIndex &index) const {
     if (!index.isValid())
         return Qt::ItemIsEnabled;

     return QAbstractItemModel::flags(index) | Qt::ItemIsEditable;
}
