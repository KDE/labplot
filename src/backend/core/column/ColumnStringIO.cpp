/***************************************************************************
    File                 : ColumnStringIO.cpp
    Project              : LabPlot
    Description          : String-IO interface of Column.
    --------------------------------------------------------------------
    Copyright            : (C) 2007-2009 Tilman Benkert (thzs@gmx.net)
    Copyright            : (C) 2013-2017 by Alexander Semke (alexander.semke@web.de)

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

#include "backend/core/column/Column.h"
#include "backend/core/column/ColumnPrivate.h"
#include "backend/core/column/ColumnStringIO.h"

/**
 * \class ColumnStringIO
 * \brief String-IO interface of Column.
 */
ColumnStringIO::ColumnStringIO(Column* owner) : AbstractColumn(""), m_owner(owner) {
}

AbstractColumn::ColumnMode ColumnStringIO::columnMode() const {
	return AbstractColumn::Text;
}

AbstractColumn::PlotDesignation ColumnStringIO::plotDesignation() const {
	return m_owner->plotDesignation();
}

int ColumnStringIO::rowCount() const {
	return m_owner->rowCount();
}

bool ColumnStringIO::isValid(int row) const {
	if (m_setting)
		return true;

	return m_owner->isValid(row);
}

void ColumnStringIO::setTextAt(int row, const QString &value) {
	m_setting = true;
	m_to_set = value;
	m_owner->copy(m_owner->d->inputFilter()->output(0), 0, row, 1);
	m_setting = false;
	m_to_set.clear();
	m_owner->setChanged();
}

QString ColumnStringIO::textAt(int row) const {
	if (m_setting)
		return m_to_set;
	else
		return m_owner->d->outputFilter()->output(0)->textAt(row);
}

bool ColumnStringIO::copy(const AbstractColumn *other) {
	if (other->columnMode() != AbstractColumn::Text) return false;
	m_owner->d->inputFilter()->input(0,other);
	m_owner->copy(m_owner->d->inputFilter()->output(0));
	m_owner->d->inputFilter()->input(0,this);
	return true;
}

bool ColumnStringIO::copy(const AbstractColumn *source, int source_start, int dest_start, int num_rows) {
	if (source->columnMode() != AbstractColumn::Text) return false;
	m_owner->d->inputFilter()->input(0,source);
	m_owner->copy(m_owner->d->inputFilter()->output(0), source_start, dest_start, num_rows);
	m_owner->d->inputFilter()->input(0,this);
	return true;
}

void ColumnStringIO::replaceTexts(int start_row, const QVector<QString>& texts) {
	Column tmp("tmp", texts);
	copy(&tmp, 0, start_row, texts.size());
}
