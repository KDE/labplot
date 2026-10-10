/*
	File                 : CommonMetaTest.cpp
	Project              : LabPlot
	Description          : General test class with MetaTypes
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2024 Stefan Gerlach <stefan.gerlach@uni.kn>

	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "CommonMetaTest.h"
#include "backend/core/AbstractAspect.h"
#include "backend/core/AbstractColumn.h"

/**
 * @brief Extended test base class for tests requiring backend library access
 *
 * Extends CommonTest with:
 * - Links against labplotbackendlib and labplotlib
 * - Registers Qt metatypes: AbstractAspect*, AbstractColumn*
 * - Enables signal/slot connections with backend objects
 *
 * Use this for tests that work with spreadsheets, columns, import/export filters,
 * or any backend aspect hierarchy functionality. For lightweight NSL/parser tests,
 * use CommonTest directly to avoid unnecessary dependencies.
 */

void CommonMetaTest::initTestCase() {
	CommonTest::initTestCase();

	qRegisterMetaType<const AbstractAspect*>("const AbstractAspect*");
	qRegisterMetaType<const AbstractColumn*>("const AbstractColumn*");
}
