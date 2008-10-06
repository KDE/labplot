/***************************************************************************
    File                 : SimpleMappingFilter.h
    Project              : SciDAVis
    --------------------------------------------------------------------
    Copyright            : (C) 2007 by Tilman Benkert
    Email                : thzs@gmx.net
    Description          : Filter that maps rows indices of a column
                           
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
   
   
#ifndef SIMPLEMAPPINGFILTER_H
#define SIMPLEMAPPINGFILTER_H

#include "SimpleCopyThroughFilter.h"
   
//! Filter that maps rows indices of a column
/**
 * The filter provides a 1:1 row mapping. This can
 * for example be used to let a non contiguous selection
 * in a column appear contiguous.
 *
 * The mappings are stored in two integer lists. For
 * each index j in m_source_rows and m_dest_rows the
 * the row number m_source_rows.at(j) in the
 * input column will appear as row m_dest_rows.at(j)
 * in the output column.
 */
class SimpleMappingFilter : public SimpleCopyThroughFilter
{
	Q_OBJECT

	public:
		//! Map one row to another index
		/**
		 * This filter only supports 1:1 mapping. Adding a mapping
		 * will remove all previous mappings involving the given 
		 * source and destination rows.
		 */
		void addMapping(int src_row, int dest_row);
		//! Remove a mapping to the given output row
		void removeMappingTo(int dest_row);
		//! Remove a mapping from the given input row
		void removeMappingFrom(int src_row);
		//! Remove all mappings
		void clearMappings();
		//! Return whether the object is read-only
		virtual bool isReadOnly() const;
		//! Set the column mode
		/**
		 * This sets the column mode and, if
		 * necessary, converts it to another datatype.
		 */
		virtual void setColumnMode(SciDAVis::ColumnMode mode);
		//! Copy another column of the same type
		/**
		 * This function will return false if the data type
		 * of 'other' is not the same as the type of 'this'.
		 * The validity information for the rows is also copied.
		 * Use a filter to convert a column to another type.
		 */
		virtual bool copy(const AbstractColumn * other);
		//! Copies part of another column of the same type
		/**
		 * This function will return false if the data type
		 * of 'other' is not the same as the type of 'this'.
		 * The validity information for the rows is also copied.
		 * \param other pointer to the column to copy
		 * \param src_start first row to copy in the column to copy
		 * \param dest_start first row to copy in
		 * \param num_rows the number of rows to copy
		 */ 
		virtual bool copy(const AbstractColumn * source, int source_start, int dest_start, int num_rows);
		//! Return the number of mapped rows
		virtual int rowCount() const;
		//! Insert some empty (or initialized with zero) rows
		virtual void insertRows(int before, int count);
		//! Remove 'count' rows starting from row 'first'
		virtual void removeRows(int first, int count);
		//! Set the column plot designation
		virtual void setPlotDesignation(SciDAVis::PlotDesignation pd);
		//! Clear the whole column
		virtual void clear();

		//! \name IntervalAttribute related functions
		//@{
		//! Return whether a certain row contains an invalid value 	 
		virtual bool isInvalid(int row) const; 	 
		//! Return whether a certain interval of rows contains only invalid values 	 
		virtual bool isInvalid(Interval<int> i) const;
		//! Return all intervals of invalid rows
		virtual QList< Interval<int> > invalidIntervals() const;
		//! Return whether a certain row is masked 	 
		virtual bool isMasked(int row) const;	 
		//! Return whether a certain interval of rows rows is fully masked 	 
		virtual bool isMasked(Interval<int> i) const;
		//! Return all intervals of masked rows
		virtual QList< Interval<int> > maskedIntervals() const;
		//! Clear all validity information
		virtual void clearValidity();
		//! Clear all masking information
		virtual void clearMasks();
		//! Set an interval invalid or valid
		/**
		 * \param i the interval
		 * \param invalid true: set invalid, false: set valid
		 */ 
		virtual void setInvalid(Interval<int> i, bool invalid = true);
		//! Overloaded function for convenience
		virtual void setInvalid(int row, bool invalid = true);
		//! Set an interval masked
		/**
		 * \param i the interval
		 * \param mask true: mask, false: unmask
		 */ 
		virtual void setMasked(Interval<int> i, bool mask = true);
		//! Overloaded function for convenience
		virtual void setMasked(int row, bool mask = true);
		//@}

		//! \name Formula related functions
		//@{
		//! Return the formula associated with row 'row' 	 
		virtual QString formula(int row) const; 
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
		virtual QList< Interval<int> > formulaIntervals() const;
		//! Set a formula string for an interval of rows
		virtual void setFormula(Interval<int> i, QString formula);
		//! Overloaded function for convenience
		virtual void setFormula(int row, QString formula);
		//! Clear all formulas
		virtual void clearFormulas();
		//@}
		
		//! \name type specific functions
		//@{
		//! Return the content of row 'row'.
		/**
		 * Use this only when dataType() is QString
		 */
		virtual QString textAt(int row) const;
		//! Set the content of row 'row'
		/**
		 * Use this only when dataType() is QString
		 */
		virtual void setTextAt(int row, const QString& new_value);
		//! Replace a range of values 
		/**
		 * Use this only when dataType() is QString
		 */
		virtual void replaceTexts(int first, const QStringList& new_values);
		//! Return the date part of row 'row'
		/**
		 * Use this only when dataType() is QDateTime
		 */
		virtual QDate dateAt(int row) const;
		//! Set the content of row 'row'
		/**
		 * Use this only when dataType() is QDateTime
		 */
		virtual void setDateAt(int row, const QDate& new_value);
		//! Return the time part of row 'row'
		/**
		 * Use this only when dataType() is QDateTime
		 */
		virtual QTime timeAt(int row) const;
		//! Set the content of row 'row'
		/**
		 * Use this only when dataType() is QDateTime
		 */
		virtual void setTimeAt(int row, const QTime& new_value);
		//! Return the QDateTime in row 'row'
		/**
		 * Use this only when dataType() is QDateTime
		 */
		virtual QDateTime dateTimeAt(int row) const;
		//! Set the content of row 'row'
		/**
		 * Use this only when dataType() is QDateTime
		 */
		virtual void setDateTimeAt(int row, const QDateTime& new_value);
		//! Replace a range of values 
		/**
		 * Use this only when dataType() is QDateTime
		 */
		virtual void replaceDateTimes(int first, const QList<QDateTime>& new_values);
		//! Return the double value in row 'row'
		/**
		 * Use this only when dataType() is double
		 */
		virtual double valueAt(int row) const;
		//! Set the content of row 'row'
		/**
		 * Use this only when dataType() is double
		 */
		virtual void setValueAt(int row, double new_value);
		//! Replace a range of values 
		/**
		 * Use this only when dataType() is double
		 */
		virtual void replaceValues(int first, const QVector<double>& new_values);
		//@}

		//! \name signal handlers
		//@{
		virtual void inputRowsAboutToBeInserted(AbstractColumn * source, int before, int count);
		virtual void inputRowsInserted(AbstractColumn * source, int before, int count);
		virtual void inputRowsAboutToBeRemoved(AbstractColumn * source, int first, int count);
		virtual void inputRowsRemoved(AbstractColumn * source, int first, int count);
		//@}

		//! \name XML related functions
		//@{
		//! Save the column as XML
		virtual void save(QXmlStreamWriter * writer) const;
		//! Load the column from XML
		virtual bool load(QXmlStreamReader * reader);
		//@}

	private:
		QList<int> m_source_rows;
		QList<int> m_dest_rows;
};

#endif // #ifndef SIMPLEMAPPINGFILTER_H
