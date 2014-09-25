/***************************************************************************
    File                 : XYFitCurve.cpp
    Project              : LabPlot
    Description          : A xy-curve defined by a fit model
    --------------------------------------------------------------------
    Copyright            : (C) 2014 Alexander Semke (alexander.semke*web.de)

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
#include "backend/gsl/ExpressionParser.h"
#include "backend/gsl/parser_extern.h"
#include "backend/gsl/parser_struct.h"

#include <gsl/gsl_blas.h>
#include <gsl/gsl_vector.h>
#include <gsl/gsl_matrix.h>

#include <KIcon>
#include <KLocale>
#include <QElapsedTimer>
#include <QDebug>

XYFitCurve::XYFitCurve(const QString& name)
		: XYCurve(name, new XYFitCurvePrivate(this)){
	init();
}

XYFitCurve::XYFitCurve(const QString& name, XYFitCurvePrivate* dd)
		: XYCurve(name, dd){
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
	d->swapSymbolsTypeId("none");
}

void XYFitCurve::recalculate() {
	Q_D(XYFitCurve);
	d->recalculate();
}

/*!
	Returns an icon to be used in the project explorer.
*/
QIcon XYFitCurve::icon() const {
	return KIcon("xy-fit-curve");
}

//##############################################################################
//##########################  getter methods  ##################################
//##############################################################################
BASIC_SHARED_D_READER_IMPL(XYFitCurve, const AbstractColumn*, xDataColumn, xDataColumn)
BASIC_SHARED_D_READER_IMPL(XYFitCurve, const AbstractColumn*, yDataColumn, yDataColumn)
BASIC_SHARED_D_READER_IMPL(XYFitCurve, const AbstractColumn*, weightsColumn, weightsColumn)
const QString& XYFitCurve::xDataColumnPath() const { Q_D(const XYFitCurve); return d->xDataColumnPath; }
const QString& XYFitCurve::yDataColumnPath() const {	Q_D(const XYFitCurve); return d->yDataColumnPath; }
const QString& XYFitCurve::weightsColumnPath() const { Q_D(const XYFitCurve);return d->weightsColumnPath; }

BASIC_SHARED_D_READER_IMPL(XYFitCurve, XYFitCurve::FitData, fitData, fitData)

const XYFitCurve::FitResult& XYFitCurve::fitResult() const {
	Q_D(const XYFitCurve);
	return d->fitResult;
}
//##############################################################################
//#################  setter methods and undo commands ##########################
//##############################################################################
STD_SETTER_CMD_IMPL_S(XYFitCurve, SetXDataColumn, const AbstractColumn*, xDataColumn)
void XYFitCurve::setXDataColumn(const AbstractColumn* column) {
	Q_D(XYFitCurve);
	if (column != d->xDataColumn)
		exec(new XYFitCurveSetXDataColumnCmd(d, column, i18n("%1: assign x-data")));
}

STD_SETTER_CMD_IMPL_S(XYFitCurve, SetYDataColumn, const AbstractColumn*, yDataColumn)
void XYFitCurve::setYDataColumn(const AbstractColumn* column) {
	Q_D(XYFitCurve);
	if (column != d->yDataColumn)
		exec(new XYFitCurveSetYDataColumnCmd(d, column, i18n("%1: assign y-data")));
}

STD_SETTER_CMD_IMPL_S(XYFitCurve, SetWeightsColumn, const AbstractColumn*, weightsColumn)
void XYFitCurve::setWeightsColumn(const AbstractColumn* column) {
	Q_D(XYFitCurve);
	if (column != d->weightsColumn)
		exec(new XYFitCurveSetWeightsColumnCmd(d, column, i18n("%1: assign weights")));
}

STD_SETTER_CMD_IMPL_F_S(XYFitCurve, SetFitData, XYFitCurve::FitData, fitData, recalculate);
void XYFitCurve::setFitData(const XYFitCurve::FitData& fitData) {
	Q_D(XYFitCurve);
	exec(new XYFitCurveSetFitDataCmd(d, fitData, i18n("%1: set fit options and perform the fit")));
}

//##############################################################################
//######################### Private implementation #############################
//##############################################################################
XYFitCurvePrivate::XYFitCurvePrivate(XYFitCurve* owner) : XYCurvePrivate(owner),
	xDataColumn(0), yDataColumn(0), weightsColumn(0),
	xColumn(0), yColumn(0), residualsColumn(0),
	xVector(0), yVector(0), residualsVector(0),
	q(owner)  {

}

