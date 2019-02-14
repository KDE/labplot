/***************************************************************************
    File                 : NSLDFTTest.h
    Project              : LabPlot
    Description          : NSL Tests for DFT
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
#ifndef NSLDFTTEST_H
#define NSLDFTTEST_H

#include "../NSLTest.h"

class NSLDFTTest : public NSLTest {
	Q_OBJECT

private slots:
	// one sided
	void testOnesided_real();
	void testOnesided_imag();
	void testOnesided_magnitude();
	void testOnesided_amplitude();
	void testOnesided_power();
	void testOnesided_phase();
	void testOnesided_dB();
	void testOnesided_squaremagnitude();
	void testOnesided_squareamplitude();
	void testOnesided_normdB();
	// two sided
	void testTwosided_real();
	void testTwosided_imag();
	void testTwosided_magnitude();
	void testTwosided_amplitude();
	void testTwosided_power();
	void testTwosided_phase();
	void testTwosided_dB();
	void testTwosided_squaremagnitude();
	void testTwosided_squareamplitude();
	void testTwosided_normdB();
	// performance
	void testPerformance_onesided();
	void testPerformance_twosided();
private:
	QString m_dataDir;
};
#endif
