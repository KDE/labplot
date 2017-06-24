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

class ColumnPrivate;
class ColumnStringIO;
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

	explicit Column(const QString& name, AbstractColumn::ColumnMode = AbstractColumn::Numeric);
	// template constructor for all supported data types (AbstractColumn::ColumnMode) must be defined in header
	template <class T>
	Column(const QString& name, QVector<T> data)
		: AbstractColumn(name), d(new class ColumnPrivate(this, AbstractColumn::Numeric, new QVector<T>(data))) {
		init();
	};
	Column(const QString& name, QStringList data); // overloaded contructor for QStringList
	void init();
	~Column();

	virtual QIcon icon() const;
	virtual QMenu* createContextMenu();

	AbstractColumn::ColumnMode columnMode() const;
	void setColumnMode(AbstractColumn::ColumnMode);

	bool copy(const AbstractColumn*);
	bool copy(const AbstractColumn* source, int source_start, int dest_start, int num_rows);

	AbstractColumn::PlotDesignation plotDesignation() const;
	void setPlotDesignation(AbstractColumn::PlotDesignation);

	bool isReadOnly() const;
	int rowCount() const;
	int width() const;
	void setWidth(const int);
	void clear();
	AbstractSimpleFilter* outputFilter() const;
	ColumnStringIO* asStringColumn() const;

	void setFormula(const QString& formula, const QStringList& variableNames, const QStringList& variableColumnPathes);
	QString formula() const;
	const QStringList& formulaVariableNames() const;
	const QStringList& formulaVariableColumnPathes() const;
	QString formula(const int) const;
	QList< Interval<int> > formulaIntervals() const;
	void setFormula(Interval<int>, QString);
	void setFormula(int, QString);
	void clearFormulas();

	const ColumnStatistics& statistics();
	void* data() const;

	QString textAt(const int) const;
	void setTextAt(const int, const QString&);
	void replaceTexts(const int, const QStringList&);

	QDate dateAt(const int) const;
	void setDateAt(const int, const QDate&);
	QTime timeAt(const int) const;
	void setTimeAt(const int, const QTime&);
	QDateTime dateTimeAt(const int) const;
	void setDateTimeAt(const int, const QDateTime&);
	void replaceDateTimes(const int, const QVector<QDateTime>&);

	double valueAt(const int) const;
	void setValueAt(const int, const double);
	virtual void replaceValues(const int, const QVector<double>&);

	void setChanged();
	void setSuppressDataChangedSignal(const bool);

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
	QActionGroup* m_usedInActionGroup;

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


#endif
