/***************************************************************************
    File                 : Column.h
    Project              : SciDAVis
    Description          : Aspect that manages a column
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

#ifndef COLUMN_H
#define COLUMN_H

#include "core/AbstractAspect.h"
#include "core/AbstractSimpleFilter.h"
#include "lib/IntervalAttribute.h"
#include "lib/XmlStreamReader.h"
class QString;

class ColumnStringIO;

//! Aspect that manages a column
/**
  This class represents a column, i.e., (mathematically) a 1D vector of 
  values with a header. It provides a public reading and (undo aware) writing 
  interface as defined in AbstractColumn. It manages special attributes
  of column rows such as masking and a validity flag. A column
  can have one of currently three data types: double, QString, or
  QDateTime. The string representation of the values can differ depending
  on the mode of the column.

  Column inherits from AbstractAspect and is intended to be a child
  of the corresponding Table in the aspect hierarchy. Columns don't
  have a view as they are intended to be displayed inside a table.
 */
class Column : public AbstractColumn
{
	Q_OBJECT

	public:
		class Private;
		friend class Private;
		//! Ctor
		/**
		 * \param name the column name (= aspect name)
		 * \param mode initial column mode
		 */
		Column(const QString& name, SciDAVis::ColumnMode mode);
		//! Ctor
		/**
		 * \param name the column name (= aspect name)
		 * \param data initial data vector
		 * \param validity a list of invalid intervals (optional)
		 */
		Column(const QString& name, QVector<double> data, IntervalAttribute<bool> validity = IntervalAttribute<bool>());
		//! Ctor
		/**
		 * \param name the column name (= aspect name)
		 * \param data initial data vector
		 * \param validity a list of invalid intervals (optional)
		 */
		Column(const QString& name, QStringList data, IntervalAttribute<bool> validity = IntervalAttribute<bool>()); 
		//! Ctor
		/**
		 * \param name the column name (= aspect name)
		 * \param data initial data vector
		 * \param validity a list of invalid intervals (optional)
		 */
		Column(const QString& name, QList<QDateTime> data, IntervalAttribute<bool> validity = IntervalAttribute<bool>());
		//! Dtor
		~Column();

		//! \name aspect related functions
		//@{
		//! Return an icon to be used for decorating the views and table column headers
		virtual QIcon icon() const;
		//@}

		//! Return the data type of the column
		SciDAVis::ColumnDataType dataType() const;
		//! Return whether the object is read-only
		bool isReadOnly() const { return false; };
		//! Return the column mode
		/**
		 * This function is most used by tables but can also be used
		 * by plots. The column mode specifies how to interpret 
		 * the values in the column additional to the data type.
		 */ 
		SciDAVis::ColumnMode columnMode() const;
		//! Set the column mode
		/**
		 * This sets the column mode and, if
		 * necessary, converts it to another datatype.
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
		//! Return the data vector size
		/**
		 * This returns the number of rows that actually contain data. 
		 * Rows beyond this can be masked etc. but should be ignored by filters,
		 * plots etc.
		 */
		int rowCount() const;
		//! Insert some empty (or initialized with zero) rows
		void insertRows(int before, int count);
		//! Remove 'count' rows starting from row 'first'
		void removeRows(int first, int count);
		//! Return the column plot designation
		SciDAVis::PlotDesignation plotDesignation() const;
		//! Set the column plot designation
		void setPlotDesignation(SciDAVis::PlotDesignation pd);
		//! Clear the whole column
		void clear();
		//! This must be called before the column is replaced by another
		void notifyReplacement(const AbstractColumn* replacement);
		//! Return the output filter (for data type -> string  conversion)
		/**
		 * This method is mainly used to get a filter that can convert
		 * the column's data type to strings (usualy to display in a view).
		 */
		AbstractSimpleFilter * outputFilter() const;
		//! Return a wrapper column object used for String I/O.
		ColumnStringIO * asStringColumn() const { return m_string_io; }

		//! \name IntervalAttribute related functions
		//@{
		//! Return whether a certain row contains an invalid value 	 
		bool isInvalid(int row) const;
		//! Return whether a certain interval of rows contains only invalid values 	 
		bool isInvalid(Interval<int> i) const;
		//! Return all intervals of invalid rows
		QList< Interval<int> > invalidIntervals() const;
		//! Return whether a certain row is masked 	 
		bool isMasked(int row) const;
		//! Return whether a certain interval of rows rows is fully masked 	 
		bool isMasked(Interval<int> i) const;
		//! Return all intervals of masked rows
		QList< Interval<int> > maskedIntervals() const;
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
		QString formula(int row) const;
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
		QList< Interval<int> > formulaIntervals() const;
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
		virtual void replaceValues(int first, const QVector<double>& new_values);
		//@}

		//! \name XML related functions
		//@{
		//! Save the column as XML
		void save(QXmlStreamWriter * writer) const;
		//! Load the column from XML
		bool load(XmlStreamReader * reader);
	private:
		//! Read XML input filter element
		bool XmlReadInputFilter(XmlStreamReader * reader);
		//! Read XML output filter element
		bool XmlReadOutputFilter(XmlStreamReader * reader);
		//! Read XML mask element
		bool XmlReadMask(XmlStreamReader * reader);
		//! Read XML formula element
		bool XmlReadFormula(XmlStreamReader * reader);
		//! Read XML row element
		bool XmlReadRow(XmlStreamReader * reader);
		//@}

	private slots:
		void notifyDisplayChange();

	private:
		//! Pointer to the private data object
		Private * m_column_private;
		ColumnStringIO * m_string_io;
		
		friend class ColumnStringIO;
};

//! String-IO interface of Column.
class ColumnStringIO : public AbstractColumn
{
	Q_OBJECT
	
	public:
		ColumnStringIO(Column * owner) : AbstractColumn(tr("as string")), m_owner(owner), m_setting(false) {}
		virtual SciDAVis::ColumnMode columnMode() const { return SciDAVis::Text; }
		virtual SciDAVis::ColumnDataType dataType() const { return SciDAVis::TypeQString; }
		virtual SciDAVis::PlotDesignation plotDesignation() const { return m_owner->plotDesignation(); }
		virtual int rowCount() const { return m_owner->rowCount(); }
		virtual QString textAt(int row) const;
		virtual void setTextAt(int row, const QString &value);
		virtual bool isInvalid(int row) const {
			if (m_setting)
				return false;
			else
				return m_owner->isInvalid(row);
		}

	private:
		Column * m_owner;
		bool m_setting;
		QString m_to_set;
};

#endif
