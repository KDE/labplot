/***************************************************************************
    File                 : StatisticsFilter.cpp
    Project              : SciDAVis
    --------------------------------------------------------------------
    Copyright            : (C) 2007,2009 by Knut Franke
    Email (use @ for *)  : knut.franke*gmx.de
    Description          : Computes standard statistics on any number of inputs.

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
#include "StatisticsFilter.h"
#include "core/AbstractColumn.h"
#include <math.h>

class StatisticsColumn : public AbstractColumn {
	public:
		StatisticsColumn(const StatisticsFilter *parent, StatisticsFilter::StatItem item);
		virtual int rowCount() const;
		virtual double valueAt(int row) const;
		virtual QString textAt(int row) const;
		virtual SciDAVis::PlotDesignation plotDesignation() const;
		virtual SciDAVis::ColumnMode columnMode() const;
		static QString nameForItem(StatisticsFilter::StatItem item);
	private:
		friend class StatisticsFilter;
		const StatisticsFilter *m_parent;
		StatisticsFilter::StatItem m_item;
};

/** ***************************************************************************
 * \class StatisticsFilter
 * \brief Computes standard statistics on any number of inputs.
 *
 * This filter is functionally equivalent to TableStatistics (except that row statics
 * are not implemented yet). It takes any number of inputs, computes statistics on
 * each and returns them on its outputs. Each output port corresponds to one
 * statistical item and provides as many rows as input ports are connected.
 ** ***************************************************************************/

/**
 * \struct StatisticsFilter::Statistics
 * \brief The values being cached for each input column provided.
 */

/**
 * \brief Standard constructor.
 */
StatisticsFilter::StatisticsFilter() : AbstractFilter(i18n("Statistics")) {
	for (int i=0; i<11; i++)
		m_columns[i] = new StatisticsColumn(this, (StatItem)i);
}

/**
 * \brief Destructor.
 */
StatisticsFilter::~StatisticsFilter() {
}

/**
 * \brief Accept any number of inputs.
 */
int StatisticsFilter::inputCount() const {
	return -1;
}

/**
 * \brief Currently, 11 statistics items are computed (see the private StatItem enum).
 */
int StatisticsFilter::outputCount() const {
	return 11;
}

AbstractColumn *StatisticsFilter::output(int port) {
	if (port < 0 || port >= outputCount()) return 0;
	return m_columns[port];
}

/**
 * \brief Number of rows = number of inputs that have been provided.
 */
int StatisticsFilter::rowCount() const {
	int result = 0;
	foreach(const AbstractColumn *i, m_inputs)
		if (i) result++;
	return result;
}

bool StatisticsFilter::inputAcceptable(int port, const AbstractColumn *source) {
	Q_UNUSED(port);
	return source->columnMode() == SciDAVis::Numeric;
}

void StatisticsFilter::inputDescriptionAboutToChange(const AbstractColumn*) {
	m_columns[0]->dataAboutToChange(m_columns[0]);
}

void StatisticsFilter::inputDescriptionChanged(const AbstractColumn*) {
	m_columns[0]->dataChanged(m_columns[0]);
}

void StatisticsFilter::inputDataAboutToChange(const AbstractColumn*) {
	// relay signal to all outputs but the first (which only holds the data source labels)
	for (int i=1; i<11; i++)
		m_columns[i]->dataAboutToChange(m_columns[i]);
}

void StatisticsFilter::inputAboutToBeDisconnected(const AbstractColumn *source)
{
	if (m_inputs.indexOf(source)+1 == m_s.size())
		m_s.resize(m_s.size()-1);
}

/**
 * \brief This is where the magic happens: data changes on an input port cause the corresponding entry in #m_s to be recomputed.
 */
