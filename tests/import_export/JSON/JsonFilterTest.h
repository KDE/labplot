/***************************************************************************
File                 : JsonFilterTest.h
Project              : LabPlot
Description          : Tests for the JSON I/O-filter.
--------------------------------------------------------------------
--------------------------------------------------------------------
Copyright            : (C) 2018 Andrey Cygankov (craftplace.ms@gmail.com)

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

#ifndef JSONFILTERTEST_H
#define JSONFILTERTEST_H

#include <QtTest>

class JsonFilterTest : public QObject {
	Q_OBJECT

private slots:
	void initTestCase();

	void testArrayImport();
	void testObjectImport01();
	void testObjectImport02();
	void testObjectImport03();
	void testObjectImport04();
};


#endif
