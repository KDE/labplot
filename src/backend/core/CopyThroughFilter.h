/***************************************************************************
    File                 : CopyThroughFilter.h
    Project              : SciDAVis
    --------------------------------------------------------------------
    Copyright            : (C) 2007 by Knut Franke
    Email (use @ for *)  : knut.franke*gmx.de
    Description          : Filter which copies all provided inputs unaltered
                           to an equal number of outputs.

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
#ifndef COPY_THROUGH_FILTER_H
#define COPY_THROUGH_FILTER_H

#include "AbstractFilter.h"

/**
 * \brief Filter which copies all provided inputs unaltered to an equal number of outputs.
 *
 * This is probably the simplest filter you can possibly write.
 * It accepts an arbitrary number of inputs and provides the same AbstractColumn objects
 * as outputs again.
 */
class CopyThroughFilter : public AbstractFilter
{
	public:
	//! Accept any number of inputs.
	virtual int inputCount() const { return -1; }
	//! Provide as many output ports as inputs have been connected.
	virtual int outputCount() const { return m_inputs.size(); }
	//! When asked for an output port, just return the corresponding input port.
	virtual AbstractColumn* output(int port) const { return m_inputs.value(port); }
};

#endif // ifndef COPY_THROUGH_FILTER_H