XYFitCurvePrivate::~XYFitCurvePrivate() {
	//no need to delete xColumn and yColumn, they are deleted
	//when the parent aspect is removed
}

struct data {
	size_t n; //number of data points
	double* x; //pointer to the vector with x-data values
	double* y; //pointer to the vector with y-data vector
	double* sigma; //pointer to the vector with sigma values
	XYFitCurve::ModelType modelType;
	int degree;
	QString* func; // string containing the definition of the model/function
	QStringList* paramNames;
};

/*!
 * \param v vector containing current values of the fit parameters
 * \param params
 * \param f vector with the weighted residuals (Yi - y[i])/sigma[i]
 */
int func_f(const gsl_vector* paramValues, void* params, gsl_vector* f) {
	int n = ((struct data*)params)->n;
	double* x = ((struct data*)params)->x;
	double* y = ((struct data*)params)->y;
	double* sigma = ((struct data*)params)->sigma;
	QByteArray funcba = ((struct data*)params)->func->toLocal8Bit();
	const char *func = funcba.data();
	QStringList* paramNames = ((struct data*)params)->paramNames;

	//set current values of the parameters
	QByteArray paramba;
	for (int j=0; j<paramNames->size(); j++) {
		paramba = paramNames->at(j).toLocal8Bit();
		assign_variable(paramba.data(), gsl_vector_get(paramValues,j));
	}

	char var[]="x";
	for (int i = 0; i < n; i++) {
		if (isnan(x[i]) || isnan(y[i]))
			continue;

		double Yi=0;
		//TODO: add checks for allowed valus of x for different models if required (x>0 for ln(x) etc.)

		assign_variable(var, x[i]);
		Yi = parse(func);
// Debugging
/*		printf("func=%s, X[%d]=%g\n	",func,i,x[i]);
		for (int j=0; j<paramNames->size(); j++)
			printf("%g ",gsl_vector_get(paramValues,j));
		printf("\n	Y[%d]=%g \n",i,Yi);
*/
		if(parse_errors()>0) {
			qDebug()<<"func_f: parse errors in parsing "<<func;
			return GSL_EINVAL;
		}

// 		Yi += base; //TODO
		if (sigma) {
				gsl_vector_set (f, i, (Yi - y[i])/sigma[i]);
		} else {
				gsl_vector_set (f, i, (Yi - y[i]));
		}
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
	int n = ((struct data*)params)->n;
	double* xVector = ((struct data*)params)->x;
	double* sigmaVector = ((struct data*)params)->sigma;
	QStringList* paramNames = ((struct data*)params)->paramNames;
	XYFitCurve::ModelType modelType = ((struct data*)params)->modelType;
	int degree = ((struct data*)params)->degree;

	// calculate the Jacobian matrix:
	// Jacobian matrix J(i,j) = dfi / dxj
	// where fi = (Yi - yi)/sigma[i],
	// Yi = model and the xj are the parameters

	double x;
	double sigma = 1.0;

	switch(modelType) {
		case XYFitCurve::Polynomial: {
			// Y(x) = c0 + c1*x + ... + cn*x^n
			for (int i=0; i<n; i++) {
				x = xVector[i];
				if (sigmaVector) sigma = sigmaVector[i];
				for (int j=0; j<paramNames->size(); ++j) {
					gsl_matrix_set(J, i, j, pow(x,j)/sigma);
				}
			}
			break;
		}
		case XYFitCurve::Power: {
			// Y(x) = a*x^b or Y(x) = a + b*x^c.
			if (degree==1) {
				double a = gsl_vector_get(paramValues,0);
				double b = gsl_vector_get(paramValues,1);
				for (int i=0; i<n; i++) {
					x = xVector[i];
					if (sigmaVector) sigma = sigmaVector[i];
					gsl_matrix_set(J, i, 0, pow(x,b)/sigma);
					gsl_matrix_set(J, i, 1, a*pow(x,b)*log(x)/sigma);
				}
			} else if (degree==2) {
				double b = gsl_vector_get(paramValues,1);
				double c = gsl_vector_get(paramValues,2);
				for (int i=0; i<n; i++) {
					x = xVector[i];
					if (sigmaVector) sigma = sigmaVector[i];
					gsl_matrix_set(J, i, 0, 1/sigma);
					gsl_matrix_set(J, i, 1, pow(x,c)/sigma);
					gsl_matrix_set(J, i, 2, b*pow(x,c)*log(x)/sigma);
				}
			}
			break;
		}
		case XYFitCurve::Exponential: {
			// Y(x) = a*exp(b*x) or Y(x) = a*exp(b*x) + c*exp(d*x) or Y(x) = a*exp(b*x) + c*exp(d*x) + e*exp(f*x)
			if (degree==1) {
				double a = gsl_vector_get(paramValues,0);
				double b = gsl_vector_get(paramValues,1);
				for (int i=0; i<n; i++) {
					x = xVector[i];
					if (sigmaVector) sigma = sigmaVector[i];
					gsl_matrix_set(J, i, 0, exp(b*x)/sigma);
					gsl_matrix_set(J, i, 1, a*x*exp(b*x)/sigma);
				}
			} else if (degree==2) {
				double a = gsl_vector_get(paramValues,0);
				double b = gsl_vector_get(paramValues,1);
				double c = gsl_vector_get(paramValues,2);
				double d = gsl_vector_get(paramValues,3);
				for (int i=0; i<n; i++) {
					x = xVector[i];
					if (sigmaVector) sigma = sigmaVector[i];
					gsl_matrix_set(J, i, 0, exp(b*x)/sigma);
					gsl_matrix_set(J, i, 1, a*x*exp(b*x)/sigma);
					gsl_matrix_set(J, i, 2, exp(d*x)/sigma);
					gsl_matrix_set(J, i, 3, c*x*exp(d*x)/sigma);
				}
			} else if (degree==3) {
				double a = gsl_vector_get(paramValues,0);
				double b = gsl_vector_get(paramValues,1);
				double c = gsl_vector_get(paramValues,2);
				double d = gsl_vector_get(paramValues,3);
				double e = gsl_vector_get(paramValues,4);
				double f = gsl_vector_get(paramValues,5);
				for (int i=0; i<n; i++) {
					x = xVector[i];
					if (sigmaVector) sigma = sigmaVector[i];
					gsl_matrix_set(J, i, 0, exp(b*x)/sigma);
					gsl_matrix_set(J, i, 1, a*x*exp(b*x)/sigma);
					gsl_matrix_set(J, i, 2, exp(d*x)/sigma);
					gsl_matrix_set(J, i, 3, c*x*exp(b*x)/sigma);
					gsl_matrix_set(J, i, 4, exp(f*x)/sigma);
					gsl_matrix_set(J, i, 5, e*x*exp(f*x)/sigma);
				}
			}
			break;
		}
		case XYFitCurve::Inverse_Exponential: {
			// Y(x) = a*(1-exp(b*x))+c
			double a = gsl_vector_get(paramValues,0);
			double b = gsl_vector_get(paramValues,1);
			//double c = gsl_vector_get(paramValues,2);
			for (int i=0; i<n; i++) {
				x = xVector[i];
				if (sigmaVector) sigma = sigmaVector[i];
				gsl_matrix_set(J, i, 0, (1.0-exp(b*x))/sigma);
				gsl_matrix_set(J, i, 1, -a*x*exp(b*x)/sigma);
				gsl_matrix_set(J, i, 2, 1.0/sigma);
			}
		}
		case XYFitCurve::Fourier: {
			// Y(x) = a0 + (a1*cos(w*x) + b1*sin(w*x)) + ... + (an*cos(n*w*x) + bn*sin(n*w*x)
			//parameters: w, a0, a1, b1, ... an, bn
			double a[degree];
			double b[degree];
			double w = gsl_vector_get(paramValues,0);
			a[0] = gsl_vector_get(paramValues,1);
			b[0] = 0;
			for (int j=1; j<degree; ++j) {
				a[j] = gsl_vector_get(paramValues,2*j);
				b[j] = gsl_vector_get(paramValues,2*j+1);
			}
			for (int i=0; i<n; i++) {
				x = xVector[i];
				if (sigmaVector) sigma = sigmaVector[i];
				double wd = 0; //first derivative with respect to the w parameter
				for (int j=1; j<degree; ++j) {
					wd += -a[j]*j*x*sin(j*w*x) + b[j]*j*x*cos(j*w*x);
				}
				gsl_matrix_set(J, i, 0, wd/sigma);
				for (int j=0; j<degree; ++j) {
					gsl_matrix_set(J, i, j+1, cos(j*w*x)/sigma);
					gsl_matrix_set(J, i, j+2, sin(j*w*x)/sigma);
				}
			}
			break;
		}
		case XYFitCurve::Gaussian: {
			// Y(x) = a1*exp(-((x-b1)/c1)^2) + a2*exp(-((x-b2)/c2)^2) + ... + an*exp(-((x-bn)/cn)^2)
			double a,b,c;
			for (int i=0; i<n; i++) {
				x = xVector[i];
				if (sigmaVector) sigma = sigmaVector[i];
				for (int j=0; j<degree; ++j) {
					a = gsl_vector_get(paramValues,3*j);
					b = gsl_vector_get(paramValues,3*j+1);
					c = gsl_vector_get(paramValues,3*j+2);
					gsl_matrix_set(J, i, 3*j, exp(-(x-b)*(x-b)/(c*c))/sigma);
					gsl_matrix_set(J, i, 3*j+1, 2*a*(x-b)/c*c*exp(-(x-b)*(x-b)/(c*c))/sigma);
					gsl_matrix_set(J, i, 3*j+2, 2*a*(x-b)*(x-b)/c*c*c*exp(-(x-b)*(x-b)/(c*c))/sigma);
				}
			}
			break;
		}
		case XYFitCurve::Lorentz: {
			// Y(x) = 1/pi*s/(s^2+(x-t)^2)
			double s = gsl_vector_get(paramValues,0);
			double t = gsl_vector_get(paramValues,1);
			for (int i=0; i<n; i++) {
				x = xVector[i];
				if (sigmaVector) sigma = sigmaVector[i];
				gsl_matrix_set(J, i, 0, (-s*s+(x-t)*(x-t))/pow(s*s+(x-t)*(x-t),2)/M_PI/sigma);
				gsl_matrix_set(J, i, 1, 2*s*(x-t)/pow(s*s+(x-t)*(x-t),2)/M_PI/sigma);
			}
			break;
		}
		case XYFitCurve::Maxwell: {
			// Y(x) = sqrt(2/pi)*exp(-x^2/(2*a^2))/a^3
			double a = gsl_vector_get(paramValues,0);
			for (int i=0; i<n; i++) {
				x = xVector[i];
				if (sigmaVector) sigma = sigmaVector[i];
				gsl_matrix_set(J, i, 0, sqrt(2/M_PI)*x*x*(x*x-3*a*a)*exp(-x*x/2/a/a)/pow(a,6)/sigma);
			}
			break;
		}
		case XYFitCurve::Custom: {
			QByteArray funcba = ((struct data*)params)->func->toLocal8Bit();
			char* func = funcba.data();
			double eps = 1.0e-5;
			QByteArray nameba;
			char* name;
			double value;
			for (int i=0; i<n; i++) {
				x = xVector[i];
				if (sigmaVector) sigma = sigmaVector[i];
				char var[]="x";
				assign_variable(var, x);

				for(int j=0; j<paramNames->size(); j++) {
					for(int k=0; k<paramNames->size();k++) {
						if(k!=j) {
							nameba = paramNames->at(k).toLocal8Bit();
							value = gsl_vector_get(paramValues,k);
							assign_variable(nameba.data(), value);
						}
					}

					nameba = paramNames->at(j).toLocal8Bit();
					name = nameba.data();
					value = gsl_vector_get(paramValues,j);
					assign_variable(name, value);
					double f_p = parse(func);

					value = value + eps;
					assign_variable(name, value);
					double f_pdp = parse(func);

					gsl_matrix_set(J, i, j, (f_pdp-f_p)/eps/sigma);
				}
			}
			break;
		}
	}

	return GSL_SUCCESS;
}

int func_fdf(const gsl_vector* x, void* params, gsl_vector* f,gsl_matrix* J) {
	func_f (x, params, f);
	func_df (x, params, J);
	return GSL_SUCCESS;
}

void XYFitCurvePrivate::recalculate() {
	qDebug()<<"XYFitCurvePrivate::recalculate()";
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
		return;
	}

	//data to fit
	double* xdata = static_cast<QVector<double>* >(dynamic_cast<Column*>( const_cast<AbstractColumn*>(xDataColumn) )->data())->data();
	double* ydata = static_cast<QVector<double>* >(dynamic_cast<Column*>( const_cast<AbstractColumn*>(yDataColumn) )->data())->data();

	//fit settings
	int maxIters = fitData.maxIterations; //maximal number of iterations
	float delta = fitData.eps; //fit tolerance
	const int np = fitData.paramNames.size(); //number of fit parameters
	if (np == 0) {
		fitResult.available = true;
		fitResult.valid = false;
		fitResult.status = i18n("Model has no parameters.");
		emit (q->dataChanged());
		return;
	}

	//determine the number of data points in the column, stop iterating after the first nan was encountered
	int n = xDataColumn->rowCount();
	for (int i=0; i<xDataColumn->rowCount(); ++i) {
		if (isnan(xdata[i]) || isnan(ydata[i])) {
			n = i;
			break;
		}
	}

	if (n == 0) {
		fitResult.available = true;
		fitResult.valid = false;
		fitResult.status = i18n("No data points available.");
		emit (q->dataChanged());
		return;
	}

	if (n<np) {
		fitResult.available = true;
		fitResult.valid = false;
		fitResult.status = i18n("The number of data points (%1) must be greater than or equal to the number of parameters (%2).").arg(n).arg(np);
		emit (q->dataChanged());
		return;
	}

	//calculate sigma for the given weights type
	double* sigma = 0;
	if (weightsColumn) {
		if (weightsColumn->rowCount()<n) {
			fitResult.available = true;
			fitResult.valid = false;
			fitResult.status = i18n("Not sufficient weight data points provided.");
			emit (q->dataChanged());
			return;
		}

		if (fitData.weightsType == XYFitCurve::WeightsFromColumn) {
			//weights from a given column -> calculate the square root of the inverse (sigma = sqrt(1/weight))
			sigma = new double[n];
			for (int i=0; i<n; ++i) {
				sigma[i] = sqrt(1/weightsColumn->valueAt(i));
			}
		} else if (fitData.weightsType == XYFitCurve::WeightsFromErrorColumn) {
			//weights from a given column with error bars (sigma = error)
			sigma = static_cast<QVector<double>* >(dynamic_cast<Column*>( const_cast<AbstractColumn*>(weightsColumn) )->data())->data();
		}
	}


	//function to fit
	gsl_multifit_function_fdf f;
	struct data params = {n, xdata, ydata, sigma, fitData.modelType, fitData.degree, &fitData.model, &fitData.paramNames};
	f.f = &func_f;
	f.df = &func_df;
	f.fdf = &func_fdf;
	f.n = n;
	f.p = np;
	f.params = &params;

	//initialize the solver
	const gsl_multifit_fdfsolver_type* T = gsl_multifit_fdfsolver_lmsder;
	gsl_multifit_fdfsolver* s = gsl_multifit_fdfsolver_alloc (T, n, np);
	double* x_init = fitData.paramStartValues.data();
	gsl_vector_view x = gsl_vector_view_array (x_init, np);
	gsl_multifit_fdfsolver_set (s, &f, &x.vector);

	//iterate
	int status;
	int iter = 0;
	fitResult.solverOutput.clear();
	writeSolverState(s);
	do {
		iter++;
		status = gsl_multifit_fdfsolver_iterate (s);
		writeSolverState(s);
		if (status) break;
		status = gsl_multifit_test_delta (s->dx, s->x, delta, delta);
	} while (status == GSL_CONTINUE && iter < maxIters);

	//get the covariance matrix
	gsl_matrix* covar = gsl_matrix_alloc (np, np);
	gsl_multifit_covar (s->J, 0.0, covar);


	//write the result
	fitResult.available = true;
	fitResult.valid = true;
	fitResult.status = QString(gsl_strerror(status)); //TODO: add i18n
	fitResult.iterations = iter;
	fitResult.dof = n-np;

	//calculate:
	//residuals (Y_i-y_i)
	//sse = sum of squared errors (SSE) = residual sum of errors (RSS) = sum of sq. residuals (SSR) = \sum_i^n (Y_i-y_i)^2
	//mse = mean squared error = 1/n \sum_i^n  (Y_i-y_i)^2
	//rmse = root-mean squared error = \sqrt(mse)
	//mae = mean absolute error = \sum_i^n |Y_i-y_i|
	//rms = residual mean square = sse/d.o.f.
	//rsd = residual standard deviation = sqrt(rms)
	//Coefficient of determination, R-squared = 1 - SSE/SSTOT with the total sum of squares SSTOT = \sum_i (y_i - ybar)^2 and ybar = 1/n \sum_i y_i
	//Adjusted Coefficient of determination  adj. R-squared = 1 - (1-R-squared^2)*n-1)/(n-np-1);

	residualsVector->resize(n);
	for (int i=0; i<n; ++i) {
		residualsVector->data()[i] = gsl_vector_get(s->f, i);
	}
	residualsColumn->setChanged();

	//gsl_blas_dnrm2() - computes the Euclidian norm (||x||_2 = \sqrt {\sum x_i^2}) of the vector with the elements (Yi - y[i])/sigma[i]
	//gsl_blas_dasum() - computes the absolute sum \sum |x_i| of the elements of the vector with the elements (Yi - y[i])/sigma[i]
	fitResult.sse = pow(gsl_blas_dnrm2(s->f), 2);
	fitResult.mse = fitResult.sse/n;
	fitResult.rmse = sqrt(fitResult.mse);
	fitResult.mae = gsl_blas_dasum(s->f);
	if (fitResult.dof!=0) {
		fitResult.rms = fitResult.sse/fitResult.dof;
		fitResult.rsd = sqrt(fitResult.rms);
	}

	//Coefficient of determination, R-squared
	double ybar = 0; //mean value of the y-data
	for (int i=0; i<n; ++i)
		ybar += ydata[i];
	ybar = ybar/n;
	double sstot = 0;
	for (int i=0; i<n; ++i)
		sstot += pow(ydata[i]-ybar, 2);
	fitResult.rsquared = 1 - fitResult.sse/sstot;
	fitResult.rsquaredAdj = 1-(1-fitResult.rsquared*fitResult.rsquared)*(n-1)/(n-np-1);

	//parameter values
	double c = GSL_MIN_DBL(1, sqrt(fitResult.sse)); //limit error for poor fit
	fitResult.paramValues.resize(np);
	fitResult.errorValues.resize(np);
	for (int i=0; i<np; i++) {
		fitResult.paramValues[i] = gsl_vector_get(s->x, i);
		fitResult.errorValues[i] = c*sqrt(gsl_matrix_get(covar,i,i));
	}

	//free resources
	gsl_multifit_fdfsolver_free(s);
	gsl_matrix_free(covar);
	if (sigma && fitData.weightsType == XYFitCurve::WeightsFromColumn)
		delete [] sigma;

	//calculate the fit function (vectors)
	ExpressionParser* parser = ExpressionParser::getInstance();
	double min = xDataColumn->minimum();
	double max = xDataColumn->maximum();
	xVector->resize(fitData.fittedPoints);
	yVector->resize(fitData.fittedPoints);
	bool rc = parser->evaluateCartesian(fitData.model, QString::number(min), QString::number(max), fitData.fittedPoints, xVector, yVector, fitData.paramNames, fitResult.paramValues);
	if (!rc) {
		xVector->clear();
		yVector->clear();
	}

	fitResult.elapsedTime = timer.elapsed();

	//redraw the curve
	emit (q->dataChanged());
}

