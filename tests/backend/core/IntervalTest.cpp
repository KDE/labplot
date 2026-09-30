/*
	File                 : IntervalTest.cpp
	Project              : LabPlot
	Description          : Tests for Interval
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2026 LabPlot contributors
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "IntervalTest.h"
#include "backend/lib/Interval.h"

void IntervalTest::testConstructionAndValidity() {
	const Interval<int> invalid;
	QVERIFY(!invalid.isValid());

	Interval<int> interval(2, 6);
	QVERIFY(interval.isValid());
	QCOMPARE(interval.size(), 5);
	QCOMPARE(interval.toString(), QStringLiteral("[2,6]"));

	interval.translate(3);
	QCOMPARE(interval.start(), 5);
	QCOMPARE(interval.end(), 9);
	interval.setStart(-1);
	QVERIFY(!interval.isValid());
}

void IntervalTest::testContainmentAndIntersection() {
	const Interval<int> outer(1, 10);
	const Interval<int> inner(3, 7);
	QVERIFY(outer.contains(inner));
	QVERIFY(outer.contains(1));
	QVERIFY(outer.contains(10));
	QVERIFY(!inner.contains(outer));

	QVERIFY(inner.intersects(outer));
	QVERIFY(outer.intersects(inner));
	const auto overlap = Interval<int>::intersection(outer, inner);
	QVERIFY(overlap.isValid());
	QCOMPARE(overlap.start(), 3);
	QCOMPARE(overlap.end(), 7);

	const auto disjoint = Interval<int>::intersection(outer, Interval<int>(12, 14));
	QVERIFY(!disjoint.isValid());
	QVERIFY(!outer.intersects(Interval<int>(12, 14)));
}

void IntervalTest::testTouchAndMerge() {
	const Interval<int> left(1, 3);
	const Interval<int> adjacent(4, 6);
	QVERIFY(left.touches(adjacent));
	QVERIFY(!left.touches(Interval<int>(5, 7)));

	const auto merged = Interval<int>::merge(left, adjacent);
	QCOMPARE(merged.start(), 1);
	QCOMPARE(merged.end(), 6);
	const auto overlapping = Interval<int>::merge(Interval<int>(2, 8), Interval<int>(5, 9));
	QCOMPARE(overlapping.start(), 2);
	QCOMPARE(overlapping.end(), 9);
}

void IntervalTest::testSubtractAndSplit() {
	const auto pieces = Interval<int>::subtract(Interval<int>(1, 9), Interval<int>(4, 6));
	QCOMPARE(pieces.size(), 2);
	QCOMPARE(pieces.at(0).start(), 1);
	QCOMPARE(pieces.at(0).end(), 3);
	QCOMPARE(pieces.at(1).start(), 7);
	QCOMPARE(pieces.at(1).end(), 9);

	QVERIFY(Interval<int>::subtract(Interval<int>(1, 9), Interval<int>(0, 10)).isEmpty());
	const auto unchanged = Interval<int>::subtract(Interval<int>(1, 9), Interval<int>(11, 12));
	QCOMPARE(unchanged.size(), 1);
	QCOMPARE(unchanged.first().start(), 1);
	QCOMPARE(unchanged.first().end(), 9);

	const auto split = Interval<int>::split(Interval<int>(1, 5), 3);
	QCOMPARE(split.size(), 2);
	QCOMPARE(split.at(0).start(), 1);
	QCOMPARE(split.at(0).end(), 2);
	QCOMPARE(split.at(1).start(), 3);
	QCOMPARE(split.at(1).end(), 5);
	const auto outside = Interval<int>::split(Interval<int>(1, 5), 8);
	QCOMPARE(outside.size(), 1);
	QCOMPARE(outside.first().start(), 1);
	QCOMPARE(outside.first().end(), 5);
}

void IntervalTest::testIntervalLists() {
	QVector<Interval<int>> intervals{Interval<int>(1, 3), Interval<int>(8, 10)};
	Interval<int>::mergeIntervalIntoList(&intervals, Interval<int>(4, 7));
	QCOMPARE(intervals.size(), 1);
	QCOMPARE(intervals.first().start(), 1);
	QCOMPARE(intervals.first().end(), 10);

	intervals = {Interval<int>(1, 4), Interval<int>(8, 10), Interval<int>(14, 16)};
	Interval<int>::restrictList(&intervals, Interval<int>(3, 12));
	QCOMPARE(intervals.size(), 2);
	QCOMPARE(intervals.at(0).start(), 3);
	QCOMPARE(intervals.at(0).end(), 4);
	QCOMPARE(intervals.at(1).start(), 8);
	QCOMPARE(intervals.at(1).end(), 10);

	Interval<int>::subtractIntervalFromList(&intervals, Interval<int>(3, 8));
	QCOMPARE(intervals.size(), 1);
	QCOMPARE(intervals.first().start(), 9);
	QCOMPARE(intervals.first().end(), 10);
}

QTEST_MAIN(IntervalTest)
