/*
	File                 : ScriptButtonPrivate.h
	Project              : LabPlot
	Description          : Private implementation for ScriptButton
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2026 Alexander Semke <alexander.semke@web.de>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef SCRIPTWORKSHEETELEMENT_PRIVATE_H
#define SCRIPTWORKSHEETELEMENT_PRIVATE_H

#include "backend/worksheet/WorksheetElementPrivate.h"

#include <QFont>

class Script;
class QGraphicsSceneMouseEvent;
class QPushButton;

class ScriptButtonPrivate : public WorksheetElementPrivate {
public:

	explicit ScriptButtonPrivate(ScriptButton*);

	QString name() const;
	void update();
	void retransform() override;
	void recalcShapeAndBoundingRect() override;

	QString text{i18n("Execute")};
	int width{100};
	int height{30};
	QColor backgroundColor{Qt::lightGray};
	QColor borderColor{Qt::black};
	double borderWidth{1.0};
	QFont font;
	QColor textColor{Qt::black};

	Script* script{nullptr};
	QPushButton* button{nullptr};

	ScriptButton* const q{nullptr};

private Q_SLOTS:
	void clicked();
};

#endif
