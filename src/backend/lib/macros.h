/*
	File                 : macros.h
	Project              : LabPlot
	Description          : Various preprocessor macros
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2008 Tilman Benkert <thzs@gmx.net>
	SPDX-FileCopyrightText: 2013-2015 Alexander Semke <alexander.semke@web.de>
	SPDX-FileCopyrightText: 2016-2021 Stefan Gerlach <stefan.gerlach@uni.kn>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef MACROS_H
#define MACROS_H

#include <QApplication>
#include <QMetaEnum>

#include "macrosWarningStyle.h"

// C++ style warning (works on Windows)
#include "Debug.h"

struct Lock {
	inline explicit Lock(bool& variable)
		: variable(variable) {
		// Make sure it is not already locked
		// somewhere else
		assert(!variable);
		this->variable = true;
	}

	inline ~Lock() {
		variable = false;
	}

private:
	bool& variable;
};

/*!
 * Used for example for connections with NumberSpinbox because those are using
 * a feedback and so breaking the connection dock -> element -> dock is not desired
 */
#define CONDITIONAL_RETURN_NO_LOCK                                                                                                                             \
	if (m_initializing)                                                                                                                                        \
		return;

/*!
 * Lock mechanism used in docks to prevent loops (dock -> element -> dock)
 * dock (locking) -> element: No feedback to the dock
 */
#define CONDITIONAL_LOCK_RETURN                                                                                                                                \
	CONDITIONAL_RETURN_NO_LOCK                                                                                                                                 \
	const Lock lock(m_initializing);

#define WAIT_CURSOR QApplication::setOverrideCursor(QCursor(Qt::WaitCursor))
#define RESET_CURSOR QApplication::restoreOverrideCursor()

#ifdef HAVE_WINDOWS
#define STDSTRING(qstr) qstr.toUtf8().constData()
#else
#define STDSTRING(qstr) qstr.toStdString()
#endif
#define UTF8_QSTRING(str) QString::fromUtf8(str)

#define TAB QStringLiteral("\t")
#define NEWLINE QStringLiteral("\n")

#define CHECK(expr)                                                                                                                                            \
	if (!(expr)) {                                                                                                                                             \
		DEBUG(Q_FUNC_INFO << ", FAILING " #expr);                                                                                                              \
		return false;                                                                                                                                          \
	}
// check if var is in [min, max)
#define INRANGE(var, min, max) (var >= min && var < max)

// access enums in Q_OBJECT/Q_GADGET classes
#define ENUM_TO_STRING(class, enum, index)                                                                                                                     \
	(class ::staticMetaObject.enumerator(class ::staticMetaObject.indexOfEnumerator(#enum)).valueToKey(static_cast<int>(index)))
#define ENUM_COUNT(class, enum) (class ::staticMetaObject.enumerator(class ::staticMetaObject.indexOfEnumerator(#enum)).keyCount())

//////////////////////// LineEdit Access ///////////////////////////////
#define SET_INT_FROM_LE(var, le)                                                                                                                               \
	{                                                                                                                                                          \
		bool ok;                                                                                                                                               \
		const int tmp = QLocale().toInt((le)->text(), &ok);                                                                                                    \
		if (ok)                                                                                                                                                \
			var = tmp;                                                                                                                                         \
	}

#define SET_DOUBLE_FROM_LE(var, le)                                                                                                                            \
	{                                                                                                                                                          \
		bool ok;                                                                                                                                               \
		const double tmp = QLocale().toDouble((le)->text(), &ok);                                                                                              \
		if (ok)                                                                                                                                                \
			var = tmp;                                                                                                                                         \
	}

// including enable recalculate
#define SET_DOUBLE_FROM_LE_REC(var, le)                                                                                                                        \
	{                                                                                                                                                          \
		QString str = (le)->text().trimmed();                                                                                                                  \
		if (!str.isEmpty()) {                                                                                                                                  \
			bool ok;                                                                                                                                           \
			const double tmp = QLocale().toDouble(str, &ok);                                                                                                   \
			if (ok) {                                                                                                                                          \
				var = tmp;                                                                                                                                     \
				enableRecalculate();                                                                                                                           \
			}                                                                                                                                                  \
		}                                                                                                                                                      \
	}

//////////////////////// Accessor ///////////////////////////////

// type: BASIC (by value), CLASS (by reference)
// D: private var, SHARED: Q_D
// DECL and IMPL
#define BASIC_ACCESSOR(type, var, method, Method)                                                                                                              \
	type method() const {                                                                                                                                      \
		return var;                                                                                                                                            \
	};                                                                                                                                                         \
	void set##Method(const type value) {                                                                                                                       \
		var = value;                                                                                                                                           \
	}
#define CLASS_ACCESSOR(type, var, method, Method)                                                                                                              \
	type method() const {                                                                                                                                      \
		return var;                                                                                                                                            \
	};                                                                                                                                                         \
	void set##Method(const type& value) {                                                                                                                      \
		var = value;                                                                                                                                           \
	}

#define BASIC_D_ACCESSOR_DECL(type, method, Method)                                                                                                            \
	type method() const;                                                                                                                                       \
	void set##Method(const type value);

#define BASIC_D_ACCESSOR_DECL_2ARG(type, arg2, method, Method)                                                                                                 \
	type method() const;                                                                                                                                       \
	void set##Method(const type value, arg2);

#define CLASS_D_ACCESSOR_DECL(type, method, Method)                                                                                                            \
	type method() const;                                                                                                                                       \
	void set##Method(const type& value);

#define BASIC_D_INDEX_ACCESSOR_DECL(type, method, Method)                                                                                                      \
	type method(int index) const;                                                                                                                              \
	void set##Method(int index, type value);

// replaces CLASS_D_READER_IMPL
#define BASIC_D_READER_IMPL(classname, type, method, var)                                                                                                      \
	type classname::method() const {                                                                                                                           \
		Q_D(const classname);                                                                                                                                  \
		return d->var;                                                                                                                                         \
	}
// replaces CLASS_SHARED_D_READER_IMPL
#define BASIC_SHARED_D_READER_IMPL(classname, type, method, var)                                                                                               \
	type classname::method() const {                                                                                                                           \
		Q_D(const classname);                                                                                                                                  \
		return d->var;                                                                                                                                         \
	}

#define BASIC_D_ACCESSOR_IMPL(classname, type, method, Method, var)                                                                                            \
	void classname::set##Method(const type value) {                                                                                                            \
		Q_D(classname);                                                                                                                                        \
		d->var = value;                                                                                                                                        \
	}                                                                                                                                                          \
	BASIC_D_READER_IMPL(classname, type, method, var)

#define CLASS_D_ACCESSOR_IMPL(classname, type, method, Method, var)                                                                                            \
	void classname::set##Method(const type& value) {                                                                                                           \
		Q_D(classname);                                                                                                                                        \
		d->var = value;                                                                                                                                        \
	}                                                                                                                                                          \
	BASIC_D_READER_IMPL(classname, type, method, var)

