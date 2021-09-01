/*
    File                 : WorksheetElementGroup.cpp
    Project              : LabPlot/SciDAVis
    Description          : Groups worksheet elements for collective operations.
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2009 Tilman Benkert (thzs*gmx.net)
    (replace * with @ in the email addresses)
    SPDX-License-Identifier: GPL-2.0-or-later
*/


#include "backend/worksheet/WorksheetElementGroup.h"

/**
 * \class WorksheetElementGroup
 * \brief Groups worksheet elements for collective operations.
 *
 * The role of this class is similar to object groups in a vector drawing program. 
 *
 */

WorksheetElementGroup::WorksheetElementGroup(const QString &name)
	: WorksheetElementContainer(name, AspectType::WorksheetElementGroup) {
}

WorksheetElementGroup::WorksheetElementGroup(const QString &name, WorksheetElementContainerPrivate *dd) 
	: WorksheetElementContainer(name, dd, AspectType::WorksheetElementGroup) {
}

WorksheetElementGroup::~WorksheetElementGroup() = default;

