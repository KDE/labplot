/*
	File                 : DatasetsUnitTest.h
	Project              : LabPlot
	Description          : Tests for Dataset related features
	--------------------------------------------------------------------
    SPDX-FileCopyrightText: 2019 Kovacs Ferencz <kferike98@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/
#ifndef MQTTUNITTEST_H
#define MQTTUNITTEST_H

#include <QtTest>

class DatasetsUnitTest : public QObject {

	Q_OBJECT	

private slots:
	void initTestCase();

	//Test processing metadata files.
	void testCategories();
	void testSubcategories();
	void testDatasets();

	//Test processing and downloading dataset
	void testProcessDataset();

	//Test adding new datasets to the existing collection
	void testNewCollection();
	void testNewCategory();
	void testNewSubcategory();
	void testNewDataset();



private:	
	void copyFiles();
	void removeFiles();

	QString m_dataDir;
	QString m_jsonDir;
};

#endif
