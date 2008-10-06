/***************************************************************************
    File                 : AbstractFilter.h
    Project              : SciDAVis
    --------------------------------------------------------------------
    Copyright            : (C) 2007,2008 by Knut Franke, Tilman Benkert
    Email (use @ for *)  : knut.franke*gmx.de, thzs*gmx.net
    Description          : Base class for all analysis operations.

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
#ifndef ABSTRACT_FILTER_H
#define ABSTRACT_FILTER_H

#include "AbstractAspect.h"
#include "AbstractColumn.h"
#include <QVector>

/**
 * \brief Base class for all analysis operations.
 *
 * AbstractFilter provides an abstraction for analysis operations. It is modelled on an
 * electronic filtering circuit: From the outside, a filter appears as a black box with
 * a number of input and output ports (obviously, those numbers do not necessarily agree).
 * 
 * \section using Using AbstractFilter
 * You can connect one AbstractColumn to each input port using
 * input(int port, AbstractColumn* source). Every output(int port) is realized
 * again by an AbstractColumn, which you can connect to as many other filters, tables
 * or plots as you like.
 * Ownership of the data sources always stays with the class which is providing the data,
 * that is, neither input() nor output() transfer ownership.
 *
 * Furthermore, you can use inputCount() and outputCount() to query the number of
 * input and output ports, respectively and you can obtain label strings for inputs (via
 * inputLabel()) and outputs (via AbstractColumn::label()). This allows generic filter
 * handling routines to be written, which is important for using filters provided by plugins.
 *
 * Its simplicity of use notwithstanding, AbstractFilter provides a powerful and versatile
 * basis also for analysis operations that would not commonly be referred to as "filter".
 * An example of such a more advanced filter implementation is StatisticsFilter.
 *
 * \section subclassing Subclassing AbstractFilter
 * The main design goal was to make implementing new filters as easy as possible.
 * Filters with only one output port can subclass AbstractSimpleFilter, which is even easier
 * to use. Filters with more than one output port have to subclass
 * AbstractFilter directly, which is slightly more involved, because in
 * addition to data transfer between these classes the signals defined by AbstractColumn
 * have to be handled on both inputs and outputs. Signals from data sources connected to the input
 * ports are automatically connected to a matching set of virtual methods, which can be
 * reimplemented by subclasses to handle these events.
 *
 * While AbstractFilter handles the tedious part of connecting a data source to an input port,
 * its subclasses are given a chance to reject such connections (e.g., based on the data type
 * of the source) by reimplementing inputAcceptable().
 *
 * \sa AbstractSimpleFilter
 */
class AbstractFilter : public AbstractAspect
{
	Q_OBJECT

	public:
		//! Standard constructor.
		AbstractFilter(const QString& name) : AbstractAspect(name) {}
		//! Destructor.
		virtual ~AbstractFilter() {}

		//! Return the number of input ports supported by the filter or -1 if any number of inputs is acceptable.
		virtual int inputCount() const = 0;
		/**
		 * \brief Return the number of output ports provided by the filter.
		 *
		 * %Note that this number need not be static, but can be dynamically determined, for example
		 * based on the inputs provided to the filter.
		 */
		virtual int outputCount() const = 0;
		/**
		 * \brief Return the index of the highest input port that is connected.
		 *
		 * Note that this is different from both the number of ports that <em>could</em> be connected,
		 * inputCount(), and the number of ports that actually <em>have been</em> connected, which are
		 * not necessarily sequential. In conjunction with input(int), this method can be used to
		 * traverse the connected inputs.
		 */
		int highestConnectedInput() const { return m_inputs.count() - 1; }
		/**
		 * \brief Connect the provided data source to the specified input port.
		 * \param port the port number to which to connect
		 * \param source the data source to connect to the input port
		 * \returns true if the connection was accepted, false otherwise.
		 *
		 * The port number is checked for validity against inputCount() and both port number and data
		 * source are passed to inputAcceptable() for review. If both checks succeed,the
		 * source is recorded in #m_inputs.
		 * If applicable, the previously connected data source is disconnected before replacing it.
		 *
		 * You can also use this method to disconnect an input without replacing it with a new one by
		 * calling it with source=0.
		 *
		 * \sa inputAcceptable(), #m_inputs
		 */
		bool input(int port, const AbstractColumn* source);
		/**
		 * \brief Connect all outputs of the provided filter to the corresponding inputs of this filter.
		 * \returns true if all connections were accepted, false otherwise
		 *
		 * Overloaded method provided for convenience.
		 */
		bool input(const AbstractFilter* sources);
		//! Return the input currently connected to the specified port, or 0.
		const AbstractColumn *input(int port) const { return m_inputs.value(port); }
		/**
		 * \brief Return the label associated to the given input port.
		 *
		 * Default labels are In1, In2, ... (or translated equivalents), but implementations can
		 * reimplement this method to produce more meaningful labels.
		 *
		 * Output ports are implicitly labeled through AbstractAspect::name().
		 */
		virtual QString inputLabel(int port) const;
		/**
		 * \brief Get the data source associated with the specified output port.
		 *
		 * The returned pointer may be 0 even for valid port numbers, for example if not all required
		 * input ports have been connected.
		 */
		virtual AbstractColumn* output(int port=0) = 0;
		//! Overloaded method for const access.
		virtual const AbstractColumn* output(int port=0) const = 0;
		