/*!
 * writes out the current state of the solver \c s
 */
void XYFitCurvePrivate::writeSolverState(gsl_multifit_fdfsolver* s) {
	QString state;

	//current parameter values, semicolon separated
	for (int i=0; i<fitData.paramNames.size(); ++i)
		state += QString::number(gsl_vector_get(s->x, i)) + '\t';

	//current value of the chi2-function
	state += QString::number(pow(gsl_blas_dnrm2 (s->f),2));
	state += ';';

	fitResult.solverOutput += state;
}


//##############################################################################
//##################  Serialization/Deserialization  ###########################
//##############################################################################
//! Save as XML
void XYFitCurve::save(QXmlStreamWriter* writer) const{
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
	writer->writeAttribute( "modelType", QString::number(d->fitData.modelType) );
	writer->writeAttribute( "weightsType", QString::number(d->fitData.weightsType) );
	writer->writeAttribute( "degree", QString::number(d->fitData.degree) );
	writer->writeAttribute( "model", d->fitData.model );
	writer->writeAttribute( "maxIterations", QString::number(d->fitData.maxIterations) );
	writer->writeAttribute( "eps", QString::number(d->fitData.eps) );
	writer->writeAttribute( "fittedPoints", QString::number(d->fitData.fittedPoints) );

	writer->writeStartElement("paramNames");
	for (int i=0; i<d->fitData.paramNames.size(); ++i)
		writer->writeTextElement("name", d->fitData.paramNames.at(i));
	writer->writeEndElement();

	writer->writeStartElement("paramStartValues");
	for (int i=0; i<d->fitData.paramStartValues.size(); ++i)
		writer->writeTextElement("startValue", QString::number(d->fitData.paramStartValues.at(i)));
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
	writer->writeAttribute( "sse", QString::number(d->fitResult.sse) );
	writer->writeAttribute( "mse", QString::number(d->fitResult.mse) );
	writer->writeAttribute( "rmse", QString::number(d->fitResult.rmse) );
	writer->writeAttribute( "mae", QString::number(d->fitResult.mae) );
	writer->writeAttribute( "rms", QString::number(d->fitResult.rms) );
	writer->writeAttribute( "rsd", QString::number(d->fitResult.rsd) );
	writer->writeAttribute( "rsquared", QString::number(d->fitResult.rsquared) );
	writer->writeAttribute( "rsquaredAdj", QString::number(d->fitResult.rsquaredAdj) );
	writer->writeAttribute( "solverOutput", d->fitResult.solverOutput );

	writer->writeStartElement("paramValues");
	for (int i=0; i<d->fitResult.paramValues.size(); ++i)
		writer->writeTextElement("value", QString::number(d->fitResult.paramValues.at(i)));
	writer->writeEndElement();

	writer->writeStartElement("errorValues");
	for (int i=0; i<d->fitResult.errorValues.size(); ++i)
		writer->writeTextElement("error", QString::number(d->fitResult.errorValues.at(i)));
	writer->writeEndElement();

	//save calculated columns if available
	if (d->xColumn) {
		d->xColumn->save(writer);
		d->yColumn->save(writer);
		d->residualsColumn->save(writer);
	}

	writer->writeEndElement(); //"fitResult"
	writer->writeEndElement(); //"xyFitCurve"
}

