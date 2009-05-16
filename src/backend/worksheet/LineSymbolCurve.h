/***************************************************************************
    File                 : LineSymbolCurve.h
    Project              : LabPlot/SciDAVis
    Description          : A curve drawn as line and/or symbols
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

#ifndef LINESYMBOLCURVE_H
#define LINESYMBOLCURVE_H

#include "worksheet/AbstractWorksheetElement.h"
#include "lib/macros.h"
#include "core/AbstractColumn.h"

class LineSymbolCurve: public AbstractWorksheetElement {
	Q_OBJECT

	public:
		LineSymbolCurve(const QString &name);
		virtual ~LineSymbolCurve();

		virtual QGraphicsItem *graphicsItem() const;

		virtual void setZValue(qreal z);
		virtual qreal zValue() const;

		BASIC_D_ACCESSOR_DECL(bool, lineVisible, LineVisible);
		BASIC_D_ACCESSOR_DECL(bool, symbolsVisible, SymbolsVisible);
		POINTER_D_ACCESSOR_DECL(const AbstractColumn, xColumn, XColumn);
		POINTER_D_ACCESSOR_DECL(const AbstractColumn, yColumn, YColumn);

		//TODO: all style related stuff (line widths, color, symbol size, etc...)
		//TODO: signal/slot connections with columns

#if 0
    	virtual QRectF boundingRect() const;
		virtual bool contains(const QPointF &position) const;
#endif

		virtual void setVisible(bool on);
		virtual bool isVisible() const;

	public slots:
		virtual void retransform();

	public:
		class Private;
	private:
		friend class Private;
		Private * const d;
};

#endif


