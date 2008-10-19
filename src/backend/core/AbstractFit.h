/***************************************************************************
    File                 : AbstractFit.h
    Project              : SciDAVis
    --------------------------------------------------------------------
    Copyright            : (C) 2007,2008 by Knut Franke
    Email (use @ for *)  : knut.franke*gmx.de
    Description          : Base class for least-squares fitting.

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
#ifndef ABSTRACT_FIT_H
#define ABSTRACT_FIT_H

#include "AbstractFilter.h"

#include <math.h>
#include <gsl/gsl_vector.h>
#include <gsl/gsl_matrix.h>

class FitSetYErrorSourceCmd;
class ResultsColumn;
class ErrorsColumn;
class NamesColumn;
class DescriptionsColumn;

/**
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
class AbstractFit : public AbstractFilter
{
	Q_OBJECT

	public:
		//! Possible sources for error data.
		enum ErrorSource {
			UnknownErrors,    //!< Errors unknown; estimate parameter errors from scatter of input data.
			AssociatedErrors, //!< Use error data set associated with Y input.
			PoissonErrors,    //!< Y data is Poisson distributed; use sqrt(Y) as error estimate.
			CustomErrors      //!< Use error data from user-specified data source.
		};
		static QString nameOf(ErrorSource source) {
			switch(source) {
				case UnknownErrors: return tr("unknown");
				case AssociatedErrors: return tr("associated");
				case PoissonErrors: return tr("Poisson (sqrt(Y))");
				case CustomErrors: return tr("user-supplied");
			}
		}
		AbstractFit();
		virtual ~AbstractFit();

		ErrorSource yErrorSource() const { return m_y_error_source; }
		void setYErrorSource(ErrorSource e);

		//!\name Handling of fit parameters
		//@{
		virtual int numParameters() const = 0;
		virtual QString parameterName(int index) const = 0;
		virtual QString parameterDescription(int index) const = 0;
		//@}

		//!\name Reimplemented from AbstractFilter
		//@{
	public:
		virtual int inputCount() const { return 3; }
		virtual QString inputName(int port) const {
			switch (port) {
				case 0: return "X";
				case 1: return "Y";
				case 2: return "Y error";
			}
		}
		virtual int outputCount() const { return 4; }
		virtual AbstractColumn * output(int port=0) {
			if (port < 0 || port >= outputCount())
				return 0;
			return m_outputs[port];
		}
		//@}

	public:
		inline double X(int i) const {
			const AbstractColumn * x = m_inputs.value(0);
			if (!x) return NAN;
			return x->valueAt(i);
		}
		inline double Y(int i) const {
			const AbstractColumn * y = m_inputs.value(1);
			if (!y) return NAN;
			return y->valueAt(i);
		}
		inline double yError(int i) const {
			if (m_y_errors && 0 <= i && i < m_input_points)
				return m_y_errors[i];
			else
				return NAN;
		}
		double chiSquare() const {
			return m_chi_square;
		}
		double chiSquareDof() const {
			return m_chi_square / (m_input_points - numParameters());
		}
		double rSquare() const;

	protected:
		//! Update internal state (number of input points and Y errors may have changed).
		void dataChanged(AbstractColumn*);
		virtual bool inputAcceptable(int port, const AbstractColumn *source);

		//! Number of data points supplied on inputs.
		int m_input_points;
		//! Where to take Y error estimates from.
		ErrorSource m_y_error_source;
		//! Y error estimates
		double * m_y_errors;
		//! Resulting fit parameters.
		gsl_vector * m_results;
		//! Covariance matrix of results.
		gsl_matrix * m_covariance_matrix;
		//! Residual sum of squares.
		double m_chi_square;

	private:
		//! Output ports.
		AbstractColumn ** m_outputs;

		friend class FitSetYErrorSourceCmd;
		friend class ResultsColumn;
		friend class ErrorsColumn;
		friend class NamesColumn;
		friend class DescriptionsColumn;
};

#endif // ifndef ABSTRACT_FIT_H
