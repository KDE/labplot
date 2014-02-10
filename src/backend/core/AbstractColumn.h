/***************************************************************************
    File                 : AbstractColumn.h
    Project              : AbstractColumn
    Description          : Interface definition for data with column logic
    --------------------------------------------------------------------
    Copyright            : (C) 2013 by Alexander Semke (alexander.semke*web.de)
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

#include "backend/core/AbstractAspect.h"

class AbstractSimpleFilter;
class QStringList;
class QString;
class QDateTime;
class QDate;
class QTime;
template<class T> class QList;
template<class T> class Interval;

class AbstractColumn : public AbstractAspect
{
	Q_OBJECT
	Q_ENUMS(PlotDesignation)
	Q_ENUMS(ColumnMode)

	public:
		enum PlotDesignation {
			noDesignation = 0,
			X = 1,
			Y = 2,
			Z = 3,
			xErr = 4,
			yErr = 5
		};

		enum ColumnMode {
			Numeric = 0,
			Text = 1,
			Month = 4,
			Day = 5,
			DateTime = 6
			// 2 and 3 are skipped to avoid problems with old obsolete values
		};
		
		class Private;

		explicit AbstractColumn(const QString& name);
		virtual ~AbstractColumn() { aboutToBeDestroyed(this);}

		virtual bool isReadOnly() const { return true; };
		virtual ColumnMode columnMode() const = 0;
		virtual void setColumnMode(AbstractColumn::ColumnMode);
		virtual PlotDesignation plotDesignation() const = 0;
		virtual void setPlotDesignation(AbstractColumn::PlotDesignation);

		virtual bool copy(const AbstractColumn *source);
		virtual bool copy(const AbstractColumn *source, int source_start, int dest_start, int num_rows);

		virtual int rowCount() const = 0;
		void insertRows(int before, int count);
		void removeRows(int first, int count);
		virtual void clear();

		bool isValid(int row) const;

		bool isMasked(int row) const;
		bool isMasked(Interval<int> i) const;
		QList< Interval<int> > maskedIntervals() const;
		void clearMasks();
		void setMasked(Interval<int> i, bool mask = true);
		void setMasked(int row, bool mask = true);

		virtual QString formula(int row) const;
		virtual QList< Interval<int> > formulaIntervals() const;
		virtual void setFormula(Interval<int> i, QString formula);
		virtual void setFormula(int row, QString formula);
		virtual void clearFormulas();

		double minimum() const;
		double maximum() const;

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

	protected:
		bool XmlReadMask(XmlStreamReader *reader);
		void XmlWriteMask(QXmlStreamWriter *writer) const;

		virtual void handleRowInsertion(int before, int count);
		virtual void handleRowRemoval(int first, int count);

	private:
		Private *m_abstract_column_private;

		friend class AbstractColumnRemoveRowsCmd;
		friend class AbstractColumnInsertRowsCmd;
		friend class AbstractColumnClearMasksCmd;
		friend class AbstractColumnSetMaskedCmd;
};

#endif

