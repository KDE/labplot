/***************************************************************************
    File                 : AbstractSimpleFilter.cpp
    Project              : AbstractColumn
    --------------------------------------------------------------------
    Copyright            : (C) 2007,2008 by Knut Franke, Tilman Benkert
    Email (use @ for *)  : knut.franke*gmx.de, thzs*gmx.net
    Description          : Simplified filter interface for filters with
                           only one output port.

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

#include "AbstractSimpleFilter.h"
#include "backend/lib/XmlStreamReader.h"

#include <QDateTime>
#include <QDate>
#include <QTime>

#include <KLocalizedString>

/**
 * \class AbstractSimpleFilter
 * \brief Simplified filter interface for filters with only one output port.
 *
 * This class is only meant to simplify implementation of a restricted subtype of filter.
 * It should not be instantiated directly. You should always either derive from
 * AbstractFilter or (if necessary) provide an actual (non-abstract) implementation.
 *
 * The trick here is that, in a sense, the filter is its own output port. This means you
 * can implement a complete filter in only one class and don't have to coordinate data
 * transfer between a filter class and a data source class.
 * Additionally, AbstractSimpleFilter offers some useful convenience methods which make writing
 * filters as painless as possible.
 *
 * For the data type of the output, all types supported by AbstractColumn (currently double, QString and
 * QDateTime) are supported.
 *
 * \section tutorial1 Tutorial, Step 1
 * The simplest filter you can write assumes there's also only one input port and rows on the
 * input correspond 1:1 to rows in the output. All you need to specify is what data type you
 * want to have (in this example double) on the input port and how to compute the output values:
 *
 * \code
 * 01 #include "AbstractSimpleFilter.h"
 * 02 class TutorialFilter1 : public AbstractSimpleFilter
 * 03 {
 * 04	protected:
 * 05		virtual bool inputAcceptable(int, AbstractColumn *source) {
 * 06			return (source->columnMode() == AbstractColumn::Numeric);
 * 07		}
 * 08	public:
 * 09		virtual AbstractColumn::ColumnMode columnMode() const { return AbstractColumn::Numeric; }
 * 10
 * 11		virtual double valueAt(int row) const {
 * 12			if (!m_inputs.value(0)) return 0.0;
 * 13			double input_value = m_inputs.value(0)->valueAt(row);
 * 14			return input_value * input_value;
 * 15		}
 * 16 };
 * \endcode
 *
 * This filter reads an input value (line 13) and returns its square (line 14).
 * Reimplementing inputAcceptable() makes sure that the data source really is of type
 * double (lines 5 to 7). Otherwise, the source will be rejected by AbstractFilter::input().
 * The output type is reported by reimplementing columnMode() (line 09).
 * Before you actually use m_inputs.value(0), make sure that the input port has
 * been connected to a data source (line 12).
 * Otherwise line 13 would result in a crash. That's it, we've already written a
 * fully-functional filter!
 *
 * Equivalently, you can write 1:1-filters for QString or QDateTime inputs by checking for
 * AbstractColumn::TypeQString or AbstractColumn::TypeQDateTime in line 6. You would then use
 * AbstractColumn::textAt(row) or AbstractColumn::dateTimeAt(row) in line 13 to access the input data.
 * For QString output, you need to implement AbstractColumn::textAt(row).
 * For QDateTime output, you have to implement three methods:
 * \code
 * virtual QDateTime dateTimeAt(int row) const;
 * virtual QDate dateAt(int row) const;
 * virtual QTime timeAt(int row) const;
 * \endcode
 *
 * \section tutorial2 Tutorial, Step 2
 * Now for something slightly more interesting: a filter that uses only every second row of its
 * input. We no longer have a 1:1 correspondence between input and output rows, so we'll have
 * to do a bit more work in order to have everything come out as expected.
 * We'll use double-typed input and output again:
 * \code
 * 01 #include "AbstractSimpleFilter.h"
 * 02 class TutorialFilter2 : public AbstractSimpleFilter
 * 03 {
 * 04	protected:
 * 05		virtual bool inputAcceptable(int, AbstractColumn *source) {
 * 06			return (source->columnMode() == AbstractColumn::Numeric);
 * 07		}
 * 08	public:
 * 09		virtual AbstractColumn::ColumnMode columnMode() const { return AbstractColumn::Numeric; }
 * \endcode
 * Even rows (including row 0) get dropped, odd rows are renumbered:
 * \code
 * 10	public:
 * 11 	virtual double valueAt(int row) const {
 * 12		if (!m_inputs.value(0)) return 0.0;
 * 13		return m_inputs.value(0)->valueAt(2*row + 1);
 * 14 	}
 * \endcode
 */

