/***************************************************************************
    File                 : AsciiFilterTest.h
    Project              : LabPlot
    Description          : Tests for the ascii filter
    --------------------------------------------------------------------
    Copyright            : (C) 2017 Alexander Semke (alexander.semke@web.de)
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
#ifndef ASCIIFILTERTEST_H
#define ASCIIFILTERTEST_H

#include <QtTest>

class AsciiFilterTest : public QObject {
	Q_OBJECT

private slots:
	void initTestCase();

	//empty and sparse files
	void testEmptyFileAppend();
	void testEmptyFilePrepend();
	void testEmptyFileReplace();

	void testEmptyLines01();

	void testSparseFile01();
	void testSparseFile02();
	void testSparseFile03();

	//header handling
	void testHeader01();
	void testHeader02();
	void testHeader03();
	void testHeader04();
	void testHeader05();
	void testHeader06();

	//different separators

	//different locales

	//handling of NANs

	//automatically skip comments

private:
	QString m_dataDir;
};
#endif
