/*
	File                 : ProjectImportTest.cpp
	Project              : LabPlot
	Description          : Tests for project imports
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2018-2023 Alexander Semke <alexander.semke@web.de>
	SPDX-FileCopyrightText: 2022 Stefan Gerlach <stefan.gerlach@uni.kn>

	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "ProjectImportTest.h"
#ifdef HAVE_LIBORIGIN
#include "backend/datasources/projects/OriginProjectParser.h"
#endif
#include "backend/core/Project.h"
#include "backend/core/Workbook.h"
#include "backend/matrix/Matrix.h"
#include "backend/spreadsheet/Spreadsheet.h"
#include "backend/worksheet/Line.h"
#include "backend/worksheet/TextLabel.h"
#include "backend/worksheet/Worksheet.h"
#include "backend/worksheet/plots/cartesian/BarPlot.h"
#include "backend/worksheet/plots/cartesian/CartesianPlot.h"
#include "backend/worksheet/plots/cartesian/CartesianPlotLegend.h"
#include "backend/worksheet/plots/cartesian/Symbol.h"
#include "backend/worksheet/plots/cartesian/XYCurve.h"
#include "backend/worksheet/plots/cartesian/XYEquationCurve.h"

// ##############################################################################
// #####################  import of LabPlot projects ############################
// ##############################################################################

// TODO

#ifdef HAVE_LIBORIGIN
// ##############################################################################
// ######################  import of Origin projects ############################
// ##############################################################################
//  project tree of the file "origin8_test_tree_import.opj"
/*
test_tree_import\
					\Book3
					\Folder
						\Book2
							\Sheet1
							\Sheet2
						\MBook2
							\MSheet1
							\MSheet2
					\Folder1
						\MBook1
							\Sheet1
						\Book1
							\Sheet1
						\Book4
							\MSheet1
						\Graph2
					\Excel1
*/

void ProjectImportTest::testOrigin01() {
	// import the opj file into LabPlot's project object
	OriginProjectParser parser;
	parser.setProjectFileName(QFINDTESTDATA(QLatin1String("data/origin8_test_tree_import.opj")));
	Project project;
	parser.importTo(&project, QStringList());

	// check the project tree for the imported project

	// first child of the root folder, spreadsheet "Book3"
	auto* aspect = project.child<AbstractAspect>(0);
	QCOMPARE(aspect != nullptr, true);
	if (aspect != nullptr)
		QCOMPARE(aspect->name(), QLatin1String("Book3"));
	QCOMPARE(dynamic_cast<Spreadsheet*>(aspect) != nullptr, true);

	// first child of the root folder, folder "Folder" -> import into a Folder
	aspect = project.child<AbstractAspect>(1);
	QCOMPARE(aspect != nullptr, true);
	if (aspect != nullptr)
		QCOMPARE(aspect->name(), QLatin1String("Folder"));
	QCOMPARE(dynamic_cast<Folder*>(aspect) != nullptr, true);

	// first child of "Folder", workbook "Book2" with two sheets -> import into a Workbook with two Spreadsheets
	aspect = project.child<AbstractAspect>(1)->child<AbstractAspect>(0);
	QCOMPARE(aspect != nullptr, true);
	if (aspect != nullptr)
		QCOMPARE(aspect->name(), QLatin1String("Book2"));
	QCOMPARE(dynamic_cast<Workbook*>(aspect) != nullptr, true);

	aspect = project.child<AbstractAspect>(1)->child<AbstractAspect>(0)->child<AbstractAspect>(0);
	QCOMPARE(aspect != nullptr, true);
	if (aspect != nullptr)
		QCOMPARE(aspect->name(), QLatin1String("Sheet1"));
	QCOMPARE(dynamic_cast<Spreadsheet*>(aspect) != nullptr, true);

	aspect = project.child<AbstractAspect>(1)->child<AbstractAspect>(0)->child<AbstractAspect>(1);
	QCOMPARE(aspect != nullptr, true);
	if (aspect != nullptr)
		QCOMPARE(aspect->name(), QLatin1String("Sheet2"));
	QCOMPARE(dynamic_cast<Spreadsheet*>(aspect) != nullptr, true);

	// second child of "Folder", matrixbook "MBook" with two matrix sheets -> import into a Workbook with two Matrices
	aspect = project.child<AbstractAspect>(1)->child<AbstractAspect>(1);
	QCOMPARE(aspect != nullptr, true);
	if (aspect != nullptr)
		QCOMPARE(aspect->name(), QLatin1String("MBook2"));
	QCOMPARE(dynamic_cast<Workbook*>(aspect) != nullptr, true);

	aspect = project.child<AbstractAspect>(1)->child<AbstractAspect>(1)->child<AbstractAspect>(0);
	QCOMPARE(aspect != nullptr, true);
	if (aspect != nullptr)
		QCOMPARE(aspect->name(), QLatin1String("MSheet1"));
	QCOMPARE(dynamic_cast<Matrix*>(aspect) != nullptr, true);

	aspect = project.child<AbstractAspect>(1)->child<AbstractAspect>(1)->child<AbstractAspect>(1);
	QCOMPARE(aspect != nullptr, true);
	if (aspect != nullptr)
		QCOMPARE(aspect->name(), QLatin1String("MSheet2"));
	QCOMPARE(dynamic_cast<Matrix*>(aspect) != nullptr, true);

	// second child of the root folder, folder "Folder1" -> import into a Folder
	aspect = project.child<AbstractAspect>(2);
	QCOMPARE(aspect != nullptr, true);
	if (aspect != nullptr)
		QCOMPARE(aspect->name(), QLatin1String("Folder1"));
	QCOMPARE(dynamic_cast<Folder*>(aspect) != nullptr, true);

	// first child of "Folder1", matrix "MBook1"
	aspect = project.child<AbstractAspect>(2)->child<AbstractAspect>(0);
	QCOMPARE(aspect != nullptr, true);
	if (aspect != nullptr)
		QCOMPARE(aspect->name(), QLatin1String("MBook1"));
	QCOMPARE(dynamic_cast<Matrix*>(aspect) != nullptr, true);

	// second child of "Folder1", workbook "Book1" with on sheet -> import into a Spreadsheet
	aspect = project.child<AbstractAspect>(2)->child<AbstractAspect>(1);
	QCOMPARE(aspect != nullptr, true);
	if (aspect != nullptr)
		QCOMPARE(aspect->name(), QLatin1String("Book1"));
	QCOMPARE(dynamic_cast<Spreadsheet*>(aspect) != nullptr, true);

	// second child of "Folder1", workbook "Book4" with on sheet -> import into a Spreadsheet
	aspect = project.child<AbstractAspect>(2)->child<AbstractAspect>(2);
	QCOMPARE(aspect != nullptr, true);
	if (aspect != nullptr)
		QCOMPARE(aspect->name(), QLatin1String("Book4"));
	QCOMPARE(dynamic_cast<Spreadsheet*>(aspect) != nullptr, true);

	// third child of "Folder1", graph "Graph"-> import into a Worksheet
	aspect = project.child<AbstractAspect>(2)->child<AbstractAspect>(3);
	QCOMPARE(aspect != nullptr, true);
	if (aspect != nullptr)
		QCOMPARE(aspect->name(), QLatin1String("Graph2"));
	QCOMPARE(dynamic_cast<Worksheet*>(aspect) != nullptr, true);
	// TODO: check the created plot in the worksheet

	// TODO: loose window: spreadsheet "Excel1"
	// aspect = project.child<AbstractAspect>(3);
	// QCOMPARE(aspect != nullptr, true);
	// QCOMPARE(aspect->name(), QLatin1String("Excel1"));
	// QCOMPARE(dynamic_cast<Spreadsheet*>(aspect) != nullptr, true);
}

