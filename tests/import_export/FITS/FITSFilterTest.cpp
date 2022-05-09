/*
	File                 : FITSFilterTest.cpp
	Project              : LabPlot
	Description          : Tests for the FITS filter
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2022 Stefan Gerlach <stefan.gerlach@uni.kn>

	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "FITSFilterTest.h"
#include "backend/datasources/filters/FITSFilter.h"
#include "backend/spreadsheet/Spreadsheet.h"

extern "C" {
#include "fitsio.h"
#include <gsl/gsl_randist.h>
#include <gsl/gsl_rng.h>
}

void FITSFilterTest::importFile1() {
	const QString& fileName = QFINDTESTDATA(QLatin1String("data/WFPC2ASSNu5780205bx.fits"));

	Spreadsheet spreadsheet("test", false);
	FITSFilter filter;
	filter.readDataFromFile(fileName, &spreadsheet, AbstractFileFilter::ImportMode::Replace);

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

// BENCHMARKS

void FITSFilterTest::benchDoubleImport_data() {
	QTest::addColumn<long>("lineCount");
	// can't transfer file name since needed in clean up

	QTemporaryFile file;
	if (!file.open()) // needed to generate file name
		return;
	file.close(); // only file name is used

	benchDataFileName = file.fileName();
	benchDataFileName.append(".fits");

	QString testName(QString::number(paths) + QLatin1String(" random double paths"));

	QTest::newRow(testName.toLatin1()) << lines;
	DEBUG("CREATE DATA FILE " << STDSTRING(benchDataFileName) << ", lines = " << lines)

	gsl_rng_env_setup();
	gsl_rng* r = gsl_rng_alloc(gsl_rng_default);
	gsl_rng_set(r, 12345);

	// create file
	int status = 0;

	fitsfile* fptr;
	fits_create_file(&fptr, benchDataFileName.toLatin1(), &status);

	long naxis = 2;
	long naxes[2] = {paths, lines};
	fits_create_img(fptr, DOUBLE_IMG, naxis, naxes, &status);

	// create data
	double path[paths] = {0.0};
	double* data = new double[paths*lines];

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
	Spreadsheet spreadsheet("test", false);
	FITSFilter filter;

	const int p = paths; // need local variable
	QBENCHMARK {
		filter.readDataFromFile(benchDataFileName, &spreadsheet, AbstractFileFilter::ImportMode::Replace);

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
