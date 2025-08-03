/*
	File                 : StatisticalPlotsTest.cpp
	Project              : LabPlot
	Description          : Tests for statistical plots like Q-Q plot, KDE plot, etc.
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2023-2025 Alexander Semke <alexander.semke@web.de>

	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "StatisticalPlotsTest.h"
#include "backend/core/Project.h"
#include "backend/core/column/Column.h"
#include "backend/spreadsheet/Spreadsheet.h"
#include "backend/worksheet/Worksheet.h"
#include "backend/worksheet/plots/cartesian/BarPlot.h"
#include "backend/worksheet/plots/cartesian/Histogram.h"
#include "backend/worksheet/plots/cartesian/KDEPlot.h"
#include "backend/worksheet/plots/cartesian/ProcessBehaviorChart.h"
#include "backend/worksheet/plots/cartesian/QQPlot.h"
#include "backend/worksheet/plots/cartesian/RunChart.h"
#include "backend/worksheet/plots/cartesian/XYCurve.h"

#include <gsl/gsl_randist.h>
#include <gsl/gsl_rng.h>

#include <QUndoStack>

// ##############################################################################
// ############################## Histogram #####################################
// ##############################################################################
/*!
 * \brief create and add a new Histogram, undo and redo this step
 */
void StatisticalPlotsTest::testHistogramInit() {
	Project project;
	auto* ws = new Worksheet(QStringLiteral("worksheet"));
	project.addChild(ws);

	auto* p = new CartesianPlot(QStringLiteral("plot"));
	ws->addChild(p);

	auto* histogram = new Histogram(QStringLiteral("histogram"));
	p->addChild(histogram);

	auto children = p->children<Histogram>();

	QCOMPARE(children.size(), 1);

	project.undoStack()->undo();
	children = p->children<Histogram>();
	QCOMPARE(children.size(), 0);

	project.undoStack()->redo();
	children = p->children<Histogram>();
	QCOMPARE(children.size(), 1);
}

/*!
 * \brief create and add a new Histogram, duplicate it and check the number of children
 */
void StatisticalPlotsTest::testHistogramDuplicate() {
	Project project;
	auto* ws = new Worksheet(QStringLiteral("worksheet"));
	project.addChild(ws);

	auto* p = new CartesianPlot(QStringLiteral("plot"));
	ws->addChild(p);

	auto* histogram = new Histogram(QStringLiteral("histogram"));
	p->addChild(histogram);

	histogram->duplicate();

	auto children = p->children<Histogram>();
	QCOMPARE(children.size(), 2);
}

/*!
 * \brief create a Histogram for 3 values check the plot ranges.
 */
void StatisticalPlotsTest::testHistogramRangeBinningTypeChanged() {
	// prepare the data
	Spreadsheet sheet(QStringLiteral("test"), false);
	sheet.setColumnCount(1);
	sheet.setRowCount(100);
	auto* column = sheet.column(0);
	column->setValueAt(0, 1.);
	column->setValueAt(1, 2.);
	column->setValueAt(2, 3.);

	// prepare the worksheet + plot
	auto* ws = new Worksheet(QStringLiteral("worksheet"));
	auto* p = new CartesianPlot(QStringLiteral("plot"));
	ws->addChild(p);

	auto* histogram = new Histogram(QStringLiteral("histogram"));
	histogram->setBinningMethod(Histogram::BinningMethod::ByNumber);
	histogram->setBinCount(3);
	histogram->setDataColumn(column);
	p->addChild(histogram);

	// the x-range is defined by the min and max values in the data [1, 3]
	// because of the bin count 3 we have one value in every bin and the y-range is [0,1]
	const auto& rangeX = p->range(Dimension::X);
	const auto& rangeY = p->range(Dimension::Y);
	QCOMPARE(rangeX.start(), 1);
	QCOMPARE(rangeX.end(), 3);
	QCOMPARE(rangeY.start(), 0);
	QCOMPARE(rangeY.end(), 1);

	// set the bin number to 1, the values 1 and 2 fall into the same bin
	histogram->setBinCount(1);
	QCOMPARE(rangeX.start(), 1);
	QCOMPARE(rangeX.end(), 3);
	QCOMPARE(rangeY.start(), 0);
	QCOMPARE(rangeY.end(), 2);
}

/*!
 * \brief create a Histogram for 3 values check the plot ranges after a row was removed in the source spreadsheet.
 */
void StatisticalPlotsTest::testHistogramRangeRowsChanged() {
	Project project;

	// prepare the data
	auto* sheet = new Spreadsheet(QStringLiteral("test"), false);
	sheet->setColumnCount(1);
	sheet->setRowCount(3);
	project.addChild(sheet);
	auto* column = sheet->column(0);
	column->setValueAt(0, 1.);
	column->setValueAt(1, 2.);
	column->setValueAt(2, 3.);

	// worksheet
	auto* ws = new Worksheet(QStringLiteral("worksheet"));
	project.addChild(ws);

	auto* p = new CartesianPlot(QStringLiteral("plot"));
	ws->addChild(p);

	auto* histogram = new Histogram(QStringLiteral("histogram"));
	histogram->setBinningMethod(Histogram::BinningMethod::ByNumber);
	histogram->setBinCount(3);
	histogram->setDataColumn(column);
	p->addChild(histogram);

	// remove the last row and check the ranges, the x-range should become [1,2]
	sheet->setRowCount(2);
	const auto& rangeX = p->range(Dimension::X);
	const auto& rangeY = p->range(Dimension::Y);
	QCOMPARE(rangeX.start(), 1);
	QCOMPARE(rangeX.end(), 2);
	QCOMPARE(rangeY.start(), 0);
	QCOMPARE(rangeY.end(), 1);

	// undo the row removal and check again, the x-range should become [1,3] again
	project.undoStack()->undo();
	QCOMPARE(rangeX.start(), 1);
	QCOMPARE(rangeX.end(), 3);
	QCOMPARE(rangeY.start(), 0);
	QCOMPARE(rangeY.end(), 1);

	// add more (empty) rows in the spreadsheet, the ranges should be unchanged
	sheet->setRowCount(5);
	QCOMPARE(rangeX.start(), 1);
	QCOMPARE(rangeX.end(), 3);
	QCOMPARE(rangeY.start(), 0);
	QCOMPARE(rangeY.end(), 1);
}

void StatisticalPlotsTest::testHistogramColumnRemoved() {
	Project project;
	auto* ws = new Worksheet(QStringLiteral("worksheet"));
	project.addChild(ws);

	auto* p = new CartesianPlot(QStringLiteral("plot"));
	ws->addChild(p);

	auto* histogram = new Histogram(QStringLiteral("histogram"));
	p->addChild(histogram);

	auto* c = new Column(QStringLiteral("TestColumn"));
	project.addChild(c);

	histogram->setDataColumn(c);
	c->setName(QStringLiteral("NewName"));
	QCOMPARE(histogram->dataColumnPath(), i18n("Project") + QStringLiteral("/NewName"));

	c->remove();

	QCOMPARE(histogram->dataColumn(), nullptr);
	QCOMPARE(histogram->dataColumnPath(), i18n("Project") + QStringLiteral("/NewName"));

	c->setName(QStringLiteral("Another new name")); // Should not lead to a crash

	QCOMPARE(histogram->dataColumn(), nullptr);
	QCOMPARE(histogram->dataColumnPath(), i18n("Project") + QStringLiteral("/NewName"));
}