/*
 * import one single folder child
 */
void ProjectImportTest::testOrigin02() {
	// import one single object
	OriginProjectParser parser;
	parser.setProjectFileName(QFINDTESTDATA(QLatin1String("data/origin8_test_tree_import.opj")));
	Project project;
	QStringList selectedPathes = {QLatin1String("test_tree_import/Folder1/Book1"),
								  QLatin1String("test_tree_import/Folder1"),
								  QLatin1String("test_tree_import")};
	parser.importTo(&project, selectedPathes);

	// check the project tree for the imported project

	// first child of the root folder, folder "Folder1" -> import into a Folder
	auto* aspect = project.child<AbstractAspect>(0);
	QCOMPARE(aspect != nullptr, true);
	if (aspect != nullptr)
		QCOMPARE(aspect->name(), QLatin1String("Folder1"));
	QCOMPARE(dynamic_cast<Folder*>(aspect) != nullptr, true);
	if (aspect != nullptr)
		QCOMPARE(aspect->childCount<AbstractAspect>(), 1);

	// first child of "Folder", workbook "Book" with one sheet -> import into a Spreadsheet
	aspect = project.child<AbstractAspect>(0)->child<AbstractAspect>(0);
	QCOMPARE(aspect != nullptr, true);
	if (aspect != nullptr)
		QCOMPARE(aspect->name(), QLatin1String("Book1"));
	QCOMPARE(dynamic_cast<Spreadsheet*>(aspect) != nullptr, true);
}

/*
 * 1. import one single folder child
 * 2. import another folder child
 * 3. check that both children are available after the second import
 */
void ProjectImportTest::testOrigin03() {
	// import one single object
	OriginProjectParser parser;
	parser.setProjectFileName(QFINDTESTDATA(QLatin1String("data/origin8_test_tree_import.opj")));
	Project project;

	// import one single child in "Folder1"
	QStringList selectedPathes = {QLatin1String("test_tree_import/Folder1/Book1"),
								  QLatin1String("test_tree_import/Folder1"),
								  QLatin1String("test_tree_import")};
	parser.importTo(&project, selectedPathes);

	// first child of the root folder, folder "Folder1" -> import into a Folder
	auto* aspect = project.child<AbstractAspect>(0);
	QCOMPARE(aspect != nullptr, true);
	if (aspect != nullptr)
		QCOMPARE(aspect->name(), QLatin1String("Folder1"));
	QCOMPARE(dynamic_cast<Folder*>(aspect) != nullptr, true);
	if (aspect != nullptr)
		QCOMPARE(aspect->childCount<AbstractAspect>(), 1);

	// first child of "Folder", workbook "Book1" with one sheet -> import into a Spreadsheet
	aspect = project.child<AbstractAspect>(0)->child<AbstractAspect>(0);
	QCOMPARE(aspect != nullptr, true);
	if (aspect != nullptr)
		QCOMPARE(aspect->name(), QLatin1String("Book1"));
	QCOMPARE(dynamic_cast<Spreadsheet*>(aspect) != nullptr, true);

	// import another child in "Folder1"
	selectedPathes.clear();
	selectedPathes << QLatin1String("test_tree_import/Folder1/Book4") << QLatin1String("test_tree_import/Folder1") << QLatin1String("test_tree_import");
	parser.importTo(&project, selectedPathes);

	// the first child should still be available in the project -> check it
	aspect = project.child<AbstractAspect>(0);
	QCOMPARE(aspect != nullptr, true);
	if (aspect != nullptr)
		QCOMPARE(aspect->name(), QLatin1String("Folder1"));
	QCOMPARE(dynamic_cast<Folder*>(aspect) != nullptr, true);

	aspect = project.child<AbstractAspect>(0)->child<AbstractAspect>(0);
	QCOMPARE(aspect != nullptr, true);
	if (aspect != nullptr)
		QCOMPARE(aspect->name(), QLatin1String("Book1"));
	QCOMPARE(dynamic_cast<Spreadsheet*>(aspect) != nullptr, true);

	// check the second child, workbook "Book4" with one sheet -> import into a Spreadsheet
	aspect = project.child<AbstractAspect>(0)->child<AbstractAspect>(1);
	QCOMPARE(aspect != nullptr, true);
	if (aspect != nullptr)
		QCOMPARE(aspect->name(), QLatin1String("Book4"));
	QCOMPARE(dynamic_cast<Spreadsheet*>(aspect) != nullptr, true);

	if (aspect != nullptr)
		QCOMPARE(aspect->childCount<AbstractAspect>(), 2);
}

/*
 * 1. import a spreadsheet
 * 2. modify a cell in it
 * 3. import the same spreadsheet once more
 * 4. check that the changes were really over-written
 */
void ProjectImportTest::testOrigin04() {
	OriginProjectParser parser;
	parser.setProjectFileName(QFINDTESTDATA(QLatin1String("data/origin8_test_tree_import.opj")));
	Project project;

	// import "Book1"
	QStringList selectedPathes = {QLatin1String("test_tree_import/Folder1/Book1"),
								  QLatin1String("test_tree_import/Folder1"),
								  QLatin1String("test_tree_import")};
	parser.importTo(&project, selectedPathes);

	// first child of folder "Folder1", workbook "Book1" with one sheet -> import into a spreadsheet
	auto* aspect = project.child<AbstractAspect>(0);
	QCOMPARE(aspect != nullptr, true);
	if (aspect != nullptr)
		QCOMPARE(aspect->name(), QLatin1String("Folder1"));
	aspect = project.child<AbstractAspect>(0)->child<AbstractAspect>(0);
	QCOMPARE(aspect != nullptr, true);
	if (aspect != nullptr)
		QCOMPARE(aspect->name(), QLatin1String("Book1"));
	auto* spreadsheet = dynamic_cast<Spreadsheet*>(aspect);
	QCOMPARE(spreadsheet != nullptr, true);

	// the (0,0)-cell has the value 1.0
	if (spreadsheet != nullptr) {
		QCOMPARE(spreadsheet->column(0)->valueAt(0), 1.0);

		// set the value to 5.0
		spreadsheet->column(0)->setValueAt(0, 5.0);
		QCOMPARE(spreadsheet->column(0)->valueAt(0), 5.0);
	}

	// re-import
	parser.importTo(&project, selectedPathes);

	// check the folder structure and the value of the (0,0)-cell again
	aspect = project.child<AbstractAspect>(0)->child<AbstractAspect>(0);
	QCOMPARE(aspect != nullptr, true);
	if (aspect != nullptr)
		QCOMPARE(aspect->name(), QLatin1String("Book1"));
	spreadsheet = dynamic_cast<Spreadsheet*>(aspect);
	QCOMPARE(spreadsheet != nullptr, true);
	if (spreadsheet != nullptr)
		QCOMPARE(spreadsheet->column(0)->valueAt(0), 1.0);
}

