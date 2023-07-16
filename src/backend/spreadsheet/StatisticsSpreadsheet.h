/*
	File                 : StatisticsSpreadsheet.h
	Project              : LabPlot
	Description          : Aspect providing a spreadsheet with the columns statistics for the parent spreadsheet
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2023 Alexander Semke <alexander.semke@web.de>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef STATISTICSSPREADSHEET_H
#define STATISTICSSPREADSHEET_H

#include "backend/spreadsheet/Spreadsheet.h"

class StatisticsSpreadsheet : public Spreadsheet {
	Q_OBJECT

public:
	explicit StatisticsSpreadsheet(Spreadsheet*, bool loading = false, AspectType type = AspectType::StatisticsSpreadsheet);
	~StatisticsSpreadsheet() override;

	enum class Metric {
		Count                     = 0x00000000,
		Minimum                   = 0x00000001,
		Maximum                   = 0x00000002,
		ArithmeticMean            = 0x00000004,
		GeometricMean             = 0x00000008,
		HarmonicMean              = 0x00000010,
		ContraharmonicMean        = 0x00000020,
		Mode                      = 0x00000040,
		FirstQuartile             = 0x00000080,
		Median                    = 0x00000100,
		ThirdQuartile             = 0x00000200,
		IQR                       = 0x00000400,
		Percentile1               = 0x00000800,
		Percentile5               = 0x00001000,
		Percentile10              = 0x00002000,
		Percentile90              = 0x00004000,
		Percentile95              = 0x00008000,
		Percentile99              = 0x00010000,
		Trimean                   = 0x00020000,
		Variance                  = 0x00040000,
		StandardDeviation         = 0x00080000,
		MeanDeviation             = 0x00100000,
		MeanDeviationAroundMedian = 0x00200000,
		MedianDeviation           = 0x00400000,
		Skewness                  = 0x00800000,
		Kurtosis                  = 0x01000000,
		Entropy                   = 0x02000000
	};
	Q_DECLARE_FLAGS(Metrics, Metric)

	QIcon icon() const override;

	Metrics metrics() const;
	void setMetrics(Metrics);

	void save(QXmlStreamWriter*) const override;
	bool load(XmlStreamReader*, bool preview) override;

private:
	void init();
	void updateStatisticsSpreadsheet();

	Spreadsheet* m_spreadsheet{nullptr};
	Metrics m_metrics;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(StatisticsSpreadsheet::Metrics)

#endif
