/*
	File                 : DependenciesTest.h
	Project              : LabPlot
	Description          : Tests for dependencies
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2023 Stefan Gerlach <stefan.gerlach@uni.kn>

	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef DEPENDENCIESTEST_H
#define DEPENDENCIESTEST_H

#include "CommonTest.h"

class DependenciesTest : public CommonTest {
	Q_OBJECT

private Q_SLOTS:
#ifdef MATIO_DISABLED
	void checkMatio() {
		QSKIP("Skipping Matio Tests, because it was disabled!");
	}
#elif !defined(HAVE_MATIO)
	void checkMatio() {
		QSKIP("Skipping Matio Tests, because it was not found!");
	}
#endif
};

#endif
