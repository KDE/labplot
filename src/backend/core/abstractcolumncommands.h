/*
    File                 : abstractcolumncommands.h
    Project              : LabPlot
    Description          : Commands to be called by AbstractColumn to modify AbstractColumnPrivate
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2007-2009 Tilman Benkert (thzs@gmx.net)
    SPDX-FileCopyrightText: 2010 Knut Franke (knut.franke@gmx.de)
    SPDX-FileCopyrightText: 2014-2021 Alexander Semke (alexander.semke@web.de)
    SPDX-License-Identifier: GPL-2.0-or-later
*/


#ifndef ABSTRACTCOLUMNCOMMANDS_H
#define ABSTRACTCOLUMNCOMMANDS_H

#include "AbstractColumnPrivate.h"
#include <QUndoCommand>

class AbstractColumnClearMasksCmd : public QUndoCommand {
public:
	explicit AbstractColumnClearMasksCmd(AbstractColumnPrivate*, QUndoCommand* parent = nullptr);
	~AbstractColumnClearMasksCmd() override;

	void redo() override;
	void undo() override;

private:
	AbstractColumnPrivate* m_col;
	IntervalAttribute<bool> m_masking;
	bool m_copied;
};

class AbstractColumnSetMaskedCmd : public QUndoCommand {
public:
	explicit AbstractColumnSetMaskedCmd(AbstractColumnPrivate*, const Interval<int>& interval, bool masked, QUndoCommand* parent = nullptr);
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
	explicit AbstractColumnInsertRowsCmd(AbstractColumn*, int before, int count, QUndoCommand* parent = nullptr);
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

class AbstractColumnSetHeatmapFormatCmd : public QUndoCommand {
public:
	explicit AbstractColumnSetHeatmapFormatCmd(AbstractColumnPrivate*, const AbstractColumn::HeatmapFormat&, QUndoCommand* parent = nullptr);
	~AbstractColumnSetHeatmapFormatCmd() override;

	void redo() override;
	void undo() override;

private:
	AbstractColumnPrivate* m_col;
	AbstractColumn::HeatmapFormat m_format;
};

class AbstractColumnRemoveHeatmapFormatCmd : public QUndoCommand {
public:
	explicit AbstractColumnRemoveHeatmapFormatCmd(AbstractColumnPrivate*, QUndoCommand* parent = nullptr);
	~AbstractColumnRemoveHeatmapFormatCmd() override;

	void redo() override;
	void undo() override;

private:
	AbstractColumnPrivate* m_col;
	AbstractColumn::HeatmapFormat m_format;
};

#endif // ifndef ABSTRACTCOLUMNCOMMANDS_H
