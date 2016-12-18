
/***************************************************************************
    File                 : XYFitCurve.cpp
    Project              : LabPlot
    Description          : A xy-curve defined by a fit model
    --------------------------------------------------------------------
    Copyright            : (C) 2014 Alexander Semke (alexander.semke@web.de)
    Copyright            : (C) 2016 Stefan Gerlach (stefan.gerlach@uni.kn)

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

/*!
  \class XYFitCurve
  \brief A xy-curve defined by a fit model

  \ingroup worksheet
*/

#include "XYFitCurve.h"
#include "XYFitCurvePrivate.h"
#include "backend/core/AbstractColumn.h"
#include "backend/core/column/Column.h"
#include "backend/lib/commandtemplates.h"
#include "backend/lib/macros.h"
#include "backend/gsl/ExpressionParser.h"

extern "C" {
#include <gsl/gsl_blas.h>
#include <gsl/gsl_vector.h>
#include <gsl/gsl_matrix.h>
#include <gsl/gsl_version.h>
#include "backend/gsl/parser.h" 
#include "backend/nsl/nsl_fit.h"
}
#include <cmath>

#include <KIcon>
#include <KLocale>
#include <QElapsedTimer>
#include <QThreadPool>

XYFitCurve::XYFitCurve(const QString& name)
		: XYCurve(name, new XYFitCurvePrivate(this)) {
	init();
}

XYFitCurve::XYFitCurve(const QString& name, XYFitCurvePrivate* dd)
		: XYCurve(name, dd) {
	init();
}

XYFitCurve::~XYFitCurve() {
	//no need to delete the d-pointer here - it inherits from QGraphicsItem
	//and is deleted during the cleanup in QGraphicsScene
}

void XYFitCurve::init() {
	Q_D(XYFitCurve);

	//TODO: read from the saved settings for XYFitCurve?
	d->lineType = XYCurve::Line;
	d->symbolsStyle = Symbol::NoSymbols;
}

void XYFitCurve::recalculate() {
	Q_D(XYFitCurve);
	d->recalculate();
}

/*!
	Returns an icon to be used in the project explorer.
*/
QIcon XYFitCurve::icon() const {
	return KIcon("labplot-xy-fit-curve");
}

//##############################################################################
//##########################  getter methods  ##################################
//##############################################################################
BASIC_SHARED_D_READER_IMPL(XYFitCurve, const AbstractColumn*, xDataColumn, xDataColumn)
BASIC_SHARED_D_READER_IMPL(XYFitCurve, const AbstractColumn*, yDataColumn, yDataColumn)
BASIC_SHARED_D_READER_IMPL(XYFitCurve, const AbstractColumn*, weightsColumn, weightsColumn)
const QString& XYFitCurve::xDataColumnPath() const { Q_D(const XYFitCurve); return d->xDataColumnPath; }
const QString& XYFitCurve::yDataColumnPath() const { Q_D(const XYFitCurve); return d->yDataColumnPath; }
const QString& XYFitCurve::weightsColumnPath() const { Q_D(const XYFitCurve);return d->weightsColumnPath; }

BASIC_SHARED_D_READER_IMPL(XYFitCurve, XYFitCurve::FitData, fitData, fitData)

const XYFitCurve::FitResult& XYFitCurve::fitResult() const {
	Q_D(const XYFitCurve);
	return d->fitResult;
}

bool XYFitCurve::isSourceDataChangedSinceLastFit() const {
	Q_D(const XYFitCurve);
	return d->sourceDataChangedSinceLastFit;
}

//##############################################################################
//#################  setter methods and undo commands ##########################
//##############################################################################
STD_SETTER_CMD_IMPL_S(XYFitCurve, SetXDataColumn, const AbstractColumn*, xDataColumn)
void XYFitCurve::setXDataColumn(const AbstractColumn* column) {
	Q_D(XYFitCurve);
	if (column != d->xDataColumn) {
		exec(new XYFitCurveSetXDataColumnCmd(d, column, i18n("%1: assign x-data")));
		emit sourceDataChangedSinceLastFit();
		if (column) {
			connect(column, SIGNAL(dataChanged(const AbstractColumn*)), this, SLOT(handleSourceDataChanged()));
			//TODO disconnect on undo
		}
	}
}

STD_SETTER_CMD_IMPL_S(XYFitCurve, SetYDataColumn, const AbstractColumn*, yDataColumn)
void XYFitCurve::setYDataColumn(const AbstractColumn* column) {
	Q_D(XYFitCurve);
	if (column != d->yDataColumn) {
		exec(new XYFitCurveSetYDataColumnCmd(d, column, i18n("%1: assign y-data")));
		emit sourceDataChangedSinceLastFit();
		if (column) {
			connect(column, SIGNAL(dataChanged(const AbstractColumn*)), this, SLOT(handleSourceDataChanged()));
			//TODO disconnect on undo
		}
	}
}

STD_SETTER_CMD_IMPL_S(XYFitCurve, SetWeightsColumn, const AbstractColumn*, weightsColumn)
void XYFitCurve::setWeightsColumn(const AbstractColumn* column) {
	Q_D(XYFitCurve);
	if (column != d->weightsColumn) {
		exec(new XYFitCurveSetWeightsColumnCmd(d, column, i18n("%1: assign weights")));
		emit sourceDataChangedSinceLastFit();
		if (column) {
			connect(column, SIGNAL(dataChanged(const AbstractColumn*)), this, SLOT(handleSourceDataChanged()));
			//TODO disconnect on undo
		}
	}
}

STD_SETTER_CMD_IMPL_F_S(XYFitCurve, SetFitData, XYFitCurve::FitData, fitData, recalculate);
void XYFitCurve::setFitData(const XYFitCurve::FitData& fitData) {
	Q_D(XYFitCurve);
	exec(new XYFitCurveSetFitDataCmd(d, fitData, i18n("%1: set fit options and perform the fit")));
}

//##############################################################################
//################################## SLOTS ####################################
//##############################################################################
void XYFitCurve::handleSourceDataChanged() {
	Q_D(XYFitCurve);
	d->sourceDataChangedSinceLastFit = true;
	emit sourceDataChangedSinceLastFit();
}

//##############################################################################
//######################### Private implementation #############################
//##############################################################################
XYFitCurvePrivate::XYFitCurvePrivate(XYFitCurve* owner) : XYCurvePrivate(owner),
	xDataColumn(0), yDataColumn(0), weightsColumn(0),
	xColumn(0), yColumn(0), residualsColumn(0),
	xVector(0), yVector(0), residualsVector(0),
	sourceDataChangedSinceLastFit(false),
	q(owner)  {

}

XYFitCurvePrivate::~XYFitCurvePrivate() {
	//no need to delete xColumn and yColumn, they are deleted
	//when the parent aspect is removed
}

/* data structure to pass parameter to functions */
struct data {
	size_t n;	//number of data points
	double* x;	//pointer to the vector with x-data values
	double* y;	//pointer to the vector with y-data values
	double* sigma;	//pointer to the vector with sigma values
	nsl_fit_model_category modelCategory;
	unsigned int modelType;
	int degree;
	QString* func;	// string containing the definition of the model/function
	QStringList* paramNames;
	double* paramMin;	// lower parameter limits
	double* paramMax;	// upper parameter limits
	bool* paramFixed;	// parameter fixed?
};

/*!
 * \param paramValues vector containing current values of the fit parameters
 * \param params
 * \param f vector with the weighted residuals (Yi - y[i])/sigma[i]
 */
