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

class FitSetYErrorSourceCmd : public QUndoCommand
{
	public:
		FitSetYErrorSourceCmd(AbstractFit * target, AbstractFit::ErrorSource source) :
			m_target(target), m_other_source(source) {
				setText(QObject::tr("%1: change error source to %2.").arg(m_target->name()).arg(AbstractFit::nameOf(m_other_source)));
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

class ResultsColumn : public AbstractColumn
{

	public:
		ResultsColumn(AbstractFit * owner) : AbstractColumn(tr("value")), m_owner(owner) {}

		virtual SciDAVis::ColumnDataType dataType() const { return SciDAVis::TypeDouble; }
		virtual SciDAVis::ColumnMode columnMode() const { return SciDAVis::Numeric; }
		virtual int rowCount() const { return m_owner->numParameters(); }
		virtual SciDAVis::PlotDesignation plotDesignation() const { return SciDAVis::Y; }
		virtual double valueAt(int row) const { return gsl_vector_get(m_owner->m_results, row); }

	private:
		AbstractFit * m_owner;
};

class ErrorsColumn : public AbstractColumn
{

	public:
		ErrorsColumn(AbstractFit * owner) : AbstractColumn(tr("error")), m_owner(owner) {}

		virtual SciDAVis::ColumnDataType dataType() const { return SciDAVis::TypeDouble; }
		virtual SciDAVis::ColumnMode columnMode() const { return SciDAVis::Numeric; }
		virtual int rowCount() const { return m_owner->numParameters(); }
		virtual SciDAVis::PlotDesignation plotDesignation() const { return SciDAVis::Y; }
		virtual double valueAt(int row) const { return sqrt(gsl_matrix_get(m_owner->m_covariance_matrix, row, row)); }

	private:
		AbstractFit * m_owner;
};

class NamesColumn : public AbstractColumn
{

	public:
		NamesColumn(AbstractFit * owner) : AbstractColumn(tr("error")), m_owner(owner) {}

		virtual SciDAVis::ColumnDataType dataType() const { return SciDAVis::TypeQString; }
		virtual SciDAVis::ColumnMode columnMode() const { return SciDAVis::Text; }
		virtual int rowCount() const { return m_owner->numParameters(); }
		virtual SciDAVis::PlotDesignation plotDesignation() const { return SciDAVis::Y; }
		virtual QString textAt(int row) const { return m_owner->parameterName(row); }

	private:
		AbstractFit * m_owner;
};

class DescriptionsColumn : public AbstractColumn
{

	public:
		DescriptionsColumn(AbstractFit * owner) : AbstractColumn(tr("error")), m_owner(owner) {}

		virtual SciDAVis::ColumnDataType dataType() const { return SciDAVis::TypeQString; }
		virtual SciDAVis::ColumnMode columnMode() const { return SciDAVis::Text; }
		virtual int rowCount() const { return m_owner->numParameters(); }
		virtual SciDAVis::PlotDesignation plotDesignation() const { return SciDAVis::Y; }
		virtual QString textAt(int row) const { return m_owner->parameterDescription(row); }

	private:
		AbstractFit * m_owner;
};

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
