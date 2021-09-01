/*
    File                 : PropertyChangeCommand.h
    Project              : SciDAVis / LabPlot
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2010 Knut Franke
    Email (use @ for *)  : Knut.Franke*gmx.net
    Description          : Generic undo command changing a single variable.

*/

/***************************************************************************
 *                                                                         *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *                                                                         *
 ***************************************************************************/

#ifndef PROPERTY_CHANGE_COMMAND_H
#define PROPERTY_CHANGE_COMMAND_H

#include <QUndoCommand>

/**
 * \class PropertyChangeCommand
 * \brief Generic undo command changing a single variable.
 *
 * Given a pointer to a variable (usually a member of the class instantiating the command, or of
 * its private implementation class) and a new value, assigns the value to the variable. A backup
 * of the old value is made, so that undo/redo can switch back and forth between the two values.
 * The value type needs to support copy construction and assignment.
 */

template<class T> class PropertyChangeCommand : public QUndoCommand {

public:
	PropertyChangeCommand(const QString &text, T *property, const T &new_value)
		: m_property(property), m_other_value(new_value) {
			setText(text);
		}

	void redo() override {
		T tmp = *m_property;
		*m_property = m_other_value;
		m_other_value = tmp;
	}

	void undo() override {
		redo();
	}

	int id() const override {
		return reinterpret_cast<std::intptr_t>(m_property);
	}

	bool mergeWith(const QUndoCommand* other) override {
		if (other->id() != id())
			return false;

		if (std::is_same<T, bool>::value)
			*m_property = *(static_cast<const PropertyChangeCommand*>(other)->m_property);
		else
			*m_property += *(static_cast<const PropertyChangeCommand*>(other)->m_property);

		return true;
	}

	T *m_property;

private:
	T m_other_value;
};

#endif // ifndef PROPERTY_CHANGE_COMMAND_H
