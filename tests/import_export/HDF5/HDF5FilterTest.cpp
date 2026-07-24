/*
	File                 : HDF5FilterTest.cpp
	Project              : LabPlot
	Description          : Tests for the HDF5 I/O-filter.
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2021-2022 Stefan Gerlach <stefan.gerlach@uni.kn>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "HDF5FilterTest.h"
#include "backend/core/column/Column.h"
#include "backend/datasources/filters/HDF5Filter.h"
#include "backend/lib/macros.h"
#include "backend/matrix/Matrix.h"
#include "backend/spreadsheet/Spreadsheet.h"

#include <KLocalizedString>

#include <gsl/gsl_randist.h>
#include <gsl/gsl_rng.h>
extern "C" {
#include <hdf5.h>
}

void HDF5FilterTest::testImportDouble() {
	Spreadsheet spreadsheet(QStringLiteral("test"), false);
	HDF5Filter filter;

	const QString& fileName = QFINDTESTDATA(QLatin1String("data/hdf5_test.h5"));
	filter.setCurrentDataSetName(QLatin1String("/arrays/2D float array"));
	filter.readDataFromFile(fileName, &spreadsheet, AbstractFileFilter::ImportMode::Replace);

	QCOMPARE(spreadsheet.columnCount(), 50);
	QCOMPARE(spreadsheet.rowCount(), 100);
	for (int i = 0; i < spreadsheet.columnCount(); i++) {
		QCOMPARE(spreadsheet.column(i)->columnMode(), AbstractColumn::ColumnMode::Double);
		QCOMPARE(spreadsheet.column(i)->name(), QLatin1String("2D float array_") + QString::number(i + 1));
	}

	QCOMPARE(spreadsheet.column(0)->plotDesignation(), AbstractColumn::PlotDesignation::X);
	for (int i = 1; i < spreadsheet.columnCount(); i++)
		QCOMPARE(spreadsheet.column(i)->plotDesignation(), AbstractColumn::PlotDesignation::Y);

	for (int i = 0; i < 4; i++)
		for (int j = 0; j < 4; j++)
			DEBUG(std::setprecision(15) << spreadsheet.column(j)->valueAt(i))

	QCOMPARE(spreadsheet.column(0)->valueAt(0), 0.000123456804431044);
	QCOMPARE(spreadsheet.column(1)->valueAt(0), 0.001234499970451);
	QCOMPARE(spreadsheet.column(2)->valueAt(0), 0);
	QCOMPARE(spreadsheet.column(3)->valueAt(0), 0);
	QCOMPARE(spreadsheet.column(0)->valueAt(1), 0);
	QCOMPARE(spreadsheet.column(1)->valueAt(1), 0.899999976158142);
	QCOMPARE(spreadsheet.column(2)->valueAt(1), 1.70000004768372);
	QCOMPARE(spreadsheet.column(3)->valueAt(1), 2.59999990463257);
	QCOMPARE(spreadsheet.column(0)->valueAt(2), 0);
	QCOMPARE(spreadsheet.column(1)->valueAt(2), 1.70000004768372);
	QCOMPARE(spreadsheet.column(2)->valueAt(2), 3.5);
	QCOMPARE(spreadsheet.column(3)->valueAt(2), 5.19999980926514);
	QCOMPARE(spreadsheet.column(0)->valueAt(3), 0.170000001788139);
	QCOMPARE(spreadsheet.column(1)->valueAt(3), 2.59999990463257);
	QCOMPARE(spreadsheet.column(2)->valueAt(3), 3.5);
	QCOMPARE(spreadsheet.column(3)->valueAt(3), 7.80000019073486);
}

void HDF5FilterTest::testImportDoublePortion() {
	Spreadsheet spreadsheet(QStringLiteral("test"), false);
	HDF5Filter filter;

	const QString& fileName = QFINDTESTDATA(QLatin1String("data/hdf5_test.h5"));
	filter.setCurrentDataSetName(QLatin1String("/arrays/2D float array"));
	// set start/end row/col
	filter.setStartRow(2);
	filter.setEndRow(3);
	filter.setStartColumn(2);
	filter.setEndColumn(3);
	filter.readDataFromFile(fileName, &spreadsheet, AbstractFileFilter::ImportMode::Replace);

	QCOMPARE(spreadsheet.columnCount(), 2);
	QCOMPARE(spreadsheet.rowCount(), 2);
	for (int i = 0; i < 2; i++) {
		QCOMPARE(spreadsheet.column(i)->columnMode(), AbstractColumn::ColumnMode::Double);
		QCOMPARE(spreadsheet.column(i)->name(), QLatin1String("2D float array_") + QString::number(i + 1));
	}

	QCOMPARE(spreadsheet.column(0)->plotDesignation(), AbstractColumn::PlotDesignation::X);
	QCOMPARE(spreadsheet.column(1)->plotDesignation(), AbstractColumn::PlotDesignation::Y);

	for (int i = 0; i < 2; i++)
		for (int j = 0; j < 2; j++)
			DEBUG(std::setprecision(15) << spreadsheet.column(j)->valueAt(i))

	QCOMPARE(spreadsheet.column(0)->valueAt(0), 0.899999976158142);
	QCOMPARE(spreadsheet.column(0)->valueAt(1), 1.70000004768372);
	QCOMPARE(spreadsheet.column(1)->valueAt(0), 1.70000004768372);
	QCOMPARE(spreadsheet.column(1)->valueAt(1), 3.5);
}

void HDF5FilterTest::testImportInt() {
	Spreadsheet spreadsheet(QStringLiteral("test"), false);
	HDF5Filter filter;

	const QString& fileName = QFINDTESTDATA(QLatin1String("data/hdf5_test.h5"));
	filter.setCurrentDataSetName(QLatin1String("/arrays/2D int array"));
	filter.readDataFromFile(fileName, &spreadsheet, AbstractFileFilter::ImportMode::Replace);

	QCOMPARE(spreadsheet.columnCount(), 50);
	QCOMPARE(spreadsheet.rowCount(), 100);
	for (int i = 0; i < spreadsheet.columnCount(); i++) {
		QCOMPARE(spreadsheet.column(i)->columnMode(), AbstractColumn::ColumnMode::Integer);
		QCOMPARE(spreadsheet.column(i)->name(), QLatin1String("2D int array_") + QString::number(i + 1));
	}

	QCOMPARE(spreadsheet.column(0)->plotDesignation(), AbstractColumn::PlotDesignation::X);
	for (int i = 1; i < spreadsheet.columnCount(); i++)
		QCOMPARE(spreadsheet.column(i)->plotDesignation(), AbstractColumn::PlotDesignation::Y);

	QCOMPARE(spreadsheet.column(0)->valueAt(0), 1000);
	QCOMPARE(spreadsheet.column(1)->valueAt(0), 1001);
	QCOMPARE(spreadsheet.column(2)->valueAt(0), 1002);
	QCOMPARE(spreadsheet.column(3)->valueAt(0), 1003);
	QCOMPARE(spreadsheet.column(0)->valueAt(1), 1100);
	QCOMPARE(spreadsheet.column(1)->valueAt(1), 1101);
	QCOMPARE(spreadsheet.column(2)->valueAt(1), 1102);
	QCOMPARE(spreadsheet.column(3)->valueAt(1), 1103);
	QCOMPARE(spreadsheet.column(0)->valueAt(2), 1200);
	QCOMPARE(spreadsheet.column(1)->valueAt(2), 1207);
	QCOMPARE(spreadsheet.column(2)->valueAt(2), 1202);
	QCOMPARE(spreadsheet.column(3)->valueAt(2), 1203);
	QCOMPARE(spreadsheet.column(0)->valueAt(3), 1300);
	QCOMPARE(spreadsheet.column(1)->valueAt(3), 1301);
	QCOMPARE(spreadsheet.column(2)->valueAt(3), 1302);
	QCOMPARE(spreadsheet.column(3)->valueAt(3), 1303);
}

void HDF5FilterTest::testImportIntPortion() {
	Spreadsheet spreadsheet(QStringLiteral("test"), false);
	HDF5Filter filter;

	const QString& fileName = QFINDTESTDATA(QLatin1String("data/hdf5_test.h5"));
	filter.setCurrentDataSetName(QLatin1String("/arrays/2D int array"));
	// set start/end row/col
	filter.setStartRow(2);
	filter.setEndRow(3);
	filter.setStartColumn(2);
	filter.setEndColumn(3);
	filter.readDataFromFile(fileName, &spreadsheet, AbstractFileFilter::ImportMode::Replace);

	QCOMPARE(spreadsheet.columnCount(), 2);
	QCOMPARE(spreadsheet.rowCount(), 2);
	for (int i = 0; i < 2; i++) {
		QCOMPARE(spreadsheet.column(i)->columnMode(), AbstractColumn::ColumnMode::Integer);
		QCOMPARE(spreadsheet.column(i)->name(), QLatin1String("2D int array_") + QString::number(i + 1));
	}

	QCOMPARE(spreadsheet.column(0)->plotDesignation(), AbstractColumn::PlotDesignation::X);
	QCOMPARE(spreadsheet.column(1)->plotDesignation(), AbstractColumn::PlotDesignation::Y);

	QCOMPARE(spreadsheet.column(0)->valueAt(0), 1101);
	QCOMPARE(spreadsheet.column(1)->valueAt(0), 1102);
	QCOMPARE(spreadsheet.column(0)->valueAt(1), 1207);
	QCOMPARE(spreadsheet.column(1)->valueAt(1), 1202);
}

void HDF5FilterTest::testImportVLEN() {
	Spreadsheet spreadsheet(QStringLiteral("test"), false);
	HDF5Filter filter;

	const QString& fileName = QFINDTESTDATA(QLatin1String("data/h5ex_t_vlen.h5"));
	filter.setCurrentDataSetName(QLatin1String("/DS1"));
	// set start/end row/col
	// filter.setStartRow(2);
	// filter.setEndRow(3);
	// filter.setStartColumn(2);
	// filter.setEndColumn(3);
	filter.readDataFromFile(fileName, &spreadsheet, AbstractFileFilter::ImportMode::Replace);

	QCOMPARE(spreadsheet.columnCount(), 2);
	QCOMPARE(spreadsheet.rowCount(), 12);
	for (int i = 0; i < 2; i++) {
		QCOMPARE(spreadsheet.column(i)->columnMode(), AbstractColumn::ColumnMode::Integer);
		QCOMPARE(spreadsheet.column(i)->name(), QLatin1String("DS1_") + QString::number(i + 1));
	}

	QCOMPARE(spreadsheet.column(0)->plotDesignation(), AbstractColumn::PlotDesignation::X);
	QCOMPARE(spreadsheet.column(1)->plotDesignation(), AbstractColumn::PlotDesignation::Y);

	QCOMPARE(spreadsheet.column(0)->valueAt(0), 3);
	QCOMPARE(spreadsheet.column(1)->valueAt(0), 1);
	QCOMPARE(spreadsheet.column(0)->valueAt(1), 2);
	QCOMPARE(spreadsheet.column(1)->valueAt(1), 1);
	QCOMPARE(spreadsheet.column(0)->valueAt(2), 1);
	QCOMPARE(spreadsheet.column(1)->valueAt(2), 2);
	QCOMPARE(spreadsheet.column(0)->valueAt(3), 0);
	QCOMPARE(spreadsheet.column(1)->valueAt(3), 3);
	QCOMPARE(spreadsheet.column(0)->valueAt(11), 0);
	QCOMPARE(spreadsheet.column(1)->valueAt(11), 144);
}
void HDF5FilterTest::testImportVLENPortion() {
	Spreadsheet spreadsheet(QStringLiteral("test"), false);
	HDF5Filter filter;

	const QString& fileName = QFINDTESTDATA(QLatin1String("data/h5ex_t_vlen.h5"));
	filter.setCurrentDataSetName(QLatin1String("/DS1"));
	// set start/end row/col
	filter.setStartRow(2);
	filter.setEndRow(5);
	filter.readDataFromFile(fileName, &spreadsheet, AbstractFileFilter::ImportMode::Replace);

	QCOMPARE(spreadsheet.columnCount(), 2);
	QCOMPARE(spreadsheet.rowCount(), 4);
	for (int i = 0; i < 2; i++) {
		QCOMPARE(spreadsheet.column(i)->columnMode(), AbstractColumn::ColumnMode::Integer);
		QCOMPARE(spreadsheet.column(i)->name(), QLatin1String("DS1_") + QString::number(i + 1));
	}

	QCOMPARE(spreadsheet.column(0)->plotDesignation(), AbstractColumn::PlotDesignation::X);
	QCOMPARE(spreadsheet.column(1)->plotDesignation(), AbstractColumn::PlotDesignation::Y);

	QCOMPARE(spreadsheet.column(0)->valueAt(0), 2);
	QCOMPARE(spreadsheet.column(1)->valueAt(0), 1);
	QCOMPARE(spreadsheet.column(0)->valueAt(1), 1);
	QCOMPARE(spreadsheet.column(1)->valueAt(1), 2);
	QCOMPARE(spreadsheet.column(0)->valueAt(2), 0);
	QCOMPARE(spreadsheet.column(1)->valueAt(2), 3);
	QCOMPARE(spreadsheet.column(0)->valueAt(3), 0);
	QCOMPARE(spreadsheet.column(1)->valueAt(3), 5);

	// first column
	HDF5Filter filter2;
	filter2.setCurrentDataSetName(QLatin1String("/DS1"));
	filter2.setStartRow(2);
	filter2.setEndRow(5);
	filter2.setEndColumn(1);
	filter2.readDataFromFile(fileName, &spreadsheet, AbstractFileFilter::ImportMode::Replace);

	QCOMPARE(spreadsheet.columnCount(), 1);
	QCOMPARE(spreadsheet.rowCount(), 2);
	QCOMPARE(spreadsheet.column(0)->columnMode(), AbstractColumn::ColumnMode::Integer);
	QCOMPARE(spreadsheet.column(0)->name(), QLatin1String("DS1_1"));

	QCOMPARE(spreadsheet.column(0)->plotDesignation(), AbstractColumn::PlotDesignation::X);

	QCOMPARE(spreadsheet.column(0)->valueAt(0), 2);
	QCOMPARE(spreadsheet.column(0)->valueAt(1), 1);

	// second column
	HDF5Filter filter3;
	filter3.setCurrentDataSetName(QLatin1String("/DS1"));
	filter3.setStartRow(2);
	filter3.setEndRow(5);
	filter3.setStartColumn(2);
	filter3.readDataFromFile(fileName, &spreadsheet, AbstractFileFilter::ImportMode::Replace);

	QCOMPARE(spreadsheet.columnCount(), 1);
	QCOMPARE(spreadsheet.rowCount(), 4);
	QCOMPARE(spreadsheet.column(0)->columnMode(), AbstractColumn::ColumnMode::Integer);
	QCOMPARE(spreadsheet.column(0)->name(), QLatin1String("DS1_2"));

	QCOMPARE(spreadsheet.column(0)->plotDesignation(), AbstractColumn::PlotDesignation::X);

	QCOMPARE(spreadsheet.column(0)->valueAt(0), 1);
	QCOMPARE(spreadsheet.column(0)->valueAt(1), 2);
	QCOMPARE(spreadsheet.column(0)->valueAt(2), 3);
	QCOMPARE(spreadsheet.column(0)->valueAt(3), 5);
}

// BENCHMARKS

void HDF5FilterTest::benchDoubleImport_data() {
	QTest::addColumn<size_t>("lineCount");
	// can't transfer file name since needed in clean up

	QTemporaryFile file;
	if (!file.open()) // needed to generate file name
		return;
	file.close(); // only file name is used

	benchDataFileName = file.fileName();
	benchDataFileName.append(QStringLiteral(".h5"));

	QString testName(QString::number(paths) + QLatin1String(" random double paths"));

	QTest::newRow(qPrintable(testName)) << lines;
	DEBUG("CREATE DATA FILE " << STDSTRING(benchDataFileName) << ", lines = " << lines)

	gsl_rng_env_setup();
	gsl_rng* r = gsl_rng_alloc(gsl_rng_default);
	gsl_rng_set(r, 12345);

	// create file
	// see https://support.hdfgroup.org/HDF5/Tutor/introductory.html
	hid_t file_id = H5Fcreate(qPrintable(benchDataFileName), H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);

	/* Create the data space for the dataset. */
	hsize_t dims[2];
	dims[0] = lines;
	dims[1] = paths;
	hid_t dataspace_id = H5Screate_simple(2, dims, nullptr);

	/* Create the dataset. */
	hid_t dataset_id = H5Dcreate(file_id, "/data", H5T_NATIVE_DOUBLE, dataspace_id, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);

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

	herr_t status = H5Dwrite(dataset_id, H5T_NATIVE_DOUBLE, H5S_ALL, H5S_ALL, H5P_DEFAULT, data);
	if (status < 0)
		WARN(Q_FUNC_INFO << ", ERROR writing data")
	status = H5Dclose(dataset_id);
	status = H5Sclose(dataspace_id);
	status = H5Fclose(file_id);

	delete[] data;

	DEBUG(Q_FUNC_INFO << ", DONE")
}

void HDF5FilterTest::benchDoubleImport() {
	Spreadsheet spreadsheet(QStringLiteral("test"), false);
	HDF5Filter filter;
	filter.setCurrentDataSetName(QLatin1String("/data"));

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

void HDF5FilterTest::benchDoubleImport_cleanup() {
	DEBUG("REMOVE DATA FILE " << STDSTRING(benchDataFileName))
	QFile::remove(benchDataFileName);
}

QTEST_MAIN(HDF5FilterTest)
