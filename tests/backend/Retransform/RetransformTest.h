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
	void aspectRetransformed(const AbstractAspect* sender, bool suppressed);
	QHash<QString, int> statistic(bool includeSuppressed);
	int elementLogCount(bool includeSuppressed);
	struct Retransformed {
		Retransformed(const AbstractAspect* aspect, bool suppressed): aspect(aspect), suppressed(suppressed) {}
		const AbstractAspect* aspect;
		bool suppressed;
	};
	bool calledExact(int requiredCallCount, bool includeSuppressed);
	int callCount(const QString& path, bool includeSuppressed);
	int callCount(const AbstractAspect *aspect, bool includeSuppressed);

	QVector<Retransformed> logs;



// helper functions
private Q_SLOTS:
	void aspectAdded(const AbstractAspect* aspect);

private Q_SLOTS:
	void initTestCase();

	void TestLoadProject();
	void TestResizeWindows();
	void TestZoomSelectionAutoscale();
	
};

#endif // RETRANSFORMTEST_H
