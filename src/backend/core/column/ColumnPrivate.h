/*
	File                 : ColumnPrivate.h
	Project              : LabPlot
	Description          : Private data class of Column
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2007, 2008 Tilman Benkert <thzs@gmx.net>
	SPDX-FileCopyrightText: 2013-2022 Alexander Semke <alexander.semke@web.de>
	SPDX-FileCopyrightText: 2020 Stefan Gerlach <stefan.gerlach@uni.kn>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef COLUMNPRIVATE_H
#define COLUMNPRIVATE_H

#include "backend/core/AbstractColumn.h"
#include "backend/core/column/Column.h"
#include "backend/lib/IntervalAttribute.h"

#include <QMap>

class Column;
class ColumnSetGlobalFormulaCmd;

class ColumnPrivate : public QObject {
	Q_OBJECT

public:
	ColumnPrivate(Column*, AbstractColumn::ColumnMode);
	~ColumnPrivate() override;
	ColumnPrivate(Column*, AbstractColumn::ColumnMode, void*);

	bool initDataContainer(bool resize = true);
	void initIOFilters();

	AbstractColumn::ColumnMode columnMode() const;
	void setColumnMode(AbstractColumn::ColumnMode);

	bool copy(const AbstractColumn*);
	bool copy(const AbstractColumn*, int source_start, int dest_start, int num_rows);
	bool copy(const ColumnPrivate*);
	bool copy(const ColumnPrivate*, int source_start, int dest_start, int num_rows);

	int indexForValue(double x) const;

	int rowCount() const;
	int rowCount(double min, double max) const;
	int availableRowCount(int max = -1) const; // valid rows (stops when max rows found)
	void resizeTo(int);

	void insertRows(int before, int count);
	void removeRows(int first, int count);
	QString name() const;

	AbstractColumn::PlotDesignation plotDesignation() const;
	void setPlotDesignation(AbstractColumn::PlotDesignation);

	int width() const;
	void setWidth(int);

	void setData(void*);
	void* data() const;
	void deleteData();
	bool valueLabelsInitialized() const;
	void removeValueLabel(const QString&);
	void setLabelsMode(Column::ColumnMode mode);
	void valueLabelsRemoveAll();
	double valueLabelsMinimum();
	double valueLabelsMaximum();

	AbstractSimpleFilter* inputFilter() const;
	AbstractSimpleFilter* outputFilter() const;

	void replaceModeData(AbstractColumn::ColumnMode, void* data, AbstractSimpleFilter* in, AbstractSimpleFilter* out);
	void replaceData(void*);

	IntervalAttribute<QString> formulaAttribute() const;
	void replaceFormulas(const IntervalAttribute<QString>& formulas);

	// global formula defined for the whole column
	QString formula() const;
	const QVector<Column::FormulaData>& formulaData() const;
	void setFormulVariableColumnsPath(int index, const QString& path);
	void setFormulVariableColumn(int index, Column* column);
	void setFormulVariableColumn(Column*);
	bool formulaAutoUpdate() const;
	bool formulaAutoResize() const;
	void setFormula(const QString& formula, const QVector<Column::FormulaData>& formulaData, bool autoUpdate, bool autoResize);
	void setFormula(const QString& formula, const QStringList& variableNames, const QStringList& variableColumnPaths, bool autoUpdate, bool autoResize);
	void updateFormula();

	// cell formulas
	QString formula(int row) const;
	QVector<Interval<int>> formulaIntervals() const;
	void setFormula(const Interval<int>& i, const QString& formula);
	void setFormula(int row, const QString& formula);
	void clearFormulas();

	// settings used to generate random values
	Column::RandomValuesData randomValuesData;

	QString textAt(int row) const;
	void setValueAt(int row, QString new_value);
	void setTextAt(int row, const QString&);
	void replaceValues(int first, const QVector<QString>&);
	void replaceTexts(int first, const QVector<QString>&);
	int dictionaryIndex(int row) const;
	const QMap<QString, int>& frequencies() const;

	QDate dateAt(int row) const;
	void setDateAt(int row, QDate);
	QTime timeAt(int row) const;
	void setTimeAt(int row, QTime);
	QDateTime dateTimeAt(int row) const;
	void setValueAt(int row, QDateTime new_value);
	void setDateTimeAt(int row, const QDateTime&);
	void replaceValues(int first, const QVector<QDateTime>&);
	void replaceDateTimes(int first, const QVector<QDateTime>&);

	double doubleAt(int row) const;
	double valueAt(int row) const;
	void setValueAt(int row, double new_value);
	void replaceValues(int first, const QVector<double>&);

	int integerAt(int row) const;
	void setValueAt(int row, int new_value);
	void setIntegerAt(int row, int new_value);
	void replaceValues(int first, const QVector<int>&);
	void replaceInteger(int first, const QVector<int>&);

	qint64 bigIntAt(int row) const;
	void setValueAt(int row, qint64 new_value);
	void setBigIntAt(int row, qint64 new_value);
	void replaceValues(int first, const QVector<qint64>&);
	void replaceBigInt(int first, const QVector<qint64>&);

	void updateProperties();
	void calculateStatistics();
	void invalidate();
	void finalizeLoad();

	void formulaVariableColumnAdded(const AbstractAspect*);

	struct CachedValuesAvailable {
		void setUnavailable() {
			statistics = false;
			min = false;
			max = false;
			hasValues = false;
			dictionary = false;
			properties = false;
		}
		bool statistics{false}; // is 'statistics' already available or needs to be (re-)calculated?
		// are minMax already calculated or needs to be (re-)calculated?
		// It is separated from statistics, because these are important values
		// which are quite often needed, but if the curve is monoton a faster algorithm is
		// used to recalculate the values
		bool min{false};
		bool max{false};
		bool hasValues{false}; // is 'hasValues' already available or needs to be (re-)calculated?
		bool dictionary{false}; // dictionary of text values, relevant for text columns only, available?
		bool properties{false}; // is 'properties' already available (true) or needs to be (re-)calculated (false)?
	};

	CachedValuesAvailable available;
	AbstractColumn::ColumnStatistics statistics;
	bool hasValues{false};
	AbstractColumn::Properties properties{
		AbstractColumn::Properties::No}; // declares the properties of the curve (monotonic increasing/decreasing ...). Speed up algorithms

	struct ValueLabels {
		void setMode(AbstractColumn::ColumnMode);
		void migrateLabels(AbstractColumn::ColumnMode newMode);
		void migrateDoubleTo(AbstractColumn::ColumnMode newMode);
		void migrateIntTo(AbstractColumn::ColumnMode newMode);
		void migrateBigIntTo(AbstractColumn::ColumnMode newMode);
		void migrateTextTo(AbstractColumn::ColumnMode newMode);
		void migrateDateTimeTo(AbstractColumn::ColumnMode newMode);
		int count() const;
		int count(double min, double max) const;
		void add(qint64, const QString&);
		void add(int, const QString&);
		void add(double, const QString&);
		void add(const QDateTime&, const QString&);
		void add(const QString&, const QString&);
		void removeAll();
		AbstractColumn::ColumnMode mode() const;
		AbstractColumn::Properties properties() const;
		bool initialized() const {
			return m_labels != nullptr;
		}
		void remove(const QString&);
		template<typename T>
		inline QVector<Column::ValueLabel<T>>* cast_vector() {
			return static_cast<QVector<Column::ValueLabel<T>>*>(m_labels);
		}
		template<typename T>
		inline const QVector<Column::ValueLabel<T>>* cast_vector() const {
			return static_cast<QVector<Column::ValueLabel<T>>*>(m_labels);
		}
		double minimum();
		double maximum();
		const QVector<Column::ValueLabel<QString>>* textValueLabels() const;
		const QVector<Column::ValueLabel<QDateTime>>* dateTimeValueLabels() const;
		const QVector<Column::ValueLabel<double>>* valueLabels() const;
		const QVector<Column::ValueLabel<int>>* intValueLabels() const;
		const QVector<Column::ValueLabel<qint64>>* bigIntValueLabels() const;
		int indexForValue(double value) const;
		double valueAt(int index) const;
		QDateTime dateTimeAt(int index) const;
		bool isValid(int index) const;
		bool isMasked(int index) const;
		QString labelAt(int index) const;

	private:
		void invalidateStatistics();
		void recalculateStatistics();
		bool init(AbstractColumn::ColumnMode);
		void deinit();

		// Do not call manually, because it is not doing a type checking!
		template<typename T>
		void remove(const T& value) {
			auto* v = cast_vector<T>();
			for (int i = 0; i < v->length(); i++) {
				if (v->at(i).value == value)
					v->remove(i);
			}
		}

	private:
		AbstractColumn::ColumnMode m_mode{AbstractColumn::ColumnMode::Integer};
		void* m_labels{nullptr}; // pointer to the container for the value labels(QMap<T, QString>)
		struct Statistics {
			bool available{false};
			double minimum;
			double maximum;
		};
		Statistics m_statistics;
	};
	ValueLabels m_labels;
	int valueLabelsCount() const;
	int valueLabelsCount(double min, double max) const;
	int valueLabelsIndexForValue(double value) const;
	double valueLabelsValueAt(int index) const;
	QString valueLabelAt(int index) const;
	void addValueLabel(qint64, const QString&);
	const QVector<Column::ValueLabel<qint64>>* bigIntValueLabels() const;
	void addValueLabel(int, const QString&);
	const QVector<Column::ValueLabel<int>>* intValueLabels() const;
	void addValueLabel(double, const QString&);
	const QVector<Column::ValueLabel<double>>* valueLabels() const;
	void addValueLabel(const QDateTime&, const QString&);
	const QVector<Column::ValueLabel<QDateTime>>* dateTimeValueLabels() const;
	void addValueLabel(const QString&, const QString&);
	const QVector<Column::ValueLabel<QString>>* textValueLabels() const;

	Column* const q{nullptr};

private:
	AbstractColumn::ColumnMode m_columnMode; // type of column data
	void* m_data{nullptr}; // pointer to the data container (QVector<T>)
	int m_rowCount{0};
	QVector<QString> m_dictionary; // dictionary for string columns
	QMap<QString, int> m_dictionaryFrequencies; // dictionary for elements frequencies in string columns

	AbstractSimpleFilter* m_inputFilter{nullptr}; // input filter for string -> data type conversion
	AbstractSimpleFilter* m_outputFilter{nullptr}; // output filter for data type -> string conversion
	QString m_formula;
	QVector<Column::FormulaData> m_formulaData;
	bool m_formulaAutoUpdate{false};
	bool m_formulaAutoResize{true};
	IntervalAttribute<QString> m_formulas;
	AbstractColumn::PlotDesignation m_plotDesignation{AbstractColumn::PlotDesignation::NoDesignation};
	int m_width{0}; // column width in the view
	QVector<QMetaObject::Connection> m_connectionsUpdateFormula;

	void initDictionary();
	void calculateTextStatistics();
	void calculateDateTimeStatistics();
	void connectFormulaColumn(const AbstractColumn*);

	// Never call this function directly, because it does no
	// mode checking.
	template<typename T>
	void setValueAtPrivate(int row, const T& new_value) {
		if (!m_data) {
			if (!initDataContainer())
				return; // failed to allocate memory
		}

		invalidate();

		Q_EMIT q->dataAboutToChange(q);
		if (row >= rowCount())
			resizeTo(row + 1);

		static_cast<QVector<T>*>(m_data)->replace(row, new_value);
		if (!q->m_suppressDataChangedSignal)
			Q_EMIT q->dataChanged(q);
	}

	// Never call this function directly, because it does no
	// mode checking.
	template<typename T>
	void replaceValuePrivate(int first, const QVector<T>& new_values) {
		if (!m_data) {
			const bool resize = (first >= 0);
			if (!initDataContainer(resize))
				return; // failed to allocate memory
		}

		invalidate();

		Q_EMIT q->dataAboutToChange(q);

		if (first < 0)
			*static_cast<QVector<T>*>(m_data) = new_values;
		else {
			const int num_rows = new_values.size();
			resizeTo(first + num_rows);

			T* ptr = static_cast<QVector<T>*>(m_data)->data();
			for (int i = 0; i < num_rows; ++i)
				ptr[first + i] = new_values.at(i);
		}

		if (!q->m_suppressDataChangedSignal)
			Q_EMIT q->dataChanged(q);
	}

private Q_SLOTS:
	void formulaVariableColumnRemoved(const AbstractAspect*);

	friend class ColumnSetGlobalFormulaCmd;
	friend class ColumnRemoveRowsCmd;
	friend class ColumnInsertRowsCmd;
};

#endif
