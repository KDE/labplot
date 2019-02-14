/***************************************************************************
    File                 : NSLDiffTest.h
    Project              : LabPlot
    Description          : NSL Tests for numerical differentiation
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
#ifndef NSLDIFFTEST_H
#define NSLDIFFTEST_H

#include "../NSLTest.h"

class NSLDiffTest : public NSLTest {
	Q_OBJECT

private slots:
	// first derivative
	void testFirst_order2();
	void testFirst_order4();
	void testFirst_avg();
	// second derivative
	void testSecond_order1();
	void testSecond_order2();
	void testSecond_order3();
	// higher derivative
	void testThird_order2();
	void testFourth_order1();
	void testFourth_order3();
	void testFifth_order2();
	void testSixth_order1();
	// performance
	void testPerformance_first();
	void testPerformance_second();
	void testPerformance_third();
private:
	QString m_dataDir;
};
#endif