// ##############################################################################
// ############################## KDE Plot ######################################
// ##############################################################################

/*!
 * \brief create and add a new KDEPlot, undo and redo this step
 */
void StatisticalPlotsTest::testKDEPlotInit() {
	Project project;
	auto* ws = new Worksheet(QStringLiteral("worksheet"));
	project.addChild(ws);

	auto* p = new CartesianPlot(QStringLiteral("plot"));
	ws->addChild(p);

	auto* kdePlot = new KDEPlot(QStringLiteral("kdeplot"));
	p->addChild(kdePlot);

	auto children = p->children<KDEPlot>();

	QCOMPARE(children.size(), 1);

	project.undoStack()->undo();
	children = p->children<KDEPlot>();
	QCOMPARE(children.size(), 0);

	project.undoStack()->redo();
	children = p->children<KDEPlot>();
	QCOMPARE(children.size(), 1);
}

/*!
 * \brief create and add a new KDEPlot, duplicate it and check the number of children
 */
void StatisticalPlotsTest::testKDEPlotDuplicate() {
	Project project;
	auto* ws = new Worksheet(QStringLiteral("worksheet"));
	project.addChild(ws);

	auto* p = new CartesianPlot(QStringLiteral("plot"));
	ws->addChild(p);

	auto* kdePlot = new KDEPlot(QStringLiteral("kdeplot"));
	p->addChild(kdePlot);

	kdePlot->duplicate();

	auto children = p->children<KDEPlot>();
	QCOMPARE(children.size(), 2);
}

/*!
 * \brief create a KDE plot for 3 values check the plot ranges.
 */
void StatisticalPlotsTest::testKDEPlotRange() {
	// prepare the data
	Spreadsheet sheet(QStringLiteral("test"), false);
	sheet.setColumnCount(1);
	sheet.setRowCount(100);
	auto* column = sheet.column(0);
	column->setValueAt(0, 2.);
	column->setValueAt(1, 4.);
	column->setValueAt(2, 6.);

	// prepare the worksheet + plot
	auto* ws = new Worksheet(QStringLiteral("worksheet"));
	auto* p = new CartesianPlot(QStringLiteral("plot"));
	ws->addChild(p);

	auto* kdePlot = new KDEPlot(QStringLiteral("kdeplot"));
	kdePlot->setKernelType(nsl_kernel_gauss);
	kdePlot->setBandwidthType(nsl_kde_bandwidth_custom);
	kdePlot->setBandwidth(0.3);
	p->addChild(kdePlot);
	kdePlot->setDataColumn(column);

	// validate with R via:
	// data <- c(2,4,6);
	// kd <- density(data,kernel="gaussian", bw=0.3)
	// plot(kd, col='blue', lwd=2)

	// check the x-range of the plot which should be [1, 7] (subtract/add 3 sigmas from/to min and max, respectively).
	const auto& rangeX = p->range(Dimension::X);
	QCOMPARE(rangeX.start(), 1);
	QCOMPARE(rangeX.end(), 7);

	// check the y-range of the plot which should be [0, 0.45]
	const auto& rangeY = p->range(Dimension::Y);
	QCOMPARE(rangeY.start(), 0.);
	QCOMPARE(rangeY.end(), 0.45);
}

// ##############################################################################
// ############################## Q-Q Plot ######################################
// ##############################################################################

/*!
 * \brief create and add a new QQPlot, undo and redo this step
 */
void StatisticalPlotsTest::testQQPlotInit() {
	Project project;
	auto* ws = new Worksheet(QStringLiteral("worksheet"));
	project.addChild(ws);

	auto* p = new CartesianPlot(QStringLiteral("plot"));
	ws->addChild(p);

	auto* qqPlot = new QQPlot(QStringLiteral("qqplot"));
	p->addChild(qqPlot);

	auto children = p->children<QQPlot>();

	QCOMPARE(children.size(), 1);

	project.undoStack()->undo();
	children = p->children<QQPlot>();
	QCOMPARE(children.size(), 0);

	project.undoStack()->redo();
	children = p->children<QQPlot>();
	QCOMPARE(children.size(), 1);
}

/*!
 * \brief create and add a new QQPlot, duplicate it and check the number of children
 */
void StatisticalPlotsTest::testQQPlotDuplicate() {
	Project project;
	auto* ws = new Worksheet(QStringLiteral("worksheet"));
	project.addChild(ws);

	auto* p = new CartesianPlot(QStringLiteral("plot"));
	ws->addChild(p);

	auto* qqPlot = new QQPlot(QStringLiteral("qqplot"));
	p->addChild(qqPlot);

	qqPlot->duplicate();

	auto children = p->children<QQPlot>();
	QCOMPARE(children.size(), 2);
}

/*!
 * \brief create QQPlot for 100 normaly distributed values and check the plot ranges.
 */
void StatisticalPlotsTest::testQQPlotRange() {
	// prepare the data
	Spreadsheet sheet(QStringLiteral("test"), false);
	sheet.setColumnCount(1);
	sheet.setRowCount(100);
	auto* column = sheet.column(0);

	// create a generator chosen by the environment variable GSL_RNG_TYPE
	gsl_rng_env_setup();
	const gsl_rng_type* T = gsl_rng_default;
	gsl_rng* r = gsl_rng_alloc(T);
	const double sigma = 1.;

	for (int i = 0; i < 100; ++i)
		column->setValueAt(i, gsl_ran_gaussian(r, sigma));

	// prepare the worksheet + plot
	auto* ws = new Worksheet(QStringLiteral("worksheet"));
	auto* p = new CartesianPlot(QStringLiteral("plot"));
	ws->addChild(p);

	auto* qqPlot = new QQPlot(QStringLiteral("qqplot"));
	p->addChild(qqPlot);
	qqPlot->setDataColumn(column);

	// check the x-range of the plot which should be [-2.5, 2.5] for the theoretical quantiles
	const auto& range = p->range(Dimension::X);
	QCOMPARE(range.start(), -2.5);
	QCOMPARE(range.end(), 2.5);
}

// ##############################################################################
// ############################## BAr Plot ######################################
// ##############################################################################

/*!
 * \brief create and add a new BarPlot, undo and redo this step
 */
void StatisticalPlotsTest::testBarPlotInit() {
	Project project;
	auto* ws = new Worksheet(QStringLiteral("worksheet"));
	project.addChild(ws);

	auto* p = new CartesianPlot(QStringLiteral("plot"));
	ws->addChild(p);

	auto* barPlot = new BarPlot(QStringLiteral("barplot"));
	p->addChild(barPlot);

	auto children = p->children<BarPlot>();

	QCOMPARE(children.size(), 1);

	project.undoStack()->undo();
	children = p->children<BarPlot>();
	QCOMPARE(children.size(), 0);

	project.undoStack()->redo();
	children = p->children<BarPlot>();
	QCOMPARE(children.size(), 1);
}

