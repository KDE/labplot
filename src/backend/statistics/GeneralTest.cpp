/***************************************************************************
	File                 : GeneralTest.cpp
	Project              : LabPlot
	Description          : Doing Hypothesis-Test on data provided
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

#include "GeneralTest.h"
#include "kdefrontend/statistics/GeneralTestView.h"
#include "backend/spreadsheet/Spreadsheet.h"
#include "backend/core/column/Column.h"
#include "backend/lib/macros.h"
#include "backend/lib/XmlStreamReader.h"
#include "backend/statistics/MyTableModel.h"

#include <QLabel>
#include <QVBoxLayout>
#include <QStandardItemModel>

#include <KLocalizedString>

#include <gsl/gsl_math.h>
#include <math.h>
extern "C" {
#include "backend/nsl/nsl_stats.h"
}

GeneralTest::GeneralTest(const QString& name, const AspectType& type) : AbstractPart(name, type),
	m_summaryLayout(new QVBoxLayout()),
	m_inputStatsTableModel(new MyTableModel()) {

	for (int i = 0; i < RESULTLINESCOUNT; i++) {
		m_resultLine[i] = new QLabel();
		m_summaryLayout->addWidget(m_resultLine[i]);
	}
}

GeneralTest::~GeneralTest() {
}

void GeneralTest::setDataSourceSpreadsheet(Spreadsheet* spreadsheet) {
	m_dataSourceSpreadsheet = spreadsheet;
	for (auto* col : m_dataSourceSpreadsheet->children<Column>())
		m_allColumns << col->name();
}

QString GeneralTest::testName() {
	return m_currTestName;
}

QString GeneralTest::statsTable() {
	return m_statsTable;
}

QVBoxLayout* GeneralTest::summaryLayout() {
	return m_summaryLayout;
}

QAbstractItemModel* GeneralTest::inputStatsTableModel() {
	return m_inputStatsTableModel;
}

void GeneralTest::setColumns(QStringList cols) {
	m_columns.clear();
	Column* column = new Column(QLatin1String("column"));
	for (QString col : cols) {
		if (!cols.isEmpty()) {
			column = m_dataSourceSpreadsheet->column(col);
			m_columns.append(column);
		}
	}
	delete column;
}

void GeneralTest::setColumns(const QVector<Column*> &cols) {
	m_columns = cols;
}

/********************************************************************************************************************
*                                 Protected functions implementations [Helper Functions]
********************************************************************************************************************/
int GeneralTest::testType(int test) {
	return test & 0x0F;
}

int GeneralTest::testSubtype(int test) {
	return test & 0xF0;
}

//TODO: we should implement or use a general round method
QString GeneralTest::round(QVariant number, int precision) {
	if (number.userType() == QMetaType::Double || number.userType() == QMetaType::Float) {
		double multiplierPrecision = gsl_pow_int(10, precision);
		int tempNum = int(number.toDouble()*multiplierPrecision*10);

		if (tempNum % 10 < 5)
			return QString::number((tempNum/10) / multiplierPrecision);
		else
			return QString::number((tempNum/10 + 1) / multiplierPrecision);
	}
	return i18n("%1", number.toString());
}

// TODO: put into Column
double GeneralTest::findSum(const Column *column, int N) {
	if (!column->isNumeric())
		return 0;

	if (N < 0)
		N = column->statistics().size;

	double sum = 0;
	for (int i = 0; i < N; i++)
		sum += column->valueAt(i);
	return sum;
}

// TODO: put into Column
double GeneralTest::findSumSq(const Column *column, int N) {
	if (!column->isNumeric())
		return 0;

	if (N < 0)
		N = column->statistics().size;

	double sumSq = 0;
	for (int i = 0; i < N; i++)
		sumSq += gsl_pow_2(column->valueAt(i));
	return sumSq;
}

