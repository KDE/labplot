/*
    File                 : ColumnStringIO.cpp
    Project              : LabPlot
    Description          : String-IO interface of Column.
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2007-2009 Tilman Benkert <thzs@gmx.net>
    SPDX-FileCopyrightText: 2013-2017 Alexander Semke <alexander.semke@web.de>
    SPDX-License-Identifier: GPL-2.0-or-later
*/


#include "backend/core/column/Column.h"
#include "backend/core/column/ColumnPrivate.h"
#include "backend/core/column/ColumnStringIO.h"
#include "backend/core/AbstractSimpleFilter.h"

/**
 * \class ColumnStringIO
 * \brief String-IO interface of Column.
 */
ColumnStringIO::ColumnStringIO(Column* owner)
	: AbstractColumn(QString(), AspectType::ColumnStringIO), m_owner(owner) {
}

AbstractColumn::ColumnMode ColumnStringIO::columnMode() const {
	return AbstractColumn::ColumnMode::Text;
}

AbstractColumn::PlotDesignation ColumnStringIO::plotDesignation() const {
	return m_owner->plotDesignation();
}

QString ColumnStringIO::plotDesignationString() const {
	return m_owner->plotDesignationString();
}

int ColumnStringIO::rowCount() const {
	return m_owner->rowCount();
}

int ColumnStringIO::availableRowCount() const {
	return m_owner->availableRowCount();
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
	if (other->columnMode() != AbstractColumn::ColumnMode::Text) return false;
	m_owner->d->inputFilter()->input(0,other);
	m_owner->copy(m_owner->d->inputFilter()->output(0));
	m_owner->d->inputFilter()->input(0,this);
	return true;
}

bool ColumnStringIO::copy(const AbstractColumn *source, int source_start, int dest_start, int num_rows) {
	if (source->columnMode() != AbstractColumn::ColumnMode::Text) return false;
	m_owner->d->inputFilter()->input(0,source);
	m_owner->copy(m_owner->d->inputFilter()->output(0), source_start, dest_start, num_rows);
	m_owner->d->inputFilter()->input(0,this);
	return true;
}

void ColumnStringIO::replaceTexts(int start_row, const QVector<QString>& texts) {
	Column tmp("tmp", texts);
	copy(&tmp, 0, start_row, texts.size());
}
