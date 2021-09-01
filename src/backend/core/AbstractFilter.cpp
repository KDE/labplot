/*
    File                 : AbstractFilter.cpp
    Project              : LabPlot
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2007 Knut Franke Tilman Benkert <knut.franke*gmx.de, thzs*gmx.net (use @ for *)>
    Description          : Base class for all analysis operations.
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "AbstractFilter.h"
#include <KLocalizedString>

#include "backend/core/AbstractColumn.h"
#include "backend/lib/macros.h"

/**
 * \class AbstractFilter
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

/**
 * \fn AbstractFilter::inputCount() const
 * \brief Return the number of input ports supported by the filter or -1 if any number of inputs is acceptable.
 */

/**
 * \fn AbstractFilter::outputCount() const
 * \brief Return the number of output ports provided by the filter.
 *
 * %Note that this number need not be static, but can be dynamically determined, for example
 * based on the inputs provided to the filter.
 */

/**
 * \brief Return the index of the highest input port that is connected.
 *
 * Note that this is different from both the number of ports that <em>could</em> be connected,
 * inputCount(), and the number of ports that actually <em>have been</em> connected, which are
 * not necessarily sequential. In conjunction with input(int), this method can be used to
 * traverse the connected inputs.
 */
int AbstractFilter::highestConnectedInput() const {
	return m_inputs.count() - 1;
}

/**
 * \brief Connect the provided data source to the specified input port.
 * \param port the port number to which to connect
 * \param source the data source to connect to the input port
 * \returns true if the connection was accepted, false otherwise.
 *
 * The port number is checked for validity against inputCount() and both port number and data
 * source are passed to inputAcceptable() for review. If both checks succeed, the
 * source is recorded in #m_inputs.
 * If applicable, the previously connected data source is disconnected before replacing it.
 *
 * You can also use this method to disconnect an input without replacing it with a new one by
 * calling it with source=0.
 *
 * \sa inputAcceptable(), #m_inputs
 */
bool AbstractFilter::input(int port, const AbstractColumn* source) {
//	DEBUG("AbstractFilter::input()");

	if (port < 0 || (inputCount() >= 0 && port >= inputCount())) return false;
	if (source && !inputAcceptable(port, source)) return false;

	if (port >= m_inputs.size()) m_inputs.resize(port+1);
	const AbstractColumn* old_input = m_inputs.value(port);
	if (source == old_input) return true;

	if (old_input) {
		disconnect(old_input, nullptr, this, nullptr);
		// replace input, notifying the filter implementation of the changes
		inputDescriptionAboutToChange(old_input);
		inputPlotDesignationAboutToChange(old_input);
		inputMaskingAboutToChange(old_input);
		inputDataAboutToChange(old_input);
		if (source && source->columnMode() != old_input->columnMode())
			inputModeAboutToChange(old_input);
	}
	if (!source)
		inputAboutToBeDisconnected(old_input);
	m_inputs[port] = source;
	if (source) { // we have a new source
//		DEBUG("	new source");
		if (old_input && source->columnMode() != old_input->columnMode())
			inputModeAboutToChange(source);
		inputDataChanged(source);
		inputMaskingChanged(source);
		inputPlotDesignationChanged(source);
		inputDescriptionChanged(source);
		// connect the source's signals
		connect(source, &AbstractColumn::aspectDescriptionAboutToChange, this,
				static_cast<void (AbstractFilter::*)(const AbstractAspect*)>(&AbstractFilter::inputDescriptionAboutToChange));
		connect(source, &AbstractColumn::aspectDescriptionChanged, this,
				static_cast<void (AbstractFilter::*)(const AbstractAspect*)>(&AbstractFilter::inputDescriptionChanged));
		connect(source, &AbstractColumn::plotDesignationAboutToChange, this, &AbstractFilter::inputPlotDesignationAboutToChange);
		connect(source, &AbstractColumn::plotDesignationChanged, this, &AbstractFilter::inputPlotDesignationChanged);
		connect(source, &AbstractColumn::modeAboutToChange, this, &AbstractFilter::inputModeAboutToChange);
		connect(source, &AbstractColumn::modeChanged, this, &AbstractFilter::inputModeChanged);
		connect(source, &AbstractColumn::dataAboutToChange, this, &AbstractFilter::inputDataAboutToChange);
		connect(source, &AbstractColumn::dataChanged, this, &AbstractFilter::inputDataChanged);
		connect(source, &AbstractColumn::rowsAboutToBeInserted, this, &AbstractFilter::inputRowsAboutToBeInserted);
		connect(source, &AbstractColumn::rowsInserted, this, &AbstractFilter::inputRowsInserted);
		connect(source, &AbstractColumn::rowsAboutToBeRemoved, this, &AbstractFilter::inputRowsAboutToBeRemoved);
		connect(source, &AbstractColumn::rowsRemoved, this, &AbstractFilter::inputRowsRemoved);
		connect(source, &AbstractColumn::maskingAboutToChange, this, &AbstractFilter::inputMaskingAboutToChange);
		connect(source, &AbstractColumn::maskingChanged, this, &AbstractFilter::inputMaskingChanged);
		connect(source, &AbstractColumn::aboutToBeDestroyed, this, &AbstractFilter::inputAboutToBeDestroyed);
	} else { // source == 0, that is, the input port has been disconnected
//		DEBUG("	no source");
		// try to shrink m_inputs
		int num_connected_inputs = m_inputs.size();
		while (m_inputs.at(num_connected_inputs-1) == nullptr) {
			num_connected_inputs--;
			if (!num_connected_inputs) break;
		}
		m_inputs.resize(num_connected_inputs);
	}

	return true;
}