GeneralTest::GeneralErrorType GeneralTest::findStats(const Column* column, int& count, double& sum, double& mean, double& std) {
	/*
	count = findCount(column);
	sum = findSum(column, count);
	mean = findMean(column, count);
	std = findStd(column, count, mean);
*/
	const auto& statistics = column->statistics();
	count = statistics.size;
	if (count < 1)
		return GeneralTest::ErrorEmptyColumn;

	mean = statistics.arithmeticMean;
	sum = findSum(column, count);
	std = statistics.standardDeviation;
	return GeneralTest::NoError;
}

GeneralTest::GeneralErrorType GeneralTest::findStatsPaired(const Column* column1, const Column* column2, int& count, double& sum, double& mean, double& std) {
	sum = 0;
	mean = 0;
	std = 0;

	int count1 = column1->rowCount();
	int count2 = column2->rowCount();

	count = qMin(count1, count2);
	double HtmlCell1, HtmlCell2;
	for (int i = 0; i < count; i++) {
		HtmlCell1 = column1->valueAt(i);
		HtmlCell2 = column2->valueAt(i);

		if (std::isnan(HtmlCell1) || std::isnan(HtmlCell2)) {
			if (std::isnan(HtmlCell1) && std::isnan(HtmlCell2))
				count = i;
			else
				return GeneralTest::ErrorUnqualSize;
			break;
		}

		sum += HtmlCell1 - HtmlCell2;
	}

	if (count < 1)
		return GeneralTest::ErrorEmptyColumn;

	mean = sum / count;

	double row;
	for (int i = 0; i < count; i++) {
		HtmlCell1 = column1->valueAt(i);
		HtmlCell2 = column2->valueAt(i);
		row = HtmlCell1 - HtmlCell2;
		std += gsl_pow_2( (row - mean));
	}

	if (count > 1)
		std = std / (count-1);

	std = sqrt(std);
	return GeneralTest::NoError;
}

void GeneralTest::countPartitions(Column* column, int& np, int& totalRows) {
	totalRows = column->rowCount();
	np = 0;
	QString HtmlCellValue;
	QMap<QString, bool> discoveredCategoricalVar;

	AbstractColumn::ColumnMode originalColMode = column->columnMode();
	column->setColumnMode(AbstractColumn::ColumnMode::Text);

	for (int i = 0; i < totalRows; i++) {
		HtmlCellValue = column->textAt(i);

		if (HtmlCellValue.isEmpty()) {
			totalRows = i;
			break;
		}

		if (discoveredCategoricalVar[HtmlCellValue])
			continue;

		discoveredCategoricalVar[HtmlCellValue] = true;
		np++;
	}
	column->setColumnMode(originalColMode);
}

GeneralTest::GeneralErrorType GeneralTest::findStatsCategorical(Column* column1, Column* column2, int n[], double sum[],
																double mean[], double std[], QMap<QString, int>& colName,
																const int& np, const int& totalRows) {
	Column* columns[] = {column1, column2};

	for (int i = 0; i < np; i++) {
		n[i] = 0;
		sum[i] = 0;
		mean[i] = 0;
		std[i] = 0;
	}

	AbstractColumn::ColumnMode originalColMode = columns[0]->columnMode();
	columns[0]->setColumnMode(AbstractColumn::ColumnMode::Text);

	int partitionNumber = 1;
	for (int i = 0; i < totalRows; i++) {
		QString name = columns[0]->textAt(i);
		double value = columns[1]->valueAt(i);

		if (std::isnan(value)) {
			columns[0]->setColumnMode(originalColMode);
			return GeneralTest::ErrorUnqualSize;
		}

		if (colName[name] == 0) {
			colName[name] = partitionNumber;
			partitionNumber++;
		}

		n[colName[name]-1]++;
		sum[colName[name]-1] += value;
	}

	for (int i = 0; i < np; i++)
		mean[i] = sum[i] / n[i];

	for (int i = 0; i < totalRows; i++) {
		QString name = columns[0]->textAt(i);
		double value = columns[1]->valueAt(i);

		std[colName[name]-1] += gsl_pow_2( (value - mean[colName[name]-1]));
	}

	for (int i = 0; i < np; i++) {
		if (n[i] > 1)
			std[i] = std[i] / (n[i] - 1);
		std[i] = sqrt(std[i]);
	}

	columns[0]->setColumnMode(originalColMode);
	if (columns[0]->isNumeric()) {

	}

	return GeneralTest::NoError;
}