void ProjectImportTest::testOriginTextNumericColumns() {
	OriginProjectParser parser;
	parser.setProjectFileName(QFINDTESTDATA(QLatin1String("data/origin8_test_workbook.opj")));
	Project project;

	// import "Book1"
	QStringList selectedPathes = {QLatin1String("origin8_test_workbook/Folder1/Book1"),
								  QLatin1String("origin8_test_workbook/Folder1"),
								  QLatin1String("origin8_test_workbook")};
	parser.importTo(&project, selectedPathes);

	// first child of folder "Folder1", workbook "Book1" with one sheet -> import into a spreadsheet
	auto* aspect = project.child<AbstractAspect>(0);
	QCOMPARE(aspect != nullptr, true);
	if (aspect != nullptr)
		QCOMPARE(aspect->name(), QLatin1String("Folder1"));
	aspect = project.child<AbstractAspect>(0)->child<AbstractAspect>(0);
	QCOMPARE(aspect != nullptr, true);
	if (aspect != nullptr)
		QCOMPARE(aspect->name(), QLatin1String("Book1"));
	auto* spreadsheet = dynamic_cast<Spreadsheet*>(aspect);
	QCOMPARE(spreadsheet != nullptr, true);

	// additional pointer check for static code analysis tools like Coverity that are not aware of QCOMPARE above
	if (!spreadsheet)
		return;

	// check the values in the imported columns
	QCOMPARE(spreadsheet->columnCount(), 6);

	// 1st column, Origin::TextNumeric:
	// first non-empty value is numerical, column is set to Numeric, empty or text values in the column a set to NAN
	Column* column = spreadsheet->column(0);
	QCOMPARE(column->columnMode(), AbstractColumn::ColumnMode::Double);
	QCOMPARE(!std::isnan(column->valueAt(0)), false);
	QCOMPARE(column->valueAt(1), 1.1);
	QCOMPARE(column->valueAt(2), 2.2);
	QCOMPARE(!std::isnan(column->valueAt(3)), false);
	QCOMPARE(!std::isnan(column->valueAt(4)), false);

	// 2nd column, Origin::TextNumeric:
	// first non-empty value is string, the column is set to Text, numerical values are converted to strings
	column = spreadsheet->column(1);
	QCOMPARE(column->columnMode(), AbstractColumn::ColumnMode::Text);
	QCOMPARE(column->textAt(0).isEmpty(), true);
	QCOMPARE(column->textAt(1), QLatin1String("a"));
	QCOMPARE(column->textAt(2), QLatin1String("b"));
	QCOMPARE(column->textAt(3), QLatin1String("1.1"));
	QCOMPARE(column->textAt(4), QLatin1String("2.2"));

	// 3rd column, Origin::TextNumeric:
	// first is numerical, column is set to Numeric, empty or text values in the column a set to NAN
	column = spreadsheet->column(2);
	QCOMPARE(column->columnMode(), AbstractColumn::ColumnMode::Double);
	QCOMPARE(column->valueAt(0), 1.1);
	QCOMPARE(column->valueAt(1), 2.2);
	QCOMPARE(!std::isnan(column->valueAt(2)), false);
	QCOMPARE(!std::isnan(column->valueAt(3)), false);
	QCOMPARE(column->valueAt(4), 3.3);

	// 4th column, Origin::TextNumeric:
	// first value is string, the column is set to Text, numerical values are converted to strings
	column = spreadsheet->column(3);
	QCOMPARE(column->columnMode(), AbstractColumn::ColumnMode::Text);
	QCOMPARE(column->textAt(0), QLatin1String("a"));
	QCOMPARE(column->textAt(1), QLatin1String("b"));
	QCOMPARE(column->textAt(2), QLatin1String("1.1"));
	QCOMPARE(column->textAt(3), QLatin1String("2.2"));
	QCOMPARE(column->textAt(4), QLatin1String("c"));

	// 5th column, Origin::Numeric
	// column is set to Numeric, empty values in the column a set to NAN
	column = spreadsheet->column(4);
	QCOMPARE(column->columnMode(), AbstractColumn::ColumnMode::Double);
	QCOMPARE(!std::isnan(column->valueAt(0)), false);
	QCOMPARE(column->valueAt(1), 1.1);
	QCOMPARE(column->valueAt(2), 2.2);
	QCOMPARE(column->valueAt(3), 3.3);
	QCOMPARE(!std::isnan(column->valueAt(4)), false);

	// 6th column, Origin::Numeric
	// column is set to Numeric, empty values in the column a set to NAN
	column = spreadsheet->column(5);
	QCOMPARE(column->columnMode(), AbstractColumn::ColumnMode::Double);
	QCOMPARE(column->valueAt(0), 1.1);
	QCOMPARE(column->valueAt(1), 2.2);
	QCOMPARE(column->valueAt(2), 3.3);
	QCOMPARE(!std::isnan(column->valueAt(3)), false);
	QCOMPARE(!std::isnan(column->valueAt(4)), false);
}