/**
 * \brief Connect all outputs of the provided filter to the corresponding inputs of this filter.
 * \returns true if all connections were accepted, false otherwise
 *
 * Overloaded method provided for convenience.
 */
bool AbstractFilter::input(const AbstractFilter* sources) {
	if (!sources) return false;
	bool result = true;
	for (int i = 0; i < sources->outputCount(); ++i)
		if (!input(i, sources->output(i)))
			result = false;
	return result;
}

/**
 * \brief Return the input currently connected to the specified port, or 0.
 */
const AbstractColumn* AbstractFilter::input(int port) const {
	return m_inputs.value(port);
}

/**
 * \brief Return the label associated to the given input port.
 *
 * Default labels are In1, In2, ... (or translated equivalents), but implementations can
 * reimplement this method to produce more meaningful labels.
 *
 * Output ports are implicitly labeled through AbstractAspect::name().
 */
QString AbstractFilter::inputLabel(int port) const {
	return i18nc("default labels of filter input ports", "In%1", port + 1);
}

/**
 * \fn AbstractColumn *AbstractFilter::output(int port=0)
 * \brief Get the data source associated with the specified output port.
 *
 * The returned pointer may be 0 even for valid port numbers, for example if not all required
 * input ports have been connected.
 */

/**
 * \fn const AbstractColumn *AbstractFilter::output(int port=0) const
 * \brief Overloaded method for const access.
 */

/**
 * \brief Return the input port to which the column is connected or -1 if it's not connected
 */
int AbstractFilter::portIndexOf(const AbstractColumn* column) {
	for (int i = 0; i < m_inputs.size(); ++i)
		if (m_inputs.at(i) == column) return i;
	return -1;
}

/**
 * \brief Give implementations a chance to reject connections to their input ports.
 *
 * If not reimplemented, all connections to ports within [0, inputCount()-1] will be accepted.
 */
bool AbstractFilter::inputAcceptable(int port, const AbstractColumn* source) {
	Q_UNUSED(port); Q_UNUSED(source); return true;
}

/**
 * \brief Called whenever an input is disconnected or deleted.
 *
 * This is only to notify implementations of the event, the default implementation is a
 * no-op.
 */
void AbstractFilter::inputAboutToBeDisconnected(const AbstractColumn* source) {
	Q_UNUSED(source);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//!\name signal handlers
//@{
////////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * \brief Name and/or comment of an input will be changed.
 *
 * \param source is always the this pointer of the column that emitted the signal.
 */
void AbstractFilter::inputDescriptionAboutToChange(const AbstractColumn* source) {
	Q_UNUSED(source);
} 

void AbstractFilter::inputDescriptionAboutToChange(const AbstractAspect* aspect) {
	const auto* col = qobject_cast<const AbstractColumn*>(aspect);
	if (col) inputDescriptionAboutToChange(col);
}

/**
 * \brief Name and/or comment of an input changed.
 *
 * \param source is always the this pointer of the column that emitted the signal.
 */
void AbstractFilter::inputDescriptionChanged(const AbstractColumn* source) {
	Q_UNUSED(source);
}

void AbstractFilter::inputDescriptionChanged(const AbstractAspect* aspect) {
	const auto* col = qobject_cast<const AbstractColumn*>(aspect);
	if (col && m_inputs.contains(col)) inputDescriptionChanged(col);
}

/**
 * \brief The plot designation of an input is about to change.
 *
 * \param source is always the this pointer of the column that emitted the signal.
 */
void AbstractFilter::inputPlotDesignationAboutToChange(const AbstractColumn* source) {
	Q_UNUSED(source);
}

/**
 * \brief The plot designation of an input changed.
 *
 * \param source is always the this pointer of the column that emitted the signal.
 */
void AbstractFilter::inputPlotDesignationChanged(const AbstractColumn* source) {
	Q_UNUSED(source);
}

/**
 * \brief The display mode and possibly the data type of an input is about to change.
 *
 * \param source is always the this pointer of the column that emitted the signal.
 */
void AbstractFilter::inputModeAboutToChange(const AbstractColumn* source) {
	Q_UNUSED(source);
}

/**
 * \brief The display mode and possibly the data type has changed.
 *
 * \param source is always the this pointer of the column that emitted the signal.
 */
void AbstractFilter::inputModeChanged(const AbstractColumn* source) {
	Q_UNUSED(source);
}

/**
 * \brief The data of an input is about to change.
 *
 * \param source is always the this pointer of the column that emitted the signal.
 */
void AbstractFilter::inputDataAboutToChange(const AbstractColumn* source) { 
	Q_UNUSED(source);
}

/**
 * \brief The data of an input has changed.
 *
 * \param source is always the this pointer of the column that emitted the signal.
 */
void AbstractFilter::inputDataChanged(const AbstractColumn* source) {
	Q_UNUSED(source);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//@}
////////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * \var AbstractFilter::m_inputs
 * \brief The data sources connected to my input ports.
 */

