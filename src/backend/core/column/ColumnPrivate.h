/*
    File                 : ColumnPrivate.h
    Project              : LabPlot
    Description          : Private data class of Column
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2007, 2008 Tilman Benkert <thzs@gmx.net>
    SPDX-FileCopyrightText: 2013-2019 Alexander Semke <alexander.semke@web.de>
    SPDX-FileCopyrightText: 2020 Stefan Gerlach <stefan.gerlach@uni.kn>
    SPDX-License-Identifier: GPL-2.0-or-later
*/


#ifndef COLUMNPRIVATE_H
#define COLUMNPRIVATE_H

#include "backend/core/AbstractColumn.h"
#include "backend/lib/IntervalAttribute.h"
#include "backend/core/column/Column.h"

class Column;
class ColumnSetGlobalFormulaCmd;

class ColumnPrivate : public QObject {
	Q_OBJECT

public:
	ColumnPrivate(Column*, AbstractColumn::ColumnMode);
	~ColumnPrivate() override;
	ColumnPrivate(Column*, AbstractColumn::ColumnMode, void*);
	void init();

	AbstractColumn::ColumnMode columnMode() const;
	void setColumnMode(AbstractColumn::ColumnMode);

	bool copy(const AbstractColumn*);
	bool copy(const AbstractColumn*, int source_start, int dest_start, int num_rows);
	bool copy(const ColumnPrivate*);
	bool copy(const ColumnPrivate*, int source_start, int dest_start, int num_rows);

	int rowCount() const;
	int availableRowCount(int max = -1) const;	// valid rows (stops when max rows found)
	void resizeTo(int);

	void insertRows(int before, int count);
	void removeRows(int first, int count);
	QString name() const;

	AbstractColumn::PlotDesignation plotDesignation() const;
	void setPlotDesignation(AbstractColumn::PlotDesignation);

	int width() const;
	void setWidth(int);

	void* data() const;
	bool hasValueLabels() const;
	void removeValueLabel(const QString&);
	void clearValueLabels();

	AbstractSimpleFilter* inputFilter() const;
	AbstractSimpleFilter* outputFilter() const;

	void replaceModeData(AbstractColumn::ColumnMode, void* data, AbstractSimpleFilter *in, AbstractSimpleFilter *out);
	void replaceData(void*);

	IntervalAttribute<QString> formulaAttribute() const;
	void replaceFormulas(const IntervalAttribute<QString>& formulas);

	//global formula defined for the whole column
	QString formula() const;
	const QVector<Column::FormulaData>& formulaData() const;
	void setFormulVariableColumnsPath(int index, const QString& path);
	void setFormulVariableColumn(int index, Column *column);
	void setFormulVariableColumn(Column*);
	bool formulaAutoUpdate() const;
	void setFormula(const QString& formula, const QVector<Column::FormulaData>& formulaData, bool autoUpdate);
	void setFormula(const QString& formula, const QStringList& variableNames,
					const QStringList& variableColumnPaths, bool autoUpdate);
	void updateFormula();

	//cell formulas
	QString formula(int row) const;
	QVector< Interval<int> > formulaIntervals() const;
	void setFormula(const Interval<int>& i, const QString& formula);
	void setFormula(int row, const QString& formula);
	void clearFormulas();

	QString textAt(int row) const;
	void setTextAt(int row, const QString&);
	void replaceTexts(int first, const QVector<QString>&);
	void addValueLabel(const QString&, const QString&);
	const QMap<QString, QString>& textValueLabels();

	QDate dateAt(int row) const;
	void setDateAt(int row, QDate);
	QTime timeAt(int row) const;
	void setTimeAt(int row, QTime);
	QDateTime dateTimeAt(int row) const;
	void setDateTimeAt(int row, const QDateTime&);
	void replaceDateTimes(int first, const QVector<QDateTime>&);
	void addValueLabel(const QDateTime&, const QString&);
	const QMap<QDateTime, QString>& dateTimeValueLabels();

	double valueAt(int row) const;
	void setValueAt(int row, double new_value);
	void replaceValues(int first, const QVector<double>&);
	void addValueLabel(double, const QString&);
	const QMap<double, QString>& valueLabels();

	int integerAt(int row) const;
	void setIntegerAt(int row, int new_value);
	void replaceInteger(int first, const QVector<int>&);
	void addValueLabel(int, const QString&);
	const QMap<int, QString>& intValueLabels();

	qint64 bigIntAt(int row) const;
	void setBigIntAt(int row, qint64 new_value);
	void replaceBigInt(int first, const QVector<qint64>&);
	void addValueLabel(qint64, const QString&);
	const QMap<qint64, QString>& bigIntValueLabels();

	void updateProperties();
	void invalidate();
	void finalizeLoad();

	mutable AbstractColumn::ColumnStatistics statistics;
	bool statisticsAvailable{false}; //is 'statistics' already available or needs to be (re-)calculated?

	bool hasValues{false};
	bool hasValuesAvailable{false}; //is 'hasValues' already available or needs to be (re-)calculated?

	mutable bool propertiesAvailable{false}; //is 'properties' already available (true) or needs to be (re-)calculated (false)?
	mutable AbstractColumn::Properties properties{AbstractColumn::Properties::No}; // declares the properties of the curve (monotonic increasing/decreasing ...). Speed up algorithms

private:
	AbstractColumn::ColumnMode m_columnMode;	// type of column data
	void* m_data{nullptr};	//pointer to the data container (QVector<T>)
	void* m_labels{nullptr};	//pointer to the container for the value labels(QMap<T, QString>)
	AbstractSimpleFilter* m_inputFilter{nullptr};	//input filter for string -> data type conversion
	AbstractSimpleFilter* m_outputFilter{nullptr};	//output filter for data type -> string conversion
	QString m_formula;
	QVector<Column::FormulaData> m_formulaData;
	bool m_formulaAutoUpdate{false};
	IntervalAttribute<QString> m_formulas;
	AbstractColumn::PlotDesignation m_plotDesignation{AbstractColumn::PlotDesignation::NoDesignation};
	int m_width{0}; //column width in the view
	Column* m_owner{nullptr};
	QVector<QMetaObject::Connection> m_connectionsUpdateFormula;

	void initLabels();
	void connectFormulaColumn(const AbstractColumn* column);

private Q_SLOTS:
	void formulaVariableColumnRemoved(const AbstractAspect*);
	void formulaVariableColumnAdded(const AbstractAspect*);

	friend ColumnSetGlobalFormulaCmd;
};

#endif
