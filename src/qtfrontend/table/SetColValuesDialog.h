/***************************************************************************
    File                 : SetColValuesDialog.h
    Project              : SciDAVis
    --------------------------------------------------------------------
    Copyright            : (C) 2006 by Ion Vasilief, 
                           Tilman Benkert
                           Knut Franke
    Email (use @ for *)  : ion_vasilief*yahoo.fr, thzs*gmx.net,
                           knut.franke*gmx.de
    Description          : Set column values dialog
                           
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
#ifndef VALUESDIALOG_H
#define VALUESDIALOG_H

#include <QDialog>
#include "core/AbstractScriptingEngine.h"

class QComboBox;
class QTextEdit;
class QSpinBox;
class QPushButton;
class QLabel;

class Table;
class ScriptEdit;

	
//! Set column values dialog
class SetColValuesDialog : public QDialog, public scripted
{ 
    Q_OBJECT

public:
    SetColValuesDialog(AbstractScriptingEngine *engine, QWidget* parent = 0, Qt::WFlags fl = 0 );
    ~SetColValuesDialog();
	
	QSize sizeHint() const ;
	void customEvent( QEvent *e );

    QComboBox* functions;
    QComboBox* boxColumn;
    QComboBox* boxSelectColumn;
    QPushButton* btnAddFunction; 
    QPushButton* btnAddCol;
    QPushButton* btnOk;
    QPushButton* btnCancel;
    QPushButton *buttonPrev;
    QPushButton *buttonNext;
    QPushButton *addCellButton;
    QPushButton *btnApply;
    ScriptEdit* commands;
    QTextEdit* explain;
	QSpinBox* start, *end;
	QLabel *colNameLabel;

public slots:
	void accept();
	bool apply();
	void prevColumn();
	void nextColumn();
	void setFunctions();
	void insertFunction();
	void insertCol();
	void insertCell();
	void insertExplain(int index);
	void setTable(Table* w);
	void updateColumn(int sc);

private:
	Table* m_table;
};

#endif //
