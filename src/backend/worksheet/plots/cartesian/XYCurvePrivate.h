/***************************************************************************
    File                 : XYCurvePrivate.h
    Project              : LabPlot
    Description          : Private members of XYCurve
    --------------------------------------------------------------------
    Copyright            : (C) 2010-2015 Alexander Semke (alexander.semke@web.de)
	Copyright            : (C) 2013 by Stefan Gerlach (stefan.gerlach@uni-konstanz.de)
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

#ifndef XYCURVEPRIVATE_H
#define XYCURVEPRIVATE_H

#include <vector>

class CartesianPlot;

class XYCurvePrivate: public QGraphicsItem {
	public:
		explicit XYCurvePrivate(XYCurve *owner);

		QString name() const;
		virtual QRectF boundingRect() const;
		QPainterPath shape() const;

		bool m_printing;
		bool m_hovered;
		bool m_suppressRecalc;
		bool m_suppressRetransform;
		QPixmap m_pixmap;
		QImage m_hoverEffectImage;
		QImage m_selectionEffectImage;
		bool m_hoverEffectImageIsDirty;
		bool m_selectionEffectImageIsDirty;

		void retransform();
		void updateLines();
		void updateDropLines();
		void updateSymbols();
		void updateValues();
		void updateErrorBars();
		bool swapVisible(bool on);
		void recalcShapeAndBoundingRect();
		void drawSymbols(QPainter*);
		void drawValues(QPainter*);
		void draw(QPainter*);
		void updatePixmap();

		virtual void paint(QPainter*, const QStyleOptionGraphicsItem*, QWidget* widget = 0);

		const AbstractColumn* xColumn;
		const AbstractColumn* yColumn;
		QString xColumnPath;
		QString yColumnPath;

		XYCurve::LineType lineType;
		int lineInterpolationPointsCount;
		QPen linePen;
		qreal lineOpacity;

		XYCurve::DropLineType dropLineType;
		QPen dropLinePen;
		qreal dropLineOpacity;

		XYCurve::SymbolsStyle symbolsStyle;
		QBrush symbolsBrush;
		QPen symbolsPen;
		qreal symbolsOpacity;
		qreal symbolsRotationAngle;
		qreal symbolsSize;
		qreal symbolsAspectRatio;

		XYCurve::ValuesType valuesType;
		const AbstractColumn* valuesColumn;
		QString valuesColumnPath;
		XYCurve::ValuesPosition valuesPosition;
		qreal valuesDistance;
		qreal valuesRotationAngle;
		qreal valuesOpacity;
		QString valuesPrefix;
		QString valuesSuffix;
		QFont valuesFont;
		QColor valuesColor;

		//error bars
		XYCurve::ErrorType xErrorType;
		const AbstractColumn* xErrorPlusColumn;
		QString xErrorPlusColumnPath;
		const AbstractColumn* xErrorMinusColumn;
		QString xErrorMinusColumnPath;

		XYCurve::ErrorType yErrorType;
		const AbstractColumn* yErrorPlusColumn;
		QString yErrorPlusColumnPath;
		const AbstractColumn* yErrorMinusColumn;
		QString yErrorMinusColumnPath;

		XYCurve::ErrorBarsType errorBarsType;
		double errorBarsCapSize;
		QPen errorBarsPen;
		qreal errorBarsOpacity;

		QPainterPath linePath;
		QPainterPath dropLinePath;
		QPainterPath valuesPath;
		QPainterPath errorBarsPath;
		QPainterPath symbolsPath;
		QRectF boundingRectangle;
		QPainterPath curveShape;
		QList<QPointF> symbolPointsLogical;	//points in logical coordinates
		QList<QPointF> symbolPointsScene;	//points in scene coordinates
		std::vector<bool> visiblePoints;	//vector of the size of symbolPointsLogical with true of false for the points currently visible or not in the plot
		QList<QPointF> valuesPoints;
		QList<QString> valuesStrings;

		XYCurve* const q;

	private:
        void contextMenuEvent(QGraphicsSceneContextMenuEvent*);
		virtual void hoverEnterEvent(QGraphicsSceneHoverEvent*);
		virtual void hoverLeaveEvent(QGraphicsSceneHoverEvent*);
};

#endif
