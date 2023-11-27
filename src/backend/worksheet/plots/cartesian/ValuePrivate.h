/*
	File                 : ValuePrivate.h
	Project              : LabPlot
	Description          : Private members of Value
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2022 Alexander Semke <alexander.semke@web.de>

	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef VALUEPRIVATE_H
#define VALUEPRIVATE_H

#include "backend/worksheet/Worksheet.h"

#include "Value.h"
#include <QColor>
#include <QFont>

class ValuePrivate {
public:
	explicit ValuePrivate(Value*);

	QString name() const;
	void updateValue();
	void updatePixmap();

	Value::Type type{Value::NoValues};
	const AbstractColumn* column{nullptr};
	QString columnPath;
	bool centerPositionAvailable{false};
	Value::Position position{Value::Above};
	qreal distance{Worksheet::convertToSceneUnits(5, Worksheet::Unit::Point)};
	qreal rotationAngle{0.0};
	qreal opacity{1.0};
	char numericFormat{'f'}; // 'f', 'g', 'e', 'E', etc. for numeric values
	int precision{2}; // number of digits for numeric values
	QString dateTimeFormat;
	QString prefix;
	QString suffix;
	QFont font;
	QColor color;

	Value* const q{nullptr};
};

#endif
