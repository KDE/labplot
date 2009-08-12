/***************************************************************************
    File                 : DropLineCurve.h
    Project              : LabPlot/SciDAVis
    Description          : A curve drawn as drop lines and/or symbols
    --------------------------------------------------------------------
    Copyright            : (C) 2009 Tilman Benkert (thzs*gmx.net)
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

#ifndef DROPLINECURVE_H
#define DROPLINECURVE_H

#include "worksheet/LineSymbolCurve.h"
#include "lib/macros.h"
#include "core/AbstractColumn.h"

class DropLineCurvePrivate;
class DropLineCurve: public LineSymbolCurve {
	Q_OBJECT

	public:
		DropLineCurve(const QString &name);
		virtual ~DropLineCurve();

	typedef DropLineCurvePrivate Private;

	protected:
		DropLineCurve(const QString &name, DropLineCurvePrivate *dd);

	private:
    	Q_DECLARE_PRIVATE(DropLineCurve)
};

#endif