#define BASIC_SHARED_D_ACCESSOR_IMPL(classname, type, method, Method, var)                                                                                     \
	void classname::set##Method(const type value) {                                                                                                            \
		Q_D(classname);                                                                                                                                        \
		d->var = value;                                                                                                                                        \
	}                                                                                                                                                          \
	BASIC_SHARED_D_READER_IMPL(classname, type, method, var)

#define CLASS_SHARED_D_ACCESSOR_IMPL(classname, type, method, Method, var)                                                                                     \
	void classname::set##Method(const type& value) {                                                                                                           \
		Q_D(classname);                                                                                                                                        \
		d->var = value;                                                                                                                                        \
	}                                                                                                                                                          \
	BASIC_SHARED_D_READER_IMPL(classname, type, method, var)

#define POINTER_D_ACCESSOR_DECL(type, method, Method)                                                                                                          \
	type* method() const;                                                                                                                                      \
	void set##Method(type* ptr);

#define FLAG_D_ACCESSOR_DECL(Method)                                                                                                                           \
	bool is##Method() const;                                                                                                                                   \
	bool has##Method() const;                                                                                                                                  \
	void set##Method(const bool value = true);                                                                                                                 \
	void enable##Method(const bool value = true);

#define FLAG_D_ACCESSOR_IMPL(classname, Method, var)                                                                                                           \
	void classname::set##Method(const bool value) {                                                                                                            \
		d->var = value;                                                                                                                                        \
	}                                                                                                                                                          \
	void classname::enable##Method(const bool value) {                                                                                                         \
		d->var = value;                                                                                                                                        \
	}                                                                                                                                                          \
	bool classname::is##Method() const {                                                                                                                       \
		return d->var;                                                                                                                                         \
	}                                                                                                                                                          \
	bool classname::has##Method() const {                                                                                                                      \
		return d->var;                                                                                                                                         \
	}

//////////////////////// Standard Setter /////////////////////

#define STD_SETTER_CMD_IMPL(class_name, cmd_name, value_type, field_name)                                                                                      \
	class class_name##cmd_name##Cmd : public StandardSetterCmd<class_name::Private, value_type> {                                                              \
	public:                                                                                                                                                    \
		class_name##cmd_name##Cmd(class_name::Private* target, value_type newValue, const KLocalizedString& description)                                       \
			: StandardSetterCmd<class_name::Private, value_type>(target, &class_name::Private::field_name, newValue, description) {                            \
		}                                                                                                                                                      \
	};

