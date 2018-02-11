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
#include "backend/datasources/projects/OriginProjectParser.h"
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


//##############################################################################
//######################  import of Origin projects ############################
//##############################################################################
//project tree of the file "origin8_test_tree_import.opj"
/*
test_tree_import\
					\Folder
						\Book
							\Sheet1
							\Sheet2
						\MBook
							\MSheet1
							\MSheet2
					\Folder1
						\BookA
							\Sheet1
						\BookB
							\Sheet1
						\Graph
						\MBook
							\MSheet1
					\BookC
*/

void ProjectImportTest::testOrigin01() {
	//TODO: activate this test once the crash with MBook2 is fixed
	return;

	//import the opj file into LabPlot's project object
	OriginProjectParser parser;
	parser.setProjectFileName(m_dataDir + QLatin1String("origin8_test_tree_import.opj"));
	Project project;
	parser.importTo(&project, QStringList());

	//check the project tree for the imported project

	//first child of the root folde, folder "Folder" -> import into a Folder
	AbstractAspect* aspect = project.child<AbstractAspect>(0);
	QCOMPARE(aspect != nullptr, true);
	QCOMPARE(aspect->name(), QLatin1String("Folder"));
	QCOMPARE(dynamic_cast<Folder*>(aspect) != nullptr, true);

	//first child of "Folder", workbook "Book" with two sheets -> import into a Workbook with two Spreadsheets
	aspect = project.child<AbstractAspect>(0)->child<AbstractAspect>(0);
	QCOMPARE(aspect != nullptr, true);
	QCOMPARE(aspect->name(), QLatin1String("Book"));
	QCOMPARE(dynamic_cast<Workbook*>(aspect) != nullptr, true);

	aspect = project.child<AbstractAspect>(0)->child<AbstractAspect>(0)->child<AbstractAspect>(0);
	QCOMPARE(aspect != nullptr, true);
	QCOMPARE(aspect->name(), QLatin1String("Sheet1"));
	QCOMPARE(dynamic_cast<Spreadsheet*>(aspect) != nullptr, true);

	aspect = project.child<AbstractAspect>(0)->child<AbstractAspect>(0)->child<AbstractAspect>(1);
	QCOMPARE(aspect != nullptr, true);
	QCOMPARE(aspect->name(), QLatin1String("Sheet2"));
	QCOMPARE(dynamic_cast<Spreadsheet*>(aspect) != nullptr, true);

	//second child of "Folder", matrixbook "MBook" with two matrix sheets -> import into a Workbook with two Matrices
	aspect = project.child<AbstractAspect>(0)->child<AbstractAspect>(1);
	QCOMPARE(aspect != nullptr, true);
	QCOMPARE(aspect->name(), QLatin1String("MBook"));
	QCOMPARE(dynamic_cast<Workbook*>(aspect) != nullptr, true);

	aspect = project.child<AbstractAspect>(0)->child<AbstractAspect>(1)->child<AbstractAspect>(0);
	QCOMPARE(aspect != nullptr, true);
	QCOMPARE(aspect->name(), QLatin1String("MSheet1"));
	QCOMPARE(dynamic_cast<Matrix*>(aspect) != nullptr, true);

	aspect = project.child<AbstractAspect>(0)->child<AbstractAspect>(1)->child<AbstractAspect>(1);
	QCOMPARE(aspect != nullptr, true);
	QCOMPARE(aspect->name(), QLatin1String("MSheet2"));
	QCOMPARE(dynamic_cast<Matrix*>(aspect) != nullptr, true);


	//second child of the root folder, folder "Folder1" -> import into a Folder
	aspect = project.child<AbstractAspect>(1);
	QCOMPARE(aspect != nullptr, true);
	QCOMPARE(aspect->name(), QLatin1String("Folder1"));
	QCOMPARE(dynamic_cast<Folder*>(aspect) != nullptr, true);

	//first child of "Folder1", worbook "BookA" with on sheet -> import into a Spreadsheet
	aspect = project.child<AbstractAspect>(1)->child<AbstractAspect>(0);
	QCOMPARE(aspect != nullptr, true);
	QCOMPARE(aspect->name(), QLatin1String("Sheet1"));
	QCOMPARE(dynamic_cast<Spreadsheet*>(aspect) != nullptr, true);

	//second child of "Folder1", worbook "BookB" with on sheet -> import into a Spreadsheet
	aspect = project.child<AbstractAspect>(1)->child<AbstractAspect>(1);
	QCOMPARE(aspect != nullptr, true);
	QCOMPARE(aspect->name(), QLatin1String("Sheet1"));
	QCOMPARE(dynamic_cast<Spreadsheet*>(aspect) != nullptr, true);

	//third child of Folder, graph "Graph"-> import into a Worksheet
	aspect = project.child<AbstractAspect>(1)->child<AbstractAspect>(2);
	QCOMPARE(aspect != nullptr, true);
	QCOMPARE(aspect->name(), QLatin1String("Graph"));
	QCOMPARE(dynamic_cast<Worksheet*>(aspect) != nullptr, true);
	//TODO: check the created plot in the worksheet

	//fourth child of "Folder1", matrixbook "MBook" with one sheet -> import into a Matrix
	aspect = project.child<AbstractAspect>(1)->child<AbstractAspect>(3);
	QCOMPARE(aspect != nullptr, true);
	QCOMPARE(aspect->name(), QLatin1String("MSheet1"));
	QCOMPARE(dynamic_cast<Matrix*>(aspect) != nullptr, true);


	//third child of the root folder, workbook "BookC" with one sheet -> import into a Spreadsheet
	aspect = project.child<AbstractAspect>(2);
	QCOMPARE(aspect != nullptr, true);
	QCOMPARE(aspect->name(), QLatin1String("Sheet1"));
	QCOMPARE(dynamic_cast<Spreadsheet*>(aspect) != nullptr, true);
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
	AbstractAspect* aspect = project.child<AbstractAspect>(0);
	QCOMPARE(aspect != nullptr, true);
	QCOMPARE(aspect->name(), QLatin1String("Folder1"));
	QCOMPARE(dynamic_cast<Folder*>(aspect) != nullptr, true);
	QCOMPARE(aspect->childCount<AbstractAspect>(), 1);

	//first child of "Folder", workbook "Book" with one sheet -> import into a Spreadsheet
	aspect = project.child<AbstractAspect>(0)->child<AbstractAspect>(0);
	QCOMPARE(aspect != nullptr, true);
	QCOMPARE(aspect->name(), QLatin1String("Book1"));
	QCOMPARE(dynamic_cast<Spreadsheet*>(aspect) != nullptr, true);
}

