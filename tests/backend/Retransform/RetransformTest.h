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

#include "../../CommonMetaTest.h"

class RetransformTest : public CommonMetaTest {
	Q_OBJECT

	// Tests
private Q_SLOTS:
	void TestLoadProject();
	void TestLoadProject2();
	void TestResizeWindows();
	void TestPadding();
	void TestCopyPastePlot();
	void TestAddCurve();
	void TestImportCSV();
	void TestImportCSVInvalidateCurve();
	void TestSetScale();

	void TestZoomSelectionAutoscale();
	void TestZoomAutoscaleSingleYRange();
	void TestZoomAutoscaleSingleXRange();
	void TestZoom();

	void TestChangePlotRange();
	void TestChangePlotRangeElement();
	void TestChangePlotRangeElement2();
	void TestChangePlotRangeElement3();

	void TestBarPlotOrientation();

	void testPlotRecalcRetransform();
	void testPlotRecalcNoRetransform();

	void removeReaddxColum();
	void removeReaddyColum();
};

/*!
 * \brief The RetransformCallCounter class
 * Used to count the retransform calls to evaluate that
 * the items retransform are called a exact number of times
 */
class RetransformCallCounter : public QObject {
	Q_OBJECT
public:
	QHash<QString, int> statistic(bool includeSuppressed);
	int elementLogCount(bool includeSuppressed);
	bool calledExact(int requiredCallCount, bool includeSuppressed);
	int callCount(const QString& path);
	int callCount(const AbstractAspect* aspect);
	void resetRetransformCount();
	void aspectRetransformed(const AbstractAspect* sender, bool suppressed);
	void retransformScaleCalled(const CartesianPlot* plot, CartesianCoordinateSystem::Dimension dim, int index);
	void aspectAdded(const AbstractAspect* aspect);

public:
	struct Retransformed {
		const AbstractAspect* aspect;
		bool suppressed;
	};

	struct ScaleRetransformed {
		const CartesianPlot* plot;
		int index;
	};
	QVector<Retransformed> logsRetransformed;
	QVector<ScaleRetransformed> logsXScaleRetransformed;
	QVector<ScaleRetransformed> logsYScaleRetransformed;
};

#endif // RETRANSFORMTEST_H
