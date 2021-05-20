/***************************************************************************
    File                 : AbstractColumn.cpp
    Project              : LabPlot
    Description          : Interface definition for data with column logic
    --------------------------------------------------------------------
    Copyright            : (C) 2007,2008 Tilman Benkert (thzs@gmx.net)
    Copyright            : (C) 2017-2020 Stefan Gerlach (stefan.gerlach@uni.kn)

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

#include "backend/core/AbstractColumn.h"
#include "backend/core/AbstractColumnPrivate.h"
#include "backend/core/abstractcolumncommands.h"
#include "backend/lib/XmlStreamReader.h"
#include "backend/lib/SignallingUndoCommand.h"

#include <QDateTime>
#include <QIcon>
#include <KLocalizedString>

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
 * \brief Ctor
 *
 * \param name the column name (= aspect name)
 */
AbstractColumn::AbstractColumn(const QString &name, AspectType type)
	: AbstractAspect(name, type), d( new AbstractColumnPrivate(this) ) {
}

AbstractColumn::~AbstractColumn() {
	emit aboutToBeDestroyed(this);
	delete d;
}

QStringList AbstractColumn::dateFormats() {
	static const QStringList dates{"yyyy-MM-dd", "yyyy/MM/dd", "dd/MM/yyyy",
		"dd/MM/yy", "dd.MM.yyyy", "dd.MM.yy", "MM/yyyy", "dd.MM.", "yyyyMMdd"};

	return dates;
}

QStringList AbstractColumn::timeFormats() {
	static const QStringList times{"hh", "hh ap", "hh:mm", "hh:mm ap",
		"hh:mm:ss", "hh:mm:ss.zzz", "hh:mm:ss:zzz", "mm:ss.zzz", "hhmmss"};

	return times;
}

QStringList AbstractColumn::dateTimeFormats() {
	// any combination of date and times
	QStringList dateTimes = dateFormats();
	for (const auto& t : timeFormats())
		dateTimes << t;
	for (const auto& d : dateFormats())
		for (const auto& t : timeFormats())
			dateTimes << d + ' ' + t;

	return dateTimes;
}

/**
 * \brief Convenience method for getting mode name
 * not translated since not used in UI
 */
QString AbstractColumn::modeName(ColumnMode mode) {
	switch (mode) {
	case ColumnMode::Numeric:
		return QLatin1String("Numeric");
	case ColumnMode::Integer:
		return QLatin1String("Integer");
	case ColumnMode::BigInt:
		return QLatin1String("BigInt");
	case ColumnMode::Text:
		return QLatin1String("Text");
	case ColumnMode::DateTime:
		return QLatin1String("DateTime");
	case ColumnMode::Month:
		return QLatin1String("Month");
	case ColumnMode::Day:
		return QLatin1String("Day");
	}

	return QLatin1String("UNDEFINED");
}

/**
 * \brief Convenience method for mode-dependent icon
 */
QIcon AbstractColumn::modeIcon(ColumnMode mode) {
	switch (mode) {
	case ColumnMode::Numeric:
	case ColumnMode::Integer:
	case ColumnMode::BigInt:
		break;
	case ColumnMode::Text:
		return QIcon::fromTheme("draw-text");
	case ColumnMode::DateTime:
	case ColumnMode::Month:
	case ColumnMode::Day:
		return QIcon::fromTheme("chronometer");
	}

	return QIcon::fromTheme("x-shape-text");
}

/**
 * \fn bool AbstractColumn::isReadOnly() const
 * \brief Return whether the object is read-only
 */

/**
 * \fn AbstractColumn::ColumnMode AbstractColumn::columnMode() const
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
void AbstractColumn::setColumnMode(AbstractColumn::ColumnMode) {}

/**
 * \brief Copy another column of the same type
 *
 * This function will return false if the data type
 * of 'other' is not the same as the type of 'this'.
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
 * \fn int AbstractColumn::availableRowCount() const
 * \brief Return the number of available data rows
 */

