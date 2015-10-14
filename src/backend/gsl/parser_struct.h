/***************************************************************************
    File                 : parser_struct.h
    Project              : LabPlot
    Description          : definition of structs used in the expression parser
    --------------------------------------------------------------------
	Copyright            : (C) 2008-2014 by Stefan Gerlach (stefan.gerlach@uni-konstanz.de)
	Copyright            : (C) 2014 by Alexander Semke (alexander.semke@web.de)

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

#ifndef PARSER_STRUCT_H
#define PARSER_STRUCT_H

struct con {
	char const *name;
	double value;
};

struct func {
	char const *name;
/*#ifdef HAVE_SOLARIS
	double (*fnct)(double);
#else*/
	double (*fnct)();
/*#endif*/
};

#endif /* PARSER_STRUCT_H */
