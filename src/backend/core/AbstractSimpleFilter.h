/***************************************************************************
    File                 : AbstractSimpleFilter.h
    Project              : SciDAVis
    --------------------------------------------------------------------
    Copyright            : (C) 2007 by Knut Franke, Tilman Benkert
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
#ifndef ABSTRACT_SIMPLE_FILTER
#define ABSTRACT_SIMPLE_FILTER

#include "AbstractFilter.h"
#include "AbstractColumn.h"
#include "lib/IntervalAttribute.h"
#include "lib/XmlStreamReader.h"
#include <QUndoCommand>
#include <QXmlStreamWriter>

// forward declaration - class follows
class SimpleFilterColumn;

/**
 * \brief Simplified filter interface for filters with only one output port.
 *
 * This class is only meant to simplify implementation of a restricted subtype of filter.
 * It should not be instantiated directly. You should always either derive from
 * AbstractFilter or (if necessary) provide an actual (non-abstract) implementation.
 *
 * The trick here is that, in a sense, the filter is its own output port. This means you
 * can implement a complete filter in only one class and don't have to coordinate data
 * transfer between a filter class and a data source class.
 * Additionaly, AbstractSimpleFilter offers some useful convenience methods which make writing
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
 * 06			return (source->dataType() == SciDAVis::TypeDouble);
 * 07		}
 * 08	public:
 * 09		virtual SciDAVis::ColumnDataType dataType() const { return SciDAVis::TypeDouble; }
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
 * The output type is repoted by reimplementing dataType() (line 09).
 * Before you actually use m_inputs.value(0), make sure that the input port has
 * been connected to a data source (line 12).
 * Otherwise line 13 would result in a crash. That's it, we've already written a
 * fully-functional filter!
 *
 * Equivalently, you can write 1:1-filters for QString or QDateTime inputs by checking for
 * SciDAVis::TypeQString or SciDAVis::TypeQDateTime in line 6. You would then use
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
 * 06			return (source->dataType() == SciDAVis::TypeDouble);
 * 07		}
 * 08	public:
 * 09		virtual SciDAVis::ColumnDataType dataType() const { return SciDAVis::TypeDouble; }
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
class AbstractSimpleFilter : public AbstractFilter
{
	Q_OBJECT

	public:
		//! Ctor
		AbstractSimpleFilter();
		//! Default to one input port.
		virtual int inputCount() const { return 1; }
		//! We manage only one output port (don't override unless you really know what you are doing).
		virtual int outputCount() const { return 1; }
		//! Return a pointer to #m_output_column on port 0 (don't override unless you really know what you are doing).
		virtual AbstractColumn* output(int port);
		virtual const AbstractColumn * output(int port) const;
		//! Copy plot designation of input port 0.
		virtual SciDAVis::PlotDesignation plotDesignation() const {
			return m_inputs.value(0) ?
				m_inputs.at(0)->plotDesignation() :
				SciDAVis::noDesignation;
		}
		//! Return the data type of the input
		virtual SciDAVis::ColumnDataType dataType() const
		{
			// calling this function while m_input is empty is a sign of very bad code
			// nevertheless it will return some rather meaningless value to
			// avoid crashes
			return m_inputs.value(0) ? m_inputs.at(0)->dataType() : SciDAVis::TypeQString;
		}
		//! Return the column mode
		/**
		 * This function is most used by tables but can also be used
		 * by plots. The column mode specifies how to interpret 
		 * the values in the column additional to the data type.
		 */ 
		virtual SciDAVis::ColumnMode columnMode() const
		{
			// calling this function while m_input is empty is a sign of very bad code
			// nevertheless it will return some rather meaningless value to
			// avoid crashes
			return m_inputs.value(0) ? m_inputs.at(0)->columnMode() : SciDAVis::Text;
		}
		//! Return the content of row 'row'.
		/**
		 * Use this only when dataType() is QString
		 */
		virtual QString textAt(int row) const
		{
			return m_inputs.value(0) ? m_inputs.at(0)->textAt(row) : QString();
		}
		//! Return the date part of row 'row'
		/**
		 * Use this only when dataType() is QDateTime
		 */
		virtual QDate dateAt(int row) const
		{
			return m_inputs.value(0) ? m_inputs.at(0)->dateAt(row) : QDate();
		}
		//! Return the time part of row 'row'
		/**
		 * Use this only when dataType() is QDateTime
		 */
		virtual QTime timeAt(int row) const
		{
			return m_inputs.value(0) ? m_inputs.at(0)->timeAt(row) : QTime();
		}
		//! Set the content of row 'row'
		/**
		 * Use this only when dataType() is QDateTime
		 */
		virtual QDateTime dateTimeAt(int row) const
		{
			return m_inputs.value(0) ? m_inputs.at(0)->dateTimeAt(row) : QDateTime();
		}
		//! Return the double value in row 'row'
		/**
		 * Use this only when dataType() is double
		 */
		virtual double valueAt(int row) const
		{
			return m_inputs.value(0) ? m_inputs.at(0)->valueAt(row) : 0.0;
		}

		//!\name assuming a 1:1 correspondence between input and output rows
		//@{
		virtual int rowCount() const {
			return m_inputs.value(0) ? m_inputs.at(0)->rowCount() : 0;
		}
		virtual QList< Interval<int> > dependentRows(Interval<int> input_range) const { return QList< Interval<int> >() << input_range; }
		//@}

		//!\name Masking
		//@{
		//! Return whether a certain row is masked
		virtual bool isMasked(int row) const { return m_masking.isSet(row); }
		//! Return whether a certain interval of rows rows is fully masked
		virtual bool isMasked(Interval<int> i) const { return m_masking.isSet(i); }
		//! Return all intervals of masked rows
		virtual QList< Interval<int> > maskedIntervals() const { return m_masking.intervals(); }
		//! Clear all masking information
		virtual void clearMasks();
		//! Set an interval masked
		/**
		 * \param i the interval
		 * \param mask true: mask, false: unmask
		 */ 
		virtual void setMasked(Interval<int> i, bool mask = true);
		//! Overloaded function for convenience
		virtual void setMasked(int row, bool mask = true) { setMasked(Interval<int>(row,row), mask); }
		//@}

		//! Return whether a certain row contains an invalid value 	 
		virtual bool isInvalid(int row) const { return m_inputs.value(0) ? m_inputs.at(0)->isInvalid(row) : false; }
		//! Return whether a certain interval of rows contains only invalid values 	 
		virtual bool isInvalid(Interval<int> i) const { return m_inputs.value(0) ? m_inputs.at(0)->isInvalid(i) : false; }
		//! Return all intervals of invalid rows
		virtual QList< Interval<int> > invalidIntervals() const 
		{
			return m_inputs.value(0) ? m_inputs.at(0)->maskedIntervals() : QList< Interval<int> >(); 
		}

		//! \name XML related functions
		//@{
		//! Save to XML
		virtual void save(QXmlStreamWriter * writer) const;
		//! Load from XML
		virtual bool load(XmlStreamReader * reader);
		//! Override this in derived classes if they have other attributes than filter_name
		virtual void writeExtraAttributes(QXmlStreamWriter * writer) const { Q_UNUSED(writer) }
		//@}

	protected:
		IntervalAttribute<bool> m_masking;

		//!\name signal handlers
		//@{
		virtual void inputPlotDesignationAboutToChange(const AbstractColumn*);
		virtual void inputPlotDesignationChanged(const AbstractColumn*);
		virtual void inputModeAboutToChange(const AbstractColumn*);
		virtual void inputModeChanged(const AbstractColumn*);
		virtual void inputDataAboutToChange(const AbstractColumn*);
		virtual void inputDataChanged(const AbstractColumn*);

		virtual void inputRowsAboutToBeInserted(const AbstractColumn * source, int before, int count);
		virtual void inputRowsInserted(const AbstractColumn * source, int before, int count);
		virtual void inputRowsAboutToBeRemoved(const AbstractColumn * source, int first, int count);
		virtual void inputRowsRemoved(const AbstractColumn * source, int first, int count);
		//@}
		
		SimpleFilterColumn *m_output_column;
};