QString GeneralTest::getHtmlTable(int row, int column, QVariant* rowMajor) {
	if (row < 1 || column < 1)
		return QString();

	QString table;
	table = QLatin1String("<style type=text/css>"
			".tg  {border-collapse:collapse;border-spacing:0;border:none;border-color:#ccc;}"
			".tg td{font-family:Arial, sans-serif;font-size:14px;padding:10px 5px;border-style:solid;border-width:0px;overflow:hidden;word-break:normal;border-color:#ccc;color:#333;background-color:#fff;}"
			".tg th{font-family:Arial, sans-serif;font-size:14px;font-weight:normal;padding:10px 5px;border-style:solid;border-width:0px;overflow:hidden;word-break:normal;border-color:#ccc;color:#333;background-color:#f0f0f0;}"
			".tg .tg-0pky{border-color:inherit;text-align:left;vertical-align:top}"
			".tg .tg-btxf{background-color:#f9f9f9;border-color:inherit;text-align:left;vertical-align:top}"
			"</style>"
			"<table class=tg>"
			"  <tr>");

	QString bg = QLatin1String("tg-0pky");
	bool pky = true;

	QString element;
	table += QLatin1String("  <tr>");
	for (int j = 0; j < column; j++) {
		element = rowMajor[j].toString();
		table += QLatin1String("    <th class=") + bg + QLatin1String("><b>") + i18n("%1", element) + QLatin1String("</b></th>");
	}
	table += QLatin1String("  </tr>");

	if (pky)
		bg = QLatin1String("tg-0pky");
	else
		bg = QLatin1String("tg-btxf");
	pky = !pky;

	for (int i = 1; i < row; i++) {
		table += QLatin1String("  <tr>");

		QString element = round(rowMajor[i*column]);
		table += QLatin1String("    <td class=") + bg + QLatin1String("><b>") + i18n("%1", element) + QLatin1String("</b></td>");
		for (int j = 1; j < column; j++) {
			element = round(rowMajor[i*column+j]);
			table += QLatin1String("    <td class=") + bg + QLatin1String(">") + i18n("%1", element) + QLatin1String("</td>");
		}

		table += QLatin1String("  </tr>");
		if (pky)
			bg = QLatin1String("tg-0pky");
		else
			bg = QLatin1String("tg-btxf");
		pky = !pky;
	}
	table +=  QLatin1String("</table>");

	return table;
}

