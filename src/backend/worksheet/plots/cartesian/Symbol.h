/***************************************************************************
    File                 : Symbol.h
    Project              : LabPlot
    Description          : Symbol
    --------------------------------------------------------------------
    Copyright            : (C) 2015-2020 Alexander Semke (alexander.semke@web.de)

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

#ifndef SYMBOL_H
#define SYMBOL_H

#include <QPainterPath>

class QString;

class Symbol {
public:
	enum class Style {NoSymbols, Circle, Square, EquilateralTriangle, RightTriangle, Bar, PeakedBar,
			SkewedBar, Diamond, Lozenge, Tie, TinyTie, Plus, Boomerang, SmallBoomerang,
			Star4, Star5, Line, Cross};

	static int stylesCount();
	static QPainterPath pathFromStyle(Symbol::Style);
	static QString nameFromStyle(Symbol::Style);
};

#endif
