/***************************************************************************
    File                 : NSLGeomTest.cpp
    Project              : LabPlot
    Description          : NSL Tests for geometric functions
    --------------------------------------------------------------------
    Copyright            : (C) 2019 Stefan Gerlach (stefan.gerlach@uni.kn)
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *  This program is free software; you can redistribute it and/or modify   *
 *  it under the terms of the GNU General Public License as published by   *
 *  the Free Software Foundation; either version 2 of the License, or      *
 *  (at your option) any later version.                                    *
 *                                                                         *
 *  This program is distributed in the hope that it will be useful,        *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 *  GNU General Public License for more details.                           *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the Free Software           *
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor,                    *
 *   Boston, MA  02110-1301  USA                                           *
 *                                                                         *
 ***************************************************************************/

#include "NSLGeomTest.h"

extern "C" {
#include "backend/nsl/nsl_geom.h"
#include "backend/nsl/nsl_geom_linesim.h"
}

void NSLGeomTest::initTestCase() {
	const QString currentDir = __FILE__;
	m_dataDir = currentDir.left(currentDir.lastIndexOf(QDir::separator())) + QDir::separator() + QLatin1String("data") + QDir::separator();
}

//##############################################################################
//#################  line sim test
//##############################################################################

void NSLGeomTest::testDist() {
	double dist = nsl_geom_point_point_dist(0, 0, 1 , 1);
	QCOMPARE(dist, M_SQRT2);
	dist = nsl_geom_point_point_dist(1, 2, 2 , 1);
	QCOMPARE(dist, M_SQRT2);
	dist = nsl_geom_point_point_dist(-1, -2, 2, 2);
	QCOMPARE(dist, 5.);

	dist = nsl_geom_point_line_dist(0, 0, 1, 0, .5, 1);
	QCOMPARE(dist, 1.);
	dist = nsl_geom_point_line_dist(0, 0, 1, 0, 0, 1);
	QCOMPARE(dist, 1.);
	dist = nsl_geom_point_line_dist(0, 0, 1, 0, 1, 1);
	QCOMPARE(dist, 1.);
	dist = nsl_geom_point_line_dist(0, 0, 1, 1, 0, 1);
	QCOMPARE(dist, M_SQRT1_2);
	dist = nsl_geom_point_line_dist(0, 0, 1, 1, 1, 0);
	QCOMPARE(dist, M_SQRT1_2);

	//TODO: nsl_geom_point_line_dist_y
	//TODO: nsl_geom_three_point_area

	dist = nsl_geom_point_point_dist3(0, 0, 0, 1, 1, 1);
	QCOMPARE(dist, M_SQRT3);
	dist = nsl_geom_point_point_dist3(-1, -1, 1, 1, 1, 1);
	QCOMPARE(dist, 2.*M_SQRT2);
}