/*!
 * \brief create and add a new BarPlot, duplicate it and check the number of children
 */
void StatisticalPlotsTest::testBarPlotDuplicate() {
	Project project;
	auto* ws = new Worksheet(QStringLiteral("worksheet"));
	project.addChild(ws);

	auto* p = new CartesianPlot(QStringLiteral("plot"));
	ws->addChild(p);

	auto* barPlot = new BarPlot(QStringLiteral("barplot"));
	p->addChild(barPlot);

	barPlot->duplicate();

	auto children = p->children<BarPlot>();
	QCOMPARE(children.size(), 2);
}

/*!
 * \brief create BarPlot for the given data and check the plot ranges.
 */
void StatisticalPlotsTest::testBarPlotRange() {
	Project project;

	// prepare the data
	auto* sheet = new Spreadsheet(QStringLiteral("test"), false);
	project.addChild(sheet);
	sheet->setColumnCount(2);
	sheet->setRowCount(2);
	auto* column1 = sheet->column(0);
	auto* column2 = sheet->column(1);

	// create a generator chosen by the environment variable GSL_RNG_TYPE
	column1->setValueAt(0, 10);
	column1->setValueAt(1, 1);
	column2->setValueAt(0, 20);
	column2->setValueAt(1, 2);

	// prepare the worksheet + plot
	auto* ws = new Worksheet(QStringLiteral("worksheet"));
	auto* p = new CartesianPlot(QStringLiteral("plot"));
	ws->addChild(p);

	auto* barPlot = new BarPlot(QStringLiteral("barplot"));
	p->addChild(barPlot);
	barPlot->setDataColumns({column1, column2});

	// check the ranges which should be [0, 2] for x and [0, 20] for y
	const auto& rangeX = p->range(Dimension::X);
	QCOMPARE(rangeX.start(), 0);
	QCOMPARE(rangeX.end(), 2);

	const auto& rangeY = p->range(Dimension::Y);
	QCOMPARE(rangeY.start(), 0);
	QCOMPARE(rangeY.end(), 20);

	// remove the first row in the spreadsheet and check the ranges which should be [0, 1] for x and [0, 2] for y
	sheet->removeRows(0, 1);

	QCOMPARE(rangeX.start(), 0);
	QCOMPARE(rangeX.end(), 1);

	QCOMPARE(rangeY.start(), 0);
	QCOMPARE(rangeY.end(), 2);

	// undo the removal and check again
	project.undoStack()->undo();
	QCOMPARE(rangeX.start(), 0);
	QCOMPARE(rangeX.end(), 2);

	QCOMPARE(rangeY.start(), 0);
	QCOMPARE(rangeY.end(), 20);

	// mask the first row in the spreadsheet and check the ranges which should be [0, 1] for x and [0, 2] for y
	project.undoStack()->beginMacro(QStringLiteral("mask"));
	column1->setMasked(0);
	column2->setMasked(0);
	project.undoStack()->endMacro();

	QCOMPARE(rangeX.start(), 0);
	QCOMPARE(rangeX.end(), 1);

	QCOMPARE(rangeY.start(), 0);
	QCOMPARE(rangeY.end(), 2);

	// undo the masking and check again
	project.undoStack()->undo();
	QCOMPARE(rangeX.start(), 0);
	QCOMPARE(rangeX.end(), 2);

	QCOMPARE(rangeY.start(), 0);
	QCOMPARE(rangeY.end(), 20);
}

// ##############################################################################
// ################### Process Behavior Chart ###################################
// ##############################################################################
/*!
 * \brief create and add a new process behavior chart, undo and redo this step
 */
void StatisticalPlotsTest::testPBChartInit() {
	Project project;
	auto* ws = new Worksheet(QStringLiteral("worksheet"));
	project.addChild(ws);

	auto* p = new CartesianPlot(QStringLiteral("plot"));
	ws->addChild(p);

	auto* pbc = new ProcessBehaviorChart(QStringLiteral("pbc"));
	p->addChild(pbc);

	auto children = p->children<ProcessBehaviorChart>();

	QCOMPARE(children.size(), 1);

	project.undoStack()->undo();
	children = p->children<ProcessBehaviorChart>();
	QCOMPARE(children.size(), 0);

	project.undoStack()->redo();
	children = p->children<ProcessBehaviorChart>();
	QCOMPARE(children.size(), 1);
}

/*!
 * \brief create and add a new process behavior chart, duplicate it and check the number of children
 */
void StatisticalPlotsTest::testPBChartDuplicate() {
	Project project;
	auto* ws = new Worksheet(QStringLiteral("worksheet"));
	project.addChild(ws);

	auto* p = new CartesianPlot(QStringLiteral("plot"));
	ws->addChild(p);

	auto* pbc = new ProcessBehaviorChart(QStringLiteral("pbc"));
	p->addChild(pbc);

	pbc->duplicate();

	auto children = p->children<ProcessBehaviorChart>();
	QCOMPARE(children.size(), 2);
}

/*!
 * perform modification on the PBC leading to the resize and modifications of the internal
 * columns and curves and check the entries on the undo stack - the internal changes should
 * not be visible on the undo stack, only the operation that was triggered by the user.
 */
void StatisticalPlotsTest::testPBChartUndoRedo() {
	Project project;

	// prepare the data, same as in testPBChartXmRAverage()
	auto* column = new Column(QLatin1String("data"), AbstractColumn::ColumnMode::Integer);
	column->setIntegers({11, 4, 6, 4, 5, 7, 5, 4, 7, 12, 4, 2, 4, 5, 6, 4, 2, 2, 5, 9, 5, 6, 5, 9});

	// prepare the worksheet + plot
	auto* ws = new Worksheet(QStringLiteral("worksheet"));
	project.addChild(ws);

	auto* p = new CartesianPlot(QStringLiteral("plot"));
	ws->addChild(p);

	auto* pbc = new ProcessBehaviorChart(QStringLiteral("pbc"));
	pbc->setType(ProcessBehaviorChart::Type::XmR);
	p->addChild(pbc);

	// check the initial number of entries on the undo stack
	auto* stack = project.undoStack();
	QCOMPARE(stack->count(), 3);

	// change the sub-type which changes the number of points/samples on X
	pbc->setType(ProcessBehaviorChart::Type::S);
	QCOMPARE(stack->count(), 4);
	stack->undo();
	QCOMPARE(stack->count(), 4);

	// use an empty column as the data source which will empty all internal columns in pbc
	pbc->setDataColumn(new Column(QLatin1String("temp")));
	QCOMPARE(stack->count(), 4);
	stack->undo();
	QCOMPARE(stack->count(), 4);
}

