/***************************************************************************
    File                 : Fit.cpp
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
#include "Fit.h"
#include "lib/ColorBox.h"
#include "analysis/fit_gsl.h"
#include "table/Table.h"
#include "matrix/Matrix.h"
#include "graph/Layer.h"
#include "graph/types/ErrorCurve.h"
#include "graph/FunctionCurve.h"

#include <gsl/gsl_statistics.h>
#include <gsl/gsl_blas.h>

#include <QApplication>
#include <QMessageBox>
#include <QDateTime>
#include <QLocale>

Fit::Fit( ApplicationWindow *parent, Layer *layer, const char * name)
: Filter( parent, layer, name)
{
	m_p = 0;
	m_n = 0;
	m_curveColorIndex = 1;
	m_solver = ScaledLevenbergMarquardt;
	m_tolerance = 1e-4;
	m_gen_function = true;
	m_points = 100;
	m_max_iterations = 1000;
	m_curve = 0;
	m_formula = QString::null;
	m_explanation = QString::null;
	m_weihting = NoWeighting;
	weighting_dataset = QString::null;
	is_non_linear = true;
	m_results = 0;
	m_errors = 0;
	m_prec = parent->fit_output_precision;
	m_init_err = false;
	chi_2 = -1;
	m_scale_errors = false;
	m_sort_data = true;
}

gsl_multifit_fdfsolver * Fit::fitGSL(gsl_multifit_function_fdf f, int &iterations, int &status)
{
	const gsl_multifit_fdfsolver_type *T;
	if (m_solver)
		T = gsl_multifit_fdfsolver_lmder;
	else
		T = gsl_multifit_fdfsolver_lmsder;

	gsl_multifit_fdfsolver *s = gsl_multifit_fdfsolver_alloc (T, m_n, m_p);
	gsl_multifit_fdfsolver_set (s, &f, m_param_init);

	size_t iter = 0;
	do
	{
		iter++;
		status = gsl_multifit_fdfsolver_iterate (s);

		if (status)
			break;

		status = gsl_multifit_test_delta (s->dx, s->x, m_tolerance, m_tolerance);
	}
	while (status == GSL_CONTINUE && (int)iter < m_max_iterations);

	gsl_multifit_covar (s->J, 0.0, covar);
	iterations = iter;
	return s;
}

gsl_multimin_fminimizer * Fit::fitSimplex(gsl_multimin_function f, int &iterations, int &status)
{
	const gsl_multimin_fminimizer_type *T = gsl_multimin_fminimizer_nmsimplex;

	//size of the simplex
	gsl_vector *ss;
	//initial vertex size vector
	ss = gsl_vector_alloc (f.n);
	//set all step sizes to 1 can be increased to converge faster
	gsl_vector_set_all (ss,10.0);

	gsl_multimin_fminimizer *s_min = gsl_multimin_fminimizer_alloc (T, f.n);
	status = gsl_multimin_fminimizer_set (s_min, &f, m_param_init, ss);
	double size;
	size_t iter = 0;
	do
	{
		iter++;
		status = gsl_multimin_fminimizer_iterate (s_min);

		if (status)
			break;
		size=gsl_multimin_fminimizer_size (s_min);
		status = gsl_multimin_test_size (size, m_tolerance);
	}

	while (status == GSL_CONTINUE && (int)iter < m_max_iterations);

	iterations = iter;
	gsl_vector_free(ss);
	return s_min;
}

void Fit::setDataCurve(int curve, double start, double end)
{
    if (m_n > 0)
		delete[] m_w;

    Filter::setDataCurve(curve, start, end);

    m_w = new double[m_n];
    if (m_layer && m_curve && ((PlotCurve *)m_curve)->type() != Layer::Function)
    {
        QList<DataCurve *> lst = ((DataCurve *)m_curve)->errorBarsList();
        foreach (DataCurve *c, lst)
        {
            ErrorCurve *er = (ErrorCurve *)c;
            if (!er->xErrors())
            {
                m_weihting = Instrumental;
                for (int i=0; i<m_n; i++)
                    m_w[i] = er->errorValue(i); //m_w are equal to the error bar values
                weighting_dataset = er->title().text();
                return;
            }
        }
    }
	// if no error bars initialize the weighting data to 1.0
    for (int i=0; i<m_n; i++)
        m_w[i] = 1.0;
}

void Fit::setInitialGuesses(double *x_init)
{
	for (int i = 0; i < m_p; i++)
		gsl_vector_set(m_param_init, i, x_init[i]);
}

void Fit::generateFunction(bool yes, int points)
{
	m_gen_function = yes;
	if (m_gen_function)
		m_points = points;
}

QString Fit::logFitInfo(double *par, int iterations, int status, const QString& plotName)
{
	QDateTime dt = QDateTime::currentDateTime ();
	QString info = "[" + dt.toString(Qt::LocalDate)+ "\t" + tr("Plot")+ ": ''" + plotName+ "'']\n";
	info += m_explanation + " " + tr("fit of dataset") + ": " + m_curve->title().text();
	if (!m_formula.isEmpty())
		info +=", " + tr("using function") + ": " + m_formula + "\n";
	else
		info +="\n";

	info += tr("Weighting Method") + ": ";
	switch(m_weihting)
	{
		case NoWeighting:
			info += tr("No weighting");
			break;
		case Instrumental:
			info += tr("Instrumental") + ", " + tr("using error bars dataset") + ": " + weighting_dataset;
			break;
		case Statistical:
			info += tr("Statistical");
			break;
		case Dataset:
			info += tr("Arbitrary Dataset") + ": " + weighting_dataset;
			break;
	}
	info +="\n";

	if (is_non_linear)
	{
		if (m_solver == NelderMeadSimplex)
			info+=tr("Nelder-Mead Simplex");
		else if (m_solver == UnscaledLevenbergMarquardt)
			info+=tr("Unscaled Levenberg-Marquardt");
		else
			info+=tr("Scaled Levenberg-Marquardt");

		info+=tr(" algorithm with tolerance = ") + QLocale().toString(m_tolerance)+"\n";
	}

	info+=tr("From x")+" = "+QLocale().toString(m_x[0], 'g', 15)+" "+tr("to x")+" = "+QLocale().toString(m_x[m_n-1], 'g', 15)+"\n";
	double chi_2_dof = chi_2/(m_n - m_p);
	for (int i=0; i<m_p; i++)
	{
		info += m_param_names[i]+" "+m_param_explain[i]+" = "+QLocale().toString(par[i], 'g', m_prec) + " +/- ";
		if (m_scale_errors)
			info += QLocale().toString(sqrt(chi_2_dof*gsl_matrix_get(covar,i,i)), 'g', m_prec) + "\n";
		else
			info += QLocale().toString(sqrt(gsl_matrix_get(covar,i,i)), 'g', m_prec) + "\n";
	}
	info += "--------------------------------------------------------------------------------------\n";
	info += "Chi^2/doF = " + QLocale().toString(chi_2_dof, 'g', m_prec) + "\n";

	double sst = (m_n-1)*gsl_stats_variance(m_y, 1, m_n);
	info += tr("R^2") + " = " + QLocale().toString(1 - chi_2/sst, 'g', m_prec) + "\n";
	info += "---------------------------------------------------------------------------------------\n";
	if (is_non_linear)
	{
		info += tr("Iterations")+ " = " + QString::number(iterations) + "\n";
		info += tr("Status") + " = " + gsl_strerror (status) + "\n";
		info +="---------------------------------------------------------------------------------------\n";
	}
	return info;
}

double Fit::rSquare()
{
	double sst = (m_n-1)*gsl_stats_variance(m_y, 1, m_n);
	return 1 - chi_2/sst;
}

QString Fit::legendInfo()
{
	QString info = tr("Dataset") + ": " + m_curve->title().text() + "\n";
	info += tr("Function") + ": " + m_formula + "\n\n";

	double chi_2_dof = chi_2/(m_n - m_p);
	info += "Chi^2/doF = " + QLocale().toString(chi_2_dof, 'g', m_prec) + "\n";
	double sst = (m_n-1)*gsl_stats_variance(m_y, 1, m_n);
	info += tr("R^2") + " = " + QLocale().toString(1 - chi_2/sst, 'g', m_prec) + "\n";

	for (int i=0; i<m_p; i++)
	{
		info += m_param_names[i] + " = " + QLocale().toString(m_results[i], 'g', m_prec) + " +/- ";
		if (m_scale_errors)
			info += QLocale().toString(sqrt(chi_2_dof*gsl_matrix_get(covar,i,i)), 'g', m_prec) + "\n";
		else
			info += QLocale().toString(sqrt(gsl_matrix_get(covar,i,i)), 'g', m_prec) + "\n";
	}
	return info;
}

bool Fit::setWeightingData(WeightingMethod w, const QString& colName)
{
	m_weihting = w;
	switch (m_weihting)
	{
		case NoWeighting:
			{
				weighting_dataset = QString::null;
				for (int i=0; i<m_n; i++)
					m_w[i] = 1.0;
			}
			break;
		case Instrumental:
			{
				bool error = true;
				ErrorCurve *er = 0;
				if (((PlotCurve *)m_curve)->type() != Layer::Function)
				{
					QList<DataCurve *> lst = ((DataCurve *)m_curve)->errorBarsList();
                	foreach (DataCurve *c, lst)
                	{
                    	er = (ErrorCurve *)c;
                    	if (!er->xErrors())
                    	{
                        	weighting_dataset = er->title().text();
                        	error = false;
                        	break;
                    	}
					}
                }
				if (error)
				{
					QMessageBox::critical((ApplicationWindow *)parent(), tr("Error"),
					tr("The curve %1 has no associated Y error bars. You cannot use instrumental weighting method.").arg(m_curve->title().text()));
					return false;
				}
				if (er)
				{
					for (int j=0; j<m_n; j++)
						m_w[j] = er->errorValue(j); //m_w are equal to the error bar values
				}
			}
			break;
		case Statistical:
			{
				weighting_dataset = m_curve->title().text();

				for (int i=0; i<m_n; i++)
					m_w[i] = sqrt(m_y[i]);
			}
			break;
		case Dataset:
			{//m_w are equal to the values of the arbitrary dataset
				if (colName.isEmpty())
					return false;

				Table* t = ((ApplicationWindow *)parent())->table(colName);
				if (!t)
					return false;

				if (t->rowCount() < m_n)
  	            {
  	            	QMessageBox::critical((ApplicationWindow *)parent(), tr("Error"),
  	                tr("The column %1 has less points than the fitted data set. Please choose another column!.").arg(colName));
  	                return false;
  	            }

				weighting_dataset = colName;

				int col = t->colIndex(colName);
				for (int i=0; i<m_n; i++)
					m_w[i] = t->cell(i, col);
			}
			break;
	}
	return true;
}

Table* Fit::parametersTable(const QString& tableName)
{
	ApplicationWindow *app = (ApplicationWindow *)parent();
	Table *t = app->newTable(tableName, m_p, 3);
	t->setHeader(QStringList() << tr("Parameter") << tr("Value") << tr ("Error"));
	for (int i=0; i<m_p; i++)
	{
		t->setText(i, 0, m_param_names[i]);
		t->setText(i, 1, QLocale().toString(m_results[i], 'g', m_prec));
		t->setText(i, 2, QLocale().toString(sqrt(gsl_matrix_get(covar,i,i)), 'g', m_prec));
	}

	t->setColPlotDesignation(2, SciDAVis::yErr);
	// TODO
	//t->setHeaderColType();
	//for (int j=0; j<3; j++)
	//	t->table()->adjustColumn(j);

	t->showNormal();
	return t;
}

Matrix* Fit::covarianceMatrix(const QString& matrixName)
{
	ApplicationWindow *app = (ApplicationWindow *)parent();
	Matrix* m = app->newMatrix(matrixName, m_p, m_p);
	for (int i = 0; i < m_p; i++)
	{
		for (int j = 0; j < m_p; j++)
			m->setText(i, j, QLocale().toString(gsl_matrix_get(covar, i, j), 'g', m_prec));
	}
	m->showNormal();
	return m;
}

double *Fit::errors()
{
	if (!m_errors) {
		m_errors = new double[m_p];
		double chi_2_dof = chi_2/(m_n - m_p);
		for (int i=0; i<m_p; i++)
		{
			if (m_scale_errors)
				m_errors[i] = sqrt(chi_2_dof*gsl_matrix_get(covar,i,i));
			else
				m_errors[i] = sqrt(gsl_matrix_get(covar,i,i));
		}
	}
	return m_errors;
}

void Fit::storeCustomFitResults(double *par)
{
	for (int i=0; i<m_p; i++)
		m_results[i] = par[i];
}

void Fit::fit()
{
	if (!m_layer || m_init_err)
		return;

	if (!m_n)
	{
		QMessageBox::critical((ApplicationWindow *)parent(), tr("Fit Error"),
				tr("You didn't specify a valid data set for this fit operation. Operation aborted!"));
		return;
	}
	if (!m_p)
	{
		QMessageBox::critical((ApplicationWindow *)parent(), tr("Fit Error"),
				tr("There are no parameters specified for this fit operation. Operation aborted!"));
		return;
	}
	if (m_p > m_n)
  	{
  		QMessageBox::critical((ApplicationWindow *)parent(), tr("Fit Error"),
  	    tr("You need at least %1 data points for this fit operation. Operation aborted!").arg(m_p));
  	    return;
  	}
	if (m_formula.isEmpty())
	{
		QMessageBox::critical((ApplicationWindow *)parent(), tr("Fit Error"),
				tr("You must specify a valid fit function first. Operation aborted!"));
		return;
	}

	QApplication::setOverrideCursor(Qt::WaitCursor);

	const char *function = m_formula.toAscii().constData();
	QString names = m_param_names.join (",");
	const char *parNames = names.toAscii().constData();

	struct FitData m_data = {m_n, m_p, m_x, m_y, m_w, function, parNames};

	int status, iterations = m_max_iterations;
	double *par = new double[m_p];
	if(m_solver == NelderMeadSimplex)
	{
		gsl_multimin_function f;
		f.f = m_fsimplex;
		f.n = m_p;
		f.params = &m_data;
		gsl_multimin_fminimizer *s_min = fitSimplex(f, iterations, status);

		for (int i=0; i<m_p; i++)
			par[i]=gsl_vector_get(s_min->x, i);

		// allocate memory and calculate covariance matrix based on residuals
		gsl_matrix *J = gsl_matrix_alloc(m_n, m_p);
		m_df(s_min->x,(void*)f.params, J);
		gsl_multifit_covar (J, 0.0, covar);
		chi_2 = s_min->fval;

		// free previousely allocated memory
		gsl_matrix_free (J);
		gsl_multimin_fminimizer_free (s_min);
	}
	else
	{
		gsl_multifit_function_fdf f;
		f.f = m_f;
		f.df = m_df;
		f.fdf = m_fdf;
		f.n = m_n;
		f.p = m_p;
		f.params = &m_data;
		gsl_multifit_fdfsolver *s = fitGSL(f, iterations, status);

		for (int i=0; i<m_p; i++)
			par[i]=gsl_vector_get(s->x, i);

		chi_2 = pow(gsl_blas_dnrm2(s->f), 2.0);
		gsl_multifit_fdfsolver_free(s);
	}

	storeCustomFitResults(par);

	ApplicationWindow *app = (ApplicationWindow *)parent();
	if (app->writeFitResultsToLog)
		app->updateLog(logFitInfo(m_results, iterations, status, m_layer->parentPlotName()));

	generateFitCurve(par);
	QApplication::restoreOverrideCursor();
}

void Fit::generateFitCurve(double *par)
{
	if (!m_gen_function)
		m_points = m_n;

	double *X = new double[m_points];
	double *Y = new double[m_points];

	calculateFitCurveData(par, X, Y);

	if (m_gen_function)
	{
		insertFitFunctionCurve(QString(name()) + tr("Fit"), X, Y);
		m_layer->replot();
		delete[] X;
		delete[] Y;
	}
	else
        m_layer->addFitCurve(addResultCurve(X, Y));
}

void Fit::insertFitFunctionCurve(const QString& name, double *x, double *y, int penWidth)
{
    QString title = m_layer->generateFunctionName(name);
	FunctionCurve *c = new FunctionCurve(FunctionCurve::Normal, title);
	c->setPen(QPen(ColorBox::color(m_curveColorIndex), penWidth));
	c->setData(x, y, m_points);
	c->setRange(m_x[0], m_x[m_n-1]);

	QString formula = m_formula;
	for (int j=0; j<m_p; j++)
	{
		QString parameter = QString::number(m_results[j], 'g', m_prec);
		formula.replace(m_param_names[j], parameter);
	}
	c->setFormula(formula.replace("--", "+").replace("-+", "-").replace("+-", "-"));
	m_layer->insertPlotItem(c, Layer::Line);
	m_layer->addFitCurve(c);
}

Fit::~Fit()
{
	if (!m_p)
		return;

	if (is_non_linear)
		gsl_vector_free(m_param_init);

	if (m_results) delete[] m_results;
	if (m_errors) delete[] m_errors;
	gsl_matrix_free (covar);
}
