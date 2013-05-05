/***************************************************************************
    File                 : AbstractSimpleFilter.h
    Project              : AbstractColumn
    --------------------------------------------------------------------
    Copyright            : (C) 2007 by Knut Franke, Tilman Benkert
    Email (use @ for *)  : knut.franke*gmx.de, thzs*gmx.net
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
#ifndef ABSTRACT_SIMPLE_FILTER
#define ABSTRACT_SIMPLE_FILTER

#include "AbstractFilter.h"
#include "AbstractColumn.h"
#include "lib/IntervalAttribute.h"

// forward declaration - class follows
class SimpleFilterColumn;

class AbstractSimpleFilter : public AbstractFilter
{
	Q_OBJECT

	public:
		AbstractSimpleFilter();
		virtual int inputCount() const;
		virtual int outputCount() const;
		virtual AbstractColumn* output(int port);
		virtual const AbstractColumn * output(int port) const;
		virtual AbstractColumn::PlotDesignation plotDesignation() const;
		virtual AbstractColumn::ColumnMode columnMode() const;
		virtual QString textAt(int row) const;
		virtual QDate dateAt(int row) const;
		virtual QTime timeAt(int row) const;
		virtual QDateTime dateTimeAt(int row) const;;
		virtual double valueAt(int row) const;

		virtual int rowCount() const;
		virtual QList< Interval<int> > dependentRows(Interval<int> inputRange) const;

		virtual void save(QXmlStreamWriter * writer) const;
		virtual bool load(XmlStreamReader * reader);
		virtual void writeExtraAttributes(QXmlStreamWriter * writer) const;

	signals:
		void formatChanged();
		void digitsChanged();

	protected:
		virtual void inputPlotDesignationAboutToChange(const AbstractColumn*);
		virtual void inputPlotDesignationChanged(const AbstractColumn*);
		virtual void inputModeAboutToChange(const AbstractColumn*);
		virtual void inputModeChanged(const AbstractColumn*);
		virtual void inputDataAboutToChange(const AbstractColumn*);
		virtual void inputDataChanged(const AbstractColumn*);

		virtual void inputRowsAboutToBeInserted(const AbstractColumn * source, int before, int count);
		virtual void inputRowsInserted(const AbstractColumn * source, int before, int count);
		virtual void inputRowsAboutToBeRemoved(const AbstractColumn * source, int first, int count);
		virtual void inputRowsRemoved(const AbstractColumn * source, int first, int count);
		
		SimpleFilterColumn *m_output_column;
};

class SimpleFilterColumn : public AbstractColumn
{
	Q_OBJECT

	public:
		SimpleFilterColumn(AbstractSimpleFilter *owner) : AbstractColumn(owner->name()), m_owner(owner) {}

		virtual AbstractColumn::ColumnMode columnMode() const;
		virtual int rowCount() const { return m_owner->rowCount(); }
		virtual AbstractColumn::PlotDesignation plotDesignation() const { return m_owner->plotDesignation(); }
		virtual QString textAt(int row) const;
		virtual QDate dateAt(int row) const;
		virtual QTime timeAt(int row) const;
		virtual QDateTime dateTimeAt(int row) const;
		virtual double valueAt(int row) const;

	private:
		AbstractSimpleFilter *m_owner;

	friend class AbstractSimpleFilter;
};

#endif // ifndef ABSTRACT_SIMPLE_FILTER