void NSLGeomTest::testLineSim() {
	const double xdata[] = {1, 2, 2.5, 3, 4, 7, 9, 11, 13, 14};
	const double ydata[] = {1, 1, 1, 3, 4, 7, 8, 12, 13, 13};
	const size_t n = 10;

	double atol = nsl_geom_linesim_clip_diag_perpoint(xdata, ydata, n);
	printf("automatic tol clip_diag_perpoint = %.15g\n", atol);
	QCOMPARE(atol, 1.76918060129541);
	atol = nsl_geom_linesim_clip_area_perpoint(xdata, ydata, n);
	printf("automatic tol clip_area_perpoint = %.15g\n", atol);
	QCOMPARE(atol, 15.6);
	atol = nsl_geom_linesim_avg_dist_perpoint(xdata, ydata, n);
	printf("automatic tol avg_dist = %.15g\n", atol);
	QCOMPARE(atol, 1.91626789723004);

	size_t index[n], i;
	const double tol = 0.6;
	const size_t result[] = {0, 2, 3, 6, 7, 9};
	printf("* Simplification (Douglas Peucker)\n");
	size_t nout = nsl_geom_linesim_douglas_peucker(xdata, ydata, n, tol, index);
	double perr = nsl_geom_linesim_positional_squared_error(xdata, ydata, n, index);
	double aerr = nsl_geom_linesim_area_error(xdata, ydata, n, index);
	printf("pos. error = %.15g, area error = %.15g\n", perr, aerr);
	QCOMPARE(nout, 6uL);
	QCOMPARE(perr, 0.0378688524590164);
	QCOMPARE(aerr, 0.25);

	for (i = 0; i < nout; i++)
		QCOMPARE(index[i], result[i]);

	const size_t no = 6;
	printf("* Simplification (Douglas Peucker variant) nout = %zu\n", no);
	double tolout = nsl_geom_linesim_douglas_peucker_variant(xdata, ydata, n, no, index);
	perr = nsl_geom_linesim_positional_squared_error(xdata, ydata, n, index);
	aerr = nsl_geom_linesim_area_error(xdata, ydata, n, index); 
	printf("tolout = %.15g, pos. error = %.15g, area error = %.15g)\n", tolout, perr, aerr);
	QCOMPARE(tolout, 0.994505452921406);
	QCOMPARE(perr, 0.0378688524590164);
	QCOMPARE(aerr, 0.25);

	for (i = 0; i < no; i++)
		QCOMPARE(index[i], result[i]);

        const size_t np = 2;
	const size_t result2[] = {0, 2, 4, 6, 8, 9};
        printf("* N-th point\n");
        nout = nsl_geom_linesim_nthpoint(n, np, index);
	perr = nsl_geom_linesim_positional_squared_error(xdata, ydata, n, index);
	aerr = nsl_geom_linesim_area_error(xdata, ydata, n, index);
        printf("pos. error = %.15g, area error = %.15g\n", perr, aerr);
	QCOMPARE(nout, 6uL);
	QCOMPARE(perr, 0.129756097560976);
	QCOMPARE(aerr, 0.525);

        for (i = 0; i < nout; i++)
		QCOMPARE(index[i], result2[i]);

        const double tol2 = 1.5;
	const size_t result3[] = {0, 3, 5, 6, 7, 9};
        printf("* Radial distance (tol = %g)\n", tol2);
        nout = nsl_geom_linesim_raddist(xdata, ydata, n, tol2, index);
	perr = nsl_geom_linesim_positional_squared_error(xdata, ydata, n, index);
	aerr = nsl_geom_linesim_area_error(xdata, ydata, n, index);
        printf("pos. error = %.15g, area error = %.15g\n", perr, aerr);
	QCOMPARE(nout, 6uL);
	QCOMPARE(perr, 0.1725);
	QCOMPARE(aerr, 0.2);

        for (i = 0; i < nout; i++)
		QCOMPARE(index[i], result3[i]);

        const double tol3 = 0.5;
        const size_t repeat = 3;
	const size_t result4[] = {0, 2, 4, 6, 7, 9};
        printf("* Perpendicular distance (repeat = %zu)\n", repeat);
        nout = nsl_geom_linesim_perpdist_repeat(xdata, ydata, n, tol3, repeat, index);
	perr = nsl_geom_linesim_positional_squared_error(xdata, ydata, n, index);
	aerr = nsl_geom_linesim_area_error(xdata, ydata, n, index);
        printf("pos. error = %.15g, area error = %.15g\n", perr, aerr);
	QCOMPARE(nout, 6uL);
	QCOMPARE(perr, 0.0519512195121951);
	QCOMPARE(aerr, 0.275);

        for (i = 0; i < nout; i++)
		QCOMPARE(index[i], result4[i]);

        const double tol4 = 0.7;
        printf("* Y distance (interpolation)\n");
        nout = nsl_geom_linesim_interp(xdata, ydata, n, tol4, index);
	perr = nsl_geom_linesim_positional_squared_error(xdata, ydata, n, index);
	aerr = nsl_geom_linesim_area_error(xdata, ydata, n, index);
        printf("pos. error = %.15g, area error = %.15g\n", perr, aerr);
	QCOMPARE(nout, 6uL);
	QCOMPARE(perr, 0.0378688524590164);
	QCOMPARE(aerr, 0.25);

        for (i = 0; i < nout; i++)
		QCOMPARE(index[i], result[i]);

        const double tol5 = 1.6;
        printf("* minimum area (Visvalingam-Whyatt)\n");
        nout = nsl_geom_linesim_visvalingam_whyatt(xdata, ydata, n, tol5, index);
	perr = nsl_geom_linesim_positional_squared_error(xdata, ydata, n, index);
	aerr = nsl_geom_linesim_area_error(xdata, ydata, n, index);
        printf("pos. error = %.15g, area error = %.15g\n", perr, aerr);
	QCOMPARE(nout, 6uL);
	QCOMPARE(perr, 0.1725);
	QCOMPARE(aerr, 0.2);

        for (i = 0; i < nout; i++)
		QCOMPARE(index[i], result3[i]);

	const size_t result5[] = {0, 2, 3, 5, 6, 7, 9};
        printf("* Perp. distance (Reumann-Witkam)\n");
        nout = nsl_geom_linesim_reumann_witkam(xdata, ydata, n, tol3, index);
	perr = nsl_geom_linesim_positional_squared_error(xdata, ydata, n, index);
	aerr = nsl_geom_linesim_area_error(xdata, ydata, n, index);
        printf("pos. error = %.15g, area error = %.15g\n", perr, aerr);
	QCOMPARE(nout, 7uL);
	QCOMPARE(perr, 0.01);
	QCOMPARE(aerr, 0.05);

        for (i = 0; i < nout; i++)
		QCOMPARE(index[i], result5[i]);

        const double mintol = 2.0;
        const double maxtol = 7.0;
        printf("* Perp. distance (Opheim)\n");
        nout = nsl_geom_linesim_opheim(xdata, ydata, n, mintol, maxtol, index);
	perr = nsl_geom_linesim_positional_squared_error(xdata, ydata, n, index);
	aerr = nsl_geom_linesim_area_error(xdata, ydata, n, index);
        printf("pos. error = %.15g, area error = %.15g\n", perr, aerr);
	QCOMPARE(nout, 6uL);
	QCOMPARE(perr, 0.129756097560976);
	QCOMPARE(aerr, 0.525);

        for (i = 0; i < nout; i++)
		QCOMPARE(index[i], result2[i]);

        const size_t region = 5;
        printf("* Simplification (Lang)\n");
        nout = nsl_geom_linesim_lang(xdata, ydata, n, tol3, region, index);
	perr = nsl_geom_linesim_positional_squared_error(xdata, ydata, n, index);
	aerr = nsl_geom_linesim_area_error(xdata, ydata, n, index);
        printf("pos. error = %.15g, area error = %.15g\n", perr, aerr);
	QCOMPARE(nout, 6uL);
	QCOMPARE(perr, 0.0519512195121951);
	QCOMPARE(aerr, 0.275);

        for (i = 0; i < nout; i++)
		QCOMPARE(index[i], result4[i]);
}

