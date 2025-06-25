/*
	File                 : DatasetsTest.cpp
	Project              : LabPlot
	Description          : Tests for Dataset related features
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2019 Kovacs Ferencz <kferike98@gmail.com>
	SPDX-FileCopyrightText: 2023 Stefan Gerlach <stefan.gerlach@uni.kn>

	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "DatasetsTest.h"

#include "backend/datasources/DatasetHandler.h"
#include "backend/datasources/filters/AsciiFilter.h"
#include "backend/lib/macros.h"
#include "backend/spreadsheet/Spreadsheet.h"
#include "frontend/DatasetModel.h"
#include "frontend/datasources/DatasetMetadataManagerDialog.h"
#include "frontend/datasources/ImportDatasetWidget.h"

#include <QDebug>
#include <QEventLoop>
#include <QFile>
#include <QStandardPaths>
#include <QTextStream>
#include <QTimer>
#include <QTreeWidgetItem>
#include <QVector>

void DatasetsTest::initTestCase() {
	CommonTest::initTestCase();

	const QString currentDir = QStringLiteral(__FILE__);
	m_dataDir = currentDir.left(currentDir.lastIndexOf(QDir::separator())) + QDir::separator() + QLatin1String("data") + QDir::separator();

	const QString baseDir = QStandardPaths::standardLocations(QStandardPaths::AppDataLocation).first();
	const QString containingDir = QStringLiteral("labplot_data");
	m_jsonDir = baseDir + QDir::separator() + containingDir + QDir::separator();

	removeFiles();
	copyFiles();
}

void DatasetsTest::copyFiles() {
	DEBUG(Q_FUNC_INFO)
	const QString baseDir = QStandardPaths::standardLocations(QStandardPaths::AppDataLocation).first();

	if (!QDir(baseDir).exists())
		QDir().mkdir(baseDir);

	if (!QDir(m_jsonDir).exists())
		QDir().mkdir(m_jsonDir);

	QFile::copy(m_dataDir + QStringLiteral("Test.json"), m_jsonDir + QStringLiteral("Test.json"));
	QFile::copy(m_dataDir + QStringLiteral("DatasetCollections.json"), m_jsonDir + QStringLiteral("DatasetCollections.json"));
}

// ##############################################################################
// #####################  Test processing metadata files  #######################
// ##############################################################################

void DatasetsTest::removeFiles() {
	DEBUG(Q_FUNC_INFO)
	const QString baseDir = QStandardPaths::standardLocations(QStandardPaths::AppDataLocation).first();
	const QString containingDir = QStringLiteral("labplot_data");
	const QString jsonDir = baseDir + QDir::separator() + containingDir + QDir::separator();

	QDir deleteDir(jsonDir);
	deleteDir.setNameFilters(QStringList() << QStringLiteral("*.*"));
	deleteDir.setFilter(QDir::Files);

	for (QString dirFile : deleteDir.entryList()) {
		deleteDir.remove(dirFile);
	}
}

void DatasetsTest::testCategories() {
	DEBUG(Q_FUNC_INFO)
	copyFiles();
	// m_jsonDir = QFINDTESTDATA(QLatin1String("data/Test.json"));
	/*	auto* importWidget = new ImportDatasetWidget(nullptr);
		auto* model = new DatasetModel(importWidget->getDatasetsMap());

		QCOMPARE(model->categories(QStringLiteral("Test")).size(), 3);
		QCOMPARE(model->categories(QStringLiteral("Test")).contains(QStringLiteral("Test_Cat")), true);
		QCOMPARE(model->categories(QStringLiteral("Test")).contains(QStringLiteral("Test_Cat_2")), true);
		QCOMPARE(model->categories(QStringLiteral("Test")).contains(QStringLiteral("Test_Cat_3")), true);

		delete importWidget;
		delete model;
	*/
}

