/*
	File                 : StatisticsColumnWidget.cpp
	Project              : LabPlot
	Description          : Widget showing statistics for column values
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2021-2026 Alexander Semke <alexander.semke@web.de>
	SPDX-FileCopyrightText: 2022 Stefan Gerlach <stefan.gerlach@uni.kn>

	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "StatisticsColumnWidget.h"
#include "backend/core/Project.h"
#include "backend/core/column/Column.h"
#include "backend/core/datatypes/DateTime2StringFilter.h"
#include "backend/lib/trace.h"
#include "backend/worksheet/Background.h"
#include "backend/worksheet/Line.h"
#include "backend/worksheet/TextLabel.h"
#include "backend/worksheet/Worksheet.h"
#include "backend/worksheet/plots/cartesian/Axis.h"
#include "backend/worksheet/plots/cartesian/BarPlot.h"
#include "backend/worksheet/plots/cartesian/BoxPlot.h"
#include "backend/worksheet/plots/cartesian/CartesianPlot.h"
#include "backend/worksheet/plots/cartesian/Histogram.h"
#include "backend/worksheet/plots/cartesian/KDEPlot.h"
#include "backend/worksheet/plots/cartesian/QQPlot.h"
#include "backend/worksheet/plots/cartesian/Symbol.h"
#include "backend/worksheet/plots/cartesian/Value.h"
#include "backend/worksheet/plots/cartesian/XYCurve.h"
#include "frontend/GuiTools.h"

#include <QTabWidget>
#include <QTextEdit>
#include <QTimer>
#include <QVBoxLayout>

#include <KLocalizedString>

#include <algorithm> //for min_element and max_element

extern "C" {
#include "backend/nsl/nsl_kde.h"
}
#include <gsl/gsl_cdf.h>
#include <gsl/gsl_math.h>
#include <gsl/gsl_statistics.h>

StatisticsColumnWidget::StatisticsColumnWidget(const Column* column, QWidget* parent)
	: QWidget(parent)
	, m_column(column)
	, m_project(new Project)
	, m_tabWidget(new QTabWidget) {
	auto* layout = new QVBoxLayout;
	layout->addWidget(m_tabWidget);
	setLayout(layout);

	const QString htmlColor = GuiTools::isDarkMode() ? QLatin1String("#5f5f5f") : QLatin1String("#D1D1D1");
	// clang-format off
	if (column->isNumeric()) {
		m_htmlOverview = QStringLiteral("<table border=0 width=100%><tr><td colspan=2 align=center bgcolor=") + htmlColor
			+ QStringLiteral("><b><big>") + i18n("Location Measures") + QStringLiteral("</big><b></td></tr>")
			+ QStringLiteral("<tr><td width=60%><b>") + i18n("Count") + QStringLiteral("<b></td><td>%1</td></tr>")
			+ QStringLiteral("<tr><td><b>") + i18n("Sum") + QStringLiteral("<b></td><td>%2</td></tr>")
			+ QStringLiteral("<tr><td><b>") + i18n("Minimum") + QStringLiteral("<b></td><td>%3</td></tr>")
			+ QStringLiteral("<tr><td><b>") + i18n("Maximum") + QStringLiteral("<b></td><td>%4</td></tr>")
			+ QStringLiteral("<tr><td><b>") + i18n("Arithmetic Mean") + QStringLiteral("<b></td><td>%5</td></tr>")
			+ QStringLiteral("<tr><td><b>") + i18n("Geometric Mean") + QStringLiteral("<b></td><td>%6</td></tr>")
			+ QStringLiteral("<tr><td><b>") + i18n("Harmonic Mean") + QStringLiteral("<b></td><td>%7</td></tr>")
			+ QStringLiteral("<tr><td><b>") + i18n("Contraharmonic Mean") + QStringLiteral("<b></td><td>%8</td></tr>")
			+ QStringLiteral("<tr><td><b>") + i18n("Mode") + QStringLiteral("<b></td><td>%9</td></tr>")
			+ QStringLiteral("<tr><td><b>") + i18n("First Quartile") + QStringLiteral("<b></td><td>%10</td></tr>")
			+ QStringLiteral("<tr><td><b>") + i18n("Median") + QStringLiteral("<b></td><td>%11</td></tr>")
			+ QStringLiteral("<tr><td><b>") + i18n("Third Quartile") + QStringLiteral("<b></td><td>%12</td></tr>")
			+ QStringLiteral("<tr><td><b>") + i18n("Trimean") + QStringLiteral("<b></td><td>%13</td></tr>")
			+ QStringLiteral("<tr></tr>")
			+ QStringLiteral("<tr><td colspan=2 align=center bgcolor=") + htmlColor + QStringLiteral("><b><big>")
			+ i18n("Dispersion Measures") + QStringLiteral("</big></b></td></tr>")
			+ QStringLiteral("<tr><td><b>") + i18n("Range") + QStringLiteral("<b></td><td>%14</td></tr>")
			+ QStringLiteral("<tr><td><b>") + i18n("Variance") + QStringLiteral("<b></td><td>%15</td></tr>")
			+ QStringLiteral("<tr><td><b>") + i18n("Standard Deviation") + QStringLiteral("<b></td><td>%16</td></tr>")
			+ QStringLiteral("<tr><td><b>") + i18n("Mean Absolute Deviation Around Mean") + QStringLiteral("<b></td><td>%17</td></tr>")
			+ QStringLiteral("<tr><td><b>") + i18n("Mean Absolute Deviation Around Median") + QStringLiteral("<b></td><td>%18</td></tr>")
			+ QStringLiteral("<tr><td><b>") + i18n("Median Absolute Deviation") + QStringLiteral("<b></td><td>%19</td></tr>")
			+ QStringLiteral("<tr><td><b>") + i18n("Interquartile Range") + QStringLiteral("<b></td><td>%20</td></tr>")
			+ QStringLiteral("<tr><td><b>") + i18n("Average 2-Period Moving Range") + QStringLiteral("<b></td><td>%21</td></tr>")
			+ QStringLiteral("<tr></tr>")
			+ QStringLiteral("<tr><td colspan=2 align=center bgcolor=") + htmlColor + QStringLiteral("><b><big>")
			+ i18n("Shape Measures") + QStringLiteral("</big></b></td></tr>")
			+ QStringLiteral("<tr><td><b>") + i18n("Skewness") + QStringLiteral("<b></td><td>%22</td></tr>")
			+ QStringLiteral("<tr><td><b>") + i18n("Kurtosis") + QStringLiteral("<b></td><td>%23</td></tr>")
			+ QStringLiteral("<tr><td><b>") + i18n("Entropy") + QStringLiteral("<b></td><td>%24</td></tr>")
			+ QStringLiteral("</table>");
	} else if (column->columnMode() == AbstractColumn::ColumnMode::Text) {
		m_htmlOverview = QStringLiteral("<table border=0 width=100%><tr><td colspan=2 align=center bgcolor=")
			+ htmlColor + QStringLiteral("><b><big>") + i18n("General") + QStringLiteral("</big><b></td></tr><tr>")
			+ QStringLiteral("<td width=60%><b>") + i18n("Count") + QStringLiteral("<b></td><td>%1</td></tr>")
			+ QStringLiteral("<tr><td><b>") + i18n("Unique Values") + QStringLiteral("<b></td><td>%2</td></tr>")
			+ QStringLiteral("</table>");
	} else { // datetime
		m_htmlOverview = QStringLiteral("<table border=0 width=100%><tr><td colspan=2 align=center bgcolor=")
			+ htmlColor + QStringLiteral("><b><big>") + i18n("General") + QStringLiteral("</big><b></td></tr>")
			+ QStringLiteral("<tr><td width=60%><b>") + i18n("Count") + QStringLiteral("<b></td><td>%1</td></tr>")
			+ QStringLiteral("<tr><td><b>") + i18n("Minimum") + QStringLiteral("<b></td><td>%2</td></tr>")
			+ QStringLiteral("<tr><td><b>") + i18n("Maximum") + QStringLiteral("<b></td><td>%3</td></tr>")
			+ QStringLiteral("</table>");
	}
	// clang-format on

	// create tab widgets for every column and show the initial text with the placeholders
	auto* vBoxLayout = new QVBoxLayout(&m_overviewWidget);
	vBoxLayout->setSpacing(0);
	m_overviewWidget.setLayout(vBoxLayout);
	m_overviewPlotWidget.setMaximumHeight(150);
	vBoxLayout->addWidget(&m_overviewPlotWidget);

	m_teOverview = new QTextEdit(this);
	m_teOverview->setReadOnly(true);
	vBoxLayout->addWidget(m_teOverview);

	m_tabWidget->addTab(&m_overviewWidget, i18n("Overview"));

	if (column->isNumeric()) {
		m_teOverview->setHtml(m_htmlOverview
								  .arg(QLatin1String("-"),
									   QLatin1String("-"),
									   QLatin1String("-"),
									   QLatin1String("-"),
									   QLatin1String("-"),
									   QLatin1String("-"),
									   QLatin1String("-"),
									   QLatin1String("-"),
									   QLatin1String("-"))
								  .arg(QLatin1String("-"),
									   QLatin1String("-"),
									   QLatin1String("-"),
									   QLatin1String("-"),
									   QLatin1String("-"),
									   QLatin1String("-"),
									   QLatin1String("-"),
									   QLatin1String("-"),
									   QLatin1String("-"))
								  .arg(QLatin1String("-"),
									   QLatin1String("-"),
									   QLatin1String("-"),
									   QLatin1String("-"),
									   QLatin1String("-"),
									   QLatin1String("-")));
		m_tabWidget->addTab(&m_histogramWidget, i18n("Histogram"));
		m_tabWidget->addTab(&m_kdePlotWidget, i18n("KDE Plot"));
		m_tabWidget->addTab(&m_qqPlotWidget, i18n("Normal Q-Q Plot"));
		m_tabWidget->addTab(&m_boxPlotWidget, i18n("Box Plot"));
	} else if (column->columnMode() == AbstractColumn::ColumnMode::Text) {
		m_teOverview->setHtml(m_htmlOverview.arg(QLatin1String("-"), QLatin1String("-")));
		m_tabWidget->addTab(&m_barPlotWidget, i18n("Bar Plot"));
		m_tabWidget->addTab(&m_paretoPlotWidget, i18n("Pareto Plot"));
	} else { // datetime
		m_teOverview->setHtml(m_htmlOverview.arg(QLatin1String("-"), QLatin1String("-"), QLatin1String("-")));
	}

	connect(m_tabWidget, &QTabWidget::currentChanged, this, &StatisticsColumnWidget::currentTabChanged);
}

StatisticsColumnWidget::~StatisticsColumnWidget() {
	disconnect(m_tabWidget, nullptr, this, nullptr); // don't react on currentChanged signal
	delete m_project;
}

void StatisticsColumnWidget::setCurrentTab(int index) {
	if (index == m_tabWidget->currentIndex())
		currentTabChanged(index); // manually call the slot so we get the data shown
	else
		m_tabWidget->setCurrentIndex(index);
}

void StatisticsColumnWidget::currentTabChanged(int index) {
	WAIT_CURSOR_AUTO_RESET;
	if (m_column->isNumeric()) {
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
	} else {
		if (index == 0 && !m_overviewInitialized)
			showOverview();
		else if (index == 1 && !m_barPlotInitialized)
			showBarPlot();
		else if (index == 2 && !m_paretoPlotInitialized)
			showParetoPlot();
	}

	Q_EMIT tabChanged(index);
}

void StatisticsColumnWidget::showOverview() {
	const Column::ColumnStatistics& statistics = m_column->statistics();

	if (m_column->isNumeric()) {
		m_teOverview->setHtml(m_htmlOverview
								  .arg(QString::number(statistics.size),
									   isNanValue(statistics.sum == INFINITY ? NAN : statistics.sum),
									   isNanValue(statistics.minimum == INFINITY ? NAN : statistics.minimum),
									   isNanValue(statistics.maximum == -INFINITY ? NAN : statistics.maximum),
									   isNanValue(statistics.arithmeticMean),
									   isNanValue(statistics.geometricMean),
									   isNanValue(statistics.harmonicMean),
									   isNanValue(statistics.contraharmonicMean),
									   modeValue(m_column, statistics.mode),
									   isNanValue(statistics.firstQuartile))
								  .arg(isNanValue(statistics.median),
									   isNanValue(statistics.thirdQuartile),
									   isNanValue(statistics.trimean),
									   isNanValue(statistics.maximum - statistics.minimum),
									   isNanValue(statistics.variance),
									   isNanValue(statistics.standardDeviation),
									   isNanValue(statistics.meanDeviation),
									   isNanValue(statistics.meanDeviationAroundMedian),
									   isNanValue(statistics.medianDeviation),
									   isNanValue(statistics.iqr),
									   isNanValue(statistics.averageTwoPeriodMovingRange))
								  .arg(isNanValue(statistics.skewness), isNanValue(statistics.kurtosis), isNanValue(statistics.entropy)));
	} else if (m_column->columnMode() == AbstractColumn::ColumnMode::Text) {
		// add the frequencies table
		const auto& frequencies = m_column->frequencies();
		const QString htmlColor = GuiTools::isDarkMode() ? QStringLiteral("#5f5f5f") : QStringLiteral("#D1D1D1");
		m_htmlOverview += QStringLiteral("<br><table border=0 width=100%>") + QStringLiteral("<tr>") + QStringLiteral("<td colspan=3 align=center bgcolor=")
			+ htmlColor + QStringLiteral("><b><big>") + i18n("Frequency Table") + QStringLiteral("</big><b></td>") + QStringLiteral("</tr>")
			+ QStringLiteral("<tr>") + QStringLiteral("<td width=60%></td>") + QStringLiteral("<td>") + i18n("Frequency") + QStringLiteral("</td>")
			+ QStringLiteral("<td>") + i18n("Percent") + QStringLiteral("</td>") + QStringLiteral("</tr>");

		auto i = frequencies.constBegin();
		while (i != frequencies.constEnd()) {
			int count = i.value();
			double percent = (double)count / statistics.size * 100;
			m_htmlOverview += QStringLiteral("<tr>") + QStringLiteral("<td>") + i.key() + QStringLiteral("</td>") + QStringLiteral("<td>")
				+ QString::number(count) + QStringLiteral("</td>") + QStringLiteral("<td>") + QString::number(percent) + QStringLiteral("%</td>")
				+ QStringLiteral("</tr>");
			++i;
		}

		m_htmlOverview += QStringLiteral("</table>");
		m_teOverview->setHtml(m_htmlOverview.arg(QString::number(statistics.size), QString::number(statistics.unique)));
	} else { // datetime
		auto* filter = static_cast<DateTime2StringFilter*>(m_column->outputFilter());
		m_teOverview->setHtml(m_htmlOverview.arg(QString::number(statistics.size),
												 QDateTime::fromMSecsSinceEpoch(statistics.minimum).toString(filter->format()),
												 QDateTime::fromMSecsSinceEpoch(statistics.maximum).toString(filter->format())));
	}

	showOverviewPlot();
	m_overviewInitialized = true;
}

void StatisticsColumnWidget::showOverviewPlot() {
	if (!m_column->isNumeric())
		return;

	// add plot
	auto* plot = addPlot(&m_overviewPlotWidget);
	plot->setSymmetricPadding(false);
	const double padding = Worksheet::convertToSceneUnits(0.5, Worksheet::Unit::Centimeter);
	plot->setHorizontalPadding(2 * padding);
	plot->setRightPadding(2 * padding);
	plot->setVerticalPadding(padding);
	plot->setBottomPadding(padding);
	plot->borderLine()->setStyle(Qt::NoPen);

	// set the axes labels
	auto axes = plot->children<Axis>();
	for (auto* axis : std::as_const(axes)) {
		axis->setSuppressRetransform(true);
		if (axis->orientation() == Axis::Orientation::Vertical)
			axis->title()->setText(QString());
		else {
			// TODO: set the font and the offset smaller and show the "Index" title after this
			// axis->title()->setText(i18n("Index"));
			axis->title()->setText(QString());
		}

		auto font = axis->labelsFont();
		font.setPointSizeF(Worksheet::convertToSceneUnits(8, Worksheet::Unit::Point));
		axis->setLabelsFont(font);
		axis->setLabelsOffset(2);
		axis->setMajorTicksDirection(Axis::ticksIn);
		axis->majorGridLine()->setStyle(Qt::NoPen);
		axis->setMinorTicksDirection(Axis::noTicks);
		axis->setArrowType(Axis::ArrowType::NoArrow);
		axis->setSuppressRetransform(false);
	}

	QApplication::processEvents(QEventLoop::AllEvents, 100);

	// x
	auto* xColumn = new Column(QStringLiteral("x"), AbstractColumn::ColumnMode::Integer);
	m_project->addChild(xColumn);
	int rows = m_column->rowCount();
	QVector<int> xData;
	xData.resize(rows);
	for (int i = 0; i < rows; ++i)
		xData[i] = i;
	xColumn->setIntegers(xData);

	// add curve
	auto* curve = new XYCurve(QString());
	curve->setSuppressRetransform(false);
	plot->addChild(curve);
	curve->line()->setStyle(Qt::SolidLine);
	curve->symbol()->setStyle(Symbol::Style::NoSymbols);
	curve->background()->setPosition(Background::Position::No);
	curve->setXColumn(xColumn);
	curve->setYColumn(m_column);

	curve->setSuppressRetransform(false);
	plot->retransform();
}

void StatisticsColumnWidget::showHistogram() {
	// add plot
	auto* plot = addPlot(&m_histogramWidget);

	auto axes = plot->children<Axis>();
	for (auto* axis : std::as_const(axes)) {
		if (axis->orientation() == Axis::Orientation::Horizontal) {
			axis->title()->setText(m_column->name());
			axis->majorGridLine()->setStyle(Qt::NoPen);
		} else
			axis->title()->setText(i18n("Frequency"));

		axis->setMinorTicksDirection(Axis::noTicks);
	}
	QApplication::processEvents(QEventLoop::AllEvents, 100);

	auto* histogram = new Histogram(QString());
	plot->addChild(histogram);
	histogram->setDataColumn(m_column);

	m_histogramInitialized = true;
}

void StatisticsColumnWidget::showKDEPlot() {
	// add plot
	auto* plot = addPlot(&m_kdePlotWidget);

	// set the axes labels
	auto axes = plot->children<Axis>();
	for (auto* axis : std::as_const(axes)) {
		if (axis->orientation() == Axis::Orientation::Horizontal)
			axis->title()->setText(m_column->name());
		else
			axis->title()->setText(i18n("Density"));

		axis->setMinorTicksDirection(Axis::noTicks);
	}

	QApplication::processEvents(QEventLoop::AllEvents, 100);

	// add normalized histogram
	auto* histogram = new Histogram(QString());
	plot->addChild(histogram);
	histogram->setNormalization(Histogram::ProbabilityDensity);
	histogram->setDataColumn(m_column);

	// add KDE Plot
	auto* kdePlot = new KDEPlot(QString());
	plot->addChild(kdePlot);
	kdePlot->setKernelType(nsl_kernel_gauss);
	kdePlot->setBandwidthType(nsl_kde_bandwidth_silverman);
	kdePlot->setDataColumn(m_column);

	m_kdePlotInitialized = true;
}

void StatisticsColumnWidget::showQQPlot() {
	// add plot
	auto* plot = addPlot(&m_qqPlotWidget);

	auto axes = plot->children<Axis>();
	for (auto* axis : std::as_const(axes)) {
		if (axis->orientation() == Axis::Orientation::Horizontal)
			axis->title()->setText(i18n("Theoretical Quantiles"));
		else
			axis->title()->setText(i18n("Sample Quantiles"));

		axis->setMinorTicksDirection(Axis::noTicks);
	}
	QApplication::processEvents(QEventLoop::AllEvents, 100);

	auto* qqPlot = new QQPlot(QString());
	plot->addChild(qqPlot);
	qqPlot->setDataColumn(m_column);

	m_qqPlotInitialized = true;
}

void StatisticsColumnWidget::showBoxPlot() {
	// add plot
	auto* plot = addPlot(&m_boxPlotWidget);

	auto axes = plot->children<Axis>();
	for (auto* axis : std::as_const(axes)) {
		if (axis->orientation() == Axis::Orientation::Horizontal) {
			axis->setLabelsPosition(Axis::LabelsPosition::NoLabels);
			axis->setMajorTicksDirection(Axis::noTicks);
			axis->majorGridLine()->setStyle(Qt::NoPen);
			axis->minorGridLine()->setStyle(Qt::NoPen);
			axis->title()->setText(QString());
		} else
			axis->title()->setText(m_column->name());

		axis->setMinorTicksDirection(Axis::noTicks);
	}
	QApplication::processEvents(QEventLoop::AllEvents, 100);

	auto* boxPlot = new BoxPlot(QString());
	boxPlot->setOrientation(BoxPlot::Orientation::Vertical);
	boxPlot->setWhiskersType(BoxPlot::WhiskersType::IQR);
	boxPlot->setDataColumns({m_column});

	// add the child to the parent _after_ the data column was set so the theme settings are properly applied in loadThemeConfig().
	plot->addChild(boxPlot);

	m_boxPlotInitialized = true;
}

void StatisticsColumnWidget::showBarPlot() {
	// add plot
	auto* plot = addPlot(&m_barPlotWidget);
	plot->title()->setText(m_column->name());
	QApplication::processEvents(QEventLoop::AllEvents, 100);

	// generate columns holding the data and the labels
	auto* dataColumn = new Column(QStringLiteral("data"));
	dataColumn->setColumnMode(AbstractColumn::ColumnMode::Integer);
	m_project->addChild(dataColumn);

	auto* labelsColumn = new Column(QStringLiteral("labels"));
	labelsColumn->setColumnMode(AbstractColumn::ColumnMode::Text);
	m_project->addChild(labelsColumn);

	// sort the frequencies and the accompanying labels
	const auto& frequencies = m_column->frequencies();
	auto i = frequencies.constBegin();
	QVector<QPair<QString, int>> pairs;
	while (i != frequencies.constEnd()) {
		pairs << QPair<QString, int>(i.key(), i.value());
		++i;
	}

	std::sort(pairs.begin(), pairs.end(), [](QPair<QString, int> a, QPair<QString, int> b) {
		return a.second > b.second;
	});

	QVector<int> bdata;
	QVector<QString> labels;
	for (const auto& pair : pairs) {
		labels << pair.first;
		bdata << pair.second;
	}
	dataColumn->replaceInteger(0, bdata);
	labelsColumn->replaceTexts(0, labels);

	// bar plot
	auto* barPlot = new BarPlot(QString());
	barPlot->setDataColumns({dataColumn});

	// add the child to the parent _after_ the data column was set so the theme settings are properly applied in loadThemeConfig().
	plot->addChild(barPlot);

	barPlot->setOrientation(BoxPlot::Orientation::Vertical);
	barPlot->value()->setType(Value::Type::BinEntries);
	barPlot->value()->setPosition(Value::Position::Above);

	// axes properties
	auto axes = plot->children<Axis>();
	for (auto* axis : std::as_const(axes)) {
		if (axis->orientation() == Axis::Orientation::Horizontal) {
			axis->title()->setText(QString());
			axis->majorGridLine()->setStyle(Qt::NoPen);
			axis->setMajorTicksStartType(Axis::TicksStartType::Offset);
			axis->setMajorTickStartOffset(0.5);
			axis->setMajorTicksType(Axis::TicksType::Spacing);
			axis->setMajorTicksSpacing(1.);
			axis->setLabelsTextType(Axis::LabelsTextType::CustomValues);
			axis->setLabelsTextColumn(labelsColumn);
		} else {
			axis->title()->setText(i18n("Frequency"));
			axis->setTitleOffsetX(Worksheet::convertToSceneUnits(-5, Worksheet::Unit::Point));
		}

		axis->setMinorTicksDirection(Axis::noTicks);
		axis->setArrowType(Axis::ArrowType::NoArrow);
	}

	m_barPlotInitialized = true;
}

void StatisticsColumnWidget::showParetoPlot() {
	DEBUG(Q_FUNC_INFO)
	auto* plot = addPlot(&m_paretoPlotWidget);
	plot->title()->setText(m_column->name());
	plot->setHorizontalPadding(Worksheet::convertToSceneUnits(2, Worksheet::Unit::Centimeter));
	plot->setRightPadding(Worksheet::convertToSceneUnits(3.2, Worksheet::Unit::Centimeter));

	// add second range for the cumulative percentage of the total number of occurrences
	plot->addYRange(Range<double>(0, 100)); // add second y range
	plot->addCoordinateSystem(); // add cs for second y range
	plot->setCoordinateSystemRangeIndex(plot->coordinateSystemCount() - 1, Dimension::Y, 1); // specify new y range for new cs
	plot->enableAutoScale(Dimension::Y, 1, false); // disable auto scale to stay at 0 .. 100

	// add second y-axis
	auto* secondAxis = new Axis(QLatin1String("y2"));
	plot->addChild(secondAxis);
	secondAxis->setOrientation(Axis::Orientation::Vertical);
	secondAxis->setPosition(Axis::Position::Right);
	secondAxis->setMajorTicksDirection(Axis::ticksBoth);
	secondAxis->setLabelsPosition(Axis::LabelsPosition::In);
	secondAxis->setLabelsSuffix(QLatin1String("%"));
	secondAxis->title()->setRotationAngle(90);
	secondAxis->setCoordinateSystemIndex(1);

	QApplication::processEvents(QEventLoop::AllEvents, 100);

	// generate columns holding the data and the labels
	int count = m_column->statistics().unique;

	auto* dataColumn = new Column(QStringLiteral("data"));
	dataColumn->setColumnMode(AbstractColumn::ColumnMode::Integer);
	m_project->addChild(dataColumn);

	auto* xColumn = new Column(QStringLiteral("x"));
	xColumn->setColumnMode(AbstractColumn::ColumnMode::Double);
	m_project->addChild(xColumn);
	QVector<double> xData(count);

	auto* yColumn = new Column(QStringLiteral("y"));
	m_project->addChild(yColumn);
	QVector<double> yData(count);

	auto* labelsColumn = new Column(QStringLiteral("labels"));
	labelsColumn->setColumnMode(AbstractColumn::ColumnMode::Text);

	// sort the frequencies and the accompanying labels and calculate the total sum of frequencies
	const auto& frequencies = m_column->frequencies();
	auto i = frequencies.constBegin();
	QVector<QPair<QString, int>> pairs;
	int row = 0;
	int totalSumOfFrequencies = 0;
	while (i != frequencies.constEnd()) {
		pairs << QPair<QString, int>(i.key(), i.value());
		xData[row] = 0.5 + row;
		totalSumOfFrequencies += i.value();
		++row;
		++i;
	}

	std::sort(pairs.begin(), pairs.end(), [](QPair<QString, int> a, QPair<QString, int> b) {
		return a.second > b.second;
	});

	QVector<int> bdata;
	QVector<QString> labels;
	for (const auto& pair : pairs) {
		labels << pair.first;
		bdata << pair.second;
	}

	// calculate the cumulative values
	int sum = 0;
	row = 0;
	for (auto value : bdata) {
		sum += value;
		if (totalSumOfFrequencies != 0)
			yData[row] = (double)sum / totalSumOfFrequencies * 100;
		++row;
	}

	dataColumn->replaceInteger(0, bdata);
	labelsColumn->replaceTexts(0, labels);
	xColumn->setValues(xData);
	yColumn->setValues(yData);

	// add bar plot
	auto* barPlot = new BarPlot(QString());
	barPlot->setOrientation(BoxPlot::Orientation::Vertical);
	barPlot->setDataColumns({dataColumn});

	// add the child to the parent _after_ the data column was set so the theme settings are properly applied in loadThemeConfig().
	plot->addChild(barPlot);

	// add cumulated percentage curve
	auto* curve = new XYCurve(QStringLiteral("curve"));
	plot->addChild(curve);
	curve->setCoordinateSystemIndex(1); // assign to the second y-range going from 0 to 100%
	curve->setXColumn(xColumn);
	curve->setYColumn(yColumn);
	curve->line()->setStyle(Qt::SolidLine);
	curve->symbol()->setStyle(Symbol::Style::Circle);
	curve->value()->setType(Value::Type::Y);
	curve->value()->setPosition(Value::Position::Right);
	curve->value()->setDistance(Worksheet::convertToSceneUnits(10, Worksheet::Unit::Point));
	curve->value()->setSuffix(QStringLiteral("%"));

	// resize the first y range to have the first point of the xy-curve at the top of the first bar
	if (yData.at(0) != 0) {
		const double max = (double)bdata.at(0) * 100. / yData.at(0);
		plot->setMax(Dimension::Y, 0, max);
	}

	// axes properties
	auto axes = plot->children<Axis>();
	bool firstYAxis = false;
	for (auto* axis : std::as_const(axes)) {
		if (axis->orientation() == Axis::Orientation::Horizontal) {
			axis->title()->setText(QString());
			axis->majorGridLine()->setStyle(Qt::NoPen);
			axis->setMajorTicksStartType(Axis::TicksStartType::Offset);
			axis->setMajorTickStartOffset(0.5);
			axis->setMajorTicksType(Axis::TicksType::Spacing);
			axis->setMajorTicksSpacing(1.);
			axis->setLabelsTextType(Axis::LabelsTextType::CustomValues);
			axis->setLabelsTextColumn(labelsColumn);
		} else {
			if (!firstYAxis) {
				axis->title()->setText(i18n("Frequency"));
				axis->setTitleOffsetX(Worksheet::convertToSceneUnits(-5, Worksheet::Unit::Point));
				axis->setMajorTicksNumber(10 + 1); // same tick number as percentage axis
				firstYAxis = true;
			} else {
				axis->title()->setText(i18n("Cumulative Percentage"));
				// TODO: work with the same offset as for the first axis after https://invent.kde.org/education/labplot/-/issues/368 was addressed
				axis->setTitleOffsetX(Worksheet::convertToSceneUnits(1.8, Worksheet::Unit::Centimeter));
			}
		}

		axis->setMinorTicksDirection(Axis::noTicks);
		axis->setArrowType(Axis::ArrowType::NoArrow);
	}

	m_paretoPlotInitialized = true;
}

CartesianPlot* StatisticsColumnWidget::addPlot(QWidget* widget) {
	auto* ws = new Worksheet(QString());
	ws->setUseViewSize(true);
	ws->setLayoutTopMargin(0.);
	ws->setLayoutBottomMargin(0.);
	ws->setLayoutLeftMargin(0.);
	ws->setLayoutRightMargin(0.);
	m_project->addChild(ws);

	auto* plot = new CartesianPlot(QString());
	plot->setSuppressRetransform(true);
	plot->setType(CartesianPlot::Type::TwoAxes);
	plot->setSymmetricPadding(false);
	const double padding = Worksheet::convertToSceneUnits(1.0, Worksheet::Unit::Centimeter);
	plot->setRightPadding(padding);
	plot->setVerticalPadding(padding);
	plot->borderLine()->setStyle(Qt::NoPen);

	ws->addChild(plot);
	plot->setSuppressRetransform(false);

	auto* layout = new QVBoxLayout(widget);
	layout->setSpacing(0);
	layout->addWidget(ws->view());
	ws->setInteractive(false);
	widget->setLayout(layout);

	return plot;
}

// helpers
const QString StatisticsColumnWidget::isNanValue(const double value) const {
	return (std::isnan(value) ? QLatin1String("-") : QLocale().toString(value, 'f'));
}

QString StatisticsColumnWidget::modeValue(const Column* column, double value) const {
	if (std::isnan(value))
		return QLatin1String("-");

	const auto numberLocale = QLocale();
	switch (column->columnMode()) {
	case AbstractColumn::ColumnMode::Integer:
		return numberLocale.toString((int)value);
	case AbstractColumn::ColumnMode::BigInt:
		return numberLocale.toString((qint64)value);
	case AbstractColumn::ColumnMode::Text:
		// TODO
	case AbstractColumn::ColumnMode::DateTime:
		// TODO
	case AbstractColumn::ColumnMode::Day:
		// TODO
	case AbstractColumn::ColumnMode::Month:
		// TODO
	case AbstractColumn::ColumnMode::Double:
		return numberLocale.toString(value, 'f');
	}

	return {};
}

/*!
 * copy the non-nan and not masked values of the current column
 * into the vector \c data.
 */
