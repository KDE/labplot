/***************************************************************************
    File                 : macros.h
    Project              : SciDAVis
    Description          : Various preprocessor macros
    --------------------------------------------------------------------
    Copyright            : (C) 2008 Tilman Benkert (thzs*gmx.net)
    Copyright            : (C) 2008 Knut Franke (knut.franke*gmx.de)
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

#ifndef MACROS_H
#define MACROS_H

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
	void set ## Method(const bool value=true); \
	void enable ## Method(const bool value=true);

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

#define WAIT_CURSOR QApplication::setOverrideCursor(QCursor(Qt::WaitCursor))
#define RESET_CURSOR QApplication::restoreOverrideCursor()

#define STD_SETTER_CMD_IMPL(class_name, cmd_name, value_type, field_name) \
class class_name ## cmd_name ## Cmd: public StandardSetterCmd<class_name::Private, value_type> { \
	public: \
		class_name ## cmd_name ## Cmd(class_name::Private *target, Loki::TypeTraits<value_type>::ParameterType newValue, const QString &description) \
			: StandardSetterCmd<class_name::Private, value_type>(target, &class_name::Private::field_name, newValue, description) {} \
};

#define STD_SETTER_CMD_IMPL_F(class_name, cmd_name, value_type, field_name, finalize_method) \
class class_name ## cmd_name ## Cmd: public StandardSetterCmd<class_name::Private, value_type> { \
	public: \
		class_name ## cmd_name ## Cmd(class_name::Private *target, Loki::TypeTraits<value_type>::ParameterType newValue, const QString &description) \
			: StandardSetterCmd<class_name::Private, value_type>(target, &class_name::Private::field_name, newValue, description) {} \
		virtual void finalize() { m_target->finalize_method(); } \
};

// setter class with finalize() and signal emmiting.
#define STD_SETTER_CMD_IMPL_F_S(class_name, cmd_name, value_type, field_name, finalize_method) \
class class_name ## cmd_name ## Cmd: public StandardSetterCmd<class_name::Private, value_type> { \
	public: \
		class_name ## cmd_name ## Cmd(class_name::Private *target, Loki::TypeTraits<value_type>::ParameterType newValue, const QString &description) \
			: StandardSetterCmd<class_name::Private, value_type>(target, &class_name::Private::field_name, newValue, description) {} \
		virtual void finalize() { m_target->finalize_method(); emit m_target->q->field_name##Changed(m_target->*m_field); } \
};

#define STD_SETTER_CMD_IMPL_I(class_name, cmd_name, value_type, field_name, init_method) \
class class_name ## cmd_name ## Cmd: public StandardSetterCmd<class_name::Private, value_type> { \
	public: \
		class_name ## cmd_name ## Cmd(class_name::Private *target, Loki::TypeTraits<value_type>::ParameterType newValue, const QString &description) \
			: StandardSetterCmd<class_name::Private, value_type>(target, &class_name::Private::field_name, newValue, description) {} \
		virtual void initialize() { m_target->init_method(); } \
};

#define STD_SETTER_CMD_IMPL_IF(class_name, cmd_name, value_type, field_name, init_method, finalize_method) \
class class_name ## cmd_name ## Cmd: public StandardSetterCmd<class_name::Private, value_type> { \
	public: \
		class_name ## cmd_name ## Cmd(class_name::Private *target, Loki::TypeTraits<value_type>::ParameterType newValue, const QString &description) \
			: StandardSetterCmd<class_name::Private, value_type>(target, &class_name::Private::field_name, newValue, description) {} \
		virtual void initialize() { m_target->init_method(); } \
		virtual void finalize() { m_target->finalize_method(); } \
};

#define STD_SWAP_METHOD_SETTER_CMD_IMPL(class_name, cmd_name, value_type, method_name) \
class class_name ## cmd_name ## Cmd: public StandardSwapMethodSetterCmd<class_name::Private, value_type> { \
	public: \
		class_name ## cmd_name ## Cmd(class_name::Private *target, Loki::TypeTraits<value_type>::ParameterType newValue, const QString &description) \
			: StandardSwapMethodSetterCmd<class_name::Private, value_type>(target, &class_name::Private::method_name, newValue, description) {} \
};

#define STD_SWAP_METHOD_SETTER_CMD_IMPL_F(class_name, cmd_name, value_type, method_name, finalize_method) \
class class_name ## cmd_name ## Cmd: public StandardSwapMethodSetterCmd<class_name::Private, value_type> { \
	public: \
		class_name ## cmd_name ## Cmd(class_name::Private *target, Loki::TypeTraits<value_type>::ParameterType newValue, const QString &description) \
			: StandardSwapMethodSetterCmd<class_name::Private, value_type>(target, &class_name::Private::method_name, newValue, description) {} \
		virtual void finalize() { m_target->finalize_method(); } \
};

#define STD_SWAP_METHOD_SETTER_CMD_IMPL_I(class_name, cmd_name, value_type, method_name, init_method) \
class class_name ## cmd_name ## Cmd: public StandardSwapMethodSetterCmd<class_name::Private, value_type> { \
	public: \
		class_name ## cmd_name ## Cmd(class_name::Private *target, Loki::TypeTraits<value_type>::ParameterType newValue, const QString &description) \
			: StandardSwapMethodSetterCmd<class_name::Private, value_type>(target, &class_name::Private::method_name, newValue, description) {} \
		virtual void initialize() { m_target->init_method(); } \
};

#define STD_SWAP_METHOD_SETTER_CMD_IMPL_IF(class_name, cmd_name, value_type, method_name, init_method, finalize_method) \
class class_name ## cmd_name ## Cmd: public StandardSwapMethodSetterCmd<class_name::Private, value_type> { \
	public: \
		class_name ## cmd_name ## Cmd(class_name::Private *target, Loki::TypeTraits<value_type>::ParameterType newValue, const QString &description) \
			: StandardSwapMethodSetterCmd<class_name::Private, value_type>(target, &class_name::Private::method_name, newValue, description) {} \
		virtual void initialize() { m_target->init_method(); } \
		virtual void finalize() { m_target->finalize_method(); } \
};


//xml-serialization/deserialization
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
	reader->raiseWarning(attributeWarning.arg("'color_r'"));				\
else																		\
	color.setRed( str.toInt() );											\
																			\
str = attribs.value("color_g").toString();									\
if(str.isEmpty())															\
	reader->raiseWarning(attributeWarning.arg("'color_g'"));				\
else																		\
	color.setGreen( str.toInt() );											\
																			\
str = attribs.value("color_b").toString();									\
if(str.isEmpty())															\
	reader->raiseWarning(attributeWarning.arg("'color_b'"));				\
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
	reader->raiseWarning(attributeWarning.arg("'style'"));					\
else																		\
	pen.setStyle( (Qt::PenStyle)str.toInt() );								\
																			\
QColor color;																\
str = attribs.value("color_r").toString();									\
if(str.isEmpty())															\
	reader->raiseWarning(attributeWarning.arg("'color_r'"));				\
else																		\
	color.setRed( str.toInt() );											\
																			\
str = attribs.value("color_g").toString();									\
if(str.isEmpty())															\
	reader->raiseWarning(attributeWarning.arg("'color_g'"));				\
else																		\
	color.setGreen( str.toInt() );											\
																			\
str = attribs.value("color_b").toString();									\
if(str.isEmpty())															\
	reader->raiseWarning(attributeWarning.arg("'color_b'"));				\
else																		\
	color.setBlue( str.toInt() );											\
																			\
pen.setColor(color);														\
																			\
str = attribs.value("width").toString();									\
if(str.isEmpty())															\
	reader->raiseWarning(attributeWarning.arg("'width'"));					\
else																		\
	pen.setWidthF( str.toDouble() );										\
} while(0)

//QFont
#define WRITE_QFONT(font) 													\
do {																		\
writer->writeAttribute( "fontFamily", font.family() );						\
writer->writeAttribute( "fontSize", QString::number(font.pointSize()) );	\
writer->writeAttribute( "fontWeight", QString::number(font.weight()) );		\
writer->writeAttribute( "fontItalic", QString::number(font.italic()) );		\
} while(0)

#define READ_QFONT(font) 													\
do {																		\
str = attribs.value("fontFamily").toString();								\
if(str.isEmpty())															\
	reader->raiseWarning(attributeWarning.arg("'fontFamily'"));				\
else																		\
	font.setFamily( str );													\
																			\
str = attribs.value("fontSize").toString();									\
if(str.isEmpty())															\
	reader->raiseWarning(attributeWarning.arg("'fontSize'"));				\
else																		\
	font.setPointSize( str.toInt() );										\
																			\
str = attribs.value("fontWeight").toString();								\
if(str.isEmpty())															\
	reader->raiseWarning(attributeWarning.arg("'fontWeight'"));				\
else																		\
	font.setWeight( str.toInt() );											\
																			\
str = attribs.value("fontItalic").toString();								\
if(str.isEmpty())															\
	reader->raiseWarning(attributeWarning.arg("'fontItalic'"));				\
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
	reader->raiseWarning(attributeWarning.arg("'brush_style'"));			\
else																		\
	brush.setStyle( (Qt::BrushStyle)str.toInt() );							\
																			\
QColor color;																\
str = attribs.value("brush_color_r").toString();							\
if(str.isEmpty())															\
	reader->raiseWarning(attributeWarning.arg("'brush_color_r'"));			\
else																		\
	color.setRed( str.toInt() );											\
																			\
str = attribs.value("brush_color_g").toString();							\
if(str.isEmpty())															\
	reader->raiseWarning(attributeWarning.arg("'brush_color_g'"));			\
else																		\
	color.setGreen( str.toInt() );											\
																			\
str = attribs.value("brush_color_b").toString();							\
if(str.isEmpty())															\
	reader->raiseWarning(attributeWarning.arg("'brush_color_b'"));			\
else																		\
	color.setBlue( str.toInt() );											\
																			\
brush.setColor(color);														\
} while(0)

#endif // MACROS_H
