#ifndef SPREADSHEET_H
#define SPREADSHEET_H

#include <QWidget>
#include <QTableWidget>
#include <QMenu>

#include "Set.h"
#include "sheettype.h"

class MainWin;

class Spreadsheet: public QTableWidget
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
private:
	MainWin *mw;
	SheetType type;	// needed for mw->active{Work,Spread}sheet()
	QString notes;
	void contextMenuEvent(QContextMenuEvent *);
	QString columnHeader(int col);
	void setColumnHeader(int col, QString name); 
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
