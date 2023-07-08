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

	enum class Metric { Count,
		Minimum,
		Maximum,
		ArithmeticMean,
		GeometricMean,
		HarmonicMean,
		ContraharmonicMean,
		Mode,
		FirstQuartile,
		Median,
		ThirdQuartile,
		IQR,
		Percentile1,
		Percentile5,
		Percentile10,
		Percentile90,
		Percentile95,
		Percentile99,
		Trimean,
		Variance,
		StandardDeviation,
		MeanDeviation,
		MeanDeviationAroundMedian,
		MedianDeviation,
		Skewness,
		Kurtosis,
		Entropy
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
