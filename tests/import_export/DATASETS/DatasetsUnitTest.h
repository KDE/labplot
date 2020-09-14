/***************************************************************************
	File                 : DatasetsUnitTest.h
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
