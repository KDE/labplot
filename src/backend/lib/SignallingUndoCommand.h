/*
    File                 : SignallingUndoCommand.h
    Project              : SciDAVis / LabPlot
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2010 Knut Franke <Knut.Franke*gmx.net (use @ for *)>
    Description          : An undo command calling a method/signal/slot on a
    QObject on redo/undo.
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef SIGNALLING_UNDO_COMMAND_H
#define SIGNALLING_UNDO_COMMAND_H

#include <QUndoCommand>
#include <QByteArray>

class SignallingUndoCommand : public QUndoCommand {
public:
	SignallingUndoCommand(const QString &text, QObject* receiver, const char* redoMethod, const char* undoMethod,
			QGenericArgument val0 = QGenericArgument(), QGenericArgument val1 = QGenericArgument(),
			QGenericArgument val2 = QGenericArgument(), QGenericArgument val3 = QGenericArgument());
	~SignallingUndoCommand() override;

	void redo() override;
	void undo() override;

private:
	QGenericArgument arg(int index);
	QByteArray m_redo, m_undo;
	QObject* m_receiver;
	int m_argument_count;
	int* m_argument_types;
	void** m_argument_data;
};

#endif // ifndef SIGNALLING_UNDO_COMMAND_H
