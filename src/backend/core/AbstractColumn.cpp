/*
	File                 : AbstractColumn.cpp
	Project              : LabPlot
	Description          : Interface definition for data with column logic
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2007, 2008 Tilman Benkert <thzs@gmx.net>
	SPDX-FileCopyrightText: 2017-2022 Stefan Gerlach <stefan.gerlach@uni.kn>
	SPDX-FileCopyrightText: 2012-2025 Alexander Semke <alexander.semke@web.de>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "backend/core/AbstractColumn.h"
#include "backend/core/AbstractColumnPrivate.h"
#include "backend/core/Settings.h"
#include "backend/core/abstractcolumncommands.h"
#include "backend/lib/XmlStreamReader.h"

#include <KLocalizedString>

#include <QDateTime>
#include <QIcon>

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
AbstractColumn::AbstractColumn(const QString& name, AspectType type)
	: AbstractAspect(name, type)
	, d(new AbstractColumnPrivate(this)) {
}

AbstractColumn::~AbstractColumn() {
	Q_EMIT aboutToBeDestroyed(this);
	delete d->m_heatmapFormat;
	delete d;
}

QStringList AbstractColumn::dateFormats() {
	static const QStringList dates{QStringLiteral("yyyy-MM-dd"),
								   QStringLiteral("yyyy.MM.dd"),
								   QStringLiteral("yyyy/MM/dd"),
								   QStringLiteral("yyyyMMdd"),
								   QStringLiteral("dd-MM-yyyy"),
								   QStringLiteral("dd.MM.yyyy"),
								   QStringLiteral("dd/MM/yyyy"),
								   QStringLiteral("ddMMyyyy"),
								   QStringLiteral("dd-MM-yy"),
								   QStringLiteral("dd.MM.yy"),
								   QStringLiteral("dd/MM/yy"),
								   QStringLiteral("ddMMyy"),
								   QStringLiteral("MM-yyyy"),
								   QStringLiteral("MM.yyyy"),
								   QStringLiteral("MM/yyyy"),
								   QStringLiteral("dd-MM"),
								   QStringLiteral("dd.MM"),
								   QStringLiteral("dd/MM"),
								   QStringLiteral("ddMM")};

	return dates;
}

QStringList AbstractColumn::timeFormats() {
	static const QStringList times{QStringLiteral("hh"),
								   QStringLiteral("hh ap"),
								   QStringLiteral("hh:mm"),
								   QStringLiteral("hh:mm ap"),
								   QStringLiteral("hh:mm:ss"),
								   QStringLiteral("hh:mm:ss.zzz"),
								   QStringLiteral("hh:mm:ss:zzz"),
								   QStringLiteral("mm:ss.zzz"),
								   QStringLiteral("hhmmss")};

	return times;
}

QStringList AbstractColumn::dateTimeFormats() {
	QStringList dateTimes = dateFormats();
	dateTimes << timeFormats();
	// any combination of date and times
	for (const auto& d : dateFormats()) {
		dateTimes << d;
		for (const auto& t : timeFormats()) {
			dateTimes << t;
			dateTimes << d + QLatin1Char(' ') + t;
		}
	}

	return dateTimes;
}

/**
 * \brief Convenience method for getting time unit string
 * translated since used in UI
 */
QString AbstractColumn::timeUnitString(TimeUnit unit) {
	switch (unit) {
	case TimeUnit::Milliseconds:
		return i18n("Milliseconds");
	case TimeUnit::Seconds:
		return i18n("Seconds");
	case TimeUnit::Minutes:
		return i18n("Minutes");
	case TimeUnit::Hours:
		return i18n("Hours");
	case TimeUnit::Days:
		return i18n("Days");
	}

	return {};
}

/**
 * \brief Convenience method for getting plot designation string
 * translated since used in UI
 */
QString AbstractColumn::plotDesignationString(PlotDesignation d, bool withBrackets) {
	QString s;

	const KConfigGroup group = Settings::group(QStringLiteral("Settings_General"));
	switch (d) {
	case PlotDesignation::NoDesignation:
		s = i18n("None");
		break;
	case PlotDesignation::X:
		s = QStringLiteral("X");
		break;
	case PlotDesignation::Y:
		s = QStringLiteral("Y");
		break;
	case PlotDesignation::Z:
		s = QStringLiteral("Z");
		break;
	case PlotDesignation::XError:
		if (group.readEntry("GUMTerms", false))
			s = i18n("X-Uncertainty");
		else
			s = i18n("X-Error");
		break;
	case PlotDesignation::XErrorPlus:
		if (group.readEntry("GUMTerms", false))
			s = i18n("X-Uncertainty +");
		else
			s = i18n("X-Error +");
		break;
	case PlotDesignation::XErrorMinus:
		if (group.readEntry("GUMTerms", false))
			s = i18n("X-Uncertainty -");
		else
			s = i18n("X-Error -");
		break;
	case PlotDesignation::YError:
		if (group.readEntry("GUMTerms", false))
			s = i18n("Y-Uncertainty");
		else
			s = i18n("Y-Error");
		break;
	case PlotDesignation::YErrorPlus:
		if (group.readEntry("GUMTerms", false))
			s = i18n("Y-Uncertainty +");
		else
			s = i18n("Y-Error +");
		break;
	case PlotDesignation::YErrorMinus:
		if (group.readEntry("GUMTerms", false))
			s = i18n("Y-Uncertainty -");
		else
			s = i18n("Y-Error -");
		break;
	default:
		return {};
	}

	if (withBrackets)
		s = QStringLiteral("[") + s + QLatin1Char(']');

	return s;
}

