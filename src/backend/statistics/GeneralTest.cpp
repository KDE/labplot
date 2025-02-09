/***************************************************************************
	File                 : GeneralTest.cpp
	Project              : LabPlot
	Description          : Performing hypothesis tests on provided data
	--------------------------------------------------------------------
	Copyright            : (C) 2019 Devanshu Agarwal
						   (agarwaldevanshu8@gmail.com)
	Copyright            : (C) 2025 Kuntal Bar
						   (barkuntal6@gmail.com)

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor,
Boston, MA  02110-1301  USA
***************************************************************************/

#include "GeneralTest.h"
#include "backend/core/column/Column.h"
#include "backend/lib/XmlStreamReader.h"
#include "backend/lib/macros.h"
#include "backend/spreadsheet/Spreadsheet.h"
#include "backend/statistics/TableModel.h"
#include "frontend/statistics/GeneralTestView.h"

#include <QLabel>
#include <QMenu>
#include <QStandardItemModel>
#include <QVBoxLayout>

#include <KLocalizedString>

#include <gsl/gsl_math.h>
#include <math.h>
extern "C" {
#include "backend/nsl/nsl_statistical_test.h"
}

GeneralTest::GeneralTest(const QString& name, const AspectType& type)
	: AbstractPart(name, type)
	, m_summaryLayout(new QVBoxLayout())
	, m_inputStatsTableModel(new TableModel()) {
	m_inputStatsTableModel->setParent(this);
	for (int i = 0; i < RESULT_LINES_COUNT; i++) {
		m_resultLabels[i] = new QLabel();
		m_summaryLayout->addWidget(m_resultLabels[i]);
	}
}

GeneralTest::~GeneralTest() {
	// No additional cleanup required.
}

void GeneralTest::setDataSourceSpreadsheet(Spreadsheet* spreadsheet) {
	m_dataSourceSpreadsheet = spreadsheet;
	m_allColumnNames.clear();
	for (auto* col : m_dataSourceSpreadsheet->children<Column>()) {
		m_allColumnNames << col->name();
	}
}

Spreadsheet* GeneralTest::getDataSourceSpreadsheet() const {
	return m_dataSourceSpreadsheet;
}

QString GeneralTest::getTestName() const {
	return m_currentTestName;
}

QString GeneralTest::getStatsTable() const {
	return m_statsTable;
}

QVBoxLayout* GeneralTest::getSummaryLayout() const {
	return m_summaryLayout;
}

QAbstractItemModel* GeneralTest::getInputStatsTableModel() const {
	return m_inputStatsTableModel;
}

void GeneralTest::setColumns(const QStringList& cols) {
	m_columns.clear();
	// For each non-empty column name, obtain the column pointer.
	for (const QString& colName : cols) {
		if (!colName.isEmpty()) {
			Column* col = m_dataSourceSpreadsheet->column(colName);
			if (col)
				m_columns.append(col);
		}
	}
}

QStringList GeneralTest::getAllColumns() const {
	QStringList allColumns;
	for (auto* col : m_dataSourceSpreadsheet->children<Column>())
		allColumns << col->name();
	return allColumns;
}

void GeneralTest::setColumns(const QVector<Column*>& cols) {
	m_columns = cols;
}

/********************************************************************************************************************
 *                        Protected Helper Functions Implementations (Renamed)
 ********************************************************************************************************************/
int GeneralTest::extractTestType(int test) {
	return test & 0x0F;
}

int GeneralTest::extractTestSubtype(int test) {
	return test & 0xF0;
}

QString GeneralTest::formatRoundedValue(QVariant number, int precision) {
	// Only round if the value is a floating point type.
	if (number.userType() == QMetaType::Double || number.userType() == QMetaType::Float) {
		double multiplier = gsl_pow_int(10, precision);
		int tempNum = static_cast<int>(number.toDouble() * multiplier * 10);

		if (tempNum % 10 < 5)
			return QString::number((tempNum / 10) / multiplier);
		else
			return QString::number((tempNum / 10 + 1) / multiplier);
	}
	return i18n("%1", number.toString());
}

double GeneralTest::calculateSum(const Column* column, int N) {
	if (!column->isNumeric())
		return 0;

	if (N < 0)
		N = column->statistics().size;

	double sum = 0;
	for (int i = 0; i < N; i++)
		sum += column->valueAt(i);
	return sum;
}

double GeneralTest::calculateSumOfSquares(const Column* column, int N) {
	if (!column->isNumeric())
		return 0;

	if (N < 0)
		N = column->statistics().size;

	double sumSq = 0;
	for (int i = 0; i < N; i++)
		sumSq += gsl_pow_2(column->valueAt(i));
	return sumSq;
}

GeneralTest::GeneralErrorType GeneralTest::computeColumnStats(const Column* column, int& count, double& sum, double& mean, double& stdDev) {
	const auto& statistics = column->statistics();
	count = statistics.size;
	if (count < 1)
		return GeneralTest::ErrorEmptyColumn;

	mean = statistics.arithmeticMean;
	sum = calculateSum(column, count);
	stdDev = statistics.standardDeviation;
	return GeneralTest::NoError;
}