QString GeneralTest::getHtmlTable3(const QList<GeneralTest::HtmlCell*>& rowMajor) {
	int rowMajorSize = rowMajor.size();

	if (rowMajorSize == 0)
		return QString();

	QString startToolTip = QLatin1String("[tooltip]");
	QString endToolTip = QLatin1String("[/tooltip]");
	QString startData = QLatin1String("[data]");
	QString endData = QLatin1String("[/data]");
	QString startTip = QLatin1String("[tip]");
	QString endTip = QLatin1String("[/tip]");

	QString table;
	table = QLatin1String("<style type=text/css>"
			".tg  {border-collapse:collapse;border: 1px solid black;}"
			".tg td{font-family:Arial, sans-serif;font-size:14px;padding:10px 5px;border: 1px solid black;overflow:hidden;word-break:normal;color:#333;background-color:#fff;}"
			".tg th{font-family:Arial, sans-serif;font-size:14px;font-weight:bold;padding:10px 5px;border: 1px solid black;overflow:hidden;word-break:normal;color:#333;background-color:#f0f0f0;}"
			"</style>");

	table += QLatin1String("<table class=tg>");

	table += QLatin1String("  <tr>");
	int prevLevel = 0;
	for (int i = 0; i < rowMajorSize; i++) {
		HtmlCell* currHtmlCell = rowMajor[i];
		if (currHtmlCell->level != prevLevel) {
			table += QLatin1String("  </tr>");
			table += QLatin1String("  <tr>");
			prevLevel = currHtmlCell->level;
		}
		QString HtmlCellStartTag = QLatin1String("<td ");
		QString HtmlCellEndTag = QLatin1String("</td>");

		if (currHtmlCell->isHeader) {
			HtmlCellStartTag = QLatin1String("<th ");
			HtmlCellEndTag = QLatin1String("</th>");
		}

		QString HtmlCellData = i18n("%1", currHtmlCell->data);

		if (!currHtmlCell->tooltip.isEmpty())
			HtmlCellData = startToolTip+
						startData+HtmlCellData+endData+
						startTip+i18n("%1", currHtmlCell->tooltip)+endTip+
					   endToolTip;

		table += HtmlCellStartTag +
				QLatin1String("rowspan=") + QString::number(currHtmlCell->rowSpanCount) +
				QLatin1String(" ") +
				QLatin1String("colspan=") + QString::number(currHtmlCell->columnSpanCount) +
				QLatin1String(">") +
				HtmlCellData +
				HtmlCellEndTag;
	}
	table += QLatin1String("  <tr>");
	table += QLatin1String("</table>");
	return table;
}

QString GeneralTest::getLine(const QString& msg, const QString& color) {
	return QLatin1String("<p style=color:") + color + QLatin1String(";>") + i18n("%1", msg) + QLatin1String("</p>");
}

void GeneralTest::printLine(const int& index, const QString& msg, const QString& color) {
	if (index < 0 || index >= 10)
		return;

	m_resultLine[index]->setText(getLine(msg, color));
	return;
}

void GeneralTest::printTooltip(const int &index, const QString &msg) {
	if (index < 0 || index >= 10)
		return;

	m_resultLine[index]->setToolTip(i18n("%1", msg));
}

void GeneralTest::printError(const QString& errorMsg) {
	printLine(0, errorMsg, QLatin1String("red"));
}

/********************************************************************************************************************
*                                          virtual functions implementations
********************************************************************************************************************/
/*!
  Saves as XML.
 */
void GeneralTest::save(QXmlStreamWriter* writer) const {
	writer->writeStartElement(QLatin1String("GeneralTest"));
	writeBasicAttributes(writer);
	writeCommentElement(writer);
	writer->writeEndElement();
}

/*!
  Loads from XML.
*/
bool GeneralTest::load(XmlStreamReader* reader, bool preview) {
	Q_UNUSED(preview)
	if (!readBasicAttributes(reader))
		return false;

	return !reader->hasError();
}

void GeneralTest::clearInputStatsTable() {
	QList<QStandardItem *> horizontalHeader = m_inputStatsTableModel->takeRow(0);
	QList<QStandardItem *> verticalHeader = m_inputStatsTableModel->takeColumn(0);

	m_inputStatsTableModel->clear();
	m_inputStatsTableModel->appendRow(horizontalHeader);

	verticalHeader.push_front(m_inputStatsTableModel->takeColumn(0)[0]);
	m_inputStatsTableModel->insertColumn(0, verticalHeader);
}

Spreadsheet *GeneralTest::dataSourceSpreadsheet() const {
	return m_dataSourceSpreadsheet;
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

/*!
  Returns a new context menu. The caller takes ownership of the menu.
*/
QMenu* GeneralTest::createContextMenu() {
	QMenu* menu = AbstractPart::createContextMenu();
	//    Q_ASSERT(menu);
	//    emit requestProjectContextMenu(menu);
	return menu;
}

QWidget* GeneralTest::view() const {
	if (!m_partView) {
		m_view = new GeneralTestView(const_cast<GeneralTest*>(this));
		m_partView = m_view;
	}
	return m_partView;
}
