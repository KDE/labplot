/***************************************************************************
    File                 : abstractcolumncommands.h
    Project              : LabPlot
    Description          : Commands to be called by AbstractColumn to modify AbstractColumnPrivate
    --------------------------------------------------------------------
    Copyright            : (C) 2007-2009 Tilman Benkert (thzs@gmx.net)
    Copyright            : (C) 2010 Knut Franke (knut.franke@gmx.de)

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
	explicit AbstractColumnClearMasksCmd(AbstractColumnPrivate* col, QUndoCommand* parent = nullptr);
	~AbstractColumnClearMasksCmd() override;

	void redo() override;
	void undo() override;

private:
	AbstractColumnPrivate *m_col;
	IntervalAttribute<bool> m_masking;
	bool m_copied;
};

class AbstractColumnSetMaskedCmd : public QUndoCommand {
public:
	explicit AbstractColumnSetMaskedCmd(AbstractColumnPrivate* col, const Interval<int>& interval, bool masked, QUndoCommand* parent = nullptr);
	~AbstractColumnSetMaskedCmd() override;

	void redo() override;
	void undo() override;

private:
	AbstractColumnPrivate* m_col;
	Interval<int> m_interval;
	bool m_masked;
	IntervalAttribute<bool> m_masking;
	bool m_copied;
};

class AbstractColumnInsertRowsCmd : public QUndoCommand {
public:
	explicit AbstractColumnInsertRowsCmd(AbstractColumn* col, int before, int count, QUndoCommand* parent = nullptr);
	~AbstractColumnInsertRowsCmd() override;

	void redo() override;
	void undo() override;

protected:
	AbstractColumnPrivate* m_col;
	int m_before;
	int m_count;
};

class AbstractColumnRemoveRowsCmd : public QUndoCommand {
public:
	explicit AbstractColumnRemoveRowsCmd(AbstractColumn* col, int first, int count, QUndoCommand* parent = nullptr);
	~AbstractColumnRemoveRowsCmd() override;

	void redo() override;
	void undo() override;

protected:
	AbstractColumnPrivate* m_col;
	int m_first;
	int m_count;
	IntervalAttribute<bool> m_masking;
};

#endif // ifndef ABSTRACTCOLUMNCOMMANDS_H
