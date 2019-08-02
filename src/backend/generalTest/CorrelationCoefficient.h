/***************************************************************************
        File                 : CorrelationCoefficient.h
        Project              : LabPlot
        Description          : Finding Correlation Coefficient on data provided
        --------------------------------------------------------------------
        Copyright            : (C) 2019 Devanshu Agarwal(agarwaldevanshu8@gmail.com)

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *  This program is free software; you can redistribute it and/or modify   *
 *  it under the terms of the GNU General Public License as published by   *
 *  the Free Software Foundation; either version 2 of the License, or      *
 *  (at your option) any later version.                                    *
 *                                                                         *
 *  This program is distributed in the hope that it will be useful,        *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 *  GNU General Public License for more details.                           *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the Free Software           *
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor,                    *
 *   Boston, MA  02110-1301  USA                                           *
 *                                                                         *
 ***************************************************************************/

#ifndef CORRELATIONCOEFFICIENT_H
#define CORRELATIONCOEFFICIENT_H

#include "GeneralTest.h"

class CorrelationCoefficientView;

class CorrelationCoefficient : public GeneralTest {
	Q_OBJECT

public:
	explicit CorrelationCoefficient(const QString& name);
	~CorrelationCoefficient() override;

	enum Test {
		Pearson,
		Kendall,
		Spearman
	};

	double correlationValue() const;
	QList<double> statisticValue() const;
	QList<double> pValue() const;

	QWidget* view() const override;

	void performTest(Test m_test, bool categoricalVariable = false);
private:
	void performPearson(bool categoricalVariable);
	void performKendall();
	void performSpearman();

	int findDiscordants(int* ranks, int start, int end);

	void convertToRanks(const Column* col, int N, QMap<double, int> &ranks);
	void convertToRanks(const Column* col, QMap<double, int> &ranks);

	double m_correlationValue;
	QList<double> m_statisticValue;
	QList<double> m_pValue;
};

#endif // CORRELATIONCOEFFICIENT_H
