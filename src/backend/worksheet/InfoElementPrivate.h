/*
	File                 : InfoElementPrivate.h
	Project              : LabPlot
	Description          : Private members of InfoElement
	--------------------------------------------------------------------
    SPDX-FileCopyrightText: 2020 Martin Marmsoler <martin.marmsoler@gmail.com>
    SPDX-FileCopyrightText: 2020 Alexander Semke <alexander.semke@web.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef INFOELEMENTPRIVATE_H
#define INFOELEMENTPRIVATE_H

#include "backend/worksheet/WorksheetElementPrivate.h"

class InfoElement;
class TextLabel;
class CustomPoint;
class CartesianPlot;
class CartesianCoordinateSystem;
class XYCurve;
class QGraphicsSceneMouseEvent;
class QPen;

class InfoElementPrivate : public WorksheetElementPrivate {

public:
	InfoElementPrivate(InfoElement* owner);
	InfoElementPrivate(InfoElement* owner, const XYCurve *);

	//reimplemented from QGraphicsItem
	QRectF boundingRect() const override;
	void paint(QPainter*, const QStyleOptionGraphicsItem*, QWidget* widget = nullptr) override;
	void keyPressEvent(QKeyEvent*) override;
	void mousePressEvent(QGraphicsSceneMouseEvent*) override;
	void mouseMoveEvent(QGraphicsSceneMouseEvent*) override;

	void init();
	void updatePosition();
	void retransform();
	void updateVerticalLine();
	void updateConnectionLine();
	void visibilityChanged();

	virtual void recalcShapeAndBoundingRect() override {};

	bool visible{true};
	double xPos;

	// TextLabel Gluepoint
	int gluePointIndex{-1}; // negative value means automatic mode
	int m_index{-1}; // index of the actual position
	QString connectionLineCurveName;
	double positionLogical;

	QPen verticalLinePen;
	qreal verticalLineOpacity;
	QPen connectionLinePen;
	qreal connectionLineOpacity;

	//TODO
//	CartesianPlot* plot{nullptr};

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
