/*
	File                 : StatisticsSpreadsheet.cpp
	Project              : LabPlot
	Description          : Aspect providing a spreadsheet table with column logic
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2023-2024 Alexander Semke <alexander.semke@web.de>
	SPDX-License-Identifier: GPL-2.0-or-later
*/
#include "StatisticsSpreadsheet.h"
#include "SpreadsheetModel.h"
#include "backend/lib/XmlStreamReader.h"
#include "backend/lib/macros.h"

#include <QIcon>
#include <QXmlStreamWriter>

#include <KConfigGroup>
#include <KLocalizedString>
#include <KSharedConfig>

/*!
  \class StatisticsSpreadsheet
  \brief Aspect providing a spreadsheet table with column logic.
  \ingroup backend
*/

StatisticsSpreadsheet::StatisticsSpreadsheet(Spreadsheet* spreadsheet, bool loading, AspectType type)
	: Spreadsheet(i18n("Column Statistics"), loading, type)
	, m_spreadsheet(spreadsheet) {
	m_metricNames = {
		{StatisticsSpreadsheet::Metric::Count, i18n("Count")},
		{StatisticsSpreadsheet::Metric::Minimum, i18n("Minimum")},
		{StatisticsSpreadsheet::Metric::Maximum, i18n("Maximum")},
		{StatisticsSpreadsheet::Metric::ArithmeticMean, i18n("ArithmeticMean")},
		{StatisticsSpreadsheet::Metric::GeometricMean, i18n("GeometricMean")},
		{StatisticsSpreadsheet::Metric::HarmonicMean, i18n("HarmonicMean")},
		{StatisticsSpreadsheet::Metric::ContraharmonicMean, i18n("ContraharmonicMean")},
		{StatisticsSpreadsheet::Metric::Mode, i18n("Mode")},
		{StatisticsSpreadsheet::Metric::FirstQuartile, i18n("FirstQuartile")},
		{StatisticsSpreadsheet::Metric::Median, i18n("Median")},
		{StatisticsSpreadsheet::Metric::ThirdQuartile, i18n("ThirdQuartile")},
		{StatisticsSpreadsheet::Metric::IQR, i18n("Interquartile Range")},
		{StatisticsSpreadsheet::Metric::Percentile1, i18n("Percentile1")},
		{StatisticsSpreadsheet::Metric::Percentile5, i18n("Percentile5")},
		{StatisticsSpreadsheet::Metric::Percentile10, i18n("Percentile10")},
		{StatisticsSpreadsheet::Metric::Percentile90, i18n("Percentile90")},
		{StatisticsSpreadsheet::Metric::Percentile95, i18n("Percentile95")},
		{StatisticsSpreadsheet::Metric::Percentile99, i18n("Percentile99")},
		{StatisticsSpreadsheet::Metric::Trimean, i18n("Trimean")},
		{StatisticsSpreadsheet::Metric::Range, i18n("Range")},
		{StatisticsSpreadsheet::Metric::Variance, i18n("Variance")},
		{StatisticsSpreadsheet::Metric::StandardDeviation, i18n("StandardDeviation")},
		{StatisticsSpreadsheet::Metric::MeanDeviation, i18n("MeanDeviation")},
		{StatisticsSpreadsheet::Metric::MeanDeviationAroundMedian, i18n("MeanDeviationAroundMedian")},
		{StatisticsSpreadsheet::Metric::MedianDeviation, i18n("MedianDeviation")},
		{StatisticsSpreadsheet::Metric::Skewness, i18n("Skewness")},
		{StatisticsSpreadsheet::Metric::Kurtosis, i18n("Kurtosis")},
		{StatisticsSpreadsheet::Metric::Entropy, i18n("Entropy")},
	};

	auto* model = m_spreadsheet->model();
	connect(model, &SpreadsheetModel::dataChanged, this, &StatisticsSpreadsheet::update);
	connect(model, &SpreadsheetModel::rowsRemoved, this, &StatisticsSpreadsheet::update);
	connect(model, &SpreadsheetModel::rowsInserted, this, &StatisticsSpreadsheet::update);
	connect(model, &SpreadsheetModel::columnsRemoved, this, &StatisticsSpreadsheet::update);
	connect(model, &SpreadsheetModel::columnsInserted, this, &StatisticsSpreadsheet::update);
	connect(model, &SpreadsheetModel::headerDataChanged, this, &StatisticsSpreadsheet::updateColumnNames);

	setUndoAware(false);
	setFixed(true);

	if (!loading)
		init();
}

StatisticsSpreadsheet::~StatisticsSpreadsheet() {
}

/*!
  Returns an icon to be used for decorating my views.
  */
