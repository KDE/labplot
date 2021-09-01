/*
    File                 : AbstractColumnPrivate.h
    Project              : LabPlot
    Description          : Private data class of AbstractColumn
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2007-2009 Tilman Benkert <thzs@gmx.net>,
    SPDX-FileCopyrightText: 2007-2009 Knut Franke <knut.franke@gmx.de>
    SPDX-FileCopyrightText: 2013-2017 Alexander Semke <alexander.semke@web.de>
    SPDX-License-Identifier: GPL-2.0-or-later
*/


#ifndef ABSTRACT_COLUMN_PRIVATE_H
#define ABSTRACT_COLUMN_PRIVATE_H

#include "backend/core/AbstractColumn.h"
#include "backend/lib/IntervalAttribute.h"

class AbstractColumnPrivate {
public:
	explicit AbstractColumnPrivate(AbstractColumn* owner);
	AbstractColumn* owner() { return m_owner; }

	QString name() const { return m_owner->name(); }

	IntervalAttribute<bool> m_masking;
	AbstractColumn::HeatmapFormat* m_heatmapFormat{nullptr};

private:
	AbstractColumn* m_owner;
};

#endif //ifndef ABSTRACT_COLUMN_PRIVATE_H
