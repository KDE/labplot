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


#endif // MACROS_H
