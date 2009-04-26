/***************************************************************************
    File                 : AbstractColumn.cpp
    Project              : SciDAVis
    Description          : Interface definition for data with column logic
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

#include "AbstractColumn.h"

#include <QtCore/QString>
#include <QtCore/QDateTime>
#include <QtCore/QDate>
#include <QtCore/QTime>
#include <math.h>

/**
 * \class AbstractColumn
 * \brief Interface definition for data with column logic
 *
 * This is an abstract base class for column-based data, 
 * i.e. mathematically a vector or technically a 1D array or list.
 * It only defines the interface but has no data members itself. 
 *
 * Classes derived from this are typically table columns or outputs
 * of filters which can be chained between table columns and plots. 
 * From the point of view of the plot functions there will be no difference 
 * between a table column and a filter output since both use this interface.
 *
 * Classes derived from this will either store a 
 * vector with entries of one certain data type, e.g. double, QString, 
 * QDateTime, or generate such values on demand. To determine the data
 * type of a class derived from this, use the columnMode() function. 
 * AbstractColumn defines all access functions for all supported data 
 * types but only those corresponding to the return value of columnMode() 
 * will return a meaningful value. Calling functions not belonging to 
 * the data type of the column is safe, but will do nothing (writing
 * function) or return some default value (reading functions).
 *
 * This class also defines all signals which indicate a data change.
 * Any class whose output values are subject to change over time must emit
 * the according signals. These signals notify any object working with the
 * column before and after a change of the column.
 * In some cases it will be necessary for a class using 
 * the column to connect aboutToBeDestroyed(), to react 
 * to a column's deletion, e.g. a filter's reaction to a 
 * table deletion.
 *
 * All writing functions have a "do nothing" standard implementation to
 * make deriving a read-only class very easy without bothering about the
 * writing interface. 
 */

/**
 * \fn bool AbstractColumn::isReadOnly() const
 * \brief Return whether the object is read-only
 */

/**
 * \fn SciDAVis::ColumnMode AbstractColumn::columnMode() const
 * \brief Return the column mode
 *
 * This function is most used by tables but can also be used
 * by plots. The column mode specifies how to interpret 
 * the values in the column additional to the data type.
 */

/**
 * \brief Set the column mode
 *
 * This sets the column mode and, if
 * necessary, converts it to another datatype.
 */
void AbstractColumn::setColumnMode(SciDAVis::ColumnMode) {}

/**
 * \brief Copy another column of the same type
 *
 * This function will return false if the data type
 * of 'other' is not the same as the type of 'this'.
 * The validity information for the rows is also copied.
 * Use a filter to convert a column to another type.
 */
bool AbstractColumn::copy(const AbstractColumn *other) {
	Q_UNUSED(other)
	return false;
}

/**
 * \brief Copies part of another column of the same type
 *
 * This function will return false if the data type
 * of 'other' is not the same as the type of 'this'.
 * The validity information for the rows is also copied.
 * \param source pointer to the column to copy
 * \param source_start first row to copy in the column to copy
 * \param destination_start first row to copy in
 * \param num_rows the number of rows to copy
 */ 
bool AbstractColumn::copy(const AbstractColumn *source, int source_start, int destination_start, int num_rows) {
	Q_UNUSED(source)
	Q_UNUSED(source_start)
	Q_UNUSED(destination_start)
	Q_UNUSED(num_rows)
	return false;
}

/**
 * \fn int AbstractColumn::rowCount() const
 * \brief Return the data vector size
 */

/**
 * \brief Insert some empty (or initialized with zero) rows
 */
void AbstractColumn::insertRows(int before, int count) {
	Q_UNUSED(before) Q_UNUSED(count)
}

/**
 * \brief Remove 'count' rows starting from row 'first'
 */
void AbstractColumn::removeRows(int first, int count) {
	Q_UNUSED(first) Q_UNUSED(count)
}

/**
 * \fn SciDAVis::PlotDesignation AbstractColumn::plotDesignation() const
 * \brief Return the column plot designation
 */

/**
 * \brief Set the column plot designation
 */
void AbstractColumn::setPlotDesignation(SciDAVis::PlotDesignation pd) {
	Q_UNUSED(pd)
}

/**
 * \brief Clear the whole column
 */
void AbstractColumn::clear() {}

////////////////////////////////////////////////////////////////////////////////////////////////////
//! \name IntervalAttribute related functions
//@{
////////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * \brief Return whether a certain row contains an invalid value
 */
bool AbstractColumn::isInvalid(int row) const {
	return !Interval<int>(0, rowCount()-1).contains(row);
}

/**
 * \brief Return whether a certain interval of rows contains only invalid values
 */
bool AbstractColumn::isInvalid(Interval<int> i) const {
	return !Interval<int>(0, rowCount()-1).contains(i);
}

