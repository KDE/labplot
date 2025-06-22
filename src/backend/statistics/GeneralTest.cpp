/*
	File                 : GeneralTest.cpp
	Project              : LabPlot
	Description          : Base class for statistical tests
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2019 Devanshu Agarwal (agarwaldevanshu8@gmail.com)
	SPDX-FileCopyrightText: 2025 Kuntal Bar (barkuntal6@gmail.com)
	SPDX-FileCopyrightText: 2025 Alexander Semke <alexander.semke@web.de>

	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "GeneralTest.h"
#include "backend/core/column/Column.h"
#include "backend/lib/XmlStreamReader.h"
#include "backend/lib/macros.h"
#include "backend/spreadsheet/Spreadsheet.h"
#include "backend/statistics/TableModel.h"
#include "frontend/statistics/GeneralTestView.h"

#include <QLabel>
#include <QMenu>
#include <QPrintDialog>
#include <QPrintPreviewDialog>
#include <QPrinter>
#include <QVBoxLayout>

#include <KLocalizedString>

#include <gsl/gsl_math.h>
#include <math.h>
extern "C" {
#include "backend/nsl/nsl_statistical_test.h"
}

GeneralTest::GeneralTest(const QString& name, const AspectType& type)
	: AbstractPart(name, type) {
}

GeneralTest::~GeneralTest() {
	// No additional cleanup required.
}

QString GeneralTest::resultHtml() const {
	return m_result;
}

void GeneralTest::setColumns(const QVector<Column*>& cols) {
	m_columns = cols;
}

/********************************************************************************************************************
 *                        Protected Helper Functions Implementations (Renamed)
 ********************************************************************************************************************/
void GeneralTest::addResultTitle(const QString& text) {
	m_result += QStringLiteral("<h1>") + text + QStringLiteral("</h1>");
}

void GeneralTest::addResultSection(const QString& text) {
	m_result += QStringLiteral("<h2>") + text + QStringLiteral("</h2>");
}

void GeneralTest::addResultLine(const QString& name, const QString& value) {
	m_result += QStringLiteral("<b>") + name + QStringLiteral("</b>: ") + value + QStringLiteral("<br>");
}

void GeneralTest::addResultLine(const QString& name, double value) {
	if (!std::isnan(value))
		addResultLine(name, QLocale().toString(value));
	else
		addResultLine(name, QStringLiteral("-"));
}

void GeneralTest::addResultLine(const QString& name) {
	m_result += name;
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

/********************************************************************************************************************
 *                        Virtual Functions Implementations
 ********************************************************************************************************************/
bool GeneralTest::exportView() const {
	return true;
}

bool GeneralTest::printView() {
	#ifndef SDK
	QPrinter printer;
	auto* dlg = new QPrintDialog(&printer, m_view);
	dlg->setWindowTitle(i18nc("@title:window", "Statistical Test"));
	bool ret;
	if ((ret = (dlg->exec() == QDialog::Accepted)))
		m_view->print(&printer);

	delete dlg;
	return ret;
#else
	return false;
#endif
}

bool GeneralTest::printPreview() const {
#ifndef SDK
	auto* dlg = new QPrintPreviewDialog(m_view);
	connect(dlg, &QPrintPreviewDialog::paintRequested, m_view, &GeneralTestView::print);
	return dlg->exec();
#else
	return false;
#endif
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

// ##############################################################################
// ##################  Serialization/Deserialization  ###########################
// ##############################################################################

//! Save as XML
void GeneralTest::save(QXmlStreamWriter* writer) const {
	writer->writeStartElement(QLatin1String("GeneralTest"));
	writeBasicAttributes(writer);
	writeCommentElement(writer);
	writer->writeEndElement();
}

//! Load from XML
bool GeneralTest::load(XmlStreamReader* reader, bool preview) {
	Q_UNUSED(preview)
	if (!readBasicAttributes(reader))
		return false;
	return !reader->hasError();
}
