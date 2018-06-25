/***************************************************************************
    File                 : commandtemplates.h
    Project              : LabPlot
    Description          : Undo/Redo command templates
    --------------------------------------------------------------------
    Copyright            : (C) 2009 Tilman Benkert (thzs@gmx.net)
	Copyright            : (C) 2017 by Alexander Semke (alexander.semke@web.de)
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

#include <KLocalizedString>

template <class target_class, typename value_type>
class StandardSetterCmd : public QUndoCommand {
public:
	StandardSetterCmd(target_class* target, value_type target_class::* field, value_type newValue, const KLocalizedString& description) // use ki18n("%1: ...")
		: m_target(target), m_field(field), m_otherValue(newValue)  {
			setText(description.subs(m_target->name()).toString());
		}

	virtual void initialize() {};
	virtual void finalize() {};

	void redo() override {
		initialize();
		value_type tmp = *m_target.*m_field;
		*m_target.*m_field = m_otherValue;
		m_otherValue = tmp;
		finalize();
	}

	void undo() override { redo(); }

protected:
	target_class* m_target;
	value_type target_class::*m_field;
	value_type m_otherValue;
};

template <class target_class, typename value_type>
class StandardMacroSetterCmd : public QUndoCommand {
public:
	StandardMacroSetterCmd(target_class* target, value_type target_class::*field, value_type newValue, const KLocalizedString& description) // use ki18n("%1: ...")
		: m_target(target), m_field(field), m_otherValue(newValue)  {
			setText(description.subs(m_target->name()).toString());
		}

	virtual void initialize() {};
	virtual void finalize() {};
	virtual void finalizeUndo() {};

	void redo() override {
		initialize();
		value_type tmp = *m_target.*m_field;
		*m_target.*m_field = m_otherValue;
		m_otherValue = tmp;
		finalize();
	}

	//call finalizeUndo() at the end where only the signal is emmited
	//and no actual finalize-method is called that can potentially
	//cause  new entries on the undo-stack
	void undo() override {
		initialize();
		value_type tmp = *m_target.*m_field;
		*m_target.*m_field = m_otherValue;
		m_otherValue = tmp;
		finalizeUndo();
	}

protected:
	target_class* m_target;
	value_type target_class::*m_field;
	value_type m_otherValue;
};

template <class target_class, typename value_type>
class StandardSwapMethodSetterCmd : public QUndoCommand {
public:
	StandardSwapMethodSetterCmd(target_class* target, value_type (target_class::*method)(value_type), value_type newValue, const KLocalizedString& description) // use ki18n("%1: ...")
		: m_target(target), m_method(method), m_otherValue(newValue) {
			setText(description.subs(m_target->name()).toString());
		}

	virtual void initialize() {};
	virtual void finalize() {};

	void redo() override {
		initialize();
		m_otherValue = (*m_target.*m_method)(m_otherValue);
		finalize();
	}

	void undo() override { redo(); }

protected:
	target_class* m_target;
	value_type (target_class::*m_method)(value_type);
	value_type m_otherValue;
};

#endif