/*!
 * test the X (XmR) chart using Average for the limits, the example is taken from Wheeler "Making Sense of Data", chapter seven.
 */
void StatisticalPlotsTest::testPBChartXmRAverage() {
	// prepare the data
	auto* column = new Column(QLatin1String("data"), AbstractColumn::ColumnMode::Integer);
	column->setIntegers({11, 4, 6, 4, 5, 7, 5, 4, 7, 12, 4, 2, 4, 5, 6, 4, 2, 2, 5, 9, 5, 6, 5, 9});

	// prepare the worksheet + plot
	auto* ws = new Worksheet(QStringLiteral("worksheet"));
	auto* p = new CartesianPlot(QStringLiteral("plot"));
	ws->addChild(p);

	auto* pbc = new ProcessBehaviorChart(QStringLiteral("pbc"));
	pbc->setType(ProcessBehaviorChart::Type::XmR);
	pbc->setLimitsMetric(ProcessBehaviorChart::LimitsMetric::Average);
	pbc->setMinLowerLimit(0.); // counts cannot become negative
	pbc->setDataColumn(column);
	p->addChild(pbc);

	// check the limits, two digit comparison with the values from the book
	QCOMPARE(std::round(pbc->center() * 100) / 100, 5.54);
	QCOMPARE(std::round(pbc->upperLimit() * 100) / 100, 12.48);
	QCOMPARE(pbc->lowerLimit(), 0);

	// check the plotted data ("statistics") - the original data is plotted
	const int rowCount = column->rowCount();
	auto* yColumn = pbc->dataCurve()->yColumn();
	QCOMPARE(yColumn, column);
	QCOMPARE(yColumn->rowCount(), rowCount);

	// index from 1 to 24 is used for x
	auto* xColumn = pbc->dataCurve()->xColumn();
	QCOMPARE(xColumn->rowCount(), rowCount);
	for (int i = 0; i < rowCount; ++i)
		QCOMPARE(xColumn->valueAt(i), i + 1);
}

/*!
 * test the mR (XmR) chart using Average for the limits, the example is taken from Wheeler "Making Sense of Data", chapter seven.
 */
void StatisticalPlotsTest::testPBChartmRAverage() {
	// prepare the data
	auto* column = new Column(QLatin1String("data"), AbstractColumn::ColumnMode::Integer);
	column->setIntegers({11, 4, 6, 4, 5, 7, 5, 4, 7, 12, 4, 2, 4, 5, 6, 4, 2, 2, 5, 9, 5, 6, 5, 9});

	// prepare the worksheet + plot
	auto* ws = new Worksheet(QStringLiteral("worksheet"));
	auto* p = new CartesianPlot(QStringLiteral("plot"));
	ws->addChild(p);

	auto* pbc = new ProcessBehaviorChart(QStringLiteral("pbc"));
	pbc->setType(ProcessBehaviorChart::Type::mR);
	pbc->setLimitsMetric(ProcessBehaviorChart::LimitsMetric::Average);
	pbc->setDataColumn(column);
	p->addChild(pbc);

	// check the limits, two digit comparison with the values from the book
	QCOMPARE(std::round(pbc->center() * 100) / 100, 2.61);
	QCOMPARE(std::round(pbc->upperLimit() * 100) / 100, 8.52); // book uses 3.27*2.61 \ approx 8.52, 3.26653*2.6087 \approx 8.52 is more precise
	QCOMPARE(pbc->lowerLimit(), 0);

	// check the plotted data ("statistics") - 23 moving ranges are plotted
	const int rowCount = 24; // total count 24, first value not available/used/plotted
	auto* yColumn = pbc->dataCurve()->yColumn();
	QCOMPARE(yColumn->rowCount(), rowCount);
	const QVector<double> ref = {7, 2, 2, 1, 2, 2, 1, 3, 5, 8, 2, 2, 1, 1, 2, 2, 0, 3, 4, 4, 1, 1, 4};
	for (int i = 0; i < rowCount - 1; ++i)
		QCOMPARE(yColumn->valueAt(i + 1), ref.at(i));

	// index from 1 to 24 is used for x
	auto* xColumn = pbc->dataCurve()->xColumn();
	QCOMPARE(xColumn->rowCount(), rowCount);
	for (int i = 0; i < rowCount; ++i)
		QCOMPARE(xColumn->valueAt(i), i + 1);
}

/*!
 * test the X (XmR) chart using Median for the limits, the example is taken from Wheeler "Making Sense of Data", chapter ten.
 */
void StatisticalPlotsTest::testPBChartXmRMedian() {
	// prepare the data
	auto* column = new Column(QLatin1String("data"), AbstractColumn::ColumnMode::Integer);
	// clang-format off
	column->setIntegers({260, 130, 189, 1080, 175, 200, 193, 120, 33, 293, 195, 571, 55698, 209, 1825, 239, 290, 254,
				93, 278, 185, 123, 9434, 408, 570, 118, 238, 207, 153, 209, 243, 110, 306, 343, 244});
	// clang-format on

	// prepare the worksheet + plot
	auto* ws = new Worksheet(QStringLiteral("worksheet"));
	auto* p = new CartesianPlot(QStringLiteral("plot"));
	ws->addChild(p);

	auto* pbc = new ProcessBehaviorChart(QStringLiteral("pbc"));
	pbc->setType(ProcessBehaviorChart::Type::XmR);
	pbc->setLimitsMetric(ProcessBehaviorChart::LimitsMetric::Median);
	pbc->setMinLowerLimit(0.); // counts cannot become negative
	pbc->setDataColumn(column);
	p->addChild(pbc);

	// check the limits, two digit comparison with the values from the book
	QCOMPARE(std::round(pbc->center() * 100) / 100, 238);
	QCOMPARE(std::round(pbc->upperLimit() * 100) / 100,
			 631.13); // book uses 630 for 238 + 3.14 * 125 = 630.5, 238 + 3.14507 * 125 \approx 631.13 is more precise
	QCOMPARE(pbc->lowerLimit(), 0);

	// check the plotted data ("statistics") - the original data is plotted
	const int rowCount = column->rowCount();
	auto* yColumn = pbc->dataCurve()->yColumn();
	QCOMPARE(yColumn, column);
	QCOMPARE(yColumn->rowCount(), rowCount);

	// index from 1 to 35 is used for x
	auto* xColumn = pbc->dataCurve()->xColumn();
	QCOMPARE(xColumn->rowCount(), rowCount);
	for (int i = 0; i < rowCount; ++i)
		QCOMPARE(xColumn->valueAt(i), i + 1);
}

/*!
 * test the mR (XmR) chart using Median for the limits, the example is taken from Wheeler "Making Sense of Data", chapter ten.
 */
