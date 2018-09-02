/***************************************************************************
    File                 : errors.h
    Project              : LabPlot
    Description          : Translatable strings for GSL error codes
    --------------------------------------------------------------------
    Copyright            : (C) 2017 Alexander Semke (alexander.semke@web.de)

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

#ifndef GSL_ERRORS_H
#define GSL_ERRORS_H

#include <gsl/gsl_errno.h>
#include <KLocalizedString>

namespace {
QString gslErrorToString(int status) {
	switch (status) {
		case GSL_SUCCESS:
			return i18n("Success");
		case GSL_FAILURE:
			return i18n("Failure");
		case GSL_CONTINUE:
			return i18n("Iteration has not converged");
		case GSL_EDOM:
			return i18n("Input domain error, e.g sqrt(-1)");
		case GSL_ERANGE:
			return i18n("Output range error, e.g. exp(1e100)");
		case GSL_EFAULT:
			return i18n("Invalid pointer");
		case GSL_EINVAL:
			return i18n("Invalid argument supplied");
		case GSL_EFAILED:
			return i18n("Generic failure");
		case GSL_EFACTOR:
			return i18n("Factorization failed");
		case GSL_ENOMEM:
			return i18n("Failed to allocate memory");
		case GSL_EBADFUNC:
			return i18n("Problem with supplied function");
		case GSL_ERUNAWAY:
			return i18n("Iterative process is out of control");
		case GSL_EMAXITER:
			return i18n("Exceeded max number of iterations");
		case GSL_EZERODIV:
			return i18n("Tried to divide by zero");
		case GSL_EBADTOL:
			return i18n("Invalid tolerance specified");
		case GSL_ETOL:
			return i18n("Failed to reach the specified tolerance");
		case GSL_EUNDRFLW:
			return i18n("Underflow");
		case GSL_EOVRFLW:
			return i18n("Overflow");
		case GSL_ELOSS:
			return i18n("Loss of accuracy");
		case GSL_EROUND:
			return i18n("Failed because of roundoff error");
		case GSL_EBADLEN:
			return i18n("Matrix, vector lengths are not conformant");
		case GSL_ENOTSQR:
			return i18n("Matrix not square");
		case GSL_ESING:
			return i18n("Apparent singularity detected");
		case GSL_EDIVERGE:
			return i18n("Integral or series is divergent");
		case GSL_EUNSUP:
			return i18n("Requested feature is not supported by the hardware");
		case GSL_EUNIMPL:
			return i18n("Requested feature not (yet) implemented");
		case GSL_ECACHE:
			return i18n("Cache limit exceeded");
		case GSL_ETABLE:
			return i18n("Table limit exceeded");
		case GSL_ENOPROG:
			return i18n("Iteration is not making progress towards solution");
		case GSL_ENOPROGJ:
			return i18n("Jacobian evaluations are not improving the solution");
		case GSL_ETOLF:
			return i18n("Cannot reach the specified tolerance in F");
		case GSL_ETOLX:
			return i18n("Cannot reach the specified tolerance in X");
		case GSL_ETOLG:
			return i18n("Cannot reach the specified tolerance in gradient");
		case GSL_EOF:
			return i18n("End of file");
		default:
			return QString(gsl_strerror(status));
	}
}
}
#endif
