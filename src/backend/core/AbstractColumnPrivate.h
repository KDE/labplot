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
#include <QBitArray>

class AbstractColumnPrivate : public QObject {
	Q_OBJECT

public:
	explicit AbstractColumnPrivate(AbstractColumn* owner);
	AbstractColumn* owner() {
		return m_owner;
	}

	QString name() const {
		return m_owner->name();
	}

	static bool needsValidityTracking(AbstractColumn::ColumnMode mode) {
		return mode == AbstractColumn::ColumnMode::Integer || mode == AbstractColumn::ColumnMode::BigInt;
	}

	bool m_suppressDataChangedSignal{false};
	QBitArray m_masking; // One bit per row - O(1) operations, predictable memory
	AbstractColumn::HeatmapFormat* m_heatmapFormat{nullptr};

private:
	AbstractColumn* m_owner;
};

#endif // ifndef ABSTRACT_COLUMN_PRIVATE_H