void StatisticalPlotsTest::testPBChartmRMedian() {
	// prepare the data
	auto* column = new Column(QLatin1String("data"), AbstractColumn::ColumnMode::Integer);
	// clang-format off
	column->setIntegers({260, 130, 189, 1080, 175,	200, 193, 120, 33, 293, 195, 571, 55698, 209, 1825, 239, 290, 254,
				93, 278, 185, 123, 9434, 408, 570, 118, 238, 207, 153, 209, 243, 110, 306, 343, 244});
	// clang-format on

	// prepare the worksheet + plot
	auto* ws = new Worksheet(QStringLiteral("worksheet"));
	auto* p = new CartesianPlot(QStringLiteral("plot"));
	ws->addChild(p);

	auto* pbc = new ProcessBehaviorChart(QStringLiteral("pbc"));
	pbc->setType(ProcessBehaviorChart::Type::mR);
	pbc->setLimitsMetric(ProcessBehaviorChart::LimitsMetric::Median);
	pbc->setDataColumn(column);
	p->addChild(pbc);

	// check the limits, two digit comparison with the values from the book
	QCOMPARE(std::round(pbc->center() * 100) / 100, 125);
	QCOMPARE(std::round(pbc->upperLimit() * 100) / 100, 483.02); // book uses 482 for 3.86 * 125 = 482.5, 3.86413*125 \ approx 483.02 is more precise
	QCOMPARE(pbc->lowerLimit(), 0);

	// check the plotted data ("statistics") - 34 moving ranges are plotted
	const int rowCount = 35; // total count 35, first value not available/used/plotted
	auto* yColumn = pbc->dataCurve()->yColumn();
	QCOMPARE(yColumn->rowCount(), rowCount);
	// clang-format off
	const QVector<double> ref = {130, 59, 891, 905, 25, 7, 73, 87, 260, 98, 376, 55127, 55489, 1616, 1586, 51, 36,
					161, 185, 93, 62, 9311, 9026, 162, 452, 120, 31, 54, 56, 34, 133, 196, 37, 99};
	// clang-format on
	for (int i = 0; i < rowCount - 1; ++i)
		QCOMPARE(yColumn->valueAt(i + 1), ref.at(i));

	// index from 1 to 24 is used for x
	auto* xColumn = pbc->dataCurve()->xColumn();
	QCOMPARE(xColumn->rowCount(), rowCount);
	for (int i = 0; i < rowCount; ++i)
		QCOMPARE(xColumn->valueAt(i), i + 1);
}

/*!
 * test the XBar (XBarR) chart using Average for the limits, the example is taken from Wheeler "Making Sense of Data", chapter 16.
 */
void StatisticalPlotsTest::testPBChartXBarRAverage() {
	// prepare the data
	auto* column = new Column(QLatin1String("data"), AbstractColumn::ColumnMode::Integer);
	column->setIntegers({751, 754, 756, 754, 753, 757, 755, 756, 754, 756, 755, 757, 756, 752, 755, 755, 751, 756, 757, 753,
						 758, 753, 756, 754, 758, 754, 755, 755, 754, 752, 754, 758, 756, 757, 759, 752, 757, 755, 754, 756});

	// prepare the worksheet + plot
	auto* ws = new Worksheet(QStringLiteral("worksheet"));
	auto* p = new CartesianPlot(QStringLiteral("plot"));
	ws->addChild(p);

	auto* pbc = new ProcessBehaviorChart(QStringLiteral("pbc"));
	pbc->setType(ProcessBehaviorChart::Type::XbarR);
	pbc->setLimitsMetric(ProcessBehaviorChart::LimitsMetric::Average);
	pbc->setDataColumn(column);
	p->addChild(pbc);

	// check the limits, two digit comparison with the values from the book
	QCOMPARE(std::round(pbc->center() * 100) / 100, 755);
	QCOMPARE(std::round(pbc->upperLimit() * 100) / 100, 757.67);
	QCOMPARE(std::round(pbc->lowerLimit() * 100) / 100, 752.33);

	// check the plotted data ("statistics") - 8 mean values for every subgroup are plotted
	const int rowCount = 8;
	auto* yColumn = pbc->dataCurve()->yColumn();
	QCOMPARE(yColumn->rowCount(), rowCount);
	const QVector<double> ref = {753.6, 755.6, 755., 754.4, 755.8, 754., 756.8, 754.8};
	for (int i = 0; i < rowCount - 1; ++i)
		QCOMPARE(yColumn->valueAt(i), ref.at(i));

	// index from 1 to 8 is used for x
	auto* xColumn = pbc->dataCurve()->xColumn();
	QCOMPARE(xColumn->rowCount(), rowCount);
	for (int i = 0; i < rowCount; ++i)
		QCOMPARE(xColumn->valueAt(i), i + 1);
}

/*!
 * test the R chart using Average for the limits, the example is taken from Wheeler "Making Sense of Data", chapter 16.
 */
void StatisticalPlotsTest::testPBChartRAverage() {
	// prepare the data
	auto* column = new Column(QLatin1String("data"), AbstractColumn::ColumnMode::Integer);
	column->setIntegers({751, 754, 756, 754, 753, 757, 755, 756, 754, 756, 755, 757, 756, 752, 755, 755, 751, 756, 757, 753,
						 758, 753, 756, 754, 758, 754, 755, 755, 754, 752, 754, 758, 756, 757, 759, 752, 757, 755, 754, 756});

	// prepare the worksheet + plot
	auto* ws = new Worksheet(QStringLiteral("worksheet"));
	auto* p = new CartesianPlot(QStringLiteral("plot"));
	ws->addChild(p);

	auto* pbc = new ProcessBehaviorChart(QStringLiteral("pbc"));
	pbc->setType(ProcessBehaviorChart::Type::R);
	pbc->setLimitsMetric(ProcessBehaviorChart::LimitsMetric::Average);
	pbc->setDataColumn(column);
	p->addChild(pbc);

	// check the limits, two digit comparison with the values from the book
	QCOMPARE(std::round(pbc->center() * 100) / 100, 4.63);
	QCOMPARE(std::round(pbc->upperLimit() * 100) / 100, 9.78); // book uses 482 for 3.86 * 125 = 482.5, 3.86413*125 \ approx 483.02 is more precise
	QCOMPARE(pbc->lowerLimit(), 0);

	// check the plotted data ("statistics") - 8 ranges for every subgroup are plotted
	const int rowCount = 8;
	auto* yColumn = pbc->dataCurve()->yColumn();
	QCOMPARE(yColumn->rowCount(), rowCount);
	const QVector<double> ref = {5, 3, 5, 6, 5, 3, 5, 5};
	for (int i = 0; i < rowCount - 1; ++i)
		QCOMPARE(yColumn->valueAt(i), ref.at(i));

	// index from 1 to 8 is used for x
	auto* xColumn = pbc->dataCurve()->xColumn();
	QCOMPARE(xColumn->rowCount(), rowCount);
	for (int i = 0; i < rowCount; ++i)
		QCOMPARE(xColumn->valueAt(i), i + 1);
}

/*!
 * test the XBar (XBarS) chart, the example is taken from Montgomery "Statistical Quality Control", chapter 6.3.
 */