/**
 * \brief Insert some empty (or initialized with invalid values) rows
 */
void AbstractColumn::insertRows(int before, int count) {
	beginMacro( i18np("%1: insert 1 row", "%1: insert %2 rows", name(), count) );
	exec(new SignallingUndoCommand("pre-signal", this, "rowsAboutToBeInserted", "rowsRemoved",
	                               Q_ARG(const AbstractColumn*,this), Q_ARG(int,before), Q_ARG(int,count)));

	handleRowInsertion(before, count);

	exec(new SignallingUndoCommand("post-signal", this, "rowsInserted", "rowsAboutToBeRemoved",
	                               Q_ARG(const AbstractColumn*,this), Q_ARG(int,before), Q_ARG(int,count)));
	endMacro();
}

void AbstractColumn::handleRowInsertion(int before, int count) {
	exec(new AbstractColumnInsertRowsCmd(this, before, count));
}

/**
 * \brief Remove 'count' rows starting from row 'first'
 */
void AbstractColumn::removeRows(int first, int count) {
	beginMacro( i18np("%1: remove 1 row", "%1: remove %2 rows", name(), count) );
	exec(new SignallingUndoCommand("change signal", this, "rowsAboutToBeRemoved", "rowsInserted",
	                               Q_ARG(const AbstractColumn*,this), Q_ARG(int,first), Q_ARG(int,count)));

	handleRowRemoval(first, count);

	exec(new SignallingUndoCommand("change signal", this, "rowsRemoved", "rowsAboutToBeInserted",
	                               Q_ARG(const AbstractColumn*,this), Q_ARG(int,first), Q_ARG(int,count)));
	endMacro();
}

void AbstractColumn::handleRowRemoval(int first, int count) {
	exec(new AbstractColumnRemoveRowsCmd(this, first, count));
}

/**
 * \fn AbstractColumn::PlotDesignation AbstractColumn::plotDesignation() const
 * \brief Return the column plot designation
 */

/**
 * \brief Set the column plot designation
 */
void AbstractColumn::setPlotDesignation(AbstractColumn::PlotDesignation pd) {
	Q_UNUSED(pd)
}

bool AbstractColumn::isNumeric() const {
	const auto mode = columnMode();
	return (mode == ColumnMode::Numeric || mode == ColumnMode::Integer || mode == ColumnMode::BigInt);
}

bool AbstractColumn::isPlottable() const {
	const auto mode = columnMode();
	return (mode == ColumnMode::Numeric || mode == ColumnMode::Integer || mode == ColumnMode::BigInt || mode == ColumnMode::DateTime);
}

/**
 * \brief Clear the whole column
 */
void AbstractColumn::clear() {}

/**
 * \brief Convenience method for mode-independent testing of validity
 */
