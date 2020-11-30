/***************************************************************************
	File                 : InfoElementPrivate.h
	Project              : LabPlot
	Description          : Private members of InfoElement
	--------------------------------------------------------------------
    Copyright            : (C) 2020 by Martin Marmsoler (martin.marmsoler@gmail.com)

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

#ifndef WORKSHEETINFOELEMENTPRIVATE_H
#define WORKSHEETINFOELEMENTPRIVATE_H

#include "QGraphicsItem"

class InfoElement;
class TextLabel;
class CustomPoint;
class CartesianPlot;
class CartesianCoordinateSystem;
class XYCurve;
class QGraphicsSceneMouseEvent;

class InfoElementPrivate: public QGraphicsItem
{
public:
    InfoElementPrivate(InfoElement *owner, CartesianPlot *);
    InfoElementPrivate(InfoElement *owner, CartesianPlot *, const XYCurve *);
	QString name() const;

	//reimplemented from QGraphicsItem
	QRectF boundingRect() const override;
	void paint(QPainter*, const QStyleOptionGraphicsItem*, QWidget* widget = nullptr) override;
	QVariant itemChange(GraphicsItemChange change, const QVariant &value) override;
	void keyPressEvent(QKeyEvent*) override;
	void mousePressEvent(QGraphicsSceneMouseEvent*) override;
	void mouseMoveEvent(QGraphicsSceneMouseEvent*) override;

	void init();
	void updatePosition();
	void retransform();
	void updateXPosLine();
	void updateConnectionLine();
	void visibilityChanged();

	bool visible{true};
	bool m_printing{false};
	double xPos;

	// TextLabel Gluepoint
	int gluePointIndex{-1}; // negative value means automatic mode
	int m_index{-1}; // index of the actual position
	// connect to this curve
	QString connectionLineCurveName;
    double position;

	QColor connectionLineColor{QColor(Qt::black)};
	double connectionLineWidth{5}; // drawing linewidth
	bool xposLineVisible{true};
	bool connectionLineVisible{true};
	QColor xposLineColor{QColor(Qt::black)};
	double xposLineWidth{5}; // drawing linewidth

    CartesianPlot* plot{nullptr};
    const CartesianCoordinateSystem* cSystem{nullptr};

	InfoElement* const q;
private:


	QPointF sceneDeltaPoint; // delta position from worksheetinfoElementPrivate to the first marker point in scene coords
	QPointF sceneDeltaTextLabel;

	QRectF boundingRectangle; //bounding rectangle of the connection line between CustomPoint and TextLabel
	QLineF connectionLine; // line between CustomPoint and TextLabel
	QLineF xposLine; // Line which connects all markerpoints, when there are more than 1
	QPointF oldMousePos;
	bool m_suppressKeyPressEvents{false};
};

#endif // WORKSHEETINFOELEMENTPRIVATE_H