void StatisticalPlotsTest::testPBChartXBarS() {
	// prepare the data
	auto* column = new Column(QLatin1String("data"), AbstractColumn::ColumnMode::Double);
	column->setValues({74.03,  74.002, 74.019, 73.992, 74.008, 73.995, 73.992, 74.001, 74.011, 74.004, 73.988, 74.024, 74.021, 74.005, 74.002, 74.002,
					   73.996, 73.993, 74.015, 74.009, 73.992, 74.007, 74.015, 73.989, 74.014, 74.009, 73.994, 73.997, 73.985, 73.993, 73.995, 74.006,
					   73.994, 74,	   74.005, 73.985, 74.003, 73.993, 74.015, 73.988, 74.008, 73.995, 74.009, 74.005, 74.004, 73.998, 74,	   73.99,
					   74.007, 73.995, 73.994, 73.998, 73.994, 73.995, 73.99,  74.004, 74,	   74.007, 74,	   73.996, 73.983, 74.002, 73.998, 73.997,
					   74.012, 74.006, 73.967, 73.994, 74,	   73.984, 74.012, 74.014, 73.998, 73.999, 74.007, 74,	   73.984, 74.005, 73.998, 73.996,
					   73.994, 74.012, 73.986, 74.005, 74.007, 74.006, 74.01,  74.018, 74.003, 74,	   73.984, 74.002, 74.003, 74.005, 73.997, 74,
					   74.01,  74.013, 74.02,  74.003, 73.982, 74.001, 74.015, 74.005, 73.996, 74.004, 73.999, 73.99,  74.006, 74.009, 74.01,  73.989,
					   73.99,  74.009, 74.014, 74.015, 74.008, 73.993, 74,	   74.01,  73.982, 73.984, 73.995, 74.017, 74.013});

	// prepare the worksheet + plot
	auto* ws = new Worksheet(QStringLiteral("worksheet"));
	auto* p = new CartesianPlot(QStringLiteral("plot"));
	ws->addChild(p);

	auto* pbc = new ProcessBehaviorChart(QStringLiteral("pbc"));
	pbc->setType(ProcessBehaviorChart::Type::XbarS);
	pbc->setDataColumn(column);
	p->addChild(pbc);

	// check the limits, three digit comparison with the values from the book
	QCOMPARE(std::round(pbc->center() * 1000) / 1000, 74.001);
	QCOMPARE(std::round(pbc->upperLimit() * 1000) / 1000,
			 74.015); // book uses 74.001 + 1.427*0.0094 = 74.014, 74.0012 + 1.4273*0.00939948 \ approx = 74.015 is more precise
	QCOMPARE(std::round(pbc->lowerLimit() * 1000) / 1000, 73.988);

	// check the plotted data ("statistics") - mean values for every subgroup/sample are plotted
	const int rowCount = 25; // 25 samples
	auto* yColumn = pbc->dataCurve()->yColumn();
	QCOMPARE(yColumn->rowCount(), rowCount);
	const QVector<double> ref = {74.010, 74.001, 74.008, 74.003, 74.003, 73.996, 74.000, 73.997, 74.004, 73.998, 73.994, 74.001, 73.998,
								 73.990, 74.006, 73.997, 74.001, 74.007, 73.998, 74.009, 74.000, 74.002, 74.002, 74.005, 73.998};
	for (int i = 0; i < rowCount - 1; ++i)
		QCOMPARE(std::round(yColumn->valueAt(i) * 1000) / 1000, ref.at(i)); // compare three digits

	// index from 1 to 25 is used for x
	auto* xColumn = pbc->dataCurve()->xColumn();
	QCOMPARE(xColumn->rowCount(), rowCount);
	for (int i = 0; i < rowCount; ++i)
		QCOMPARE(xColumn->valueAt(i), i + 1);
}

/*!
 * test the S chart, the example is taken from Montgomery "Statistical Quality Control", chapter 6.3.
 */
void StatisticalPlotsTest::testPBChartS() {
	// prepare the data
	auto* column = new Column(QLatin1String("data"), AbstractColumn::ColumnMode::Double);
	column->setValues({74.03,  74.002, 74.019, 73.992, 74.008, 73.995, 73.992, 74.001, 74.011, 74.004, 73.988, 74.024, 74.021, 74.005, 74.002, 74.002,
					   73.996, 73.993, 74.015, 74.009, 73.992, 74.007, 74.015, 73.989, 74.014, 74.009, 73.994, 73.997, 73.985, 73.993, 73.995, 74.006,
					   73.994, 74,	   74.005, 73.985, 74.003, 73.993, 74.015, 73.988, 74.008, 73.995, 74.009, 74.005, 74.004, 73.998, 74,	   73.99,
					   74.007, 73.995, 73.994, 73.998, 73.994, 73.995, 73.99,  74.004, 74,	   74.007, 74,	   73.996, 73.983, 74.002, 73.998, 73.997,
					   74.012, 74.006, 73.967, 73.994, 74,	   73.984, 74.012, 74.014, 73.998, 73.999, 74.007, 74,	   73.984, 74.005, 73.998, 73.996,
					   73.994, 74.012, 73.986, 74.005, 74.007, 74.006, 74.01,  74.018, 74.003, 74,	   73.984, 74.002, 74.003, 74.005, 73.997, 74,
					   74.01,  74.013, 74.02,  74.003, 73.982, 74.001, 74.015, 74.005, 73.996, 74.004, 73.999, 73.99,  74.006, 74.009, 74.01,  73.989,
					   73.99,  74.009, 74.014, 74.015, 74.008, 73.993, 74,	   74.01,  73.982, 73.984, 73.995, 74.017, 74.013});

	// prepare the worksheet + plot
	auto* ws = new Worksheet(QStringLiteral("worksheet"));
	auto* p = new CartesianPlot(QStringLiteral("plot"));
	ws->addChild(p);

	auto* pbc = new ProcessBehaviorChart(QStringLiteral("pbc"));
	pbc->setType(ProcessBehaviorChart::Type::S);
	pbc->setDataColumn(column);
	p->addChild(pbc);

	// check the limits, four digit comparison with the values from the book
	QCOMPARE(std::round(pbc->center() * 10000) / 10000, 0.0094);
	QCOMPARE(std::round(pbc->upperLimit() * 10000) / 10000, 0.0196);
	QCOMPARE(pbc->lowerLimit(), 0.);

	// check the plotted data ("statistics") - standard deviations for every subgroup/sample are plotted
	const int rowCount = 25; // 25 samples
	auto* yColumn = pbc->dataCurve()->yColumn();
	QCOMPARE(yColumn->rowCount(), rowCount);
	const QVector<double> ref = {0.0148, 0.0075, 0.0147, 0.0091, 0.0122, 0.0087, 0.0055, 0.0123, 0.0055, 0.0063, 0.0029, 0.0042, 0.0105,
								 0.0153, 0.0073, 0.0078, 0.0106, 0.0070, 0.0085, 0.0080, 0.0122, 0.0074, 0.0119, 0.0087, 0.0162};
	for (int i = 0; i < rowCount - 1; ++i)
		QCOMPARE(std::round(yColumn->valueAt(i) * 10000) / 10000, ref.at(i)); // compare four digits

	// index from 1 to 25 is used for x
	auto* xColumn = pbc->dataCurve()->xColumn();
	QCOMPARE(xColumn->rowCount(), rowCount);
	for (int i = 0; i < rowCount; ++i)
		QCOMPARE(xColumn->valueAt(i), i + 1);
}