bool AbstractColumn::isValid(int row) const {
	switch (columnMode()) {
	case ColumnMode::Numeric:
		return !(std::isnan(valueAt(row)) || std::isinf(valueAt(row)));
	case ColumnMode::Integer:	// there is no invalid integer
	case ColumnMode::BigInt:
		return true;
	case ColumnMode::Text:
		return !textAt(row).isNull();
	case ColumnMode::DateTime:
	case ColumnMode::Month:
	case ColumnMode::Day:
		return dateTimeAt(row).isValid();
	}

	return false;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//! \name IntervalAttribute related functions
//@{
////////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * \brief Return whether a certain row is masked
 */
bool AbstractColumn::isMasked(int row) const {
	return d->m_masking.isSet(row);
}

/**
 * \brief Return whether a certain interval of rows is fully masked
 */
bool AbstractColumn::isMasked(const Interval<int>& i) const {
	return d->m_masking.isSet(i);
}

/**
 * \brief Return all intervals of masked rows
 */
QVector< Interval<int> > AbstractColumn::maskedIntervals() const {
	return d->m_masking.intervals();
}

/**
 * \brief Clear all masking information
 */
void AbstractColumn::clearMasks() {
	exec(new AbstractColumnClearMasksCmd(d),
	     "maskingAboutToChange", "maskingChanged", Q_ARG(const AbstractColumn*,this));
}

/**
 * \brief Set an interval masked
 *
 * \param i the interval
 * \param mask true: mask, false: unmask
 */
void AbstractColumn::setMasked(const Interval<int>& i, bool mask) {
	exec(new AbstractColumnSetMaskedCmd(d, i, mask),
	     "maskingAboutToChange", "maskingChanged", Q_ARG(const AbstractColumn*,this));
}

/**
 * \brief Overloaded function for convenience
 */
void AbstractColumn::setMasked(int row, bool mask) {
	setMasked(Interval<int>(row,row), mask);
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
 * QVector< Interval<int> > intervals = my_column.formulaIntervals();
 * foreach(Interval<int> interval, intervals)
 * 	list << QString(interval.toString() + ": " + my_column.formula(interval.start()));
 * \endcode
 */
QVector< Interval<int> > AbstractColumn::formulaIntervals() const {
	return QVector< Interval<int> >();
}

/**
 * \brief Set a formula string for an interval of rows
 */
void AbstractColumn::setFormula(const Interval<int>& i, const QString& formula) {
	Q_UNUSED(i) Q_UNUSED(formula)
}

/**
 * \brief Overloaded function for convenience
 */
void AbstractColumn::setFormula(int row, const QString& formula) {
	Q_UNUSED(row) Q_UNUSED(formula)
}

/**
 * \brief Clear all formulas
 */
void AbstractColumn::clearFormulas() {};


//conditional formatting
bool AbstractColumn::hasHeatmapFormat() const {
	return (d->m_heatmapFormat != nullptr);
}

AbstractColumn::HeatmapFormat& AbstractColumn::heatmapFormat() const {
	if (!d->m_heatmapFormat)
		d->m_heatmapFormat = new HeatmapFormat();

	return *(d->m_heatmapFormat);
}

void AbstractColumn::setHeatmapFormat(const AbstractColumn::HeatmapFormat& format) {
	if (!d->m_heatmapFormat)
		d->m_heatmapFormat = new HeatmapFormat();

	*(d->m_heatmapFormat) = format;
}

void AbstractColumn::removeFormat() {
	delete d->m_heatmapFormat;
	d->m_heatmapFormat = nullptr;
}

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
	return QString();
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
void AbstractColumn::replaceTexts(int first, const QVector<QString>& new_values) {
	Q_UNUSED(first) Q_UNUSED(new_values)
};

/**
 * \brief Return the date part of row 'row'
 *
 * Use this only when columnMode() is DateTime, Month or Day
 */
QDate AbstractColumn::dateAt(int row) const {
	Q_UNUSED(row);
	return QDate{};
}

/**
 * \brief Set the content of row 'row'
 *
 * Use this only when columnMode() is DateTime, Month or Day
 */
void AbstractColumn::setDateAt(int row, QDate new_value) {
	Q_UNUSED(row) Q_UNUSED(new_value)
};

/**
 * \brief Return the time part of row 'row'
 *
 * Use this only when columnMode() is DateTime, Month or Day
 */
QTime AbstractColumn::timeAt(int row) const {
	Q_UNUSED(row);
	return QTime{};
}

/**
 * \brief Set the content of row 'row'
 *
 * Use this only when columnMode() is DateTime, Month or Day
 */
void AbstractColumn::setTimeAt(int row, QTime new_value) {
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
void AbstractColumn::replaceDateTimes(int first, const QVector<QDateTime>& new_values) {
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
void AbstractColumn::setValueAt(int row, const double new_value) {
	Q_UNUSED(row) Q_UNUSED(new_value)
};

/**
 * \brief Replace a range of values
 *
 * Use this only when columnMode() is Numeric
 */
void AbstractColumn::replaceValues(int first, const QVector<double>& new_values) {
	Q_UNUSED(first) Q_UNUSED(new_values)
}

/**
 * \brief Return the integer value in row 'row'
 *
 * Use this only when columnMode() is Integer
 */
int AbstractColumn::integerAt(int row) const {
	Q_UNUSED(row);
	return 42;
}

/**
 * \brief Set the content of row 'row'
 *
 * Use this only when columnMode() is Integer
 */
void AbstractColumn::setIntegerAt(int row, const int new_value) {
	Q_UNUSED(row) Q_UNUSED(new_value)
};

/**
 * \brief Replace a range of values
 *
 * Use this only when columnMode() is Integer
 */
void AbstractColumn::replaceInteger(int first, const QVector<int>& new_values) {
	Q_UNUSED(first) Q_UNUSED(new_values)
}

/**
 * \brief Return the bigint value in row 'row'
 *
 * Use this only when columnMode() is BigInt
 */
qint64 AbstractColumn::bigIntAt(int row) const {
	Q_UNUSED(row);
	return 42;
}

/**
 * \brief Set the content of row 'row'
 *
 * Use this only when columnMode() is BigInt
 */
void AbstractColumn::setBigIntAt(int row, const qint64 new_value) {
	Q_UNUSED(row) Q_UNUSED(new_value)
};

/**
 * \brief Replace a range of values
 *
 * Use this only when columnMode() is BigInt
 */
void AbstractColumn::replaceBigInt(int first, const QVector<qint64>& new_values) {
	Q_UNUSED(first) Q_UNUSED(new_values)
}

/**
 * Returns the properties hold by this column (no, constant, monotonic increasing, monotonic decreasing,...)
 * Is used in XYCurve to improve the search velocity for the y value for a specific x value
 */
AbstractColumn::Properties AbstractColumn::properties() const {
	return AbstractColumn::Properties::No;
}

/**********************************************************************/
double AbstractColumn::minimum(int count) const {
	Q_UNUSED(count);
	return -INFINITY;
}

double AbstractColumn::minimum(int startIndex, int endIndex) const {
	Q_UNUSED(startIndex);
	Q_UNUSED(endIndex);
	return -INFINITY;
}

double AbstractColumn::maximum(int count) const {
	Q_UNUSED(count);
	return INFINITY;
}

double AbstractColumn::maximum(int startIndex, int endIndex) const {
	Q_UNUSED(startIndex);
	Q_UNUSED(endIndex);
	return INFINITY;
}

bool AbstractColumn::indicesMinMax(double v1, double v2, int& start, int& end)  const {
	Q_UNUSED(v1)
	Q_UNUSED(v2)
	Q_UNUSED(start)
	Q_UNUSED(end)
	return false;
}

int AbstractColumn::indexForValue(double x) const {
	Q_UNUSED(x)
	return 0;
}

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
 * \brief Data of the column will be changed
 *
 * 'source' is always the this pointer of the column that
 * emitted this signal. This way it's easier to use
 * one handler for lots of columns.
 */

/**
 * \fn void AbstractColumn::dataChanged(const AbstractColumn *source)
 * \brief Data of the column has changed
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

/**
 * \brief Read XML mask element
 */
bool AbstractColumn::XmlReadMask(XmlStreamReader *reader) {
	Q_ASSERT(reader->isStartElement() && reader->name() == "mask");

	bool ok1, ok2;
	int start, end;
	start = reader->readAttributeInt("start_row", &ok1);
	end = reader->readAttributeInt("end_row", &ok2);
	if (!ok1 || !ok2) {
		reader->raiseError(i18n("invalid or missing start or end row"));
		return false;
	}
	setMasked(Interval<int>(start,end));
	if (!reader->skipToEndElement()) return false;

	return true;
}

/**
 * \brief Write XML mask element
 */
void AbstractColumn::XmlWriteMask(QXmlStreamWriter *writer) const {
	for (const auto& interval : maskedIntervals()) {
		writer->writeStartElement("mask");
		writer->writeAttribute("start_row", QString::number(interval.start()));
		writer->writeAttribute("end_row", QString::number(interval.end()));
		writer->writeEndElement();
	}
}
