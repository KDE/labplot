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

#include <cmath>

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

		explicit Column(const QString& name, AbstractColumn::ColumnMode mode = AbstractColumn::Numeric);
		Column(const QString& name, QVector<double> data);
		Column(const QString& name, QStringList data);
		Column(const QString& name, QList<QDateTime> data);
		void init();
		~Column();

		virtual QIcon icon() const;
		virtual QMenu* createContextMenu();

		AbstractColumn::ColumnMode columnMode() const;
		void setColumnMode(AbstractColumn::ColumnMode);

		bool copy(const AbstractColumn*);
		bool copy(const AbstractColumn* source, int source_start, int dest_start, int num_rows);

		AbstractColumn::PlotDesignation plotDesignation() const;
		void setPlotDesignation(AbstractColumn::PlotDesignation pd);

		bool isReadOnly() const;
		int rowCount() const;
		int width() const;
		void setWidth(int value);
		void clear();
		AbstractSimpleFilter* outputFilter() const;
		ColumnStringIO* asStringColumn() const;

		void setFormula(const QString& formula, const QStringList& variableNames, const QStringList& variableColumnPathes);
		QString formula() const;
		const QStringList& formulaVariableNames() const;
		const QStringList& formulaVariableColumnPathes() const;
		QString formula(int) const;
		QList< Interval<int> > formulaIntervals() const;
		void setFormula(Interval<int>, QString);
		void setFormula(int, QString);
		void clearFormulas();

		const ColumnStatistics& statistics();
		void* data() const;

		QString textAt(int) const;
		void setTextAt(int, const QString&);
		void replaceTexts(int, const QStringList&);

		QDate dateAt(int) const;
		void setDateAt(int, const QDate&);
		QTime timeAt(int) const;
		void setTimeAt(int, const QTime&);
		QDateTime dateTimeAt(int) const;
		void setDateTimeAt(int, const QDateTime&);
		void replaceDateTimes(int, const QList<QDateTime>&);

		double valueAt(int) const;
		void setValueAt(int, double);
		virtual void replaceValues(int, const QVector<double>&);

		void setChanged();
		void setSuppressDataChangedSignal(bool);

		void save(QXmlStreamWriter*) const;
		bool load(XmlStreamReader*);

	private:
		bool XmlReadInputFilter(XmlStreamReader*);
		bool XmlReadOutputFilter(XmlStreamReader*);
		bool XmlReadFormula(XmlStreamReader*);
		bool XmlReadRow(XmlStreamReader*);

		void handleRowInsertion(int before, int count);
		void handleRowRemoval(int first, int count);

		void calculateStatistics();
		void setStatisticsAvailable(bool);
		bool statisticsAvailable() const;

		bool m_suppressDataChangedSignal;
		QActionGroup* usedInActionGroup;

		friend class ColumnStringIO;

		ColumnStringIO* m_string_io;

		ColumnPrivate* d;
		friend class ColumnPrivate;

	signals:
		void requestProjectContextMenu(QMenu*);

	private slots:
		void navigateTo(QAction*);
		void handleFormatChange();
};

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
		virtual void replaceTexts(int start_row, const QStringList& texts);

	private:
		Column* m_owner;
		bool m_setting;
		QString m_to_set;
};

#endif
