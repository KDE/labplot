/*
	File                 : StatisticsColumnWidget.cpp
	Project              : LabPlot
	Description          : Widget showing statistics for column values
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2021 Alexander Semke <alexander.semke@web.de>
    SPDX-FileCopyrightText: 2022 Stefan Gerlach <stefan.gerlach@uni.kn>

    SPDX-License-Identifier: GPL-2.0-or-later
*/


#include "StatisticsColumnWidget.h"
#include "backend/core/column/Column.h"
#include "backend/core/Project.h"
#include "backend/lib/macros.h"
#include "backend/worksheet/Worksheet.h"
#include "backend/worksheet/TextLabel.h"
#include "backend/worksheet/plots/cartesian/Axis.h"
#include "backend/worksheet/plots/cartesian/BoxPlot.h"
#include "backend/worksheet/plots/cartesian/CartesianPlot.h"
#include "backend/worksheet/plots/cartesian/Histogram.h"
#include "backend/worksheet/plots/PlotArea.h"
#include "backend/worksheet/plots/cartesian/Symbol.h"
#include "backend/worksheet/plots/cartesian/XYCurve.h"
#include "backend/lib/macros.h"

#include <QTabWidget>
#include <QTextEdit>
#include <QTimer>
#include <QVBoxLayout>

#include <KLocalizedString>

#include <cmath>
#include <algorithm> //for min_element and max_element

extern "C" {
#include <gsl/gsl_cdf.h>
#include <gsl/gsl_math.h>
#include <gsl/gsl_statistics.h>
#include "backend/nsl/nsl_kde.h"
}