void NSLGeomTest::testLineSimMorse() {
	printf("NSLGeomTest::testLineSimMorse()\n");

	const QString fileName = m_dataDir + "morse_code.dat";
	FILE *file;
	if((file = fopen(fileName.toLocal8Bit().constData(), "r")) == NULL) {
		printf("ERROR reading %s. Giving up.\n", fileName.toLocal8Bit().constData());
		return;
	}

	const int N = 152000;
	const int NOUT = 15200;

	printf("NSLGeomTest::testLineSimMorse(): allocating space for reading data\n");
	double* xdata = (double *)malloc(N*sizeof(double));
	if (xdata == NULL)
		return;
	double* ydata = (double *)malloc(N*sizeof(double));
	if (ydata == NULL) {
		free(xdata);
		return;
	}

	printf("NSLGeomTest::testLineSimMorse(): reading data from file\n");
	size_t i;
	for (i = 0; i < N; i++)
		fscanf(file,"%lf %lf", &xdata[i], &ydata[i]);

	double atol = nsl_geom_linesim_clip_diag_perpoint(xdata, ydata, N);
	printf("automatic tol clip_diag_perpoint = %.15g\n", atol);
	QCOMPARE(atol, 0.999993446759985);
	atol = nsl_geom_linesim_clip_area_perpoint(xdata, ydata, N);
	printf("automatic tol clip_area_perpoint = %.15g\n", atol);
	QCOMPARE(atol, 34.4653732526316);
	atol = nsl_geom_linesim_avg_dist_perpoint(xdata, ydata, N);
	printf("automatic tol avg_dist = %.15g\n", atol);
	QCOMPARE(atol, 4.72091524721907);

	printf("* Simplification (Douglas Peucker variant) nout = %d\n", NOUT);

	double tolout;
	size_t index[N];
        QBENCHMARK {
		tolout = nsl_geom_linesim_douglas_peucker_variant(xdata, ydata, N, NOUT, index);
		QCOMPARE(tolout, 11.5280857733246);
        }

	double perr = nsl_geom_linesim_positional_squared_error(xdata, ydata, N, index);
	double aerr = nsl_geom_linesim_area_error(xdata, ydata, N, index);
	printf("maxtol = %.15g (pos. error = %.15g, area error = %.15g)\n", tolout, perr, aerr);
	QCOMPARE(perr, 11.9586266895937);
	QCOMPARE(aerr, 17.558046450762);

	free(xdata);
	free(ydata);
}

//##############################################################################
//#################  performance
//##############################################################################

QTEST_MAIN(NSLGeomTest)
