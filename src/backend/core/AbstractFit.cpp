/***************************************************************************
    File                 : AbstractFit.cpp
    Project              : SciDAVis
    --------------------------------------------------------------------
    Copyright            : (C) 2008 by Knut Franke
    Email (use @ for *)  : knut.franke*gmx.de
    Description          : Base class for doing fits using the algorithms
                           provided by GSL.

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

#include "AbstractFit.h"
#include <QUndoCommand>
#include <gsl/gsl_statistics.h>

/**
 * \brief Command for setting the error source of a fit.
 */
class FitSetYErrorSourceCmd : public QUndoCommand
{
	public:
		FitSetYErrorSourceCmd(AbstractFit * target, AbstractFit::ErrorSource source) :
			m_target(target), m_other_source(source) {
				setText(QObject::i18n("%1: change error source to %2.", m_target->name(), AbstractFit::nameOf(m_other_source)));
			}

		void undo() {
			AbstractFit::ErrorSource tmp = m_target->m_y_error_source;
			m_target->m_y_error_source = m_other_source;
			m_other_source = tmp;
			m_target->dataChanged(0);
		}

		void redo() { undo(); }

	private:
		AbstractFit * m_target;
		AbstractFit::ErrorSource m_other_source;
};

/**
 * \brief Column representing the result parameters of a fit.
 */
class ResultsColumn : public AbstractColumn
{

	public:
		ResultsColumn(AbstractFit * owner) : AbstractColumn(i18n("value")), m_owner(owner) {}

		virtual SciDAVis::ColumnMode columnMode() const { return SciDAVis::Numeric; }
		virtual int rowCount() const { return m_owner->numParameters(); }
		virtual SciDAVis::PlotDesignation plotDesignation() const { return SciDAVis::Y; }
		virtual double valueAt(int row) const { return gsl_vector_get(m_owner->m_results, row); }

	private:
		AbstractFit * m_owner;
};

/**
 * \brief Column representing standard errors of result parameters.
 */
class ErrorsColumn : public AbstractColumn
{

	public:
		ErrorsColumn(AbstractFit * owner) : AbstractColumn(i18n("error")), m_owner(owner) {}

		virtual SciDAVis::ColumnMode columnMode() const { return SciDAVis::Numeric; }
		virtual int rowCount() const { return m_owner->numParameters(); }
		virtual SciDAVis::PlotDesignation plotDesignation() const { return SciDAVis::Y; }
		virtual double valueAt(int row) const { return sqrt(gsl_matrix_get(m_owner->m_covariance_matrix, row, row)); }

	private:
		AbstractFit * m_owner;
};

/**
 * \brief Column representing the parameter names of a fit.
 */
class NamesColumn : public AbstractColumn
{

	public:
		NamesColumn(AbstractFit * owner) : AbstractColumn(i18n("error")), m_owner(owner) {}

		virtual SciDAVis::ColumnMode columnMode() const { return SciDAVis::Text; }
		virtual int rowCount() const { return m_owner->numParameters(); }
		virtual SciDAVis::PlotDesignation plotDesignation() const { return SciDAVis::Y; }
		virtual QString textAt(int row) const { return m_owner->parameterName(row); }

	private:
		AbstractFit * m_owner;
};

/**
 * \brief Column representing descriptions for the parameters of a fit.
 */
class DescriptionsColumn : public AbstractColumn
{

	public:
		DescriptionsColumn(AbstractFit * owner) : AbstractColumn(i18n("error")), m_owner(owner) {}

		virtual SciDAVis::ColumnMode columnMode() const { return SciDAVis::Text; }
		virtual int rowCount() const { return m_owner->numParameters(); }
		virtual SciDAVis::PlotDesignation plotDesignation() const { return SciDAVis::Y; }
		virtual QString textAt(int row) const { return m_owner->parameterDescription(row); }

	private:
		AbstractFit * m_owner;
};

/**
 * \class AbstractFit
 * \brief Base class for least-squares fitting.
 *
 * This is a completely algorithm-agnostic minimal base class, providing mainly a common
 * interface for AbstractNonlinearFit and AbstractLinearFit.
 *
 * Input is accepted on three ports: X, Y and Y error. Y is assumed to be a function of X, which is
 * approximated by the function specified by the implementation of AbstractFit being used.
 *
 * On its output ports, AbstractFit provides (in order) the current values of the parameters,
 * error estimates, their names, and textual descriptions. Current values can be initial values
 * specified by the user, automated estimates made by the implementation, or the results of the fit.
 * You only ever get to see the first two when you haven't connected the input ports of
 * AbstractFit yet, because after the first connect, the fit is automatically recomputed.
 */

/**
 * \enum AbstractFit::ErrorSource
 * \brief Possible sources for error data.
 */