void StatisticsColumnWidget::copyValidData(QVector<double>& bdata) const {
	const int rowCount = m_column->rowCount();
	bdata.reserve(rowCount);
	double val;
	if (m_column->columnMode() == AbstractColumn::ColumnMode::Double) {
		auto* rowValues = reinterpret_cast<QVector<double>*>(m_column->data());
		for (int row = 0; row < rowCount; ++row) {
			val = rowValues->value(row);
			if (std::isnan(val) || m_column->isMasked(row))
				continue;

			bdata.push_back(val);
		}
	} else if (m_column->columnMode() == AbstractColumn::ColumnMode::Integer) {
		auto* rowValues = reinterpret_cast<QVector<int>*>(m_column->data());
		for (int row = 0; row < rowCount; ++row) {
			val = rowValues->value(row);
			if (std::isnan(val) || m_column->isMasked(row))
				continue;

			bdata.push_back(val);
		}
	} else if (m_column->columnMode() == AbstractColumn::ColumnMode::BigInt) {
		auto* rowValues = reinterpret_cast<QVector<qint64>*>(m_column->data());
		for (int row = 0; row < rowCount; ++row) {
			val = rowValues->value(row);
			if (std::isnan(val) || m_column->isMasked(row))
				continue;

			bdata.push_back(val);
		}
	}

	if (bdata.size() < rowCount)
		bdata.squeeze();
}
