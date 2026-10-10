/*
	File                 : AbstractFilterTest.h
	Project              : LabPlot
	Description          : Tests for AbstractFilter and AbstractSimpleFilter
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2026 LabPlot contributors
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef ABSTRACTFILTERTEST_H
#define ABSTRACTFILTERTEST_H

#include "../../CommonMetaTest.h"

class AbstractFilterTest : public CommonMetaTest {
	Q_OBJECT

private Q_SLOTS:
	void testInputPorts();
	void testFilterInputForwarding();
	void testInputDestroyed();
	void testSimpleFilterOutputColumn();
	void testCopyThroughFilter();
};

#endif