/*!
 * test the NP chart, the example is taken from Wheeler "Making Sense of Data", chapter 14.
 */
void StatisticalPlotsTest::testPBChartNP() {
	// prepare the data
	auto* column = new Column(QLatin1String("data"), AbstractColumn::ColumnMode::Integer);
	// clang-format off
	column->setIntegers({27, 19, 18, 16, 16, 12, 15, 13, 16, 16, 9, 17, 21, 15, 39, 21, 14, 23, 19, 29, 30});
	// clang-format on

	// prepare the worksheet + plot
	auto* ws = new Worksheet(QStringLiteral("worksheet"));
	auto* p = new CartesianPlot(QStringLiteral("plot"));
	ws->addChild(p);

	auto* pbc = new ProcessBehaviorChart(QStringLiteral("pbc"));
	pbc->setType(ProcessBehaviorChart::Type::NP);
	pbc->setSampleSize(100);
	pbc->setDataColumn(column);
	p->addChild(pbc);

	// check the limits, two digit comparison with the values from the book
	QCOMPARE(std::round(pbc->center() * 100) / 100, 19.29);
	QCOMPARE(std::round(pbc->upperLimit() * 100) / 100, 31.12);
	QCOMPARE(std::round(pbc->lowerLimit() * 100) / 100, 7.45);

	// check the plotted data ("statistics") - the original data is plotted
	const int rowCount = column->rowCount();
	auto* yColumn = pbc->dataCurve()->yColumn();
	QCOMPARE(yColumn, column);
	QCOMPARE(yColumn->rowCount(), rowCount);

	// index from 1 to 21 is used for x
	auto* xColumn = pbc->dataCurve()->xColumn();
	QCOMPARE(xColumn->rowCount(), rowCount);
	for (int i = 0; i < rowCount; ++i)
		QCOMPARE(xColumn->valueAt(i), i + 1);
}

/*!
 * test the P chart, the example is taken from Wheeler "Making Sense of Data", chapter 14.
 */
void StatisticalPlotsTest::testPBChartP() {
	// prepare the data
	auto* column = new Column(QLatin1String("data"), AbstractColumn::ColumnMode::Integer);
	auto* column2 = new Column(QLatin1String("data2"), AbstractColumn::ColumnMode::Integer);
	// clang-format off
	column->setIntegers({27, 31, 70, 54, 69, 101, 28, 37, 47, 46, 70, 105, 19, 33, 68, 44, 74, 124, 21, 32, 65, 46, 75, 117});
	column2->setIntegers({102, 146, 280, 207, 322, 410, 99, 143, 235, 185, 271, 469, 101, 140, 292, 207, 257, 511, 94, 139, 229, 170, 290, 407});
	// clang-format on

	// prepare the worksheet + plot
	auto* ws = new Worksheet(QStringLiteral("worksheet"));
	auto* p = new CartesianPlot(QStringLiteral("plot"));
	ws->addChild(p);

	auto* pbc = new ProcessBehaviorChart(QStringLiteral("pbc"));
	pbc->setType(ProcessBehaviorChart::Type::P);
	pbc->setExactLimitsEnabled(false);
	pbc->setDataColumn(column);
	pbc->setData2Column(column2);
	p->addChild(pbc);

	// check the limits, two digit comparison with the values from the book
	QCOMPARE(std::round(pbc->center() * 100) / 100, 0.25);
	QCOMPARE(std::round(pbc->upperLimit() * 100) / 100, 0.33);
	QCOMPARE(std::round(pbc->lowerLimit() * 100) / 100, 0.16);

	// check the plotted data ("statistics") - proportions are plotted
	const int rowCount = 24; // 24 values
	auto* yColumn = pbc->dataCurve()->yColumn();
	QCOMPARE(yColumn->rowCount(), rowCount);
	const QVector<double> ref = {0.265, 0.212, 0.250, 0.261, 0.214, 0.246, 0.283, 0.259, 0.200, 0.249, 0.258, 0.224,
								 0.188, 0.236, 0.233, 0.213, 0.288, 0.243, 0.223, 0.230, 0.284, 0.271, 0.259, 0.287};
	for (int i = 0; i < rowCount - 1; ++i)
		QCOMPARE(std::round(yColumn->valueAt(i) * 1000) / 1000, ref.at(i)); // compare three digits

	// index from 1 to 24 is used for x
	auto* xColumn = pbc->dataCurve()->xColumn();
	QCOMPARE(xColumn->rowCount(), rowCount);
	for (int i = 0; i < rowCount; ++i)
		QCOMPARE(xColumn->valueAt(i), i + 1);
}

/*!
 * test the C chart, the example is taken from Wheeler "Making Sense of Data", chapter 14.
 */
void StatisticalPlotsTest::testPBChartC() {
	// prepare the data
	auto* column = new Column(QLatin1String("data"), AbstractColumn::ColumnMode::Integer);
	// clang-format off
	column->setIntegers({0, 2, 3, 2, 0, 1, 3, 1, 1, 0, 1, 4, 0, 2, 0, 1, 4, 2, 0, 1, 3, 3, 2, 0, 1, 0, 1, 3, 1, 2, 5});
	// clang-format on

	// prepare the worksheet + plot
	auto* ws = new Worksheet(QStringLiteral("worksheet"));
	auto* p = new CartesianPlot(QStringLiteral("plot"));
	ws->addChild(p);

	auto* pbc = new ProcessBehaviorChart(QStringLiteral("pbc"));
	pbc->setType(ProcessBehaviorChart::Type::C);
	pbc->setDataColumn(column);
	p->addChild(pbc);

	// check the limits, two digit comparison with the values from the book
	QCOMPARE(std::round(pbc->center() * 100) / 100, 1.58);
	QCOMPARE(std::round(pbc->upperLimit() * 100) / 100, 5.35);
	QCOMPARE(std::round(pbc->lowerLimit() * 100) / 100, 0.);

	// check the plotted data ("statistics") - the original data is plotted
	const int rowCount = column->rowCount();
	auto* yColumn = pbc->dataCurve()->yColumn();
	QCOMPARE(yColumn, column);
	QCOMPARE(yColumn->rowCount(), rowCount);

	// index from 1 to 31 is used for x
	auto* xColumn = pbc->dataCurve()->xColumn();
	QCOMPARE(xColumn->rowCount(), rowCount);
	for (int i = 0; i < rowCount; ++i)
		QCOMPARE(xColumn->valueAt(i), i + 1);
}

/*!
 * test the U chart, the example is taken from Wheeler "Making Sense of Data", chapter 14.
 */