// setter class with finalize()
#define STD_SETTER_CMD_IMPL_F(class_name, cmd_name, value_type, field_name, finalize_method)                                                                   \
	class class_name##cmd_name##Cmd : public StandardSetterCmd<class_name::Private, value_type> {                                                              \
	public:                                                                                                                                                    \
		class_name##cmd_name##Cmd(class_name::Private* target, value_type newValue, const KLocalizedString& description)                                       \
			: StandardSetterCmd<class_name::Private, value_type>(target, &class_name::Private::field_name, newValue, description) {                            \
		}                                                                                                                                                      \
		virtual void finalize() override {                                                                                                                     \
			m_target->finalize_method();                                                                                                                       \
		}                                                                                                                                                      \
	};

// setter class with signal emitting.
#define STD_SETTER_CMD_IMPL_S(class_name, cmd_name, value_type, field_name)                                                                                    \
	class class_name##cmd_name##Cmd : public StandardSetterCmd<class_name::Private, value_type> {                                                              \
	public:                                                                                                                                                    \
		class_name##cmd_name##Cmd(class_name::Private* target, value_type newValue, const KLocalizedString& description, QUndoCommand* parent = nullptr)       \
			: StandardSetterCmd<class_name::Private, value_type>(target, &class_name::Private::field_name, newValue, description, parent) {                    \
		}                                                                                                                                                      \
		virtual void finalize() override {                                                                                                                     \
			Q_EMIT m_target->q->field_name##Changed(m_target->*m_field);                                                                                       \
		}                                                                                                                                                      \
	};

// setter class for 3D plot classes need extra type var
#define STD_SETTER_CMD_IMPL_S_T(class_name, cmd_name, value_type, field_name, type_name, finalize_method)                                                      \
class class_name##cmd_name##Cmd : public StandardSetterCmd<class_name::Private, value_type> {                                                              \
		public:                                                                                                                                                    \
		class_name##cmd_name##Cmd(class_name::Private* target,                                                                                                 \
								  value_type newValue,                                                                                                         \
								  type_name type,                                                                                                              \
								  const KLocalizedString& description,                                                                                         \
								  QUndoCommand* parent = nullptr)                                                                                              \
			: StandardSetterCmd<class_name::Private, value_type>(target, &class_name::Private::field_name, newValue, description, parent)                      \
, plotType(type) {                                                                                                                                 \
}                                                                                                                                                      \
		virtual void finalize() override {                                                                                                                     \
			m_target->finalize_method(static_cast<type_name>(plotType));                                                                                       \
Q_EMIT m_target->q->field_name##Changed(m_target->*m_field);                                                                                       \
}                                                                                                                                                      \
																																							   \
	private:                                                                                                                                                   \
type_name plotType;                                                                                                                                    \
};

// setter class with finalize() and signal emitting.
#define STD_SETTER_CMD_IMPL_F_S(class_name, cmd_name, value_type, field_name, finalize_method)                                                                 \
	class class_name##cmd_name##Cmd : public StandardSetterCmd<class_name::Private, value_type> {                                                              \
	public:                                                                                                                                                    \
		class_name##cmd_name##Cmd(class_name::Private* target, value_type newValue, const KLocalizedString& description, QUndoCommand* parent = nullptr)       \
			: StandardSetterCmd<class_name::Private, value_type>(target, &class_name::Private::field_name, newValue, description, parent) {                    \
		}                                                                                                                                                      \
		virtual void finalize() override {                                                                                                                     \
			m_target->finalize_method();                                                                                                                       \
			Q_EMIT m_target->q->field_name##Changed(m_target->*m_field);                                                                                       \
		}                                                                                                                                                      \
	};

// setter class with finalize() and signal emitting, one field_name signal and one custom signal.
#define STD_SETTER_CMD_IMPL_F_S_SC(class_name, cmd_name, value_type, field_name, finalize_method, custom_signal)                                               \
	class class_name##cmd_name##Cmd : public StandardSetterCmd<class_name::Private, value_type> {                                                              \
	public:                                                                                                                                                    \
		class_name##cmd_name##Cmd(class_name::Private* target, value_type newValue, const KLocalizedString& description, QUndoCommand* parent = nullptr)       \
			: StandardSetterCmd<class_name::Private, value_type>(target, &class_name::Private::field_name, newValue, description, parent) {                    \
		}                                                                                                                                                      \
		virtual void finalize() override {                                                                                                                     \
			m_target->finalize_method();                                                                                                                       \
			Q_EMIT m_target->q->field_name##Changed(m_target->*m_field);                                                                                       \
			Q_EMIT m_target->q->custom_signal();                                                                                                               \
		}                                                                                                                                                      \
	};

