/*
	File                 : TextLabelTest.h
	Project              : LabPlot
	Description          : Tests for TextLabel
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2022 Stefan Gerlach <stefan.gerlach@uni.kn>

	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef TEXTLABELTEST_H
#define TEXTLABELTEST_H

#include "../../CommonMetaTest.h"

class TextLabelTest : public CommonMetaTest {
	Q_OBJECT

private Q_SLOTS:

	void addPlot();
	void multiLabelEditColorChange();
	void multiLabelEditTextChange();
	void multiLabelEditColorChangeSelection();
};

#endif