		//! Return the input port to which the column is connected or -1 if it's not connected
		int portIndexOf(const AbstractColumn * column)
		{
			for(int i=0; i<m_inputs.size(); i++)
				if(m_inputs.at(i) == column) return i;
			return -1;
		}

	protected:
		/**
		 * \brief Give implementations a chance to reject connections to their input ports.
		 *
		 * If not reimplemented, all connections to ports within [0, inputCount()-1] will be accepted.
		 */
		virtual bool inputAcceptable(int port, const AbstractColumn *source) {
			Q_UNUSED(port); Q_UNUSED(source); return true;
		}
		/**
		 * \brief Called whenever an input is disconnected or deleted.
		 *
		 * This is only to notify implementations of the event, the default implementation is a
		 * no-op.
		 */
		virtual void inputAboutToBeDisconnected(const AbstractColumn * source) { Q_UNUSED(source); }

	protected slots:

		//!\name signal handlers
		//@{
		/**
		 * \brief Name and/or comment of an input will be changed.
		 *
		 * \param source is always the this pointer of the column that emitted the signal.
		 */
		virtual void inputDescriptionAboutToChange(const AbstractColumn * source) { Q_UNUSED(source); } 
		void inputDescriptionAboutToChange(const AbstractAspect * aspect) {
			const AbstractColumn * col = qobject_cast<const AbstractColumn*>(aspect);
			if (col) inputDescriptionAboutToChange(col);
		}
		//! 
		/**
		 * \brief Name and/or comment of an input changed.
		 *
		 * \param source is always the this pointer of the column that emitted the signal.
		 */
		virtual void inputDescriptionChanged(const AbstractColumn * source) { Q_UNUSED(source); }
		void inputDescriptionChanged(const AbstractAspect * aspect) {
			const AbstractColumn * col = qobject_cast<const AbstractColumn*>(aspect);
			if (col && m_inputs.contains(col)) inputDescriptionChanged(col);
		}
		/**
		 * \brief The plot designation of an input is about to change.
		 *
		 * \param source is always the this pointer of the column that emitted the signal.
		 */
		virtual void inputPlotDesignationAboutToChange(const AbstractColumn * source) { Q_UNUSED(source); }
;
		/**
		 * \brief The plot designation of an input changed.
		 *
		 * \param source is always the this pointer of the column that emitted the signal.
		 */
		virtual void inputPlotDesignationChanged(const AbstractColumn * source) { Q_UNUSED(source); }
		/**
		 * \brief The display mode and possibly the data type of an input is about to change.
		 *
		 * \param source is always the this pointer of the column that emitted the signal.
		 */
		virtual void inputModeAboutToChange(const AbstractColumn * source) { Q_UNUSED(source); }
		/**
		 * \brief The display mode and possibly the data type has changed.
		 *
		 * \param source is always the this pointer of the column that emitted the signal.
		 */
		virtual void inputModeChanged(const AbstractColumn * source) { Q_UNUSED(source); }
		/**
		 * \brief The data of an input is about to change.
		 *
		 * \param source is always the this pointer of the column that emitted the signal.
		 */
		virtual void inputDataAboutToChange(const AbstractColumn * source) { Q_UNUSED(source); }
		/**
		 * \brief The data of an input has changed.
		 *
		 * \param source is always the this pointer of the column that emitted the signal.
		 */
		virtual void inputDataChanged(const AbstractColumn * source) { Q_UNUSED(source); }
		/**
		 * \brief An input is about to be replaced.
		 *
		 * This signal is handled by AbstractFilter and mapped to input(int,AbstractColumn*),
		 * which calls inputDescriptionAboutToChange(), inputPlotDesignationAboutToChange(),
		 * inputDataAboutToChange(), inputMaskingAboutToChange(), 
		 * inputDescripionChanged(), inputMaskingChanged()
		 * inputPlotDesignationChanged() and inputDataChanged().
		 * inputModeAboutToChange() and inputModeChanged() are
		 * called if the new column has a different mode (and thereby possibly data type).
		 * Thus, filter implementations won't have to bother with it most of the time.
		 */
		virtual void inputAboutToBeReplaced(const AbstractColumn * source, const AbstractColumn* replacement);

		virtual void inputRowsAboutToBeInserted(const AbstractColumn * source, int before, int count) {
			Q_UNUSED(source); Q_UNUSED(before); Q_UNUSED(count);
		}
		virtual void inputRowsInserted(const AbstractColumn * source, int before, int count) {
			Q_UNUSED(source); Q_UNUSED(before); Q_UNUSED(count);
		}
		virtual void inputRowsAboutToBeRemoved(const AbstractColumn * source, int first, int count) {
			Q_UNUSED(source); Q_UNUSED(first); Q_UNUSED(count);
		}
		virtual void inputRowsRemoved(const AbstractColumn * source, int first, int count) {
			Q_UNUSED(source); Q_UNUSED(first); Q_UNUSED(count);
		}
		virtual void inputMaskingAboutToChange(const AbstractColumn * source) {
			Q_UNUSED(source);
		}
		virtual void inputMaskingChanged(const AbstractColumn * source) {
			Q_UNUSED(source);
		}
		void inputAboutToBeDestroyed(const AbstractColumn * source) {
			input(portIndexOf(source), 0);
		}
		//@}

	protected:
		//! The data sources connected to my input ports.
		QVector<const AbstractColumn*> m_inputs;
};

#endif // ifndef ABSTRACT_FILTER_H