class SimpleFilterColumn : public AbstractColumn
{
	Q_OBJECT

	public:
		SimpleFilterColumn(AbstractSimpleFilter *owner) : AbstractColumn(owner->name()), m_owner(owner) {}

		virtual SciDAVis::ColumnDataType dataType() const { return m_owner->dataType(); }
		virtual SciDAVis::ColumnMode columnMode() const { return m_owner->columnMode(); }
		virtual int rowCount() const { return m_owner->rowCount(); }
		virtual SciDAVis::PlotDesignation plotDesignation() const { return m_owner->plotDesignation(); }
		virtual bool isInvalid(int row) const { return m_owner->isInvalid(row); }
		virtual bool isInvalid(Interval<int> i) const { return m_owner->isInvalid(i); }
		virtual QList< Interval<int> > invalidIntervals() const { return m_owner->invalidIntervals(); }
		virtual bool isMasked(int row) const { return m_owner->isMasked(row); }
		virtual bool isMasked(Interval<int> i) const { return m_owner->isMasked(i); }
		virtual QList< Interval<int> > maskedIntervals() const { return m_owner->maskedIntervals(); }
		virtual void clearMasks() { m_owner->clearMasks(); }
		virtual QString textAt(int row) const { return m_owner->textAt(row); }
		virtual QDate dateAt(int row) const { return m_owner->dateAt(row); }
		virtual QTime timeAt(int row) const { return m_owner->timeAt(row); }
		virtual QDateTime dateTimeAt(int row) const { return m_owner->dateTimeAt(row); }
		virtual double valueAt(int row) const { return m_owner->valueAt(row); }

	private:
		AbstractSimpleFilter *m_owner;

	friend class AbstractSimpleFilter;
};

#endif // ifndef ABSTRACT_SIMPLE_FILTER