/**
 * \brief Convenience method for getting mode string
 * translated since used in UI
 */
QString AbstractColumn::columnModeString(ColumnMode mode) {
	switch (mode) {
	case ColumnMode::Double:
		return i18n("Double");
	case ColumnMode::Integer:
		return i18n("Integer");
	case ColumnMode::BigInt:
		return i18n("Big Integer");
	case ColumnMode::Text:
		return i18n("Text");
	case ColumnMode::DateTime:
		return i18n("Date & Time");
	case ColumnMode::Month:
		return i18n("Month Names");
	case ColumnMode::Day:
		return i18n("Day Names");
	}

	return i18n("UNDEFINED");
}

/**
 * \brief Convenience method for mode-dependent icon
 */
QIcon AbstractColumn::modeIcon(ColumnMode mode) {
	switch (mode) {
	case ColumnMode::Double:
	case ColumnMode::Integer:
	case ColumnMode::BigInt:
		break;
	case ColumnMode::Text:
		return QIcon::fromTheme(QStringLiteral("draw-text"));
	case ColumnMode::DateTime:
	case ColumnMode::Month:
	case ColumnMode::Day:
		return QIcon::fromTheme(QStringLiteral("chronometer"));
	}

	return QIcon::fromTheme(QStringLiteral("x-shape-text"));
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
void AbstractColumn::setColumnMode(AbstractColumn::ColumnMode) {
}

/**
 * \brief Copy another column of the same type
 *
 * This function will return false if the data type
 * of 'other' is not the same as the type of 'this'.
 * Use a filter to convert a column to another type.
 */
bool AbstractColumn::copy(const AbstractColumn* /*other*/) {
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
bool AbstractColumn::copy(const AbstractColumn* /*source*/, int /*source_start*/, int /*destination_start*/, int /*num_rows*/) {
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
	beginMacro(i18np("%1: insert 1 row", "%1: insert %2 rows", name(), count));
	handleRowInsertion(before, count);
	endMacro();
}

void AbstractColumn::handleRowInsertion(int before, int count) {
	new AbstractColumnInsertRowsCmd(this, before, count);
}

/**
 * \brief Remove 'count' rows starting from row 'first'
 */
void AbstractColumn::removeRows(int first, int count) {
	beginMacro(i18np("%1: remove 1 row", "%1: remove %2 rows", name(), count));
	handleRowRemoval(first, count);
	endMacro();
}

void AbstractColumn::handleRowRemoval(int first, int count) {
	new AbstractColumnRemoveRowsCmd(this, first, count);
}

/**
 * \fn AbstractColumn::PlotDesignation AbstractColumn::plotDesignation() const
 * \brief Return the column plot designation
 */

/**
 * \brief Set the column plot designation
 */
void AbstractColumn::setPlotDesignation(AbstractColumn::PlotDesignation) {
}

bool AbstractColumn::isNumeric() const {
	const auto mode = columnMode();
	return (mode == ColumnMode::Double || mode == ColumnMode::Integer || mode == ColumnMode::BigInt);
}

bool AbstractColumn::isPlottable() const {
	const auto mode = columnMode();
	return (isNumeric() || mode == ColumnMode::DateTime);
}

/**
 * \brief Clear the whole column
 */
void AbstractColumn::clear() {
}

/**
 * \brief Convenience method for mode-independent testing of validity
 */
bool AbstractColumn::isValid(int row) const {
	switch (columnMode()) {
	case ColumnMode::Double:
		return std::isfinite(doubleAt(row));
	case ColumnMode::Integer: // there is no invalid integer
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
QVector<Interval<int>> AbstractColumn::maskedIntervals() const {
	return d->m_masking.intervals();
}

/**
 * \brief Clear all masking information
 */
void AbstractColumn::clearMasks() {
	exec(new AbstractColumnClearMasksCmd(d), "maskingAboutToChange", "maskingChanged", QArgument<const AbstractColumn*>("const AbstractColumn*", this));
}

/**
 * \brief Set an interval masked
 *
 * \param i the interval
 * \param mask true: mask, false: unmask
 */
void AbstractColumn::setMasked(const Interval<int>& i, bool mask) {
	exec(new AbstractColumnSetMaskedCmd(d, i, mask), "maskingAboutToChange", "maskingChanged", QArgument<const AbstractColumn*>("const AbstractColumn*", this));
}

/**
 * \brief Overloaded function for convenience
 */
void AbstractColumn::setMasked(int row, bool mask) {
	setMasked(Interval<int>(row, row), mask);
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
QString AbstractColumn::formula(int /*row*/) const {
	return {};
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
QVector<Interval<int>> AbstractColumn::formulaIntervals() const {
	return {};
}

/**
 * \brief Set a formula string for an interval of rows
 */
void AbstractColumn::setFormula(const Interval<int>&, const QString& /*formula*/) {
}

/**
 * \brief Overloaded function for convenience
 */
void AbstractColumn::setFormula(int /*row*/, const QString& /*formula*/) {
}

/**
 * \brief Clear all formulas
 */
void AbstractColumn::clearFormulas() {
}

// conditional formatting
bool AbstractColumn::hasHeatmapFormat() const {
	return (d->m_heatmapFormat != nullptr);
}

AbstractColumn::HeatmapFormat& AbstractColumn::heatmapFormat() const {
	if (!d->m_heatmapFormat)
		d->m_heatmapFormat = new HeatmapFormat();

	return *(d->m_heatmapFormat);
}

void AbstractColumn::setHeatmapFormat(const AbstractColumn::HeatmapFormat& format) {
	exec(new AbstractColumnSetHeatmapFormatCmd(d, format));
}

void AbstractColumn::removeFormat() {
	exec(new AbstractColumnRemoveHeatmapFormatCmd(d));
}

/*!
 * resets the connections for the dependent objects like plots and formulas in other columns
 * to the dataChanged signal in the current column.
 */
void AbstractColumn::reset() {
	// don't disconnect from all signals since we'd loose all connections done in AbstractAspect::connectChild().
	// disconnect from the dataChanged signal only, the reconnect is triggered in Project::descriptionChanged().
	Q_EMIT aboutToReset(this);
	disconnect(this, &AbstractColumn::dataChanged, nullptr, nullptr);
}

/*!
 * invalidates the internal properties of the columns and emits the signal \c dataChanged().
 *
 * call this function if the data of the column was changed directly via the data()-pointer
 * and not via the setValueAt() or when multiple cells are being notified and the signal \c dataChanged()
 * is suppressed for performance reasons and this function needs to be called after all cells were modified.
 */
void AbstractColumn::setDataChanged() {
	invalidateProperties();
	if (!d->m_suppressDataChangedSignal)
		Q_EMIT dataChanged(this);
}

/*!
 * suppresses the \c dataChanged signal if \c value is \c true, enables it otherwise.
 *
 * used when multiple cells are being modified and the signal needs to be suppressed for performance reasons,
 * \sa setDataChanged().
 */
void AbstractColumn::setSuppressDataChangedSignal(bool value) {
	d->m_suppressDataChangedSignal = value;
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
QString AbstractColumn::textAt(int /*row*/) const {
	return {};
}

/**
 * \brief Set the content of row 'row'
 *
 * Use this only when columnMode() is Text
 */
void AbstractColumn::setTextAt(int /*row*/, const QString& /*new_text*/) {
}

/**
 * \brief Replace a range of values
 *
 * Use this only when columnMode() is Text
 */
void AbstractColumn::replaceTexts(int /*first*/, const QVector<QString>& /*new_text*/) {
}

/**
 * \brief Return the position/index in the dictionary for the text value at \param row
 *
 * Use this only when columnMode() is Text
 */
int AbstractColumn::dictionaryIndex(int /* row */) const {
	return 0;
}
/**
 * \brief Return the date part of row 'row'
 *
 * Use this only when columnMode() is DateTime, Month or Day
 */
QDate AbstractColumn::dateAt(int /*row*/) const {
	return QDate{};
}

/**
 * \brief Set the content of row 'row'
 *
 * Use this only when columnMode() is DateTime, Month or Day
 */
void AbstractColumn::setDateAt(int /*row*/, QDate) {
}

/**
 * \brief Return the time part of row 'row'
 *
 * Use this only when columnMode() is DateTime, Month or Day
 */
QTime AbstractColumn::timeAt(int /*row*/) const {
	return {};
}

/**
 * \brief Set the content of row 'row'
 *
 * Use this only when columnMode() is DateTime, Month or Day
 */
void AbstractColumn::setTimeAt(int /*row*/, QTime) {
}

/**
 * \brief Return the QDateTime in row 'row'
 *
 * Use this only when columnMode() is DateTime, Month or Day
 */
QDateTime AbstractColumn::dateTimeAt(int /*row*/) const {
	return {};
}

/**
 * \brief Set the content of row 'row'
 *
 * Use this only when columnMode() is DateTime, Month or Day
 */
void AbstractColumn::setDateTimeAt(int /*row*/, const QDateTime&) {
}

/**
 * \brief Replace a range of values
 *
 * Use this only when columnMode() is DateTime, Month or Day
 */
void AbstractColumn::replaceDateTimes(int /*first*/, const QVector<QDateTime>&) {
}

/**
 * \brief Return the double value in row 'row'
 *
 * Use this only when columnMode() is Numeric
 */
double AbstractColumn::doubleAt(int /*row*/) const {
	return NAN;
}

/**
 * \brief Return the double value in row 'row' independent of the column mode.
 *
 * Integer and big integer values are converted to double, NAN is returned for other modes.
 */
double AbstractColumn::valueAt(int /*row*/) const {
	return NAN;
}

/**
 * \brief Set the content of row 'row'
 *
 * Use this only when columnMode() is Numeric
 */
void AbstractColumn::setValueAt(int /*row*/, const double) {
}

/**
 * \brief Replace a range of values
 *
 * Use this only when columnMode() is Numeric
 */
void AbstractColumn::replaceValues(int /*first*/, const QVector<double>&) {
}

/**
 * \brief Return the integer value in row 'row'
 *
 * Use this only when columnMode() is Integer
 */
int AbstractColumn::integerAt(int /*row*/) const {
	return 42;
}

/**
 * \brief Set the content of row 'row'
 *
 * Use this only when columnMode() is Integer
 */
void AbstractColumn::setIntegerAt(int /*row*/, const int) {
}

/**
 * \brief Replace a range of values
 *
 * Use this only when columnMode() is Integer
 */
void AbstractColumn::replaceInteger(int /*first*/, const QVector<int>&) {
}

/**
 * \brief Return the bigint value in row 'row'
 *
 * Use this only when columnMode() is BigInt
 */
qint64 AbstractColumn::bigIntAt(int /*row*/) const {
	return 42;
}

/**
 * \brief Set the content of row 'row'
 *
 * Use this only when columnMode() is BigInt
 */
void AbstractColumn::setBigIntAt(int /*row*/, const qint64) {
}

/**
 * \brief Replace a range of values
 *
 * Use this only when columnMode() is BigInt
 */
void AbstractColumn::replaceBigInt(int /*first*/, const QVector<qint64>&) {
}

/**
 * Returns the properties hold by this column (no, constant, monotonic increasing, monotonic decreasing,...)
 * Is used in XYCurve to improve the search velocity for the y value for a specific x value
 */
AbstractColumn::Properties AbstractColumn::properties() const {
	return AbstractColumn::Properties::No;
}

/**********************************************************************/
double AbstractColumn::minimum(int /*count*/) const {
	return INFINITY;
}

double AbstractColumn::minimum(int /*startIndex*/, int /*endIndex*/) const {
	return INFINITY;
}

double AbstractColumn::maximum(int /*count*/) const {
	return -INFINITY;
}

double AbstractColumn::maximum(int /*startIndex*/, int /*endIndex*/) const {
	return -INFINITY;
}

bool AbstractColumn::indicesMinMax(double /*v1*/, double /*v2*/, int& /*start*/, int& /*end*/) const {
	return false;
}

int AbstractColumn::indexForValue(double /*x*/, bool /* smaller */) const {
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
bool AbstractColumn::XmlReadMask(XmlStreamReader* reader) {
	Q_ASSERT(reader->isStartElement() && reader->name() == QLatin1String("mask"));

	bool ok1, ok2;
	int start, end;
	start = reader->readAttributeInt(QStringLiteral("start_row"), &ok1);
	end = reader->readAttributeInt(QStringLiteral("end_row"), &ok2);
	if (!ok1 || !ok2) {
		reader->raiseError(i18n("invalid or missing start or end row"));
		return false;
	}
	setMasked(Interval<int>(start, end));
	if (!reader->skipToEndElement())
		return false;

	return true;
}

/**
 * \brief Write XML mask element
 */
void AbstractColumn::XmlWriteMask(QXmlStreamWriter* writer) const {
	for (const auto& interval : maskedIntervals()) {
		writer->writeStartElement(QStringLiteral("mask"));
		writer->writeAttribute(QStringLiteral("start_row"), QString::number(interval.start()));
		writer->writeAttribute(QStringLiteral("end_row"), QString::number(interval.end()));
		writer->writeEndElement();
	}
}