void ProjectImportTest::testOrigin_2folder_with_graphs() {
	OriginProjectParser parser;
	parser.setProjectFileName(QFINDTESTDATA(QLatin1String("data/2folder-with-graphs.opj")));
	Project project;
	parser.importTo(&project, QStringList());

	// check the project tree for the imported project

	// Folder 1
	auto* f1 = dynamic_cast<Folder*>(project.child<AbstractAspect>(0));
	QVERIFY(f1 != nullptr);
	QCOMPARE(f1->name(), QLatin1String("Folder1"));

	auto* s1 = dynamic_cast<Spreadsheet*>(f1->child<AbstractAspect>(0));
	QVERIFY(s1 != nullptr);
	QCOMPARE(s1->name(), QLatin1String("Book1"));

	QCOMPARE(s1->columnCount(), 2);
	QCOMPARE(s1->rowCount(), 32);

	auto* c1 = dynamic_cast<Column*>(s1->child<AbstractAspect>(0));
	QVERIFY(c1 != nullptr);
	QCOMPARE(c1->name(), QLatin1String("A"));
	auto* c2 = dynamic_cast<Column*>(s1->child<AbstractAspect>(1));
	QVERIFY(c2 != nullptr);
	QCOMPARE(c2->name(), QLatin1String("B"));

	auto* w1 = dynamic_cast<Worksheet*>(f1->child<AbstractAspect>(1));
	QVERIFY(w1 != nullptr);
	QCOMPARE(w1->name(), QLatin1String("Graph1"));

	auto* plot = dynamic_cast<CartesianPlot*>(w1->child<CartesianPlot>(0));
	QVERIFY(plot != nullptr);

	QCOMPARE(plot->name(), QLatin1String("Plot1"));

	auto* xAxis = dynamic_cast<Axis*>(plot->child<Axis>(0));
	QVERIFY(xAxis != nullptr);
	QCOMPARE(xAxis->name(), QStringLiteral("x"));
	auto* yAxis = dynamic_cast<Axis*>(plot->child<Axis>(1));
	QVERIFY(yAxis != nullptr);
	QCOMPARE(yAxis->name(), QStringLiteral("y"));

	auto* legend = dynamic_cast<CartesianPlotLegend*>(plot->child<CartesianPlotLegend>(0));
	QVERIFY(legend != nullptr);
	QCOMPARE(legend->name(), QStringLiteral("legend"));

	auto* curve = dynamic_cast<XYCurve*>(plot->child<XYCurve>(0));
	QVERIFY(curve != nullptr);
	QCOMPARE(curve->name(), QStringLiteral("Length"));

	CHECK_RANGE(plot, curve, Dimension::X, 0., 9.);
	CHECK_RANGE(plot, curve, Dimension::Y, 0., 9.);

	QCOMPARE(curve->xColumnPath(), QLatin1String("2folder-with-graphs/Folder1/Book1/A"));
	QCOMPARE(curve->yColumnPath(), QLatin1String("2folder-with-graphs/Folder1/Book1/B"));
	QCOMPARE(curve->lineType(), XYCurve::LineType::Line);
	QCOMPARE(curve->legendVisible(), true);
	QCOMPARE(curve->lineType(), XYCurve::LineType::Line);
	QCOMPARE(curve->lineSkipGaps(), false);
	QCOMPARE(curve->line()->opacity(), 1);
	QCOMPARE(curve->dropLine()->dropLineType(), XYCurve::DropLineType::NoDropLine);
	QCOMPARE(curve->valuesType(), XYCurve::ValuesType::NoValues);
	// TODO: check more curve properties

	auto* symbol = curve->symbol();
	QVERIFY(symbol != nullptr);
	QCOMPARE(symbol->style(), Symbol::Style::SquareHalf);
#if defined(HAVE_WINDOWS)
	QCOMPARE(symbol->size(), 30.4692260817);
#else
	QCOMPARE(symbol->size(), 89.8842169409);
#endif
	// TODO: more symbol props

	// Folder 2
	auto* f2 = dynamic_cast<Folder*>(project.child<AbstractAspect>(1));
	QVERIFY(f2 != nullptr);
	QCOMPARE(f2->name(), QLatin1String("Folder2"));

	auto* s2 = dynamic_cast<Spreadsheet*>(f2->child<AbstractAspect>(0));
	QVERIFY(s2 != nullptr);
	QCOMPARE(s2->name(), QLatin1String("Book2"));

	QCOMPARE(s1->columnCount(), 2);
	QCOMPARE(s1->rowCount(), 32);

	auto* w2 = dynamic_cast<Worksheet*>(f2->child<AbstractAspect>(1));
	QVERIFY(w2 != nullptr);
	QCOMPARE(w2->name(), QLatin1String("Graph2"));

	plot = dynamic_cast<CartesianPlot*>(w2->child<CartesianPlot>(0));
	QVERIFY(plot != nullptr);

	QCOMPARE(plot->name(), QLatin1String("Plot1"));

	xAxis = dynamic_cast<Axis*>(plot->child<Axis>(0));
	QVERIFY(xAxis != nullptr);
	QCOMPARE(xAxis->name(), QStringLiteral("x"));
	yAxis = dynamic_cast<Axis*>(plot->child<Axis>(1));
	QVERIFY(yAxis != nullptr);
	QCOMPARE(yAxis->name(), QStringLiteral("y"));

	legend = dynamic_cast<CartesianPlotLegend*>(plot->child<CartesianPlotLegend>(0));
	QVERIFY(legend != nullptr);
	QCOMPARE(legend->name(), QStringLiteral("legend"));

	curve = dynamic_cast<XYCurve*>(plot->child<XYCurve>(0));
	QVERIFY(curve != nullptr);
	QCOMPARE(curve->name(), QStringLiteral("B"));

	CHECK_RANGE(plot, curve, Dimension::X, .5, 5.5);
	CHECK_RANGE(plot, curve, Dimension::Y, .5, 5.5);

	QCOMPARE(curve->xColumnPath(), QLatin1String("2folder-with-graphs/Folder2/Book2/A"));
	QCOMPARE(curve->yColumnPath(), QLatin1String("2folder-with-graphs/Folder2/Book2/B"));
	QCOMPARE(curve->lineType(), XYCurve::LineType::Line);
	QCOMPARE(curve->legendVisible(), true);
	QCOMPARE(curve->lineType(), XYCurve::LineType::Line);
	QCOMPARE(curve->lineSkipGaps(), false);
	QCOMPARE(curve->line()->opacity(), 1);
	QCOMPARE(curve->dropLine()->dropLineType(), XYCurve::DropLineType::NoDropLine);
	QCOMPARE(curve->valuesType(), XYCurve::ValuesType::NoValues);
	// TODO: check more curve properties

	symbol = curve->symbol();
	QVERIFY(symbol != nullptr);
	QCOMPARE(symbol->style(), Symbol::Style::Hexagon);
#if defined(HAVE_WINDOWS)
	QCOMPARE(symbol->size(), 30.4692260817);
#else
	QCOMPARE(symbol->size(), 89.8842169409);
#endif
	// TODO: more symbol props
}

