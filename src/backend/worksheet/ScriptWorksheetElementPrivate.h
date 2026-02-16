/*
	File                 : ScriptWorksheetElementPrivate.h
	Project              : LabPlot
	Description          : Private implementation for ScriptWorksheetElement
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2026 Alexander Semke <alexander.semke@web.de>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef SCRIPTWORKSHEETELEMENT_PRIVATE_H
#define SCRIPTWORKSHEETELEMENT_PRIVATE_H

#include "backend/worksheet/WorksheetElementPrivate.h"

#include <QBrush>
#include <QFont>
#include <QPen>

class Script;
class QGraphicsSceneMouseEvent;

class ScriptWorksheetElementPrivate : public WorksheetElementPrivate {
public:
	explicit ScriptWorksheetElementPrivate(ScriptWorksheetElement* owner);

	QString name() const override {
		return QStringLiteral("ScriptWorksheetElement");
	}

	void retransform() override;
	void recalcShapeAndBoundingRect() override;
	void updatePosition();
	void update();

	// Properties
	QString text{QStringLiteral("Execute")};
	int width{100};
	int height{30};
	QColor backgroundColor{Qt::lightGray};
	QColor borderColor{Qt::black};
	double borderWidth{1.0};
	QFont font;
	QColor textColor{Qt::black};

	// Script reference
	Script* script{nullptr};

	// QGraphicsItem interface
	void paint(QPainter*, const QStyleOptionGraphicsItem*, QWidget* widget = nullptr) override;
	void mousePressEvent(QGraphicsSceneMouseEvent*) override;
	void mouseReleaseEvent(QGraphicsSceneMouseEvent*) override;
	void hoverEnterEvent(QGraphicsSceneHoverEvent*) override;
	void hoverLeaveEvent(QGraphicsSceneHoverEvent*) override;

private:
	bool m_hovered{false};
	bool m_pressed{false};

	ScriptWorksheetElement* const q;
};

#endif
