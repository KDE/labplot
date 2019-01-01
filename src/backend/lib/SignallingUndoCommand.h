/***************************************************************************
    File                 : SignallingUndoCommand.h
    Project              : SciDAVis / LabPlot
    --------------------------------------------------------------------
    Copyright            : (C) 2010 Knut Franke
    Email (use @ for *)  : Knut.Franke*gmx.net
    Description          : An undo command calling a method/signal/slot on a
                           QObject on redo/undo.

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