StatisticsColumnWidget::StatisticsColumnWidget(const Column* column, QWidget* parent) : QWidget(parent),
	m_column(column),
	m_project(new Project),
	m_tabWidget(new QTabWidget)
{

	auto* layout = new QVBoxLayout;
	layout->addWidget(m_tabWidget);
	setLayout(layout);

	const QString htmlColor = (palette().color(QPalette::Base).lightness() < 128) ? QLatin1String("#5f5f5f") : QLatin1String("#D1D1D1");
	m_htmlText = QString("<table border=0 width=100%>"
	                     "<tr>"
	                     "<td colspan=2 align=center bgcolor=" + htmlColor + "><b><big>"
	                     + i18n("Location Measures")+
	                     "</big><b></td>"
	                     "</tr>"
// 	                     "<tr></tr>"
	                     "<tr>"
	                     "<td width=70%><b>"
	                     + i18n("Count")+
	                     "<b></td>"
	                     "<td>%1</td>"
	                     "</tr>"
						 "<tr>"
	                     "<td><b>"
	                     + i18n("Minimum")+
	                     "<b></td>"
	                     "<td>%2</td>"
	                     "</tr>"
	                     "<tr>"
	                     "<td><b>"
	                     + i18n("Maximum")+
	                     "<b></td>"
	                     "<td>%3</td>"
	                     "</tr>"
	                     "<tr>"
	                     "<td><b>"
	                     + i18n("Arithmetic mean")+
	                     "<b></td>"
	                     "<td>%4</td>"
	                     "</tr>"
	                     "<tr>"
	                     "<td><b>"
	                     + i18n("Geometric mean")+
	                     "<b></td>"
	                     "<td>%5</td>"
	                     "</tr>"
	                     "<tr>"
	                     "<td><b>"
	                     + i18n("Harmonic mean")+
	                     "<b></td>"
	                     "<td>%6</td>"
	                     "</tr>"
	                     "<tr>"
	                     "<td><b>"
	                     + i18n("Contraharmonic mean")+
	                     "<b></td>"
	                     "<td>%7</td>"
	                     "</tr>"
						"<tr>"
	                     "<td><b>"
	                     + i18n("Mode")+
	                     "<b></td>"
	                     "<td>%8</td>"
	                     "</tr>"
						 "<tr>"
	                     "<td><b>"
	                     + i18n("First Quartile")+
	                     "<b></td>"
	                     "<td>%9</td>"
	                     "</tr>"
						 "<tr>"
	                     "<td><b>"
	                     + i18n("Median")+
	                     "<b></td>"
	                     "<td>%10</td>"
	                     "</tr>"
	                     "<tr>"
	                     "<td><b>"
	                     + i18n("Third Quartile")+
	                     "<b></td>"
	                     "<td>%11</td>"
	                     "</tr>"
	                     "<tr>"
	                     "<td><b>"
	                     + i18n("Trimean")+
	                     "<b></td>"
	                     "<td>%12</td>"
	                     "</tr>"
	                     "<tr></tr>"
	                     "<tr>"
	                     "<td colspan=2 align=center bgcolor=" + htmlColor + "><b><big>"
	                     + i18n("Dispersion Measures")+
	                     "</big></b></td>"
	                     "</tr>"
// 	                     "<tr></tr>"
	                     "<tr>"
	                     "<td><b>"
	                     + i18n("Variance")+
	                     "<b></td>"
						 "<td>%13</td>"
	                     "</tr>"
	                     "<tr>"
	                     "<td><b>"
	                     + i18n("Standard deviation")+
	                     "<b></td>"
						 "<td>%14</td>"
	                     "</tr>"
	                     "<tr>"
	                     "<td><b>"
	                     + i18n("Mean absolute deviation around mean")+
	                     "<b></td>"
						 "<td>%15</td>"
	                     "</tr>"
	                     "<tr>"
	                     "<td><b>"
	                     + i18n("Mean absolute deviation around median")+
	                     "<b></td>"
						 "<td>%16</td>"
	                     "</tr>"
	                     "<tr>"
	                     "<td><b>"
	                     + i18n("Median absolute deviation")+
	                     "<b></td>"
						 "<td>%17</td>"
	                     "</tr>"
						 "<tr>"
						  "<td><b>"
						  + i18n("Interquartile Range")+
						  "<b></td>"
						  "<td>%18</td>"
						  "</tr>"
	                     "<tr></tr>"
	                     "<tr>"
	                     "<td colspan=2 align=center bgcolor=" + htmlColor + "><b><big>"
	                     + i18n("Shape Measures")+
	                     "</big></b></td>"
	                     "</tr>"
// 	                     "<tr></tr>"
	                     "<tr>"
	                     "<td><b>"
	                     + i18n("Skewness")+
	                     "<b></td>"
	                     "<td>%19</td>"
	                     "</tr>"
	                     "<tr>"
	                     "<td><b>"
	                     + i18n("Kurtosis")+
	                     "<b></td>"
	                     "<td>%20</td>"
	                     "</tr>"
	                     "<tr>"
	                     "<td><b>"
	                     + i18n("Entropy")+
	                     "<b></td>"
	                     "<td>%21</td>"
	                     "</tr>"
	                     "</table>");

	//create tab widgets for every column and show the initial text with the placeholders
	m_teOverview = new QTextEdit(this);
	m_teOverview->setReadOnly(true);
	m_teOverview->setHtml(m_htmlText.arg(QLatin1String("-"), QLatin1String("-"), QLatin1String("-"), QLatin1String("-"),
									QLatin1String("-"), QLatin1String("-"), QLatin1String("-"), QLatin1String("-"), QLatin1String("-")).
									arg(QLatin1String("-"), QLatin1String("-"), QLatin1String("-"), QLatin1String("-"), QLatin1String("-"),
										QLatin1String("-"), QLatin1String("-"), QLatin1String("-"), QLatin1String("-")).
									arg(QLatin1String("-"), QLatin1String("-"), QLatin1String("-")));

	m_tabWidget->addTab(m_teOverview, i18n("Overview"));
	m_tabWidget->addTab(&m_histogramWidget, i18n("Histogram"));
	m_tabWidget->addTab(&m_kdePlotWidget, i18n("KDE Plot"));
	m_tabWidget->addTab(&m_qqPlotWidget, i18n("Normal Q-Q Plot"));
	m_tabWidget->addTab(&m_boxPlotWidget, i18n("Box Plot"));

	connect(m_tabWidget, &QTabWidget::currentChanged, this, &StatisticsColumnWidget::currentTabChanged);
}

StatisticsColumnWidget::~StatisticsColumnWidget() {
	disconnect(m_tabWidget, nullptr, this, nullptr); //don't react on currentChanged signal
	delete m_project;
}

void StatisticsColumnWidget::setCurrentTab(int index) {
	if (index == m_tabWidget->currentIndex())
		currentTabChanged(index); //manually call the slot so we get the data shown
	else
		m_tabWidget->setCurrentIndex(index);
}

void StatisticsColumnWidget::currentTabChanged(int index) {
	if (index == 0 && !m_overviewInitialized)
		showOverview();
	else if (index == 1 && !m_histogramInitialized)
		showHistogram();
	else if (index == 2 && !m_kdePlotInitialized)
		showKDEPlot();
	else if (index == 3 && !m_qqPlotInitialized)
		showQQPlot();
	else if (index == 4 && !m_boxPlotInitialized)
		showBoxPlot();

	Q_EMIT tabChanged(index);
}