void ProjectImportTest::testOrigin_2graphs() {
	OriginProjectParser parser;
	parser.setProjectFileName(QFINDTESTDATA(QLatin1String("data/2graphs.opj")));
	Project project;
	parser.importTo(&project, QStringList());

	// check the project tree for the imported project
	//  Book 2
	auto* s2 = dynamic_cast<Spreadsheet*>(project.child<AbstractAspect>(0));
	QVERIFY(s2 != nullptr);
	QCOMPARE(s2->name(), QLatin1String("Book2"));

	QCOMPARE(s2->columnCount(), 2);
	QCOMPARE(s2->rowCount(), 32);

	auto* c1 = dynamic_cast<Column*>(s2->child<AbstractAspect>(0));
	QVERIFY(c1 != nullptr);
	QCOMPARE(c1->name(), QLatin1String("A"));
	auto* c2 = dynamic_cast<Column*>(s2->child<AbstractAspect>(1));
	QVERIFY(c2 != nullptr);
	QCOMPARE(c2->name(), QLatin1String("B"));

	// Graph 2
	auto* w2 = dynamic_cast<Worksheet*>(project.child<AbstractAspect>(1));
	QVERIFY(w2 != nullptr);
	QCOMPARE(w2->name(), QLatin1String("Graph2"));

	auto* plot = dynamic_cast<CartesianPlot*>(w2->child<CartesianPlot>(0));
	QVERIFY(plot != nullptr);

	QCOMPARE(plot->name(), QLatin1String("Plot1"));

	auto* xAxis = dynamic_cast<Axis*>(plot->child<Axis>(0));
	QVERIFY(xAxis != nullptr);
	QCOMPARE(xAxis->name(), QStringLiteral("x"));
	auto* yAxis = dynamic_cast<Axis*>(plot->child<Axis>(1));
	QVERIFY(yAxis != nullptr);
	QCOMPARE(yAxis->name(), QStringLiteral("y"));

	auto* legend = dynamic_cast<CartesianPlotLegend*>(plot->child<CartesianPlotLegend>(0));
	QVERIFY(legend != nullptr);
	QCOMPARE(legend->name(), QStringLiteral("legend"));

	auto* curve = dynamic_cast<XYCurve*>(plot->child<XYCurve>(0));
	QVERIFY(curve != nullptr);
	QCOMPARE(curve->name(), QStringLiteral("B")); // TODO: Origin uses Comments as curve name: "Length"
	QCOMPARE(curve->coordinateSystemIndex(), plot->defaultCoordinateSystemIndex());

	CHECK_RANGE(plot, curve, Dimension::X, .5, 5.5);
	CHECK_RANGE(plot, curve, Dimension::Y, .5, 5.5);

	QCOMPARE(curve->xColumnPath(), QLatin1String("2graphs/Book2/A"));
	QCOMPARE(curve->yColumnPath(), QLatin1String("2graphs/Book2/B"));
	QCOMPARE(curve->lineType(), XYCurve::LineType::Line);
	QCOMPARE(curve->legendVisible(), true);
	QCOMPARE(curve->lineType(), XYCurve::LineType::Line);
	QCOMPARE(curve->lineSkipGaps(), false);
	QCOMPARE(curve->line()->opacity(), 1);
	QCOMPARE(curve->dropLine()->dropLineType(), XYCurve::DropLineType::NoDropLine);
	QCOMPARE(curve->valuesType(), XYCurve::ValuesType::NoValues);
	// TODO: more curve properties

	auto* symbol = curve->symbol();
	QVERIFY(symbol != nullptr);
	QCOMPARE(symbol->style(), Symbol::Style::Hexagon);
#if defined(HAVE_WINDOWS)
	QCOMPARE(symbol->size(), 30.4692260817);
#else
	QCOMPARE(symbol->size(), 89.8842169409);
#endif
	// TODO: more symbol props

	// Graph 1
	auto* w1 = dynamic_cast<Worksheet*>(project.child<AbstractAspect>(2));
	QVERIFY(w1 != nullptr);
	QCOMPARE(w1->name(), QLatin1String("Graph1"));

	plot = dynamic_cast<CartesianPlot*>(w1->child<CartesianPlot>(0));
	QVERIFY(plot != nullptr);

	QCOMPARE(plot->name(), QLatin1String("Plot1"));

	xAxis = dynamic_cast<Axis*>(plot->child<Axis>(0));
	QVERIFY(xAxis != nullptr);
	QCOMPARE(xAxis->name(), QStringLiteral("x"));
	yAxis = dynamic_cast<Axis*>(plot->child<Axis>(1));
	QVERIFY(yAxis != nullptr);
	QCOMPARE(yAxis->name(), QStringLiteral("y"));

	legend = dynamic_cast<CartesianPlotLegend*>(plot->child<CartesianPlotLegend>(0));
	QVERIFY(legend != nullptr);
	QCOMPARE(legend->name(), QStringLiteral("legend"));

	curve = dynamic_cast<XYCurve*>(plot->child<XYCurve>(0));
	QVERIFY(curve != nullptr);
	QCOMPARE(curve->name(), QStringLiteral("Length"));
	QCOMPARE(curve->coordinateSystemIndex(), plot->defaultCoordinateSystemIndex());

	CHECK_RANGE(plot, curve, Dimension::X, 0., 9.);
	CHECK_RANGE(plot, curve, Dimension::Y, 0., 9.);

	QCOMPARE(curve->xColumnPath(), QLatin1String("2graphs/Book1/A"));
	QCOMPARE(curve->yColumnPath(), QLatin1String("2graphs/Book1/B"));
	QCOMPARE(curve->legendVisible(), true);
	QCOMPARE(curve->lineType(), XYCurve::LineType::Line);
	QCOMPARE(curve->lineSkipGaps(), false);
	QCOMPARE(curve->line()->opacity(), 1);
	QCOMPARE(curve->dropLine()->dropLineType(), XYCurve::DropLineType::NoDropLine);
	QCOMPARE(curve->valuesType(), XYCurve::ValuesType::NoValues);
	// TODO: more curve properties

	symbol = curve->symbol();
	QVERIFY(symbol != nullptr);
	QCOMPARE(symbol->style(), Symbol::Style::SquareHalf);
#if defined(HAVE_WINDOWS)
	QCOMPARE(symbol->size(), 30.4692260817);
#else
	QCOMPARE(symbol->size(), 89.8842169409);
#endif
	// TODO: more symbol props

	// Book 1
	auto* s1 = dynamic_cast<Spreadsheet*>(project.child<AbstractAspect>(3));
	QVERIFY(s1 != nullptr);
	QCOMPARE(s1->name(), QLatin1String("Book1"));

	QCOMPARE(s2->columnCount(), 2);
	QCOMPARE(s2->rowCount(), 32);

	c1 = dynamic_cast<Column*>(s1->child<AbstractAspect>(0));
	QVERIFY(c1 != nullptr);
	QCOMPARE(c1->name(), QLatin1String("A"));
	c2 = dynamic_cast<Column*>(s1->child<AbstractAspect>(1));
	QVERIFY(c2 != nullptr);
	QCOMPARE(c2->name(), QLatin1String("B"));
}

void ProjectImportTest::testOriginHistogram() {
	OriginProjectParser parser;
	parser.setProjectFileName(QFINDTESTDATA(QLatin1String("data/histogram.opj")));
	Project project;
	parser.importTo(&project, QStringList());

	// check the project tree for the imported project
	auto* folder = dynamic_cast<Folder*>(project.child<AbstractAspect>(0));
	QVERIFY(folder != nullptr);
	QCOMPARE(folder->name(), QLatin1String("Folder1"));

	// Book
	auto* sheet = dynamic_cast<Spreadsheet*>(folder->child<AbstractAspect>(0));
	QVERIFY(sheet != nullptr);
	QCOMPARE(sheet->name(), QLatin1String("Book1"));
	QCOMPARE(sheet->columnCount(), 2);
	QCOMPARE(sheet->rowCount(), 32);

	auto* col1 = dynamic_cast<Column*>(sheet->child<AbstractAspect>(0));
	QVERIFY(col1 != nullptr);
	QCOMPARE(col1->name(), QLatin1String("A"));

	auto* col2 = dynamic_cast<Column*>(sheet->child<AbstractAspect>(1));
	QVERIFY(col2 != nullptr);
	QCOMPARE(col2->name(), QLatin1String("B"));

	// Graph 1
	auto* worksheet = dynamic_cast<Worksheet*>(folder->child<AbstractAspect>(1));
	QVERIFY(worksheet != nullptr);
	QCOMPARE(worksheet->name(), QLatin1String("Graph1"));

	auto* plot = dynamic_cast<CartesianPlot*>(worksheet->child<CartesianPlot>(0));
	QVERIFY(plot != nullptr);
	QCOMPARE(plot->name(), QLatin1String("Plot1"));

	auto* histogram = dynamic_cast<Histogram*>(plot->child<Histogram>(0));
	QVERIFY(histogram != nullptr);
	QCOMPARE(histogram->name(), QStringLiteral("A"));
	QCOMPARE(histogram->coordinateSystemIndex(), plot->defaultCoordinateSystemIndex());
	QCOMPARE(histogram->orientation(), Histogram::Orientation::Vertical);
	// TODO: chek more properties of the histogram

	// Graph 2
	worksheet = dynamic_cast<Worksheet*>(folder->child<AbstractAspect>(2));
	QVERIFY(worksheet != nullptr);
	QCOMPARE(worksheet->name(), QLatin1String("Graph2"));

	plot = dynamic_cast<CartesianPlot*>(worksheet->child<CartesianPlot>(0));
	QVERIFY(plot != nullptr);
	QCOMPARE(plot->name(), QLatin1String("Plot1"));

	histogram = dynamic_cast<Histogram*>(plot->child<Histogram>(0));
	QVERIFY(histogram != nullptr);
	QCOMPARE(histogram->name(), QStringLiteral("A"));
	QCOMPARE(histogram->coordinateSystemIndex(), plot->defaultCoordinateSystemIndex());
	QCOMPARE(histogram->orientation(), Histogram::Orientation::Horizontal);
	// TODO: check more properties of the histogram
}

