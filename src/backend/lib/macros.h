/***************************************************************************
    File                 : macros.h
    Project              : LabPlot
    Description          : Various preprocessor macros
    --------------------------------------------------------------------
    Copyright            : (C) 2008 Tilman Benkert (thzs@gmx.net)
    Copyright            : (C) 2013-2015 Alexander Semke (alexander.semke@web.de)
    Copyright            : (C) 2016-2020 Stefan Gerlach (stefan.gerlach@uni.kn)

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

#ifndef MACROS_H
#define MACROS_H

#include <QApplication>
#include <QMetaEnum>

// C++ style warning (works on Windows)
#include <iostream>
#define WARN(x) std::cout << x << std::endl;

#ifndef NDEBUG
#include <QDebug>
#define QDEBUG(x) qDebug() << x;
// C++ style debugging (works on Windows)
#include <iomanip>
#define DEBUG(x) std::cout << x << std::endl;
#else
#define QDEBUG(x) {}
#define DEBUG(x) {}
#endif

#if QT_VERSION < 0x050700
template <class T>
constexpr std::add_const_t<T>& qAsConst(T& t) noexcept {
    return t;
}
#endif

#define WAIT_CURSOR QApplication::setOverrideCursor(QCursor(Qt::WaitCursor))
#define RESET_CURSOR QApplication::restoreOverrideCursor()

#define UTF8_QSTRING(str) QString::fromUtf8(str)
#define STDSTRING(qstr) qstr.toUtf8().constData()

#define ENUM_TO_STRING(class, enum, value) \
    (class::staticMetaObject.enumerator(class::staticMetaObject.indexOfEnumerator(#enum)).valueToKey(static_cast<int>(value)))
#define ENUM_COUNT(class, enum) \
	(class::staticMetaObject.enumerator(class::staticMetaObject.indexOfEnumerator(#enum)).keyCount())

// define number locale from setting (using system locale when QLocale::AnyLanguage)
#define SET_NUMBER_LOCALE \
QLocale::Language numberLocaleLanguage = static_cast<QLocale::Language>(KSharedConfig::openConfig()->group("Settings_General").readEntry( QLatin1String("DecimalSeparatorLocale"), static_cast<int>(QLocale::Language::AnyLanguage) )); \
QLocale numberLocale(numberLocaleLanguage == QLocale::AnyLanguage ? QLocale() : numberLocaleLanguage); \
if (numberLocale.language() == QLocale::Language::C) \
	numberLocale.setNumberOptions(QLocale::DefaultNumberOptions);

//////////////////////// LineEdit Access ///////////////////////////////
#define SET_INT_FROM_LE(var, le) { \
	bool ok; \
	SET_NUMBER_LOCALE \
	const int tmp = numberLocale.toInt((le)->text(), &ok); \
	if (ok) \
		var = tmp; \
}

#define SET_DOUBLE_FROM_LE(var, le) { \
	bool ok; \
	SET_NUMBER_LOCALE \
	const double tmp = numberLocale.toDouble((le)->text(), &ok); \
	if (ok) \
		var = tmp; \
}

//////////////////////// Accessor ///////////////////////////////

#define BASIC_ACCESSOR(type, var, method, Method) \
	type method() const { return var; }; \
	void set ## Method(const type value) { var = value; }
#define CLASS_ACCESSOR(type, var, method, Method) \
	type method() const { return var; }; \
	void set ## Method(const type & value) { var = value; }

#define BASIC_D_ACCESSOR_DECL(type, method, Method) \
	type method() const; \
	void set ## Method(const type value);

#define BASIC_D_ACCESSOR_IMPL(classname, type, method, Method, var) \
	void classname::set ## Method(const type value) \
	{ \
		d->var = value; \
	} \
	type classname::method() const \
	{ \
		return d->var; \
	}

#define BASIC_D_READER_IMPL(classname, type, method, var) \
	type classname::method() const \
	{ \
		return d->var; \
	}

#define BASIC_SHARED_D_READER_IMPL(classname, type, method, var) \
	type classname::method() const \
	{ \
		Q_D(const classname); \
		return d->var; \
	}

#define CLASS_D_ACCESSOR_DECL(type, method, Method) \
	type method() const; \
	void set ## Method(const type & value);

#define CLASS_D_ACCESSOR_IMPL(classname, type, method, Method, var) \
	void classname::set ## Method(const type & value) \
	{ \
		d->var = value; \
	} \
	type classname::method() const \
	{ \
		return d->var; \
	}

#define CLASS_D_READER_IMPL(classname, type, method, var) \
	type classname::method() const \
	{ \
		return d->var; \
	}

#define CLASS_SHARED_D_READER_IMPL(classname, type, method, var) \
	type classname::method() const \
	{ \
		Q_D(const classname); \
		return d->var; \
	}

#define POINTER_D_ACCESSOR_DECL(type, method, Method) \
	type *method() const; \
	void set ## Method(type *ptr);

#define FLAG_D_ACCESSOR_DECL(Method) \
	bool is ## Method() const; \
	bool has ## Method() const; \
	void set ## Method(const bool value = true); \
	void enable ## Method(const bool value = true);

#define FLAG_D_ACCESSOR_IMPL(classname, Method, var) \
	void classname::set ## Method(const bool value) \
	{ \
		d->var = value; \
	} \
	void classname::enable ## Method(const bool value) \
	{ \
		d->var = value; \
	} \
	bool classname::is ## Method() const \
	{ \
		return d->var; \
	} \
	bool classname::has ## Method() const \
	{ \
		return d->var; \
	}

//////////////////////// Standard Setter /////////////////////

#define STD_SETTER_CMD_IMPL(class_name, cmd_name, value_type, field_name) \
class class_name ## cmd_name ## Cmd: public StandardSetterCmd<class_name::Private, value_type> { \
	public: \
		class_name ## cmd_name ## Cmd(class_name::Private *target, value_type newValue, const KLocalizedString &description) \
			: StandardSetterCmd<class_name::Private, value_type>(target, &class_name::Private::field_name, newValue, description) {} \
};

#define STD_SETTER_CMD_IMPL_F(class_name, cmd_name, value_type, field_name, finalize_method) \
class class_name ## cmd_name ## Cmd: public StandardSetterCmd<class_name::Private, value_type> { \
	public: \
		class_name ## cmd_name ## Cmd(class_name::Private *target, value_type newValue, const KLocalizedString &description) \
			: StandardSetterCmd<class_name::Private, value_type>(target, &class_name::Private::field_name, newValue, description) {} \
		virtual void finalize() override { m_target->finalize_method(); } \
};

// setter class with finalize() and signal emitting.
#define STD_SETTER_CMD_IMPL_S(class_name, cmd_name, value_type, field_name) \
class class_name ## cmd_name ## Cmd: public StandardSetterCmd<class_name::Private, value_type> { \
	public: \
		class_name ## cmd_name ## Cmd(class_name::Private *target, value_type newValue, const KLocalizedString &description) \
			: StandardSetterCmd<class_name::Private, value_type>(target, &class_name::Private::field_name, newValue, description) {} \
		virtual void finalize() override { emit m_target->q->field_name##Changed(m_target->*m_field); } \
};

#define STD_SETTER_CMD_IMPL_F_S(class_name, cmd_name, value_type, field_name, finalize_method) \
class class_name ## cmd_name ## Cmd: public StandardSetterCmd<class_name::Private, value_type> { \
	public: \
		class_name ## cmd_name ## Cmd(class_name::Private *target, value_type newValue, const KLocalizedString &description) \
			: StandardSetterCmd<class_name::Private, value_type>(target, &class_name::Private::field_name, newValue, description) {} \
		virtual void finalize() override { m_target->finalize_method(); emit m_target->q->field_name##Changed(m_target->*m_field); } \
};

// setter class with finalize() and signal emitting for changing several properties in one single step (embedded in beginMacro/endMacro)
#define STD_SETTER_CMD_IMPL_M_F_S(class_name, cmd_name, value_type, field_name, finalize_method) \
class class_name ## cmd_name ## Cmd: public StandardMacroSetterCmd<class_name::Private, value_type> { \
	public: \
		class_name ## cmd_name ## Cmd(class_name::Private *target, value_type newValue, const KLocalizedString &description) \
			: StandardMacroSetterCmd<class_name::Private, value_type>(target, &class_name::Private::field_name, newValue, description) {} \
		virtual void finalize() override { m_target->finalize_method(); emit m_target->q->field_name##Changed(m_target->*m_field); } \
		virtual void finalizeUndo() override { emit m_target->q->field_name##Changed(m_target->*m_field); } \
};

#define STD_SETTER_CMD_IMPL_I(class_name, cmd_name, value_type, field_name, init_method) \
class class_name ## cmd_name ## Cmd: public StandardSetterCmd<class_name::Private, value_type> { \
	public: \
		class_name ## cmd_name ## Cmd(class_name::Private *target, value_type newValue, const KLocalizedString &description) \
			: StandardSetterCmd<class_name::Private, value_type>(target, &class_name::Private::field_name, newValue, description) {} \
		virtual void initialize() { m_target->init_method(); } \
};

#define STD_SETTER_CMD_IMPL_IF(class_name, cmd_name, value_type, field_name, init_method, finalize_method) \
class class_name ## cmd_name ## Cmd: public StandardSetterCmd<class_name::Private, value_type> { \
	public: \
		class_name ## cmd_name ## Cmd(class_name::Private *target, value_type newValue, const KLocalizedString &description) \
			: StandardSetterCmd<class_name::Private, value_type>(target, &class_name::Private::field_name, newValue, description) {} \
		virtual void initialize() { m_target->init_method(); } \
		virtual void finalize() { m_target->finalize_method(); } \
};

#define STD_SWAP_METHOD_SETTER_CMD_IMPL(class_name, cmd_name, value_type, method_name) \
class class_name ## cmd_name ## Cmd: public StandardSwapMethodSetterCmd<class_name::Private, value_type> { \
	public: \
		class_name ## cmd_name ## Cmd(class_name::Private *target, value_type newValue, const KLocalizedString &description) \
			: StandardSwapMethodSetterCmd<class_name::Private, value_type>(target, &class_name::Private::method_name, newValue, description) {} \
};

#define STD_SWAP_METHOD_SETTER_CMD_IMPL_F(class_name, cmd_name, value_type, method_name, finalize_method) \
class class_name ## cmd_name ## Cmd: public StandardSwapMethodSetterCmd<class_name::Private, value_type> { \
	public: \
		class_name ## cmd_name ## Cmd(class_name::Private *target, value_type newValue, const KLocalizedString &description) \
			: StandardSwapMethodSetterCmd<class_name::Private, value_type>(target, &class_name::Private::method_name, newValue, description) {} \
		virtual void finalize() override { m_target->finalize_method(); } \
};

#define STD_SWAP_METHOD_SETTER_CMD_IMPL_I(class_name, cmd_name, value_type, method_name, init_method) \
class class_name ## cmd_name ## Cmd: public StandardSwapMethodSetterCmd<class_name::Private, value_type> { \
	public: \
		class_name ## cmd_name ## Cmd(class_name::Private *target, value_type newValue, const KLocalizedString &description) \
			: StandardSwapMethodSetterCmd<class_name::Private, value_type>(target, &class_name::Private::method_name, newValue, description) {} \
		virtual void initialize() { m_target->init_method(); } \
};

#define STD_SWAP_METHOD_SETTER_CMD_IMPL_IF(class_name, cmd_name, value_type, method_name, init_method, finalize_method) \
class class_name ## cmd_name ## Cmd: public StandardSwapMethodSetterCmd<class_name::Private, value_type> { \
	public: \
		class_name ## cmd_name ## Cmd(class_name::Private *target, value_type newValue, const KLocalizedString &description) \
			: StandardSwapMethodSetterCmd<class_name::Private, value_type>(target, &class_name::Private::method_name, newValue, description) {} \
		virtual void initialize() { m_target->init_method(); } \
		virtual void finalize() { m_target->finalize_method(); } \
};


//////////////////////// XML - serialization/deserialization /////

//QColor
#define WRITE_QCOLOR(color) 												\
do { 																		\
writer->writeAttribute( "color_r", QString::number(color.red()) );			\
writer->writeAttribute( "color_g", QString::number(color.green()) ); 		\
writer->writeAttribute( "color_b", QString::number(color.blue()) ); 		\
} while (0)

#define READ_QCOLOR(color) 													\
do {																		\
str = attribs.value("color_r").toString();									\
if(str.isEmpty())															\
	reader->raiseWarning(attributeWarning.subs("color_r").toString());				\
else																		\
	color.setRed( str.toInt() );											\
																			\
str = attribs.value("color_g").toString();									\
if(str.isEmpty())															\
	reader->raiseWarning(attributeWarning.subs("color_g").toString());				\
else																		\
	color.setGreen( str.toInt() );											\
																			\
str = attribs.value("color_b").toString();									\
if(str.isEmpty())															\
	reader->raiseWarning(attributeWarning.subs("color_b").toString());				\
else																		\
	color.setBlue( str.toInt() );											\
} while(0)

//QPen
#define WRITE_QPEN(pen) 													\
do { 																		\
writer->writeAttribute( "style", QString::number(pen.style()) ); 			\
writer->writeAttribute( "color_r", QString::number(pen.color().red()) ); 	\
writer->writeAttribute( "color_g", QString::number(pen.color().green()) ); 	\
writer->writeAttribute( "color_b", QString::number(pen.color().blue()) ); 	\
writer->writeAttribute( "width", QString::number(pen.widthF()) ); 			\
} while (0)

#define READ_QPEN(pen) 														\
do {																		\
str = attribs.value("style").toString(); 									\
if(str.isEmpty())															\
	reader->raiseWarning(attributeWarning.subs("style").toString());					\
else																		\
	pen.setStyle( (Qt::PenStyle)str.toInt() );								\
																			\
QColor color;																\
str = attribs.value("color_r").toString();									\
if(str.isEmpty())															\
	reader->raiseWarning(attributeWarning.subs("color_r").toString());				\
else																		\
	color.setRed( str.toInt() );											\
																			\
str = attribs.value("color_g").toString();									\
if(str.isEmpty())															\
	reader->raiseWarning(attributeWarning.subs("color_g").toString());				\
else																		\
	color.setGreen( str.toInt() );											\
																			\
str = attribs.value("color_b").toString();									\
if(str.isEmpty())															\
	reader->raiseWarning(attributeWarning.subs("color_b").toString());				\
else																		\
	color.setBlue( str.toInt() );											\
																			\
pen.setColor(color);														\
																			\
str = attribs.value("width").toString();									\
if(str.isEmpty())															\
	reader->raiseWarning(attributeWarning.subs("width").toString());					\
else																		\
	pen.setWidthF( str.toDouble() );										\
} while(0)

//QFont
#define WRITE_QFONT(font) 													\
do {																		\
writer->writeAttribute( "fontFamily", font.family() );						\
writer->writeAttribute( "fontSize", QString::number(font.pixelSize()) );	\
writer->writeAttribute( "fontPointSize", QString::number(font.pointSize()));\
writer->writeAttribute( "fontWeight", QString::number(font.weight()) );		\
writer->writeAttribute( "fontItalic", QString::number(font.italic()) );		\
} while(0)

#define READ_QFONT(font) 													\
do {																		\
str = attribs.value("fontFamily").toString();								\
if(str.isEmpty())															\
	reader->raiseWarning(attributeWarning.subs("fontFamily").toString());				\
else																		\
	font.setFamily( str );													\
																			\
str = attribs.value("fontSize").toString();									\
if(str.isEmpty())															\
	reader->raiseWarning(attributeWarning.subs("fontSize").toString());				\
else {																		\
	int size = str.toInt();													\
	if (size != -1)															\
		font.setPixelSize(size);											\
}																			\
																			\
str = attribs.value("fontPointSize").toString();							\
if(str.isEmpty())															\
	reader->raiseWarning(attributeWarning.subs("fontPointSize").toString());			\
else {																		\
	int size = str.toInt();													\
	if (size != -1)															\
		font.setPointSize(size);											\
}																			\
																			\
str = attribs.value("fontWeight").toString();								\
if(str.isEmpty())															\
	reader->raiseWarning(attributeWarning.subs("fontWeight").toString());				\
else																		\
	font.setWeight( str.toInt() );											\
																			\
str = attribs.value("fontItalic").toString();								\
if(str.isEmpty())															\
	reader->raiseWarning(attributeWarning.subs("fontItalic").toString());				\
else																		\
	font.setItalic( str.toInt() );											\
} while(0)

//QBrush
#define WRITE_QBRUSH(brush) 													\
do {																			\
writer->writeAttribute("brush_style", QString::number(brush.style()) );			\
writer->writeAttribute("brush_color_r", QString::number(brush.color().red()));	\
writer->writeAttribute("brush_color_g", QString::number(brush.color().green()));\
writer->writeAttribute("brush_color_b", QString::number(brush.color().blue()));	\
} while(0)

#define READ_QBRUSH(brush) 													\
do {																		\
str = attribs.value("brush_style").toString();								\
if(str.isEmpty())															\
	reader->raiseWarning(attributeWarning.subs("brush_style").toString());			\
else																		\
	brush.setStyle( (Qt::BrushStyle)str.toInt() );							\
																			\
QColor color;																\
str = attribs.value("brush_color_r").toString();							\
if(str.isEmpty())															\
	reader->raiseWarning(attributeWarning.subs("brush_color_r").toString());			\
else																		\
	color.setRed( str.toInt() );											\
																			\
str = attribs.value("brush_color_g").toString();							\
if(str.isEmpty())															\
	reader->raiseWarning(attributeWarning.subs("brush_color_g").toString());			\
else																		\
	color.setGreen( str.toInt() );											\
																			\
str = attribs.value("brush_color_b").toString();							\
if(str.isEmpty())															\
	reader->raiseWarning(attributeWarning.subs("brush_color_b").toString());			\
else																		\
	color.setBlue( str.toInt() );											\
																			\
brush.setColor(color);														\
} while(0)



//Column
#define WRITE_COLUMN(column, columnName) 											\
do {																				\
if (column){																		\
	writer->writeAttribute( #columnName, column->path() );							\
} else {																			\
	writer->writeAttribute( #columnName, QString() );										\
}																					\
} while(0)

//column names can be empty in case no columns were used before save
//the actual pointers to the x- and y-columns are restored in Project::load()
#define READ_COLUMN(columnName)														\
do {																				\
	str = attribs.value(#columnName).toString();									\
	d->columnName ##Path = str;														\
} while(0)

#define READ_INT_VALUE(name, var, type) \
str = attribs.value(name).toString(); \
if (str.isEmpty()) \
	reader->raiseWarning(attributeWarning.subs(name).toString()); \
else \
	d->var = (type)str.toInt();

#define READ_DOUBLE_VALUE(name, var) \
str = attribs.value(name).toString(); \
if (str.isEmpty()) \
	reader->raiseWarning(attributeWarning.subs(name).toString()); \
else \
	d->var = str.toDouble();

#define READ_STRING_VALUE(name, var) \
str = attribs.value(name).toString(); \
if (str.isEmpty()) \
	reader->raiseWarning(attributeWarning.subs(name).toString()); \
else \
	d->var = str;

//used in Project::load()
#define RESTORE_COLUMN_POINTER(obj, col, Col) 										\
do {																				\
if (!obj->col ##Path().isEmpty()) {													\
	for (Column* column : columns) {												\
		if (!column) continue;														\
		if (column->path() == obj->col ##Path()) {									\
 			obj->set## Col(column);													\
			break;				 													\
		}																			\
	}																				\
}																					\
} while(0)



#define WRITE_PATH(obj, name) 														\
do {																				\
if (obj){																			\
	writer->writeAttribute( #name, obj->path() );									\
} else {																			\
	writer->writeAttribute( #name, QString() );											\
}																					\
} while(0)

#define READ_PATH(name)																\
do {																				\
	str = attribs.value(#name).toString();											\
	d->name ##Path = str;															\
} while(0)

#define RESTORE_POINTER(obj, name, Name, Type, list) 								\
do {																				\
if (!obj->name ##Path().isEmpty()) {												\
	for (AbstractAspect* aspect : list) {											\
		if (aspect->path() == obj->name ##Path()) {									\
			auto a = dynamic_cast<Type*>(aspect);									\
			if (!a) continue;														\
 			obj->set## Name(a);														\
			break;				 													\
		}																			\
	}																				\
}																					\
} while(0)

#endif // MACROS_H
