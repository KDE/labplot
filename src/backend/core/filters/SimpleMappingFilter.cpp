/***************************************************************************
    File                 : SimpleMappingFilter.cpp
    Project              : SciDAVis
    --------------------------------------------------------------------
    Copyright            : (C) 2007 by Tilman Benkert
    Email                : thzs@gmx.net
    Description          : Filter that maps rows indices of a column
                           
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

// TODO: SimpleMappingFilter is broken since we changed the filter design the last time
#include "SimpleMappingFilter.h"
#include "Interval.h"
#include "IntervalAttribute.h"
#include <QStringList>

void SimpleMappingFilter::addMapping(int src_row, int dest_row)
{
	int dest_pos = m_dest_rows.indexOf(dest_row);

	emit m_output_column->dataAboutToChange(m_output_column);
	if(dest_pos != -1)
	{
		m_source_rows.removeAt(dest_pos);
		m_dest_rows.removeAt(dest_pos);
	}

	int src_pos = m_source_rows.indexOf(src_row);
	if(src_pos != -1)
	{
		m_source_rows.removeAt(src_pos);
		m_dest_rows.removeAt(src_pos);
	}
	m_source_rows.append(src_row);
	m_dest_rows.append(dest_row);
	emit m_output_column->dataChanged(m_output_column);
}

void SimpleMappingFilter::removeMappingTo(int dest_row)
{
	emit m_output_column->dataAboutToChange(m_output_column);
	int dest_pos = m_dest_rows.indexOf(dest_row);

	if(dest_pos != -1)
	{
		m_source_rows.removeAt(dest_pos);
		m_dest_rows.removeAt(dest_pos);
	}
	emit m_output_column->dataChanged(m_output_column);
}

void SimpleMappingFilter::removeMappingFrom(int src_row)
{
	emit m_output_column->dataAboutToChange(m_output_column);
	int src_pos = m_source_rows.indexOf(src_row);

	if(src_pos != -1)
	{
		m_source_rows.removeAt(src_pos);
		m_dest_rows.removeAt(src_pos);
	}
	emit m_output_column->dataChanged(m_output_column);
}

void SimpleMappingFilter::clearMappings() 
{ 
	m_source_rows.clear(); 
	m_dest_rows.clear(); 
}

bool SimpleMappingFilter::isReadOnly() const 
{ 
	return m_inputs.value(0) ? m_inputs.at(0)->isReadOnly() : true; 
}

void SimpleMappingFilter::setColumnMode(SciDAVis::ColumnMode mode)
{
	if(m_inputs.value(0) && !isReadOnly()) m_inputs[0]->setColumnMode(mode); 
}


bool SimpleMappingFilter::copy(const AbstractColumn * source, int source_start, int dest_start, int num_rows)
{
	if(!m_inputs.value(0)) return false;
	if(isReadOnly()) return false;
	for(int i=0; i<num_rows; i++)
	{
		int index = m_dest_rows.indexOf(dest_start+i);
		if(index == -1) continue;
		if(!m_inputs.at(0)->copy(source, source_start+i, m_source_rows.at(index), 1)) return false;
	}		

	return true;
};

bool SimpleMappingFilter::copy(const AbstractColumn * other)
{
	if(!m_inputs.value(0)) return false;
	if(isReadOnly()) return false;
	return copy(other, 0, 0, other->rowCount());
}

void SimpleMappingFilter::inputRowsAboutToBeInserted(AbstractColumn * source, int before, int count)
{
	Q_UNUSED(source);
	Q_UNUSED(before);
	Q_UNUSED(count);
}

void SimpleMappingFilter::inputRowsInserted(AbstractColumn * source, int before, int count)
{
	Q_UNUSED(source);
	for(int i=0; i<m_source_rows.count(); i++)
	{
		int src_row = m_source_rows.at(i);
		if(src_row >= before) 
			m_source_rows.replace(i, src_row + count);
	}
}

void SimpleMappingFilter::inputRowsAboutToBeRemoved(AbstractColumn * source, int first, int count)
{
	Q_UNUSED(source);
	for(int i=0; i<m_source_rows.count(); i++)
	{
		int src_row = m_source_rows.at(i);
		if(src_row >= first && src_row < first+count) 
		{
			removeMappingFrom(src_row);
			i--;
		}
	}
	for(int i=0; i<m_source_rows.count(); i++)
	{
		int src_row = m_source_rows.at(i);
		if(src_row >= first) 
			m_source_rows.replace(i, src_row - count);
	}
}

void SimpleMappingFilter::inputRowsRemoved(AbstractColumn * source, int first, int count)
{
	Q_UNUSED(source);
	Q_UNUSED(first);
	Q_UNUSED(count);
}

void SimpleMappingFilter::insertRows(int before, int count)
{
	if(!m_inputs.value(0)) return;
	if(isReadOnly()) return;
	int index = m_dest_rows.indexOf(before), src_row;
	if(index == -1 && before != rowCount()) return;

	if(before == rowCount())
	{
		index = m_dest_rows.indexOf(before-1);
		if(index == -1) return; // this happens when no mappings are defined
		src_row = m_source_rows.at(index)+1;
	}
	else
		src_row = m_source_rows.at(index);

	emit m_output_column->rowsAboutToBeInserted(m_output_column, before, count);
	m_inputs.at(0)->insertRows(src_row, count);

	for(int i=0; i<m_dest_rows.count(); i++)
	{
		int dest_row = m_dest_rows.at(i);
		if(dest_row >= before) 
			m_dest_rows.replace(i, dest_row + count);
	}

	for(int i=0; i<count; i++)
		addMapping(src_row+i, before+i);
	emit m_output_column->rowsInserted(m_output_column, before, count);
}

void SimpleMappingFilter::removeRows(int first, int count)
{
	if(!m_inputs.value(0)) return;
	if(isReadOnly()) return;
	emit m_output_column->rowsAboutToBeRemoved(m_output_column, first, count);
	for(int i=0; i<count; i++)
	{
		int index = m_dest_rows.indexOf(first+i);
		if(index == -1) continue;
		m_inputs.at(0)->removeRows(m_source_rows.at(index), 1);
	}		
	for(int i=0; i<m_dest_rows.count(); i++)
	{
		int dest_row = m_dest_rows.at(i);
		if(dest_row >= first) 
			m_dest_rows.replace(i, dest_row - count);
	}
	emit m_output_column->rowsRemoved(m_output_column, first, count);
}

void SimpleMappingFilter::setPlotDesignation(SciDAVis::PlotDesignation pd)
{
	if(!m_inputs.value(0)) return;
	if(isReadOnly()) return;
	m_inputs.at(0)->setPlotDesignation(pd);	
}

void SimpleMappingFilter::clear() 
{
	if(!m_inputs.value(0)) return;
	if(isReadOnly()) return;
	while(rowCount() > 0)
		removeRows(m_dest_rows.at(0), 1);
}

void SimpleMappingFilter::clearValidity() 
{
	if(!m_inputs.value(0)) return;
	if(isReadOnly()) return;
	for(int i=0; i<m_source_rows.count(); i++)
		m_inputs.at(0)->setInvalid(m_source_rows.at(i), false);
}

void SimpleMappingFilter::clearMasks() 
{
	if(!m_inputs.value(0)) return;
	if(isReadOnly()) return;
	for(int i=0; i<m_source_rows.count(); i++)
		m_inputs.at(0)->setMasked(m_source_rows.at(i), false);
}

void SimpleMappingFilter::setInvalid(Interval<int> i, bool invalid)
{
	if(!m_inputs.value(0)) return;
	if(isReadOnly()) return;
	for(int c=i.start(); c<=i.end(); c++)
		setInvalid(c, invalid);
}

void SimpleMappingFilter::setInvalid(int row, bool invalid)
{
	if(!m_inputs.value(0)) return;
	if(isReadOnly()) return;
	int index = m_dest_rows.indexOf(row);
	if(index == -1) return;
	m_inputs.at(0)->setInvalid(m_source_rows.at(index), invalid);
}

void SimpleMappingFilter::setMasked(Interval<int> i, bool mask)
{
	if(!m_inputs.value(0)) return;
	if(isReadOnly()) return;
	for(int c=i.start(); c<=i.end(); c++)
		setMasked(c, mask);
}

void SimpleMappingFilter::setMasked(int row, bool mask) 
{
	if(!m_inputs.value(0)) return;
	if(isReadOnly()) return;
	int index = m_dest_rows.indexOf(row);
	if(index == -1) return;
	m_inputs.at(0)->setMasked(m_source_rows.at(index), mask);
}

int SimpleMappingFilter::rowCount() const
{
	int max = -1;
	for(int i=0; i<m_dest_rows.count(); i++)
		if(m_dest_rows.at(i) > max) max = m_dest_rows.at(i);
	return max+1;
}

bool SimpleMappingFilter::isInvalid(int row) const
{
	if(!m_inputs.value(0)) return false;
	int index = m_dest_rows.indexOf(row);
	if(index == -1) return false;
	return m_inputs.at(0)->isInvalid(m_source_rows.at(index));
}

bool SimpleMappingFilter::isInvalid(Interval<int> i) const
{
	if(!m_inputs.value(0)) return false;
	for(int c=i.start(); c<=i.end(); c++)
		if(!isInvalid(c)) return false;
	return true;
}

QList< Interval<int> > SimpleMappingFilter::invalidIntervals() const
{
	IntervalAttribute<bool> attrib;	
	for(int i=0; i<m_dest_rows.count(); i++)
		attrib.setValue(Interval<int>(m_dest_rows.at(i),m_dest_rows.at(i)), isInvalid(m_dest_rows.at(i)));
	return attrib.intervals();
}

bool SimpleMappingFilter::isMasked(int row) const
{
	if(!m_inputs.value(0)) return false;
	int index = m_dest_rows.indexOf(row);
	if(index == -1) return false;
	return m_inputs.at(0)->isMasked(m_source_rows.at(index));
}

bool SimpleMappingFilter::isMasked(Interval<int> i) const
{
	if(!m_inputs.value(0)) return false;
	for(int c=i.start(); c<=i.end(); c++)
		if(!isMasked(c)) return false;
	return true;
}

QList< Interval<int> > SimpleMappingFilter::maskedIntervals() const
{
	IntervalAttribute<bool> attrib;	
	for(int i=0; i<m_dest_rows.count(); i++)
		attrib.setValue(Interval<int>(m_dest_rows.at(i),m_dest_rows.at(i)), isMasked(m_dest_rows.at(i)));
	return attrib.intervals();
}

QString SimpleMappingFilter::formula(int row) const
{
	if(!m_inputs.value(0)) QString();
	int index = m_dest_rows.indexOf(row);
	if(index == -1) QString();
	return m_inputs.at(0)->formula(m_source_rows.at(index));
}

QList< Interval<int> > SimpleMappingFilter::formulaIntervals() const
{
	IntervalAttribute<QString> attrib;	
	for(int i=0; i<m_dest_rows.count(); i++)
		attrib.setValue(Interval<int>(m_dest_rows.at(i),m_dest_rows.at(i)), formula(m_dest_rows.at(i)));
	return attrib.intervals();
}

void SimpleMappingFilter::setFormula(Interval<int> i, QString formula)
{
	if(!m_inputs.value(0)) return;
	if(isReadOnly()) return;
	for(int c=i.start(); c<=i.end(); c++)
		setFormula(c, formula);
}

void SimpleMappingFilter::setFormula(int row, QString formula)
{
	if(!m_inputs.value(0)) return;
	if(isReadOnly()) return;
	int index = m_dest_rows.indexOf(row);
	if(index == -1) return;
	m_inputs.at(0)->setFormula(m_source_rows.at(index), formula);
}

void SimpleMappingFilter::clearFormulas()
{
	if(!m_inputs.value(0)) return;
	if(isReadOnly()) return;
	for(int i=0; i<m_source_rows.count(); i++)
		m_inputs.at(0)->setFormula(m_source_rows.at(i), QString());

}

QString SimpleMappingFilter::textAt(int row) const
{
	if(!m_inputs.value(0)) return QString();
	int index = m_dest_rows.indexOf(row);
	if(index == -1) return QString();
	return m_inputs.at(0)->textAt(m_source_rows.at(index));
}

void SimpleMappingFilter::setTextAt(int row, const QString& new_value)
{
	if(!m_inputs.value(0)) return;
	if(isReadOnly()) return;
	int index = m_dest_rows.indexOf(row);
	if(index == -1) return;
	m_inputs.at(0)->setTextAt(m_source_rows.at(index), new_value);
}

void SimpleMappingFilter::replaceTexts(int first, const QStringList& new_values)
{ 
	if(!m_inputs.value(0)) return;
	if(isReadOnly()) return;
	int count = new_values.count();
	for(int i=0; i<count; i++)
		setTextAt(first+i, new_values.at(i));
}

QDate SimpleMappingFilter::dateAt(int row) const
{
	if(!m_inputs.value(0)) return QDate();
	int index = m_dest_rows.indexOf(row);
	if(index == -1) return QDate();
	return m_inputs.at(0)->dateAt(m_source_rows.at(index));
}

void SimpleMappingFilter::setDateAt(int row, const QDate& new_value) 
{
	if(!m_inputs.value(0)) return;
	if(isReadOnly()) return;
	int index = m_dest_rows.indexOf(row);
	if(index == -1) return;
	m_inputs.at(0)->setDateAt(m_source_rows.at(index), new_value);
}

QTime SimpleMappingFilter::timeAt(int row) const
{
	if(!m_inputs.value(0)) return QTime();
	int index = m_dest_rows.indexOf(row);
	if(index == -1) QTime();
	return m_inputs.at(0)->timeAt(m_source_rows.at(index));
}

void SimpleMappingFilter::setTimeAt(int row, const QTime& new_value)
{
	if(!m_inputs.value(0)) return;
	if(isReadOnly()) return;
	int index = m_dest_rows.indexOf(row);
	if(index == -1) return;
	m_inputs.at(0)->setTimeAt(m_source_rows.at(index), new_value);
}

QDateTime SimpleMappingFilter::dateTimeAt(int row) const
{
	if(!m_inputs.value(0)) return QDateTime();
	int index = m_dest_rows.indexOf(row);
	if(index == -1) return QDateTime();
	return m_inputs.at(0)->dateTimeAt(m_source_rows.at(index));
}

void SimpleMappingFilter::setDateTimeAt(int row, const QDateTime& new_value)
{
	if(!m_inputs.value(0)) return;
	if(isReadOnly()) return;
	int index = m_dest_rows.indexOf(row);
	if(index == -1) return;
	m_inputs.at(0)->setDateTimeAt(m_source_rows.at(index), new_value);
}

void SimpleMappingFilter::replaceDateTimes(int first, const QList<QDateTime>& new_values)
{
	if(!m_inputs.value(0)) return;
	if(isReadOnly()) return;
	int count = new_values.count();
	for(int i=0; i<count; i++)
		setDateTimeAt(first+i, new_values.at(i));
}

double SimpleMappingFilter::valueAt(int row) const 
{
	if(!m_inputs.value(0)) return 0.0;
	int index = m_dest_rows.indexOf(row);
	if(index == -1) return 0.0;
	return m_inputs.at(0)->valueAt(m_source_rows.at(index));
}

void SimpleMappingFilter::setValueAt(int row, double new_value) 
{
	if(!m_inputs.value(0)) return;
	if(isReadOnly()) return;
	int index = m_dest_rows.indexOf(row);
	if(index == -1) return;
	m_inputs.at(0)->setValueAt(m_source_rows.at(index), new_value);
}

void SimpleMappingFilter::replaceValues(int first, const QVector<double>& new_values)
{
	if(!m_inputs.value(0)) return;
	if(isReadOnly()) return;
	int count = new_values.count();
	for(int i=0; i<count; i++)
		setValueAt(first+i, new_values.at(i));
}

void SimpleMappingFilter::save(QXmlStreamWriter * writer) const
{
	writer->writeStartElement("simple_filter");
	writer->writeAttribute("filter_name", "SimpleMappingFilter");
	for(int i=0; i<m_source_rows.size(); i++)
	{
		writer->writeStartElement("mapping");
		writer->writeAttribute("source_row", QString::number(m_source_rows.at(i)));
		writer->writeAttribute("destination_row", QString::number(m_dest_rows.at(i)));
		writer->writeEndElement();	
	}
	writer->writeEndElement();
}

bool SimpleMappingFilter::load(QXmlStreamReader * reader)
{
	QString prefix(tr("XML read error: ","prefix for XML error messages"));
	QString postfix(tr(" (loading failed)", "postfix for XML error messages"));

	clearMappings();

	if(reader->isStartElement() && reader->name() == "simple_filter") 
	{
		QXmlStreamAttributes attribs = reader->attributes();
		QString str = attribs.value(reader->namespaceUri().toString(), "filter_name").toString();
		if(str != "SimpleMappingFilter")
			reader->raiseError(prefix+tr("incompatible filter type")+postfix);

		// read child elements
		while (!reader->atEnd()) 
		{
			reader->readNext();

			if (reader->isEndElement()) break;

			if (reader->isStartElement()) 
			{
				if(reader->name() == "mapping")
				{
					QXmlStreamAttributes attribs = reader->attributes();
					QString src_str = attribs.value(reader->namespaceUri().toString(), "source_row").toString();
					QString dest_str = attribs.value(reader->namespaceUri().toString(), "destination_row").toString();
					bool ok1, ok2;
					int src_row = src_str.toInt(&ok1);
					int dest_row = dest_str.toInt(&ok2);
					if( !ok1 || !ok2 )
					{
						reader->raiseError(prefix+tr("invalid mapping element")+postfix);
						return false;
					}
					else
						addMapping(src_row, dest_row);	
					reader->readNext(); // read end element of mapping
				}
				else
					reader->readElementText(); // unknown element
			} 
		}
		reader->readNext(); // read end element of simple_filter
	}
	else
		reader->raiseError(prefix+tr("no simple filter element found")+postfix);

	return !reader->error();
}