// setter class with finalize() and signal emitting for changing several properties in one single step (embedded in beginMacro/endMacro)
#define STD_SETTER_CMD_IMPL_M_F_S(class_name, cmd_name, value_type, field_name, finalize_method)                                                               \
	class class_name##cmd_name##Cmd : public StandardMacroSetterCmd<class_name::Private, value_type> {                                                         \
	public:                                                                                                                                                    \
		class_name##cmd_name##Cmd(class_name::Private* target, value_type newValue, const KLocalizedString& description)                                       \
			: StandardMacroSetterCmd<class_name::Private, value_type>(target, &class_name::Private::field_name, newValue, description) {                       \
		}                                                                                                                                                      \
		virtual void finalize() override {                                                                                                                     \
			m_target->finalize_method();                                                                                                                       \
			Q_EMIT m_target->q->field_name##Changed(m_target->*m_field);                                                                                       \
		}                                                                                                                                                      \
		virtual void finalizeUndo() override {                                                                                                                 \
			Q_EMIT m_target->q->field_name##Changed(m_target->*m_field);                                                                                       \
		}                                                                                                                                                      \
	};

#define STD_SETTER_CMD_IMPL_I(class_name, cmd_name, value_type, field_name, init_method)                                                                       \
	class class_name##cmd_name##Cmd : public StandardSetterCmd<class_name::Private, value_type> {                                                              \
	public:                                                                                                                                                    \
		class_name##cmd_name##Cmd(class_name::Private* target, value_type newValue, const KLocalizedString& description)                                       \
			: StandardSetterCmd<class_name::Private, value_type>(target, &class_name::Private::field_name, newValue, description) {                            \
		}                                                                                                                                                      \
		virtual void initialize() {                                                                                                                            \
			m_target->init_method();                                                                                                                           \
		}                                                                                                                                                      \
	};

#define STD_SETTER_CMD_IMPL_IF(class_name, cmd_name, value_type, field_name, init_method, finalize_method)                                                     \
	class class_name##cmd_name##Cmd : public StandardSetterCmd<class_name::Private, value_type> {                                                              \
	public:                                                                                                                                                    \
		class_name##cmd_name##Cmd(class_name::Private* target, value_type newValue, const KLocalizedString& description)                                       \
			: StandardSetterCmd<class_name::Private, value_type>(target, &class_name::Private::field_name, newValue, description) {                            \
		}                                                                                                                                                      \
		virtual void initialize() {                                                                                                                            \
			m_target->init_method();                                                                                                                           \
		}                                                                                                                                                      \
		virtual void finalize() {                                                                                                                              \
			m_target->finalize_method();                                                                                                                       \
		}                                                                                                                                                      \
	};

#define STD_SWAP_METHOD_SETTER_CMD_IMPL(class_name, cmd_name, value_type, method_name)                                                                         \
	class class_name##cmd_name##Cmd : public StandardSwapMethodSetterCmd<class_name::Private, value_type> {                                                    \
	public:                                                                                                                                                    \
		class_name##cmd_name##Cmd(class_name::Private* target, value_type newValue, const KLocalizedString& description)                                       \
			: StandardSwapMethodSetterCmd<class_name::Private, value_type>(target, &class_name::Private::method_name, newValue, description) {                 \
		}                                                                                                                                                      \
	};

#define STD_SWAP_METHOD_SETTER_CMD_IMPL_F(class_name, cmd_name, value_type, method_name, finalize_method)                                                      \
	class class_name##cmd_name##Cmd : public StandardSwapMethodSetterCmd<class_name::Private, value_type> {                                                    \
	public:                                                                                                                                                    \
		class_name##cmd_name##Cmd(class_name::Private* target, value_type newValue, const KLocalizedString& description)                                       \
			: StandardSwapMethodSetterCmd<class_name::Private, value_type>(target, &class_name::Private::method_name, newValue, description) {                 \
		}                                                                                                                                                      \
		virtual void finalize() override {                                                                                                                     \
			m_target->finalize_method();                                                                                                                       \
		}                                                                                                                                                      \
	};

#define STD_SWAP_METHOD_SETTER_CMD_IMPL_I(class_name, cmd_name, value_type, method_name, init_method)                                                          \
	class class_name##cmd_name##Cmd : public StandardSwapMethodSetterCmd<class_name::Private, value_type> {                                                    \
	public:                                                                                                                                                    \
		class_name##cmd_name##Cmd(class_name::Private* target, value_type newValue, const KLocalizedString& description)                                       \
			: StandardSwapMethodSetterCmd<class_name::Private, value_type>(target, &class_name::Private::method_name, newValue, description) {                 \
		}                                                                                                                                                      \
		virtual void initialize() {                                                                                                                            \
			m_target->init_method();                                                                                                                           \
		}                                                                                                                                                      \
	};

#define STD_SWAP_METHOD_SETTER_CMD_IMPL_IF(class_name, cmd_name, value_type, method_name, init_method, finalize_method)                                        \
	class class_name##cmd_name##Cmd : public StandardSwapMethodSetterCmd<class_name::Private, value_type> {                                                    \
	public:                                                                                                                                                    \
		class_name##cmd_name##Cmd(class_name::Private* target, value_type newValue, const KLocalizedString& description)                                       \
			: StandardSwapMethodSetterCmd<class_name::Private, value_type>(target, &class_name::Private::method_name, newValue, description) {                 \
		}                                                                                                                                                      \
		virtual void initialize() {                                                                                                                            \
			m_target->init_method();                                                                                                                           \
		}                                                                                                                                                      \
		virtual void finalize() {                                                                                                                              \
			m_target->finalize_method();                                                                                                                       \
		}                                                                                                                                                      \
	};

