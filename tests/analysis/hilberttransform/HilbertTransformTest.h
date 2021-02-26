/***************************************************************************
    File                 : HilbertTransformTest.h
    Project              : LabPlot
    Description          : Tests for Hilbert transform
    --------------------------------------------------------------------
    Copyright            : (C) 2021 Stefan Gerlach (stefan.gerlach@uni.kn)
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
#ifndef HILBERTTRANSFORMTEST_H
#define HILBERTTRANSFORMTEST_H

#include <../AnalysisTest.h>

class HilbertTransformTest : public AnalysisTest {
	Q_OBJECT

private slots:
	void test1();
	void test2();
	void test3();
	void test4();

	void testPerformance();
};
#endif