void ProjectImportTest::testOriginBarPlot() {
	OriginProjectParser parser;
	parser.setProjectFileName(QFINDTESTDATA(QLatin1String("data/bar-column.opj")));
	Project project;
	parser.importTo(&project, QStringList());

	// check the project tree for the imported project
	auto* folder = dynamic_cast<Folder*>(project.child<AbstractAspect>(0));
	QVERIFY(folder != nullptr);
	QCOMPARE(folder->name(), QLatin1String("Folder1"));

	// Book
	auto* sheet = dynamic_cast<Spreadsheet*>(folder->child<AbstractAspect>(0));
	QVERIFY(sheet != nullptr);
	QCOMPARE(sheet->name(), QLatin1String("Book1"));
	QCOMPARE(sheet->columnCount(), 2);
	QCOMPARE(sheet->rowCount(), 32);

	auto* col1 = dynamic_cast<Column*>(sheet->child<AbstractAspect>(0));
	QVERIFY(col1 != nullptr);
	QCOMPARE(col1->name(), QLatin1String("A"));

	auto* col2 = dynamic_cast<Column*>(sheet->child<AbstractAspect>(1));
	QVERIFY(col2 != nullptr);
	QCOMPARE(col2->name(), QLatin1String("B"));

	// Graph 2
	auto* worksheet = dynamic_cast<Worksheet*>(folder->child<AbstractAspect>(1));
	QVERIFY(worksheet != nullptr);
	QCOMPARE(worksheet->name(), QLatin1String("Graph2"));

	auto* plot = dynamic_cast<CartesianPlot*>(worksheet->child<CartesianPlot>(0));
	QVERIFY(plot != nullptr);
	QCOMPARE(plot->name(), QLatin1String("Plot1"));

	auto* barPlot = dynamic_cast<BarPlot*>(plot->child<BarPlot>(0));
	QVERIFY(barPlot != nullptr);
	QCOMPARE(barPlot->name(), QStringLiteral("B"));
	QCOMPARE(barPlot->coordinateSystemIndex(), plot->defaultCoordinateSystemIndex());
	QCOMPARE(barPlot->orientation(), BarPlot::Orientation::Horizontal);
	// TODO: chek more properties of the barPlot

	// Graph 1
	worksheet = dynamic_cast<Worksheet*>(folder->child<AbstractAspect>(2));
	QVERIFY(worksheet != nullptr);
	QCOMPARE(worksheet->name(), QLatin1String("Graph1"));

	plot = dynamic_cast<CartesianPlot*>(worksheet->child<CartesianPlot>(0));
	QVERIFY(plot != nullptr);
	QCOMPARE(plot->name(), QLatin1String("Plot1"));

	barPlot = dynamic_cast<BarPlot*>(plot->child<BarPlot>(0));
	QVERIFY(barPlot != nullptr);
	QCOMPARE(barPlot->name(), QStringLiteral("B"));
	QCOMPARE(barPlot->coordinateSystemIndex(), plot->defaultCoordinateSystemIndex());
	QCOMPARE(barPlot->orientation(), BarPlot::Orientation::Vertical);
	// TODO: check more properties of the barPlot
}

/*!
 * read a project file containing one plot area/layer with one single coordinate system with 4 axes and one curve.
 */
void ProjectImportTest::testOriginSingleLayerTwoAxes() {
	// import the opj file into LabPlot's project object
	OriginProjectParser parser;
	parser.setProjectFileName(QFINDTESTDATA(QLatin1String("data/single_layer_two_axes.opj")));
	parser.setGraphLayerAsPlotArea(true); // read every layer as a new plot area
	Project project;
	parser.importTo(&project, QStringList());

	const auto& plots = project.children<CartesianPlot>(AbstractAspect::ChildIndexFlag::Recursive);
	QCOMPARE(plots.count(), 1);

	// check the plot area
	const auto* plot1 = plots.first();
	QVERIFY(plot1 != nullptr);

	// ranges and axes - there should be one single coordinate system and 4 axes on the plot area
	QCOMPARE(plot1->rangeCount(Dimension::X), 1);
	QCOMPARE(plot1->rangeCount(Dimension::Y), 1);
	QCOMPARE(plot1->coordinateSystemCount(), 1);

	const auto& rangeX1 = plot1->range(Dimension::X, 0);
	QCOMPARE(rangeX1.start(), 1.5);
	QCOMPARE(rangeX1.end(), 8.5);
	QCOMPARE(rangeX1.scale(), RangeT::Scale::Linear);
	QCOMPARE(rangeX1.format(), RangeT::Format::Numeric);

	const auto& rangeY1 = plot1->range(Dimension::Y, 0);
	QCOMPARE(rangeY1.start(), 3.5);
	QCOMPARE(rangeY1.end(), 8.5);
	QCOMPARE(rangeY1.scale(), RangeT::Scale::Linear);
	QCOMPARE(rangeY1.format(), RangeT::Format::Numeric);

	// axes
	const auto& axes = plot1->children<Axis>();
	QCOMPARE(axes.count(), 4);

	// curve
	const auto& curves1 = plot1->children<XYCurve>();
	QCOMPARE(curves1.count(), 1);
	QCOMPARE(curves1.constFirst()->coordinateSystemIndex(), 0);
}

/*!
 * read a project file containing one plot area/layer with one single coordinate system with 4 axes and one curve.
 * the line of the second axes is hidden, only the labels are shown - we still have to create for axes objects after the import.
 */
void ProjectImportTest::testOriginSingleLayerTwoAxesWithoutLines() {
	// import the opj file into LabPlot's project object
	OriginProjectParser parser;
	parser.setProjectFileName(QFINDTESTDATA(QLatin1String("data/single_layer_two_axes_without_lines.opj")));
	parser.setGraphLayerAsPlotArea(true); // read every layer as a new plot area
	Project project;
	parser.importTo(&project, QStringList());

	const auto& plots = project.children<CartesianPlot>(AbstractAspect::ChildIndexFlag::Recursive);
	QCOMPARE(plots.count(), 1);

	// check the plot area
	const auto* plot1 = plots.first();
	QVERIFY(plot1 != nullptr);

	// ranges and axes - there should be one single coordinate system and 4 axes on the plot area
	QCOMPARE(plot1->rangeCount(Dimension::X), 1);
	QCOMPARE(plot1->rangeCount(Dimension::Y), 1);
	QCOMPARE(plot1->coordinateSystemCount(), 1);

	const auto& rangeX1 = plot1->range(Dimension::X, 0);
	QCOMPARE(rangeX1.start(), 1.5);
	QCOMPARE(rangeX1.end(), 8.5);
	QCOMPARE(rangeX1.scale(), RangeT::Scale::Linear);
	QCOMPARE(rangeX1.format(), RangeT::Format::Numeric);

	const auto& rangeY1 = plot1->range(Dimension::Y, 0);
	QCOMPARE(rangeY1.start(), 3.5);
	QCOMPARE(rangeY1.end(), 8.5);
	QCOMPARE(rangeY1.scale(), RangeT::Scale::Linear);
	QCOMPARE(rangeY1.format(), RangeT::Format::Numeric);

	// axes
	const auto& axes = plot1->children<Axis>();
	QCOMPARE(axes.count(), 4);

	// curve
	const auto& curves1 = plot1->children<XYCurve>();
	QCOMPARE(curves1.count(), 1);
	QCOMPARE(curves1.constFirst()->coordinateSystemIndex(), 0);
}