// setter class for QGraphicsitem settings because
// there field_name() and setter_method() is used to get and set values
// with finalize() function and signal emitting.
#define GRAPHICSITEM_SETTER_CMD_IMPL_F_S(class_name, cmd_name, value_type, field_name, setter_method, finalize_method)                                         \
	class class_name##cmd_name##Cmd : public QUndoCommand {                                                                                                    \
	public:                                                                                                                                                    \
		class_name##cmd_name##Cmd(class_name::Private* target, value_type newValue, const KLocalizedString& description, QUndoCommand* parent = nullptr)       \
			: QUndoCommand(parent)                                                                                                                             \
			, m_target(target)                                                                                                                                 \
			, m_otherValue(newValue) {                                                                                                                         \
			setText(description.subs(m_target->name()).toString());                                                                                            \
		}                                                                                                                                                      \
		void redo() override {                                                                                                                                 \
			value_type tmp = m_target->field_name();                                                                                                           \
			m_target->setter_method(m_otherValue);                                                                                                             \
			m_otherValue = tmp;                                                                                                                                \
			QUndoCommand::redo(); /* redo all childs */                                                                                                        \
			finalize();                                                                                                                                        \
		}                                                                                                                                                      \
                                                                                                                                                               \
		void undo() override {                                                                                                                                 \
			redo();                                                                                                                                            \
		}                                                                                                                                                      \
		void finalize() {                                                                                                                                      \
			m_target->finalize_method();                                                                                                                       \
			Q_EMIT m_target->q->field_name##Changed(m_target->field_name());                                                                                   \
		}                                                                                                                                                      \
                                                                                                                                                               \
	private:                                                                                                                                                   \
		class_name::Private* m_target;                                                                                                                         \
		value_type m_otherValue;                                                                                                                               \
	};

//////////////////////// XML - serialization/deserialization /////
// TODO: do we really need all these tabs?
// TODO: why "do {...} while(0)"?

// QColor
#define WRITE_QCOLOR(color)                                                                                                                                    \
	{                                                                                                                                                          \
		writer->writeAttribute(QStringLiteral("color_r"), QString::number(color.red()));                                                                       \
		writer->writeAttribute(QStringLiteral("color_g"), QString::number(color.green()));                                                                     \
		writer->writeAttribute(QStringLiteral("color_b"), QString::number(color.blue()));                                                                      \
	}

#define WRITE_QCOLOR2(color, label)                                                                                                                            \
	{                                                                                                                                                          \
		writer->writeAttribute(QStringLiteral(label "_r"), QString::number(color.red()));                                                                      \
		writer->writeAttribute(QStringLiteral(label "_g"), QString::number(color.green()));                                                                    \
		writer->writeAttribute(QStringLiteral(label "_b"), QString::number(color.blue()));                                                                     \
	}

#define READ_QCOLOR(color)                                                                                                                                     \
	{                                                                                                                                                          \
		str = attribs.value(QStringLiteral("color_r")).toString();                                                                                             \
		if (str.isEmpty())                                                                                                                                     \
			reader->raiseMissingAttributeWarning(QStringLiteral("color_r"));                                                                                   \
		else                                                                                                                                                   \
			color.setRed(str.toInt());                                                                                                                         \
                                                                                                                                                               \
		str = attribs.value(QStringLiteral("color_g")).toString();                                                                                             \
		if (str.isEmpty())                                                                                                                                     \
			reader->raiseMissingAttributeWarning(QStringLiteral("color_g"));                                                                                   \
		else                                                                                                                                                   \
			color.setGreen(str.toInt());                                                                                                                       \
                                                                                                                                                               \
		str = attribs.value(QStringLiteral("color_b")).toString();                                                                                             \
		if (str.isEmpty())                                                                                                                                     \
			reader->raiseMissingAttributeWarning(QStringLiteral("color_b"));                                                                                   \
		else                                                                                                                                                   \
			color.setBlue(str.toInt());                                                                                                                        \
	}

#define READ_QCOLOR2(color, label)                                                                                                                             \
	{                                                                                                                                                          \
		str = attribs.value(QStringLiteral(label "_r")).toString();                                                                                            \
		if (str.isEmpty())                                                                                                                                     \
			reader->raiseMissingAttributeWarning(QStringLiteral(label "_r"));                                                                                  \
		else                                                                                                                                                   \
			color.setRed(str.toInt());                                                                                                                         \
                                                                                                                                                               \
		str = attribs.value(QStringLiteral(label "_g")).toString();                                                                                            \
		if (str.isEmpty())                                                                                                                                     \
			reader->raiseMissingAttributeWarning(QStringLiteral(label "_g"));                                                                                  \
		else                                                                                                                                                   \
			color.setGreen(str.toInt());                                                                                                                       \
                                                                                                                                                               \
		str = attribs.value(QStringLiteral(label "_b")).toString();                                                                                            \
		if (str.isEmpty())                                                                                                                                     \
			reader->raiseMissingAttributeWarning(QStringLiteral(label "_b"));                                                                                  \
		else                                                                                                                                                   \
			color.setBlue(str.toInt());                                                                                                                        \
	}
