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

#include "backend/worksheet/plots/cartesian/XYCurve.h"

#include <QGraphicsItem>
#include <vector>

extern "C" {
#include <gsl/gsl_histogram.h>
}

class HistogramPrivate : public QGraphicsItem {
public:
	explicit HistogramPrivate(Histogram* owner);
	~HistogramPrivate() override;

	QString name() const;
	QRectF boundingRect() const override;
	QPainterPath shape() const override;

	void retransform();
	void recalcHistogram();
	void updateType();
	void updateOrientation();
	void updateLines();
	void verticalHistogram();
	void horizontalHistogram();
	void updateSymbols();
	void updateValues();
	void updateFilling();
	void updateErrorBars();
	void updatePixmap();
	bool swapVisible(bool on);
	void recalcShapeAndBoundingRect();

	void setHover(bool on);
	bool activateCurve(QPointF mouseScenePos, double maxDist);

	double xMinimum();
	double xMaximum();
	double yMinimum();
	double yMaximum();
	double getMaximumOccuranceofHistogram();

	bool m_suppressRecalc{false};

	//General
	const AbstractColumn* dataColumn{nullptr};
	QString dataColumnPath;
	Histogram::HistogramType type{Histogram::Ordinary};
	Histogram::HistogramOrientation orientation{Histogram::Vertical};
	Histogram::HistogramNormalization normalization{Histogram::Count};
	Histogram::BinningMethod binningMethod{Histogram::SquareRoot};
	int totalCount{0};
	int binCount{10};
	float binWidth{1.0f};
	bool autoBinRanges{true};
	double binRangesMin{0.0};
	double binRangesMax{1.0};

	//line
	Histogram::LineType lineType{Histogram::Bars};
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
	int value{0};
	Histogram::ValuesType valuesType{Histogram::NoValues};
	const AbstractColumn* valuesColumn{nullptr};
	QString valuesColumnPath;
	Histogram::ValuesPosition valuesPosition{Histogram::ValuesAbove};
	qreal valuesDistance;
	qreal valuesRotationAngle;
	qreal valuesOpacity;
	char valuesNumericFormat; //'g', 'e', 'E', etc. for numeric values
	int valuesPrecision; //number of digits for numeric values
	QString valuesDateTimeFormat;
	QString valuesPrefix;
	QString valuesSuffix;
	QFont valuesFont;
	QColor valuesColor;

	//filling
	bool fillingEnabled{true};
	PlotArea::BackgroundType fillingType;
	PlotArea::BackgroundColorStyle fillingColorStyle;
	PlotArea::BackgroundImageStyle fillingImageStyle;
	Qt::BrushStyle fillingBrushStyle;
	QColor fillingFirstColor;
	QColor fillingSecondColor;
	QString fillingFileName;
	qreal fillingOpacity;

	//error bars
	Histogram::ErrorType errorType{Histogram::NoError};
	XYCurve::ErrorBarsType errorBarsType;
	double errorBarsCapSize{1};
	QPen errorBarsPen;
	qreal errorBarsOpacity;

	QPainterPath linePath;
	QPainterPath symbolsPath;
	QPainterPath valuesPath;
	QRectF boundingRectangle;
	QPainterPath curveShape;
	//TODO: use Qt container
	//TODO: add m_
	QVector<QLineF> lines;
	QVector<QPointF> pointsLogical;	//points in logical coordinates
	QVector<QPointF> pointsScene;	//points in scene coordinates
	std::vector<bool> visiblePoints;	//vector of the size of symbolPointsLogical with true of false for the points currently visible or not in the plot
	QVector<QPointF> valuesPoints;
	QVector<QString> valuesStrings;
	QPolygonF fillPolygon;

	Histogram* const q;

private:
	gsl_histogram* m_histogram{nullptr};
	size_t m_bins{0};

//	bool m_printing{false};
	bool m_hovered{false};
	bool m_suppressRetransform{false};
	QPixmap m_pixmap;
	QImage m_hoverEffectImage;
	QImage m_selectionEffectImage;
	bool m_hoverEffectImageIsDirty{false};
	bool m_selectionEffectImageIsDirty{false};

	void contextMenuEvent(QGraphicsSceneContextMenuEvent*) override;
	void mousePressEvent(QGraphicsSceneMouseEvent*) override;
	void hoverEnterEvent(QGraphicsSceneHoverEvent*) override;
	void hoverLeaveEvent(QGraphicsSceneHoverEvent*) override;
	void paint(QPainter*, const QStyleOptionGraphicsItem*, QWidget* widget = nullptr) override;

	void histogramValue(double& value, int bin);
	void drawSymbols(QPainter*);
	void drawValues(QPainter*);
	void drawFilling(QPainter*);
	void draw(QPainter*);
};

#endif
