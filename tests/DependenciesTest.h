/*
	File                 : DependenciesTest.h
	Project              : LabPlot
	Description          : Tests for dependencies
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2023 Stefan Gerlach <stefan.gerlach@uni.kn>

	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef DEPENDENCIESTEST_H
#define DEPENDENCIESTEST_H

#include <QtTest>

class DependenciesTest : public QObject {
	Q_OBJECT

private Q_SLOTS:
#ifdef MATIO_DISABLED
	void checkMatio() {
		QSKIP("Skipping Matio Tests, because it was disabled!");
	}
#elif !defined(HAVE_MATIO)
	void checkMatio() {
		QSKIP("Skipping Matio Tests, because it was not found!");
	}
#endif
#ifdef HDF5_DISABLED
	void checkHDF5() {
		QSKIP("Skipping HDF5 Tests, because it was disabled!");
	}
#elif !defined(HAVE_HDF5)
	void checkHDF5() {
		QSKIP("Skipping HDF5 Tests, because it was not found!");
	}
#endif
#ifdef NETCDF_DISABLED
	void checkNetCDF() {
		QSKIP("Skipping NetCDF Tests, because it was disabled!");
	}
#elif !defined(HAVE_NETCDF)
	void checkNetCDF() {
		QSKIP("Skipping NetCDF Tests, because it was not found!");
	}
#endif
#ifdef FITS_DISABLED
	void checkFITS() {
		QSKIP("Skipping FITS Tests, because it was disabled!");
	}
#elif !defined(HAVE_FITS)
	void checkFITS() {
		QSKIP("Skipping FITS Tests, because it was not found!");
	}
#endif
#ifdef READSTAT_DISABLED
	void checkReadStat() {
		QSKIP("Skipping ReadStat Tests, because it was disabled!");
	}
#elif !defined(HAVE_READSTAT)
	void checkReadStat() {
		QSKIP("Skipping ReadStat Tests, because it was not found or could not be build!");
	}
#endif
#ifdef XLSX_DISABLED
	void checkXLSX() {
		QSKIP("Skipping XLSX Tests, because it was disabled!");
	}
#elif !defined(HAVE_QXLSX)
	void checkXLSX() {
		QSKIP("Skipping XLSX Tests, because it was not found or could not be build!");
	}
#endif
#ifdef ROOT_DISABLED
	void checkROOT() {
		QSKIP("Skipping ROOT Tests, because it was disabled!");
	}
#elif !defined(HAVE_ZIP)
	void checkROOT() {
		QSKIP("Skipping ROOT Tests, because ZLIB or LZ4 was not found!");
	}
#endif
#ifdef CANTOR_DISABLED
	void checkCantor() {
		QSKIP("Skipping Cantor Tests, because it was disabled!");
	}
#elif !defined(HAVE_CANTOR_LIBS) && !defined(HAVE_NEW_CANTOR_LIBS)
	void checkCantor() {
		QSKIP("Skipping Cantor Tests, because it was not found!");
	}
#endif
#ifdef VECTOR_BLF_DISABLED
	void checkVectorBLF() {
		QSKIP("Skipping Vector BLF Tests, because it was disabled!");
	}
#elif !defined(HAVE_VECTOR_BLF)
	void checkVectorBLF() {
		QSKIP("Skipping Vector Tests, because it was not found!");
	}
#endif
};

#endif
