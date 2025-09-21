/*
	File                 : PlotPrivate.h
	Project              : LabPlot
	Description          : Plot - private implementation
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2022-2023 Alexander Semke <alexander.semke@web.de>

	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef PLOTPRIVATE_H
#define PLOTPRIVATE_H

#include "backend/worksheet/WorksheetElementPrivate.h"
#include "backend/worksheet/plots/cartesian/Plot.h"

class PlotPrivate : public WorksheetElementPrivate {
public:
	explicit PlotPrivate(Plot*);
	virtual bool activatePlot(QPointF mouseScenePos, double maxDist = -1);
	Plot* const q;
	bool legendVisible{true};

protected:
	virtual void contextMenuEvent(QGraphicsSceneContextMenuEvent*) override;

protected:
	QPixmap m_pixmap;
	QImage m_hoverEffectImage;
	QImage m_selectionEffectImage;
	bool m_hoverEffectImageIsDirty{false};
	bool m_selectionEffectImageIsDirty{false};
};

#endif
