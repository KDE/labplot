/***************************************************************************
    File                 : ProjectImportTest.cpp
    Project              : LabPlot
    Description          : Tests for project imports
    --------------------------------------------------------------------
    Copyright            : (C) 2018 Alexander Semke (alexander.semke@web.de)
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

#include "ProjectImportTest.h"
#ifdef HAVE_LIBORIGIN
#include "backend/datasources/projects/OriginProjectParser.h"
#endif
#include "backend/core/Project.h"
#include "backend/core/Workbook.h"
#include "backend/matrix/Matrix.h"
#include "backend/worksheet/Worksheet.h"
#include "backend/worksheet/plots/cartesian/CartesianPlot.h"
#include "backend/spreadsheet/Spreadsheet.h"

void ProjectImportTest::initTestCase() {
	const QString currentDir = __FILE__;
	m_dataDir = currentDir.left(currentDir.lastIndexOf(QDir::separator())) + QDir::separator() + QLatin1String("data") + QDir::separator();

	// needed in order to have the signals triggered by SignallingUndoCommand, see LabPlot.cpp
	//TODO: redesign/remove this
	qRegisterMetaType<const AbstractAspect*>("const AbstractAspect*");
	qRegisterMetaType<const AbstractColumn*>("const AbstractColumn*");
}

//##############################################################################
//#####################  import of LabPlot projects ############################
//##############################################################################


#ifdef HAVE_LIBORIGIN
//##############################################################################
//######################  import of Origin projects ############################
//##############################################################################
//project tree of the file "origin8_test_tree_import.opj"
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

	//import the opj file into LabPlot's project object
	OriginProjectParser parser;
	parser.setProjectFileName(m_dataDir + QLatin1String("origin8_test_tree_import.opj"));
	Project project;
	parser.importTo(&project, QStringList());

	//check the project tree for the imported project

	//first child of the root folder, spreadsheet "Book3"
	auto* aspect = project.child<AbstractAspect>(0);
	QCOMPARE(aspect != nullptr, true);
	if (aspect != nullptr)
		QCOMPARE(aspect->name(), QLatin1String("Book3"));
	QCOMPARE(dynamic_cast<Spreadsheet*>(aspect) != nullptr, true);


	//first child of the root folder, folder "Folder" -> import into a Folder
	aspect = project.child<AbstractAspect>(1);
	QCOMPARE(aspect != nullptr, true);
	if (aspect != nullptr)
		QCOMPARE(aspect->name(), QLatin1String("Folder"));
	QCOMPARE(dynamic_cast<Folder*>(aspect) != nullptr, true);

	//first child of "Folder", workbook "Book2" with two sheets -> import into a Workbook with two Spreadsheets
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

	//second child of "Folder", matrixbook "MBook" with two matrix sheets -> import into a Workbook with two Matrices
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


	//second child of the root folder, folder "Folder1" -> import into a Folder
	aspect = project.child<AbstractAspect>(2);
	QCOMPARE(aspect != nullptr, true);
	if (aspect != nullptr)
		QCOMPARE(aspect->name(), QLatin1String("Folder1"));
	QCOMPARE(dynamic_cast<Folder*>(aspect) != nullptr, true);

	//first child of "Folder1", matrix "MBook1"
	aspect = project.child<AbstractAspect>(2)->child<AbstractAspect>(0);
	QCOMPARE(aspect != nullptr, true);
	if (aspect != nullptr)
		QCOMPARE(aspect->name(), QLatin1String("MBook1"));
	QCOMPARE(dynamic_cast<Matrix*>(aspect) != nullptr, true);

	//second child of "Folder1", workbook "Book1" with on sheet -> import into a Spreadsheet
	aspect = project.child<AbstractAspect>(2)->child<AbstractAspect>(1);
	QCOMPARE(aspect != nullptr, true);
	if (aspect != nullptr)
		QCOMPARE(aspect->name(), QLatin1String("Book1"));
	QCOMPARE(dynamic_cast<Spreadsheet*>(aspect) != nullptr, true);

	//second child of "Folder1", workbook "Book4" with on sheet -> import into a Spreadsheet
	aspect = project.child<AbstractAspect>(2)->child<AbstractAspect>(2);
	QCOMPARE(aspect != nullptr, true);
	if (aspect != nullptr)
		QCOMPARE(aspect->name(), QLatin1String("Book4"));
	QCOMPARE(dynamic_cast<Spreadsheet*>(aspect) != nullptr, true);

	//third child of "Folder1", graph "Graph"-> import into a Worksheet
	aspect = project.child<AbstractAspect>(2)->child<AbstractAspect>(3);
	QCOMPARE(aspect != nullptr, true);
	if (aspect != nullptr)
		QCOMPARE(aspect->name(), QLatin1String("Graph2"));
	QCOMPARE(dynamic_cast<Worksheet*>(aspect) != nullptr, true);
	//TODO: check the created plot in the worksheet

	// TODO: loose window: spreadsheet "Excel1"
	//aspect = project.child<AbstractAspect>(3);
	//QCOMPARE(aspect != nullptr, true);
	//QCOMPARE(aspect->name(), QLatin1String("Excel1"));
	//QCOMPARE(dynamic_cast<Spreadsheet*>(aspect) != nullptr, true);
}

/*
 * import one single folder child
 */