int func_f(const gsl_vector* paramValues, void* params, gsl_vector* f) {
	size_t n = ((struct data*)params)->n;
	double* x = ((struct data*)params)->x;
	double* y = ((struct data*)params)->y;
	double* sigma = ((struct data*)params)->sigma;
	nsl_fit_model_category modelCategory = ((struct data*)params)->modelCategory;
	unsigned int modelType = ((struct data*)params)->modelType;
	QByteArray funcba = ((struct data*)params)->func->toLocal8Bit();	// a local byte array is needed!
	QStringList* paramNames = ((struct data*)params)->paramNames;
	double *min = ((struct data*)params)->paramMin;
	double *max = ((struct data*)params)->paramMax;

	// set current values of the parameters
	for (int i = 0; i < paramNames->size(); i++) {
		double x = gsl_vector_get(paramValues, i);
		// bound values if limits are set
		assign_variable(paramNames->at(i).toLocal8Bit().data(), nsl_fit_map_bound(x, min[i], max[i]));
		DEBUG_LOG("Parameter"<<i<<'['<<min[i]<<','<<max[i]<<"] free/bound:"<<QString::number(x, 'g', 15)
			<<' '<<QString::number(nsl_fit_map_bound(x, min[i], max[i]), 'g', 15));
	}

	const char *func = funcba.data();	// function to evaluate
	for (size_t i = 0; i < n; i++) {
		if (std::isnan(x[i]) || std::isnan(y[i]))
			continue;

		// checks for allowed values of x for different models
		// TODO: more to check
		if (modelCategory == nsl_fit_model_distribution && modelType == nsl_fit_model_lognormal) {
			if (x[i] < 0)
				x[i] = 0;
		}

		assign_variable("x", x[i]);
		double Yi = parse(func);

//		DEBUG_LOG("evaluate function"<<QString(func)<<": f(x["<<i<<"]) ="<<Yi);

		if (parse_errors() > 0)
			return GSL_EINVAL;

		if (sigma)
			gsl_vector_set (f, i, (Yi - y[i])/sigma[i]);
		else
			gsl_vector_set (f, i, (Yi - y[i]));
	}

	return GSL_SUCCESS;
}

/*!
 * calculates the matrix elements of Jacobian matrix
 * \param paramValues current parameter values
 * \param params
 * \param J Jacobian matrix
 * */
