/*
    File                 : NetCDFFilterTest.cpp
    Project              : LabPlot
    Description          : Tests for the binary filter
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2022 Stefan Gerlach <stefan.gerlach@uni.kn>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "NetCDFFilterTest.h"
#include "backend/datasources/filters/NetCDFFilter.h"
#include "backend/spreadsheet/Spreadsheet.h"

extern "C" {
#include <gsl/gsl_randist.h>
#include <gsl/gsl_rng.h>
#include <netcdf.h>
}

#define ERRCODE -1
#define ERR(e) {printf("Error: %s\n", nc_strerror(e)); exit(ERRCODE);}

//void NetCDFFilterTest::importXYZ() {
//}

// DOUBLE data

void NetCDFFilterTest::benchDoubleImport_data() {
	QTest::addColumn<size_t>("lineCount");
	// can't transfer file name since needed in clean up

	QTemporaryFile file;
	if (!file.open())	// needed to generate file name
		return;
	file.close();	// only file name is used

	benchDataFileName = file.fileName();
	benchDataFileName.append(".nc4");

	QString testName(QString::number(paths) + QLatin1String(" double cols"));

	QTest::newRow(testName.toLatin1()) << lines;
	DEBUG("CREATE DATA FILE " << STDSTRING(benchDataFileName) <<  ", lines = " << lines)

	gsl_rng_env_setup();
	gsl_rng* r = gsl_rng_alloc(gsl_rng_default);
	gsl_rng_set(r, 12345);

	// define parameter 
	int status, ncid;
	if ((status = nc_create(benchDataFileName.toLatin1(), NC_NETCDF4, &ncid)))
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

	if ((status = nc_enddef(ncid)))
		ERR(status);

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
	nc_put_var_double(ncid, varid, data);

	if ((status = nc_close(ncid)))
		ERR(status);

	delete[] data;

	DEBUG(Q_FUNC_INFO << ", DONE")
}

void NetCDFFilterTest::benchDoubleImport() {
	QFETCH(size_t, lineCount);

	Spreadsheet spreadsheet("test", false);
	NetCDFFilter filter;
	filter.setCurrentVarName(QLatin1String("paths"));

	QBENCHMARK {
		filter.readDataFromFile(benchDataFileName, &spreadsheet, AbstractFileFilter::ImportMode::Replace);

		QCOMPARE(spreadsheet.columnCount(), 5);
		QCOMPARE(spreadsheet.rowCount(), lineCount);

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
