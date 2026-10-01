/*
	File                 : NSLSortTest.cpp
	Project              : LabPlot
	Description          : Tests for NSL sorting helpers
	--------------------------------------------------------------------
		SPDX-FileCopyrightText: 2026 Stefan Gerlach <stefan.gerlach@uni.kn>

	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "NSLSortTest.h"

#include <iterator>

extern "C" {
#include "backend/nsl/nsl_sort.h"
}

void NSLSortTest::testSizeTOrdering() {
	size_t lhs = 2;
	size_t rhs = 7;
	QVERIFY(nsl_sort_compare_size_t(&lhs, &rhs) < 0);
	QVERIFY(nsl_sort_compare_size_t(&rhs, &lhs) > 0);
	QCOMPARE(nsl_sort_compare_size_t(&lhs, &lhs), 0);

	size_t values[] = {7, 2, 5, 2, 1};
	nsl_sort_size_t(values, std::size(values));
	const size_t expected[] = {1, 2, 2, 5, 7};
	for (size_t i = 0; i < std::size(values); ++i)
		QCOMPARE(values[i], expected[i]);
}

QTEST_MAIN(NSLSortTest)