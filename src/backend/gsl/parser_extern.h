/***************************************************************************
    File                 : parser_extern.h
    Project              : LabPlot
    Description          : some definitions for the linker
    --------------------------------------------------------------------
    Copyright            : (C) 2008-2014 by Stefan Gerlach (stefan.gerlach@uni.kn)
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

#ifndef PARSER_EXTERN_H
#define PARSER_EXTERN_H

extern "C" double parse(const char[]);
extern "C" int parse_errors();
extern "C" void init_table();
extern "C" void delete_table();
extern "C" void* assign_variable(const char* variable, double value);

extern "C" struct con _constants[];
extern "C" struct func _functions[];

#endif /* PARSER_EXTERN_H */