GeneralTest::GeneralErrorType
GeneralTest::computePairedColumnStats(const Column* column1, const Column* column2, int& count, double& sum, double& mean, double& stdDev) {
	sum = 0;
	mean = 0;
	stdDev = 0;

	int count1 = column1->rowCount();
	int count2 = column2->rowCount();

	count = qMin(count1, count2);
	double value1, value2;
	for (int i = 0; i < count; i++) {
		value1 = column1->valueAt(i);
		value2 = column2->valueAt(i);

		if (std::isnan(value1) || std::isnan(value2)) {
			if (std::isnan(value1) && std::isnan(value2))
				count = i;
			else
				return GeneralTest::ErrorUnqualSize;
			break;
		}
		sum += value1 - value2;
	}

	if (count < 1)
		return GeneralTest::ErrorEmptyColumn;

	mean = sum / count;

	double diff, variance = 0;
	for (int i = 0; i < count; i++) {
		value1 = column1->valueAt(i);
		value2 = column2->valueAt(i);
		diff = value1 - value2;
		variance += gsl_pow_2(diff - mean);
	}

	if (count > 1)
		variance /= (count - 1);

	stdDev = sqrt(variance);
	return GeneralTest::NoError;
}

void GeneralTest::countUniquePartitions(Column* column, int& np, int& totalRows) {
	totalRows = column->rowCount();
	np = 0;
	QString cellValue;
	QMap<QString, bool> discoveredValues;

	// Temporarily switch the column mode to text.
	AbstractColumn::ColumnMode originalMode = column->columnMode();
	column->setColumnMode(AbstractColumn::ColumnMode::Text);

	for (int i = 0; i < totalRows; i++) {
		cellValue = column->textAt(i);
		if (cellValue.isEmpty()) {
			totalRows = i;
			break;
		}
		if (discoveredValues.contains(cellValue))
			continue;
		discoveredValues[cellValue] = true;
		np++;
	}
	column->setColumnMode(originalMode);
}

GeneralTest::GeneralErrorType GeneralTest::computeCategoricalStats(Column* column1,
																   Column* column2,
																   int n[],
																   double sum[],
																   double mean[],
																   double stdDev[],
																   QMap<QString, int>& colName,
																   const int& np,
																   const int& totalRows) {
	Column* columns[] = {column1, column2};

	for (int i = 0; i < np; i++) {
		n[i] = 0;
		sum[i] = 0;
		mean[i] = 0;
		stdDev[i] = 0;
	}

	AbstractColumn::ColumnMode originalMode = columns[0]->columnMode();
	columns[0]->setColumnMode(AbstractColumn::ColumnMode::Text);

	int partitionNumber = 1;
	for (int i = 0; i < totalRows; i++) {
		QString name = columns[0]->textAt(i);
		double value = columns[1]->valueAt(i);

		if (std::isnan(value)) {
			columns[0]->setColumnMode(originalMode);
			return GeneralTest::ErrorUnqualSize;
		}

		if (colName[name] == 0) {
			colName[name] = partitionNumber;
			partitionNumber++;
		}

		n[colName[name] - 1]++;
		sum[colName[name] - 1] += value;
	}

	for (int i = 0; i < np; i++)
		mean[i] = sum[i] / n[i];

	for (int i = 0; i < totalRows; i++) {
		QString name = columns[0]->textAt(i);
		double value = columns[1]->valueAt(i);
		stdDev[colName[name] - 1] += gsl_pow_2((value - mean[colName[name] - 1]));
	}

	for (int i = 0; i < np; i++) {
		if (n[i] > 1)
			stdDev[i] = stdDev[i] / (n[i] - 1);
		stdDev[i] = sqrt(stdDev[i]);
	}

	columns[0]->setColumnMode(originalMode);
	// The following check appears to be legacy.
	if (columns[0]->isNumeric()) { }
	return GeneralTest::NoError;
}

QString GeneralTest::buildHtmlTable(int row, int column, QVariant* data) {
	QString table;
	table = QLatin1String("<style type='text/css'>") + QLatin1String("table {border-collapse: collapse; margin: auto; width: 80%; font-size: 18px;}")
		+ QLatin1String("th, td {border: 1px solid black; padding: 12px; text-align: center; font-size: 16px;}")
		+ QLatin1String("th {background-color: #f2f2f2; font-size: 18px;}") + QLatin1String("</style>") + QLatin1String("<div style='text-align: center;'>")
		+ QLatin1String("<table>");
	table += QLatin1String("<tr>");
	// Add column headers.
	for (int j = 0; j < column; ++j)
		table += QLatin1String("<th>") + data[j].toString() + QLatin1String("</th>");
	table += QLatin1String("</tr>");
	// Add data rows.
	for (int i = 1; i < row; ++i) {
		table += QLatin1String("<tr>");
		for (int j = 0; j < column; ++j)
			table += QLatin1String("<td>") + data[i * column + j].toString() + QLatin1String("</td>");
		table += QLatin1String("</tr>");
	}
	table += QLatin1String("</table>");
	table += QLatin1String("</table></div>");
	return table;
}

