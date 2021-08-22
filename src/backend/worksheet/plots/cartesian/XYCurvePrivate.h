/***************************************************************************
    File                 : XYCurvePrivate.h
    Project              : LabPlot
    Description          : Private members of XYCurve
    --------------------------------------------------------------------
    Copyright            : (C) 2010-2020 Alexander Semke (alexander.semke@web.de)
    Copyright            : (C) 2013-2020 by Stefan Gerlach (stefan.gerlach@uni.kn)
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

#include <QGraphicsItem>
#include <vector>

class CartesianPlot;
class CartesianCoordinateSystem;
class Symbol;
class XYCurve;

class XYCurvePrivate : public QGraphicsItem {
public:
	explicit XYCurvePrivate(XYCurve*);

	QRectF boundingRect() const override;
	QPainterPath shape() const override;

	QString name() const;
	void retransform();
	void recalcLogicalPoints();
	void updateLines();
	void addLine(QPointF p0, QPointF p1, QPointF& lastPoint, qint64& pixelDiff, int numberOfPixelX, double minLogicalDiffX); // for any x scale
	void addLinearLine(QPointF p0, QPointF p1, QPointF& lastPoint, double minLogicalDiffX, qint64& pixelDiff);	// optimized for linear x scale
	void addUniqueLine(QPointF p0, QPointF p1, QPointF& lastPoint, qint64& pixelDiff);	// finally add line if unique (no overlay)
	void updateDropLines();
	void updateSymbols();
	void updateRug();
	void updateValues();
	void updateFilling();
	void updateErrorBars();
	bool swapVisible(bool);
	void recalcShapeAndBoundingRect();
	void updatePixmap();
	void suppressRetransform(bool);

	void setHover(bool on);
	bool activateCurve(QPointF mouseScenePos, double maxDist);
	bool pointLiesNearLine(const QPointF p1, const QPointF p2, const QPointF pos, const double maxDist) const;
	bool pointLiesNearCurve(const QPointF mouseScenePos, const QPointF curvePosPrevScene, const QPointF curvePosScene, const int index, const double maxDist) const;

	//data source
	const AbstractColumn* xColumn{nullptr};
	const AbstractColumn* yColumn{nullptr};
	QString dataSourceCurvePath;
	QString xColumnPath;
	QString yColumnPath;
	bool sourceDataChangedSinceLastRecalc{false};

	//line
	XYCurve::LineType lineType;
	bool lineSkipGaps;
	bool lineIncreasingXOnly;
	int lineInterpolationPointsCount;
	QPen linePen;
	qreal lineOpacity;

	//drop lines
	XYCurve::DropLineType dropLineType;
	QPen dropLinePen;
	qreal dropLineOpacity;

	//symbols
	Symbol* symbol{nullptr};

	//rug
	bool rugEnabled{false};
	WorksheetElement::Orientation rugOrientation{WorksheetElement::Orientation::Vertical};
	double rugOffset;
	double rugLength;
	double rugWidth;
	QPainterPath rugPath;

	//values
	XYCurve::ValuesType valuesType;
	const AbstractColumn* valuesColumn{nullptr};
	QString valuesColumnPath;
	XYCurve::ValuesPosition valuesPosition;
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
	XYCurve::FillingPosition fillingPosition;
	PlotArea::BackgroundType fillingType;
	PlotArea::BackgroundColorStyle fillingColorStyle;
	PlotArea::BackgroundImageStyle fillingImageStyle;
	Qt::BrushStyle fillingBrushStyle;
	QColor fillingFirstColor;
	QColor fillingSecondColor;
	QString fillingFileName;
	qreal fillingOpacity;

	//error bars
	XYCurve::ErrorType xErrorType;
	const AbstractColumn* xErrorPlusColumn{nullptr};
	QString xErrorPlusColumnPath;
	const AbstractColumn* xErrorMinusColumn{nullptr};
	QString xErrorMinusColumnPath;

	XYCurve::ErrorType yErrorType;
	const AbstractColumn* yErrorPlusColumn{nullptr};
	QString yErrorPlusColumnPath;
	const AbstractColumn* yErrorMinusColumn{nullptr};
	QString yErrorMinusColumnPath;

	XYCurve::ErrorBarsType errorBarsType;
	double errorBarsCapSize;
	QPen errorBarsPen;
	qreal errorBarsOpacity;

	XYCurve* const q;
	friend class XYCurve;

//	CartesianPlot* plot{nullptr};
//	const CartesianCoordinateSystem* cSystem{nullptr};	//current cSystem

private:
	CartesianPlot* plot() const { return q->m_plot; }	// convenience method
	void contextMenuEvent(QGraphicsSceneContextMenuEvent*) override;
	void mousePressEvent(QGraphicsSceneMouseEvent*) override;
	QVariant itemChange(GraphicsItemChange change, const QVariant & value) override;
	void paint(QPainter*, const QStyleOptionGraphicsItem*, QWidget* widget = nullptr) override;

	void drawSymbols(QPainter*);
	void drawValues(QPainter*);
	void drawFilling(QPainter*);
	void draw(QPainter*);

	//TODO: add m_
	QPainterPath linePath;
	QPainterPath dropLinePath;
	QPainterPath valuesPath;
	QPainterPath errorBarsPath;
	QPainterPath symbolsPath;
	QRectF boundingRectangle;
	QPainterPath curveShape;
	QVector<QLineF> m_lines;
	QVector<QPointF> m_logicalPoints;	//points in logical coordinates
	QVector<QPointF> m_scenePoints;		//points in scene coordinates
	QVector<bool> m_pointVisible;		//if point is currently visible in plot (size of m_logicalPoints)
	QVector<QPointF> m_valuePoints;		//points for showing value
	QVector<QString> m_valueStrings;	//strings for showing value
	QVector<QPolygonF> m_fillPolygons;	//polygons for filling
	//TODO: QVector, rename, usage
	std::vector<int> validPointsIndicesLogical;	//original indices in the source columns for valid and non-masked values (size of m_logicalPoints)
	std::vector<bool> connectedPointsLogical;  	//true for points connected with the consecutive point (size of m_logicalPoints)

	QPixmap m_pixmap;
	QImage m_hoverEffectImage;
	QImage m_selectionEffectImage;
	bool m_hoverEffectImageIsDirty{false};
	bool m_selectionEffectImageIsDirty{false};
	bool m_hovered{false};
	bool m_suppressRecalc{false};
	bool m_suppressRetransform{false};
	QPointF mousePos;
};

#endif