/*!
 * read a project file containing two plot areas with one single coordinate system and one curve in each the plot area.
 */
void ProjectImportTest::testOriginMultiLayersAsPlotAreas() {
	// import the opj file into LabPlot's project object
	OriginProjectParser parser;
	parser.setProjectFileName(QFINDTESTDATA(QLatin1String("data/two_layers_as_two_plot_areas.opj")));
	parser.setGraphLayerAsPlotArea(true); // read every layer as a new plot area
	Project project;
	parser.importTo(&project, QStringList());

	const auto& plots = project.children<CartesianPlot>(AbstractAspect::ChildIndexFlag::Recursive);
	QCOMPARE(plots.count(), 2);

	// check the first plot area
	const auto* plot1 = plots.first();
	QVERIFY(plot1 != nullptr);

	// ranges
	QCOMPARE(plot1->rangeCount(Dimension::X), 1);
	QCOMPARE(plot1->rangeCount(Dimension::Y), 1);
	QCOMPARE(plot1->coordinateSystemCount(), 1);

	const auto& rangeX1 = plot1->range(Dimension::X, 0);
	QCOMPARE(rangeX1.start(), 1.5);
	QCOMPARE(rangeX1.end(), 8.5);
	QCOMPARE(rangeX1.scale(), RangeT::Scale::Linear);
	QCOMPARE(rangeX1.format(), RangeT::Format::Numeric);

	const auto& rangeY1 = plot1->range(Dimension::Y, 0);
	QCOMPARE(rangeY1.start(), 3.5);
	QCOMPARE(rangeY1.end(), 8.5);
	QCOMPARE(rangeY1.scale(), RangeT::Scale::Linear);
	QCOMPARE(rangeY1.format(), RangeT::Format::Numeric);

	// curve
	const auto& curves1 = plot1->children<XYCurve>();
	QCOMPARE(curves1.count(), 1);
	QCOMPARE(curves1.constFirst()->coordinateSystemIndex(), 0);

	// check the second plot area
	const auto* plot2 = plots.at(1);
	QVERIFY(plot2 != nullptr);

	// ranges
	QCOMPARE(plot2->rangeCount(Dimension::X), 1);
	QCOMPARE(plot2->rangeCount(Dimension::Y), 1);
	QCOMPARE(plot2->coordinateSystemCount(), 1);

	const auto& rangeX2 = plot2->range(Dimension::X, 0);
	QCOMPARE(rangeX2.start(), 1.5);
	QCOMPARE(rangeX2.end(), 8.5);
	QCOMPARE(rangeX2.scale(), RangeT::Scale::Linear);
	QCOMPARE(rangeX2.format(), RangeT::Format::Numeric);

	const auto& rangeY2 = plot2->range(Dimension::Y, 0);
	QCOMPARE(rangeY2.start(), 8.);
	QCOMPARE(rangeY2.end(), 18.);
	QCOMPARE(rangeY2.scale(), RangeT::Scale::Linear);
	QCOMPARE(rangeY2.format(), RangeT::Format::Numeric);

	// curve
	const auto& curves2 = plot1->children<XYCurve>();
	QCOMPARE(curves2.count(), 1);
	QCOMPARE(curves2.constFirst()->coordinateSystemIndex(), 0);
}

/*!
 * read a project file containing one plot area with two coordinate systems ("two axes") and one curve per each coordinate system.
 */
void ProjectImportTest::testOriginMultiLayersAsCoordinateSystems() {
	// import the opj file into LabPlot's project object
	OriginProjectParser parser;
	parser.setProjectFileName(QFINDTESTDATA(QLatin1String("data/two_layers_as_two_coordinate_systems.opj")));
	parser.setGraphLayerAsPlotArea(false); // read every layer as a new coordinate system
	Project project;
	parser.importTo(&project, QStringList());

	// check the ranges of the CartesianPlot in the project
	const auto& plots = project.children<CartesianPlot>(AbstractAspect::ChildIndexFlag::Recursive);
	QCOMPARE(plots.count(), 1);

	const auto* plot = plots.first();
	QVERIFY(plot != nullptr);

	// ranges
	QCOMPARE(plot->rangeCount(Dimension::X), 1);
	QCOMPARE(plot->rangeCount(Dimension::Y), 2);

	const auto& rangeX = plot->range(Dimension::X, 0);
	QCOMPARE(rangeX.start(), 1.);
	QCOMPARE(rangeX.end(), 12.);
	QCOMPARE(rangeX.scale(), RangeT::Scale::Linear);
	QCOMPARE(rangeX.format(), RangeT::Format::Numeric);

	const auto& rangeY1 = plot->range(Dimension::Y, 0);
	QCOMPARE(rangeY1.start(), 1.75);
	QCOMPARE(rangeY1.end(), 5.25);
	QCOMPARE(rangeY1.scale(), RangeT::Scale::Linear);
	QCOMPARE(rangeY1.format(), RangeT::Format::Numeric);

	const auto& rangeY2 = plot->range(Dimension::Y, 1);
	QCOMPARE(rangeY2.start(), 20);
	QCOMPARE(rangeY2.end(), 47.5);
	QCOMPARE(rangeY2.scale(), RangeT::Scale::Linear);
	QCOMPARE(rangeY2.format(), RangeT::Format::Numeric);

	// coordinate systems
	QCOMPARE(plot->coordinateSystemCount(), 2);

	// curves, two curves in total, one curve for every layer/coordinate system
	const auto& curves = plot->children<XYCurve>();
	QCOMPARE(curves.count(), 2);
	QCOMPARE(curves.at(0)->coordinateSystemIndex(), 0);
	QCOMPARE(curves.at(1)->coordinateSystemIndex(), 1);
}

/*!
 * read a project file containing one plot area with two coordinate systems ("two axes") and one curve per each coordinate system.
 * the test project was created with a newer version of Origin and has a legend defined via an annotation that we need to
 * properly interpret.
 */
