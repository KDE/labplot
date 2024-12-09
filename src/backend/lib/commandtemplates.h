/*
	File                 : commandtemplates.h
	Project              : LabPlot
	Description          : Undo/Redo command templates
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2009 Tilman Benkert <thzs@gmx.net>
	SPDX-FileCopyrightText: 2017 Alexander Semke <alexander.semke@web.de>
	SPDX-FileCopyrightText: 2020 Stefan Gerlach <stefan.gerlach@uni.kn>

	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef COMMANDTEMPLATES_H
#define COMMANDTEMPLATES_H

#include <QUndoCommand>

#include <KLocalizedString>

#include "backend/lib/macros.h"

/*!
 * \brief The LongExecutionCmd class
 * Use this undo command if you expect long execution times
 * Executes all child commands and sets the cursor to the waiting symbol
 */
class LongExecutionCmd : public QUndoCommand {
public:
	LongExecutionCmd(const QString& text, QUndoCommand* parent = nullptr)
		: QUndoCommand(text, parent) {
	}

	virtual void redo() override {
		WAIT_CURSOR;
		QUndoCommand::redo();
		RESET_CURSOR;
	}

	virtual void undo() override {
		WAIT_CURSOR;
		QUndoCommand::undo();
		RESET_CURSOR;
	}
};

template<class target_class, typename value_type>
class StandardSetterCmd : public QUndoCommand {
public:
	StandardSetterCmd(target_class* target,
					  value_type target_class::* field,
					  value_type newValue,
					  const KLocalizedString& description,
					  QUndoCommand* parent = nullptr) // use ki18n("%1: ...")
		: QUndoCommand(parent)
		, m_target(target)
		, m_field(field)
		, m_otherValue(newValue) {
		setText(description.subs(m_target->name()).toString());
	}

	virtual void initialize() {
	}
	virtual void finalize() {
	}

	void redo() override {
		initialize();
		value_type tmp = *m_target.*m_field;
		*m_target.*m_field = m_otherValue;
		m_otherValue = std::move(tmp);
		QUndoCommand::redo(); // redo all childs
		finalize();
	}

	void undo() override {
		redo();
	}

protected:
	target_class* m_target;
	value_type target_class::* m_field;
	value_type m_otherValue;
};

template<class target_class, typename value_type>
class StandardQVectorSetterCmd : public QUndoCommand {
public:
	StandardQVectorSetterCmd(target_class* target,
							 QVector<value_type> target_class::* field,
							 int index,
							 value_type newValue,
							 const KLocalizedString& description) // use ki18n("%1: ...")
		: m_target(target)
		, m_field(field)
		, m_index(index)
		, m_otherValue(newValue) {
		setText(description.subs(m_target->name()).toString());
	}

	virtual void initialize() {
	}
	virtual void finalize() {
	}

	void redo() override {
		DEBUG(Q_FUNC_INFO);
		initialize();
		value_type tmp = (*m_target.*m_field).at(m_index);
		(*m_target.*m_field)[m_index] = m_otherValue;
		m_otherValue = tmp;
		QUndoCommand::redo(); // redo all childs
		finalize();
	}

	void undo() override {
		redo();
	}

protected:
	target_class* m_target;
	QVector<value_type> target_class::* m_field;
	int m_index;
	value_type m_otherValue;
};

template<class target_class, typename value_type>
class StandardMacroSetterCmd : public QUndoCommand {
public:
	StandardMacroSetterCmd(target_class* target,
						   value_type target_class::* field,
						   value_type newValue,
						   const KLocalizedString& description) // use ki18n("%1: ...")
		: m_target(target)
		, m_field(field)
		, m_otherValue(newValue) {
		setText(description.subs(m_target->name()).toString());
	}

	virtual void initialize() {
	}
	virtual void finalize() {
	}
	virtual void finalizeUndo() {
	}

	void redo() override {
		initialize();
		value_type tmp = *m_target.*m_field;
		*m_target.*m_field = m_otherValue;
		m_otherValue = tmp;
		QUndoCommand::redo(); // redo all childs
		finalize();
	}

	// call finalizeUndo() at the end where only the signal is emitted
	// and no actual finalize-method is called that can potentially
	// cause  new entries on the undo-stack
	void undo() override {
		initialize();
		value_type tmp = *m_target.*m_field;
		*m_target.*m_field = m_otherValue;
		m_otherValue = tmp;
		QUndoCommand::undo(); // undo all childs
		finalizeUndo();
	}

protected:
	target_class* m_target;
	value_type target_class::* m_field;
	value_type m_otherValue;
};

template<class target_class, typename value_type>
class StandardSwapMethodSetterCmd : public QUndoCommand {
public:
	StandardSwapMethodSetterCmd(target_class* target,
								value_type (target_class::*method)(value_type),
								value_type newValue,
								const KLocalizedString& description) // use ki18n("%1: ...")
		: m_target(target)
		, m_method(method)
		, m_otherValue(newValue) {
		setText(description.subs(m_target->name()).toString());
	}

	virtual void initialize() {
	}
	virtual void finalize() {
	}

	void redo() override {
		initialize();
		m_otherValue = (*m_target.*m_method)(m_otherValue);
		QUndoCommand::redo(); // redo all childs
		finalize();
	}

	void undo() override {
		redo();
	}

protected:
	target_class* m_target;
	value_type (target_class::*m_method)(value_type);
	value_type m_otherValue;
};

/*
 * Setter class for struct fields within private objects. See Heatmap.cpp for an example
 * Macro: STRUCT_SETTER_CMD_IMPL_F_S(...)
 */
template<class target_class, typename structType, typename value_type>
class StructSetterCmd : public QUndoCommand {
public:
	StructSetterCmd(target_class* target,
					structType target_class::*structField,
					value_type structType::*field,
					value_type newValue,
					const KLocalizedString& description,
					QUndoCommand* parent = nullptr)
		: QUndoCommand(parent)
		, m_target(target)
		, m_structField(structField)
		, m_field(field)
		, m_value(newValue) {
		setText(description.subs(m_target->name()).toString());
	}

	virtual void redo() {
		const auto oldValue = (m_target->*m_structField).*m_field;
		(m_target->*m_structField).*m_field = m_value;
		m_value = oldValue;

		finalize();
	}

	virtual void undo() {
		redo();
	}

	virtual void finalize() {
	}

protected:
	structType target_class::*m_structField;
	target_class* m_target;
	value_type structType::*m_field;
	value_type m_value;
};

#endif
