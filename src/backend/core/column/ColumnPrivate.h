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

//! Private data class of Column
/**
 The writing interface defined here is only to be used by column commands and Column contructors.
*/
class Column::Private
{
	public:
		//! Ctor
		Private(Column * owner, SciDAVis::ColumnMode mode);
		//! Dtor
		~Private();
		//! Special ctor (to be called from Column only!)
		Private(Column * owner, SciDAVis::ColumnDataType type, SciDAVis::ColumnMode mode, 
				void * data, IntervalAttribute<bool> validity);

		//! Return the data type of the column
		SciDAVis::ColumnDataType dataType() const { return m_data_type; };
		//! Return whether the object is read-only
		bool isReadOnly() const { return false; };
		//! Return the column mode
		/**
		 * This function is most used by tables but can also be used
		 * by plots. The column mode specifies how to interpret 
		 * the values in the column additional to the data type.
		 */ 
		SciDAVis::ColumnMode columnMode() const { return m_column_mode; };
		//! Set the column mode
		/**
		 * This sets the column mode and, if
		 * necessary, converts it to another datatype.
		 * Remark: setting the mode back to undefined (the 
		 * initial value) is not supported.
		 */
		void setColumnMode(SciDAVis::ColumnMode mode);

		//! Copy another column of the same type
		/**
		 * This function will return false if the data type
		 * of 'other' is not the same as the type of 'this'.
		 * The validity information for the rows is also copied.
		 * Use a filter to convert a column to another type.
		 */
		bool copy(const AbstractColumn * other);
		//! Copies a part of another column of the same type
		/**
		 * This function will return false if the data type
		 * of 'other' is not the same as the type of 'this'.
		 * The validity information for the rows is also copied.
		 * \param other pointer to the column to copy
		 * \param src_start first row to copy in the column to copy
		 * \param dest_start first row to copy in
		 * \param num_rows the number of rows to copy
		 */ 
		bool copy(const AbstractColumn * source, int source_start, int dest_start, int num_rows);
		//! Copy another column of the same type
		/**
		 * This function will return false if the data type
		 * of 'other' is not the same as the type of 'this'.
		 * The validity information for the rows is also copied.
		 * Use a filter to convert a column to another type.
		 */
		bool copy(const Private * other);
		//! Copies a part of another column of the same type
		/**
		 * This function will return false if the data type
		 * of 'other' is not the same as the type of 'this'.
		 * The validity information for the rows is also copied.
		 * \param other pointer to the column to copy
		 * \param src_start first row to copy in the column to copy
		 * \param dest_start first row to copy in
		 * \param num_rows the number of rows to copy
		 */ 
		bool copy(const Private * source, int source_start, int dest_start, int num_rows);
		//! Return the data vector size
		/**
		 * This returns the number of rows that actually contain data. 
		 * Rows beyond this can be masked etc. but should be ignored by filters,
		 * plots etc.
		 */
		int rowCount() const;
		//! Resize the vector to the specified number of rows
		/**
		 * Since selecting and masking rows higher than the
		 * real internal number of rows is supported, this
		 * does not change the interval attributes. Also
		 * no signal is emitted. If the new rows are filled
		 * with values AbstractColumn::dataChanged()
		 * must be emitted.
		 */
		void resizeTo(int new_size);
		//! Insert some empty (or initialized with zero) rows
		void insertRows(int before, int count);
		//! Remove 'count' rows starting from row 'first'
		void removeRows(int first, int count);
		//! Return the column name/label
		QString name() const;
		//! Return the column comment
		QString comment() const;
		//! Return the column plot designation
		SciDAVis::PlotDesignation plotDesignation() const { return m_plot_designation; };
		//! Set the column plot designation
		void setPlotDesignation(SciDAVis::PlotDesignation pd);
		//! Clear the whole column
		void clear();
		//! Return the data pointer
		void * dataPointer() const { return m_data; }
		//! Return the input filter (for string -> data type conversion)
		AbstractSimpleFilter* inputFilter() const { return m_input_filter; }
		//! Return the output filter (for data type -> string  conversion)
		AbstractSimpleFilter* outputFilter() const { return m_output_filter; }
		//! Replace all mode related members
		/** 
		 * Replace column mode, data type, data pointer, validity and filters directly 
		 */
		void replaceModeData(SciDAVis::ColumnMode mode, SciDAVis::ColumnDataType type, void * data,
			AbstractSimpleFilter *in_filter, AbstractSimpleFilter *out_filter, IntervalAttribute<bool> validity);
		//! Replace data pointer and validity
		void replaceData(void * data, IntervalAttribute<bool> validity);
		//! Return the validity interval attribute
		IntervalAttribute<bool> validityAttribute() { return m_validity; }
		//! Return the masking interval attribute
		IntervalAttribute<bool> maskingAttribute() { return m_masking; }
		//! Replace the list of intervals of masked rows
		void replaceMasking(IntervalAttribute<bool> masking); 
		//! Return the interval attribute representing the formula strings
		IntervalAttribute<QString> formulaAttribute() { return m_formulas; }
		//! Replace the interval attribute for the formula strings
		void replaceFormulas(IntervalAttribute<QString> formulas); 

