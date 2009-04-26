/***************************************************************************
    File                 : ColumnPrivate.h
    Project              : SciDAVis
    Description          : Private data class of Column
    --------------------------------------------------------------------
    Copyright            : (C) 2007,2008 Tilman Benkert (thzs*gmx.net)
                           (replace * with @ in the email addresses) 

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

#ifndef COLUMNPRIVATE_H
#define COLUMNPRIVATE_H

#include <QObject>
#include "lib/IntervalAttribute.h"
#include "core/column/Column.h"
class AbstractSimpleFilter;
class QString;

class Column::Private
{
	public:
		Private(Column * owner, SciDAVis::ColumnMode mode);
		~Private();
		Private(Column * owner, SciDAVis::ColumnMode mode, void * data);

		SciDAVis::ColumnMode columnMode() const;
		void setColumnMode(SciDAVis::ColumnMode mode);

		bool copy(const AbstractColumn * other);
		bool copy(const AbstractColumn * source, int source_start, int dest_start, int num_rows);
		bool copy(const Private * other);
		bool copy(const Private * source, int source_start, int dest_start, int num_rows);
		int rowCount() const;
		void resizeTo(int new_size);
		void insertRows(int before, int count);
		void removeRows(int first, int count);
		QString name() const;
		SciDAVis::PlotDesignation plotDesignation() const;
		void setPlotDesignation(SciDAVis::PlotDesignation pd);
		int width() const;
		void setWidth(int value);
		void *dataPointer() const;
		AbstractSimpleFilter* inputFilter() const;
		AbstractSimpleFilter* outputFilter() const;
		void replaceModeData(SciDAVis::ColumnMode mode, void * data, AbstractSimpleFilter *in_filter,
				AbstractSimpleFilter *out_filter);
		void replaceData(void * data);
		IntervalAttribute<bool> maskingAttribute() const;
		void replaceMasking(IntervalAttribute<bool> masking); 
		IntervalAttribute<QString> formulaAttribute() const;
		void replaceFormulas(IntervalAttribute<QString> formulas); 

		bool isMasked(int row) const;
		bool isMasked(Interval<int> i) const;
		QList< Interval<int> > maskedIntervals() const;
		void clearMasks();
		void setMasked(Interval<int> i, bool mask = true);
		void setMasked(int row, bool mask = true);

		QString formula(int row) const;
		QList< Interval<int> > formulaIntervals() const;
		void setFormula(Interval<int> i, QString formula);
		void setFormula(int row, QString formula);
		void clearFormulas();
		
		QString textAt(int row) const;
		void setTextAt(int row, const QString& new_value);
		void replaceTexts(int first, const QStringList& new_values);
		QDate dateAt(int row) const;
		void setDateAt(int row, const QDate& new_value);
		QTime timeAt(int row) const;
		void setTimeAt(int row, const QTime& new_value);
		QDateTime dateTimeAt(int row) const;
		void setDateTimeAt(int row, const QDateTime& new_value);
		void replaceDateTimes(int first, const QList<QDateTime>& new_values);
		double valueAt(int row) const;
		void setValueAt(int row, double new_value);
		void replaceValues(int first, const QVector<double>& new_values);

	private:
		SciDAVis::ColumnMode m_column_mode;
		void * m_data;
		AbstractSimpleFilter* m_input_filter;
		AbstractSimpleFilter* m_output_filter;
		IntervalAttribute<bool> m_masking;
		IntervalAttribute<QString> m_formulas;
		SciDAVis::PlotDesignation m_plot_designation;
		int m_width;
		Column * m_owner;
};

#endif
