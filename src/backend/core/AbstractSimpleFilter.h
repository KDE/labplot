/***************************************************************************
    File                 : AbstractSimpleFilter.h
    Project              : AbstractColumn
    --------------------------------------------------------------------
    Copyright            : (C) 2007 Knut Franke (knut.franke@gmx.de)
    Copyright            : (C) 2007 Tilman Benkert (thzs@gmx.net)
    Copyright            : (C) 2017 Stefan Gerlach (stefan.gerlach@uni.kn)
    Description          : Simplified filter interface for filters with
                           only one output port.
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
#ifndef ABSTRACTSIMPLEFILTER_H
#define ABSTRACTSIMPLEFILTER_H

#include "AbstractFilter.h"
#include "AbstractColumn.h"
#include "backend/lib/IntervalAttribute.h"

class SimpleFilterColumn;

class AbstractSimpleFilter : public AbstractFilter {
	Q_OBJECT

public:
	AbstractSimpleFilter();
	virtual int inputCount() const override;
	virtual int outputCount() const override;
	virtual AbstractColumn* output(int port) override;
	virtual const AbstractColumn * output(int port) const override;
	virtual AbstractColumn::PlotDesignation plotDesignation() const;
	virtual AbstractColumn::ColumnMode columnMode() const;
	virtual QString textAt(int row) const;
	virtual QDate dateAt(int row) const;
	virtual QTime timeAt(int row) const;
	virtual QDateTime dateTimeAt(int row) const;;
	virtual double valueAt(int row) const;
	virtual int integerAt(int row) const;

	virtual int rowCount() const;
	virtual QList<Interval<int>> dependentRows(Interval<int> inputRange) const;

	virtual void save(QXmlStreamWriter*) const override;
	virtual bool load(XmlStreamReader*, bool preview) override;
	virtual void writeExtraAttributes(QXmlStreamWriter*) const;

signals:
	void formatChanged();
	void digitsChanged();

protected:
	virtual void inputPlotDesignationAboutToChange(const AbstractColumn*) override;
	virtual void inputPlotDesignationChanged(const AbstractColumn*) override;
	virtual void inputModeAboutToChange(const AbstractColumn*) override;
	virtual void inputModeChanged(const AbstractColumn*) override;
	virtual void inputDataAboutToChange(const AbstractColumn*) override;
	virtual void inputDataChanged(const AbstractColumn*) override;

	virtual void inputRowsAboutToBeInserted(const AbstractColumn * source, int before, int count) override;
	virtual void inputRowsInserted(const AbstractColumn * source, int before, int count) override;
	virtual void inputRowsAboutToBeRemoved(const AbstractColumn * source, int first, int count) override;
	virtual void inputRowsRemoved(const AbstractColumn * source, int first, int count) override;

	SimpleFilterColumn* m_output_column;
};

class SimpleFilterColumn : public AbstractColumn {
	Q_OBJECT

public:
	SimpleFilterColumn(AbstractSimpleFilter* owner) : AbstractColumn(owner->name()), m_owner(owner) {}

	virtual AbstractColumn::ColumnMode columnMode() const override;
	virtual int rowCount() const override { return m_owner->rowCount(); }
	virtual AbstractColumn::PlotDesignation plotDesignation() const override { return m_owner->plotDesignation(); }
	virtual QString textAt(int row) const override;
	virtual QDate dateAt(int row) const override;
	virtual QTime timeAt(int row) const override;
	virtual QDateTime dateTimeAt(int row) const override;
	virtual double valueAt(int row) const override;
	virtual int integerAt(int row) const override;
	virtual void save(QXmlStreamWriter*) const override {};
	virtual bool load(XmlStreamReader*, bool preview) override {Q_UNUSED(preview); return true;};
private:
	AbstractSimpleFilter* m_owner;

	friend class AbstractSimpleFilter;
};

#endif // ifndef ABSTRACTSIMPLEFILTER_H

