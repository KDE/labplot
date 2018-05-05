/***************************************************************************
    File                 : ColumnPrivate.h
    Project              : LabPlot
    Description          : Private data class of Column
    --------------------------------------------------------------------
    Copyright            : (C) 2007,2008 Tilman Benkert (thzs@gmx.net)
    Copyright            : (C) 2013-2017 Alexander Semke (alexander.semke@web.de)

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

#ifndef COLUMNPRIVATE_H
#define COLUMNPRIVATE_H

#include "backend/core/AbstractColumn.h"
#include "backend/lib/IntervalAttribute.h"

class Column;

class ColumnPrivate : QObject {
	Q_OBJECT

public:
	ColumnPrivate(Column*, AbstractColumn::ColumnMode);
	~ColumnPrivate() override;
	ColumnPrivate(Column*, AbstractColumn::ColumnMode, void*);

	AbstractColumn::ColumnMode columnMode() const;
	void setColumnMode(AbstractColumn::ColumnMode);

	bool copy(const AbstractColumn*);
	bool copy(const AbstractColumn*, int source_start, int dest_start, int num_rows);
	bool copy(const ColumnPrivate*);
	bool copy(const ColumnPrivate*, int source_start, int dest_start, int num_rows);

	int rowCount() const;
	void resizeTo(int);

	void insertRows(int before, int count);
	void removeRows(int first, int count);
	QString name() const;

	AbstractColumn::PlotDesignation plotDesignation() const;
	void setPlotDesignation(AbstractColumn::PlotDesignation);

	int width() const;
	void setWidth(int);

	void* data() const;

	AbstractSimpleFilter* inputFilter() const;
	AbstractSimpleFilter* outputFilter() const;

	void replaceModeData(AbstractColumn::ColumnMode, void* data, AbstractSimpleFilter *in, AbstractSimpleFilter *out);
	void replaceData(void*);

	IntervalAttribute<QString> formulaAttribute() const;
	void replaceFormulas(const IntervalAttribute<QString>& formulas);

	QString formula() const;
	const QStringList& formulaVariableNames() const;
	const QStringList& formulaVariableColumnPathes() const;
	void setFormula(const QString& formula, const QStringList& variableNames, const QStringList& variableColumnPathes);
	QString formula(int row) const;
	QVector< Interval<int> > formulaIntervals() const;
	void setFormula(Interval<int> i, QString formula);
	void setFormula(int row, QString formula);
	void clearFormulas();

	QString textAt(int row) const;
	void setTextAt(int row, const QString&);
	void replaceTexts(int first, const QVector<QString>&);

	QDate dateAt(int row) const;
	void setDateAt(int row, QDate);
	QTime timeAt(int row) const;
	void setTimeAt(int row, QTime);
	QDateTime dateTimeAt(int row) const;
	void setDateTimeAt(int row, const QDateTime&);
	void replaceDateTimes(int first, const QVector<QDateTime>&);

	double valueAt(int row) const;
	void setValueAt(int row, double new_value);
	void replaceValues(int first, const QVector<double>&);

	int integerAt(int row) const;
	void setIntegerAt(int row, int new_value);
	void replaceInteger(int first, const QVector<int>&);

	AbstractColumn::ColumnStatistics statistics;
	bool statisticsAvailable; //is 'statistics' already available or needs to be (re-)calculated?

	bool hasValues;
	bool hasValuesAvailable; //is 'hasValues' already available or needs to be (re-)calculated?

private:
	AbstractColumn::ColumnMode m_column_mode;	// type of column data
	void* m_data;	//pointer to the data container (QVector<T>)
	AbstractSimpleFilter* m_input_filter;	//input filter for string -> data type conversion
	AbstractSimpleFilter* m_output_filter;	//output filter for data type -> string conversion
	QString m_formula;
	QStringList m_formulaVariableNames;
	QStringList m_formulaVariableColumnPathes;
	IntervalAttribute<QString> m_formulas;
	AbstractColumn::PlotDesignation m_plot_designation;
	int m_width; //column width in the view
	Column* m_owner;
};

#endif