int func_df(const gsl_vector* paramValues, void* params, gsl_matrix* J) {
	size_t n = ((struct data*)params)->n;
	double* xVector = ((struct data*)params)->x;
	double* sigmaVector = ((struct data*)params)->sigma;
	nsl_fit_model_category modelCategory = ((struct data*)params)->modelCategory;
	unsigned int modelType = ((struct data*)params)->modelType;
	int degree = ((struct data*)params)->degree;
	QStringList* paramNames = ((struct data*)params)->paramNames;
	double *min = ((struct data*)params)->paramMin;
	double *max = ((struct data*)params)->paramMax;
	bool *fixed = ((struct data*)params)->paramFixed;

	// calculate the Jacobian matrix:
	// Jacobian matrix J(i,j) = df_i / dx_j
	// where f_i = (Y_i - y_i)/sigma[i],
	// Y_i = model and the x_j are the parameters

	double x, sigma = 1.0;

	switch (modelCategory) {
	case nsl_fit_model_basic:
		switch (modelType) {
		case nsl_fit_model_polynomial:	// Y(x) = c0 + c1*x + ... + cn*x^n
			for (size_t i = 0; i < n; i++) {
				x = xVector[i];
				if (sigmaVector) sigma = sigmaVector[i];
				for (int j = 0; j < paramNames->size(); ++j) {
					if (fixed[j])
						gsl_matrix_set(J, i, j, 0.);
					else
						gsl_matrix_set(J, i, j, nsl_fit_model_polynomial_param_deriv(x, j, sigma));
				}
			}
			break;
		case nsl_fit_model_power:	// Y(x) = a*x^b or Y(x) = a + b*x^c.
			if (degree == 1) {
				double a = nsl_fit_map_bound(gsl_vector_get(paramValues, 0), min[0], max[0]);
				double b = nsl_fit_map_bound(gsl_vector_get(paramValues, 1), min[1], max[1]);
				for (size_t i = 0; i < n; i++) {
					x = xVector[i];
					if (sigmaVector) sigma = sigmaVector[i];

					for (int j = 0; j < 2; j++) {
						if (fixed[j])
							gsl_matrix_set(J, i, j, 0.);
						else
							gsl_matrix_set(J, i, j, nsl_fit_model_power1_param_deriv(j, x, a, b, sigma));
					}
				}
				} else if (degree == 2) {
				double b = nsl_fit_map_bound(gsl_vector_get(paramValues, 1), min[1], max[1]);
				double c = nsl_fit_map_bound(gsl_vector_get(paramValues, 2), min[2], max[2]);
				for (size_t i = 0; i < n; i++) {
					x = xVector[i];
					if (sigmaVector) sigma = sigmaVector[i];

					for (int j = 0; j < 3; j++) {
						if (fixed[j])
							gsl_matrix_set(J, i, j, 0.);
						else
							gsl_matrix_set(J, i, j, nsl_fit_model_power2_param_deriv(j, x, b, c, sigma));
					}
				}
			}
			break;
		case nsl_fit_model_exponential:	// Y(x) = a*exp(b*x) or Y(x) = a*exp(b*x) + c*exp(d*x) or Y(x) = a*exp(b*x) + c*exp(d*x) + e*exp(f*x)
			if (degree == 1) {
				double a = nsl_fit_map_bound(gsl_vector_get(paramValues, 0), min[0], max[0]);
				double b = nsl_fit_map_bound(gsl_vector_get(paramValues, 1), min[1], max[1]);
				for (size_t i = 0; i < n; i++) {
					x = xVector[i];
					if (sigmaVector) sigma = sigmaVector[i];

					for (int j = 0; j < 2; j++) {
						if (fixed[j])
							gsl_matrix_set(J, i, j, 0.);
						else
							gsl_matrix_set(J, i, j, nsl_fit_model_exponential1_param_deriv(j, x, a, b, sigma));
					}
				}
			} else if (degree == 2) {
				double a = nsl_fit_map_bound(gsl_vector_get(paramValues, 0), min[0], max[0]);
				double b = nsl_fit_map_bound(gsl_vector_get(paramValues, 1), min[1], max[1]);
				double c = nsl_fit_map_bound(gsl_vector_get(paramValues, 2), min[2], max[2]);
				double d = nsl_fit_map_bound(gsl_vector_get(paramValues, 3), min[3], max[3]);
				for (size_t i = 0; i < n; i++) {
					x = xVector[i];
					if (sigmaVector) sigma = sigmaVector[i];
	
					for (int j = 0; j < 4; j++) {
						if (fixed[j])
							gsl_matrix_set(J, i, j, 0.);
						else
							gsl_matrix_set(J, i, j, nsl_fit_model_exponential2_param_deriv(j, x, a, b, c, d, sigma));
					}
				}
			} else if (degree == 3) {
				double a = nsl_fit_map_bound(gsl_vector_get(paramValues, 0), min[0], max[0]);
				double b = nsl_fit_map_bound(gsl_vector_get(paramValues, 1), min[1], max[1]);
				double c = nsl_fit_map_bound(gsl_vector_get(paramValues, 2), min[2], max[2]);
				double d = nsl_fit_map_bound(gsl_vector_get(paramValues, 3), min[3], max[3]);
				double e = nsl_fit_map_bound(gsl_vector_get(paramValues, 4), min[4], max[4]);
				double f = nsl_fit_map_bound(gsl_vector_get(paramValues, 5), min[5], max[5]);
				for (size_t i = 0; i < n; i++) {
					x = xVector[i];
					if (sigmaVector) sigma = sigmaVector[i];

					for (int j = 0; j < 6; j++) {
						if (fixed[j])
							gsl_matrix_set(J, i, j, 0.);
						else
							gsl_matrix_set(J, i, j, nsl_fit_model_exponential3_param_deriv(j, x, a, b, c, d, e, f, sigma));
					}
				}
			}
			break;
		case nsl_fit_model_inverse_exponential: {	// Y(x) = a*(1-exp(b*x))+c
			double a = nsl_fit_map_bound(gsl_vector_get(paramValues, 0), min[0], max[0]);
			double b = nsl_fit_map_bound(gsl_vector_get(paramValues, 1), min[1], max[1]);
			for (size_t i = 0; i < n; i++) {
				x = xVector[i];
				if (sigmaVector) sigma = sigmaVector[i];

				for (int j = 0; j < 3; j++) {
					if (fixed[j])
						gsl_matrix_set(J, i, j, 0.);
					else
						gsl_matrix_set(J, i, j, nsl_fit_model_inverse_exponential_param_deriv(j, x, a, b, sigma));
				}
			}
			break;
		}
		case nsl_fit_model_fourier: {	// Y(x) = a0 + (a1*cos(w*x) + b1*sin(w*x)) + ... + (an*cos(n*w*x) + bn*sin(n*w*x)
			//parameters: w, a0, a1, b1, ... an, bn
			double a[degree];
			double b[degree];
			double w = nsl_fit_map_bound(gsl_vector_get(paramValues, 0), min[0], max[0]);
			a[0] = nsl_fit_map_bound(gsl_vector_get(paramValues, 1), min[1], max[1]);
			b[0] = 0;
			for (int i = 1; i < degree; ++i) {
				a[i] = nsl_fit_map_bound(gsl_vector_get(paramValues, 2*i), min[2*i], max[2*i]);
				b[i] = nsl_fit_map_bound(gsl_vector_get(paramValues, 2*i+1), min[2*i+1], max[2*i+1]);
			}
			for (size_t i = 0; i < n; i++) {
				x = xVector[i];
				if (sigmaVector) sigma = sigmaVector[i];
				double wd = 0; //first derivative with respect to the w parameter
				for (int j = 1; j < degree; ++j) {
					wd += -a[j]*j*x*sin(j*w*x) + b[j]*j*x*cos(j*w*x);
				}

				gsl_matrix_set(J, i, 0, wd/sigma);
				gsl_matrix_set(J, i, 1, 1./sigma);
				for (int j = 1; j <= degree; ++j) {
					gsl_matrix_set(J, i, 2*j, nsl_fit_model_fourier_param_deriv(0, j, x, w, sigma));
					gsl_matrix_set(J, i, 2*j+1, nsl_fit_model_fourier_param_deriv(1, j, x, w, sigma));
				}

				for (int j = 0; j <= 2*degree+1; j++)
					if (fixed[j])
						gsl_matrix_set(J, i, j, 0.);
			}
			break;
        	}
		}
		break;
	case nsl_fit_model_peak:
		switch (modelType) {
		case nsl_fit_model_gaussian: {	// Y(x) = 1./sqrt(2*pi)*( a1/s1 * exp(-((x-mu1)/s1)^2/2) + a2/s2 * exp(-((x-mu2)/s2)^2/2) + ... + an/sn * exp(-((x-mun)/sn)^2/2) )
			double s, mu, a;
			for (size_t i = 0; i < n; i++) {
				x = xVector[i];
				if (sigmaVector) sigma = sigmaVector[i];

				for (int j = 0; j < degree; ++j) {
					s = nsl_fit_map_bound(gsl_vector_get(paramValues, 3*j), min[3*j], max[3*j]);
					mu = nsl_fit_map_bound(gsl_vector_get(paramValues, 3*j+1), min[3*j+1], max[3*j+1]);
					a = nsl_fit_map_bound(gsl_vector_get(paramValues, 3*j+2), min[3*j+2], max[3*j+2]);
				
					gsl_matrix_set(J, i, 3*j, nsl_fit_model_gaussian_param_deriv(0, x, s, mu, a, sigma));
					gsl_matrix_set(J, i, 3*j+1, nsl_fit_model_gaussian_param_deriv(1, x, s, mu, a, sigma));
					gsl_matrix_set(J, i, 3*j+2, nsl_fit_model_gaussian_param_deriv(2, x, s, mu, a, sigma));
				}

				for (int j = 0; j < 3*degree; j++)
					if (fixed[j])
						gsl_matrix_set(J, i, j, 0.);
			}
			break;
		}
		case nsl_fit_model_cauchy_lorentz: {	// Y(x) = 1./pi * ( a1*s1/(s1^2+(x-t1)^2) + a2*s2/(s2^2+(x-t2)^2) + ... + an*sn/(sn^2+(x-tn)^2)
			double s, t, a;
			for (size_t i = 0; i < n; i++) {
				x = xVector[i];
				if (sigmaVector) sigma = sigmaVector[i];

				for (int j = 0; j < degree; j++) {
					s = nsl_fit_map_bound(gsl_vector_get(paramValues, 3*j), min[3*j], max[3*j]);
					t = nsl_fit_map_bound(gsl_vector_get(paramValues, 3*j+1), min[3*j+1], max[3*j+1]);
					a = nsl_fit_map_bound(gsl_vector_get(paramValues, 3*j+2), min[3*j+2], max[3*j+2]);

					gsl_matrix_set(J, i, 3*j, nsl_fit_model_cauchy_lorentz_param_deriv(0, x, s, t, a, sigma));
					gsl_matrix_set(J, i, 3*j+1, nsl_fit_model_cauchy_lorentz_param_deriv(1, x, s, t, a, sigma));
					gsl_matrix_set(J, i, 3*j+2, nsl_fit_model_cauchy_lorentz_param_deriv(2, x, s, t, a, sigma));
				}

				for (int j = 0; j < 3*degree; j++)
					if (fixed[j])
						gsl_matrix_set(J, i, j, 0.);
			}
			break;
		}
		case nsl_fit_model_sech: {
			double s = nsl_fit_map_bound(gsl_vector_get(paramValues, 0), min[0], max[0]);
			double mu = nsl_fit_map_bound(gsl_vector_get(paramValues, 1), min[1], max[1]);
			double a = nsl_fit_map_bound(gsl_vector_get(paramValues, 2), min[2], max[2]);
			for (size_t i = 0; i < n; i++) {
				x = xVector[i];
				if (sigmaVector) sigma = sigmaVector[i];

				for (int j = 0; j < 3; j++) {
					if (fixed[j])
						gsl_matrix_set(J, i, j, 0.);
					else
						gsl_matrix_set(J, i, j, nsl_fit_model_sech_param_deriv(j, x, s, mu, a, sigma));
				}
			}
			break;
		}
		case nsl_fit_model_logistic: {
			double s = nsl_fit_map_bound(gsl_vector_get(paramValues, 0), min[0], max[0]);
			double mu = nsl_fit_map_bound(gsl_vector_get(paramValues, 1), min[1], max[1]);
			double a = nsl_fit_map_bound(gsl_vector_get(paramValues, 2), min[2], max[2]);
			for (size_t i = 0; i < n; i++) {
				x = xVector[i];
				if (sigmaVector) sigma = sigmaVector[i];

				for (int j = 0; j < 3; j++) {
					if (fixed[j])
						gsl_matrix_set(J, i, j, 0.);
					else
						gsl_matrix_set(J, i, j, nsl_fit_model_logistic_param_deriv(j, x, s, mu, a, sigma));
				}
			}
			break;
		}
		}
		break;
	case nsl_fit_model_growth:
		switch (modelType) {
		case nsl_fit_model_atan:
		case nsl_fit_model_tanh: 
		case nsl_fit_model_algebraic_sigmoid:
		case nsl_fit_model_sigmoid: 		// Y(x) = a/(1+exp(-k*(x-mu)))
		case nsl_fit_model_erf:
		case nsl_fit_model_hill: {
			double s = nsl_fit_map_bound(gsl_vector_get(paramValues, 0), min[0], max[0]);
			double mu = nsl_fit_map_bound(gsl_vector_get(paramValues, 1), min[1], max[1]);
			double a = nsl_fit_map_bound(gsl_vector_get(paramValues, 2), min[2], max[2]);
			for (size_t i = 0; i < n; i++) {
				x = xVector[i];
				if (sigmaVector) sigma = sigmaVector[i];

				for (int j = 0; j < 3; j++) {
					if (fixed[j])
						gsl_matrix_set(J, i, j, 0.);
					else {
						if (modelType == nsl_fit_model_atan)
							gsl_matrix_set(J, i, j, nsl_fit_model_atan_param_deriv(j, x, s, mu, a, sigma));
						else if (modelType == nsl_fit_model_tanh)
							gsl_matrix_set(J, i, j, nsl_fit_model_tanh_param_deriv(j, x, s, mu, a, sigma));
						else if (modelType == nsl_fit_model_algebraic_sigmoid)
							gsl_matrix_set(J, i, j, nsl_fit_model_algebraic_sigmoid_param_deriv(j, x, s, mu, a, sigma));
						else if (modelType == nsl_fit_model_sigmoid)
							gsl_matrix_set(J, i, j, nsl_fit_model_sigmoid_param_deriv(j, x, s, mu, a, sigma));
						else if (modelType == nsl_fit_model_erf)
							gsl_matrix_set(J, i, j, nsl_fit_model_erf_param_deriv(j, x, s, mu, a, sigma));
						else if (modelType == nsl_fit_model_hill)
							gsl_matrix_set(J, i, j, nsl_fit_model_hill_param_deriv(j, x, s, mu, a, sigma));
					}
				}
			}
			break;
		}
		case nsl_fit_model_gompertz: {	// Y(x) = a*exp(-b*exp(-c*x));
			double a = nsl_fit_map_bound(gsl_vector_get(paramValues, 0), min[0], max[0]);
			double b = nsl_fit_map_bound(gsl_vector_get(paramValues, 1), min[1], max[1]);
			double c = nsl_fit_map_bound(gsl_vector_get(paramValues, 2), min[2], max[2]);
			for (size_t i = 0; i < n; i++) {
				x = xVector[i];
				if (sigmaVector) sigma = sigmaVector[i];

				for (int j = 0; j < 3; j++) {
					if (fixed[j])
						gsl_matrix_set(J, i, j, 0.);
					else
						gsl_matrix_set(J, i, j, nsl_fit_model_gompertz_param_deriv(j, x, a, b, c, sigma));
				}
			}
			break;
		}
		}
		break;
	case nsl_fit_model_distribution:
		switch (modelType) {
		case nsl_fit_model_maxwell: {	// Y(x) = c*sqrt(2/pi) * x^2/a^3 * exp(-(x/a)^2/2)
			double a = nsl_fit_map_bound(gsl_vector_get(paramValues, 0), min[0], max[0]);
			double c = nsl_fit_map_bound(gsl_vector_get(paramValues, 1), min[1], max[1]);
			for (size_t i = 0; i < n; i++) {
				x = xVector[i];
				if (sigmaVector) sigma = sigmaVector[i];
	
				for (int j = 0; j < 2; j++) {
					if (fixed[j])
						gsl_matrix_set(J, i, j, 0.);
					else
						gsl_matrix_set(J, i, j, nsl_fit_model_maxwell_param_deriv(j, x, a, c, sigma));
				}
			}
			break;
		}
		case nsl_fit_model_lognormal: {	// Y(x) = a/(sqrt(2*pi)*x*s) * exp(-(log(x)-mu)^2/(2*s^2));
			double b = nsl_fit_map_bound(gsl_vector_get(paramValues, 0), min[0], max[0]);
			double mu = nsl_fit_map_bound(gsl_vector_get(paramValues, 1), min[1], max[1]);
			double a = nsl_fit_map_bound(gsl_vector_get(paramValues, 2), min[2], max[2]);
			for (size_t i = 0; i < n; i++) {
				x = xVector[i];
				if (sigmaVector) sigma = sigmaVector[i];
				if (x > 0) {
					for (int j = 0; j < 3; j++) {
						if (fixed[j])
							gsl_matrix_set(J, i, j, 0.);
						else
							gsl_matrix_set(J, i, j, nsl_fit_model_lognormal_param_deriv(j, x, b, mu, a, sigma));
					}
				} else {	// TODO: how to deal correctly with x <= 0
					gsl_matrix_set(J, i, 0, 0);
					gsl_matrix_set(J, i, 1, 0);
					gsl_matrix_set(J, i, 2, 0);
				}
			}
			break;
		}
		case nsl_fit_model_gamma: {
			double b = nsl_fit_map_bound(gsl_vector_get(paramValues, 0), min[0], max[0]);
			double p = nsl_fit_map_bound(gsl_vector_get(paramValues, 1), min[1], max[1]);
			double a = nsl_fit_map_bound(gsl_vector_get(paramValues, 2), min[2], max[2]);
			for (size_t i = 0; i < n; i++) {
				x = xVector[i];
				if (sigmaVector) sigma = sigmaVector[i];
	
				for (int j = 0; j < 3; j++) {
					if (fixed[j])
						gsl_matrix_set(J, i, j, 0.);
					else
						gsl_matrix_set(J, i, j, nsl_fit_model_gamma_param_deriv(j, x, b, p, a, sigma));
				}
			}
			break;
		}
		case nsl_fit_model_laplace: {
			double s = nsl_fit_map_bound(gsl_vector_get(paramValues, 0), min[0], max[0]);
			double mu = nsl_fit_map_bound(gsl_vector_get(paramValues, 1), min[1], max[1]);
			double a = nsl_fit_map_bound(gsl_vector_get(paramValues, 2), min[2], max[2]);
			for (size_t i = 0; i < n; i++) {
				x = xVector[i];
				if (sigmaVector) sigma = sigmaVector[i];
	
				for (int j = 0; j < 3; j++) {
					if (fixed[j])
						gsl_matrix_set(J, i, j, 0.);
					else
						gsl_matrix_set(J, i, j, nsl_fit_model_laplace_param_deriv(j, x, s, mu, a, sigma));
				}
			}
			break;
		}
		case nsl_fit_model_rayleigh: {
			double s = nsl_fit_map_bound(gsl_vector_get(paramValues, 0), min[0], max[0]);
			double a = nsl_fit_map_bound(gsl_vector_get(paramValues, 1), min[1], max[1]);
			for (size_t i = 0; i < n; i++) {
				x = xVector[i];
				if (sigmaVector) sigma = sigmaVector[i];
	
				for (int j = 0; j < 2; j++) {
					if (fixed[j])
						gsl_matrix_set(J, i, j, 0.);
					else
						gsl_matrix_set(J, i, j, nsl_fit_model_rayleigh_param_deriv(j, x, s, a, sigma));
				}
			}
			break;
		}
		case nsl_fit_model_levy: {
			double g = nsl_fit_map_bound(gsl_vector_get(paramValues, 0), min[0], max[0]);
			double mu = nsl_fit_map_bound(gsl_vector_get(paramValues, 1), min[1], max[1]);
			double a = nsl_fit_map_bound(gsl_vector_get(paramValues, 2), min[2], max[2]);
			for (size_t i = 0; i < n; i++) {
				x = xVector[i];
				if (sigmaVector) sigma = sigmaVector[i];
	
				for (int j = 0; j < 3; j++) {
					if (fixed[j])
						gsl_matrix_set(J, i, j, 0.);
					else
						gsl_matrix_set(J, i, j, nsl_fit_model_levy_param_deriv(j, x, g, mu, a, sigma));
				}
			}
			break;
		}
		case nsl_fit_model_chi_square: {
			double n = nsl_fit_map_bound(gsl_vector_get(paramValues, 0), min[0], max[0]);
			double a = nsl_fit_map_bound(gsl_vector_get(paramValues, 1), min[1], max[1]);
			for (size_t i = 0; i < n; i++) {
				x = xVector[i];
				if (sigmaVector) sigma = sigmaVector[i];
	
				for (int j = 0; j < 2; j++) {
					if (fixed[j])
						gsl_matrix_set(J, i, j, 0.);
					else
						gsl_matrix_set(J, i, j, nsl_fit_model_chi_square_param_deriv(j, x, n, a, sigma));
				}
			}
			break;
		}
		case nsl_fit_model_weibull: {	//Y(x) = a * k/l * ((x-mu)/l)^(k-1) * exp(-((x-mu)/l)^k);
			double k = nsl_fit_map_bound(gsl_vector_get(paramValues, 0), min[0], max[0]);
			double l = nsl_fit_map_bound(gsl_vector_get(paramValues, 1), min[1], max[1]);
			double mu = nsl_fit_map_bound(gsl_vector_get(paramValues, 2), min[2], max[2]);
			double a = nsl_fit_map_bound(gsl_vector_get(paramValues, 3), min[3], max[3]);
			for (size_t i = 0; i < n; i++) {
				x = xVector[i];
				if (sigmaVector) sigma = sigmaVector[i];
	
				if (x > 0) {
					for (int j = 0; j < 4; j++) {
						if (fixed[j])
							gsl_matrix_set(J, i, j, 0.);
						else
							gsl_matrix_set(J, i, j, nsl_fit_model_weibull_param_deriv(j, x, k, l, mu, a, sigma));
					}
				} else {	// TODO: how to deal correctly with (x-c)/b <=0
					gsl_matrix_set(J, i, 0, 0);
					gsl_matrix_set(J, i, 1, 0);
					gsl_matrix_set(J, i, 2, 0);
					gsl_matrix_set(J, i, 3, 0);
				}
			}
			break;
		}
		case nsl_fit_model_frechet: {
			double a = nsl_fit_map_bound(gsl_vector_get(paramValues, 0), min[0], max[0]);
			double mu = nsl_fit_map_bound(gsl_vector_get(paramValues, 1), min[1], max[1]);
			double s = nsl_fit_map_bound(gsl_vector_get(paramValues, 2), min[2], max[2]);
			double c = nsl_fit_map_bound(gsl_vector_get(paramValues, 3), min[3], max[3]);
			for (size_t i = 0; i < n; i++) {
				x = xVector[i];
				if (sigmaVector) sigma = sigmaVector[i];
	
				for (int j = 0; j < 4; j++) {
					if (fixed[j])
						gsl_matrix_set(J, i, j, 0.);
					else
						gsl_matrix_set(J, i, j, nsl_fit_model_frechet_param_deriv(j, x, a, mu, s, c, sigma));
				}
			}
			break;
		}
		case nsl_fit_model_gumbel: {	// Y(x) = a/b * exp((x-mu)/b - exp((x-mu)/b));
			double b = nsl_fit_map_bound(gsl_vector_get(paramValues, 0), min[0], max[0]);
			double mu = nsl_fit_map_bound(gsl_vector_get(paramValues, 1), min[1], max[1]);
			double a = nsl_fit_map_bound(gsl_vector_get(paramValues, 2), min[2], max[2]);
			for (size_t i = 0; i < n; i++) {
				x = xVector[i];
				if (sigmaVector) sigma = sigmaVector[i];
	
				for (int j = 0; j < 3; j++) {
					if (fixed[j])
						gsl_matrix_set(J, i, j, 0.);
					else
						gsl_matrix_set(J, i, j, nsl_fit_model_gumbel_param_deriv(j, x, b, mu, a, sigma));
				}
			}
			break;
		}
		}
		break;
	case nsl_fit_model_custom:
		QByteArray funcba = ((struct data*)params)->func->toLocal8Bit();
		char* func = funcba.data();
		QByteArray nameba;
		double value;
		const unsigned int np = paramNames->size();
		for (size_t i = 0; i < n; i++) {
			x = xVector[i];
			assign_variable("x", x);
			if (sigmaVector) sigma = sigmaVector[i];

			for (unsigned int j = 0; j < np; j++) {
				for (unsigned int k = 0; k < np; k++) {
					if (k != j) {
						nameba = paramNames->at(k).toLocal8Bit();
						value = nsl_fit_map_bound(gsl_vector_get(paramValues, k), min[k], max[k]);
						assign_variable(nameba.data(), value);
					}
				}

				nameba = paramNames->at(j).toLocal8Bit();
				char *name = nameba.data();
				value = nsl_fit_map_bound(gsl_vector_get(paramValues, j), min[j], max[j]);
				assign_variable(name, value);
				double f_p = parse(func);

				double eps = 1.e-9*fabs(f_p);	// adapt step size to value
				value += eps;
				assign_variable(name, value);
				double f_pdp = parse(func);

//		qDebug()<<"evaluate deriv"<<QString(func)<<": f(x["<<i<<"]) ="<<QString::number(f_p, 'g', 15);
//		qDebug()<<"evaluate deriv"<<QString(func)<<": f(x["<<i<<"]+dx) ="<<QString::number(f_pdp, 'g', 15);
//		qDebug()<<"	deriv = "<<QString::number((f_pdp-f_p)/eps/sigma, 'g', 15);

				if (fixed[j])
					gsl_matrix_set(J, i, j, 0.);
				else	// calculate finite difference
					gsl_matrix_set(J, i, j, (f_pdp - f_p)/eps/sigma);
			}
		}
	}

	return GSL_SUCCESS;
}

