/*
	File                 : AbstractAspectTest.h
	Project              : LabPlot
	Description          : Tests for AbstractAspect
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2023 Martin Marmsoler <martin.marmsoler@gmail.com>

	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef ABSTRACTASPECTTEST_H
#define ABSTRACTASPECTTEST_H

#include "../../CommonTest.h"

class AbstractAspectTest : public CommonTest {
	Q_OBJECT

private Q_SLOTS:
	void name();
	void copyPaste();
	void saveLoad();

	void moveUp();
	void moveDown();
	void moveUpDown();
};

#endif // ABSTRACTASPECTTEST_H
