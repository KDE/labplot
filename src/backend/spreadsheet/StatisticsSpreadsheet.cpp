/*
	File                 : StatisticsSpreadsheet.cpp
	Project              : LabPlot
	Description          : Aspect providing a spreadsheet table with column logic
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2023 Alexander Semke <alexander.semke@web.de>
	SPDX-License-Identifier: GPL-2.0-or-later
*/
#include "StatisticsSpreadsheet.h"

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
	: Spreadsheet(i18n("Column Statistics of '%1'", spreadsheet->name()), loading, type),
	m_spreadsheet(spreadsheet) {
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

/*!
	initializes the spreadsheet with the default number of columns and rows
*/
void StatisticsSpreadsheet::init() {
	// KConfig config;
	// KConfigGroup group = config.group(QLatin1String("StatisticsSpreadsheet"));

	setRowCount(m_spreadsheet->columnCount());
	// setColumnCount(22);
	setColumnCount(7);

	const auto& columns = children<Column>();
	columns.at(0)->setName(i18n("Column"));
	columns.at(0)->setColumnMode(AbstractColumn::ColumnMode::Text);
	columns.at(1)->setName(i18n("Count"));
	columns.at(1)->setColumnMode(AbstractColumn::ColumnMode::Integer);
	columns.at(2)->setName(i18n("Minimum"));
	columns.at(3)->setName(i18n("Maximum"));
	columns.at(4)->setName(i18n("Arithmetic mean"));
	columns.at(5)->setName(i18n("Variance"));
	columns.at(6)->setName(i18n("Standard Deviation"));

	// columns.at(0)->setName(i18n("Column"));
	// columns.at(0)->setColumnMode(AbstractColumn::ColumnMode::Text);
	// columns.at(1)->setName(i18n("Count"));
	// columns.at(1)->setColumnMode(AbstractColumn::ColumnMode::Integer);
	// columns.at(2)->setName(i18n("Minimum"));
	// columns.at(3)->setName(i18n("Maximum"));
	// columns.at(4)->setName(i18n("Arithmetic mean"));
	// columns.at(5)->setName(i18n("Geometric mean"));
	// columns.at(6)->setName(i18n("Harmonic mean"));
	// columns.at(7)->setName(i18n("Contraharmonic mean"));
	// columns.at(8)->setName(i18n("Mode"));
	// columns.at(9)->setName(i18n("First quartile"));
	// columns.at(10)->setName(i18n("Median"));
	// columns.at(11)->setName(i18n("Third quartile"));
	// columns.at(12)->setName(i18n("Trimean"));
	// columns.at(13)->setName(i18n("Variance"));
	// columns.at(14)->setName(i18n("Standard Deviation"));
	// columns.at(15)->setName(i18n("Mean absolute deviation around mean"));
	// columns.at(16)->setName(i18n("Mean absolute deviation around median"));
	// columns.at(17)->setName(i18n("Median absolute deviation"));
	// columns.at(18)->setName(i18n("Interquartile range"));
	// columns.at(19)->setName(i18n("Skewness"));
	// columns.at(20)->setName(i18n("Kurtosis"));
	// columns.at(21)->setName(i18n("Entropy"));

	updateStatisticsSpreadsheet();
}

void StatisticsSpreadsheet::updateStatisticsSpreadsheet() {
	if (rowCount() != m_spreadsheet->columnCount())
		setRowCount(m_spreadsheet->columnCount());

	const auto& columns = m_spreadsheet->children<Column>();
	const auto& statisticsColumns = children<Column>();
	for (int i = 0; i < columns.count(); ++i) {
		const auto* column = columns.at(i);
		const auto& statistics = column->statistics();

		statisticsColumns.at(0)->setTextAt(i, column->name());
		statisticsColumns.at(1)->setIntegerAt(i, statistics.size);
		statisticsColumns.at(2)->setValueAt(i, statistics.minimum);
		statisticsColumns.at(3)->setValueAt(i, statistics.maximum);
		statisticsColumns.at(4)->setValueAt(i, statistics.arithmeticMean);
		statisticsColumns.at(5)->setValueAt(i, statistics.variance);
		statisticsColumns.at(6)->setValueAt(i, statistics.standardDeviation);

		// statisticsColumns.at(0)->setTextAt(i, column->name());
		// statisticsColumns.at(1)->setIntegerAt(i, statistics.size);
		// statisticsColumns.at(2)->setValueAt(i, statistics.minimum);
		// statisticsColumns.at(3)->setValueAt(i, statistics.maximum);
		// statisticsColumns.at(4)->setValueAt(i, statistics.arithmeticMean);
		// statisticsColumns.at(5)->setValueAt(i, statistics.geometricMean);
		// statisticsColumns.at(6)->setValueAt(i, statistics.harmonicMean);
		// statisticsColumns.at(7)->setValueAt(i, statistics.contraharmonicMean);
		// statisticsColumns.at(8)->setValueAt(i, statistics.mode);
		// statisticsColumns.at(9)->setValueAt(i, statistics.firstQuartile);
		// statisticsColumns.at(10)->setValueAt(i, statistics.median);
		// statisticsColumns.at(11)->setValueAt(i, statistics.thirdQuartile);
		// statisticsColumns.at(12)->setValueAt(i, statistics.trimean);
		// statisticsColumns.at(13)->setValueAt(i, statistics.variance);
		// statisticsColumns.at(14)->setValueAt(i, statistics.standardDeviation);
		// statisticsColumns.at(15)->setValueAt(i, statistics.meanDeviation);
		// statisticsColumns.at(16)->setValueAt(i, statistics.meanDeviationAroundMedian);
		// statisticsColumns.at(17)->setValueAt(i, statistics.medianDeviation);
		// statisticsColumns.at(18)->setValueAt(i, statistics.iqr);
		// statisticsColumns.at(19)->setValueAt(i, statistics.skewness);
		// statisticsColumns.at(20)->setValueAt(i, statistics.kurtosis);
		// statisticsColumns.at(21)->setValueAt(i, statistics.entropy);
	}
}

// ##############################################################################
// ##################  Serialization/Deserialization  ###########################
// ##############################################################################
/*!
  Saves as XML.
 */
void StatisticsSpreadsheet::save(QXmlStreamWriter* writer) const {

}

/*!
  Loads from XML.
*/
bool StatisticsSpreadsheet::load(XmlStreamReader* reader, bool preview) {

	return true;
}
