#ifndef SPREADSHEET_H
#define SPREADSHEET_H

//#define TABLEVIEW

#include <QWidget>
#include <QMenu>
#ifdef TABLEVIEW
#include <QTableView>
#include "TableModel.h"
#else
#include <QTableWidget>
#endif

#include "Set.h"
#include "sheettype.h"

class MainWin;

#ifdef TABLEVIEW
class Spreadsheet: public QTableView
#else
class Spreadsheet: public QTableWidget
#endif
{
	Q_OBJECT
public:
	Spreadsheet(MainWin *m);
	SheetType sheetType() { return type; }
	void resetHeader();
	void addSet(Set *set);
	QString columnName(int col);
	void setColumnName(int col, QString name);
	QString columnType(int col);
	void setColumnType(int col, QString name);
	QString columnFormat(int col);
	void setColumnFormat(int col, QString name);
#ifdef TABLEVIEW
// TODO
	int currentColumn() { return 0; }
	int rowCount() { return model()->rowCount(); }
	void setRowCount(int c) { }
//	void setRowCount(int c) { ((TableModel *)model())->setRowCount(c); }
	int columnCount() { return model()->columnCount(); }
	void setColumnCount(int c) { }
//	void setColumnCount(int c) { ((TableModel *)model())->setColumnCount(c); }
#endif
private:
	MainWin *mw;
	SheetType type;	// needed for mw->active{Work,Spread}sheet()
	QString notes;
	void contextMenuEvent(QContextMenuEvent *);
	QString columnHeader(int col);
	void setColumnHeader(int col, QString name); 
	int filledRows(int col);	//!< returns number of filled rows in column col
public slots:
	void Menu(QMenu *menu);
	void setTitle(QString title="");
	void setRowNumber(int row=0);
	QString Notes() const { return notes; }	
	void setNotes(QString notes="");
	void setProperties(QString label=0, int type=1, int format=0);	
private slots:
	void newSpreadsheet();	// needed for menu since direct connect crashes
};

#endif //SPREADSHEET
