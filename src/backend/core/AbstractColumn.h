/***************************************************************************
    File                 : AbstractColumn.h
    Project              : SciDAVis
    Description          : Interface definition for data with column logic
    --------------------------------------------------------------------
    Copyright            : (C) 2007,2008 Tilman Benkert (thzs*gmx.net)
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

#ifndef ABSTRACTCOLUMN_H
#define ABSTRACTCOLUMN_H

#include "lib/Interval.h"
#include "core/globals.h"
#include "core/AbstractAspect.h"

class AbstractSimpleFilter;
class QStringList;
class QString;
class QDateTime;
class QDate;
class QTime;
template<class T> class QList;

class AbstractColumn : public AbstractAspect
{
	Q_OBJECT

	public:
		AbstractColumn(const QString& name) : AbstractAspect(name) {}
		virtual ~AbstractColumn() { aboutToBeDestroyed(this);}

		virtual bool isReadOnly() const { return true; };
		virtual SciDAVis::ColumnMode columnMode() const = 0;
		virtual void setColumnMode(SciDAVis::ColumnMode mode);
		virtual bool copy(const AbstractColumn *other);
		virtual bool copy(const AbstractColumn * source, int source_start, int dest_start, int num_rows);

		virtual int rowCount() const = 0;
		virtual void insertRows(int before, int count);
		virtual void removeRows(int first, int count);
		virtual SciDAVis::PlotDesignation plotDesignation() const = 0;
		virtual void setPlotDesignation(SciDAVis::PlotDesignation pd);
		virtual void clear();

		bool isValid(int row) const;
		virtual bool isMasked(int row) const;
		virtual bool isMasked(Interval<int> i) const;
		virtual QList< Interval<int> > maskedIntervals() const;
		virtual void clearMasks();
		virtual void setMasked(Interval<int> i, bool mask = true);
		virtual void setMasked(int row, bool mask = true);

		virtual QString formula(int row) const;
		virtual QList< Interval<int> > formulaIntervals() const;
		virtual void setFormula(Interval<int> i, QString formula);
		virtual void setFormula(int row, QString formula);
		virtual void clearFormulas();
		
		virtual QString textAt(int row) const;
		virtual void setTextAt(int row, const QString& new_value);
		virtual void replaceTexts(int first, const QStringList& new_values);
		virtual QDate dateAt(int row) const;
		virtual void setDateAt(int row, const QDate& new_value);
		virtual QTime timeAt(int row) const;
		virtual void setTimeAt(int row, const QTime& new_value);
		virtual QDateTime dateTimeAt(int row) const;
		virtual void setDateTimeAt(int row, const QDateTime& new_value);
		virtual void replaceDateTimes(int first, const QList<QDateTime>& new_values);
		virtual double valueAt(int row) const;
		virtual void setValueAt(int row, double new_value);
		virtual void replaceValues(int first, const QVector<double>& new_values);

	signals: 
		void plotDesignationAboutToChange(const AbstractColumn * source); 
		void plotDesignationChanged(const AbstractColumn * source); 
		void modeAboutToChange(const AbstractColumn * source); 
		void modeChanged(const AbstractColumn * source); 
		void dataAboutToChange(const AbstractColumn * source); 
		void dataChanged(const AbstractColumn * source); 
		void rowsAboutToBeInserted(const AbstractColumn * source, int before, int count); 
		void rowsInserted(const AbstractColumn * source, int before, int count); 
		void rowsAboutToBeRemoved(const AbstractColumn * source, int first, int count); 
		void rowsRemoved(const AbstractColumn * source, int first, int count); 
		void maskingAboutToChange(const AbstractColumn * source); 
		void maskingChanged(const AbstractColumn * source); 
		void aboutToBeDestroyed(const AbstractColumn * source);

		friend class ColumnPrivate;
		friend class AbstractSimpleFilter;
		friend class SimpleCopyThroughFilter;
};

#endif

