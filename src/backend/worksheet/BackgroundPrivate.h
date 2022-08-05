/*
	File                 : BackgroundPrivate.h
	Project              : LabPlot
	Description          : Private members of Background
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2022 Alexander Semke <alexander.semke@web.de>

	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef BACKGROUNDPRIVATE_H
#define BACKGROUNDPRIVATE_H

#include <QBrush>
#include <QPen>

class BackgroundPrivate {
public:
	explicit BackgroundPrivate(Background*);

	QString name() const;
	void update();
	void updatePosition();

	QString prefix{QLatin1String("Background")};
	bool enabledAvailable{false};
	bool positionAvailable{false};

	bool enabled{false};
	Background::Position position;
	Background::Type type{Background::Type::Color};
	Background::ColorStyle colorStyle{Background::ColorStyle::SingleColor};
	Background::ImageStyle imageStyle{Background::ImageStyle::Scaled};
	Qt::BrushStyle brushStyle{Qt::SolidPattern};
	QColor firstColor{Qt::white};
	QColor secondColor{Qt::black};
	QString fileName;
	double opacity{1.0};
	Background* const q{nullptr};
};

#endif
