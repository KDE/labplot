/*
	File                 : FITSFilterTest.cpp
	Project              : LabPlot
	Description          : Tests for the FITS filter
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2022-2025 Stefan Gerlach <stefan.gerlach@uni.kn>

	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "FITSFilterTest.h"
#include "backend/datasources/filters/FITSFilter.h"
#include "backend/lib/macros.h"
#include "backend/spreadsheet/Spreadsheet.h"
#include "frontend/spreadsheet/SpreadsheetView.h"
#include "frontend/spreadsheet/EquidistantValuesDialog.h"

#include <gsl/gsl_randist.h>
#include <gsl/gsl_rng.h>

extern "C" {
#include "fitsio.h"
}

void FITSFilterTest::importFile1() {
	const QString& fileName = QFINDTESTDATA(QLatin1String("data/WFPC2ASSNu5780205bx.fits"));

	Spreadsheet spreadsheet(QStringLiteral("test"), false);
	FITSFilter filter;
	filter.readDataFromFile(fileName, &spreadsheet);

	QCOMPARE(spreadsheet.columnCount(), 100);
	QCOMPARE(spreadsheet.rowCount(), 100);

	WARN(spreadsheet.column(0)->valueAt(0))
	WARN(spreadsheet.column(1)->valueAt(0))
	WARN(spreadsheet.column(0)->valueAt(1))
	WARN(spreadsheet.column(1)->valueAt(1))
	WARN(spreadsheet.column(99)->valueAt(99))
	QCOMPARE(spreadsheet.column(0)->valueAt(0), 1.0315774679184);
	QCOMPARE(spreadsheet.column(1)->valueAt(0), 1.08554399013519);
	QCOMPARE(spreadsheet.column(0)->valueAt(1), 0.476544350385666);
	QCOMPARE(spreadsheet.column(1)->valueAt(1), 0.369004756212234);
	QCOMPARE(spreadsheet.column(99)->valueAt(99), 0.487100154161453);
}

void FITSFilterTest::importFile2() {
	const QString& fileName = QFINDTESTDATA(QLatin1String("data/WFPC2u5780205r_c0fx.fits"));

	Spreadsheet spreadsheet(QStringLiteral("test"), false);
	FITSFilter filter;
	filter.readDataFromFile(fileName, &spreadsheet);

	QCOMPARE(spreadsheet.columnCount(), 200);
	QCOMPARE(spreadsheet.rowCount(), 200);

	WARN(spreadsheet.column(0)->valueAt(0))
	WARN(spreadsheet.column(1)->valueAt(0))
	WARN(spreadsheet.column(0)->valueAt(1))
	WARN(spreadsheet.column(1)->valueAt(1))
	WARN(spreadsheet.column(99)->valueAt(99))
	QCOMPARE(spreadsheet.column(0)->valueAt(0), -1.54429864883423);
	QCOMPARE(spreadsheet.column(1)->valueAt(0), 0.91693103313446);
	QCOMPARE(spreadsheet.column(0)->valueAt(1), -0.882439613342285);
	QCOMPARE(spreadsheet.column(1)->valueAt(1), -1.09242105484009);
	QCOMPARE(spreadsheet.column(99)->valueAt(99), -0.387779891490936);

	// read table
	const QString& tableName = fileName + QLatin1String("[u5780205r_cvt.c0h.tab]");
	FITSFilter filter2;
	filter2.readDataFromFile(tableName, &spreadsheet);

	QCOMPARE(spreadsheet.columnCount(), 49);
	QCOMPARE(spreadsheet.rowCount(), 4);

	WARN(spreadsheet.column(0)->valueAt(0))
	WARN(spreadsheet.column(1)->valueAt(0))
	WARN(spreadsheet.column(0)->valueAt(1))
	WARN(spreadsheet.column(1)->valueAt(1))
	WARN(spreadsheet.column(48)->valueAt(3))
	QCOMPARE(spreadsheet.column(0)->valueAt(0), 182.6311886308);
	QCOMPARE(spreadsheet.column(1)->valueAt(0), 39.39633673411);
	QCOMPARE(spreadsheet.column(0)->valueAt(1), 182.6255233634);
	QCOMPARE(spreadsheet.column(1)->valueAt(1), 39.41214313815);
	QCOMPARE(spreadsheet.column(48)->valueAt(3), 0.3466465);
}

void FITSFilterTest::exportImport() {
	// create Spreadsheet with data
	Spreadsheet spreadsheet(QStringLiteral("Spreadsheet"), false);
	spreadsheet.setColumnCount(3);
        spreadsheet.setRowCount(10);

	const QVector<double> data{0.5, -0.2, GSL_NAN, 2.0, -1.0};

	auto* col1{spreadsheet.column(0)};
	col1->setColumnMode(AbstractColumn::ColumnMode::Integer);
        auto* col2{spreadsheet.column(1)};
        auto* col3{spreadsheet.column(2)};

	SpreadsheetView view(&spreadsheet, false);
	view.selectColumn(0);
	view.fillWithRowNumbers();
	view.selectColumn(1);

	EquidistantValuesDialog dlg(&spreadsheet);
        dlg.setColumns(QVector<Column*>{col2});
        dlg.setType(EquidistantValuesDialog::Type::FixedNumber);
        dlg.setNumber(6);
        dlg.setFromValue(1.);
        dlg.setToValue(2.);
        dlg.generate();

	col3->replaceValues(0, data);

	// export to FITS
	QTemporaryFile file;
	if (!file.open()) // needed to generate file name
		return;
	file.close(); // only file name is used

	QString fileName = file.fileName();
	fileName.append(QStringLiteral(".fits"));

	FITSFilter filter;
	filter.write(fileName, &spreadsheet);

	// import FITS file to another spreadsheet
	Spreadsheet spreadsheet2(QStringLiteral("Import"), false);
	filter.readDataFromFile(fileName, &spreadsheet2);

	QCOMPARE(spreadsheet2.columnCount(), 3);
	QCOMPARE(spreadsheet2.rowCount(), 10);

	// check values
	for (int i = 0; i < spreadsheet2.rowCount(); i++)
		QCOMPARE(spreadsheet2.column(0)->valueAt(i), i+1);

	for (int i = 0; i < 6; i++)
		QCOMPARE(spreadsheet2.column(1)->valueAt(i), 1. + i * 0.2);

	for (int i = 0; i < data.size(); i++)
		QCOMPARE(spreadsheet2.column(2)->valueAt(i), data.at(i));

	//TODO QFile::remove(fileName);
}

// BENCHMARKS

void FITSFilterTest::benchDoubleImport_data() {
	QTest::addColumn<long>("lineCount");
	// can't transfer file name since needed in clean up

	QTemporaryFile file;
	if (!file.open()) // needed to generate file name
		return;
	file.close(); // only file name is used

	benchDataFileName = file.fileName();
	benchDataFileName.append(QStringLiteral(".fits"));

	QString testName(QString::number(paths) + QLatin1String(" random double paths"));

	QTest::newRow(qPrintable(testName)) << lines;
	DEBUG("CREATE DATA FILE " << STDSTRING(benchDataFileName) << ", lines = " << lines)

	gsl_rng_env_setup();
	gsl_rng* r = gsl_rng_alloc(gsl_rng_default);
	gsl_rng_set(r, 12345);

	// create file
	int status = 0;

	fitsfile* fptr;
	fits_create_file(&fptr, qPrintable(benchDataFileName), &status);

	long naxis = 2;
	long naxes[2] = {paths, lines};
	fits_create_img(fptr, DOUBLE_IMG, naxis, naxes, &status);

	// create data
	double path[paths] = {0.0};
	double* data = new double[paths * lines];

	const double delta = 0.25;
	const int dt = 1;
	const double sigma = delta * delta * dt;
	for (long i = 0; i < lines; ++i) {
		// std::cout << "line " << i+1 << std::endl;

		for (int p = 0; p < paths; ++p) {
			path[p] += gsl_ran_gaussian_ziggurat(r, sigma);
			data[p + i * paths] = path[p];
		}
	}

	long fpixel = 1, nelements = naxes[0] * naxes[1];
	// write data and close file
	fits_write_img(fptr, TDOUBLE, fpixel, nelements, data, &status);

	fits_close_file(fptr, &status);
	fits_report_error(stderr, status);

	delete[] data;

	DEBUG(Q_FUNC_INFO << ", DONE")
}

void FITSFilterTest::benchDoubleImport() {
	Spreadsheet spreadsheet(QStringLiteral("test"), false);
	FITSFilter filter;

	const int p = paths; // need local variable
	QBENCHMARK {
		filter.readDataFromFile(benchDataFileName, &spreadsheet);

		QCOMPARE(spreadsheet.columnCount(), p);
		QCOMPARE(spreadsheet.rowCount(), lines);

		QCOMPARE(spreadsheet.column(0)->valueAt(0), 0.120997813055);
		QCOMPARE(spreadsheet.column(1)->valueAt(0), 0.119301077563219);
		QCOMPARE(spreadsheet.column(2)->valueAt(0), -0.0209979608555485);
	}
}

void FITSFilterTest::benchDoubleImport_cleanup() {
	DEBUG("REMOVE DATA FILE " << STDSTRING(benchDataFileName))
	QFile::remove(benchDataFileName);
}

QTEST_MAIN(FITSFilterTest)
