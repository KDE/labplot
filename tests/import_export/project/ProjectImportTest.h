/***************************************************************************
    File                 : ProjectImportTest.h
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
#ifndef PROJECTIMPORTTEST_H
#define PROJECTIMPORTTEST_H

#include <QtTest>

class ProjectImportTest : public QObject {
	Q_OBJECT

private slots:
	void initTestCase();

	//import of LabPlot projects

	//import of Origin projects
	void testOrigin01();
	void testOrigin02();
	void testOrigin03();
	void testOrigin04();
	void testOriginTextNumericColumns();

private:
	QString m_dataDir;
};
#endif
