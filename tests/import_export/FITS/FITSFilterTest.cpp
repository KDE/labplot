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
#include <gsl/gsl_randist.h>
#include <gsl/gsl_rng.h>
#include <fitsio.h>
}

#define ERRCODE -1
#define ERR(e) {printf("Error: %s\n", nc_strerror(e)); exit(ERRCODE);}

void FITSFilterTest::importFile1() {
	const QString& fileName = QFINDTESTDATA(QLatin1String("data/HRSz0yd020fm_c2f.fits"));

	Spreadsheet spreadsheet("test", false);
	FITSFilter filter;
	//TODO
	//filter.readDataFromFile(fileName, &spreadsheet, AbstractFileFilter::ImportMode::Replace);

	//QCOMPARE(spreadsheet.columnCount(), 2001);
	//QCOMPARE(spreadsheet.rowCount(), 5);

	// TODO: not reproducable!
	//QCOMPARE(spreadsheet.column(0)->valueAt(0), 4.95209585675e-316);
}

// BENCHMARKS

/*void FITSFilterTest::benchDoubleImport_data() {
	QTest::addColumn<size_t>("lineCount");
	// can't transfer file name since needed in clean up

	QTemporaryFile file;
	if (!file.open())	// needed to generate file name
		return;
	file.close();	// only file name is used

	benchDataFileName = file.fileName();
	benchDataFileName.append(".nc4");

	QString testName(QString::number(paths) + QLatin1String(" random double paths"));

	QTest::newRow(testName.toLatin1()) << lines;
	DEBUG("CREATE DATA FILE " << STDSTRING(benchDataFileName) <<  ", lines = " << lines)

	gsl_rng_env_setup();
	gsl_rng* r = gsl_rng_alloc(gsl_rng_default);
	gsl_rng_set(r, 12345);

	// crate file
	// TODO

	// create data
	double path[paths] = {0.0};
	double *data = new double[paths*lines];

	const double delta = 0.25;
	const int dt = 1;
	const double sigma = delta*delta * dt;
	for (size_t i = 0; i < lines; ++i) {
		//std::cout << "line " << i+1 << std::endl;

		for (int p = 0; p < paths; ++p) {
			path[p] += gsl_ran_gaussian_ziggurat(r, sigma);
			data[p + i*paths] = path[p];
		}
	}

	delete[] data;

	DEBUG(Q_FUNC_INFO << ", DONE")
}

void FITSFilterTest::benchDoubleImport() {
	Spreadsheet spreadsheet("test", false);
	FITSFilter filter;
	filter.setCurrentVarName(QLatin1String("paths"));

	const int p = paths;	// need local variable
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
}*/

QTEST_MAIN(FITSFilterTest)