// TODO: should simple filters have a name argument?
/**
 * \brief Ctor
 */
AbstractSimpleFilter::AbstractSimpleFilter()
	: AbstractFilter("SimpleFilter"), m_output_column(new SimpleFilterColumn(this)) {
	addChild(m_output_column);
}

/**
 * \brief Default to one input port.
 */
int AbstractSimpleFilter::inputCount() const {
	return 1;
}

/**
 * \brief We manage only one output port (don't override unless you really know what you are doing).
 */
int AbstractSimpleFilter::outputCount() const {
	return 1;
}

/**
 * \brief Copy plot designation of input port 0.
 */
AbstractColumn::PlotDesignation AbstractSimpleFilter::plotDesignation() const {
	return m_inputs.value(0) ? m_inputs.at(0)->plotDesignation() : AbstractColumn::NoDesignation;
}

/**
 * \brief Return the column mode
 *
 * This function is most used by tables but can also be used
 * by plots. The column mode specifies how to interpret
 * the values in the column additional to the data type.
 */
AbstractColumn::ColumnMode AbstractSimpleFilter::columnMode() const {
	// calling this function while m_input is empty is a sign of very bad code
	// nevertheless it will return some rather meaningless value to
	// avoid crashes
	return m_inputs.value(0) ? m_inputs.at(0)->columnMode() : AbstractColumn::Text;
}

/**
 * \brief Return the content of row 'row'.
 *
 * Use this only when columnMode() is Text
 */
QString AbstractSimpleFilter::textAt(int row) const {
	return m_inputs.value(0) ? m_inputs.at(0)->textAt(row) : QString();
}

/**
 * \brief Return the date part of row 'row'
 *
 * Use this only when columnMode() is DateTime, Month or Day
 */
QDate AbstractSimpleFilter::dateAt(int row) const {
	return m_inputs.value(0) ? m_inputs.at(0)->dateAt(row) : QDate();
}

/**
 * \brief Return the time part of row 'row'
 *
 * Use this only when columnMode() is DateTime, Month or Day
 */
QTime AbstractSimpleFilter::timeAt(int row) const {
	return m_inputs.value(0) ? m_inputs.at(0)->timeAt(row) : QTime();
}

/**
 * \brief Set the content of row 'row'
 *
 * Use this only when columnMode() is DateTime, Month or Day
 */
QDateTime AbstractSimpleFilter::dateTimeAt(int row) const {
	return m_inputs.value(0) ? m_inputs.at(0)->dateTimeAt(row) : QDateTime();
}

/**
 * \brief Return the double value in row 'row'
 *
 * Use this only when columnMode() is Numeric
 */
double AbstractSimpleFilter::valueAt(int row) const {
	return m_inputs.value(0) ? m_inputs.at(0)->valueAt(row) : 0.0;
}

/**
 * \brief Return the integer value in row 'row'
 *
 * Use this only when columnMode() is Integer
 */
int AbstractSimpleFilter::integerAt(int row) const {
	return m_inputs.value(0) ? m_inputs.at(0)->integerAt(row) : 0;
}

/**
 * \brief Number of output rows == number of input rows
 *
 * ... unless overridden in a subclass.
 */
int AbstractSimpleFilter::rowCount() const {
	return m_inputs.value(0) ? m_inputs.at(0)->rowCount() : 0;
}

/**
 * \brief Rows that will change when the given input interval changes.
 *
 * This implementation assumes a 1:1 correspondence between input and output rows, but can be
 * overridden in subclasses.
 */
