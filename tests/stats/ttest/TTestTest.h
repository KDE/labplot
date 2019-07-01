/***************************************************************************
	File                 : CorrelationTest.h
	Project              : LabPlot
	Description          : Tests for data correlation
	--------------------------------------------------------------------
	Copyright            : (C) 2019 Devanshu Agarwal (agarwaldevanshu8@gmail.com)
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
#ifndef TTESTTEST_H
#define TTESTTEST_H

#include <../../CommonTest.h>

class TTestTest : public CommonTest {
	Q_OBJECT

private slots:
	// Two Sample Independent Test
	void twoSampleIndependent_data();
	void twoSampleIndependent();

	void twoSamplePaired_data();
	void twoSamplePaired();

	void oneSample_data();
	void oneSample();
};
#endif
