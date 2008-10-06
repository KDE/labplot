/***************************************************************************
    File                 : ReadOnlyTableModel.cpp
    Project              : SciDAVis
    --------------------------------------------------------------------
    Copyright            : (C) 2007 by Tilman Benkert,
                           Knut Franke
    Email (use @ for *)  : thzs*gmx.net,
                           knut.franke*gmx.de
    Description          : Model to display the output of a filter

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

#include "ReadOnlyTableModel.h"
#include "CopyThroughFilter.h"
#include "datatypes/AbstractStringDataSource.h"
#include "datatypes/Double2StringFilter.h"
#include "datatypes/DateTime2StringFilter.h"

ReadOnlyTableModel::~ReadOnlyTableModel()
{
	foreach(AbstractFilter *i, m_output_filters)
		if (i) delete i;
}

Qt::ItemFlags ReadOnlyTableModel::flags(const QModelIndex & index ) const
{
	if (index.isValid())
		return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
	else
		return Qt::ItemIsEnabled;
}


QVariant ReadOnlyTableModel::data(const QModelIndex &index, int role) const
{
	if( !index.isValid() ||
			((role != Qt::DisplayRole) && (role != Qt::EditRole) && (role != Qt::ToolTipRole) ) ||
			!m_output_filters.value(index.column()) )
		return QVariant();

	return QVariant(static_cast<AbstractStringDataSource*>(m_output_filters.at(index.column())->output(0))->textAt(index.row()));
}




QVariant ReadOnlyTableModel::headerData(int section, Qt::Orientation orientation, int role) const 
{
	if ((role != Qt::DisplayRole) && (role != Qt::EditRole) && (role != Qt::ToolTipRole) )
		return QVariant();
	switch(orientation) 
	{
		case Qt::Horizontal: 
			return m_inputs.value(section) ? m_inputs.at(section)->label() : QVariant();
		case Qt::Vertical: 
			return QVariant(QString::number(section+1));
	}
}

int ReadOnlyTableModel::rowCount(const QModelIndex &) const 
{
	int rows, result = 0;
	foreach(AbstractDataSource *i, m_inputs) 
	{
		if (!i) continue;
		if ((rows = i->rowCount()) > result)
			result = rows;
	}
	return result;
}

int ReadOnlyTableModel::columnCount(const QModelIndex &) const 
{ 
	return m_inputs.size(); 
}

QModelIndex ReadOnlyTableModel::index(int row, int column, const QModelIndex &parent) const 
{
	Q_UNUSED(parent)
	return createIndex(row, column);
}

QModelIndex ReadOnlyTableModel::parent(const QModelIndex &) const
{ 
	return QModelIndex(); 
}

int ReadOnlyTableModel::inputCount() const 
{ 	
	return -1; 
}

bool ReadOnlyTableModel::inputAcceptable(int, AbstractDataSource *source)
{
	return source->inherits("AbstractStringDataSource") ||
		source->inherits("AbstractDoubleDataSource") ||
		source->inherits("AbstractDateTimeDataSource");
}

int ReadOnlyTableModel::outputCount() const 
{ 
	return 0; 
}

AbstractDataSource* ReadOnlyTableModel::output(int) const 
{ 
	return 0; 
}

void ReadOnlyTableModel::inputDataChanged(int port) 
{
	if (port >= m_output_filters.size())
		m_output_filters.insert(port, 0);
	AbstractFilter *old_filter = m_output_filters.at(port);
	if (m_inputs.at(port)) {
		if (old_filter) {
			// input is connected and there's already a filter for it
			if (!old_filter->input(0, m_inputs.at(port))) {
				// can't connect => type of input changed
				delete old_filter;
				m_output_filters[port] = newOutputFilterFor(m_inputs.at(port));
			}
		} else // just create a new filter for the input
			m_output_filters[port] = newOutputFilterFor(m_inputs.at(port));
	} else {
		// input disconnected, therefore we delete its filter
		if (old_filter)
			delete old_filter;
		// shrink m_output_filters to size of m_inputs
		for (int i=m_output_filters.size(); i>m_inputs.size(); i--)
			m_output_filters.removeLast();
	}
	emit dataChanged(createIndex(0,port), createIndex(m_inputs[port]->rowCount()-1,port));
}

AbstractFilter *ReadOnlyTableModel::newOutputFilterFor(AbstractDataSource *source)
{
	AbstractFilter *result = 0;
	if (source->inherits("AbstractStringDataSource"))
		result = new CopyThroughFilter();
	else if (source->inherits("AbstractDoubleDataSource"))
		result = new Double2StringFilter();
	else if (source->inherits("AbstractDateTimeDataSource"))
		result = new DateTime2StringFilter();
	else
		return 0;
	result->input(0, source);
	return result;
}

void ReadOnlyTableModel::inputDescriptionChanged(int port) 
{
	emit headerDataChanged(Qt::Horizontal, port, port);
}


