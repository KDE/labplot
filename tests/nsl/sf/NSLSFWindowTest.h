/***************************************************************************
    File                 : NSLSFWindowTest.h
    Project              : LabPlot
    Description          : NSL Tests for special window functions
    --------------------------------------------------------------------
    Copyright            : (C) 2019 Stefan Gerlach (stefan.gerlach@uni.kn)
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
#ifndef NSLSFWINDOWTEST_H
#define NSLSFWINDOWTEST_H

#include <QtTest>

class NSLSFWindowTest : public QObject {
	Q_OBJECT

private slots:
	void initTestCase();

	void testWindowTypes();

	void testPerformance_triangle();
	void testPerformance_welch();
	void testPerformance_flat_top();
private:
	QString m_dataDir;
};
#endif
