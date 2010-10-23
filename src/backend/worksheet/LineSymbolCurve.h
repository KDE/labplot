/***************************************************************************
    File                 : LineSymbolCurve.h
    Project              : LabPlot/SciDAVis
    Description          : A curve drawn as line and/or symbols
    --------------------------------------------------------------------
    Copyright            : (C) 2009 Tilman Benkert (thzs*gmx.net)
    Copyright            : (C) 2010 Alexander Semke (alexander.semke*web.de)
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

class LineSymbolCurvePrivate;
class LineSymbolCurve: public AbstractWorksheetElement {
	Q_OBJECT

	public:
		enum LineType {Line, StepsLeft, StepsRight, DropLineVertical, DropLineHorizontal, Segments2, Segments3};
		static QStringList lineTypes();
		
		LineSymbolCurve(const QString &name);
		virtual ~LineSymbolCurve();

		virtual QGraphicsItem *graphicsItem() const;

		POINTER_D_ACCESSOR_DECL(const AbstractColumn, xColumn, XColumn);
		POINTER_D_ACCESSOR_DECL(const AbstractColumn, yColumn, YColumn);
		
// 		BASIC_D_ACCESSOR_DECL(bool, lineVisible, LineVisible);
		BASIC_D_ACCESSOR_DECL(LineType, lineType, LineType);
		CLASS_D_ACCESSOR_DECL(QPen, linePen, LinePen);
		BASIC_D_ACCESSOR_DECL(qreal, lineOpacity, LineOpacity);
		
// 		BASIC_D_ACCESSOR_DECL(bool, symbolsVisible, SymbolsVisible);
		BASIC_D_ACCESSOR_DECL(qreal, symbolsOpacity, SymbolsOpacity);
		BASIC_D_ACCESSOR_DECL(qreal, symbolRotationAngle, SymbolRotationAngle);
		BASIC_D_ACCESSOR_DECL(qreal, symbolSize, SymbolSize);
		BASIC_D_ACCESSOR_DECL(qreal, symbolAspectRatio, SymbolAspectRatio);
		CLASS_D_ACCESSOR_DECL(QString, symbolTypeId, SymbolTypeId);
		CLASS_D_ACCESSOR_DECL(QBrush, symbolsBrush, SymbolsBrush);
		CLASS_D_ACCESSOR_DECL(QPen, symbolsPen, SymbolsPen);
		

		//TODO: signal/slot connections with columns

		virtual void setVisible(bool on);
		virtual bool isVisible() const;

		typedef AbstractWorksheetElement BaseClass;
		typedef LineSymbolCurvePrivate Private;

	public slots:
		virtual void retransform();
		virtual void handlePageResize(double horizontalRatio, double verticalRatio);

	protected:
		LineSymbolCurve(const QString &name, LineSymbolCurvePrivate *dd);
		LineSymbolCurvePrivate * const d_ptr;

	private:
    	Q_DECLARE_PRIVATE(LineSymbolCurve)
};

#endif