/**
 * \brief Return all intervals of invalid rows
 */
QList< Interval<int> > AbstractColumn::invalidIntervals() const {
	return QList< Interval<int> >();
}

/**
 * \brief Return whether a certain row is masked 	 
 */
bool AbstractColumn::isMasked(int row) const {
	Q_UNUSED(row);
	return false;
}

/**
 * \brief Return whether a certain interval of rows rows is fully masked 	 
 */
bool AbstractColumn::isMasked(Interval<int> i) const {
	Q_UNUSED(i);
	return false;
}

/**
 * \brief Return all intervals of masked rows
 */
QList< Interval<int> > AbstractColumn::maskedIntervals() const {
	return QList< Interval<int> >();
}

/**
 * \brief Clear all validity information
 */
void AbstractColumn::clearValidity() {};

/**
 * \brief Clear all masking information
 */
void AbstractColumn::clearMasks() {};

/**
 * \brief Set an interval invalid or valid
 *
 * \param i the interval
 * \param invalid true: set invalid, false: set valid
 */ 
void AbstractColumn::setInvalid(Interval<int> i, bool invalid) {
	Q_UNUSED(i)
	Q_UNUSED(invalid)
};

/**
 * \brief Overloaded function for convenience
 */
void AbstractColumn::setInvalid(int row, bool invalid) {
	Q_UNUSED(row) Q_UNUSED(invalid)
}

/**
 * \brief Set an interval masked
 *
 * \param i the interval
 * \param mask true: mask, false: unmask
 */ 
void AbstractColumn::setMasked(Interval<int> i, bool mask) {
	Q_UNUSED(i) Q_UNUSED(mask)
}

/**
 * \brief Overloaded function for convenience
 */
