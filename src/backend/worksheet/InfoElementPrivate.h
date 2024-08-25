/*
	File                 : InfoElementPrivate.h
	Project              : LabPlot
	Description          : Private members of InfoElement
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2020 Martin Marmsoler <martin.marmsoler@gmail.com>
	SPDX-FileCopyrightText: 2020-2022 Alexander Semke <alexander.semke@web.de>

	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef INFOELEMENTPRIVATE_H
#define INFOELEMENTPRIVATE_H

#include "backend/worksheet/WorksheetElementPrivate.h"

class InfoElement;
class XYCurve;
class QGraphicsSceneMouseEvent;

class InfoElementPrivate : public WorksheetElementPrivate {
public:
	InfoElementPrivate(InfoElement* owner);
	InfoElementPrivate(InfoElement* owner, const XYCurve*);

	// reimplemented from QGraphicsItem
	void paint(QPainter*, const QStyleOptionGraphicsItem*, QWidget* widget = nullptr) override;
	void keyPressEvent(QKeyEvent*) override;
	void mousePressEvent(QGraphicsSceneMouseEvent*) override;
	void mouseMoveEvent(QGraphicsSceneMouseEvent*) override;

	void init();
	void updatePosition();
	void retransform() override;
	void updateVerticalLine();
	void updateConnectionLine();
	bool changeVisibility(bool on);
	void updateValid();

	virtual bool activate(QPointF mouseScenePos, double maxDist = -1);

	// TextLabel Gluepoint
	int gluePointIndex{-1}; // negative value means automatic mode
	int m_index{-1}; // index of the actual position
	QString connectionLineCurveName;
	double positionLogical;

	Line* verticalLine{nullptr};
	Line* connectionLine{nullptr};

	bool valid{true};

	// TODO
	//	CartesianPlot* plot{nullptr};

	InfoElement* const q;

private:
	void recalcShape();
	void recalcShapeAndBoundingRect(const QRectF&);
	virtual void recalcShapeAndBoundingRect() override;
	virtual void contextMenuEvent(QGraphicsSceneContextMenuEvent*) override;

private:
	QPointF sceneDeltaPoint; // delta position from worksheetinfoElementPrivate to the first marker point in scene coords
	QPointF sceneDeltaTextLabel;

	QLineF m_connectionLine; // line between CustomPoint and TextLabel
	QLineF xposLine; // Line which connects all markerpoints, when there are more than 1
	QPointF oldMousePos;
	bool m_suppressKeyPressEvents{false};
};

#endif // WORKSHEETINFOELEMENTPRIVATE_H
