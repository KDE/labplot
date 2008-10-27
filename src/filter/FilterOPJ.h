/***************************************************************************
    File                 : FilterOPJ.h
    Project              : LabPlot
    --------------------------------------------------------------------
    Copyright            : (C) 2008 by Stefan Gerlach
    Email (use @ for *)  : stefan.gerlach*uni-konstanz.de
    Description          : OPJ import/export filter
                           
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

#ifndef FILTEROPJ_H
#define FILTEROPJ_H

#include <QString>
#include "../kdefrontend/MainWin.h"
#include "../elements/Symbol.h"

class FilterOPJ
{
public:
	FilterOPJ(MainWin *mw, QString filename);
	int import();
	void setSymbolType(Symbol *symbol,int type);
	Qt::PenStyle translateOriginLineStyle(int linestyle) const;
	QColor translateOriginColor(int color) const;
	QString translateOriginColType(int type) const;
	QString parseOriginText(const QString &str) const;
	QString parseOriginTags(const QString &str) const;
private:
	MainWin *mw;
	QString filename;
};

#endif //FILTEROPJ_H
