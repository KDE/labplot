/***************************************************************************
    File                 : SignallingUndoCommand.cpp
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

#include "SignallingUndoCommand.h"
#include <QMetaObject>
#include <QMetaType>

/**
 * \class SignallingUndoCommand
 * \brief An undo command calling a method/signal/slot on a QObject on redo/undo.
 *
 * SignallingUndoCommand is a generic undo command which can be used in cases where undo/redo events
 * need to be forwarded to given methods, signals or slots of a QObject. That is, it behaves like a
 * cross between an undo command and a Qt signal-slot (or signal-signal) connection. Different
 * methods can be selected for undo and redo, but they are supposed to have the same signature.
 *
 * The intended use case is to have SignallingUndoCommand trigger notification signals before and
 * after one or more undo commands change some internal state; compare
 * AbstractAspect::exec(QUndoCommand*,const char*,const char*,QGenericArgument,QGenericArgument,QGenericArgument,QGenericArgument).
 * The advantage of separating out the signalling into this class is that the names and
 * arguments of the signals appear in the source code of the Aspect instead of its private class or
 * commands; this is desirable because signals are conceptually part of the public API rather than
 * the internal implementation (compare \ref aspect "The Aspect Framework").
 *
 * SignallingUndoCommand uses Qt's meta object system to dynamically invoke the target method, so
 * the class declaring the method needs to inherit from QObject and contain the Q_OBJECT macro;
 * just as if you wanted it to participate in signal-slot connections (though the methods to be
 * invoked need to be neither signals nor slots).
 * Method arguments are given using the macro Q_ARG(typename, const value&). Since
 * the variable given as "value" will likely be out of scope when undo() is called, a copy needs to
 * be created. This uses QMetaType, which means that (non-trivial) argument types need to be
 * registered using qRegisterMetaType() before giving them to a SignallingUndoCommand (in
 * particular, this also goes for pointers to custom data types). The situation here is analogous
 * to an asynchronous method invocation using QMetaMethod::invoke() with Qt::QueuedConnection.
 */

/**
 * \brief Constructor.
 *
 * \arg \c text A description of the undo command (compare QUndoCommand::setText()).
 * \arg \c receiver The object whose methods/signals/slots should be invoked.
 * \arg \c redo The name of the method to be called when the command is (re-)executed; excluding the signature.
 * \arg \c undo Analogously to redo, the method to be called when the command is undone.
 * \arg <tt>val0,val1,val2,val3</tt> Arguments to the undo and redo methods; to be given using Q_ARG().
 *
 * Simple example:
 * \code
 * QUndoStack stack;
 * QAction action;
 * stack.push(new SignallingUndoCommand(i18n("enable action"), &action, "setEnabled", "setDisabled", Q_ARG(bool, true)));
 * \endcode
 */
SignallingUndoCommand::SignallingUndoCommand(const QString &text, QObject *receiver, const char *redoMethod, const char *undoMethod,
				QGenericArgument val0, QGenericArgument val1,
				QGenericArgument val2, QGenericArgument val3)
	: QUndoCommand(text),
	m_redo(redoMethod),
	m_undo(undoMethod),
	m_receiver(receiver)
{
	// munge arguments
	const char *type_names[] = { val0.name(), val1.name(), val2.name(), val3.name() };
	void *argument_data[] = { val0.data(), val1.data(), val2.data(), val3.data() };
	for (m_argument_count=0; qstrlen(type_names[m_argument_count]) > 0; ++m_argument_count);

	// copy arguments (Q_ARG references will often go out of scope before redo/undo are called)
	m_argument_types = new int[m_argument_count];
	Q_CHECK_PTR(m_argument_types);
	m_argument_data = new void*[m_argument_count];
	Q_CHECK_PTR(m_argument_data);
	for (int i=0; i<m_argument_count; i++) {
		m_argument_types[i] = QMetaType::type(type_names[i]);
		if (m_argument_types[i]) // type is known to QMetaType
			m_argument_data[i] = QMetaType::create(m_argument_types[i], argument_data[i]);
		else
			qWarning("SignallingUndoCommand: failed to copy unknown type %s"
					" (needs to be registered with qRegisterMetaType())!\n", type_names[i]);
	}
}

SignallingUndoCommand::~SignallingUndoCommand() {
	for (int i=0; i<m_argument_count; ++i)
		if (m_argument_types[i] && m_argument_data[i])
			QMetaType::destroy(m_argument_types[i], m_argument_data[i]);
	delete[] m_argument_types;
	delete[] m_argument_data;
}

QGenericArgument SignallingUndoCommand::arg(int index) {
	if (index >= m_argument_count)
		return QGenericArgument();
	else
		return QGenericArgument(QMetaType::typeName(m_argument_types[index]), m_argument_data[index]);
}

void SignallingUndoCommand::redo() {
	const QMetaObject *mo = m_receiver->metaObject();
	if (!mo->invokeMethod(m_receiver, m_redo, arg(0), arg(1), arg(2), arg(3)))
		qWarning("FAILED to invoke %s on %s\n", m_redo.constData(), mo->className());
}

void SignallingUndoCommand::undo() {
	const QMetaObject *mo = m_receiver->metaObject();
	if (!mo->invokeMethod(m_receiver, m_undo, arg(0), arg(1), arg(2), arg(3)))
		qWarning("FAILED to invoke %s on %s\n", m_undo.constData(), mo->className());
}

