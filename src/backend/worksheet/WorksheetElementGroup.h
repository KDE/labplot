/*
    File                 : WorksheetElementGroup.h
    Project              : LabPlot/SciDAVis
    Description          : Groups worksheet elements for collective operations.
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2009 Tilman Benkert <thzs*gmx.net  (use @ for *)>
    SPDX-License-Identifier: GPL-2.0-or-later
*/


#ifndef WORKSHEETELEMENTGROUP_H
#define WORKSHEETELEMENTGROUP_H

#include "backend/worksheet/WorksheetElementContainer.h"

class WorksheetElementGroup: public WorksheetElementContainer {
	Q_OBJECT

public:
	explicit WorksheetElementGroup(const QString &name);
	~WorksheetElementGroup() override;

protected:
	WorksheetElementGroup(const QString &name, WorksheetElementContainerPrivate *dd);
};

#endif