QString GeneralTest::buildHtmlTableFromCells(const QList<HtmlText*>& cells) {
	int cellCount = cells.size();
	if (cellCount == 0)
		return QString();

	// HTML tooltip markers.
	const QString startToolTip = QLatin1String("[tooltip]");
	const QString endToolTip = QLatin1String("[/tooltip]");
	const QString startData = QLatin1String("[data]");
	const QString endData = QLatin1String("[/data]");
	const QString startTip = QLatin1String("[tip]");
	const QString endTip = QLatin1String("[/tip]");

	QString table;
	table = QLatin1String(
		"<style type='text/css'>"
		".tg  {border-collapse:collapse;border: 1px solid black;}"
		".tg td{font-family:Arial, sans-serif;font-size:14px;padding:10px 5px;border: 1px solid "
		"black;overflow:hidden;word-break:normal;color:#333;background-color:#fff;}"
		".tg th{font-family:Arial, sans-serif;font-size:14px;font-weight:bold;padding:10px 5px;border: 1px solid "
		"black;overflow:hidden;word-break:normal;color:#333;background-color:#f0f0f0;}"
		"</style>");
	table += QLatin1String("<table class='tg'>");

	table += QLatin1String("<tr>");
	int prevLevel = 0;
	for (int i = 0; i < cellCount; i++) {
		HtmlText* currentCell = cells[i];
		if (currentCell->level != prevLevel) {
			table += QLatin1String("</tr><tr>");
			prevLevel = currentCell->level;
		}
		QString cellStartTag = currentCell->isHeader ? QLatin1String("<th ") : QLatin1String("<td ");
		QString cellEndTag = currentCell->isHeader ? QLatin1String("</th>") : QLatin1String("</td>");
		QString cellContent = i18n("%1", currentCell->data);
		if (!currentCell->tooltip.isEmpty())
			cellContent = startToolTip + startData + cellContent + endData + startTip + i18n("%1", currentCell->tooltip) + endTip + endToolTip;
		table += cellStartTag + QLatin1String("rowspan=") + QString::number(currentCell->rowSpanCount) + QLatin1String(" colspan=")
			+ QString::number(currentCell->columnSpanCount) + QLatin1String(">") + cellContent + cellEndTag;
	}
	table += QLatin1String("</tr></table>");
	return table;
}

QString GeneralTest::formatHtmlLine(const QString& msg, const QString& color) {
	return QLatin1String("<p style='color:") + color + QLatin1String(";'>") + i18n("%1", msg) + QLatin1String("</p>");
}

void GeneralTest::displayLine(const int& index, const QString& msg, const QString& color) {
	if (index < 0 || index >= RESULT_LINES_COUNT)
		return;

	QString formattedMsg = QLatin1String("<p style='color:") + color + QLatin1String("; font-size:14px;'>") + i18n("%1", msg) + QLatin1String("</p>");
	m_resultLabels[index]->setText(formattedMsg);
}

void GeneralTest::displayTooltip(const int& index, const QString& msg) {
	if (index < 0 || index >= RESULT_LINES_COUNT)
		return;

	m_resultLabels[index]->setToolTip(i18n("%1", msg));
}

void GeneralTest::displayError(const QString& errorMsg) {
	displayLine(0, errorMsg, QLatin1String("red"));
}

/********************************************************************************************************************
 *                        Virtual Functions Implementations
 ********************************************************************************************************************/

void GeneralTest::save(QXmlStreamWriter* writer) const {
	writer->writeStartElement(QLatin1String("GeneralTest"));
	writeBasicAttributes(writer);
	writeCommentElement(writer);
	writer->writeEndElement();
}

bool GeneralTest::load(XmlStreamReader* reader, bool preview) {
	Q_UNUSED(preview)
	if (!readBasicAttributes(reader))
		return false;
	return !reader->hasError();
}

void GeneralTest::clearInputStats() {
	QList<QStandardItem*> horizontalHeader = m_inputStatsTableModel->takeRow(0);
	QList<QStandardItem*> verticalHeader = m_inputStatsTableModel->takeColumn(0);

	m_inputStatsTableModel->clear();
	m_inputStatsTableModel->appendRow(horizontalHeader);

	verticalHeader.push_front(m_inputStatsTableModel->takeColumn(0)[0]);
	m_inputStatsTableModel->insertColumn(0, verticalHeader);
}

bool GeneralTest::exportView() const {
	return true;
}

bool GeneralTest::printView() {
	return true;
}

bool GeneralTest::printPreview() const {
	return true;
}

QMenu* GeneralTest::createContextMenu() {
	QMenu* menu = AbstractPart::createContextMenu();
	// Additional menu customization can be done here.
	return menu;
}

QWidget* GeneralTest::view() const {
	if (!m_partView) {
		m_view = new GeneralTestView(const_cast<GeneralTest*>(this));
		m_partView = m_view;
	}
	return m_partView;
}
