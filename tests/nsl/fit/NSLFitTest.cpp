/***************************************************************************
    File                 : NSLFitTest.cpp
    Project              : LabPlot
    Description          : NSL Tests for fitting
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

#include "NSLFitTest.h"

extern "C" {
#include "backend/nsl/nsl_fit.h"
}

//##############################################################################
//#################  bound test
//##############################################################################

const int N = 11;
const double data_unbound[] = {-4, -3, -2, -1, -.1, 0, .1, 1, 2, 3, 4};
const double result_bound[] = {1.63520374296189, 0.288319987910199, -0.863946140238523, -0.762206477211845, 0.350249875029758, 0.5, 0.649750124970242, 1.76220647721184, 1.86394614023852, 0.711680012089801, -0.635203742961892};
const double data_bound[]={-1, -.99, -.5, 0, .49, .5, .51, 1, 1.5, 1.99, 2};
const double result_unbound[] = {-1.5707963267949, -1.45526202651066, -0.729727656226966, -0.339836909454122, -0.00666671605037044, 0, 0.00666671605037033, 0.339836909454122, 0.729727656226966, 1.45526202651066, 1.5707963267949};

void NSLFitTest::testBounds() {
	int i;
	for (i = 0; i < N; i++) {
		double x = nsl_fit_map_bound(data_unbound[i], -1, 2);
//		printf("%g -> %.15g\n", data_unbound[i], x);
		QCOMPARE(x, result_bound[i]);
	}

	for (i = 0; i < N; i++) {
		double x = nsl_fit_map_unbound(data_bound[i], -1, 2);
//		printf("%g -> %.15g\n", data_bound[i], x);
		QCOMPARE(x, result_unbound[i]);
	}

}

//##############################################################################
//#################  performance
//##############################################################################

QTEST_MAIN(NSLFitTest)
