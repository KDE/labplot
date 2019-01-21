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

#include <QtTest>

class NSLDFTTest : public QObject {
	Q_OBJECT

private slots:
	void initTestCase();

	// one sided
	void test_onesided_real();
	void test_onesided_imag();
	void test_onesided_magnitude();
	void test_onesided_amplitude();
	void test_onesided_power();
	void test_onesided_phase();
	void test_onesided_dB();
	void test_onesided_squaremagnitude();
	void test_onesided_squareamplitude();
	void test_onesided_normdB();
	// two sided
	void test_twosided_real();
	void test_twosided_imag();
	void test_twosided_magnitude();
	void test_twosided_amplitude();
	void test_twosided_power();
	void test_twosided_phase();
	void test_twosided_dB();
	void test_twosided_squaremagnitude();
	void test_twosided_squareamplitude();
	void test_twosided_normdB();
	// performance
	// TODO
private:
	QString m_dataDir;
};
#endif
