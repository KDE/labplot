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
	Value::Position position{Value::Above};
	qreal distance;
	qreal rotationAngle;
	qreal opacity;
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
