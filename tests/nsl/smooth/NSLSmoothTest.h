/***************************************************************************
    File                 : NSLDiffTest.h
    Project              : LabPlot
    Description          : NSL Tests for smoothing
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
#ifndef NSLSMOOTHTEST_H
#define NSLSMOOTHTEST_H

#include "../NSLTest.h"

class NSLSmoothTest : public NSLTest {
	Q_OBJECT

private slots:
	// moving average tests
	void testMA_padnone();
	void testMA_padmirror();
	void testMA_padnearest();
	void testMA_padconstant();
	void testMA_padperiodic();
	// lagged moving average tests
	void testMAL_padnone();
	void testMAL_padmirror();
	void testMAL_padnearest();
	void testMAL_padconstant();
	void testMAL_padperiodic();
	// percentile tests
	void testPercentile_padnone();
	void testPercentile_padmirror();
	void testPercentile_padnearest();
	void testPercentile_padconstant();
	void testPercentile_padperiodic();
	// Savivitzky-Golay coeff tests
	void testSG_coeff31();
	void testSG_coeff51();
	void testSG_coeff53();
	void testSG_coeff73();
	void testSG_coeff74();

	// TODO: performance
	//void testPerformance();
private:
	QString m_dataDir;
};
#endif