void AbstractColumn::setMasked(int row, bool mask) {
	Q_UNUSED(row) Q_UNUSED(mask)
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//@}
////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////
//! \name Formula related functions
//@{
////////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * \brief Return the formula associated with row 'row'
 */
QString AbstractColumn::formula(int row) const {
	Q_UNUSED(row);
	return QString();
}

/**
 * \brief Return the intervals that have associated formulas
 *
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
QList< Interval<int> > AbstractColumn::formulaIntervals() const {
	return QList< Interval<int> >();
}

/**
 * \brief Set a formula string for an interval of rows
 */
void AbstractColumn::setFormula(Interval<int> i, QString formula) {
	Q_UNUSED(i) Q_UNUSED(formula)
}

/**
 * \brief Overloaded function for convenience
 */
void AbstractColumn::setFormula(int row, QString formula) {
	Q_UNUSED(row) Q_UNUSED(formula)
}

/**
 * \brief Clear all formulas
 */
void AbstractColumn::clearFormulas() {};

////////////////////////////////////////////////////////////////////////////////////////////////////
//@}
////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////
//! \name type specific functions
//@{
////////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * \brief Return the content of row 'row'.
 *
 * Use this only when columnMode() is Text
 */
QString AbstractColumn::textAt(int row) const {
	Q_UNUSED(row);
	return "";
}

/**
 * \brief Set the content of row 'row'
 *
 * Use this only when columnMode() is Text
 */
void AbstractColumn::setTextAt(int row, const QString& new_value) {
	Q_UNUSED(row) Q_UNUSED(new_value) 
}

/**
 * \brief Replace a range of values 
 *
 * Use this only when columnMode() is Text
 */
void AbstractColumn::replaceTexts(int first, const QStringList& new_values) {
	Q_UNUSED(first) Q_UNUSED(new_values)
};

/**
 * \brief Return the date part of row 'row'
 *
 * Use this only when columnMode() is DateTime, Month or Day
 */
QDate AbstractColumn::dateAt(int row) const {
	Q_UNUSED(row);
	return QDate();
}

/**
 * \brief Set the content of row 'row'
 *
 * Use this only when columnMode() is DateTime, Month or Day
 */
void AbstractColumn::setDateAt(int row, const QDate& new_value) {
	Q_UNUSED(row) Q_UNUSED(new_value)
};

/**
 * \brief Return the time part of row 'row'
 *
 * Use this only when columnMode() is DateTime, Month or Day
 */
QTime AbstractColumn::timeAt(int row) const {
	Q_UNUSED(row);
	return QTime();
}

/**
 * \brief Set the content of row 'row'
 *
 * Use this only when columnMode() is DateTime, Month or Day
 */
void AbstractColumn::setTimeAt(int row, const QTime& new_value) {
	Q_UNUSED(row) Q_UNUSED(new_value)
}

/**
 * \brief Return the QDateTime in row 'row'
 *
 * Use this only when columnMode() is DateTime, Month or Day
 */
QDateTime AbstractColumn::dateTimeAt(int row) const {
	Q_UNUSED(row);
	return QDateTime();
}

/**
 * \brief Set the content of row 'row'
 *
 * Use this only when columnMode() is DateTime, Month or Day
 */
void AbstractColumn::setDateTimeAt(int row, const QDateTime& new_value) {
	Q_UNUSED(row) Q_UNUSED(new_value)
};

/**
 * \brief Replace a range of values 
 *
 * Use this only when columnMode() is DateTime, Month or Day
 */
void AbstractColumn::replaceDateTimes(int first, const QList<QDateTime>& new_values) {
	Q_UNUSED(first) Q_UNUSED(new_values)
};

/**
 * \brief Return the double value in row 'row'
 *
 * Use this only when columnMode() is Numeric
 */
double AbstractColumn::valueAt(int row) const {
	Q_UNUSED(row);
	return NAN;
}

/**
 * \brief Set the content of row 'row'
 *
 * Use this only when columnMode() is Numeric
 */
void AbstractColumn::setValueAt(int row, double new_value) {
	Q_UNUSED(row) Q_UNUSED(new_value)
};

/**
 * \brief Replace a range of values 
 *
 * Use this only when columnMode() is Numeric
 */
void AbstractColumn::replaceValues(int first, const QVector<double>& new_values) {
	Q_UNUSED(first) Q_UNUSED(new_values)
};

////////////////////////////////////////////////////////////////////////////////////////////////////
//@}
////////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * \fn void AbstractColumn::plotDesignationAboutToChange(const AbstractColumn *source)
 * \brief Column plot designation will be changed
 *
 * 'source' is always the this pointer of the column that
 * emitted this signal. This way it's easier to use
 * one handler for lots of columns.
 */

/**
 * \fn void AbstractColumn::plotDesignationChanged(const AbstractColumn *source)
 * \brief Column plot designation changed
 *
 * 'source' is always the this pointer of the column that
 * emitted this signal. This way it's easier to use
 * one handler for lots of columns.
 */

/**
 * \fn void AbstractColumn::modeAboutToChange(const AbstractColumn *source)
 * \brief Column mode (possibly also the data type) will be changed
 *
 * 'source' is always the this pointer of the column that
 * emitted this signal. This way it's easier to use
 * one handler for lots of columns.
 */

/**
 * \fn void AbstractColumn::modeChanged(const AbstractColumn *source)
 * \brief Column mode (possibly also the data type) changed
 *
 * 'source' is always the this pointer of the column that
 * emitted this signal. This way it's easier to use
 * one handler for lots of columns.
 */

/**
 * \fn void AbstractColumn::dataAboutToChange(const AbstractColumn *source)
 * \brief Data (including validity) of the column will be changed
 *
 * 'source' is always the this pointer of the column that
 * emitted this signal. This way it's easier to use
 * one handler for lots of columns.
 */

/**
 * \fn void AbstractColumn::dataChanged(const AbstractColumn *source)
 * \brief Data (including validity) of the column has changed
 *
 * Important: When data has changed also the number
 * of rows in the column may have changed without
 * any other signal emission.
 * 'source' is always the this pointer of the column that
 * emitted this signal. This way it's easier to use
 * one handler for lots of columns.
 */

/**
 * \fn void AbstractColumn::rowsAboutToBeInserted(const AbstractColumn *source, int before, int count)
 * \brief Rows will be inserted
 *
 *	\param source the column that emitted the signal
 *	\param before the row to insert before
 *	\param count the number of rows to be inserted
 */

/**
 * \fn void AbstractColumn::rowsInserted(const AbstractColumn *source, int before, int count)
 * \brief Rows have been inserted
 *
 *	\param source the column that emitted the signal
 *	\param before the row to insert before
 *	\param count the number of rows to be inserted
 */

/**
 * \fn void AbstractColumn::rowsAboutToBeRemoved(const AbstractColumn *source, int first, int count)
 * \brief Rows will be deleted
 *
 *	\param source the column that emitted the signal
 *	\param first the first row to be deleted
 *	\param count the number of rows to be deleted
 */

/**
 * \fn void AbstractColumn::rowsRemoved(const AbstractColumn *source, int first, int count)
 * \brief Rows have been deleted
 *
 *	\param source the column that emitted the signal
 *	\param first the first row that was deleted
 *	\param count the number of deleted rows
 */

/**
 * \fn void AbstractColumn::maskingAboutToChange(const AbstractColumn *source)
 * \brief Rows are about to be masked or unmasked
 */

/**
 * \fn void AbstractColumn::maskingChanged(const AbstractColumn *source)
 * \brief Rows have been masked or unmasked
 */

/**
 * \fn void AbstractColumn::aboutToBeDestroyed(const AbstractColumn *source)
 * \brief Emitted shortl before this column is deleted
 *
 * \param source the object emitting this signal
 *
 * This is needed by AbstractFilter. 
 */
