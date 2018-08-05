/***************************************************************************
    File                 : HistogramPrivate.h
    Project              : LabPlot
    Description          : Private members of Histogram
    --------------------------------------------------------------------
    Copyright            : (C) 2016 Anu Mittal (anu22mittal@gmail.com)
    Copyright            : (C) 2018 Alexander Semke (alexander.semke@web.de)

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

#ifndef HISTOGRAMPRIVATE_H
#define HISTOGRAMPRIVATE_H

#include <QGraphicsItem>
#include <vector>
#include <QFont>
#include <QPen>

extern "C" {
#include <gsl/gsl_histogram.h>
}

class HistogramPrivate : public QGraphicsItem {
public:
	explicit HistogramPrivate(Histogram* owner);

	QString name() const;
	QRectF boundingRect() const override;
	QPainterPath shape() const override;

	bool m_printing;
	bool m_hovered;
	bool m_suppressRetransform;
	bool m_suppressRecalc;
	QPixmap m_pixmap;
	QImage m_hoverEffectImage;
	QImage m_selectionEffectImage;
	bool m_hoverEffectImageIsDirty;
	bool m_selectionEffectImageIsDirty;

	void retransform();
	void recalcLogicalPoints();
	void recalcHistogram();
	void updateType();
	void updateOrientation();
	void updateLines();
	void verticalHistogram();
	void horizontalHistogram();
	void updateSymbols();
	void updateValues();
	void updateFilling();
	bool swapVisible(bool on);
	void recalcShapeAndBoundingRect();

	void drawSymbols(QPainter*);
	void drawValues(QPainter*);
	void drawFilling(QPainter*);
	void draw(QPainter*);
	void updatePixmap();

	double getYMaximum();
	double getYMinimum();
	double getXMinimum();
	double getXMaximum();
	double getMaximumOccuranceofHistogram();

	void paint(QPainter*, const QStyleOptionGraphicsItem*, QWidget* widget = 0) override;

	//General
	const AbstractColumn* dataColumn;
	QString dataColumnPath;
	Histogram::HistogramType type;
	Histogram::HistogramOrientation orientation;
	Histogram::BinningMethod binningMethod;
	int binCount;
	float binWidth;

	//line
	Histogram::LineType lineType;
	QPen linePen;
	qreal lineOpacity;

	//symbols
	Symbol::Style symbolsStyle;
	QBrush symbolsBrush;
	QPen symbolsPen;
	qreal symbolsOpacity;
	qreal symbolsRotationAngle;
	qreal symbolsSize;

	//values
	int value;
	Histogram::ValuesType valuesType;
	const AbstractColumn* valuesColumn;
	QString valuesColumnPath;
	Histogram::ValuesPosition valuesPosition;
	qreal valuesDistance;
	qreal valuesRotationAngle;
	qreal valuesOpacity;
	QString valuesPrefix;
	QString valuesSuffix;
	QFont valuesFont;
	QColor valuesColor;

	//filling
	bool fillingEnabled;
	PlotArea::BackgroundType fillingType;
	PlotArea::BackgroundColorStyle fillingColorStyle;
	PlotArea::BackgroundImageStyle fillingImageStyle;
	Qt::BrushStyle fillingBrushStyle;
	QColor fillingFirstColor;
	QColor fillingSecondColor;
	QString fillingFileName;
	qreal fillingOpacity;

	QPainterPath linePath;
	QPainterPath valuesPath;
	QRectF boundingRectangle;
	QPainterPath curveShape;
	QVector<QLineF> lines;
	QVector<QPointF> symbolPointsLogical;	//points in logical coordinates
	QVector<QPointF> symbolPointsScene;	//points in scene coordinates
	std::vector<bool> visiblePoints;	//vector of the size of symbolPointsLogical with true of false for the points currently visible or not in the plot
	QVector<QPointF> valuesPoints;
	QVector<QString> valuesStrings;
	QVector<QPolygonF> fillPolygons;

	Histogram* const q;

private:
	gsl_histogram* m_histogram;
	size_t m_bins;

	void contextMenuEvent(QGraphicsSceneContextMenuEvent*) override;
	void hoverEnterEvent(QGraphicsSceneHoverEvent*) override;
	void hoverLeaveEvent(QGraphicsSceneHoverEvent*) override;
};

#endif
