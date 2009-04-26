/***************************************************************************
    File                 : StatisticsFilter.h
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
#ifndef STATISTICS_FILTER_H
#define STATISTICS_FILTER_H

#include "core/AbstractFilter.h"

class StatisticsColumn;

class StatisticsFilter : public AbstractFilter {
	public:
		StatisticsFilter();
		virtual ~StatisticsFilter();
		virtual int inputCount() const;
		virtual int outputCount() const;
		virtual AbstractColumn *output(int port);
		int rowCount() const;
	protected:
		virtual bool inputAcceptable(int port, const AbstractColumn *source);
		virtual void inputDescriptionAboutToChange(const AbstractColumn*);
		virtual void inputDescriptionChanged(const AbstractColumn*);
		virtual void inputDataAboutToChange(const AbstractColumn*);
		virtual void inputAboutToBeDisconnected(const AbstractColumn*);
		virtual void inputDataChanged(int port);
	private:
		struct Statistics {
			int first_valid_row, last_valid_row, min_index, max_index, N;
			double min, max, sum, variance;
		};
		enum StatItem { Label, Rows, Mean, Sigma, Variance, Sum, iMax, Max, iMin, Min, N };

		QVector<Statistics> m_s;
		StatisticsColumn* m_columns[11];
		friend class StatisticsColumn;
};

#endif // ifndef STATISTICS_FILTER_H

