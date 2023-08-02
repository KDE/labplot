/*
	File                 : ClevelandDotPlot.cpp
	Project              : LabPlot
	Description          : Dot Plot
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2023 Alexander Semke <alexander.semke@web.de>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "ClevelandDotPlot.h"
#include "ClevelandDotPlotPrivate.h"
#include <QIcon>

/**
 * \class ClevelandDotPlot
 * \brief Cleveland's Dot Plot - a modification of the bar plot showing symbol ("dots")
 * at the top of the bars instead of the bars.
 */

ClevelandDotPlot::ClevelandDotPlot(const QString& name) : LollipopPlot(name, AspectType::ClevelandDotPlot) {
	init();
}

ClevelandDotPlot::ClevelandDotPlot(const QString& name, ClevelandDotPlotPrivate* dd)
	: LollipopPlot(name, dd) {
	init();
}

// no need to delete the d-pointer here - it inherits from QGraphicsItem
// and is deleted during the cleanup in QGraphicsScene
ClevelandDotPlot::~ClevelandDotPlot() = default;

/*!
	Returns an icon to be used in the project explorer.
*/
QIcon ClevelandDotPlot::icon() const {
	return QIcon::fromTheme(QLatin1String("office-chart-bar"));
}
