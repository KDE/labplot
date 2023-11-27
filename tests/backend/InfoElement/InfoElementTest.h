/*
	File                 : InfoElementTest.h
	Project              : LabPlot
	Description          : Tests for InfoElement
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2023 Martin Marmsoler <martin.marmsoler@gmail.com>

 SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef INFOELEMENTTEST_H
#define INFOELEMENTTEST_H

#include "../../CommonTest.h"

class InfoElementTest : public CommonTest {
	Q_OBJECT

private Q_SLOTS:

	void addPlot();
	void addRemoveCurve();
};

#endif // INFOELEMENTTEST_H