void StatisticsColumnWidget::showOverview() {
	WAIT_CURSOR;
	const Column::ColumnStatistics& statistics = m_column->statistics();

	m_teOverview->setHtml(m_htmlText.arg(QString::number(statistics.size),
									isNanValue(statistics.minimum == INFINITY ? NAN : statistics.minimum),
									isNanValue(statistics.maximum == -INFINITY ? NAN : statistics.maximum),
									isNanValue(statistics.arithmeticMean),
									isNanValue(statistics.geometricMean),
									isNanValue(statistics.harmonicMean),
									isNanValue(statistics.contraharmonicMean),
									modeValue(m_column, statistics.mode),
									isNanValue(statistics.firstQuartile)).
						arg(isNanValue(statistics.median),
							isNanValue(statistics.thirdQuartile),
							isNanValue(statistics.trimean),
							isNanValue(statistics.variance),
							isNanValue(statistics.standardDeviation),
							isNanValue(statistics.meanDeviation),
							isNanValue(statistics.meanDeviationAroundMedian),
							isNanValue(statistics.medianDeviation),
							isNanValue(statistics.iqr)).
						arg(isNanValue(statistics.skewness),
							isNanValue(statistics.kurtosis),
							isNanValue(statistics.entropy)));
	RESET_CURSOR;

	m_overviewInitialized = true;
}

void StatisticsColumnWidget::showHistogram() {
	WAIT_CURSOR;

	//add plot
	auto* plot = addPlot(&m_histogramWidget);

	auto axes = plot->children<Axis>();
	for (auto* axis : qAsConst(axes)) {
		auto wrapper = axis->title()->text();
		QTextEdit te(wrapper.text);
		te.selectAll();
		if (axis->orientation() == Axis::Orientation::Horizontal) {
			te.setText(m_column->name());
			axis->setMajorGridPen(QPen(Qt::NoPen));
		} else
			te.setText(i18n("Frequency"));

		axis->title()->setText(te.toHtml());

		axis->setMinorTicksDirection(Axis::noTicks);
	}
	QApplication::processEvents(QEventLoop::AllEvents, 100);

	auto* histogram = new Histogram(QString());
	plot->addChild(histogram);
	histogram->setDataColumn(m_column);

	plot->retransform();
	m_histogramInitialized = true;
	RESET_CURSOR;
}

void StatisticsColumnWidget::showKDEPlot() {
	WAIT_CURSOR;

	//add plot
	auto* plot = addPlot(&m_kdePlotWidget);

	//set the axes lables
	auto axes = plot->children<Axis>();
	for (auto* axis : qAsConst(axes)) {
		auto wrapper = axis->title()->text();
		QTextEdit te(wrapper.text);
		te.selectAll();
		if (axis->orientation() == Axis::Orientation::Horizontal)
			te.setText(m_column->name());
		else
			te.setText(i18n("Density"));

		axis->title()->setText(te.toHtml());

		axis->setMinorTicksDirection(Axis::noTicks);
	}

	QApplication::processEvents(QEventLoop::AllEvents, 100);

	//add normalized histogram
	auto* histogram = new Histogram(QString());
	plot->addChild(histogram);
	histogram->setNormalization(Histogram::ProbabilityDensity);
	histogram->setDataColumn(m_column);

	//copy the non-nan and not masked values
	QVector<double> data;
	copyValidData(data);

	//calculate 200 points to plot
	int count = 200;
	QVector<double> xData;
	QVector<double> yData;
	xData.resize(count);
	yData.resize(count);
	double min = *std::min_element(data.constBegin(), data.constEnd());
	double max = *std::max_element(data.constBegin(), data.constEnd());
	double step = (max - min)/count;
	int n = data.count();
	double h = qMax(nsl_kde_normal_dist_bandwith(data.data(), n), 1e-6);
	for (int i = 0; i < count; ++i) {
		double x = min + i*step;
		xData[i] = x;
		yData[i] = nsl_kde(data.data(), x, h, n);
	}

	auto* xColumn = new Column("x");
	xColumn->replaceValues(0, xData);

	auto* yColumn = new Column("y");
	yColumn->replaceValues(0, yData);

	//add KDE curve
	XYCurve* curve = new XYCurve("");
	curve->suppressRetransform(false);
	plot->addChild(curve);
	QPen pen = curve->linePen();
	pen.setStyle(Qt::SolidLine);
	curve->setLinePen(pen);
	curve->symbol()->setStyle(Symbol::Style::NoSymbols);
	curve->setFillingPosition(XYCurve::FillingPosition::NoFilling);
	curve->setXColumn(xColumn);
	curve->setYColumn(yColumn);

	curve->suppressRetransform(false);
	plot->retransform();
	m_kdePlotInitialized = true;
	RESET_CURSOR;
}

