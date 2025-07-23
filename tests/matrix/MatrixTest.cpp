/*
	File                 : MatrixTest.cpp
	Project              : LabPlot
	Description          : Tests for the Matrix
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2022 Stefan Gerlach <stefan.gerlach@uni.kn>

	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "MatrixTest.h"
#include "backend/core/Project.h"
#include "backend/matrix/Matrix.h"
#include "frontend/matrix/MatrixView.h"

void MatrixTest::testLoadSaveNoData() {
	constexpr auto rowCount = 10;
	constexpr auto columnCount = 5;
	QString savePath;

	{
		Project project;
		project.setSaveData(false);
		auto* m = new Matrix(rowCount, columnCount, QStringLiteral("Test"));
		project.addChild(m);

		QCOMPARE(m->rowCount(), rowCount);
		QCOMPARE(m->columnCount(), columnCount);

		for (int r = 0; r < rowCount; r++) {
			for (int c = 0; c < columnCount; c++) {
				m->setCell(r, c, (double)(r * columnCount + c));
			}
		}
		SAVE_PROJECT("testLoadSaveNoData");
	}

	{
		Project project;
		QVERIFY(project.load(savePath));

		const auto& matrices = project.children<Matrix>();
		QCOMPARE(matrices.size(), 1);
		const auto* matrix = matrices.at(0);
		QCOMPARE(matrix->name(), QStringLiteral("Test"));
		QCOMPARE(matrix->rowCount(), 0);
		QCOMPARE(matrix->columnCount(), 0);
	}
}

void MatrixTest::testLoadSaveWithData() {
	constexpr auto rowCount = 10;
	constexpr auto columnCount = 5;
	QString savePath;

	{
		Project project;
		project.setSaveData(true);
		auto* m = new Matrix(rowCount, columnCount, QStringLiteral("Test"));
		project.addChild(m);

		QCOMPARE(m->rowCount(), rowCount);
		QCOMPARE(m->columnCount(), columnCount);

		for (int r = 0; r < rowCount; r++) {
			for (int c = 0; c < columnCount; c++) {
				m->setCell(r, c, (double)(r * columnCount + c));
			}
		}
		SAVE_PROJECT("testLoadSaveNoData");
	}

	{
		Project project;
		QVERIFY(project.load(savePath));

		const auto& matrices = project.children<Matrix>();
		QCOMPARE(matrices.size(), 1);
		const auto* matrix = matrices.at(0);
		QCOMPARE(matrix->name(), QStringLiteral("Test"));
		QCOMPARE(matrix->rowCount(), rowCount);
		QCOMPARE(matrix->columnCount(), columnCount);

		for (int r = 0; r < rowCount; r++) {
			for (int c = 0; c < columnCount; c++) {
				VALUES_EQUAL(matrix->cell<double>(r, c), r * columnCount + c);
			}
		}
	}
}

QTEST_MAIN(MatrixTest)