QList< Interval<int> > AbstractSimpleFilter::dependentRows(const Interval<int>& inputRange) const {
	return QList< Interval<int> >() << inputRange;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//!\name signal handlers
//@{
////////////////////////////////////////////////////////////////////////////////////////////////////

void AbstractSimpleFilter::inputPlotDesignationAboutToChange(const AbstractColumn*) {
	emit m_output_column->plotDesignationAboutToChange(m_output_column);
}

void AbstractSimpleFilter::inputPlotDesignationChanged(const AbstractColumn*) {
	emit m_output_column->plotDesignationChanged(m_output_column);
}

void AbstractSimpleFilter::inputModeAboutToChange(const AbstractColumn*) {
	emit m_output_column->dataAboutToChange(m_output_column);
}

void AbstractSimpleFilter::inputModeChanged(const AbstractColumn*) {
	emit m_output_column->dataChanged(m_output_column);
}

void AbstractSimpleFilter::inputDataAboutToChange(const AbstractColumn*) {
	emit m_output_column->dataAboutToChange(m_output_column);
}

void AbstractSimpleFilter::inputDataChanged(const AbstractColumn*) {
	emit m_output_column->dataChanged(m_output_column);
}

void AbstractSimpleFilter::inputRowsAboutToBeInserted(const AbstractColumn * source, int before, int count) {
	Q_UNUSED(source);
	Q_UNUSED(count);
	foreach (const Interval<int>& output_range, dependentRows(Interval<int>(before, before)))
		emit m_output_column->rowsAboutToBeInserted(m_output_column, output_range.start(), output_range.size());
}

void AbstractSimpleFilter::inputRowsInserted(const AbstractColumn * source, int before, int count) {
	Q_UNUSED(source);
	Q_UNUSED(count);
	foreach (const Interval<int>& output_range, dependentRows(Interval<int>(before, before)))
		emit m_output_column->rowsInserted(m_output_column, output_range.start(), output_range.size());
}

void AbstractSimpleFilter::inputRowsAboutToBeRemoved(const AbstractColumn * source, int first, int count) {
	Q_UNUSED(source);
	foreach (const Interval<int>& output_range, dependentRows(Interval<int>(first, first+count-1)))
		emit m_output_column->rowsAboutToBeRemoved(m_output_column, output_range.start(), output_range.size());
}

void AbstractSimpleFilter::inputRowsRemoved(const AbstractColumn * source, int first, int count) {
	Q_UNUSED(source);
	foreach (const Interval<int>& output_range, dependentRows(Interval<int>(first, first+count-1)))
		emit m_output_column->rowsRemoved(m_output_column, output_range.start(), output_range.size());
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//@}
////////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * \brief Return a pointer to #m_output_column on port 0 (don't override unless you really know what you are doing).
 */
AbstractColumn *AbstractSimpleFilter::output(int port) {
	return port == 0 ? static_cast<AbstractColumn*>(m_output_column) : 0;
}

const AbstractColumn *AbstractSimpleFilter::output(int port) const {
	return port == 0 ? static_cast<const AbstractColumn*>(m_output_column) : 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//! \name serialize/deserialize
//@{
////////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * \brief Save to XML
 */
void AbstractSimpleFilter::save(QXmlStreamWriter * writer) const {
	writer->writeStartElement("simple_filter");
	writeBasicAttributes(writer);
	writeExtraAttributes(writer);
	writer->writeAttribute("filter_name", metaObject()->className());
	writeCommentElement(writer);
	writer->writeEndElement();
}

/**
 * \brief Override this in derived classes if they have other attributes than filter_name
 */
void AbstractSimpleFilter::writeExtraAttributes(QXmlStreamWriter * writer) const {
	Q_UNUSED(writer)
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//@}
////////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * \brief Load from XML
 */
bool AbstractSimpleFilter::load(XmlStreamReader* reader, bool preview) {
	Q_UNUSED(preview); //TODO

	if(reader->isStartElement() && reader->name() == "simple_filter") {
		if (!readBasicAttributes(reader)) return false;

		QXmlStreamAttributes attribs = reader->attributes();
		QString str = attribs.value(reader->namespaceUri().toString(), "filter_name").toString();
		if(str != metaObject()->className()) {
			reader->raiseError(i18n("incompatible filter type"));
			return false;
		}

		// read child elements
		while (!reader->atEnd()) {
			reader->readNext();

			if (reader->isEndElement()) break;

			if (reader->isStartElement()) {
				if (reader->name() == "comment") {
					if (!readCommentElement(reader)) return false;
				} else { // unknown element
					reader->raiseWarning(i18n("unknown element '%1'", reader->name().toString()));
					if (!reader->skipToEndElement()) return false;
				}
			}
		}
	} else
		reader->raiseError(i18n("no simple filter element found"));

	return !reader->hasError();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//! \class SimpleFilterColumn
////////////////////////////////////////////////////////////////////////////////////////////////////

AbstractColumn::ColumnMode SimpleFilterColumn::columnMode() const {
	return m_owner->columnMode();
}

QString SimpleFilterColumn::textAt(int row) const {
	return m_owner->textAt(row);
}

QDate SimpleFilterColumn::dateAt(int row) const {
	return m_owner->dateAt(row);
}

QTime SimpleFilterColumn::timeAt(int row) const {
	return m_owner->timeAt(row);
}

QDateTime SimpleFilterColumn::dateTimeAt(int row) const {
	return m_owner->dateTimeAt(row);
}

double SimpleFilterColumn::valueAt(int row) const {
	return m_owner->valueAt(row);
}

int SimpleFilterColumn::integerAt(int row) const {
	return m_owner->integerAt(row);
}