QIcon StatisticsSpreadsheet::icon() const {
	return QIcon::fromTheme(QStringLiteral("view-statistics"));
}

StatisticsSpreadsheet::Metrics StatisticsSpreadsheet::metrics() const {
	return m_metrics;
}

void StatisticsSpreadsheet::setMetrics(Metrics metrics) {
	m_metrics = metrics;
	update();
}

/*!
	initializes the spreadsheet with the default number of columns and rows
*/
void StatisticsSpreadsheet::init() {
	// measures that are shown on default
	KConfig config;
	KConfigGroup group = config.group(QLatin1String("StatisticsSpreadsheet"));

	Metrics defaultMetrics;
	defaultMetrics.setFlag(StatisticsSpreadsheet::Metric::Count);
	defaultMetrics.setFlag(StatisticsSpreadsheet::Metric::Minimum);
	defaultMetrics.setFlag(StatisticsSpreadsheet::Metric::Maximum);
	defaultMetrics.setFlag(StatisticsSpreadsheet::Metric::ArithmeticMean);
	defaultMetrics.setFlag(StatisticsSpreadsheet::Metric::Variance);
	defaultMetrics.setFlag(StatisticsSpreadsheet::Metric::StandardDeviation);

	m_metrics = static_cast<Metrics>(group.readEntry(QStringLiteral("Metrics"), static_cast<int>(defaultMetrics)));

	update();
}

/*!
 * updates the content of the statistics spreadsheet.
 * called when the data in the parent spreadsheet was modified.
 */
void StatisticsSpreadsheet::update() {
	// determine the number of activated metrics and properly resize the spreadsheet
	int colCount = 1; // first column for "column name"
	auto it = m_metricNames.constBegin();
	while (it != m_metricNames.constEnd()) {
		if (m_metrics.testFlag(it.key()))
			++colCount;
		++it;
	}

	setUndoAware(false);
	setRowCount(m_spreadsheet->columnCount());
	setColumnCount(colCount);
	setUndoAware(true);

	// make all columns in this statistics spreadsheet undo unaware
	const auto& statisticsColumns = children<Column>();
	for (auto* col : statisticsColumns) {
		col->setUndoAware(false);
		col->setFixed(true);
	}

	// rename the columns
	statisticsColumns.at(0)->setName(i18n("Column"));
	statisticsColumns.at(0)->setColumnMode(AbstractColumn::ColumnMode::Text);

	int colIndex = 1;
	it = m_metricNames.constBegin();
	while (it != m_metricNames.constEnd()) {
		if (m_metrics.testFlag(it.key())) {
			auto* col = statisticsColumns.at(colIndex);
			col->setName(it.value());

			if (it.key() == StatisticsSpreadsheet::Metric::Count)
				col->setColumnMode(AbstractColumn::ColumnMode::Integer);
			else
				col->setColumnMode(AbstractColumn::ColumnMode::Double);

			++colIndex;
		}
		++it;
	}

	// show the column names in the first column of the statistics spreadsheet
	const auto& columns = m_spreadsheet->children<Column>();
	auto* statisticsColumn = statisticsColumns.at(0);
	for (int i = 0; i < columns.count(); ++i)
		statisticsColumn->setTextAt(i, columns.at(i)->name());

	// show other statistics metrics that were activated
	colIndex = 1;
	it = m_metricNames.constBegin();
	while (it != m_metricNames.constEnd()) {
		if (m_metrics.testFlag(it.key())) {
			statisticsColumn = statisticsColumns.at(colIndex);
			for (int i = 0; i < columns.count(); ++i) {
				const auto* column = columns.at(i);
				const auto& statistics = column->statistics();

				switch (it.key()) {
				case Metric::Count:
					statisticsColumn->setIntegerAt(i, statistics.size);
					break;
				case Metric::Minimum:
					statisticsColumn->setValueAt(i, statistics.minimum);
					break;
				case Metric::Maximum:
					statisticsColumn->setValueAt(i, statistics.maximum);
					break;
				case Metric::ArithmeticMean:
					statisticsColumn->setValueAt(i, statistics.arithmeticMean);
					break;
				case Metric::GeometricMean:
					statisticsColumn->setValueAt(i, statistics.geometricMean);
					break;
				case Metric::HarmonicMean:
					statisticsColumn->setValueAt(i, statistics.harmonicMean);
					break;
				case Metric::ContraharmonicMean:
					statisticsColumn->setValueAt(i, statistics.contraharmonicMean);
					break;
				case Metric::Mode:
					statisticsColumn->setValueAt(i, statistics.mode);
					break;
				case Metric::FirstQuartile:
					statisticsColumn->setValueAt(i, statistics.firstQuartile);
					break;
				case Metric::Median:
					statisticsColumn->setValueAt(i, statistics.median);
					break;
				case Metric::ThirdQuartile:
					statisticsColumn->setValueAt(i, statistics.thirdQuartile);
					break;
				case Metric::IQR:
					statisticsColumn->setValueAt(i, statistics.iqr);
					break;
				case Metric::Percentile1:
					statisticsColumn->setValueAt(i, statistics.percentile_1);
					break;
				case Metric::Percentile5:
					statisticsColumn->setValueAt(i, statistics.percentile_5);
					break;
				case Metric::Percentile10:
					statisticsColumn->setValueAt(i, statistics.percentile_10);
					break;
				case Metric::Percentile90:
					statisticsColumn->setValueAt(i, statistics.percentile_90);
					break;
				case Metric::Percentile95:
					statisticsColumn->setValueAt(i, statistics.percentile_95);
					break;
				case Metric::Percentile99:
					statisticsColumn->setValueAt(i, statistics.percentile_99);
					break;
				case Metric::Trimean:
					statisticsColumn->setValueAt(i, statistics.trimean);
					break;
				case Metric::Range:
					statisticsColumn->setValueAt(i, statistics.maximum - statistics.minimum);
					break;
				case Metric::Variance:
					statisticsColumn->setValueAt(i, statistics.variance);
					break;
				case Metric::StandardDeviation:
					statisticsColumn->setValueAt(i, statistics.standardDeviation);
					break;
				case Metric::MeanDeviation:
					statisticsColumn->setValueAt(i, statistics.meanDeviation);
					break;
				case Metric::MeanDeviationAroundMedian:
					statisticsColumn->setValueAt(i, statistics.meanDeviationAroundMedian);
					break;
				case Metric::MedianDeviation:
					statisticsColumn->setValueAt(i, statistics.medianDeviation);
					break;
				case Metric::Skewness:
					statisticsColumn->setValueAt(i, statistics.skewness);
					break;
				case Metric::Kurtosis:
					statisticsColumn->setValueAt(i, statistics.kurtosis);
					break;
				case Metric::Entropy:
					statisticsColumn->setValueAt(i, statistics.entropy);
					break;
				}
			}

			++colIndex;
		}

		++it;
	}
}

