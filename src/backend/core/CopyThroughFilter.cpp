/*
    File                 : CopyThroughFilter.cpp
    Project              : SciDAVis
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2007 Knut Franke <knut.franke*gmx.de (use @ for *)>
    Description          : Filter which copies all provided inputs unaltered
    to an equal number of outputs.
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "CopyThroughFilter.h"

/**
 * \class CopyThroughFilter
 * \brief Filter which copies all provided inputs unaltered to an equal number of outputs.
 *
 * This is probably the simplest filter you can possibly write.
 * It accepts an arbitrary number of inputs and provides the same AbstractColumn objects
 * as outputs again.
 */

/**
 * \brief Accept any number of inputs.
 */
int CopyThroughFilter::inputCount() const {
	return -1;
}

/**
 * \brief Provide as many output ports as inputs have been connected.
 */
int CopyThroughFilter::outputCount() const {
	return m_inputs.size();
}

/**
 * \brief When asked for an output port, just return the corresponding input port.
 */
AbstractColumn *CopyThroughFilter::output(int port) const {
	return 0;
	//TODO: return m_inputs.value(port);
}

