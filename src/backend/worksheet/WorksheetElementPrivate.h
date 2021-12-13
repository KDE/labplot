/*
	File                 : WorksheetElementPrivate.h
	Project              : LabPlot
	Description          : Private member of WorksheetElement
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2012-2021 Martin Marmsoler <martin.marmsoler@gmail.com>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef WORKSHEETELEMENTPRIVATE
#define WORKSHEETELEMENTPRIVATE

#include <QGraphicsItem>

#include "WorksheetElement.h"
#include "Worksheet.h"

class WorksheetElement;

class WorksheetElementPrivate: public QGraphicsItem {

public:
	WorksheetElementPrivate(WorksheetElement *);

public:
	// position in parent's coordinate system, the label gets aligned around this point
	// TODO: try to get away the Worksheet dependency
	WorksheetElement::PositionWrapper position{
		QPoint(Worksheet::convertToSceneUnits(1, Worksheet::Unit::Centimeter), Worksheet::convertToSceneUnits(1, Worksheet::Unit::Centimeter)),
	WorksheetElement::HorizontalPosition::Center, WorksheetElement::VerticalPosition::Center};
	WorksheetElement::HorizontalAlignment horizontalAlignment{WorksheetElement::HorizontalAlignment::Center};
	WorksheetElement::VerticalAlignment verticalAlignment{WorksheetElement::VerticalAlignment::Center};
	bool positionInvalid{false};
	bool coordinateBindingEnabled{false};
	QPointF positionLogical;
	qreal rotationAngle{0.0};
	QRectF boundingRectangle; //bounding rectangle of the text
	bool suppressItemChangeEvent{false};
	bool suppressRetransform{false};
	WorksheetElement* const q{nullptr};

public:
	bool swapVisible(bool on);
	QString name() const;
	virtual void retransform() = 0;
	virtual void recalcShapeAndBoundingRect() = 0;
	QRectF boundingRect() const override;
	void paint(QPainter*, const QStyleOptionGraphicsItem*, QWidget* widget = nullptr) override;
	virtual void keyPressEvent(QKeyEvent*) override;
	virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent* event) override;
	virtual QVariant itemChange(GraphicsItemChange change, const QVariant &value) override;
	QPointF mapParentToPlotArea(QPointF);
	QPointF mapPlotAreaToParent(QPointF);
};

#endif WORKSHEETELEMENTPRIVATE
