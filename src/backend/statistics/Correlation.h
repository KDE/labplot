/*
	File                 : Correlation.h
	Project              : LabPlot
	Description          : Bivariate Correlation Coefficients
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2019  Devanshu Agarwal(agarwaldevanshu8@gmail.com)
	SPDX-FileCopyrightText: 2023 Alexander Semke <alexander.semke@web.de>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef CORRELATION_H
#define CORRELATION_H

#include "GeneralTest.h"

class Correlation : public GeneralTest {
	Q_OBJECT

public:
	explicit Correlation(const QString& name);
	~Correlation() override;

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

#endif // CORRELATION_H
