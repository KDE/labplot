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

class AbstractColumn;

class ErrorBarPrivate {
public:
	explicit ErrorBarPrivate(ErrorBar*);

	QString name() const;
	void update();

	QString prefix{QLatin1String("ErrorBar")};

	ErrorBar::Type type{ErrorBar::Type::NoError};
	const AbstractColumn* plusColumn{nullptr};
	QString plusColumnPath;
	const AbstractColumn* minusColumn{nullptr};
	QString minusColumnPath;

	ErrorBar* const q{nullptr};
};

#endif
