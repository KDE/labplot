/***************************************************************************
    File                 : Fit.h
    Project              : SciDAVis
    --------------------------------------------------------------------
    Copyright            : (C) 2006 by Ion Vasilief, Tilman Benkert
    Email (use @ for *)  : ion_vasilief*yahoo.fr, thzs*gmx.net
    Description          : Fit base class
                           
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
#ifndef FIT_H
#define FIT_H

#include <QObject>

#include "ApplicationWindow.h"
#include "Filter.h"

#include <gsl/gsl_multifit_nlin.h>
#include <gsl/gsl_multimin.h>

class Table;
class Matrix;

//! Fit base class
class Fit : public Filter
{
	Q_OBJECT

	public:

		typedef double (*fit_function_simplex)(const gsl_vector *, void *);
		typedef int (*fit_function)(const gsl_vector *, void *, gsl_vector *);
		typedef int (*fit_function_df)(const gsl_vector *, void *, gsl_matrix *);
		typedef int (*fit_function_fdf)(const gsl_vector *, void *, gsl_vector *, gsl_matrix *);

		enum Algorithm{ScaledLevenbergMarquardt, UnscaledLevenbergMarquardt, NelderMeadSimplex};
		enum WeightingMethod{NoWeighting, Instrumental, Statistical, Dataset};

		Fit(ApplicationWindow *parent, Layer *layer = 0, const char * name = 0);
		~Fit();

		//! Actually does the fit. Should be reimplemented in derived classes.
		virtual void fit();
        virtual bool run(){return false;};

		//! Sets the data set to be used for weighting
		bool setWeightingData(WeightingMethod w, const QString& colName = QString::null);

		void setDataCurve(int curve, double start, double end);

		QString formula(){return m_formula;};
		int numParameters() {return m_p;}

		void setInitialGuess(int parIndex, double val){gsl_vector_set(m_param_init, parIndex, val);};
		void setInitialGuesses(double *x_init);

		virtual void guessInitialValues(){};

		void setAlgorithm(Algorithm s){m_solver = s;};

		//! Specifies weather the result of the fit is a function curve
		void generateFunction(bool yes, int points = 100);

		//! Output string added to the plot as a new legend
		virtual QString legendInfo();

		//! Returns a vector with the fit results
		double* results(){return m_results;};

		//! Returns a vector with the standard deviations of the results
		double* errors();

		//! Returns the sum of squares of the residuals from the best-fit line
		double chiSquare() {return chi_2;};

		//! Returns R^2
		double rSquare();

		//! Specifies wheather the errors must be scaled with sqrt(chi_2/dof)
		void scaleErrors(bool yes = true){m_scale_errors = yes;};

		Table* parametersTable(const QString& tableName);
		Matrix* covarianceMatrix(const QString& matrixName);

	private:
		//! Pointer to the GSL multifit minimizer (for simplex algorithm)
		gsl_multimin_fminimizer * fitSimplex(gsl_multimin_function f, int &iterations, int &status);

		//! Pointer to the GSL multifit solver
		gsl_multifit_fdfsolver * fitGSL(gsl_multifit_function_fdf f, int &iterations, int &status);

		//! Customs and stores the fit results according to the derived class specifications. Used by exponential fits.
		virtual void storeCustomFitResults(double *par);

	protected:
		//! Adds the result curve as a FunctionCurve to the plot, if m_gen_function = true
		void insertFitFunctionCurve(const QString& name, double *x, double *y, int penWidth = 1);

		//! Adds the result curve to the plot
		virtual void generateFitCurve(double *par);

		//! Calculates the data for the output fit curve and store itin the X an Y vectors
		virtual void calculateFitCurveData(double *par, double *X, double *Y) { Q_UNUSED(par) Q_UNUSED(X) Q_UNUSED(Y)   };

		//! Output string added to the result log
		virtual QString logFitInfo(double *par, int iterations, int status, const QString& plotName);

		fit_function m_f;
		fit_function_df m_df;
		fit_function_fdf m_fdf;
		fit_function_simplex m_fsimplex;

		//! Number of fit parameters
		int m_p;

		//! Initial guesses for the fit parameters 
		gsl_vector *m_param_init;

		/*! \brief Tells whether the fitter uses non-linear/simplex fitting 
		 * with an initial parameters set, that must be freed in the destructor.
		 */
		bool is_non_linear;

		//! weighting data set used for the fit
		double *m_w;

		//! Names of the fit parameters
		QStringList m_param_names;

		//! Stores a list of short explanations for the significance of the fit parameters
		QStringList m_param_explain;

		//! Specifies weather the result curve is a FunctionCurve or a normal curve with the same x values as the fit data
		bool m_gen_function;

		//! Algorithm type
		Algorithm m_solver;

		//! The fit formula
		QString m_formula;

		//! Covariance matrix
		gsl_matrix *covar;

		//! The kind of weighting to be performed on the data
		WeightingMethod m_weihting;

		//! The name of the weighting dataset
		QString weighting_dataset;

		//! Stores the result parameters
		double *m_results;

		//! Stores standard deviations of the result parameters
		double *m_errors;

		//! The sum of squares of the residuals from the best-fit line
		double chi_2;

		//! Specifies wheather the errors must be scaled with sqrt(chi_2/dof)
		bool m_scale_errors;
};

#endif
