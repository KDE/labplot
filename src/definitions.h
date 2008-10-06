/***************************************************************************
    File                 : definitions.h
    Project              : LabPlot
    --------------------------------------------------------------------
    Copyright            : (C) 2008 by Stefan Gerlach, Alexander Semke
    Email (use @ for *)  : stefan.gerlach*uni-konstanz.de, alexander.semke*web.de
    Description          : general definitions
                           
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

#ifndef DEFINITIONS_H
#define DEFINITIONS_H

// (QString, title, Title)	variable is m_title
#define ACCESS(type, name, Method) \
	type name() const { return m_ ## name; } \
	void set ## Method(const type value) { m_ ## name=value; }
// (m_transparent, Transparent)
#define ACCESSFLAG(var, Method) \
	bool is ## Method() const { return var; } \
	bool has ## Method() const { return var; } \
	void enable ## Method(const bool value=true) { var=value; } \
	void set ## Method(const bool value=true) { var=value; }

#endif // DEFINITIONS_H