/*!
 * updates the content of the first column that has the names of the columns of the parent spreadsheet.
 * called when the columns in the parent spreadsheet are renamed.
 */
void StatisticsSpreadsheet::updateColumnNames() {
	const auto& columns = m_spreadsheet->children<Column>();
	auto* nameColumn =  children<Column>().at(0);
	for (int i = 0; i < columns.count(); ++i)
		nameColumn->setTextAt(i, columns.at(i)->name());
}

// ##############################################################################
// ##################  Serialization/Deserialization  ###########################
// ##############################################################################
/*!
  Saves as XML.
 */
void StatisticsSpreadsheet::save(QXmlStreamWriter* writer) const {
	writer->writeStartElement(QStringLiteral("statisticsSpreadsheet"));
	writeBasicAttributes(writer);
	writer->writeAttribute(QStringLiteral("metrics"), QString::number(m_metrics));

	// columns
	const auto& columns = children<Column>(ChildIndexFlag::IncludeHidden);
	for (auto* column : columns)
		column->save(writer);

	writer->writeEndElement(); // "statisticsSpreadsheet"
}

/*!
  Loads from XML.
*/
bool StatisticsSpreadsheet::load(XmlStreamReader* reader, bool preview) {
	if (preview)
		return true;

	if (!readBasicAttributes(reader))
		return false;

	const auto& attribs = reader->attributes();
	const auto& str = attribs.value(QStringLiteral("metrics")).toString();
	if (str.isEmpty())
		reader->raiseMissingAttributeWarning(QStringLiteral("metrics"));
	else
		m_metrics = static_cast<StatisticsSpreadsheet::Metric>(str.toInt());

	// read child elements
	while (!reader->atEnd()) {
		reader->readNext();

		if (reader->isEndElement() && reader->name() == QStringLiteral("statisticsSpreadsheet"))
			break;

		if (reader->isStartElement()) {
			if (reader->name() == QStringLiteral("column")) {
				Column* column = new Column(QString());
				column->setIsLoading(true);
				if (!column->load(reader, preview)) {
					delete column;
					setColumnCount(0);
					return false;
				}
				addChildFast(column);
			} else { // unknown element
				reader->raiseUnknownElementWarning();
				if (!reader->skipToEndElement())
					return false;
			}
		}
	}

	return !reader->hasError();
}
