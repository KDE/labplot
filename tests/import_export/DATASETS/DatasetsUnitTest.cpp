/***************************************************************************
	File                 : DatasetsUnitTest.cpp
	Project              : LabPlot
	Description          : Tests for Dataset related features
	--------------------------------------------------------------------
	Copyright            : (C) 2019 Kovacs Ferencz (kferike98@gmail.com)
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

#include "DatasetsUnitTest.h"

#include "backend/datasources/filters/AsciiFilter.h"
#include "backend/spreadsheet/Spreadsheet.h"
#include "backend/datasources/DatasetHandler.h"
#include "kdefrontend/DatasetModel.h"
#include "kdefrontend/datasources/ImportDatasetWidget.h"

#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <QVector>
#include <QTimer>
#include <QEventLoop>
#include <QTreeWidgetItem>

void DatasetsUnitTest::initTestCase() {

	// needed in order to have the signals triggered by SignallingUndoCommand, see LabPlot.cpp
	//TODO: redesign/remove this
	qRegisterMetaType<const AbstractAspect*>("const AbstractAspect*");
	qRegisterMetaType<const AbstractColumn*>("const AbstractColumn*");
}

void DatasetsUnitTest::testCategories() {
	ImportDatasetWidget* importWidget = new ImportDatasetWidget(nullptr);
	DatasetModel* model = new DatasetModel(importWidget->getDatasetsMap());

	QCOMPARE(model->testCategories().size(), 3);
	QCOMPARE(model->testCategories().contains("Test_Cat"), true);
	QCOMPARE(model->testCategories().contains("Test_Cat_2"), true);
	QCOMPARE(model->testCategories().contains("Test_Cat_3"), true);

	delete importWidget;
	delete  model;
}

void DatasetsUnitTest::testSubcategories() {
	ImportDatasetWidget* importWidget = new ImportDatasetWidget(nullptr);
	DatasetModel* model = new DatasetModel(importWidget->getDatasetsMap());

	QCOMPARE(model->testSubcategories("Test_Cat").size(), 4);
	QCOMPARE(model->testSubcategories("Test_Cat").contains("Test_Subcat"), true);
	QCOMPARE(model->testSubcategories("Test_Cat").contains("Test_Subcat2"), true);
	QCOMPARE(model->testSubcategories("Test_Cat").contains("Test_Subcat3"), true);
	QCOMPARE(model->testSubcategories("Test_Cat").contains("Test_Subcat4"), true);

	delete importWidget;
	delete  model;
}

void DatasetsUnitTest::testDatasets() {
	ImportDatasetWidget* importWidget = new ImportDatasetWidget(nullptr);
	DatasetModel* model = new DatasetModel(importWidget->getDatasetsMap());

	QCOMPARE(model->testDatasets("Test_Cat", "Test_Subcat").size(), 1);
	QCOMPARE(model->testDatasets("Test_Cat", "Test_Subcat").contains("test1"), true);
	QCOMPARE(model->allTestDatasets().size(), 6);

	delete importWidget;
	delete  model;
}

void DatasetsUnitTest::testProcessDataset() {
	ImportDatasetWidget* importWidget = new ImportDatasetWidget(nullptr);

	Spreadsheet* spreadsheet = new Spreadsheet("test");
	DatasetHandler* datasetHandler = new DatasetHandler(spreadsheet);
	importWidget->processTest("Test_Cat", "Test_Subcat", "test1", datasetHandler);

	QCOMPARE(spreadsheet->rowCount(), 23);
	QCOMPARE(spreadsheet->columnCount(), 2);

	Column* firstColumn = spreadsheet->column(0);
	Column* secondColumn = spreadsheet->column(1);
	QCOMPARE(firstColumn->valueAt(0), "Jan 12");
	QCOMPARE(secondColumn->valueAt(3),1212000);

	delete importWidget;
	delete spreadsheet;
	delete datasetHandler;
}

QTEST_MAIN(DatasetsUnitTest)


