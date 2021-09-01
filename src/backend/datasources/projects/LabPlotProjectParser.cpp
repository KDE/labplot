/*
    File                 : LabPlotProjectParser.h
    Project              : LabPlot
    Description          : parser for LabPlot projects
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2017 Alexander Semke (alexander.semke@web.de)

SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "backend/datasources/projects/LabPlotProjectParser.h"
#include "backend/core/Project.h"

/*!
\class LabPlotProjectParser
\brief parser for LabPlot projects.

\ingroup datasources
*/

LabPlotProjectParser::LabPlotProjectParser() : ProjectParser() {
	m_topLevelClasses = {AspectType::Folder, AspectType::Workbook,
	                     AspectType::Spreadsheet, AspectType::Matrix,
	                     AspectType::Worksheet, AspectType::CantorWorksheet,
	                     AspectType::Datapicker, AspectType::LiveDataSource};
}

bool LabPlotProjectParser::load(Project* project, bool preview) {
	return project->load(m_projectFileName, preview);
}
