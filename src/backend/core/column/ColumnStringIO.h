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
	ColumnStringIO(Column* owner);
	virtual AbstractColumn::ColumnMode columnMode() const;
	virtual AbstractColumn::PlotDesignation plotDesignation() const;
	virtual int rowCount() const;
	virtual QString textAt(int) const;
	virtual void setTextAt(int, const QString&);
	virtual bool isValid(int) const;
	virtual bool copy(const AbstractColumn*);
	virtual bool copy(const AbstractColumn* source, int source_start, int dest_start, int num_rows);
	virtual void replaceTexts(int start_row, const QVector<QString>& texts);
	virtual void save(QXmlStreamWriter*) const override {};
	virtual bool load(XmlStreamReader*, bool preview) override {Q_UNUSED(preview); return true;};
private:
	Column* m_owner;
	bool m_setting;
	QString m_to_set;
};

#endif