/*
 * 1. import one single folder child
 * 2. import another folder child
 * 3. check that both children are available after the second import
 */
void ProjectImportTest::testOrigin03() {
	//TODO: this test fails at the moment -> we need to implement this logic first
	return;

	//import one single object
	OriginProjectParser parser;
	parser.setProjectFileName(m_dataDir + QLatin1String("origin8_test_tree_import.opj"));
	Project project;

	//import one single child in "Folder1"
	QStringList selectedPathes = {QLatin1String("test_tree_import/Folder1/Book1"), QLatin1String("test_tree_import/Folder1"), QLatin1String("test_tree_import")};
	parser.importTo(&project, selectedPathes);

	//first child of the root folder, folder "Folder1" -> import into a Folder
	AbstractAspect* aspect = project.child<AbstractAspect>(0);
	QCOMPARE(aspect != nullptr, true);
	QCOMPARE(aspect->name(), QLatin1String("Folder1"));
	QCOMPARE(dynamic_cast<Folder*>(aspect) != nullptr, true);
	QCOMPARE(aspect->childCount<AbstractAspect>(), 1);

	//first child of "Folder", workbook "Book1" with one sheet -> import into a Spreadsheet
	aspect = project.child<AbstractAspect>(0)->child<AbstractAspect>(0);
	QCOMPARE(aspect != nullptr, true);
	QCOMPARE(aspect->name(), QLatin1String("Book1"));
	QCOMPARE(dynamic_cast<Spreadsheet*>(aspect) != nullptr, true);


	//import another child in "Folder1"
	selectedPathes.clear();
	selectedPathes << QLatin1String("test_tree_import/Folder1/Book4") << QLatin1String("test_tree_import/Folder1") << QLatin1String("test_tree_import");
	parser.importTo(&project, selectedPathes);

	//the first child should still be available in the project -> check it
	aspect = project.child<AbstractAspect>(0);
	QCOMPARE(aspect != nullptr, true);
	QCOMPARE(aspect->name(), QLatin1String("Folder1"));
	QCOMPARE(dynamic_cast<Folder*>(aspect) != nullptr, true);

	aspect = project.child<AbstractAspect>(0)->child<AbstractAspect>(0);
	QCOMPARE(aspect != nullptr, true);
	QCOMPARE(aspect->name(), QLatin1String("Book1"));
	QCOMPARE(dynamic_cast<Spreadsheet*>(aspect) != nullptr, true);

	//check the second child, workbook "Book4" with one sheet -> import into a Spreadsheet
	aspect = project.child<AbstractAspect>(0)->child<AbstractAspect>(1);
	QCOMPARE(aspect != nullptr, true);
	QCOMPARE(aspect->name(), QLatin1String("Book4"));
	QCOMPARE(dynamic_cast<Spreadsheet*>(aspect) != nullptr, true);

	QCOMPARE(aspect->childCount<AbstractAspect>(), 2);
}

QTEST_MAIN(ProjectImportTest)