/**
 * \var AbstractFit::UnknownErrors
 * \brief Errors unknown; estimate parameter errors from scatter of input data.
 */
/**
 * \var AbstractFit::AssociatedErrors
 * \brief Use error data set associated with Y input.
 */
/**
 * \var AbstractFit::PoissonErrors
 * \brief Y data is Poisson distributed; use sqrt(Y) as error estimate.
 */
/**
 * \var AbstractFit::CustomErrors
 * \brief Use error data from user-specified data source.
 */

AbstractFit::AbstractFit() :
	AbstractFilter(metaObject()->className()),
	m_y_error_source(AssociatedErrors),
	m_y_errors(0),
	m_results(0),
	m_covariance_matrix(0),
	m_chi_square(-1)
{
	m_outputs = new AbstractColumn*[4];
	m_outputs[0] = new ResultsColumn(this);
	m_outputs[1] = new ErrorsColumn(this);
	m_outputs[2] = new NamesColumn(this);
	m_outputs[3] = new DescriptionsColumn(this);
}

AbstractFit::~AbstractFit()
{
	for (int i=0; i<4; i++)
		delete m_outputs[i];
	delete[] m_outputs;
	delete[] m_y_errors;
	if (m_results)
		gsl_vector_free(m_results);
	if (m_covariance_matrix)
		gsl_matrix_free(m_covariance_matrix);
}

/**
 * \brief Currently set source of Y standard errors.
 */
ErrorSource AbstractFit::yErrorSource() const {
	return m_y_error_source;
}

/**
 * \brief Change the source of Y standard errors.
 */
void AbstractFit::setYErrorSource(ErrorSource e)
{
	exec(new FitSetYErrorSourceCmd(this, e));
}

bool AbstractFit::inputAcceptable(int port, const AbstractColumn *source)
{
	if (port < 0 || port >= inputCount())
		return false;
	return source->columnMode() == SciDAVis::Numeric;
}

/**
 * \brief Update internal state (number of input points and Y errors may have changed).
 */
void AbstractFit::dataChanged(AbstractColumn*)
{
	const AbstractColumn * x = m_inputs.value(0);
	const AbstractColumn * y = m_inputs.value(1);
	if (!x || !y) return;
	m_input_points = qMin(x->rowCount(), y->rowCount());
	if (m_y_error_source == CustomErrors) {
		const AbstractColumn * err = m_inputs.value(2);
		if (!err) return;
		m_input_points = qMin(m_input_points, err->rowCount());
	}

	delete[] m_y_errors;
	m_y_errors = new double[m_input_points];

	switch (m_y_error_source) {
		case UnknownErrors:
			for (int i=0; i<m_input_points; i++)
				m_y_errors[i] = 1.0;
			break;
		case PoissonErrors:
			{
			const AbstractColumn * y = m_inputs.value(1);
			if (!y) return;
			for (int i=0; i<m_input_points; i++)
				m_y_errors[i] = 1.0/sqrt(y->valueAt(i));
			break;
			}
		case CustomErrors:
			{
			const AbstractColumn * err = m_inputs.value(2);
			if (!err) return;
			for (int i=0; i<m_input_points; i++)
				m_y_errors[i] = err->valueAt(i);
			break;
			}
	}

	if (m_results)
		gsl_vector_free(m_results);
	m_results = gsl_vector_alloc(numParameters());
	if (m_covariance_matrix)
		gsl_matrix_free(m_covariance_matrix);
	m_covariance_matrix = gsl_matrix_alloc(numParameters(), numParameters());
}

double AbstractFit::rSquare() const
{
	const AbstractColumn * y = m_inputs.value(1);
	if (!y) return 0;

	double * y_vals = new double[m_input_points];
	double * weights = new double[m_input_points];
	for (int i=0; i<m_input_points; i++) {
		y_vals[i] = y->valueAt(i);
		weights[i] = 1.0/pow(m_y_errors[i], 2);
	}
	double tss = gsl_stats_wtss(weights, 1, y_vals, 1, m_input_points);
	delete[] y_vals;
	delete[] weights;
	return 1 - m_chi_square/tss;
}

/**
 * \var AbstractFit::m_input_points
 * \brief Number of data points supplied on inputs.
 */

/**
 * \var AbstractFit::m_y_error_source
 * \brief Where to take Y error estimates from.
 */

/**
 * \var AbstractFit::m_y_errors
 * \brief Y error estimates
 */

/**
 * \var AbstractFit::m_results
 * \brief Resulting fit parameters.
 */

/**
 * \var AbstractFit::m_covariance_matrix
 * \brief Covariance matrix of results.
 */

/**
 * \var AbstractFit::m_chi_square
 * \brief Residual sum of squares.
 */

/**
 * \var AbstractFit::m_outputs
 * \brief Output ports.
 */