void StatisticsFilter::inputDataChanged(int port)
{
	if (port >= m_s.size()) m_s.resize(port+1);
	Statistics *s = &m_s[port];

	// initialize some entries for the following iteration
	s->sum = 0; s->N = 0;
	s->min_index = s->max_index = s->first_valid_row = s->last_valid_row = -1;
	s->min = INFINITY;
	s->max = -INFINITY;

	const AbstractColumn *column = m_inputs.at(port);
	if (!column) return;

	// iterate over all rows, determining first_valid_row, last_valid_row, N, min, max and sum
	for (int row = 0; row <= column->rowCount(); row++) {
		double val = m_inputs.at(port)->valueAt(row);
		if (std::isnan(val)) continue;
		if (s->first_valid_row == -1) s->first_valid_row = row;
		s->last_valid_row = row;
		s->N++;
		if (val < s->min) {
			s->min = val;
			s->min_index = row;
		} else if (val > s->max) {
			s->max = val;
			s->max_index = row;
		}
		s->sum += val;
	}

	// iterate a second time, using the now-known mean to compute the variance
	double mean = s->sum / double(s->N);
	s->variance = 0;
	for (int row = 0; row <= column->rowCount(); row++) {
		double val = column->valueAt(row);
		if (std::isnan(val)) continue;
		s->variance += pow(val - mean, 2);
	}
	s->variance /= double(s->N);

	// emit signals on all output ports that might have changed
	for (int i=1; i<11; i++)
		m_columns[i]->dataChanged(m_columns[i]);
}

/** ***************************************************************************
 * \class StatisticsColumn
 * \brief Provides vectorized access to the results of a StatisticsFilter
 ** ***************************************************************************/

/**
 * \brief Standard constructor
 */
StatisticsColumn::StatisticsColumn(const StatisticsFilter *parent, StatisticsFilter::StatItem item)
: AbstractColumn(nameForItem(item)), m_parent(parent), m_item(item) {}

QString StatisticsColumn::nameForItem(StatisticsFilter::StatItem item) {
	switch(item) {
		case StatisticsFilter::Label: return (i18n("Name"));
		case StatisticsFilter::Rows: return (i18n("Rows"));
		case StatisticsFilter::Mean: return (i18n("Mean"));
		case StatisticsFilter::Sigma: return (i18n("StandardDev"));
		case StatisticsFilter::Variance: return (i18n("Variance"));
		case StatisticsFilter::Sum: return (i18n("Sum"));
		case StatisticsFilter::iMax: return (i18n("iMax"));
		case StatisticsFilter::Max: return (i18n("Max"));
		case StatisticsFilter::iMin: return (i18n("iMin"));
		case StatisticsFilter::Min: return (i18n("Min"));
		case StatisticsFilter::N: return (i18n("N"));
	}
}

int StatisticsColumn::rowCount() const {
	return m_parent->rowCount();
}

double StatisticsColumn::valueAt(int row) const {
	if (row<0 || row>=m_parent->rowCount()) return NAN;
	const StatisticsFilter::Statistics *s = &(m_parent->m_s.at(row));
	switch(m_item) {
		case StatisticsFilter::Mean: return s->sum / double(s->N);
		case StatisticsFilter::Sigma: return sqrt(s->variance);
		case StatisticsFilter::Variance: return s->variance;
		case StatisticsFilter::Sum: return s->sum;
		case StatisticsFilter::iMax: return s->max_index + 1;
		case StatisticsFilter::Max: return s->max;
		case StatisticsFilter::iMin: return s->min_index + 1;
		case StatisticsFilter::Min: return s->min;
		case StatisticsFilter::N: return s->N;
		default: return NAN;
	}
}

QString StatisticsColumn::textAt(int row) const {
	const StatisticsFilter::Statistics *s = &(m_parent->m_s.at(row));
	switch (m_item) {
		case StatisticsFilter::Label:
			return m_parent->m_inputs.value(row) ?
				m_parent->m_inputs[row]->name() :
				QString();
		case StatisticsFilter::Rows: return QString("[%1,%2]").arg(s->first_valid_row + 1).arg(s->last_valid_row+1);
		default: return QString();
	}
}

SciDAVis::PlotDesignation StatisticsColumn::plotDesignation() const {
	return SciDAVis::noDesignation;
}

SciDAVis::ColumnMode StatisticsColumn::columnMode() const {
	switch (m_item) {
		case StatisticsFilter::Label:
		case StatisticsFilter::Rows:
			return SciDAVis::Text;
		case StatisticsFilter::Mean:
		case StatisticsFilter::Sigma:
		case StatisticsFilter::Variance:
		case StatisticsFilter::Sum:
		case StatisticsFilter::iMax:
		case StatisticsFilter::Max:
		case StatisticsFilter::iMin:
		case StatisticsFilter::Min:
		case StatisticsFilter::N:
			return SciDAVis::Numeric;
	}
}
