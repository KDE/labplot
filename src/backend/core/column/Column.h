/***************************************************************************
    File                 : Column.h
    Project              : LabPlot
    Description          : Aspect that manages a column
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

#ifndef COLUMN_H
#define COLUMN_H

#include "backend/core/AbstractSimpleFilter.h"
#include "backend/lib/XmlStreamReader.h"

class ColumnStringIO;
class ColumnPrivate;
class QActionGroup;

class Column : public AbstractColumn {
	Q_OBJECT

	public:
		struct ColumnStatistics {
			ColumnStatistics() {
				minimum = NAN;
				maximum = NAN;
				arithmeticMean = NAN;
				geometricMean = NAN;
				harmonicMean = NAN;
				contraharmonicMean = NAN;
				median = NAN;
				variance = NAN;
				standardDeviation = NAN;
				meanDeviation = NAN;
				meanDeviationAroundMedian = NAN;
				medianDeviation = NAN;
				skewness = NAN;
				kurtosis = NAN;
				entropy = NAN;
			}
			double minimum;
			double maximum;
			double arithmeticMean;
			double geometricMean;
			double harmonicMean;
			double contraharmonicMean;
			double median;
			double variance;
			double standardDeviation;
			double meanDeviation; // mean absolute deviation around mean
			double meanDeviationAroundMedian; // mean absolute deviation around median
			double medianDeviation; // median absolute deviation
			double skewness;
			double kurtosis;
			double entropy;
		};

		friend class ColumnPrivate;

		explicit Column(const QString& name, AbstractColumn::ColumnMode mode = AbstractColumn::Numeric);
		Column(const QString& name, QVector<double> data);
		Column(const QString& name, QStringList data);
		Column(const QString& name, QList<QDateTime> data);
		void init();
		~Column();

		virtual QIcon icon() const;
		virtual QMenu* createContextMenu();

		bool isReadOnly() const;
		AbstractColumn::ColumnMode columnMode() const;
		void setColumnMode(AbstractColumn::ColumnMode mode);
		bool copy(const AbstractColumn * other);
		bool copy(const AbstractColumn * source, int source_start, int dest_start, int num_rows);
		int rowCount() const;
		AbstractColumn::PlotDesignation plotDesignation() const;
		void setPlotDesignation(AbstractColumn::PlotDesignation pd);
		int width() const;
		void setWidth(int value);
		void clear();
		AbstractSimpleFilter *outputFilter() const;
		ColumnStringIO *asStringColumn() const;

		void setFormula(const QString& formula, const QStringList& variableNames, const QStringList& variableColumnPathes);
		QString formula() const;
		const QStringList& formulaVariableNames() const;
		const QStringList& formulaVariableColumnPathes() const;

		QString formula(int row) const;
		QList< Interval<int> > formulaIntervals() const;
		void setFormula(Interval<int> i, QString formula);
		void setFormula(int row, QString formula);
		void clearFormulas();

		const ColumnStatistics& statistics();
		void* data() const;
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
		void setChanged();
		void setSuppressDataChangedSignal(bool);

		void save(QXmlStreamWriter*) const;
		bool load(XmlStreamReader*);

	private:
		bool XmlReadInputFilter(XmlStreamReader * reader);
		bool XmlReadOutputFilter(XmlStreamReader * reader);
		bool XmlReadFormula(XmlStreamReader * reader);
		bool XmlReadRow(XmlStreamReader * reader);

		void handleRowInsertion(int before, int count);
		void handleRowRemoval(int first, int count);

		void calculateStatistics();
		void setStatisticsAvailable(bool available);
		bool statisticsAvailable() const;

		ColumnPrivate* m_column_private;
		ColumnStringIO* m_string_io;
		bool m_suppressDataChangedSignal;
		QActionGroup* usedInActionGroup;

		friend class ColumnStringIO;

	signals:
		void requestProjectContextMenu(QMenu*);
		void widthAboutToChange(const Column*);
		void widthChanged(const Column*);

	private slots:
		void navigateTo(QAction*);
		void handleFormatChange();
};

class ColumnStringIO : public AbstractColumn {
	Q_OBJECT

	public:
		ColumnStringIO(Column * owner) : AbstractColumn(""), m_owner(owner), m_setting(false) {}
		virtual AbstractColumn::ColumnMode columnMode() const { return AbstractColumn::Text; }
		virtual AbstractColumn::PlotDesignation plotDesignation() const { return m_owner->plotDesignation(); }
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
