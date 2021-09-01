/*
	File                 : DatasetsUnitTest.cpp
	Project              : LabPlot
	Description          : Tests for Dataset related features
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2019 Kovacs Ferencz (kferike98@gmail.com)
*/

/***************************************************************************
 *                                                                         *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *                                                                         *
 ***************************************************************************/

#include "DatasetsUnitTest.h"

#include "backend/datasources/filters/AsciiFilter.h"
#include "backend/spreadsheet/Spreadsheet.h"
#include "backend/datasources/DatasetHandler.h"
#include "kdefrontend/DatasetModel.h"
#include "kdefrontend/datasources/ImportDatasetWidget.h"
#include "kdefrontend/datasources/DatasetMetadataManagerDialog.h"

#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <QVector>
#include <QTimer>
#include <QEventLoop>
#include <QTreeWidgetItem>
#include <QStandardPaths>

void DatasetsUnitTest::initTestCase() {	
	const QString currentDir = __FILE__;
	m_dataDir = currentDir.left(currentDir.lastIndexOf(QDir::separator())) + QDir::separator() + QLatin1String("data") + QDir::separator();

	const QString baseDir = QStandardPaths::standardLocations(QStandardPaths::AppDataLocation).first();
	const QString containingDir = "labplot_data";
	m_jsonDir = baseDir + QDir::separator() + containingDir + QDir::separator();

	// needed in order to have the signals triggered by SignallingUndoCommand, see LabPlot.cpp
	//TODO: redesign/remove this
	qRegisterMetaType<const AbstractAspect*>("const AbstractAspect*");
	qRegisterMetaType<const AbstractColumn*>("const AbstractColumn*");

	removeFiles();
	copyFiles();
}

void DatasetsUnitTest::copyFiles() {
	const QString baseDir = QStandardPaths::standardLocations(QStandardPaths::AppDataLocation).first();

	if(!QDir(baseDir).exists())
		QDir().mkdir(baseDir);

	if(!QDir(m_jsonDir).exists())
		QDir().mkdir(m_jsonDir);

	QFile::copy(m_dataDir + "Test.json", m_jsonDir + "Test.json");
	QFile::copy(m_dataDir + "DatasetCollections.json", m_jsonDir + "DatasetCollections.json");
}

//##############################################################################
//#####################  Test processing metadata files  #######################
//##############################################################################

void DatasetsUnitTest::removeFiles() {
	const QString baseDir = QStandardPaths::standardLocations(QStandardPaths::AppDataLocation).first();
	const QString containingDir = "labplot_data";
	const QString jsonDir = baseDir + QDir::separator() + containingDir + QDir::separator();

	QDir deleteDir(jsonDir);
	deleteDir.setNameFilters(QStringList() << "*.*");
	deleteDir.setFilter(QDir::Files);

	for(QString dirFile : deleteDir.entryList())	{
		deleteDir.remove(dirFile);
	}
}

void DatasetsUnitTest::testCategories() {
	copyFiles();
	ImportDatasetWidget* importWidget = new ImportDatasetWidget(nullptr);
	DatasetModel* model = new DatasetModel(importWidget->getDatasetsMap());

	QCOMPARE(model->categories("Test").size(), 3);
	QCOMPARE(model->categories("Test").contains("Test_Cat"), true);
	QCOMPARE(model->categories("Test").contains("Test_Cat_2"), true);
	QCOMPARE(model->categories("Test").contains("Test_Cat_3"), true);

	delete importWidget;
	delete  model;
}

void DatasetsUnitTest::testSubcategories() {
	ImportDatasetWidget* importWidget = new ImportDatasetWidget(nullptr);
	DatasetModel* model = new DatasetModel(importWidget->getDatasetsMap());

	QCOMPARE(model->subcategories("Test", "Test_Cat").size(), 4);
	QCOMPARE(model->subcategories("Test", "Test_Cat").contains("Test_Subcat"), true);
	QCOMPARE(model->subcategories("Test", "Test_Cat").contains("Test_Subcat2"), true);
	QCOMPARE(model->subcategories("Test", "Test_Cat").contains("Test_Subcat3"), true);
	QCOMPARE(model->subcategories("Test", "Test_Cat").contains("Test_Subcat4"), true);

	delete importWidget;
	delete  model;
}

void DatasetsUnitTest::testDatasets() {
	ImportDatasetWidget* importWidget = new ImportDatasetWidget(nullptr);
	DatasetModel* model = new DatasetModel(importWidget->getDatasetsMap());

	QCOMPARE(model->datasets("Test", "Test_Cat", "Test_Subcat").size(), 1);
	QCOMPARE(model->datasets("Test", "Test_Cat", "Test_Subcat").contains("test1"), true);
	QCOMPARE(model->allDatasetsList().toStringList().size(), 6);

	delete importWidget;
	delete  model;
}

//##############################################################################
//###################  Test processing and downloading dataset  ################
//##############################################################################

