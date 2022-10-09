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

#include <QPen>

class LinePrivate {
public:
	explicit LinePrivate(Line*);

	QString name() const;
	void update();
	void updatePixmap();

	bool histogramLineTypeAvailable{false};

	Histogram::LineType histogramLineType{Histogram::Bars};
	QString prefix{QLatin1String("Line")};
	QPen pen;
	double opacity{1.0};

	Line* const q{nullptr};
};

#endif
