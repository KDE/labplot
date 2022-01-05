/*
    File                 : ColumnStringIO.h
    Project              : LabPlot
    Description          : Aspect that manages a column string IO
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2007-2009 Tilman Benkert <thzs@gmx.net>
    SPDX-FileCopyrightText: 2013-2017 Alexander Semke <alexander.semke@web.de>
    SPDX-License-Identifier: GPL-2.0-or-later
*/


#ifndef COLUMNSTRINGIO_H
#define COLUMNSTRINGIO_H

#include "backend/core/column/Column.h"

class ColumnStringIO : public AbstractColumn {
	Q_OBJECT

public:
	explicit ColumnStringIO(Column* owner);
	AbstractColumn::ColumnMode columnMode() const override;
	AbstractColumn::PlotDesignation plotDesignation() const override;
	int rowCount() const override;
	int availableRowCount() const override;
	QString textAt(int) const override;
	void setTextAt(int, const QString&) override;
	virtual bool isValid(int) const;
	bool copy(const AbstractColumn*) override;
	bool copy(const AbstractColumn* source, int source_start, int dest_start, int num_rows) override;
	void replaceTexts(int start_row, const QVector<QString>& texts) override;
	void save(QXmlStreamWriter*) const override {};
	bool load(XmlStreamReader*, bool preview) override {Q_UNUSED(preview); return true;};
private:
	Column* m_owner;
	bool m_setting{false};
	QString m_to_set;
};

#endif
