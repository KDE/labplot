/*
	File                 : CorrelationCoefficient.h
	Project              : LabPlot
	Description          : Correlation Coefficients/Tests
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2019  Devanshu Agarwal(agarwaldevanshu8@gmail.com)
	SPDX-FileCopyrightText: 2023 Alexander Semke <alexander.semke@web.de>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef CORRELATIONCOEFFICIENT_H
#define CORRELATIONCOEFFICIENT_H

#include "GeneralTest.h"

class CorrelationCoefficient : public GeneralTest {
	Q_OBJECT

public:
	explicit CorrelationCoefficient(const QString& name);
	~CorrelationCoefficient() override;

	enum Method {Pearson, Spearman, Kendall, ChiSquare};

	void performTest(Method, bool categoricalVariable = false, bool calculateStats = true);
	void initInputStatsTable(int test, bool calculateStats, int nRows, int nColumns);
	void setInputStatsTableNRows(int nRows);
	void setInputStatsTableNCols(int nColumns);
	void exportStatTableToSpreadsheet();
	double correlationValue() const;
	QList<double> statisticValue() const;

private:
	void performPearson(bool categoricalVariable);
	void performKendall();
	void performSpearman();
	void performChiSquareIndpendence(bool calculateStats);

	int findDiscordants(int* ranks, int start, int end);
	void convertToRanks(const Column* col, int N, QMap<double, int> &ranks);

	double m_correlationValue;
	QList<double> m_statisticValue;
	QList<double> m_pValue;
};

#endif // CORRELATIONCOEFFICIENT_H
