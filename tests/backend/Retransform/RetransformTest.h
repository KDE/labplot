/*
	File                 : RetransformTest.h
	Project              : LabPlot
	Description          : Tests to evaluate retransforming
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2021 Martin Marmsoler <martin.marmsoler@gmail.com>

	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef RETRANSFORMTEST_H
#define RETRANSFORMTEST_H

#include "../../CommonTest.h"

class RetransformTest : public CommonTest {
	Q_OBJECT

private:
	void resetRetransformCount();
	QHash<QString, int> statistic(bool includeSuppressed);
	int elementLogCount(bool includeSuppressed);
	struct Retransformed {
		const AbstractAspect* aspect;
		bool suppressed;
	};

	struct ScaleRetransformed {
		const CartesianPlot* plot;
		int index;
	};

	bool calledExact(int requiredCallCount, bool includeSuppressed);
	int callCount(const QString& path, bool includeSuppressed);
	int callCount(const AbstractAspect *aspect, bool includeSuppressed);

	QVector<Retransformed> logsRetransformed;
	QVector<ScaleRetransformed> logsXScaleRetransformed;
	QVector<ScaleRetransformed> logsYScaleRetransformed;



// helper functions
private Q_SLOTS:
	void aspectAdded(const AbstractAspect* aspect);
	void aspectRetransformed(const AbstractAspect* sender, bool suppressed);
	void retransformXScaleCalled(const CartesianPlot* plot, int index);
	void retransformYScaleCalled(const CartesianPlot* plot, int index);

// Tests
private Q_SLOTS:
	void TestLoadProject();
	void TestResizeWindows();
	void TestZoomSelectionAutoscale();
	void TestPadding();
	void TestCopyPastePlot();
	
};

#endif // RETRANSFORMTEST_H
