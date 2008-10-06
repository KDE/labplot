/***************************************************************************
    File                 : AbstractSimpleFilter.cpp
    Project              : SciDAVis
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
#include <QtDebug>

// TODO: should simple filters have a name argument?
AbstractSimpleFilter::AbstractSimpleFilter()
	: AbstractFilter("SimpleFilter"), m_output_column(new SimpleFilterColumn(this))
{
	addChild(m_output_column);
}

void AbstractSimpleFilter::clearMasks()
{
	emit m_output_column->maskingAboutToChange(m_output_column);
	m_masking.clear();
	emit m_output_column->maskingChanged(m_output_column);
}

void AbstractSimpleFilter::setMasked(Interval<int> i, bool mask)
{
	emit m_output_column->maskingAboutToChange(m_output_column);
	m_masking.setValue(i, mask);
	emit m_output_column->maskingChanged(m_output_column);
}

void AbstractSimpleFilter::inputPlotDesignationAboutToChange(const AbstractColumn*)
{
	emit m_output_column->plotDesignationAboutToChange(m_output_column);
}

void AbstractSimpleFilter::inputPlotDesignationChanged(const AbstractColumn*)
{
	emit m_output_column->plotDesignationChanged(m_output_column);
}

void AbstractSimpleFilter::inputModeAboutToChange(const AbstractColumn*)
{
	emit m_output_column->dataAboutToChange(m_output_column);
}

void AbstractSimpleFilter::inputModeChanged(const AbstractColumn*)
{
	emit m_output_column->dataChanged(m_output_column);
}

void AbstractSimpleFilter::inputDataAboutToChange(const AbstractColumn*)
{
	emit m_output_column->dataAboutToChange(m_output_column);
}

void AbstractSimpleFilter::inputDataChanged(const AbstractColumn*)
{
	emit m_output_column->dataChanged(m_output_column);
}

void AbstractSimpleFilter::inputRowsAboutToBeInserted(const AbstractColumn * source, int before, int count)
{
	Q_UNUSED(source);
	Q_UNUSED(count);
	foreach(Interval<int> output_range, dependentRows(Interval<int>(before, before)))
		emit m_output_column->rowsAboutToBeInserted(m_output_column, output_range.start(), output_range.size());
}

void AbstractSimpleFilter::inputRowsInserted(const AbstractColumn * source, int before, int count)
{
	Q_UNUSED(source);
	Q_UNUSED(count);
	foreach(Interval<int> output_range, dependentRows(Interval<int>(before, before)))
		emit m_output_column->rowsInserted(m_output_column, output_range.start(), output_range.size());
}

void AbstractSimpleFilter::inputRowsAboutToBeRemoved(const AbstractColumn * source, int first, int count)
{
	Q_UNUSED(source);
	foreach(Interval<int> output_range, dependentRows(Interval<int>(first, first+count-1)))
		emit m_output_column->rowsAboutToBeRemoved(m_output_column, output_range.start(), output_range.size());
}

void AbstractSimpleFilter::inputRowsRemoved(const AbstractColumn * source, int first, int count)
{
	Q_UNUSED(source);
	foreach(Interval<int> output_range, dependentRows(Interval<int>(first, first+count-1)))
		emit m_output_column->rowsRemoved(m_output_column, output_range.start(), output_range.size());
}

AbstractColumn *AbstractSimpleFilter::output(int port)
{
	return port == 0 ? static_cast<AbstractColumn*>(m_output_column) : 0;
}

const AbstractColumn *AbstractSimpleFilter::output(int port) const
{
	return port == 0 ? static_cast<const AbstractColumn*>(m_output_column) : 0;
}

void AbstractSimpleFilter::save(QXmlStreamWriter * writer) const 
{
	writer->writeStartElement("simple_filter");
	writeBasicAttributes(writer);
	writeExtraAttributes(writer);
	writer->writeAttribute("filter_name", metaObject()->className());
	writeCommentElement(writer);
	writer->writeEndElement();
}

bool AbstractSimpleFilter::load(XmlStreamReader * reader)
{
	if(reader->isStartElement() && reader->name() == "simple_filter") 
	{
		if (!readBasicAttributes(reader)) return false;

		QXmlStreamAttributes attribs = reader->attributes();
		QString str = attribs.value(reader->namespaceUri().toString(), "filter_name").toString();
		if(str != metaObject()->className())
		{
			reader->raiseError(tr("incompatible filter type"));
			return false;
		}

		// read child elements
		while (!reader->atEnd()) 
		{
			reader->readNext();

			if (reader->isEndElement()) break;

			if (reader->isStartElement()) 
			{
				if (reader->name() == "comment")
				{
					if (!readCommentElement(reader)) return false;
				}
				else // unknown element
				{
					reader->raiseWarning(tr("unknown element '%1'").arg(reader->name().toString()));
					if (!reader->skipToEndElement()) return false;
				}
			} 
		}
	}
	else
		reader->raiseError(tr("no simple filter element found"));

	return !reader->hasError();
}

