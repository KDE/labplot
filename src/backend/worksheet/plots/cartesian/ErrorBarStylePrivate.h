/*
	File                 : ErrorBarStylePrivate.h
	Project              : LabPlot
	Description          : Private members of ErrorBarStyle
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2024 Alexander Semke <alexander.semke@web.de>

	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef ERRORBARSTYLEPRIVATE_H
#define ERRORBARSTYLEPRIVATE_H

class ErrorBarStylePrivate {
public:
	explicit ErrorBarStylePrivate(ErrorBarStyle*);

	QString name() const;
	void update();
	void updatePixmap();

	QString prefix{QLatin1String("ErrorBarStyle")};
	ErrorBarStyle::Type type{ErrorBarStyle::Type::Simple};
	double capSize{1.};
	Line* line{nullptr};

	ErrorBarStyle* const q{nullptr};
};

#endif