int func_fdf(const gsl_vector* x, void* params, gsl_vector* f, gsl_matrix* J) {
	func_f(x, params, f);
	func_df(x, params, J);

	return GSL_SUCCESS;
}

void XYFitCurvePrivate::recalculate() {
	QElapsedTimer timer;
	timer.start();

	//create fit result columns if not available yet, clear them otherwise
	if (!xColumn) {
		xColumn = new Column("x", AbstractColumn::Numeric);
		yColumn = new Column("y", AbstractColumn::Numeric);
		residualsColumn = new Column("residuals", AbstractColumn::Numeric);
		xVector = static_cast<QVector<double>* >(xColumn->data());
		yVector = static_cast<QVector<double>* >(yColumn->data());
		residualsVector = static_cast<QVector<double>* >(residualsColumn->data());

		xColumn->setHidden(true);
		q->addChild(xColumn);

		yColumn->setHidden(true);
		q->addChild(yColumn);

		q->addChild(residualsColumn);

		q->setUndoAware(false);
		q->setXColumn(xColumn);
		q->setYColumn(yColumn);
		q->setUndoAware(true);
	} else {
		xVector->clear();
		yVector->clear();
		residualsVector->clear();
	}

	// clear the previous result
	fitResult = XYFitCurve::FitResult();

	if (!xDataColumn || !yDataColumn) {
		emit (q->dataChanged());
		sourceDataChangedSinceLastFit = false;
		return;
	}

	//fit settings
	const int maxIters = fitData.maxIterations;	//maximal number of iterations
	const double delta = fitData.eps;		//fit tolerance
	const unsigned int np = fitData.paramNames.size(); //number of fit parameters
	if (np == 0) {
		fitResult.available = true;
		fitResult.valid = false;
		fitResult.status = i18n("Model has no parameters.");
		emit (q->dataChanged());
		sourceDataChangedSinceLastFit = false;
		return;
	}

	//check column sizes
	if (xDataColumn->rowCount() != yDataColumn->rowCount()) {
		fitResult.available = true;
		fitResult.valid = false;
		fitResult.status = i18n("Number of x and y data points must be equal.");
		emit (q->dataChanged());
		sourceDataChangedSinceLastFit = false;
		return;
	}
	if (weightsColumn) {
		if (weightsColumn->rowCount() < xDataColumn->rowCount()) {
			fitResult.available = true;
			fitResult.valid = false;
			fitResult.status = i18n("Not sufficient weight data points provided.");
			emit (q->dataChanged());
			sourceDataChangedSinceLastFit = false;
			return;
		}
	}

	//copy all valid data point for the fit to temporary vectors
	QVector<double> xdataVector;
	QVector<double> ydataVector;
	QVector<double> sigmaVector;
	double xmin = fitData.xRange.first();
	double xmax = fitData.xRange.last();
	for (int row=0; row < xDataColumn->rowCount(); ++row) {
		//only copy those data where _all_ values (for x, y and sigma, if given) are valid
		if (!std::isnan(xDataColumn->valueAt(row)) && !std::isnan(yDataColumn->valueAt(row))
			&& !xDataColumn->isMasked(row) && !yDataColumn->isMasked(row)) {

			// only when inside given range
			if (xDataColumn->valueAt(row) >= xmin && xDataColumn->valueAt(row) <= xmax) {
				if (!weightsColumn) {
					xdataVector.append(xDataColumn->valueAt(row));
					ydataVector.append(yDataColumn->valueAt(row));
				} else {
					if (!std::isnan(weightsColumn->valueAt(row))) {
						xdataVector.append(xDataColumn->valueAt(row));
						ydataVector.append(yDataColumn->valueAt(row));

						if (fitData.weightsType == XYFitCurve::WeightsFromColumn) {
							//weights from a given column -> calculate the square root of the inverse (sigma = sqrt(1/weight))
							sigmaVector.append( sqrt(1./weightsColumn->valueAt(row)) );
						} else if (fitData.weightsType == XYFitCurve::WeightsFromErrorColumn) {
							//weights from a given column with error bars (sigma = error)
							sigmaVector.append( weightsColumn->valueAt(row) );
						}
					}
				}
			}
		}
	}

	//number of data points to fit
	const size_t n = xdataVector.size();
	if (n == 0) {
		fitResult.available = true;
		fitResult.valid = false;
		fitResult.status = i18n("No data points available.");
		emit (q->dataChanged());
		sourceDataChangedSinceLastFit = false;
		return;
	}

	if (n < np) {
		fitResult.available = true;
		fitResult.valid = false;
		fitResult.status = i18n("The number of data points (%1) must be greater than or equal to the number of parameters (%2).", n, np);
		emit (q->dataChanged());
		sourceDataChangedSinceLastFit = false;
		return;
	}

	double* xdata = xdataVector.data();
	double* ydata = ydataVector.data();
	double* sigma = 0;
	if (sigmaVector.size())
		sigma = sigmaVector.data();

	/////////////////////// GSL >= 2 has a complete new interface! But the old one is still supported. ///////////////////////////
	// GSL >= 2 : "the 'fdf' field of gsl_multifit_function_fdf is now deprecated and does not need to be specified for nonlinear least squares problems"
	for (unsigned int i = 0; i < np; i++)
		DEBUG_LOG("fixed parameter"<<i<<fitData.paramFixed.data()[i]);

	//function to fit
	gsl_multifit_function_fdf f;
	struct data params = {n, xdata, ydata, sigma, fitData.modelCategory, fitData.modelType, fitData.degree, &fitData.model, &fitData.paramNames, 
				fitData.paramLowerLimits.data(), fitData.paramUpperLimits.data(), fitData.paramFixed.data()};
	f.f = &func_f;
	f.df = &func_df;
	f.fdf = &func_fdf;
	f.n = n;
	f.p = np;
	f.params = &params;

	// initialize the derivative solver (using Levenberg-Marquardt robust solver)
	const gsl_multifit_fdfsolver_type* T = gsl_multifit_fdfsolver_lmsder;
	gsl_multifit_fdfsolver* s = gsl_multifit_fdfsolver_alloc(T, n, np);

	// set start values
	double* x_init = fitData.paramStartValues.data();
	double* x_min = fitData.paramLowerLimits.data();
	double* x_max = fitData.paramUpperLimits.data();
	// scale start values if limits are set
	for (unsigned int i = 0; i < np; i++)
		x_init[i] = nsl_fit_map_unbound(x_init[i], x_min[i], x_max[i]);
	gsl_vector_view x = gsl_vector_view_array(x_init, np);
	// initialize solver with function f and inital guess x
	gsl_multifit_fdfsolver_set(s, &f, &x.vector);

	//iterate
	int status;
	int iter = 0;
	fitResult.solverOutput.clear();
	writeSolverState(s);
	do {
		iter++;
		status = gsl_multifit_fdfsolver_iterate(s);
		writeSolverState(s);
		if (status) break;
		status = gsl_multifit_test_delta(s->dx, s->x, delta, delta);
	} while (status == GSL_CONTINUE && iter < maxIters);

	// unscale start values again
	for (unsigned int i = 0; i < np; i++)
		x_init[i] = nsl_fit_map_bound(x_init[i], x_min[i], x_max[i]);

	//get the covariance matrix
	//TODO: scale the Jacobian when limits are used before constructing the covar matrix?
	gsl_matrix* covar = gsl_matrix_alloc(np, np);
#if GSL_MAJOR_VERSION >= 2
	// the Jacobian is not part of the solver anymore
	gsl_matrix *J = gsl_matrix_alloc(s->fdf->n, s->fdf->p);
	gsl_multifit_fdfsolver_jac(s, J);
	gsl_multifit_covar(J, 0.0, covar);
#else
	gsl_multifit_covar(s->J, 0.0, covar);
#endif

	//write the result
	fitResult.available = true;
	fitResult.valid = true;
	fitResult.status = QString(gsl_strerror(status)); // i18n? GSL does not support translations
	fitResult.iterations = iter;
	fitResult.dof = n - np;

	//calculate:
	//residuals r_i = y_i - Y_i = - (Y_i - y_i)
	//sse = sum of squared errors (SSE) = residual sum of errors (RSS) = sum of sq. residuals (SSR) = \sum_i^n (Y_i-y_i)^2
	//mse = mean squared error = 1/n \sum_i^n  (Y_i-y_i)^2
	//rmse = root-mean squared error = \sqrt(mse)
	//mae = mean absolute error = \sum_i^n |Y_i-y_i|
	//rms = residual mean square = sse/d.o.f.
	//rsd = residual standard deviation = sqrt(rms)
	//Coefficient of determination, R-squared = 1 - SSE/SSTOT with the total sum of squares SSTOT = \sum_i (y_i - ybar)^2 and ybar = 1/n \sum_i y_i
	//Adjusted Coefficient of determination  adj. R-squared = 1 - (1-R-squared^2)*(n-1)/(n-np-1);
	// see also http://www.originlab.com/doc/Origin-Help/NLFit-Algorithm


	//gsl_blas_dnrm2() - computes the Euclidian norm (||x||_2 = \sqrt {\sum x_i^2}) of the vector with the elements (Yi - y[i])/sigma[i]
	//gsl_blas_dasum() - computes the absolute sum \sum |x_i| of the elements of the vector with the elements (Yi - y[i])/sigma[i]
	fitResult.sse = pow(gsl_blas_dnrm2(s->f), 2);
	fitResult.mse = fitResult.sse/n;
	fitResult.rmse = sqrt(fitResult.mse);
	fitResult.mae = gsl_blas_dasum(s->f);
	if (fitResult.dof != 0) {
		fitResult.rms = fitResult.sse/fitResult.dof;
		fitResult.rsd = sqrt(fitResult.rms);
	}

	//coefficient of determination, R-squared
	double ybar = 0; //mean value of the y-data
	for (size_t i = 0; i < n; ++i)
		ybar += ydata[i];
	ybar = ybar/n;
	double sstot = 0;
	for (size_t i = 0; i < n; ++i)
		sstot += pow(ydata[i]-ybar, 2);
	fitResult.rsquared = 1. - fitResult.sse/sstot;
	fitResult.rsquaredAdj = 1. - (1. - fitResult.rsquared*fitResult.rsquared)*(n-1.)/(n-np-1.);

	//parameter values
	const double c = GSL_MIN_DBL(1., sqrt(fitResult.sse)); //limit error for poor fit
	fitResult.paramValues.resize(np);
	fitResult.errorValues.resize(np);
	for (unsigned int i = 0; i < np; i++) {
		// scale resulting values if they are bounded
		fitResult.paramValues[i] = nsl_fit_map_bound(gsl_vector_get(s->x, i), x_min[i], x_max[i]);
		// use results as start values if desired
		if (fitData.useResults) {
			fitData.paramStartValues.data()[i] = fitResult.paramValues[i];
			DEBUG_LOG("saving parameter"<<i<<fitResult.paramValues[i]<<fitData.paramStartValues.data()[i]);
		}
		fitResult.errorValues[i] = c*sqrt(gsl_matrix_get(covar, i, i));
	}

	// fill residuals vector. To get residuals on the correct x values, fill the rest with zeros.
	residualsVector->resize(xDataColumn->rowCount());
	if (fitData.evaluateFullRange) {	// evaluate full range of residuals
		xVector->resize(xDataColumn->rowCount());
		for (int i = 0; i < xDataColumn->rowCount(); i++)
			(*xVector)[i] = xDataColumn->valueAt(i);
		ExpressionParser* parser = ExpressionParser::getInstance();
		bool rc = parser->evaluateCartesian(fitData.model, xVector, residualsVector,
							fitData.paramNames, fitResult.paramValues);
		for (int i = 0; i < xDataColumn->rowCount(); i++)
			(*residualsVector)[i] = yDataColumn->valueAt(i) - (*residualsVector)[i];
		if (!rc)
			residualsVector->clear();
	} else {	// only selected range
		size_t j = 0;
		for (int i = 0; i < xDataColumn->rowCount(); i++) {
			if (xDataColumn->valueAt(i) >= xmin && xDataColumn->valueAt(i) <= xmax)
				residualsVector->data()[i] = - gsl_vector_get(s->f, j++);
			else	// outside range
				residualsVector->data()[i] = 0;
		}
	}
	residualsColumn->setChanged();

	//free resources
	gsl_multifit_fdfsolver_free(s);
	gsl_matrix_free(covar);

	//calculate the fit function (vectors)
	ExpressionParser* parser = ExpressionParser::getInstance();
	if (fitData.evaluateFullRange) { // evaluate fit on full data range if selected
		xmin = xDataColumn->minimum();
		xmax = xDataColumn->maximum();
	}
	xVector->resize(fitData.evaluatedPoints);
	yVector->resize(fitData.evaluatedPoints);
	bool rc = parser->evaluateCartesian(fitData.model, QString::number(xmin), QString::number(xmax), fitData.evaluatedPoints, xVector, yVector,
						fitData.paramNames, fitResult.paramValues);
	if (!rc) {
		xVector->clear();
		yVector->clear();
	}

	fitResult.elapsedTime = timer.elapsed();

	//redraw the curve
	emit (q->dataChanged());
	sourceDataChangedSinceLastFit = false;
}