void ProjectImportTest::testOrigin02() {
	//import one single object
	OriginProjectParser parser;
	parser.setProjectFileName(m_dataDir + QLatin1String("origin8_test_tree_import.opj"));
	Project project;
	QStringList selectedPathes = {QLatin1String("test_tree_import/Folder1/Book1"), QLatin1String("test_tree_import/Folder1"), QLatin1String("test_tree_import")};
	parser.importTo(&project, selectedPathes);

	//check the project tree for the imported project

	//first child of the root folder, folder "Folder1" -> import into a Folder
	auto* aspect = project.child<AbstractAspect>(0);
	QCOMPARE(aspect != nullptr, true);
	if (aspect != nullptr)
		QCOMPARE(aspect->name(), QLatin1String("Folder1"));
	QCOMPARE(dynamic_cast<Folder*>(aspect) != nullptr, true);
	if (aspect != nullptr)
		QCOMPARE(aspect->childCount<AbstractAspect>(), 1);

	//first child of "Folder", workbook "Book" with one sheet -> import into a Spreadsheet
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
	//import one single object
	OriginProjectParser parser;
	parser.setProjectFileName(m_dataDir + QLatin1String("origin8_test_tree_import.opj"));
	Project project;

	//import one single child in "Folder1"
	QStringList selectedPathes = {QLatin1String("test_tree_import/Folder1/Book1"), QLatin1String("test_tree_import/Folder1"), QLatin1String("test_tree_import")};
	parser.importTo(&project, selectedPathes);

	//first child of the root folder, folder "Folder1" -> import into a Folder
	auto* aspect = project.child<AbstractAspect>(0);
	QCOMPARE(aspect != nullptr, true);
	if (aspect != nullptr)
		QCOMPARE(aspect->name(), QLatin1String("Folder1"));
	QCOMPARE(dynamic_cast<Folder*>(aspect) != nullptr, true);
	if (aspect != nullptr)
		QCOMPARE(aspect->childCount<AbstractAspect>(), 1);

	//first child of "Folder", workbook "Book1" with one sheet -> import into a Spreadsheet
	aspect = project.child<AbstractAspect>(0)->child<AbstractAspect>(0);
	QCOMPARE(aspect != nullptr, true);
	if (aspect != nullptr)
		QCOMPARE(aspect->name(), QLatin1String("Book1"));
	QCOMPARE(dynamic_cast<Spreadsheet*>(aspect) != nullptr, true);


	//import another child in "Folder1"
	selectedPathes.clear();
	selectedPathes << QLatin1String("test_tree_import/Folder1/Book4") << QLatin1String("test_tree_import/Folder1") << QLatin1String("test_tree_import");
	parser.importTo(&project, selectedPathes);

	//the first child should still be available in the project -> check it
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

	//check the second child, workbook "Book4" with one sheet -> import into a Spreadsheet
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
	parser.setProjectFileName(m_dataDir + QLatin1String("origin8_test_tree_import.opj"));
	Project project;

	//import "Book1"
	QStringList selectedPathes = {QLatin1String("test_tree_import/Folder1/Book1"), QLatin1String("test_tree_import/Folder1"), QLatin1String("test_tree_import")};
	parser.importTo(&project, selectedPathes);

	//first child of folder "Folder1", workbook "Book1" with one sheet -> import into a spreadsheet
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

	//the (0,0)-cell has the value 1.0
	if (spreadsheet != nullptr) {
		QCOMPARE(spreadsheet->column(0)->valueAt(0), 1.0);

		//set the value to 5.0
		spreadsheet->column(0)->setValueAt(0, 5.0);
		QCOMPARE(spreadsheet->column(0)->valueAt(0), 5.0);
	}

	//re-import
	parser.importTo(&project, selectedPathes);

	//check the folder structure and the value of the (0,0)-cell again
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
	parser.setProjectFileName(m_dataDir + QLatin1String("origin8_test_workbook.opj"));
	Project project;

	//import "Book1"
	QStringList selectedPathes = {QLatin1String("origin8_test_workbook/Folder1/Book1"), QLatin1String("origin8_test_workbook/Folder1"), QLatin1String("origin8_test_workbook")};
	parser.importTo(&project, selectedPathes);

	//first child of folder "Folder1", workbook "Book1" with one sheet -> import into a spreadsheet
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

	//check the values in the imported columns
	if (spreadsheet != nullptr)
		QCOMPARE(spreadsheet->columnCount(), 6);

	//1st column, Origin::TextNumeric:
	//first non-empty value is numerical, column is set to Numeric, empty or text values in the column a set to NAN
	Column* column = spreadsheet->column(0);
	QCOMPARE(column->columnMode(), AbstractColumn::ColumnMode::Numeric);
	QCOMPARE(!std::isnan(column->valueAt(0)), false);
	QCOMPARE(column->valueAt(1), 1.1);
	QCOMPARE(column->valueAt(2), 2.2);
	QCOMPARE(!std::isnan(column->valueAt(3)), false);
	QCOMPARE(!std::isnan(column->valueAt(4)), false);

	//2nd column, Origin::TextNumeric:
	//first non-empty value is string, the column is set to Text, numerical values are converted to strings
	column = spreadsheet->column(1);
	QCOMPARE(column->columnMode(), AbstractColumn::ColumnMode::Text);
	QCOMPARE(column->textAt(0).isEmpty(), true);
	QCOMPARE(column->textAt(1), QLatin1String("a"));
	QCOMPARE(column->textAt(2), QLatin1String("b"));
	QCOMPARE(column->textAt(3), QLatin1String("1.1"));
	QCOMPARE(column->textAt(4), QLatin1String("2.2"));

	//3rd column, Origin::TextNumeric:
	//first is numerical, column is set to Numeric, empty or text values in the column a set to NAN
	column = spreadsheet->column(2);
	QCOMPARE(column->columnMode(), AbstractColumn::ColumnMode::Numeric);
	QCOMPARE(column->valueAt(0), 1.1);
	QCOMPARE(column->valueAt(1), 2.2);
	QCOMPARE(!std::isnan(column->valueAt(2)), false);
	QCOMPARE(!std::isnan(column->valueAt(3)), false);
	QCOMPARE(column->valueAt(4), 3.3);

	//4th column, Origin::TextNumeric:
	//first value is string, the column is set to Text, numerical values are converted to strings
	column = spreadsheet->column(3);
	QCOMPARE(column->columnMode(), AbstractColumn::ColumnMode::Text);
	QCOMPARE(column->textAt(0), QLatin1String("a"));
	QCOMPARE(column->textAt(1), QLatin1String("b"));
	QCOMPARE(column->textAt(2), QLatin1String("1.1"));
	QCOMPARE(column->textAt(3), QLatin1String("2.2"));
	QCOMPARE(column->textAt(4), QLatin1String("c"));

	//5th column, Origin::Numeric
	//column is set to Numeric, empty values in the column a set to NAN
	column = spreadsheet->column(4);
	QCOMPARE(column->columnMode(), AbstractColumn::ColumnMode::Numeric);
	QCOMPARE(!std::isnan(column->valueAt(0)), false);
	QCOMPARE(column->valueAt(1), 1.1);
	QCOMPARE(column->valueAt(2), 2.2);
	QCOMPARE(column->valueAt(3), 3.3);
	QCOMPARE(!std::isnan(column->valueAt(4)), false);

	//6th column, Origin::Numeric
	//column is set to Numeric, empty values in the column a set to NAN
	column = spreadsheet->column(5);
	QCOMPARE(column->columnMode(), AbstractColumn::ColumnMode::Numeric);
	QCOMPARE(column->valueAt(0), 1.1);
	QCOMPARE(column->valueAt(1), 2.2);
	QCOMPARE(column->valueAt(2), 3.3);
	QCOMPARE(!std::isnan(column->valueAt(3)), false);
	QCOMPARE(!std::isnan(column->valueAt(4)), false);
}

