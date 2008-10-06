/***************************************************************************
    File                 : MatrixModel.cpp
    Project              : SciDAVis
    Description          : Model for the access to a Matrix
    --------------------------------------------------------------------
    Copyright            : (C) 2008 Tilman Benkert (thzs*gmx.net)
                           (replace * with @ in the email addresses) 

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

#include "matrix/Matrix.h"
#include "matrix/MatrixModel.h"
#include <QString>
#include <QBrush>
#include <QColor>

MatrixModel::MatrixModel(Matrix * matrix)
	: QAbstractItemModel(0), m_matrix(matrix)
{
	connect(m_matrix, SIGNAL(columnsAboutToBeInserted(int, int)),
			this, SLOT(handleColumnsAboutToBeInserted(int, int)));
	connect(m_matrix, SIGNAL(columnsInserted(int, int)),
			this, SLOT(handleColumnsInserted(int, int)));
	connect(m_matrix, SIGNAL(columnsAboutToBeRemoved(int, int)),
			this, SLOT(handleColumnsAboutToBeRemoved(int, int)));
	connect(m_matrix, SIGNAL(columnsRemoved(int, int)),
			this, SLOT(handleColumnsRemoved(int, int)));
	connect(m_matrix, SIGNAL(rowsAboutToBeInserted(int, int)),
			this, SLOT(handleRowsAboutToBeInserted(int, int)));
	connect(m_matrix, SIGNAL(rowsInserted(int, int)),
			this, SLOT(handleRowsInserted(int, int)));
	connect(m_matrix, SIGNAL(rowsAboutToBeRemoved(int, int)),
			this, SLOT(handleRowsAboutToBeRemoved(int, int)));
	connect(m_matrix, SIGNAL(rowsRemoved(int, int)),
			this, SLOT(handleRowsRemoved(int, int)));
	connect(m_matrix, SIGNAL(dataChanged(int, int, int, int)),
			this, SLOT(handleDataChanged(int, int, int, int)));
	connect(m_matrix, SIGNAL(coordinatesChanged()),
			this, SLOT(handleCoordinatesChanged()));
	connect(m_matrix, SIGNAL(formatChanged()),
			this, SLOT(handleFormatChanged()));
}

MatrixModel::~MatrixModel()
{
}

Qt::ItemFlags MatrixModel::flags(const QModelIndex & index ) const
{
	if (index.isValid())
		return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable;
	else
		return Qt::ItemIsEnabled;
}

QVariant MatrixModel::data(const QModelIndex &index, int role) const
{
	if( !index.isValid() )
		return QVariant();
	
	int row = index.row();
	int col = index.column();

	switch(role)
	{
		case Qt::ToolTipRole:
		case Qt::EditRole:
		case Qt::DisplayRole:
			return QVariant(m_matrix->text(row, col));
		case Qt::BackgroundRole:
			return QVariant(QBrush(QColor(0xff,0xff,0x77))); // yellow color to distinguish a matrix from a table
	}

	return QVariant();
}

QVariant MatrixModel::headerData(int section, Qt::Orientation orientation, int role) const
{
	QString result;
	switch(orientation) 
	{
		case Qt::Horizontal:
			switch(role) 
			{
				case Qt::DisplayRole:
				case Qt::ToolTipRole:
					result += QString::number(section+1) + QString(" (");
					double diff = m_matrix->xEnd() - m_matrix->xStart();
					double step = 0.0;
					if (m_matrix->columnCount() > 1)
						step = diff/double(m_matrix->columnCount()-1);
					result += QLocale().toString(m_matrix->xStart()+double(section)*step, 
							m_matrix->numericFormat(), m_matrix->displayedDigits());

					result += QString(")");
					return QVariant(result);
			}
		case Qt::Vertical:
			switch(role) 
			{
				case Qt::DisplayRole:
				case Qt::ToolTipRole:
					result += QString::number(section+1) + QString(" (");
					double diff = m_matrix->yEnd() - m_matrix->yStart();
					double step = 0.0;
					if (m_matrix->rowCount() > 1)
						step = diff/double(m_matrix->rowCount()-1);
					// TODO: implement decent double == 0 check
					if (diff < 1e-10)
						result += QLocale().toString(m_matrix->yStart(), 
								m_matrix->numericFormat(), m_matrix->displayedDigits());
					else
						result += QLocale().toString(m_matrix->yStart()+double(section)*step, 
								m_matrix->numericFormat(), m_matrix->displayedDigits());

					result += QString(")");
					return QVariant(result);
			}
	}
	return QVariant();
}

int MatrixModel::rowCount(const QModelIndex &parent) const
{
	Q_UNUSED(parent)
	return m_matrix->rowCount();
}

int MatrixModel::columnCount(const QModelIndex & parent) const
{
	Q_UNUSED(parent)
	return m_matrix->columnCount();
}

bool MatrixModel::setData(const QModelIndex & index, const QVariant & value, int role)
{
	if (!index.isValid())
		return false;

	int row = index.row();
	int column = index.column();

	if(role ==  Qt::EditRole)
	{
			m_matrix->setCell(row, column, value.toDouble());
			return true;
	}
	return false;
}

QModelIndex MatrixModel::index(int row, int column, const QModelIndex &parent) const
{
	Q_UNUSED(parent)
	return createIndex(row, column);
}

QModelIndex MatrixModel::parent(const QModelIndex & child) const
{
	Q_UNUSED(child)
    return QModelIndex();
}

void MatrixModel::handleColumnsAboutToBeInserted(int before, int count)
{
	beginInsertColumns(QModelIndex(), before, before+count-1);
}

void MatrixModel::handleColumnsInserted(int first, int count)
{
	Q_UNUSED(first)
	Q_UNUSED(count)
	endInsertColumns();
}

void MatrixModel::handleColumnsAboutToBeRemoved(int first, int count)
{
	beginRemoveColumns(QModelIndex(), first, first+count-1);
}

void MatrixModel::handleColumnsRemoved(int first, int count)
{
	Q_UNUSED(first)
	Q_UNUSED(count)
	endRemoveColumns();	 
}

void MatrixModel::handleRowsAboutToBeInserted(int before, int count)
{
	beginInsertRows(QModelIndex(), before, before+count-1);
}

void MatrixModel::handleRowsInserted(int first, int count)
{
	Q_UNUSED(first)
	Q_UNUSED(count)
	endInsertRows();
}

void MatrixModel::handleRowsAboutToBeRemoved(int first, int count)
{
	beginRemoveRows(QModelIndex(), first, first+count-1);
}

void MatrixModel::handleRowsRemoved(int first, int count)
{
	Q_UNUSED(first)
	Q_UNUSED(count)
	endRemoveRows();	 
}

void MatrixModel::handleDataChanged(int top, int left, int bottom, int right)
{
	emit dataChanged(index(top, left), index(bottom, right));
}

void MatrixModel::handleCoordinatesChanged()
{
	emit headerDataChanged(Qt::Horizontal, 0, columnCount()-1); 
	emit headerDataChanged(Qt::Vertical, 0, rowCount()-1); 
}

void MatrixModel::handleFormatChanged()
{
	handleCoordinatesChanged();
	handleDataChanged(0, 0, rowCount()-1, columnCount()-1); 
}

