/***************************************************************************
    File                 : AbstractWorksheetElement.h
    Project              : LabPlot/SciDAVis
    Description          : Base class for all Worksheet children.
    --------------------------------------------------------------------
    Copyright            : (C) 2009 Tilman Benkert (thzs*gmx.net)
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

#ifndef ABSTRACTWORKSHEETELEMENT_H
#define ABSTRACTWORKSHEETELEMENT_H

#include "backend/core/AbstractAspect.h"
#include <QGraphicsItem>
class QAction;

class AbstractWorksheetElement: public AbstractAspect {
	Q_OBJECT

	public:
		explicit AbstractWorksheetElement(const QString &name);
		virtual ~AbstractWorksheetElement();

		virtual QGraphicsItem *graphicsItem() const = 0;

		virtual void setVisible(bool on) = 0;
		virtual bool isVisible() const = 0;
		virtual bool isFullyVisible() const;
		virtual void setPrinting(bool) = 0;
		virtual QMenu *createContextMenu();

		static QPainterPath shapeFromPath(const QPainterPath &path, const QPen &pen);

	public slots:
		virtual void retransform() = 0;
		virtual void handlePageResize(double horizontalRatio, double verticalRatio);

	private:
		QMenu *m_drawingOrderMenu;
		QMenu *m_moveBehindMenu;
		QMenu *m_moveInFrontOfMenu;

	private slots:
		void prepareMoveBehindMenu();
		void prepareMoveInFrontOfMenu();
		void execMoveBehind(QAction *action);
		void execMoveInFrontOf(QAction *action);

	signals:
		friend class AbstractPlotSetHorizontalPaddingCmd;
		friend class AbstractPlotSetVerticalPaddingCmd;
		void horizontalPaddingChanged(float);
		void verticalPaddingChanged(float);
};

#endif