void ProjectImportTest::testParseOriginTags_data()
{
	QTest::addColumn<QString>("originTag");
	QTest::addColumn<QString>("labPlotHTML");

	QTest::newRow("bold") << "\\b(bold)" << "<b>bold</b>";

	QTest::newRow("italic") << "\\i(italic)" << "<i>italic</i>";

	QTest::newRow("strike through") << "\\s(strike through)" << "<s>strike through</s>";

	QTest::newRow("underlined") << "\\u(underlined)" << "<u>underlined</u>";

	QTest::newRow("greek char") << "\\g(a)" << "<font face=Symbol>a</font>";

	QTest::newRow("sub-script") << "a\\-(b)" << "a<sub>b</sub>";

	QTest::newRow("super-script") << "a\\+(b)" << "a<sup>b</sup>";

	QTest::newRow("set-font") << "\\f:dejavu sans(text)" << "<font face=\"dejavu sans\">text</font>";

	QTest::newRow("font-size")	<< "some \\p200(big) text"
								<< "some <span style=\"font-size: 200%\">big</span> text";

	QTest::newRow("color")	<< "some \\c15(colored) text"
							<< "some <span style=\"color: #8000ff\">colored</span> text";

	QTest::newRow("nested-non-tag-parenthesis")
			<< "\\b(text (c) and (fh) and a(t) and empty ())"
			<< "<b>text (c) and (fh) and a(t) and empty ()</b>";

	QTest::newRow("nested-tags")
			<< "\\b(bold text with some \\i(italic) bits and some \\c15(color)) "
			   "then a change of \\f:dejavu sans(font)"
			<< "<b>bold text with some <i>italic</i> bits and some "
			   "<span style=\"color: #8000ff\">color</span></b> "
			   "then a change of <font face=\"dejavu sans\">font</font>";

	QTest::newRow("nested-tags-with-extra-spaces")
			<< "\\ b (bold text with some \\ i(italic) bits and some \\c 15 (color)) "
			   "then a change of \\ f:dejavu sans( font)"
			<< "<b>bold text with some <i>italic</i> bits and some "
			   "<span style=\"color: #8000ff\">color</span></b> "
			   "then a change of <font face=\"dejavu sans\"> font</font>";
}

void ProjectImportTest::testParseOriginTags()
{
	QFETCH(QString, originTag);
	QFETCH(QString, labPlotHTML);

	OriginProjectParser parser;
	QCOMPARE(parser.parseOriginTags(originTag), labPlotHTML);
}

#endif

QTEST_MAIN(ProjectImportTest)
