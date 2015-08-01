/***************************************************************************
    File                 : ColumnPrivate.h
    Project              : AbstractColumn
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

#include "backend/lib/IntervalAttribute.h"
#include "backend/core/column/Column.h"

class AbstractSimpleFilter;
class QString;

class Column::Private
{
	public:
		Private(Column * owner, AbstractColumn::ColumnMode mode);
		~Private();
		Private(Column * owner, AbstractColumn::ColumnMode mode, void * data);

		Column *owner() { return m_owner; }

		AbstractColumn::ColumnMode columnMode() const;
		void setColumnMode(AbstractColumn::ColumnMode mode);

		bool copy(const AbstractColumn * other);
		bool copy(const AbstractColumn * source, int source_start, int dest_start, int num_rows);
		bool copy(const Private * other);
		bool copy(const Private * source, int source_start, int dest_start, int num_rows);
		int rowCount() const;
		void resizeTo(int new_size);
		void insertRows(int before, int count);
		void removeRows(int first, int count);
		QString name() const;
		AbstractColumn::PlotDesignation plotDesignation() const;
		void setPlotDesignation(AbstractColumn::PlotDesignation);
		int width() const;
		void setWidth(int value);
		void *dataPointer() const;
		AbstractSimpleFilter* inputFilter() const;
		AbstractSimpleFilter* outputFilter() const;
		void replaceModeData(AbstractColumn::ColumnMode mode, void * data, AbstractSimpleFilter *in_filter,
				AbstractSimpleFilter *out_filter);
		void replaceData(void * data);
		IntervalAttribute<QString> formulaAttribute() const;
		void replaceFormulas(IntervalAttribute<QString> formulas);
		void replaceFormula(QString);

		QString formula() const;
		void setFormula(QString);

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
		AbstractColumn::ColumnMode m_column_mode;
		void * m_data;
		AbstractSimpleFilter* m_input_filter;
		AbstractSimpleFilter* m_output_filter;
		QString m_formula;
		IntervalAttribute<QString> m_formulas;
		AbstractColumn::PlotDesignation m_plot_designation;
		int m_width;
		Column * m_owner;
};

#endif
