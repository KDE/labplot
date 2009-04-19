/***************************************************************************
    File                 : commandtemplates.h
    Project              : LabPlot/SciDAVis
    Description          : Undo command templates.
    --------------------------------------------------------------------
    Copyright            : (C) 2009 Tilman Benkert (thzs*gmx.net)
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

#ifndef COMMANDTEMPLATES_H
#define COMMANDTEMPLATES_H

#include <QUndoCommand>

template <class target_class, typename value_type>
class StandardSetterCmd: public QUndoCommand {
	public:
		StandardSetterCmd(target_class *target, value_type target_class:: *field, 
				const value_type &newValue, const QString &description) // use tr("%1: ...") for last arg
			: m_target(target), m_field(field), m_otherValue(newValue)  {
				setText(description.arg(m_target->name()));
			}

		virtual void initialize() {};
		virtual void finalize() {};

		virtual void redo() {
			initialize();
			value_type tmp = *m_target.*m_field;
			*m_target.*m_field = m_otherValue;
			m_otherValue = tmp;
			finalize();
		}

		virtual void undo() { redo(); }

	protected:
		target_class *m_target;
		value_type target_class:: *m_field;
		value_type m_otherValue;
};

template <class target_class, typename value_type>
class StandardSwapMethodSetterCmd: public QUndoCommand {
	public:
		StandardSwapMethodSetterCmd(target_class *target, value_type (target_class::*method)(const value_type &), 
				const value_type &newValue, const QString &description) // use tr("%1: ...") for last arg
			: m_target(target), m_method(method), m_otherValue(newValue) {
				setText(description.arg(m_target->name()));
			}

		virtual void initialize() {};
		virtual void finalize() {};

		virtual void redo() {
			initialize();
			m_otherValue = (*m_target.*m_method)(m_otherValue);
			finalize();
		}

		virtual void undo() { redo(); }

	protected:
		target_class *m_target;
		value_type (target_class:: *m_method)(const value_type &);
		value_type m_otherValue;
};


#endif


