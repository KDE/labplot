/***************************************************************************
    File                 : AbstractFilter.cpp
    Project              : SciDAVis
    --------------------------------------------------------------------
    Copyright            : (C) 2007 by Knut Franke, Tilman Benkert
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
#include "AbstractFilter.h"

bool AbstractFilter::input(int port, const AbstractColumn* source)
{
	if (port<0 || (inputCount()>=0 && port>=inputCount())) return false;
	if (source && !inputAcceptable(port, source)) return false;
	if (port >= m_inputs.size()) m_inputs.resize(port+1);
	const AbstractColumn* old_input = m_inputs.value(port);
	if (source == old_input) return true;
	if (old_input) 
	{
		disconnect(old_input, 0, this, 0);
		// replace input, notifying the filter implementation of the changes
		inputDescriptionAboutToChange(old_input);
		inputPlotDesignationAboutToChange(old_input);
		inputMaskingAboutToChange(old_input);
		inputDataAboutToChange(old_input);
		if(source && source->columnMode() != old_input->columnMode())
			inputModeAboutToChange(old_input);
	}
	if (!source)
		inputAboutToBeDisconnected(old_input);
	m_inputs[port] = source;
	if (source) { // we have a new source
		if(old_input && source->columnMode() != old_input->columnMode())
			inputModeAboutToChange(source);
		inputDataChanged(source);
		inputMaskingChanged(source);
		inputPlotDesignationChanged(source);
		inputDescriptionChanged(source);
		// connect the source's signals
		QObject::connect(source, SIGNAL(aspectDescriptionAboutToChange(const AbstractAspect *)),
				this, SLOT(inputDescriptionAboutToChange(const AbstractAspect *)));
		QObject::connect(source, SIGNAL(aspectDescriptionChanged(const AbstractAspect *)),
				this, SLOT(inputDescriptionChanged(const AbstractAspect *)));
		QObject::connect(source, SIGNAL(plotDesignationAboutToChange(const AbstractColumn *)),
				this, SLOT(inputPlotDesignationAboutToChange(const AbstractColumn *)));
		QObject::connect(source, SIGNAL(plotDesignationChanged(const AbstractColumn *)),
				this, SLOT(inputPlotDesignationChanged(const AbstractColumn *)));
		QObject::connect(source, SIGNAL(modeAboutToChange(const AbstractColumn *)),
				this, SLOT(inputModeAboutToChange(const AbstractColumn *)));
		QObject::connect(source, SIGNAL(modeChanged(const AbstractColumn *)),
				this, SLOT(inputModeChanged(const AbstractColumn *)));
		QObject::connect(source, SIGNAL(dataAboutToChange(const AbstractColumn *)),
				this, SLOT(inputDataAboutToChange(const AbstractColumn *)));
		QObject::connect(source, SIGNAL(dataChanged(const AbstractColumn *)),
				this, SLOT(inputDataChanged(const AbstractColumn *)));
		QObject::connect(source, SIGNAL(aboutToBeReplaced(const AbstractColumn *,const AbstractColumn*)),
				this, SLOT(inputAboutToBeReplaced(const AbstractColumn *,const AbstractColumn*)));
		QObject::connect(source, 
			SIGNAL(rowsAboutToBeInserted(const AbstractColumn *,int,int)),
			this, SLOT(inputRowsAboutToBeInserted(const AbstractColumn *,int,int)));
		QObject::connect(source, 
			SIGNAL(rowsInserted(const AbstractColumn *,int,int)),
			this, SLOT(inputRowsInserted(const AbstractColumn *,int,int)));
		QObject::connect(source, 
			SIGNAL(rowsAboutToBeRemoved(const AbstractColumn *,int,int)),
			this, SLOT(inputRowsAboutToBeRemoved(const AbstractColumn *,int,int)));
		QObject::connect(source, 
			SIGNAL(rowsRemoved(const AbstractColumn *, int, int)),
			this, SLOT(inputRowsRemoved(const AbstractColumn *,int,int)));
		QObject::connect(source, SIGNAL(maskingAboutToChange(const AbstractColumn *)),
				this, SLOT(inputMaskingAboutToChange(const AbstractColumn *)));
		QObject::connect(source, SIGNAL(maskingChanged(const AbstractColumn *)),
				this, SLOT(inputMaskingChanged(const AbstractColumn *)));
		QObject::connect(source, SIGNAL(aboutToBeDestroyed(const AbstractColumn *)),
				this, SLOT(inputAboutToBeDestroyed(const AbstractColumn *)));
	} else { // source==0, that is, the input port has been disconnected
		// try to shrink m_inputs
		int num_connected_inputs = m_inputs.size();
		while ( m_inputs.at(num_connected_inputs-1) == 0 )
		{
			num_connected_inputs--;
			if(!num_connected_inputs) break;
		}
		m_inputs.resize(num_connected_inputs);
	}
	return true;
}

bool AbstractFilter::input(const AbstractFilter* sources)
{
	if (!sources) return false;
	bool result = true;
	for (int i=0; i<sources->outputCount(); i++)
		if (!input(i, sources->output(i)))
			result = false;
	return result;
}

QString AbstractFilter::inputLabel(int port) const
{
	return QObject::tr("In%1").arg(port + 1);
}

void AbstractFilter::inputAboutToBeReplaced(const AbstractColumn * source, const AbstractColumn* replacement)
{
	input(portIndexOf(source), replacement);
}

