/*
File                 : ReadStatFilterPrivate.h
Project              : LabPlot
Description          : Private implementation class for ReadStatFilter.
--------------------------------------------------------------------
SPDX-FileCopyrightText: 2021 Stefan Gerlach <stefan.gerlach@uni.kn>

SPDX-License-Identifier: GPL-2.0-or-later
*/
#ifndef READSTATFILTERPRIVATE_H
#define READSTATFILTERPRIVATE_H

#ifdef HAVE_READSTAT
extern "C" {
#include <readstat.h>
}
#endif

#include "backend/lib/macros.h"

class AbstractDataSource;

class LabelSet {
	QVector<QString> m_labels;
	QVector<QString> m_valuesString;
	QVector<int> m_valuesInt;
	QVector<double> m_valuesDouble;

public:
	LabelSet() {}

	QVector<QString> labels() const { return m_labels; }
	QString valueString(int i) const { return m_valuesString.at(i); }
	int valueInt(int i) const { return m_valuesInt.at(i);}
	double valueDouble(int i) const { return m_valuesDouble.at(i);}

	void add(QString value, QString label) {
		if (m_valuesInt.size() > 0 || m_valuesDouble.size() > 0) {
			DEBUG(Q_FUNC_INFO << ", WARNING: can't add string value to integer/double label set");
			return;
		}

		m_valuesString.append(value);
		m_labels.append(label);
	}
	void add(int value, QString label) {
		if (m_valuesDouble.size() > 0 || m_valuesString.size() > 0) {
			DEBUG(Q_FUNC_INFO << ", WARNING: can't add integer value to double/string label set");
			return;
		}

		m_valuesInt.append(value);
		m_labels.append(label);
	}
	void add(double value, QString label) {
		if (m_valuesInt.size() > 0 || m_valuesString.size() > 0) {
			DEBUG(Q_FUNC_INFO << ", WARNING: can't add double value to int/string label set");
			return;
		}

		m_valuesDouble.append(value);
		m_labels.append(label);
	}

	int size() const {
		return m_labels.size();
	}
};

class ReadStatFilterPrivate {

public:
	explicit ReadStatFilterPrivate(ReadStatFilter*);

#ifdef HAVE_READSTAT
	// callbacks (get*)
	static int getMetaData(readstat_metadata_t *, void *);
	static int getVarName(int index, readstat_variable_t*, const char *val_labels, void *);
	static int getColumnModes(int row, readstat_variable_t*, readstat_value_t, void *);
	static int getValuesPreview(int row, readstat_variable_t*, readstat_value_t, void *);
	static int getValues(int row, readstat_variable_t*, readstat_value_t, void *);
	static int getNotes(int index, const char* note, void *);
	static int getFWeights(readstat_variable_t*, void *);
	static int getValueLabels(const char *val_labels, readstat_value_t, const char *label, void *);
	readstat_error_t parse(const QString& fileName, bool preview = false, bool prepare = false);
#endif
	QVector<QStringList> preview(const QString& fileName, int lines);
	void readDataFromFile(const QString& fileName, AbstractDataSource* = nullptr,
			AbstractFileFilter::ImportMode = AbstractFileFilter::ImportMode::Replace);
	void write(const QString& fileName, AbstractDataSource*);
	static QStringList m_varNames;
	static QVector<AbstractColumn::ColumnMode> m_columnModes;
	static QVector<QStringList> m_dataStrings;

	const ReadStatFilter* q;

	static int m_startRow;
	static int m_endRow;
	static int m_startColumn;
	static int m_endColumn;
private:
	//int m_status;

	static int m_varCount;	// nr of cols (vars)
	static int m_rowCount;	// nr of rows
	static QStringList m_lineString;
	static std::vector<void*> m_dataContainer;
	static QStringList m_notes;
	static QVector<QString> m_valueLabels;
	static QMap<QString, LabelSet> m_labelSets;
};

#endif
