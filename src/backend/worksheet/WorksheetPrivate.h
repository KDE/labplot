/***************************************************************************
    File                 : WorksheetPrivate.h
    Project              : LabPlot/SciDAVis
    Description          : Private members of Worksheet.
    --------------------------------------------------------------------
    Copyright            : (C) 2012 by Alexander Semke (alexander.semke*web.de)
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

#ifndef WORKSHEETPRIVATE_H
#define WORKSHEETPRIVATE_H

#include <QBrush>

class Worksheet;
class WorksheetPrivate{
	public:
		WorksheetPrivate(Worksheet *owner);
		virtual ~WorksheetPrivate();

		WorksheetGraphicsScene *m_scene;
		QRectF swapPageRect(const QRectF& rect);
		mutable QWidget *m_view;
		WorksheetGraphicsScene *scene() const;
		friend QGraphicsScene *WorksheetModel::scene() const;

		QString name() const;
		QRectF swapRect(const QRectF &newRect);
		
		void update();
		
		PlotArea::BackgroundType backgroundType;
		PlotArea::BackgroundColorStyle backgroundColorStyle;
		PlotArea::BackgroundImageStyle backgroundImageStyle;
		QBrush backgroundBrush;
		QColor backgroundFirstColor;
		QColor backgroundSecondColor;
		QString backgroundFileName;
		qreal backgroundOpacity;
		
		bool layoutActive;
		Worksheet::Layout layout;
		float layoutTopMargin;
		float layoutBottomMargin;
		float layoutLeftMargin;
		float layoutRightMargin;
		float layoutVerticalSpacing;
		float layoutHorizontalSpacing;
		float layoutColumnsCount;
		float layoutRowsCount;

		Worksheet * const q;
};

#endif
