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

class AbstractFit : public AbstractFilter
{
	Q_OBJECT

	public:
		enum ErrorSource {
			UnknownErrors,
			AssociatedErrors,
			PoissonErrors,
			CustomErrors
		};
		static QString nameOf(ErrorSource source) {
			switch(source) {
				case UnknownErrors: return i18n("unknown");
				case AssociatedErrors: return i18n("associated");
				case PoissonErrors: return i18n("Poisson (sqrt(Y))");
				case CustomErrors: return i18n("user-supplied");
			}
		}
		AbstractFit();
		virtual ~AbstractFit();

		ErrorSource yErrorSource() const;
		void setYErrorSource(ErrorSource e);

		virtual int numParameters() const = 0;
		virtual QString parameterName(int index) const = 0;
		virtual QString parameterDescription(int index) const = 0;

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
		void dataChanged(AbstractColumn*);
		virtual bool inputAcceptable(int port, const AbstractColumn *source);

		int m_input_points;
		ErrorSource m_y_error_source;
		double * m_y_errors;
		gsl_vector * m_results;
		gsl_matrix * m_covariance_matrix;
		double m_chi_square;

	private:
		AbstractColumn ** m_outputs;

		friend class FitSetYErrorSourceCmd;
		friend class ResultsColumn;
		friend class ErrorsColumn;
		friend class NamesColumn;
		friend class DescriptionsColumn;
};

#endif // ifndef ABSTRACT_FIT_H
