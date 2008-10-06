/***************************************************************************
    File                 : ReadOnlyTableModel.h
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

#include <QAbstractItemModel>
#include "AbstractFilter.h"

//! Model to display the output of a filter
/**
 * This model is used to display the output of
 * a filter in a table. It is read-only for the
 * user. Only the corresponding filter can change 
 * the data.
 */
class ReadOnlyTableModel : public QAbstractItemModel, public AbstractFilter
{
	Q_OBJECT
	public:
	~ReadOnlyTableModel();

	//! \name Reimplemented from QAbstractItemModel
	//@{
	Qt::ItemFlags flags( const QModelIndex & index ) const;
	QVariant data(const QModelIndex &index, int role) const;
	QVariant headerData(int section, Qt::Orientation orientation, int role) const;
	int rowCount(const QModelIndex &) const;
	int columnCount(const QModelIndex &) const;
	QModelIndex index(int row, int column, const QModelIndex &parent) const;
	QModelIndex parent(const QModelIndex &) const;
	//@}

	//! \name Reimplemented from AbstractFilter
	//@{
	virtual int inputCount() const;
	virtual int outputCount() const;
	virtual AbstractDataSource* output(int) const;

	protected:
	virtual bool inputAcceptable(int port, AbstractDataSource *source);
	virtual void inputDataChanged(int port);
	virtual void inputDescriptionChanged(int port);
	//@}

	private:
	//! Filters used for converting the data received to QString.
	QList<AbstractFilter*> m_output_filters;
	//! Constructs a <type of source> -> QString filter and connects its first input to source.
	AbstractFilter *newOutputFilterFor(AbstractDataSource *source);
};

