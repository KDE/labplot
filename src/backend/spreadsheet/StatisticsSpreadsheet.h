/*
	File                 : StatisticsSpreadsheet.h
	Project              : LabPlot
	Description          : Aspect providing a spreadsheet with the columns statistics for the parent spreadsheet
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2023-2024 Alexander Semke <alexander.semke@web.de>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef STATISTICSSPREADSHEET_H
#define STATISTICSSPREADSHEET_H

#include "backend/spreadsheet/Spreadsheet.h"

#ifdef SDK
#include "labplot_export.h"
class LABPLOT_EXPORT StatisticsSpreadsheet : public Spreadsheet {
#else
class StatisticsSpreadsheet : public Spreadsheet {
#endif
	Q_OBJECT

public:
	explicit StatisticsSpreadsheet(Spreadsheet*, bool loading = false, AspectType type = AspectType::StatisticsSpreadsheet);
	~StatisticsSpreadsheet() override;

	enum class Metric {
		Count = 0x00000001,
		Minimum = 0x00000002,
		Maximum = 0x00000004,
		ArithmeticMean = 0x00000008,
		GeometricMean = 0x00000010,
		HarmonicMean = 0x00000020,
		ContraharmonicMean = 0x00000040,
		Mode = 0x00000080,
		FirstQuartile = 0x00000100,
		Median = 0x00000200,
		ThirdQuartile = 0x00000400,
		IQR = 0x00000800,
		Percentile1 = 0x00001000,
		Percentile5 = 0x00002000,
		Percentile10 = 0x00004000,
		Percentile90 = 0x00008000,
		Percentile95 = 0x00010000,
		Percentile99 = 0x00020000,
		Trimean = 0x00040000,
		Variance = 0x00080000,
		StandardDeviation = 0x00100000,
		MeanDeviation = 0x00200000,
		MeanDeviationAroundMedian = 0x00400000,
		MedianDeviation = 0x00800000,
		Skewness = 0x01000000,
		Kurtosis = 0x02000000,
		Entropy = 0x04000000,
		Range = 0x08000000
	};
	Q_DECLARE_FLAGS(Metrics, Metric)

	QIcon icon() const override;

	Metrics metrics() const;
	void setMetrics(Metrics);

	void save(QXmlStreamWriter*) const override;
	bool load(XmlStreamReader*, bool preview) override;

private:
	void init();
	void update();
	void updateColumnNames();

	Spreadsheet* m_spreadsheet{nullptr};
	Metrics m_metrics;
	QVector<Metric> m_metricValues;
	QVector<QString> m_metricNames;

	friend class SpreadsheetTest;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(StatisticsSpreadsheet::Metrics)

#endif
