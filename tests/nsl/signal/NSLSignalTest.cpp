/*
	File                 : NSLSignalTest.cpp
	Project              : LabPlot
	Description          : Tests for NSL signal processing algorithms
	--------------------------------------------------------------------
		SPDX-FileCopyrightText: 2026 Stefan Gerlach <stefan.gerlach@uni.kn>

	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "NSLSignalTest.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <utility>

extern "C" {
#include "backend/nsl/nsl_changepoint.h"
#include "backend/nsl/nsl_conv.h"
#include "backend/nsl/nsl_corr.h"
#include "backend/nsl/nsl_hilbert.h"
}

void NSLSignalTest::testChangePoints() {
	double x[] = {0, 1, 2, 3, 4, 5, 6, 7};
	double y[] = {0, 0, 0, 0, 10, 10, 10, 10};
	size_t changepoints[2] = {};

	QCOMPARE(nsl_changepoint_binary_segmentation(x, y, 8, 1., 2, changepoints, 2), size_t(1));
	QCOMPARE(changepoints[0], size_t(4));
	QCOMPARE(nsl_changepoint_pelt(x, y, 8, 1., 2, changepoints, 2), size_t(1));
	QCOMPARE(changepoints[0], size_t(4));
	QCOMPARE(nsl_changepoint_binary_segmentation(x, y, 8, 1., 2, changepoints, 0), size_t(0));
	QCOMPARE(nsl_changepoint_pelt(x, y, 8, 1., 2, changepoints, 0), size_t(0));
}

void NSLSignalTest::testConvolution() {
	const double signal[] = {1, 2, 3};
	double response[] = {1, 1};
	double output[4] = {};
	const double expected[] = {1, 3, 5, 3};

	QCOMPARE(nsl_conv_linear_direct(signal, 3, response, 2, nsl_conv_norm_none, nsl_conv_wrap_none, output), 0);
	for (size_t i = 0; i < 4; ++i)
		QVERIFY(std::abs(output[i] - expected[i]) < 1.e-12);

	double signalCopy[] = {1, 2, 3};
	double responseCopy[] = {1, 1};
	QCOMPARE(nsl_conv_convolution(signalCopy, 3, responseCopy, 2, nsl_conv_type_linear, nsl_conv_method_direct, nsl_conv_norm_none, nsl_conv_wrap_none, output),
			 0);
	for (size_t i = 0; i < 4; ++i)
		QVERIFY(std::abs(output[i] - expected[i]) < 1.e-12);

	QCOMPARE(nsl_conv_convolution_direction(signalCopy,
											3,
											responseCopy,
											2,
											nsl_conv_direction_forward,
											nsl_conv_type_linear,
											nsl_conv_method_direct,
											nsl_conv_norm_none,
											nsl_conv_wrap_none,
											output),
			 0);
	for (size_t i = 0; i < 4; ++i)
		QVERIFY(std::abs(output[i] - expected[i]) < 1.e-12);

	QCOMPARE(nsl_conv_fft_type(signal, 3, responseCopy, 2, nsl_conv_direction_forward, nsl_conv_type_linear, nsl_conv_norm_none, nsl_conv_wrap_none, output),
			 0);
	for (size_t i = 0; i < 4; ++i)
		QVERIFY(std::abs(output[i] - expected[i]) < 1.e-10);

	double deconvolutionInput[] = {1, 3, 5, 3};
	double deconvolutionResponse[] = {1, 1};
	double deconvolutionOutput[5] = {};
	QCOMPARE(nsl_conv_deconvolution(deconvolutionInput,
									4,
									deconvolutionResponse,
									2,
									nsl_conv_type_linear,
									nsl_conv_norm_none,
									nsl_conv_wrap_none,
									deconvolutionOutput),
			 0);

	double circularResponse[] = {1, 1, 1};
	double circularOutput[3] = {};
	QCOMPARE(nsl_conv_circular_direct(signal, 3, circularResponse, 3, nsl_conv_norm_none, nsl_conv_wrap_none, circularOutput), 0);
	for (double value : circularOutput)
		QVERIFY(std::abs(value - 6.) < 1.e-12);

	const std::array<std::pair<nsl_conv_kernel_type, size_t>, 10> kernels = {{{nsl_conv_kernel_avg, 3},
																			  {nsl_conv_kernel_smooth_triangle, 3},
																			  {nsl_conv_kernel_smooth_gaussian, 5},
																			  {nsl_conv_kernel_first_derivative, 2},
																			  {nsl_conv_kernel_smooth_first_derivative, 3},
																			  {nsl_conv_kernel_second_derivative, 3},
																			  {nsl_conv_kernel_third_derivative, 4},
																			  {nsl_conv_kernel_fourth_derivative, 5},
																			  {nsl_conv_kernel_gaussian, 5},
																			  {nsl_conv_kernel_lorentzian, 5}}};
	for (const auto& [type, size] : kernels) {
		std::array<double, 9> kernel{};
		QCOMPARE(nsl_conv_standard_kernel(kernel.data(), size, type), 0);
		for (size_t i = 0; i < size; ++i)
			QVERIFY(std::isfinite(kernel[i]));
	}
	std::array<double, 3> average{};
	QCOMPARE(nsl_conv_standard_kernel(average.data(), average.size(), nsl_conv_kernel_avg), 0);
	for (double value : average)
		QCOMPARE(value, 1.);
}

void NSLSignalTest::testCorrelation() {
	double signal[] = {1, 2, 3};
	double output[5] = {};
	QCOMPARE(nsl_corr_correlation(signal, 3, signal, 3, nsl_corr_type_linear, nsl_corr_norm_none, output), 0);
	const double expected[] = {3, 8, 14, 8, 3};
	for (size_t i = 0; i < 5; ++i)
		QVERIFY(std::abs(output[i] - expected[i]) < 1.e-10);

	QCOMPARE(nsl_corr_fft_type(signal, 3, signal, 3, nsl_corr_type_linear, nsl_corr_norm_coeff, output), 0);
	QVERIFY(std::abs(output[2] - 1.) < 1.e-10);
	QVERIFY(std::abs(output[0] - output[4]) < 1.e-10);
}

void NSLSignalTest::testHilbertTransform() {
	double singleton[] = {2.};
	QCOMPARE(nsl_hilbert_transform(singleton, 1, 1, nsl_hilbert_result_imag), 1);

	double constant[8];
	std::fill(std::begin(constant), std::end(constant), 2.);
	QCOMPARE(nsl_hilbert_transform(constant, 1, 8, nsl_hilbert_result_imag), 0);
	for (double value : constant)
		QVERIFY(std::abs(value) < 1.e-10);

	std::fill(std::begin(constant), std::end(constant), -2.);
	QCOMPARE(nsl_hilbert_transform(constant, 1, 8, nsl_hilbert_result_envelope), 0);
	for (double value : constant)
		QVERIFY(std::abs(value - 2.) < 1.e-10);
}

QTEST_MAIN(NSLSignalTest)