// QPen
#define WRITE_QPEN(pen)                                                                                                                                        \
	{                                                                                                                                                          \
		writer->writeAttribute(QStringLiteral("style"), QString::number(pen.style()));                                                                         \
		writer->writeAttribute(QStringLiteral("color_r"), QString::number(pen.color().red()));                                                                 \
		writer->writeAttribute(QStringLiteral("color_g"), QString::number(pen.color().green()));                                                               \
		writer->writeAttribute(QStringLiteral("color_b"), QString::number(pen.color().blue()));                                                                \
		writer->writeAttribute(QStringLiteral("width"), QString::number(pen.widthF()));                                                                        \
	}

#define READ_QPEN(pen)                                                                                                                                         \
	{                                                                                                                                                          \
		str = attribs.value(QStringLiteral("style")).toString();                                                                                               \
		if (str.isEmpty())                                                                                                                                     \
			reader->raiseMissingAttributeWarning(QStringLiteral("style"));                                                                                     \
		else                                                                                                                                                   \
			pen.setStyle(static_cast<Qt::PenStyle>(str.toInt()));                                                                                              \
                                                                                                                                                               \
		QColor color;                                                                                                                                          \
		str = attribs.value(QStringLiteral("color_r")).toString();                                                                                             \
		if (str.isEmpty())                                                                                                                                     \
			reader->raiseMissingAttributeWarning(QStringLiteral("color_r"));                                                                                   \
		else                                                                                                                                                   \
			color.setRed(str.toInt());                                                                                                                         \
                                                                                                                                                               \
		str = attribs.value(QStringLiteral("color_g")).toString();                                                                                             \
		if (str.isEmpty())                                                                                                                                     \
			reader->raiseMissingAttributeWarning(QStringLiteral("color_g"));                                                                                   \
		else                                                                                                                                                   \
			color.setGreen(str.toInt());                                                                                                                       \
                                                                                                                                                               \
		str = attribs.value(QStringLiteral("color_b")).toString();                                                                                             \
		if (str.isEmpty())                                                                                                                                     \
			reader->raiseMissingAttributeWarning(QStringLiteral("color_b"));                                                                                   \
		else                                                                                                                                                   \
			color.setBlue(str.toInt());                                                                                                                        \
                                                                                                                                                               \
		pen.setColor(color);                                                                                                                                   \
                                                                                                                                                               \
		str = attribs.value(QStringLiteral("width")).toString();                                                                                               \
		if (str.isEmpty())                                                                                                                                     \
			reader->raiseMissingAttributeWarning(QStringLiteral("width"));                                                                                     \
		else                                                                                                                                                   \
			pen.setWidthF(str.toDouble());                                                                                                                     \
	}

// QFont
#define WRITE_QFONT(font)                                                                                                                                      \
	{                                                                                                                                                          \
		writer->writeAttribute(QStringLiteral("fontFamily"), font.family());                                                                                   \
		writer->writeAttribute(QStringLiteral("fontSize"), QString::number(font.pixelSize()));                                                                 \
		writer->writeAttribute(QStringLiteral("fontPointSize"), QString::number(font.pointSize()));                                                            \
		writer->writeAttribute(QStringLiteral("fontWeight"), QString::number(font.weight()));                                                                  \
		writer->writeAttribute(QStringLiteral("fontItalic"), QString::number(font.italic()));                                                                  \
	}

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0) // uses font.setLegacyWeight(int)
#define READ_QFONT(font)                                                                                                                                       \
	{                                                                                                                                                          \
		str = attribs.value(QStringLiteral("fontFamily")).toString();                                                                                          \
		if (str.isEmpty())                                                                                                                                     \
			reader->raiseMissingAttributeWarning(QStringLiteral("fontFamily"));                                                                                \
		else                                                                                                                                                   \
			font.setFamily(str);                                                                                                                               \
                                                                                                                                                               \
		str = attribs.value(QStringLiteral("fontSize")).toString();                                                                                            \
		if (str.isEmpty())                                                                                                                                     \
			reader->raiseMissingAttributeWarning(QStringLiteral("fontSize"));                                                                                  \
		else {                                                                                                                                                 \
			int size = str.toInt();                                                                                                                            \
			if (size != -1)                                                                                                                                    \
				font.setPixelSize(size);                                                                                                                       \
		}                                                                                                                                                      \
                                                                                                                                                               \
		str = attribs.value(QStringLiteral("fontPointSize")).toString();                                                                                       \
		if (str.isEmpty())                                                                                                                                     \
			reader->raiseMissingAttributeWarning(QStringLiteral("fontPointSize"));                                                                             \
		else {                                                                                                                                                 \
			int size = str.toInt();                                                                                                                            \
			if (size != -1)                                                                                                                                    \
				font.setPointSize(size);                                                                                                                       \
		}                                                                                                                                                      \
                                                                                                                                                               \
		str = attribs.value(QStringLiteral("fontWeight")).toString();                                                                                          \
		if (str.isEmpty())                                                                                                                                     \
			reader->raiseMissingAttributeWarning(QStringLiteral("fontWeight"));                                                                                \
		else                                                                                                                                                   \
			font.setLegacyWeight(str.toInt());                                                                                                                 \
                                                                                                                                                               \
		str = attribs.value(QStringLiteral("fontItalic")).toString();                                                                                          \
		if (str.isEmpty())                                                                                                                                     \
			reader->raiseMissingAttributeWarning(QStringLiteral("fontItalic"));                                                                                \
		else                                                                                                                                                   \
			font.setItalic(str.toInt());                                                                                                                       \
	}