void StatisticalPlotsTest::testPBChartU() {
	// prepare the data
	auto* column = new Column(QLatin1String("data"), AbstractColumn::ColumnMode::Integer);
	auto* column2 = new Column(QLatin1String("data2"), AbstractColumn::ColumnMode::Double);
	// clang-format off
	column->setIntegers({656, 620, 681, 681, 660, 731, 694, 695, 683, 729, 715, 743, 762, 735, 780, 737, 770, 727, 784, 839, 779, 853, 804, 832});
	column2->setValues({2.86, 2.72, 2.97, 3.00, 3.10, 3.13, 3.37, 3.38, 3.36, 3.41, 3.48, 3.66, 3.59, 3.37, 3.83, 3.67, 3.92, 3.77, 4.01, 4.12, 3.98, 4.18, 4.14, 4.36});
	// clang-format on

	// prepare the worksheet + plot
	auto* ws = new Worksheet(QStringLiteral("worksheet"));
	auto* p = new CartesianPlot(QStringLiteral("plot"));
	ws->addChild(p);

	auto* pbc = new ProcessBehaviorChart(QStringLiteral("pbc"));
	pbc->setType(ProcessBehaviorChart::Type::U);
	pbc->setExactLimitsEnabled(false);
	pbc->setDataColumn(column);
	pbc->setData2Column(column2);
	p->addChild(pbc);

	// check the limits, one digit comparison with the value for the center taken from the book,
	// approximated upper and lower limits are "self-calculated"
	QCOMPARE(std::round(pbc->center() * 10) / 10, 207.2);
	QCOMPARE(std::round(pbc->upperLimit() * 10) / 10, 230.1);
	QCOMPARE(std::round(pbc->lowerLimit() * 10) / 10, 184.3);

	// check the plotted data ("statistics") - rates are plotted
	const int rowCount = 24; // 24 values
	auto* yColumn = pbc->dataCurve()->yColumn();
	QCOMPARE(yColumn->rowCount(), rowCount);
	// clang-format off
	const QVector<int> ref = {229, 228, 229, 227, 213, 234, 206, 206, 203, 214, 205, 203,
							212, 218, 204 /* Wheeler uses 203 for 203.5 */, 201, 196 /* 770/3.92 \approx 196 */, 193,
							196, 204 /* Wheeler uses 203 for 203.5 */, 196, 204, 194, 191};
	// clang-format on
	for (int i = 0; i < rowCount - 1; ++i)
		QCOMPARE(std::round(yColumn->valueAt(i)), ref.at(i)); // compare zero digits

	// index from 1 to 24 is used for x
	auto* xColumn = pbc->dataCurve()->xColumn();
	QCOMPARE(xColumn->rowCount(), rowCount);
	for (int i = 0; i < rowCount; ++i)
		QCOMPARE(xColumn->valueAt(i), i + 1);
}

// ##############################################################################
// ############################ Run Chart #######################################
// ##############################################################################
/*!
 * \brief create and add a new run chart, undo and redo this step
 */
void StatisticalPlotsTest::testRunChartInit() {
	Project project;
	auto* ws = new Worksheet(QStringLiteral("worksheet"));
	project.addChild(ws);

	auto* p = new CartesianPlot(QStringLiteral("plot"));
	ws->addChild(p);

	auto* pbc = new RunChart(QStringLiteral("chart"));
	p->addChild(pbc);

	auto children = p->children<RunChart>();

	QCOMPARE(children.size(), 1);

	project.undoStack()->undo();
	children = p->children<RunChart>();
	QCOMPARE(children.size(), 0);

	project.undoStack()->redo();
	children = p->children<RunChart>();
	QCOMPARE(children.size(), 1);
}

/*!
 * \brief create and add a new run chart, duplicate it and check the number of children
 */
void StatisticalPlotsTest::testRunChartDuplicate() {
	Project project;
	auto* ws = new Worksheet(QStringLiteral("worksheet"));
	project.addChild(ws);

	auto* p = new CartesianPlot(QStringLiteral("plot"));
	ws->addChild(p);

	auto* pbc = new RunChart(QStringLiteral("chart"));
	p->addChild(pbc);

	pbc->duplicate();

	auto children = p->children<RunChart>();
	QCOMPARE(children.size(), 2);
}

/*!
 * test the run chart using Average for the center line, data from from Wheeler "Making Sense of Data", chapter seven.
 */
void StatisticalPlotsTest::testRunChartCenterAverage() {
	// prepare the data
	auto* column = new Column(QLatin1String("data"), AbstractColumn::ColumnMode::Integer);
	column->setIntegers({11, 4, 6, 4, 5, 7, 5, 4, 7, 12, 4, 2, 4, 5, 6, 4, 2, 2, 5, 9, 5, 6, 5, 9});

	// prepare the worksheet + plot
	auto* ws = new Worksheet(QStringLiteral("worksheet"));
	auto* p = new CartesianPlot(QStringLiteral("plot"));
	ws->addChild(p);

	auto* chart = new RunChart(QStringLiteral("chart"));
	chart->setDataColumn(column);
	chart->setCenterMetric(RunChart::CenterMetric::Average);
	p->addChild(chart);

	// check the limits, two digit comparison with the values from the book
	QCOMPARE(std::round(chart->center() * 100) / 100, 5.54);

	// index from 1 to 24 is used for x
	const int rowCount = 24;
	auto* xColumn = chart->dataCurve()->xColumn();
	QCOMPARE(xColumn->rowCount(), rowCount);
	for (int i = 0; i < rowCount; ++i)
		QCOMPARE(xColumn->valueAt(i), i + 1);
}

/*!
 * test the run chart using Median for the center line, data from from Wheeler "Making Sense of Data", chapter seven.
 */
void StatisticalPlotsTest::testRunChartCenterMedian() {
	// prepare the data
	auto* column = new Column(QLatin1String("data"), AbstractColumn::ColumnMode::Integer);
	column->setIntegers({11, 4, 6, 4, 5, 7, 5, 4, 7, 12, 4, 2, 4, 5, 6, 4, 2, 2, 5, 9, 5, 6, 5, 9});

	// prepare the worksheet + plot
	auto* ws = new Worksheet(QStringLiteral("worksheet"));
	auto* p = new CartesianPlot(QStringLiteral("plot"));
	ws->addChild(p);

	auto* chart = new RunChart(QStringLiteral("chart"));
	chart->setDataColumn(column);
	chart->setCenterMetric(RunChart::CenterMetric::Median);
	p->addChild(chart);

	// check the limits, two digit comparison with the values from the book
	QCOMPARE(std::round(chart->center() * 100) / 100, 5.0);

	// index from 1 to 24 is used for x
	const int rowCount = 24;
	auto* xColumn = chart->dataCurve()->xColumn();
	QCOMPARE(xColumn->rowCount(), rowCount);
	for (int i = 0; i < rowCount; ++i)
		QCOMPARE(xColumn->valueAt(i), i + 1);
}

QTEST_MAIN(StatisticalPlotsTest)
