/***************************************************************************
    File                 : ColumnStringIO.h
    Project              : LabPlot
    Description          : Aspect that manages a column string IO
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
