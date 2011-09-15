/***************************************************************************
    File                 : importitems.h
    Project              : LabPlot
    --------------------------------------------------------------------
    Copyright            : (C) 2008 by Stefan Gerlach
    Email (use @ for *)  : stefan.gerlach*uni-konstanz.de
    Description          : import items
                           
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

#ifndef IMPORTITEMS_H
#define IMPORTITEMS_H

#include "import.h"

QStringList separatoritems = (QStringList()<<"auto"<<","<<":");
QStringList commentitems = (QStringList()<<"#"<<"!"<<"//"<<"+"<<"c"<<":"<<";");
QStringList formatitems = (QStringList()<<"double"<<"float"<<"int (8 bit)"<<"int (16 bit)"<<"int (32 bit)"<<"int (64 bit)");
QStringList byteorderitems = (QStringList()<<"Big endian"<<"Little Endian");
#endif // IMPORTITEMS_H
