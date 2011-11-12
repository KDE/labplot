/***************************************************************************
    File                 : PlotArea.h
    Project              : LabPlot/SciDAVis
    Description          : Plot area (for background filling and clipping).
    --------------------------------------------------------------------
    Copyright            : (C) 2009 Tilman Benkert (thzs*gmx.net)
    Copyright            : (C) 2011 by Alexander Semke (alexander.semke*web.de)
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

#ifndef PLOTAREA_H
#define PLOTAREA_H

#include "worksheet/WorksheetElementContainer.h"
#include "lib/macros.h"

class PlotAreaPrivate;

class PlotArea: public WorksheetElementContainer {
	Q_OBJECT

	public:
		PlotArea(const QString &name);
		virtual ~PlotArea();
		
		enum BackgroundType{Color, Image};
		enum BackgroundColorStyle{SingleColor, HorizontalLinearGradient, VerticalLinearGradient,
																TopLeftDiagonalLinearGradient, BottomLeftDiagonalLinearGradient,
																RadialGradient};
		enum BackgroundImageStyle{Scaled, Tiled};
		

		BASIC_D_ACCESSOR_DECL(qreal, opacity, Opacity);
		
		BASIC_D_ACCESSOR_DECL(PlotArea::BackgroundType, backgroundType, BackgroundType);
		BASIC_D_ACCESSOR_DECL(PlotArea::BackgroundColorStyle, backgroundColorStyle, BackgroundColorStyle);
		BASIC_D_ACCESSOR_DECL(PlotArea::BackgroundImageStyle, backgroundImageStyle, BackgroundImageStyle);
		CLASS_D_ACCESSOR_DECL(QBrush, backgroundBrush, BackgroundBrush);
		CLASS_D_ACCESSOR_DECL(QColor, backgroundFirstColor, BackgroundFirstColor);
		CLASS_D_ACCESSOR_DECL(QColor, backgroundSecondColor, BackgroundSecondColor);
		CLASS_D_ACCESSOR_DECL(QString, backgroundFileName, BackgroundFileName);
		
		CLASS_D_ACCESSOR_DECL(QPen, borderPen, BorderPen);

		BASIC_D_ACCESSOR_DECL(bool, clippingEnabled, ClippingEnabled);
		CLASS_D_ACCESSOR_DECL(QRectF, rect, Rect);
		
		typedef WorksheetElementContainer BaseClass;
		typedef PlotAreaPrivate Private;

	public slots:
		virtual void retransform();
		virtual void handlePageResize(double horizontalRatio, double verticalRatio);

	protected:
		PlotArea(const QString &name, PlotAreaPrivate *dd);

	private:
    	Q_DECLARE_PRIVATE(PlotArea)
		void init();
};

#endif