void ProjectImportTest::testOriginMultiLayersAsCoordinateSystemsWithLegend() {
	// import the opj file into LabPlot's project object
	OriginProjectParser parser;
	parser.setProjectFileName(QFINDTESTDATA(QLatin1String("data/two_layers_as_two_coordinate_systems_with_legend.opj")));
	parser.setGraphLayerAsPlotArea(false); // read every layer as a new coordinate system
	Project project;
	parser.importTo(&project, QStringList());

	// check the ranges of the CartesianPlot in the project
	const auto& plots = project.children<CartesianPlot>(AbstractAspect::ChildIndexFlag::Recursive);
	QCOMPARE(plots.count(), 1);

	const auto* plot = plots.first();
	QVERIFY(plot != nullptr);

	// ranges
	QCOMPARE(plot->rangeCount(Dimension::X), 1);
	QCOMPARE(plot->rangeCount(Dimension::Y), 2);

	const auto& rangeX = plot->range(Dimension::X, 0);
	QCOMPARE(rangeX.start(), 1.);
	QCOMPARE(rangeX.end(), 12.);
	QCOMPARE(rangeX.scale(), RangeT::Scale::Linear);
	QCOMPARE(rangeX.format(), RangeT::Format::Numeric);

	const auto& rangeY1 = plot->range(Dimension::Y, 0);
	QCOMPARE(rangeY1.start(), 1.75);
	QCOMPARE(rangeY1.end(), 5.25);
	QCOMPARE(rangeY1.scale(), RangeT::Scale::Linear);
	QCOMPARE(rangeY1.format(), RangeT::Format::Numeric);

	const auto& rangeY2 = plot->range(Dimension::Y, 1);
	QCOMPARE(rangeY2.start(), 20);
	QCOMPARE(rangeY2.end(), 47.5);
	QCOMPARE(rangeY2.scale(), RangeT::Scale::Linear);
	QCOMPARE(rangeY2.format(), RangeT::Format::Numeric);

	// coordinate systems
	QCOMPARE(plot->coordinateSystemCount(), 2);

	// curves, two curves in total, one curve for every layer/coordinate system
	const auto& curves = plot->children<XYCurve>();
	QCOMPARE(curves.count(), 2);
	QCOMPARE(curves.at(0)->coordinateSystemIndex(), 0);
	QCOMPARE(curves.at(1)->coordinateSystemIndex(), 1);

	// legend
	const auto& legends = plot->children<CartesianPlotLegend>();
	QCOMPARE(legends.count(), 1);
}

void ProjectImportTest::testImportOriginStrings() {
	OriginProjectParser parser;
	parser.setProjectFileName(QFINDTESTDATA(QLatin1String("data/strings_testing.opj")));
	// parser.setGraphLayerAsPlotArea(false); // read every layer as a new coordinate system
	Project project;
	parser.importTo(&project, QStringList());

	const auto& plots = project.children<CartesianPlot>(AbstractAspect::ChildIndexFlag::Recursive);
	QCOMPARE(plots.count(), 1);

	const auto* plot = plots.first();
	QVERIFY(plot != nullptr);

	const auto& curves = plot->children<XYCurve>();
	QCOMPARE(curves.count(), 3); // formula ignored
	// const auto* curve = curves.at(0);
	const auto& axes = plot->children<Axis>();
	QCOMPARE(axes.count(), 4);
	const auto* xAxis = axes.at(0);
	const auto* x2Axis = axes.at(1);
	const auto* yAxis = axes.at(2);
	const auto* y2Axis = axes.at(3);

	// axis label
	QCOMPARE(xAxis->name(), QStringLiteral("x"));
	QCOMPARE(yAxis->name(), QStringLiteral("y"));
	QCOMPARE(x2Axis->name(), QStringLiteral("x top"));
	QCOMPARE(y2Axis->name(), QStringLiteral("y right"));
	QTextEdit xte(xAxis->title()->text().text);
	QCOMPARE(xte.toPlainText(), QStringLiteral("A"));
	QTextEdit yte(yAxis->title()->text().text);
	QCOMPARE(yte.toPlainText(), QStringLiteral("Log of x"));
	QTextEdit x2te(x2Axis->title()->text().text);
	QCOMPARE(x2te.toPlainText(), QStringLiteral("x axis title"));
	QTextEdit y2te(y2Axis->title()->text().text);
	QCOMPARE(y2te.toPlainText(), QStringLiteral("B"));

	// axis prefix, suffix
	QCOMPARE(xAxis->labelsPrefix(), QStringLiteral("pb"));
	QCOMPARE(xAxis->labelsSuffix(), QStringLiteral("sb"));
	QCOMPARE(yAxis->labelsPrefix(), QStringLiteral("pl"));
	QCOMPARE(yAxis->labelsSuffix(), QStringLiteral("sl"));
	QCOMPARE(x2Axis->labelsPrefix(), QStringLiteral("pt"));
	QCOMPARE(x2Axis->labelsSuffix(), QStringLiteral("st"));
	QCOMPARE(y2Axis->labelsPrefix(), QStringLiteral("pr"));
	QCOMPARE(y2Axis->labelsSuffix(), QStringLiteral("sr"));
	// TODO: axis formula is not imported yet
	QCOMPARE(xAxis->scalingFactor(), 1.);
	QCOMPARE(yAxis->scalingFactor(), 1.);
	QCOMPARE(x2Axis->scalingFactor(), 1.);
	QCOMPARE(y2Axis->scalingFactor(), 1.);

	// TODO: formula
	// const auto& eqCurves = plot->children<XYEquationCurve>();
	// QCOMPARE(curves.count(), 1);
	// const auto* curve = curves.at(0);
	// DEBUG("EQUATION FORMULA: " << curve->equationData().expression1.toStdString())
}

void ProjectImportTest::testParseOriginTags_data() {
	QTest::addColumn<QString>("originTag");
	QTest::addColumn<QString>("labPlotHTML");

	QTest::newRow("bold") << "\\b(bold)"
						  << "<b>bold</b>";

	QTest::newRow("italic") << "\\i(italic)"
							<< "<i>italic</i>";

	QTest::newRow("strike through") << "\\s(strike through)"
									<< "<s>strike through</s>";

	QTest::newRow("underlined") << "\\u(underlined)"
								<< "<u>underlined</u>";

	QTest::newRow("greek char") << "\\g(a)"
								<< "&alpha;";

	QTest::newRow("sub-script") << "a\\-(b)"
								<< "a<sub>b</sub>";

	QTest::newRow("super-script") << "a\\+(b)"
								  << "a<sup>b</sup>";

	QTest::newRow("set-font") << "\\f:dejavu sans(text)"
							  << "<font face=\"dejavu sans\">text</font>";

	QTest::newRow("font-size") << "some \\p200(big) text"
							   << "some <span style=\"font-size: 200%\">big</span> text";

	QTest::newRow("color") << "some \\c15(colored) text"
						   << "some <span style=\"color: #8000ff\">colored</span> text";

	QTest::newRow("nested-non-tag-parenthesis") << "\\b(text (c) and (fh) and a(t) and empty ())"
												<< "<b>text (c) and (fh) and a(t) and empty ()</b>";

	QTest::newRow("nested-tags") << "\\b(bold text with some \\i(italic) bits and some \\c15(color)) "
									"then a change of \\f:dejavu sans(font)"
								 << "<b>bold text with some <i>italic</i> bits and some "
									"<span style=\"color: #8000ff\">color</span></b> "
									"then a change of <font face=\"dejavu sans\">font</font>";

	QTest::newRow("nested-tags-with-extra-spaces") << "\\ b (bold text with some \\ i(italic) bits and some \\c 15 (color)) "
													  "then a change of \\ f:dejavu sans( font)"
												   << "<b>bold text with some <i>italic</i> bits and some "
													  "<span style=\"color: #8000ff\">color</span></b> "
													  "then a change of <font face=\"dejavu sans\"> font</font>";
}

void ProjectImportTest::testParseOriginTags() {
	QFETCH(QString, originTag);
	QFETCH(QString, labPlotHTML);

	OriginProjectParser parser;
	QCOMPARE(parser.parseOriginTags(originTag), labPlotHTML);
}

#endif

QTEST_MAIN(ProjectImportTest)
