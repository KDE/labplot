#ifndef SPREADSHEET_H
#define SPREADSHEET_H

#include <QWidget>
#include <QMenu>
#include <QTableView>
#include "TableModel.h"

#include "Set.h"
#include "sheettype.h"

class MainWin;

class Spreadsheet: public QTableView
{
	Q_OBJECT
public:
	Spreadsheet(MainWin *m);
	~Spreadsheet();
	SheetType sheetType() const { return type; }
	void resetHeader(int from=0);
	void addSet(Set *set);
	QString columnName(int col) const;
	void setColumnName(int col, QString name);
	QString columnType(int col) const;
	void setColumnType(int col, QString name);
	QString columnFormat(int col) const;
	void setColumnFormat(int col, QString name);

	int rowCount() const { return model()->rowCount(); }
	void setRowCount(int count) { ((TableModel *)model())->setRowCount(count); }
	int columnCount() const { return model()->columnCount(); }
	void setColumnCount(int count) {
		int oldcount=columnCount();
		((TableModel *)model())->setColumnCount(count); 
		resetHeader(oldcount);
	}
	QString text(int row, int col) const;
	void setText(int row, int col, QString text);
	int currentRow() const;			//!< returns current row (latest selection)
	int currentColumn() const;		//!< returns current column (latest selection)
	QList<int> currentRows() const;		//!< returns a sorted list of selected rows
	QList<int> currentColumns() const;	//!< returns a sorted list of selected columns
private:
	MainWin *mw;
	SheetType type;			//!< needed for mw->active{Work,Spread}sheet()
	QString notes;
	void contextMenuEvent(QContextMenuEvent *);
	QString columnHeader(int col) const;
	void setColumnHeader(int col, QString name) {
		model()->setHeaderData(col,Qt::Horizontal,name);
	}
	int filledRows(int col) const;	//!< returns number of filled rows in column col
public slots:
	void Menu(QMenu *menu);
	void setTitle(QString title="");
	void setRowNumber(int row=0);
	void addColumn() { setColumnCount(columnCount()+1); }
	QString Notes() const { return notes; }	
	void setNotes(QString notes="");
	void setProperties(QString label=0, int type=1, int format=0);	
private slots:
	void plot();		//!< create a plot from the selected data
	void exportData();	//!< export selected data (ExportDialog)
	void setColumnValues();	//!< set colum  values (ColumnValuesDialog)
	void deleteSelectedColumns();
	void deleteSelectedRows();
};

#endif //SPREADSHEET
