/*
	File                 : NSLSFWindowTest.cpp
	Project              : LabPlot
	Description          : NSL Tests for special window functions
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2019 Stefan Gerlach <stefan.gerlach@uni.kn>

	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "NSLSFWindowTest.h"

extern "C" {
#include "backend/nsl/nsl_sf_window.h"
}

// ##############################################################################
// #################  window types
// ##############################################################################

void NSLSFWindowTest::testWindowTypes() {
	const int N = 10;
	double data[] = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1};
	double result[][N] = {
		{1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
		{0.1, 0.3, 0.5, 0.7, 0.9, 0.9, 0.7, 0.5, 0.3, 0.1},
		{0, 2 / 9., 4 / 9., 6 / 9., 8 / 9., 8 / 9., 6 / 9., 4 / 9., 2 / 9., 0},
		{2 / 11., 4 / 11., 6 / 11., 8 / 11., 10 / 11., 10 / 11., 8 / 11., 6 / 11., 4 / 11., 2 / 11.},
		{0.330578512396694,
		 0.59504132231405,
		 0.793388429752066,
		 0.925619834710744,
		 0.991735537190083,
		 0.991735537190083,
		 0.925619834710744,
		 0.793388429752066,
		 0.59504132231405,
		 0.330578512396694},
		{0, 0.116977778440511, 0.413175911166535, 0.75, 0.969846310392954, 0.969846310392954, 0.75, 0.413175911166535, 0.116977778440511, 0},
		{0.08, 0.18761955616527, 0.460121838273212, 0.77, 0.972258605561518, 0.972258605561518, 0.77, 0.460121838273212, 0.18761955616527, 0.08},
		{0, 0.0508696326538654, 0.258000501503662, 0.63, 0.951129865842472, 0.951129865842472, 0.63, 0.258000501503662, 0.0508696326538655, 0},
		{0, 0.0137486265628393, 0.141900826716656, 0.514746, 0.930560546720505, 0.930560546720505, 0.514746, 0.141900826716656, 0.0137486265628393, 0},
		{0.0003628,
		 0.01789099867138,
		 0.15559612641629,
		 0.5292298,
		 0.933220224912329,
		 0.93322022491233,
		 0.5292298,
		 0.155596126416291,
		 0.0178909986713801,
		 0.0003628},
		{6.e-05, 0.0150711734102182, 0.147039557862381, 0.520575, 0.9316592687274, 0.931659268727401, 0.520575, 0.147039557862382, 0.0150711734102182, 6.e-05},
		{0, -0.0867710194112928, -0.331895219303666, 0.918, 4.00066623871496, 4.00066623871496, 0.918, -0.331895219303665, -0.0867710194112926, 0},
		{0,
		 0.342020143325669,
		 0.642787609686539,
		 0.866025403784439,
		 0.984807753012208,
		 0.984807753012208,
		 0.866025403784439,
		 0.642787609686539,
		 0.342020143325669,
		 0},
		{0, 0.142236444948122, 0.420680359153233, 0.73, 0.950416529231978, 0.950416529231979, 0.73, 0.420680359153233, 0.142236444948122, 0},
		{0,
		 0.263064408273866,
		 0.564253278793615,
		 0.826993343132688,
		 0.979815536051017,
		 0.979815536051017,
		 0.826993343132688,
		 0.564253278793615,
		 0.263064408273866,
		 0}};

	for (int t = (int)nsl_sf_window_uniform; t <= (int)nsl_sf_window_lanczos; t++) {
		nsl_sf_apply_window(data, N, (nsl_sf_window_type)t);
		for (int i = 0; i < N; i++)
			QCOMPARE(data[i] + 1., result[t][i] + 1.);
	}
}

// ##############################################################################
// #################  performance
// ##############################################################################

void NSLSFWindowTest::testPerformance_triangle() {
	const int N = 1e6;
	double* data = new double[N];

	QBENCHMARK {
		nsl_sf_apply_window(data, N, nsl_sf_window_triangle);
	}
	delete[] data;
}

void NSLSFWindowTest::testPerformance_welch() {
	const int N = 1e6;
	double* data = new double[N];

	QBENCHMARK {
		nsl_sf_apply_window(data, N, nsl_sf_window_welch);
	}
	delete[] data;
}

void NSLSFWindowTest::testPerformance_flat_top() {
	const int N = 1e6;
	double* data = new double[N];

	QBENCHMARK {
		nsl_sf_apply_window(data, N, nsl_sf_window_flat_top);
	}
	delete[] data;
}

QTEST_MAIN(NSLSFWindowTest)
