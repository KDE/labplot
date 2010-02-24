/***************************************************************************
    File                 : abstractcolumncommands.h
    Project              : SciDAVis/LabPlot
    Description          : Commands to be called by AbstractColumn to modify AbstractColumn::Private
    --------------------------------------------------------------------
    Copyright            : (C) 2007-2009 Tilman Benkert (thzs*gmx.net),
	                                      Knut Franke (knut.franke*gmx.de)
	 								(C) 2010 by Knut Franke
                           (replace * with @ in the email addresses) 

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

#ifndef ABSTRACTCOLUMNCOMMANDS_H
#define ABSTRACTCOLUMNCOMMANDS_H

#include "AbstractColumnPrivate.h"
#include <QUndoCommand>

class AbstractColumnClearMasksCmd : public QUndoCommand {
public:
	AbstractColumnClearMasksCmd(AbstractColumn::Private *col, QUndoCommand *parent = 0);
	~AbstractColumnClearMasksCmd();

	virtual void redo();
	virtual void undo();

private:
	AbstractColumn::Private *m_col;
	IntervalAttribute<bool> m_masking;
	bool m_copied;

};

class AbstractColumnSetMaskedCmd : public QUndoCommand
{
public:
	AbstractColumnSetMaskedCmd(AbstractColumn::Private * col, Interval<int> interval, bool masked, QUndoCommand * parent = 0 );
	~AbstractColumnSetMaskedCmd();

	virtual void redo();
	virtual void undo();

private:
	AbstractColumn::Private * m_col;
	Interval<int> m_interval;
	bool m_masked;
	IntervalAttribute<bool> m_masking;
	bool m_copied;

};

class AbstractColumnInsertRowsCmd : public QUndoCommand
{
public:
	AbstractColumnInsertRowsCmd(AbstractColumn * col, int before, int count, QUndoCommand * parent = 0 );
	~AbstractColumnInsertRowsCmd();

	virtual void redo();
	virtual void undo();

protected:
	AbstractColumn::Private * m_col;
	int m_before;
	int m_count;
};

class AbstractColumnRemoveRowsCmd : public QUndoCommand
{
public:
	AbstractColumnRemoveRowsCmd(AbstractColumn * col, int first, int count, QUndoCommand * parent = 0 );
	~AbstractColumnRemoveRowsCmd();

	virtual void redo();
	virtual void undo();

protected:
	AbstractColumn::Private * m_col;
	int m_first;
	int m_count;
	IntervalAttribute<bool> m_masking;
};

#endif // ifndef ABSTRACTCOLUMNCOMMANDS_H
