/***************************************************************************
    File                 : Column.h
    Project              : SciDAVis
    Description          : Aspect that manages a column
    --------------------------------------------------------------------
    Copyright            : (C) 2007-2009 Tilman Benkert (thzs*gmx.net)
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

#ifndef COLUMN_H
#define COLUMN_H

#include "core/AbstractAspect.h"
#include "core/AbstractSimpleFilter.h"
#include "lib/IntervalAttribute.h"
#include "lib/XmlStreamReader.h"
class QString;

class ColumnStringIO;

class Column : public AbstractColumn
{
	Q_OBJECT

	public:
		class Private;
		friend class Private;

		static void staticInit();

		Column(const QString& name, SciDAVis::ColumnMode mode);
		Column(const QString& name, QVector<double> data);
		Column(const QString& name, QStringList data); 
		Column(const QString& name, QList<QDateTime> data);
		void init();
		~Column();

		virtual QIcon icon() const;

		bool isReadOnly() const;
		SciDAVis::ColumnMode columnMode() const;
		void setColumnMode(SciDAVis::ColumnMode mode);
		bool copy(const AbstractColumn * other);
		bool copy(const AbstractColumn * source, int source_start, int dest_start, int num_rows);
		int rowCount() const;
		SciDAVis::PlotDesignation plotDesignation() const;
		void setPlotDesignation(SciDAVis::PlotDesignation pd);
		int width() const;
		void setWidth(int value);
		void clear();
		AbstractSimpleFilter *outputFilter() const;
		ColumnStringIO *asStringColumn() const;

		QString formula(int row) const;
		QList< Interval<int> > formulaIntervals() const;
		void setFormula(Interval<int> i, QString formula);
		void setFormula(int row, QString formula);
		void clearFormulas();
		
		QString textAt(int row) const;
		void setTextAt(int row, const QString& new_value);
		void replaceTexts(int first, const QStringList& new_values);
		QDate dateAt(int row) const;
		void setDateAt(int row, const QDate& new_value);
		QTime timeAt(int row) const;
		void setTimeAt(int row, const QTime& new_value);
		QDateTime dateTimeAt(int row) const;
		void setDateTimeAt(int row, const QDateTime& new_value);
		void replaceDateTimes(int first, const QList<QDateTime>& new_values);
		double valueAt(int row) const;
		void setValueAt(int row, double new_value);
		virtual void replaceValues(int first, const QVector<double>& new_values);

		void save(QXmlStreamWriter * writer) const;
		bool load(XmlStreamReader * reader);
	private:
		bool XmlReadInputFilter(XmlStreamReader * reader);
		bool XmlReadOutputFilter(XmlStreamReader * reader);
		bool XmlReadFormula(XmlStreamReader * reader);
		bool XmlReadRow(XmlStreamReader * reader);

		void handleRowInsertion(int before, int count);
		void handleRowRemoval(int first, int count);

	signals:
		void widthAboutToChange(const Column*);
		void widthChanged(const Column*);

	private slots:
		void handleFormatChange();

	private:
		Private * m_column_private;
		ColumnStringIO * m_string_io;
		
		friend class ColumnStringIO;
};

class ColumnStringIO : public AbstractColumn
{
	Q_OBJECT
	
	public:
		ColumnStringIO(Column * owner) : AbstractColumn(tr("as string")), m_owner(owner), m_setting(false) {}
		virtual SciDAVis::ColumnMode columnMode() const { return SciDAVis::Text; }
		virtual SciDAVis::PlotDesignation plotDesignation() const { return m_owner->plotDesignation(); }
		virtual int rowCount() const { return m_owner->rowCount(); }
		virtual QString textAt(int row) const;
		virtual void setTextAt(int row, const QString &value);
		virtual bool isValid(int row) const {
			if (m_setting)
				return true;
			else
				return m_owner->isValid(row);
		}
		virtual bool copy(const AbstractColumn *other);
		virtual bool copy(const AbstractColumn *source, int source_start, int dest_start, int num_rows);
		virtual void replaceTexts(int start_row, const QStringList &texts);

	private:
		Column * m_owner;
		bool m_setting;
		QString m_to_set;
};

#endif