void DatasetsUnitTest::testProcessDataset() {
	ImportDatasetWidget* importWidget = new ImportDatasetWidget(nullptr);

	Spreadsheet* spreadsheet = new Spreadsheet("test");
	DatasetHandler* datasetHandler = new DatasetHandler(spreadsheet);
	importWidget->setCollection("Test");
	importWidget->setCategory("Test_Cat");
	importWidget->setSubcategory("Test_Subcat");
	importWidget->setDataset("test1");
	importWidget->loadDatasetToProcess(datasetHandler);

	QTimer timer;
	timer.setSingleShot(true);
	QEventLoop loop;
	connect(datasetHandler,  &DatasetHandler::downloadCompleted, &loop, &QEventLoop::quit);
	connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);
	timer.start(1500);
	loop.exec();

	bool datasetFound = false;

	if(timer.isActive()){
		timer.stop();
		datasetFound = true;
	}
	qDebug() << datasetFound;

	QCOMPARE(datasetFound, true);
	QCOMPARE(spreadsheet->rowCount(), 60);
	QCOMPARE(spreadsheet->columnCount(), 4);

	Column* firstColumn = spreadsheet->column(0);
	Column* secondColumn = spreadsheet->column(1);
	Column* thirdColumn = spreadsheet->column(2);
	Column* fourthColumn = spreadsheet->column(3);

	QCOMPARE(firstColumn->valueAt(0), 1);
	QCOMPARE(secondColumn->textAt(3),"4/86");
	QCOMPARE(thirdColumn->valueAt(0), -0.061134);
	QCOMPARE(fourthColumn->valueAt(0), 0.03016);

	delete importWidget;
	delete spreadsheet;
	delete datasetHandler;
}

//##############################################################################
//###########  Test adding new datasets to the existing collection  ############
//##############################################################################

void DatasetsUnitTest::testNewCollection() {
	removeFiles();
	copyFiles();

	ImportDatasetWidget* importWidget = new ImportDatasetWidget(nullptr);
	DatasetMetadataManagerDialog* datasetDialog = new DatasetMetadataManagerDialog(nullptr, importWidget->getDatasetsMap());

	delete importWidget;

	datasetDialog->setCollection("Test2");
	datasetDialog->setCategory("Test_Cat");
	datasetDialog->setSubcategory("Test_Subcat");
	datasetDialog->setShortName("test_new");
	datasetDialog->setFullName("New test dataset");
	datasetDialog->setDescription("This is a new test dataset");
	datasetDialog->setURL("www.testdataset.com");
	datasetDialog->updateDocument(m_jsonDir);

	importWidget = new ImportDatasetWidget(nullptr);
	DatasetModel* model = new DatasetModel(importWidget->getDatasetsMap());

	QCOMPARE(model->collections().size(), 2);
	QCOMPARE(model->allDatasetsList().toStringList().size(), 7);
}

void DatasetsUnitTest::testNewCategory() {
	removeFiles();
	copyFiles();

	ImportDatasetWidget* importWidget = new ImportDatasetWidget(nullptr);
	DatasetMetadataManagerDialog* datasetDialog = new DatasetMetadataManagerDialog(nullptr, importWidget->getDatasetsMap());

	delete importWidget;

	datasetDialog->setCollection("Test");
	datasetDialog->setCategory("Test_Cat_4");
	datasetDialog->setSubcategory("Test_Subcat");
	datasetDialog->setShortName("test_new");
	datasetDialog->setFullName("New test dataset");
	datasetDialog->setDescription("This is a new test dataset");
	datasetDialog->setURL("www.testdataset.com");
	datasetDialog->updateDocument(m_jsonDir);

	importWidget = new ImportDatasetWidget(nullptr);
	DatasetModel* model = new DatasetModel(importWidget->getDatasetsMap());

	QCOMPARE(model->categories("Test").size(), 4);
	QCOMPARE(model->allDatasetsList().toStringList().size(), 7);
}

void DatasetsUnitTest::testNewSubcategory() {
	removeFiles();
	copyFiles();

	ImportDatasetWidget* importWidget = new ImportDatasetWidget(nullptr);
	DatasetMetadataManagerDialog* datasetDialog = new DatasetMetadataManagerDialog(nullptr, importWidget->getDatasetsMap());

	delete importWidget;

	datasetDialog->setCollection("Test");
	datasetDialog->setCategory("Test_Cat");
	datasetDialog->setSubcategory("Test_Subcat5");
	datasetDialog->setShortName("test_new");
	datasetDialog->setFullName("New test dataset");
	datasetDialog->setDescription("This is a new test dataset");
	datasetDialog->setURL("www.testdataset.com");
	datasetDialog->updateDocument(m_jsonDir);

	importWidget = new ImportDatasetWidget(nullptr);
	DatasetModel* model = new DatasetModel(importWidget->getDatasetsMap());

	QCOMPARE(model->subcategories("Test", "Test_Cat").size(), 5);
	QCOMPARE(model->allDatasetsList().toStringList().size(), 7);
}

void DatasetsUnitTest::testNewDataset() {
	removeFiles();
	copyFiles();

	ImportDatasetWidget* importWidget = new ImportDatasetWidget(nullptr);
	DatasetMetadataManagerDialog* datasetDialog = new DatasetMetadataManagerDialog(nullptr, importWidget->getDatasetsMap());

	delete importWidget;

	datasetDialog->setCollection("Test");
	datasetDialog->setCategory("Test_Cat");
	datasetDialog->setSubcategory("Test_Subcat");
	datasetDialog->setShortName("test_new");
	datasetDialog->setFullName("New test dataset");
	datasetDialog->setDescription("This is a new test dataset");
	datasetDialog->setURL("www.testdataset.com");
	datasetDialog->updateDocument(m_jsonDir);

	importWidget = new ImportDatasetWidget(nullptr);
	DatasetModel* model = new DatasetModel(importWidget->getDatasetsMap());

	QCOMPARE(model->datasets("Test", "Test_Cat", "Test_Subcat").size(), 2);
	QCOMPARE(model->allDatasetsList().toStringList().size(), 7);
}

QTEST_MAIN(DatasetsUnitTest)


