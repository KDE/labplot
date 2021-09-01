/*
    File                 : LabPlotProjectParser.h
    Project              : LabPlot
    Description          : parser for LabPlot projects
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2017 Alexander Semke (alexander.semke@web.de)

*/

/***************************************************************************
 *                                                                         *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *                                                                         *
 ***************************************************************************/
#ifndef LABPLOTPROJECTPARSER_H
#define LABPLOTPROJECTPARSER_H

#include "backend/datasources/projects/ProjectParser.h"

class LabPlotProjectParser : public ProjectParser {
	Q_OBJECT

public:
	LabPlotProjectParser();

protected:
	bool load(Project*, bool) override;
};

#endif // LABPLOTPROJECTPARSER_H
