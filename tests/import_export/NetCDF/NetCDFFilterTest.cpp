/*
	File                 : NetCDFFilterTest.cpp
	Project              : LabPlot
	Description          : Tests for the NetCDF filter
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2022 Stefan Gerlach <stefan.gerlach@uni.kn>

	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "NetCDFFilterTest.h"
#include "backend/datasources/filters/NetCDFFilter.h"
#include "backend/lib/macros.h"
#include "backend/spreadsheet/Spreadsheet.h"

#include <gsl/gsl_randist.h>
#include <gsl/gsl_rng.h>
extern "C" {
#include <netcdf.h>
}

#define ERRCODE -1
#define ERR(e)                                                                                                                                                 \
	{                                                                                                                                                          \
		printf("Error: %s\n", nc_strerror(e));                                                                                                                 \
		exit(ERRCODE);                                                                                                                                         \
	}

void NetCDFFilterTest::importFile1() {
	const QString& fileName = QFINDTESTDATA(QLatin1String("data/madis-hydro.nc"));

	Spreadsheet spreadsheet(QStringLiteral("test"), false);
	NetCDFFilter filter;
	filter.setCurrentVarName(QLatin1String("lastRecord"));
	filter.readDataFromFile(fileName, &spreadsheet, AbstractFileFilter::ImportMode::Replace);

	QCOMPARE(spreadsheet.columnCount(), 1);
	QCOMPARE(spreadsheet.rowCount(), 2000);

	QCOMPARE(spreadsheet.column(0)->valueAt(0), 1018);
	QCOMPARE(spreadsheet.column(0)->valueAt(1), 1132);
	QCOMPARE(spreadsheet.column(0)->valueAt(2), 980);
	QCOMPARE(spreadsheet.column(0)->valueAt(1998), -1);
	QCOMPARE(spreadsheet.column(0)->valueAt(1999), -1);

	NetCDFFilter filter2;
	filter2.setCurrentVarName(QLatin1String("latitude"));
	filter2.readDataFromFile(fileName, &spreadsheet, AbstractFileFilter::ImportMode::Replace);

	QCOMPARE(spreadsheet.columnCount(), 1);
	QCOMPARE(spreadsheet.rowCount(), 1176);

	QCOMPARE(spreadsheet.column(0)->valueAt(0), 39.8050003052);
	QCOMPARE(spreadsheet.column(0)->valueAt(1), 39.8230018616);
	QCOMPARE(spreadsheet.column(0)->valueAt(2), 39.8199996948);
	QCOMPARE(spreadsheet.column(0)->valueAt(1174), 40.2159996033);
	QCOMPARE(spreadsheet.column(0)->valueAt(1175), 40.2480010986);
}

void NetCDFFilterTest::importFile2() {
	const QString& fileName = QFINDTESTDATA(QLatin1String("data/OMI-Aura_L2-example.nc"));

	Spreadsheet spreadsheet(QStringLiteral("test"), false);
	NetCDFFilter filter;
	filter.setCurrentVarName(QLatin1String("CovarianceMatrix"));
	filter.readDataFromFile(fileName, &spreadsheet, AbstractFileFilter::ImportMode::Replace);

	QCOMPARE(spreadsheet.columnCount(), 465);
	QCOMPARE(spreadsheet.rowCount(), 2);

	QCOMPARE(spreadsheet.column(0)->valueAt(0), 76);
	QCOMPARE(spreadsheet.column(0)->valueAt(1), 78);
	QCOMPARE(spreadsheet.column(1)->valueAt(0), -17);
	QCOMPARE(spreadsheet.column(1)->valueAt(1), -18);
	QCOMPARE(spreadsheet.column(2)->valueAt(0), 49);
	QCOMPARE(spreadsheet.column(2)->valueAt(1), 50);

	NetCDFFilter filter2;
	filter2.setCurrentVarName(QLatin1String("O3.COLUMN.PARTIAL"));
	filter2.readDataFromFile(fileName, &spreadsheet, AbstractFileFilter::ImportMode::Replace);

	QCOMPARE(spreadsheet.columnCount(), 18);
	QCOMPARE(spreadsheet.rowCount(), 2);

	WARN(spreadsheet.column(0)->valueAt(1))
	WARN(spreadsheet.column(1)->valueAt(0))
	WARN(spreadsheet.column(1)->valueAt(1))
	WARN(spreadsheet.column(17)->valueAt(0))
	WARN(spreadsheet.column(17)->valueAt(1))
	QCOMPARE(spreadsheet.column(0)->valueAt(0), 0.323581278324);
	QCOMPARE(spreadsheet.column(0)->valueAt(1), 0.317059576511383);
	QCOMPARE(spreadsheet.column(1)->valueAt(0), 1.23419499397278);
	QCOMPARE(spreadsheet.column(1)->valueAt(1), 1.224156498909);
	QCOMPARE(spreadsheet.column(17)->valueAt(0), 8.46088886260986);
	QCOMPARE(spreadsheet.column(17)->valueAt(1), 8.35280704498291);
}

void NetCDFFilterTest::importFile3() {
	const QString& fileName = QFINDTESTDATA(QLatin1String("data/testrh.nc"));

	Spreadsheet spreadsheet(QStringLiteral("test"), false);
	NetCDFFilter filter;
	filter.setCurrentVarName(QLatin1String("var1"));
	filter.readDataFromFile(fileName, &spreadsheet, AbstractFileFilter::ImportMode::Replace);

	QCOMPARE(spreadsheet.columnCount(), 1);
	QCOMPARE(spreadsheet.rowCount(), 10000);

	QCOMPARE(spreadsheet.column(0)->valueAt(0), 420);
	QCOMPARE(spreadsheet.column(0)->valueAt(1), 197);
	QCOMPARE(spreadsheet.column(0)->valueAt(2), 391.5);
	QCOMPARE(spreadsheet.column(0)->valueAt(9998), 186.5);
	QCOMPARE(spreadsheet.column(0)->valueAt(9999), 444);
}

// BENCHMARKS

void NetCDFFilterTest::benchDoubleImport_data() {
	QTest::addColumn<size_t>("lineCount");
	// can't transfer file name since needed in clean up

	QTemporaryFile file;
	if (!file.open()) // needed to generate file name
		return;
	file.close(); // only file name is used

	benchDataFileName = file.fileName();
	benchDataFileName.append(QStringLiteral(".nc4"));

	QString testName(QString::number(paths) + QLatin1String(" random double paths"));

	QTest::newRow(qPrintable(testName)) << lines;
	DEBUG("CREATE DATA FILE " << STDSTRING(benchDataFileName) << ", lines = " << lines)

	gsl_rng_env_setup();
	gsl_rng* r = gsl_rng_alloc(gsl_rng_default);
	gsl_rng_set(r, 12345);

	// define parameter
	int status, ncid;
	if ((status = nc_create(qPrintable(benchDataFileName), NC_NETCDF4, &ncid)))
		ERR(status);

	int xdimid, ydimid;
	if ((status = nc_def_dim(ncid, "line", lines, &xdimid)))
		ERR(status);
	if ((status = nc_def_dim(ncid, "path", paths, &ydimid)))
		ERR(status);

	const int ndims = 2;
	int dimids[] = {xdimid, ydimid};

	int varid;
	if ((status = nc_def_var(ncid, "paths", NC_DOUBLE, ndims, dimids, &varid)))
		ERR(status);
	// compress data (~10-20% smaller, but 3x-5x slower)
	// (ncid, varid, shuffle, deflate, deflatelevel)
	// if ((status = nc_def_var_deflate(ncid, varid, 1, 1, 9)))
	//	ERR(status);

	if ((status = nc_enddef(ncid)))
		ERR(status);

	// create data
	double path[paths] = {0.0};
	double* data = new double[paths * lines];

	const double delta = 0.25;
	const int dt = 1;
	const double sigma = delta * delta * dt;
	for (size_t i = 0; i < lines; ++i) {
		// std::cout << "line " << i+1 << std::endl;

		for (int p = 0; p < paths; ++p) {
			path[p] += gsl_ran_gaussian_ziggurat(r, sigma);
			data[p + i * paths] = path[p];
		}
	}

	nc_put_var_double(ncid, varid, data);

	if ((status = nc_close(ncid)))
		ERR(status);

	delete[] data;

	DEBUG(Q_FUNC_INFO << ", DONE")
}

void NetCDFFilterTest::benchDoubleImport() {
	Spreadsheet spreadsheet(QStringLiteral("test"), false);
	NetCDFFilter filter;
	filter.setCurrentVarName(QLatin1String("paths"));

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

void NetCDFFilterTest::benchDoubleImport_cleanup() {
	DEBUG("REMOVE DATA FILE " << STDSTRING(benchDataFileName))
	QFile::remove(benchDataFileName);
}

QTEST_MAIN(NetCDFFilterTest)