/*!
 * writes out the current state of the solver \c s
 */
void XYFitCurvePrivate::writeSolverState(gsl_multifit_fdfsolver* s) {
	QString state;

	//current parameter values, semicolon separated
	double* min = fitData.paramLowerLimits.data();
	double* max = fitData.paramUpperLimits.data();
	for (int i = 0; i < fitData.paramNames.size(); ++i) {
		double x = gsl_vector_get(s->x, i);
		// map parameter if bounded
		state += QString::number(nsl_fit_map_bound(x, min[i], max[i])) + '\t';
	}

	//current value of the chi2-function
	state += QString::number(pow(gsl_blas_dnrm2 (s->f), 2));
	state += ';';

	fitResult.solverOutput += state;
}


//##############################################################################
//##################  Serialization/Deserialization  ###########################
//##############################################################################
//! Save as XML
void XYFitCurve::save(QXmlStreamWriter* writer) const {
	Q_D(const XYFitCurve);

	writer->writeStartElement("xyFitCurve");

	//write xy-curve information
	XYCurve::save(writer);

	//write xy-fit-curve specific information

	//fit data
	writer->writeStartElement("fitData");
	WRITE_COLUMN(d->xDataColumn, xDataColumn);
	WRITE_COLUMN(d->yDataColumn, yDataColumn);
	WRITE_COLUMN(d->weightsColumn, weightsColumn);
	writer->writeAttribute( "autoRange", QString::number(d->fitData.autoRange) );
	writer->writeAttribute( "xRangeMin", QString::number(d->fitData.xRange.first(), 'g', 15) );
	writer->writeAttribute( "xRangeMax", QString::number(d->fitData.xRange.last(), 'g', 15) );
	writer->writeAttribute( "modelCategory", QString::number(d->fitData.modelCategory) );
	writer->writeAttribute( "modelType", QString::number(d->fitData.modelType) );
	writer->writeAttribute( "weightsType", QString::number(d->fitData.weightsType) );
	writer->writeAttribute( "degree", QString::number(d->fitData.degree) );
	writer->writeAttribute( "model", d->fitData.model );
	writer->writeAttribute( "maxIterations", QString::number(d->fitData.maxIterations) );
	writer->writeAttribute( "eps", QString::number(d->fitData.eps, 'g', 15) );
	writer->writeAttribute( "evaluatedPoints", QString::number(d->fitData.evaluatedPoints) );
	writer->writeAttribute( "evaluateFullRange", QString::number(d->fitData.evaluateFullRange) );
	writer->writeAttribute( "useResults", QString::number(d->fitData.useResults) );

	writer->writeStartElement("paramNames");
	foreach (const QString &name, d->fitData.paramNames)
		writer->writeTextElement("name", name);
	writer->writeEndElement();

	writer->writeStartElement("paramStartValues");
	foreach (const double &value, d->fitData.paramStartValues)
		writer->writeTextElement("startValue", QString::number(value, 'g', 15));
	writer->writeEndElement();

	// use 16 digits to handle -DBL_MAX
	writer->writeStartElement("paramLowerLimits");
	foreach (const double &limit, d->fitData.paramLowerLimits)
		writer->writeTextElement("lowerLimit", QString::number(limit, 'g', 16));
	writer->writeEndElement();

	// use 16 digits to handle DBL_MAX
	writer->writeStartElement("paramUpperLimits");
	foreach (const double &limit, d->fitData.paramUpperLimits)
		writer->writeTextElement("upperLimit", QString::number(limit, 'g', 16));
	writer->writeEndElement();

	writer->writeStartElement("paramFixed");
	foreach (const double &fixed, d->fitData.paramFixed)
		writer->writeTextElement("fixed", QString::number(fixed));
	writer->writeEndElement();

	writer->writeEndElement();

	//fit results (generated columns and goodness of the fit)
	//sse = sum of squared errors (SSE) = residual sum of errors (RSS) = sum of sq. residuals (SSR) = \sum_i^n (Y_i-y_i)^2
	//mse = mean squared error = 1/n \sum_i^n  (Y_i-y_i)^2
	//rmse = root-mean squared error = \sqrt(mse)
	//mae = mean absolute error = \sum_i^n |Y_i-y_i|
	//rms = residual mean square = sse/d.o.f.
	//rsd = residual standard deviation = sqrt(rms)
	//R-squared
	//adjusted R-squared
	writer->writeStartElement("fitResult");
	writer->writeAttribute( "available", QString::number(d->fitResult.available) );
	writer->writeAttribute( "valid", QString::number(d->fitResult.valid) );
	writer->writeAttribute( "status", d->fitResult.status );
	writer->writeAttribute( "iterations", QString::number(d->fitResult.iterations) );
	writer->writeAttribute( "time", QString::number(d->fitResult.elapsedTime) );
	writer->writeAttribute( "dof", QString::number(d->fitResult.dof) );
	writer->writeAttribute( "sse", QString::number(d->fitResult.sse, 'g', 15) );
	writer->writeAttribute( "mse", QString::number(d->fitResult.mse, 'g', 15) );
	writer->writeAttribute( "rmse", QString::number(d->fitResult.rmse, 'g', 15) );
	writer->writeAttribute( "mae", QString::number(d->fitResult.mae, 'g', 15) );
	writer->writeAttribute( "rms", QString::number(d->fitResult.rms, 'g', 15) );
	writer->writeAttribute( "rsd", QString::number(d->fitResult.rsd, 'g', 15) );
	writer->writeAttribute( "rsquared", QString::number(d->fitResult.rsquared, 'g', 15) );
	writer->writeAttribute( "rsquaredAdj", QString::number(d->fitResult.rsquaredAdj, 'g', 15) );
	writer->writeAttribute( "solverOutput", d->fitResult.solverOutput );

	writer->writeStartElement("paramValues");
	foreach (const double &value, d->fitResult.paramValues)
		writer->writeTextElement("value", QString::number(value, 'g', 15));
	writer->writeEndElement();

	writer->writeStartElement("errorValues");
	foreach (const double value, d->fitResult.errorValues)
		writer->writeTextElement("error", QString::number(value, 'g', 15));
	writer->writeEndElement();

	//save calculated columns if available
	if (d->xColumn && d->yColumn && d->residualsColumn) {
		d->xColumn->save(writer);
		d->yColumn->save(writer);
		d->residualsColumn->save(writer);
	}

	writer->writeEndElement(); //"fitResult"
	writer->writeEndElement(); //"xyFitCurve"
}

