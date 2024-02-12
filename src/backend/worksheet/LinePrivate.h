/*
	File                 : LinePrivate.h
	Project              : LabPlot
	Description          : Private members of Line
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2022 Alexander Semke <alexander.semke@web.de>

	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef LINEPRIVATE_H
#define LINEPRIVATE_H

#include "backend/worksheet/Worksheet.h"

#include <QPen>

class LinePrivate {
public:
	explicit LinePrivate(Line*);

	QString name() const;
	void update();
	void updatePixmap();

	QString prefix{QLatin1String("Line")};
	bool createXmlElement{true};

	// histogram specific parameters
	bool histogramLineTypeAvailable{false};
	Histogram::LineType histogramLineType{Histogram::Bars};

	XYCurve::DropLineType dropLineType{XYCurve::DropLineType::NoDropLine};

	// common parameters
	Qt::PenStyle style{Qt::SolidLine};
	double width{Worksheet::convertToSceneUnits(1.0, Worksheet::Unit::Point)};
	QColor color{Qt::black};
	QPen pen;
	double opacity{1.0};

	Line* const q{nullptr};
};

#endif
