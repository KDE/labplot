/*
	File                 : ClevelandDotPlot.h
	Project              : LabPlot
	Description          : Dot Plot
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2023 Alexander Semke <alexander.semke@web.de>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef DOTPLOT_H
#define DOTPLOT_H

#include "backend/worksheet/plots/cartesian/LollipopPlot.h"

class ClevelandDotPlotPrivate;

#ifdef SDK
#include "labplot_export.h"
class LABPLOT_EXPORT ClevelandDotPlot : LollipopPlot {
#else
class ClevelandDotPlot : public LollipopPlot {
#endif
	Q_OBJECT

public:
	explicit ClevelandDotPlot(const QString&);
	~ClevelandDotPlot() override;

	QIcon icon() const override;

protected:
	ClevelandDotPlot(const QString& name, ClevelandDotPlotPrivate* dd);

private:
	Q_DECLARE_PRIVATE(ClevelandDotPlot)
};

#endif