void StatisticsColumnWidget::showQQPlot() {
	WAIT_CURSOR;

	//add plot
	auto* plot = addPlot(&m_qqPlotWidget);

	auto axes = plot->children<Axis>();
	for (auto* axis : qAsConst(axes)) {
		auto wrapper = axis->title()->text();
		QTextEdit te(wrapper.text);
		te.selectAll();
		if (axis->orientation() == Axis::Orientation::Horizontal)
			te.setText(i18n("Theoretical Quantiles"));
		else
			te.setText(i18n("Sample Quantiles"));

		axis->title()->setText(te.toHtml());

		axis->setMinorTicksDirection(Axis::noTicks);
	}
	QApplication::processEvents(QEventLoop::AllEvents, 100);

	//copy the non-nan and not masked values into a new vector
	QVector<double> rawData;
	copyValidData(rawData);
	size_t n = rawData.count();

	//sort the data to calculate the percentiles
	std::sort(rawData.begin(), rawData.end());

	//calculate y-values - the percentiles for the column data
	Column* yColumn = new Column("y");
	m_project->addChildFast(yColumn);
	QVector<double> yData;
	for (int i = 1; i < 100; ++i)
		yData << gsl_stats_quantile_from_sorted_data(rawData.data(), 1, n, double(i)/100.);

	yColumn->replaceValues(0, yData);

	//calculate x-values - the percentiles for the standard normal distribution
	Column* xColumn = new Column("x");
	m_project->addChildFast(xColumn);
	QVector<double> xData;
	for (int i = 1; i < 100; ++i)
		xData << gsl_cdf_gaussian_Pinv(double(i)/100., 1.0);

	xColumn->replaceValues(0, xData);

	//add curve with the quantiles
	XYCurve* curve = new XYCurve("");
	curve->suppressRetransform(true);
	plot->addChild(curve);
	curve->setLinePen(Qt::NoPen);
	curve->symbol()->setStyle(Symbol::Style::Circle);
	curve->setFillingPosition(XYCurve::FillingPosition::NoFilling);
	curve->setXColumn(xColumn);
	curve->setYColumn(yColumn);

	//add the reference line connecting (x1, y1) = (-0.6745, Q1) and (x2, y2) = (0.6745, Q2)
	double y1 = gsl_stats_quantile_from_sorted_data(rawData.data(), 1, n, 0.25);
	double y2 = gsl_stats_quantile_from_sorted_data(rawData.data(), 1, n, 0.75);
	double x1 = -0.6745;
	double x2 = 0.6745;

	//we only want do show the line starting from x = PInv(0.01) = -2.32635
	//and going to x = PInv(0.99) = 2.32635;
	double k = (y2 - y1)/(x2 - x1);
	double b = y1 - k*x1;
	double x1New = -2.32635;
	double x2New = 2.32635;
	double y1New = k*x1New + b;
	double y2New = k*x2New + b;

	Column* xColumn2 = new Column("x2");
	m_project->addChildFast(xColumn2);
	xColumn2->setValueAt(0, x1New);
	xColumn2->setValueAt(1, x2New);

	Column* yColumn2 = new Column("y2");
	m_project->addChildFast(yColumn2);
	yColumn2->setValueAt(0, y1New);
	yColumn2->setValueAt(1, y2New);

	XYCurve* curve2 = new XYCurve("2");
	curve2->suppressRetransform(true);
	plot->addChild(curve2);
	QPen pen = curve2->linePen();
	pen.setStyle(Qt::SolidLine);
	curve2->setLinePen(pen);
	curve2->symbol()->setStyle(Symbol::Style::NoSymbols);
	curve2->setFillingPosition(XYCurve::FillingPosition::NoFilling);
	curve2->setXColumn(xColumn2);
	curve2->setYColumn(yColumn2);

	curve->suppressRetransform(false);
	curve2->suppressRetransform(false);
	plot->retransform();
	m_qqPlotInitialized = true;
	RESET_CURSOR;
}