#else
#define READ_QFONT(font)                                                                                                                                       \
	{                                                                                                                                                          \
		str = attribs.value(QStringLiteral("fontFamily")).toString();                                                                                          \
		if (str.isEmpty())                                                                                                                                     \
			reader->raiseMissingAttributeWarning(QStringLiteral("fontFamily"));                                                                                \
		else                                                                                                                                                   \
			font.setFamily(str);                                                                                                                               \
                                                                                                                                                               \
		str = attribs.value(QStringLiteral("fontSize")).toString();                                                                                            \
		if (str.isEmpty())                                                                                                                                     \
			reader->raiseMissingAttributeWarning(QStringLiteral("fontSize"));                                                                                  \
		else {                                                                                                                                                 \
			int size = str.toInt();                                                                                                                            \
			if (size != -1)                                                                                                                                    \
				font.setPixelSize(size);                                                                                                                       \
		}                                                                                                                                                      \
                                                                                                                                                               \
		str = attribs.value(QStringLiteral("fontPointSize")).toString();                                                                                       \
		if (str.isEmpty())                                                                                                                                     \
			reader->raiseMissingAttributeWarning(QStringLiteral("fontPointSize"));                                                                             \
		else {                                                                                                                                                 \
			int size = str.toInt();                                                                                                                            \
			if (size != -1)                                                                                                                                    \
				font.setPointSize(size);                                                                                                                       \
		}                                                                                                                                                      \
                                                                                                                                                               \
		str = attribs.value(QStringLiteral("fontWeight")).toString();                                                                                          \
		if (str.isEmpty())                                                                                                                                     \
			reader->raiseMissingAttributeWarning(QStringLiteral("fontWeight"));                                                                                \
		else                                                                                                                                                   \
			font.setWeight(str.toInt());                                                                                                                       \
                                                                                                                                                               \
		str = attribs.value(QStringLiteral("fontItalic")).toString();                                                                                          \
		if (str.isEmpty())                                                                                                                                     \
			reader->raiseMissingAttributeWarning(QStringLiteral("fontItalic"));                                                                                \
		else                                                                                                                                                   \
			font.setItalic(str.toInt());                                                                                                                       \
	}
#endif

// QBrush
#define WRITE_QBRUSH(brush)                                                                                                                                    \
	{                                                                                                                                                          \
		writer->writeAttribute(QStringLiteral("brush_style"), QString::number(brush.style()));                                                                 \
		writer->writeAttribute(QStringLiteral("brush_color_r"), QString::number(brush.color().red()));                                                         \
		writer->writeAttribute(QStringLiteral("brush_color_g"), QString::number(brush.color().green()));                                                       \
		writer->writeAttribute(QStringLiteral("brush_color_b"), QString::number(brush.color().blue()));                                                        \
	}

#define READ_QBRUSH(brush)                                                                                                                                     \
	{                                                                                                                                                          \
		str = attribs.value(QStringLiteral("brush_style")).toString();                                                                                         \
		if (str.isEmpty())                                                                                                                                     \
			reader->raiseMissingAttributeWarning(QStringLiteral("brush_style"));                                                                               \
		else                                                                                                                                                   \
			brush.setStyle(static_cast<Qt::BrushStyle>(str.toInt()));                                                                                          \
                                                                                                                                                               \
		QColor color;                                                                                                                                          \
		str = attribs.value(QStringLiteral("brush_color_r")).toString();                                                                                       \
		if (str.isEmpty())                                                                                                                                     \
			reader->raiseMissingAttributeWarning(QStringLiteral("brush_color_r"));                                                                             \
		else                                                                                                                                                   \
			color.setRed(str.toInt());                                                                                                                         \
                                                                                                                                                               \
		str = attribs.value(QStringLiteral("brush_color_g")).toString();                                                                                       \
		if (str.isEmpty())                                                                                                                                     \
			reader->raiseMissingAttributeWarning(QStringLiteral("brush_color_g"));                                                                             \
		else                                                                                                                                                   \
			color.setGreen(str.toInt());                                                                                                                       \
                                                                                                                                                               \
		str = attribs.value(QStringLiteral("brush_color_b")).toString();                                                                                       \
		if (str.isEmpty())                                                                                                                                     \
			reader->raiseMissingAttributeWarning(QStringLiteral("brush_color_b"));                                                                             \
		else                                                                                                                                                   \
			color.setBlue(str.toInt());                                                                                                                        \
                                                                                                                                                               \
		brush.setColor(color);                                                                                                                                 \
	}