/*void DatasetsTest::testSubcategories() {
	auto* importWidget = new ImportDatasetWidget(nullptr);
	auto* model = new DatasetModel(importWidget->getDatasetsMap());

	QCOMPARE(model->subcategories(QStringLiteral("Test"), QStringLiteral("Test_Cat")).size(), 4);
	QCOMPARE(model->subcategories(QStringLiteral("Test"), QStringLiteral("Test_Cat")).contains(QStringLiteral("Test_Subcat")), true);
	QCOMPARE(model->subcategories(QStringLiteral("Test"), QStringLiteral("Test_Cat")).contains(QStringLiteral("Test_Subcat2")), true);
	QCOMPARE(model->subcategories(QStringLiteral("Test"), QStringLiteral("Test_Cat")).contains(QStringLiteral("Test_Subcat3")), true);
	QCOMPARE(model->subcategories(QStringLiteral("Test"), QStringLiteral("Test_Cat")).contains(QStringLiteral("Test_Subcat4")), true);

	delete importWidget;
	delete model;
}

void DatasetsTest::testDatasets() {
	auto* importWidget = new ImportDatasetWidget(nullptr);
	auto* model = new DatasetModel(importWidget->getDatasetsMap());

	QCOMPARE(model->datasets(QStringLiteral("Test"), QStringLiteral("Test_Cat"), QStringLiteral("Test_Subcat")).size(), 1);
	QCOMPARE(model->datasets(QStringLiteral("Test"), QStringLiteral("Test_Cat"), QStringLiteral("Test_Subcat")).contains(QStringLiteral("test1")), true);
	QCOMPARE(model->allDatasetsList().toStringList().size(), 6);

	delete importWidget;
	delete model;
}

//##############################################################################
//###################  Test processing and downloading dataset  ################
//##############################################################################

void DatasetsTest::testProcessDataset() {
	auto* importWidget = new ImportDatasetWidget(nullptr);

	auto* spreadsheet = new Spreadsheet(QStringLiteral("test"));
	auto* datasetHandler = new DatasetHandler(spreadsheet);
	importWidget->setCollection(QStringLiteral("Test"));
	importWidget->setCategory(QStringLiteral("Test_Cat"));
	importWidget->setSubcategory(QStringLiteral("Test_Subcat"));
	importWidget->setDataset(QStringLiteral("test1"));
	importWidget->import(datasetHandler);

	QTimer timer;
	timer.setSingleShot(true);
	QEventLoop loop;
	connect(datasetHandler, &DatasetHandler::downloadCompleted, &loop, &QEventLoop::quit);
	connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);
	timer.start(1500);
	loop.exec();

	bool datasetFound = false;

	if (timer.isActive()) {
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
	QCOMPARE(secondColumn->textAt(3), QStringLiteral("4/86"));
	QCOMPARE(thirdColumn->valueAt(0), -0.061134);
	QCOMPARE(fourthColumn->valueAt(0), 0.03016);

	delete importWidget;
	delete spreadsheet;
	delete datasetHandler;
}

//##############################################################################
//###########  Test adding new datasets to the existing collection  ############
//##############################################################################

void DatasetsTest::testNewCollection() {
	removeFiles();
	copyFiles();

	auto* importWidget = new ImportDatasetWidget(nullptr);
	auto* datasetDialog = new DatasetMetadataManagerDialog(nullptr, importWidget->getDatasetsMap());

	delete importWidget;

	datasetDialog->setCollection(QStringLiteral("Test2"));
	datasetDialog->setCategory(QStringLiteral("Test_Cat"));
	datasetDialog->setSubcategory(QStringLiteral("Test_Subcat"));
	datasetDialog->setShortName(QStringLiteral("test_new"));
	datasetDialog->setFullName(QStringLiteral("New test dataset"));
	datasetDialog->setDescription(QStringLiteral("This is a new test dataset"));
	datasetDialog->setURL(QStringLiteral("www.testdataset.com"));
	datasetDialog->updateDocument(m_jsonDir);

	importWidget = new ImportDatasetWidget(nullptr);
	auto* model = new DatasetModel(importWidget->getDatasetsMap());

	QCOMPARE(model->collections().size(), 2);
	QCOMPARE(model->allDatasetsList().toStringList().size(), 7);
}

void DatasetsTest::testNewCategory() {
	removeFiles();
	copyFiles();

	auto* importWidget = new ImportDatasetWidget(nullptr);
	auto* datasetDialog = new DatasetMetadataManagerDialog(nullptr, importWidget->getDatasetsMap());

	delete importWidget;

	datasetDialog->setCollection(QStringLiteral("Test"));
	datasetDialog->setCategory(QStringLiteral("Test_Cat_4"));
	datasetDialog->setSubcategory(QStringLiteral("Test_Subcat"));
	datasetDialog->setShortName(QStringLiteral("test_new"));
	datasetDialog->setFullName(QStringLiteral("New test dataset"));
	datasetDialog->setDescription(QStringLiteral("This is a new test dataset"));
	datasetDialog->setURL(QStringLiteral("www.testdataset.com"));
	datasetDialog->updateDocument(m_jsonDir);

	importWidget = new ImportDatasetWidget(nullptr);
	auto* model = new DatasetModel(importWidget->getDatasetsMap());

	QCOMPARE(model->categories(QStringLiteral("Test")).size(), 4);
	QCOMPARE(model->allDatasetsList().toStringList().size(), 7);
}

void DatasetsTest::testNewSubcategory() {
	removeFiles();
	copyFiles();

	auto* importWidget = new ImportDatasetWidget(nullptr);
	auto* datasetDialog = new DatasetMetadataManagerDialog(nullptr, importWidget->getDatasetsMap());

	delete importWidget;

	datasetDialog->setCollection(QStringLiteral("Test"));
	datasetDialog->setCategory(QStringLiteral("Test_Cat"));
	datasetDialog->setSubcategory(QStringLiteral("Test_Subcat5"));
	datasetDialog->setShortName(QStringLiteral("test_new"));
	datasetDialog->setFullName(QStringLiteral("New test dataset"));
	datasetDialog->setDescription(QStringLiteral("This is a new test dataset"));
	datasetDialog->setURL(QStringLiteral("www.testdataset.com"));
	datasetDialog->updateDocument(m_jsonDir);

	importWidget = new ImportDatasetWidget(nullptr);
	auto* model = new DatasetModel(importWidget->getDatasetsMap());

	QCOMPARE(model->subcategories(QStringLiteral("Test"), QStringLiteral("Test_Cat")).size(), 5);
	QCOMPARE(model->allDatasetsList().toStringList().size(), 7);
}

void DatasetsTest::testNewDataset() {
	removeFiles();
	copyFiles();

	auto* importWidget = new ImportDatasetWidget(nullptr);
	auto* datasetDialog = new DatasetMetadataManagerDialog(nullptr, importWidget->getDatasetsMap());

	delete importWidget;

	datasetDialog->setCollection(QStringLiteral("Test"));
	datasetDialog->setCategory(QStringLiteral("Test_Cat"));
	datasetDialog->setSubcategory(QStringLiteral("Test_Subcat"));
	datasetDialog->setShortName(QStringLiteral("test_new"));
	datasetDialog->setFullName(QStringLiteral("New test dataset"));
	datasetDialog->setDescription(QStringLiteral("This is a new test dataset"));
	datasetDialog->setURL(QStringLiteral("www.testdataset.com"));
	datasetDialog->updateDocument(m_jsonDir);

	importWidget = new ImportDatasetWidget(nullptr);
	auto* model = new DatasetModel(importWidget->getDatasetsMap());

	QCOMPARE(model->datasets(QStringLiteral("Test"), QStringLiteral("Test_Cat"), QStringLiteral("Test_Subcat")).size(), 2);
	QCOMPARE(model->allDatasetsList().toStringList().size(), 7);
}*/

QTEST_MAIN(DatasetsTest)