void StatisticsColumnWidget::showBoxPlot() {
	WAIT_CURSOR;

	//add plot
	auto* plot = addPlot(&m_boxPlotWidget);

	auto axes = plot->children<Axis>();
	for (auto* axis : qAsConst(axes)) {
		auto wrapper = axis->title()->text();
		QTextEdit te(wrapper.text);
		te.selectAll();
		if (axis->orientation() == Axis::Orientation::Horizontal) {
			axis->setLabelsPosition(Axis::LabelsPosition::NoLabels);
			axis->setMajorTicksDirection(Axis::noTicks);
			axis->setMajorGridPen(QPen(Qt::NoPen));
			axis->setMinorGridPen(QPen(Qt::NoPen));
			te.setText(QString());
		} else
			te.setText(m_column->name());

		axis->title()->setText(te.toHtml());

		axis->setMinorTicksDirection(Axis::noTicks);
	}
	QApplication::processEvents(QEventLoop::AllEvents, 100);

	auto* boxPlot = new BoxPlot(QString());
	boxPlot->setOrientation(BoxPlot::Orientation::Vertical);
	boxPlot->setWhiskersType(BoxPlot::WhiskersType::IQR);
	plot->addChild(boxPlot);

	QVector<const AbstractColumn*> columns;
	columns << const_cast<Column*>(m_column);
	boxPlot->setDataColumns(columns);

	plot->retransform();
	m_boxPlotInitialized = true;
	RESET_CURSOR;
}

CartesianPlot* StatisticsColumnWidget::addPlot(QWidget* widget) {
	Worksheet* worksheet = new Worksheet(QString());
	worksheet->setUseViewSize(true);
	worksheet->setLayoutTopMargin(0.);
	worksheet->setLayoutBottomMargin(0.);
	worksheet->setLayoutLeftMargin(0.);
	worksheet->setLayoutRightMargin(0.);
	m_project->addChild(worksheet);

	auto* plot = new CartesianPlot(QString());
	plot->setSuppressRetransform(true);
	plot->setType(CartesianPlot::Type::TwoAxes);
	plot->setSymmetricPadding(false);
	const double padding = Worksheet::convertToSceneUnits(1.0, Worksheet::Unit::Centimeter);
	plot->setRightPadding(padding);
	plot->setVerticalPadding(padding);

	QPen pen = plot->plotArea()->borderPen();
	pen.setStyle(Qt::NoPen);
	plot->plotArea()->setBorderPen(pen);

	worksheet->addChild(plot);
	plot->setSuppressRetransform(false);

	auto* layout = new QVBoxLayout(widget);
	layout->setSpacing(0);
	layout->addWidget(worksheet->view());
	worksheet->setInteractive(false);
	widget->setLayout(layout);

	return plot;
}

//helpers
const QString StatisticsColumnWidget::isNanValue(const double value) const {
	SET_NUMBER_LOCALE
	return (std::isnan(value) ? QLatin1String("-") : numberLocale.toString(value,'f'));
}

QString StatisticsColumnWidget::modeValue(const Column* column, double value) const {
	if (std::isnan(value))
		return QLatin1String("-");

	SET_NUMBER_LOCALE
	switch (column->columnMode()) {
	case AbstractColumn::ColumnMode::Integer:
		return numberLocale.toString((int)value);
	case AbstractColumn::ColumnMode::BigInt:
		return numberLocale.toString((qint64)value);
	case AbstractColumn::ColumnMode::Text:
		//TODO
	case AbstractColumn::ColumnMode::DateTime:
		//TODO
	case AbstractColumn::ColumnMode::Day:
		//TODO
	case AbstractColumn::ColumnMode::Month:
		//TODO
	case AbstractColumn::ColumnMode::Double:
		return numberLocale.toString(value, 'f');
	}

	return QString();
}

/*!
 * copy the non-nan and not masked values of the current column
 * into the vector \c data.
 */
void StatisticsColumnWidget::copyValidData(QVector<double>& data) const {
	const int rowCount = m_column->rowCount();
	data.reserve(rowCount);
	double val;
	if (m_column->columnMode() == AbstractColumn::ColumnMode::Double) {
		auto* rowValues = reinterpret_cast<QVector<double>*>(m_column->data());
		for (int row = 0; row < rowCount; ++row) {
			val = rowValues->value(row);
			if (std::isnan(val) || m_column->isMasked(row))
				continue;

			data.push_back(val);
		}
	} else if (m_column->columnMode() == AbstractColumn::ColumnMode::Integer) {
		auto* rowValues = reinterpret_cast<QVector<int>*>(m_column->data());
		for (int row = 0; row < rowCount; ++row) {
			val = rowValues->value(row);
			if (std::isnan(val) || m_column->isMasked(row))
				continue;

			data.push_back(val);
		}
	} else if (m_column->columnMode() == AbstractColumn::ColumnMode::BigInt) {
		auto* rowValues = reinterpret_cast<QVector<qint64>*>(m_column->data());
		for (int row = 0; row < rowCount; ++row) {
			val = rowValues->value(row);
			if (std::isnan(val) || m_column->isMasked(row))
				continue;

			data.push_back(val);
		}
	}

	if (data.size() < rowCount)
		data.squeeze();
}
