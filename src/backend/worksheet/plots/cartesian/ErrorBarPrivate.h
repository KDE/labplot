/*
	File                 : ErrorBarPrivate.h
	Project              : LabPlot
	Description          : Private members of ErrorBar
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2024 Alexander Semke <alexander.semke@web.de>

	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef ERRORBARPRIVATE_H
#define ERRORBARPRIVATE_H

#include "backend/worksheet/plots/cartesian/ErrorBar.h"

class AbstractColumn;
class Line;

class ErrorBarPrivate {
public:
	explicit ErrorBarPrivate(ErrorBar*, ErrorBar::Dimension dim);

	QString name() const;
	void update();
	void updatePixmap();

	ErrorBar::Dimension dimension;

	ErrorBar::ErrorType xErrorType{ErrorBar::ErrorType::NoError};
	const AbstractColumn* xPlusColumn{nullptr};
	QString xPlusColumnPath;
	const AbstractColumn* xMinusColumn{nullptr};
	QString xMinusColumnPath;

	ErrorBar::ErrorType yErrorType{ErrorBar::ErrorType::NoError};
	const AbstractColumn* yPlusColumn{nullptr};
	QString yPlusColumnPath;
	const AbstractColumn* yMinusColumn{nullptr};
	QString yMinusColumnPath;

	ErrorBar::Type type{ErrorBar::Type::Simple};
	double capSize{1.};
	Line* line{nullptr};

	ErrorBar* const q{nullptr};
};

#endif