// Column
#define WRITE_COLUMN(column, columnName)                                                                                                                       \
	if (column) {                                                                                                                                              \
		writer->writeAttribute(QStringLiteral(#columnName), column->path());                                                                                   \
	} else {                                                                                                                                                   \
		writer->writeAttribute(QStringLiteral(#columnName), QString());                                                                                        \
	}

// column names can be empty in case no columns were used before save
// the actual pointers to the x- and y-columns are restored in Project::load()
#define READ_COLUMN(columnName)                                                                                                                                \
	{                                                                                                                                                          \
		str = attribs.value(QStringLiteral(#columnName)).toString();                                                                                           \
		d->columnName##Path = str;                                                                                                                             \
	}

#define READ_INT_VALUE_DIRECT(name, var, type)                                                                                                                 \
	{                                                                                                                                                          \
		str = attribs.value(QStringLiteral(name)).toString();                                                                                                  \
		if (str.isEmpty())                                                                                                                                     \
			reader->raiseMissingAttributeWarning(QStringLiteral(name));                                                                                        \
		else                                                                                                                                                   \
			var = static_cast<type>(str.toInt());                                                                                                              \
	}

#define READ_INT_VALUE(name, var, type) READ_INT_VALUE_DIRECT(name, d->var, type)

#define READ_DOUBLE_VALUE(name, var)                                                                                                                           \
	{                                                                                                                                                          \
		str = attribs.value(QStringLiteral(name)).toString();                                                                                                  \
		if (str.isEmpty())                                                                                                                                     \
			reader->raiseMissingAttributeWarning(QStringLiteral(name));                                                                                        \
		else                                                                                                                                                   \
			d->var = str.toDouble();                                                                                                                           \
	}

#define QGRAPHICSITEM_READ_DOUBLE_VALUE(name, Var)                                                                                                             \
	{                                                                                                                                                          \
		str = attribs.value(QStringLiteral(name)).toString();                                                                                                  \
		if (str.isEmpty())                                                                                                                                     \
			reader->raiseMissingAttributeWarning(QStringLiteral(name));                                                                                        \
		else                                                                                                                                                   \
			d->set##Var(str.toDouble());                                                                                                                       \
	}

#define READ_STRING_VALUE(name, var)                                                                                                                           \
	{ d->var = attribs.value(QLatin1String(name)).toString(); }

// used in Project::load()
#define RESTORE_COLUMN_POINTER(obj, col, Col)                                                                                                                  \
	if (!obj->col##Path().isEmpty()) {                                                                                                                         \
		for (auto* column : columns) {                                                                                                                         \
			if (!column)                                                                                                                                       \
				continue;                                                                                                                                      \
			if (column->path() == obj->col##Path()) {                                                                                                          \
				obj->set##Col(column);                                                                                                                         \
				break;                                                                                                                                         \
			}                                                                                                                                                  \
		}                                                                                                                                                      \
	}

#define WRITE_PATH(obj, name)                                                                                                                                  \
	if (obj) {                                                                                                                                                 \
		writer->writeAttribute(QLatin1String(#name), obj->path());                                                                                             \
	} else {                                                                                                                                                   \
		writer->writeAttribute(QLatin1String(#name), QString());                                                                                               \
	}

#define READ_PATH(name)                                                                                                                                        \
	{                                                                                                                                                          \
		str = attribs.value(QLatin1String(#name)).toString();                                                                                                  \
		d->name##Path = str;                                                                                                                                   \
	}

#define RESTORE_POINTER(obj, name, Name, Type, list)                                                                                                           \
	if (!obj->name##Path().isEmpty()) {                                                                                                                        \
		for (auto* aspect : list) {                                                                                                                            \
			if (aspect->path() == obj->name##Path()) {                                                                                                         \
				auto a = dynamic_cast<Type*>(aspect);                                                                                                          \
				if (!a)                                                                                                                                        \
					continue;                                                                                                                                  \
				obj->set##Name(a);                                                                                                                             \
				break;                                                                                                                                         \
			}                                                                                                                                                  \
		}                                                                                                                                                      \
	}

#endif // MACROS_H