		//! \name IntervalAttribute related functions
		//@{
		//! Return whether a certain row contains an invalid value 	 
		bool isInvalid(int row) const { return m_validity.isSet(row); } 	 
		//! Return whether a certain interval of rows contains only invalid values 	 
		bool isInvalid(Interval<int> i) const { return m_validity.isSet(i); } 	 
		//! Return all intervals of invalid rows
		QList< Interval<int> > invalidIntervals() const { return m_validity.intervals(); } 	 
		//! Return whether a certain row is masked 	 
		bool isMasked(int row) const { return m_masking.isSet(row); } 	 
		//! Return whether a certain interval of rows rows is fully masked 	 
		bool isMasked(Interval<int> i) const { return m_masking.isSet(i); }
		//! Return all intervals of masked rows
		QList< Interval<int> > maskedIntervals() const { return m_masking.intervals(); } 	 
		//! Clear all validity information
		void clearValidity();
		//! Clear all masking information
		void clearMasks();
		//! Set an interval invalid or valid
		/**
		 * \param i the interval
		 * \param invalid true: set invalid, false: set valid
		 */ 
		void setInvalid(Interval<int> i, bool invalid = true);
		//! Overloaded function for convenience
		void setInvalid(int row, bool invalid = true);
		//! Set an interval masked
		/**
		 * \param i the interval
		 * \param mask true: mask, false: unmask
		 */ 
		void setMasked(Interval<int> i, bool mask = true);
		//! Overloaded function for convenience
		void setMasked(int row, bool mask = true);
		//@}

		//! \name Formula related functions
		//@{
		//! Return the formula associated with row 'row' 	 
		QString formula(int row) const { return m_formulas.value(row); }
		//! Return the intervals that have associated formulas
		/**
		 * This can be used to make a list of formulas with their intervals.
		 * Here is some example code:
		 *
		 * \code
		 * QStringList list;
		 * QList< Interval<int> > intervals = my_column.formulaIntervals();
		 * foreach(Interval<int> interval, intervals)
		 * 	list << QString(interval.toString() + ": " + my_column.formula(interval.start()));
		 * \endcode
		 */
		QList< Interval<int> > formulaIntervals() const { return m_formulas.intervals(); }
		//! Set a formula string for an interval of rows
		void setFormula(Interval<int> i, QString formula);
		//! Overloaded function for convenience
		void setFormula(int row, QString formula);
		//! Clear all formulas
		void clearFormulas();
		//@}
		
		//! \name type specific functions
		//@{
		//! Return the content of row 'row'.
		/**
		 * Use this only when dataType() is QString
		 */
		QString textAt(int row) const;
		//! Set the content of row 'row'
		/**
		 * Use this only when dataType() is QString
		 */
		void setTextAt(int row, const QString& new_value);
		//! Replace a range of values 
		/**
		 * Use this only when dataType() is QString
		 */
		void replaceTexts(int first, const QStringList& new_values);
		//! Return the date part of row 'row'
		/**
		 * Use this only when dataType() is QDateTime
		 */
		QDate dateAt(int row) const;
		//! Set the content of row 'row'
		/**
		 * Use this only when dataType() is QDateTime
		 */
		void setDateAt(int row, const QDate& new_value);
		//! Return the time part of row 'row'
		/**
		 * Use this only when dataType() is QDateTime
		 */
		QTime timeAt(int row) const;
		//! Set the content of row 'row'
		/**
		 * Use this only when dataType() is QDateTime
		 */
		void setTimeAt(int row, const QTime& new_value);
		//! Return the QDateTime in row 'row'
		/**
		 * Use this only when dataType() is QDateTime
		 */
		QDateTime dateTimeAt(int row) const;
		//! Set the content of row 'row'
		/**
		 * Use this only when dataType() is QDateTime
		 */
		void setDateTimeAt(int row, const QDateTime& new_value);
		//! Replace a range of values 
		/**
		 * Use this only when dataType() is QDateTime
		 */
		void replaceDateTimes(int first, const QList<QDateTime>& new_values);
		//! Return the double value in row 'row'
		double valueAt(int row) const;
		//! Set the content of row 'row'
		/**
		 * Use this only when dataType() is double
		 */
		void setValueAt(int row, double new_value);
		//! Replace a range of values 
		/**
		 * Use this only when dataType() is double
		 */
		void replaceValues(int first, const QVector<double>& new_values);
		//@}

	private:
		//! \name data members
		//@{
		//! Data type string
		/**
		 * double, QString, or QDateTime
		 */ 
		SciDAVis::ColumnDataType m_data_type;
		//! The column mode
		/**
		 * The column mode specifies how to interpret 
		 * the values in the column additional to the data type.
		 */
		SciDAVis::ColumnMode m_column_mode;
		//! Pointer to the data vector
		/**
		 * This will point to a QVector<double>, QStringList or
		 * QList<QDateTime> depending on the stored data type.
		 */
		void * m_data;
		//! The input filter (for string -> data type conversion)
		AbstractSimpleFilter* m_input_filter;
		//! The output filter (for data type -> string conversion)
		AbstractSimpleFilter* m_output_filter;
		IntervalAttribute<bool> m_validity;
		IntervalAttribute<bool> m_masking;
		IntervalAttribute<QString> m_formulas;
		//! The plot designation
		SciDAVis::PlotDesignation m_plot_designation;
		//! The owner column
		Column * m_owner;
		//@}
		
};

#endif