//! Load from XML
bool XYFitCurve::load(XmlStreamReader* reader){
	Q_D(XYFitCurve);

    if(!reader->isStartElement() || reader->name() != "xyFitCurve"){
        reader->raiseError(i18n("no xy fit curve element found"));
        return false;
    }

    QString attributeWarning = i18n("Attribute '%1' missing or empty, default value is used");
    QXmlStreamAttributes attribs;
    QString str;

    while (!reader->atEnd()){
        reader->readNext();
        if (reader->isEndElement() && reader->name() == "xyFitCurve")
            break;

        if (!reader->isStartElement())
            continue;

		if (reader->name() == "xyCurve") {
            if ( !XYCurve::load(reader) )
				return false;
		}else if (reader->name() == "fitData"){
			attribs = reader->attributes();

			READ_COLUMN(xDataColumn);
			READ_COLUMN(yDataColumn);
			READ_COLUMN(weightsColumn);

			str = attribs.value("modelType").toString();
            if(str.isEmpty())
                reader->raiseWarning(attributeWarning.arg("'modelType'"));
            else
                d->fitData.modelType = (XYFitCurve::ModelType)str.toInt();

			str = attribs.value("weightsType").toString();
            if(str.isEmpty())
                reader->raiseWarning(attributeWarning.arg("'weightsType'"));
            else
                d->fitData.weightsType = (XYFitCurve::WeightsType)str.toInt();

			str = attribs.value("degree").toString();
            if(str.isEmpty())
                reader->raiseWarning(attributeWarning.arg("'degree'"));
            else
                d->fitData.degree = str.toInt();

			str = attribs.value("model").toString();
            if(str.isEmpty())
                reader->raiseWarning(attributeWarning.arg("'model'"));
            else
                d->fitData.model = str;

			str = attribs.value("maxIterations").toString();
            if(str.isEmpty())
                reader->raiseWarning(attributeWarning.arg("'maxIterations'"));
            else
                d->fitData.maxIterations = str.toInt();

			str = attribs.value("eps").toString();
            if(str.isEmpty())
                reader->raiseWarning(attributeWarning.arg("'eps'"));
            else
                d->fitData.eps = str.toDouble();

			str = attribs.value("fittedPoints").toString();
            if(str.isEmpty())
                reader->raiseWarning(attributeWarning.arg("'fittedPoints'"));
            else
                d->fitData.fittedPoints = str.toInt();
		} else if (reader->name() == "name"){
			d->fitData.paramNames<<reader->readElementText();
		} else if (reader->name() == "startValue"){
			d->fitData.paramStartValues<<reader->readElementText().toDouble();
		} else if (reader->name() == "value"){
			d->fitResult.paramValues<<reader->readElementText().toDouble();
		} else if (reader->name() == "error"){
			d->fitResult.errorValues<<reader->readElementText().toDouble();
		}else if (reader->name() == "fitResult"){
			attribs = reader->attributes();

			str = attribs.value("available").toString();
            if(str.isEmpty())
                reader->raiseWarning(attributeWarning.arg("'available'"));
            else
                d->fitResult.available = str.toInt();

			str = attribs.value("valid").toString();
            if(str.isEmpty())
                reader->raiseWarning(attributeWarning.arg("'valid'"));
            else
                d->fitResult.valid = str.toInt();

			str = attribs.value("status").toString();
            if(str.isEmpty())
                reader->raiseWarning(attributeWarning.arg("'status'"));
            else
                d->fitResult.status = str;

			str = attribs.value("iterations").toString();
            if(str.isEmpty())
                reader->raiseWarning(attributeWarning.arg("'iterations'"));
            else
                d->fitResult.iterations = str.toInt();

			str = attribs.value("time").toString();
            if(str.isEmpty())
                reader->raiseWarning(attributeWarning.arg("'time'"));
            else
                d->fitResult.elapsedTime = str.toInt();

			str = attribs.value("dof").toString();
            if(str.isEmpty())
                reader->raiseWarning(attributeWarning.arg("'dof'"));
            else
                d->fitResult.dof = str.toDouble();

			str = attribs.value("sse").toString();
            if(str.isEmpty())
                reader->raiseWarning(attributeWarning.arg("'sse'"));
            else
                d->fitResult.sse = str.toDouble();

			str = attribs.value("mse").toString();
            if(str.isEmpty())
                reader->raiseWarning(attributeWarning.arg("'mse'"));
            else
                d->fitResult.mse = str.toDouble();

			str = attribs.value("rmse").toString();
            if(str.isEmpty())
                reader->raiseWarning(attributeWarning.arg("'rmse'"));
            else
                d->fitResult.rmse = str.toDouble();

			str = attribs.value("mae").toString();
            if(str.isEmpty())
                reader->raiseWarning(attributeWarning.arg("'mae'"));
            else
                d->fitResult.mae = str.toDouble();

			str = attribs.value("rms").toString();
            if(str.isEmpty())
                reader->raiseWarning(attributeWarning.arg("'rms'"));
            else
                d->fitResult.rms = str.toDouble();

			str = attribs.value("rsd").toString();
            if(str.isEmpty())
                reader->raiseWarning(attributeWarning.arg("'rsd'"));
            else
                d->fitResult.rsd = str.toDouble();

			str = attribs.value("rsquared").toString();
            if(str.isEmpty())
                reader->raiseWarning(attributeWarning.arg("'rsquared'"));
            else
                d->fitResult.rsquared = str.toDouble();

			str = attribs.value("rsquaredAdj").toString();
            if(str.isEmpty())
                reader->raiseWarning(attributeWarning.arg("'rsquaredAdj'"));
            else
                d->fitResult.rsquaredAdj = str.toDouble();

			str = attribs.value("solverOutput").toString();
            if(str.isEmpty())
                reader->raiseWarning(attributeWarning.arg("'solverOutput'"));
            else
                d->fitResult.solverOutput = str;
		} else if(reader->name() == "column") {
			Column* column = new Column("", AbstractColumn::Numeric);
			if (!column->load(reader)) {
				delete column;
				return false;
			}
			if (column->name()=="x")
				d->xColumn = column;
			else if (column->name()=="y")
				d->yColumn = column;
			else if (column->name()=="residuals")
				d->residualsColumn = column;
		}
	}

	if (d->xColumn) {
		d->xColumn->setHidden(true);
		addChild(d->xColumn);

		d->yColumn->setHidden(true);
		addChild(d->yColumn);

		addChild(d->residualsColumn);

		d->xVector = static_cast<QVector<double>* >(d->xColumn->data());
		d->yVector = static_cast<QVector<double>* >(d->yColumn->data());
		d->residualsVector = static_cast<QVector<double>* >(d->residualsColumn->data());

		setUndoAware(false);
		setXColumn(d->xColumn);
		setYColumn(d->yColumn);
		setUndoAware(true);
	}

	retransform();
	return true;
}
