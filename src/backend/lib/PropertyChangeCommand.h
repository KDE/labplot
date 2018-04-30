/***************************************************************************
    File                 : PropertyChangeCommand.h
    Project              : SciDAVis / LabPlot
    --------------------------------------------------------------------
    Copyright            : (C) 2010 Knut Franke
    Email (use @ for *)  : Knut.Franke*gmx.net
    Description          : Generic undo command changing a single variable.

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

		*m_property += *(static_cast<const PropertyChangeCommand*>(other)->m_property);
		return true;
	}

	T *m_property;

private:
	T m_other_value;
};

#endif // ifndef PROPERTY_CHANGE_COMMAND_H
