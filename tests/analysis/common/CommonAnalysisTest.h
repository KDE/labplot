/*
	File                 : CommonAnalysisTest.h
	Project              : LabPlot
	Description          : Tests for common analysis tasks
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2024 Martin Marmsoler <martin.marmsoler@gmail.com>

	SPDX-License-Identifier: GPL-2.0-or-later
*/
#ifndef COMMON_ANALYSIS_TEST_H
#define COMMON_ANALYSIS_TEST_H

#include <../AnalysisTest.h>

class CommonAnalysisTest : public AnalysisTest {
	Q_OBJECT

private Q_SLOTS:

	void dataImportRecalculationAnalysisCurveColumnDependency();
};
#endif // COMMON_ANALYSIS_TEST_H