//! Load from XML
bool XYFitCurve::load(XmlStreamReader* reader) {
	Q_D(XYFitCurve);

	if (!reader->isStartElement() || reader->name() != "xyFitCurve") {
		reader->raiseError(i18n("no xy fit curve element found"));
		return false;
	}

	QString attributeWarning = i18n("Attribute '%1' missing or empty, default value is used");
	QXmlStreamAttributes attribs;
	QString str;

	while (!reader->atEnd()) {
		reader->readNext();
		if (reader->isEndElement() && reader->name() == "xyFitCurve")
			break;

		if (!reader->isStartElement())
			continue;

		if (reader->name() == "xyCurve") {
			if ( !XYCurve::load(reader) )
				return false;
		} else if (reader->name() == "fitData") {
			attribs = reader->attributes();

			READ_COLUMN(xDataColumn);
			READ_COLUMN(yDataColumn);
			READ_COLUMN(weightsColumn);

			READ_INT_VALUE("autoRange", fitData.autoRange, bool);
			READ_DOUBLE_VALUE("xRangeMin", fitData.xRange.first());
			READ_DOUBLE_VALUE("xRangeMax", fitData.xRange.last());
			READ_INT_VALUE("modelCategory", fitData.modelCategory, nsl_fit_model_category);
			READ_INT_VALUE("modelType", fitData.modelType, unsigned int);
			READ_INT_VALUE("weightsType", fitData.weightsType, XYFitCurve::WeightsType);
			READ_INT_VALUE("degree", fitData.degree, int);
			READ_STRING_VALUE("model", fitData.model);
			READ_INT_VALUE("maxIterations", fitData.maxIterations, int);
			READ_DOUBLE_VALUE("eps", fitData.eps);
			READ_INT_VALUE("fittedPoints", fitData.evaluatedPoints, size_t);	// old name
			READ_INT_VALUE("evaluatedPoints", fitData.evaluatedPoints, size_t);
			READ_INT_VALUE("evaluateFullRange", fitData.evaluateFullRange, bool);
			READ_INT_VALUE("useResults", fitData.useResults, bool);
		} else if (reader->name() == "name") {
			d->fitData.paramNames<<reader->readElementText();
		} else if (reader->name() == "startValue") {
			d->fitData.paramStartValues<<reader->readElementText().toDouble();
		} else if (reader->name() == "fixed") {
			d->fitData.paramFixed<<(bool)reader->readElementText().toInt();
		} else if (reader->name() == "lowerLimit") {
			bool ok;
			double x = reader->readElementText().toDouble(&ok);
			if (ok)	// -DBL_MAX results in conversion error
				d->fitData.paramLowerLimits<<x;
			else
				d->fitData.paramLowerLimits<<-DBL_MAX;
		} else if (reader->name() == "upperLimit") {
			bool ok;
			double x = reader->readElementText().toDouble(&ok);
			if (ok)	// DBL_MAX results in conversion error
				d->fitData.paramUpperLimits<<x;
			else
				d->fitData.paramUpperLimits<<DBL_MAX;
		} else if (reader->name() == "value") {
			d->fitResult.paramValues<<reader->readElementText().toDouble();
		} else if (reader->name() == "error") {
			d->fitResult.errorValues<<reader->readElementText().toDouble();
		} else if (reader->name() == "fitResult") {
			attribs = reader->attributes();

			READ_INT_VALUE("available", fitResult.available, int);
			READ_INT_VALUE("valid", fitResult.valid, int);
			READ_STRING_VALUE("status", fitResult.status);
			READ_INT_VALUE("iterations", fitResult.iterations, int);
			READ_INT_VALUE("time", fitResult.elapsedTime, int);
			READ_DOUBLE_VALUE("dof", fitResult.dof);
			READ_DOUBLE_VALUE("sse", fitResult.sse);
			READ_DOUBLE_VALUE("mse", fitResult.mse);
			READ_DOUBLE_VALUE("rmse", fitResult.rmse);
			READ_DOUBLE_VALUE("mae", fitResult.mae);
			READ_DOUBLE_VALUE("rms", fitResult.rms);
			READ_DOUBLE_VALUE("rsd", fitResult.rsd);
			READ_DOUBLE_VALUE("rsquared", fitResult.rsquared);
			READ_DOUBLE_VALUE("rsquaredAdj", fitResult.rsquaredAdj);
			READ_STRING_VALUE("solverOutput", fitResult.solverOutput);
		} else if (reader->name() == "column") {
			Column* column = new Column("", AbstractColumn::Numeric);
			if (!column->load(reader)) {
				delete column;
				return false;
			}
			if (column->name() == "x")
				d->xColumn = column;
			else if (column->name() == "y")
				d->yColumn = column;
			else if (column->name() == "residuals")
				d->residualsColumn = column;
		}
	}

	// wait for data to be read before using the pointers
	QThreadPool::globalInstance()->waitForDone();

	// new fit model style
	if (d->fitData.modelCategory == nsl_fit_model_basic && d->fitData.modelType >= NSL_FIT_MODEL_BASIC_COUNT)
		d->fitData.modelType = 0;

	// fixed and limits are not saved in project (old project)
	if (d->fitData.paramFixed.size() == 0) {
		for (int i = 0; i < d->fitData.paramStartValues.size(); i++)
			d->fitData.paramFixed<<false;
	}
	if (d->fitData.paramLowerLimits.size() == 0) {
		for (int i = 0; i < d->fitData.paramStartValues.size(); i++)
			d->fitData.paramLowerLimits<<-DBL_MAX;
	}
	if (d->fitData.paramUpperLimits.size() == 0) {
		for (int i = 0; i < d->fitData.paramStartValues.size(); i++)
			d->fitData.paramUpperLimits<<DBL_MAX;
	}

	if (d->xColumn && d->yColumn && d->residualsColumn) {
		d->xColumn->setHidden(true);
		addChild(d->xColumn);

		d->yColumn->setHidden(true);
		addChild(d->yColumn);

		addChild(d->residualsColumn);

		d->xVector = static_cast<QVector<double>* >(d->xColumn->data());
		d->yVector = static_cast<QVector<double>* >(d->yColumn->data());
		d->residualsVector = static_cast<QVector<double>* >(d->residualsColumn->data());

		setUndoAware(false);
		XYCurve::d_ptr->xColumn = d->xColumn;
		XYCurve::d_ptr->yColumn = d->yColumn;
		setUndoAware(true);
	}

	return true;
}
