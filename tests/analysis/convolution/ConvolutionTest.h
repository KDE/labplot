/***************************************************************************
    File                 : ConvolutionTest.h
    Project              : LabPlot
    Description          : Tests for data convolution
    --------------------------------------------------------------------
    Copyright            : (C) 2018 Stefan Gerlach (stefan.gerlach@uni.kn)
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
#ifndef CONVOLUTIONTEST_H
#define CONVOLUTIONTEST_H

#include <QtTest>
#include <backend/lib/macros.h>	// DEBUG()

extern "C" {
#include <gsl/gsl_math.h>
}

class ConvolutionTest : public QObject {
	Q_OBJECT

private slots:
	void initTestCase();

	// linear tests
	void testLinear();
	void testLinear_noX();
	void testLinear_swapped();
	void testLinear_swapped_noX();
	void testLinear_norm();
	void testLinear_swapped_norm();
//TODO
//	void testLinear_wrapMax();
//	void testLinear_swapped_wrapMax();
//	void testLinear_wrapCenter();
//	void testLinear_swapped_wrapCenter();

	//TODO: circular tests
};
#endif
