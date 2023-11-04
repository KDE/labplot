/*
	File                 : WorksheetElementPrivate.h
	Project              : LabPlot
	Description          : Private member of WorksheetElement
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2012-2021 Martin Marmsoler <martin.marmsoler@gmail.com>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef WORKSHEETELEMENTPRIVATE_H
#define WORKSHEETELEMENTPRIVATE_H

#include "Worksheet.h"
#include <QGraphicsItem>

class WorksheetElement;

class WorksheetElementPrivate : public QGraphicsItem {
public:
	WorksheetElementPrivate(WorksheetElement*);

	// position in parent's coordinate system, the label gets aligned around this point
	// TODO: try to get away the Worksheet dependency
	WorksheetElement::PositionWrapper position{
		QPointF(Worksheet::convertToSceneUnits(1, Worksheet::Unit::Centimeter), Worksheet::convertToSceneUnits(1, Worksheet::Unit::Centimeter)),
		WorksheetElement::HorizontalPosition::Center,
		WorksheetElement::VerticalPosition::Center,
		WorksheetElement::PositionLimit::None};
	WorksheetElement::HorizontalAlignment horizontalAlignment{WorksheetElement::HorizontalAlignment::Center};
	WorksheetElement::VerticalAlignment verticalAlignment{WorksheetElement::VerticalAlignment::Center};
	bool positionInvalid{false};
	bool coordinateBindingEnabled{false};
	QPointF positionLogical;
	QRectF boundingRectangle; // bounding rectangle of the text
	bool suppressItemChangeEvent{false};
	bool suppressRetransform{false};
	WorksheetElement* const q{nullptr};
	bool insidePlot{true}; // point inside the plot (visible) or not
	bool locked{false};

	bool swapVisible(bool on);
	QString name() const;
	virtual void retransform() = 0;
	virtual void recalcShapeAndBoundingRect() = 0;
	void updatePosition();
	QRectF boundingRect() const override;
	void paint(QPainter*, const QStyleOptionGraphicsItem*, QWidget* widget = nullptr) override;
	virtual void keyPressEvent(QKeyEvent*) override;
	virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent*) override;
	virtual QVariant itemChange(GraphicsItemChange, const QVariant& value) override;
	virtual bool sceneEvent(QEvent *event) override;
	QPointF mapParentToPlotArea(QPointF);
	QPointF mapPlotAreaToParent(QPointF);
};

#endif
