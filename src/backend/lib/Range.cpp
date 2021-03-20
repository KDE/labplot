/***************************************************************************
    File                 : Range.cpp
    Project              : LabPlot
    --------------------------------------------------------------------
    Copyright            : (C) 2021 Stefan Gerlach (stefan.gerlach@uni.kn)
    Description          : basic data range class

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

#include "Range.h"

const QStringList RangeT::scaleNames{ i18n("Linear"), i18n("Log10"), i18n("Log2"), i18n("Ln"), i18n("Sqrt"), i18n("Square"), i18n("Inverse") };

