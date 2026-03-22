/*
	File                 : ColumnPrivate.cpp
	Project              : AbstractColumn
	Description          : Private data class of Column
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2007-2008 Tilman Benkert <thzs@gmx.net>
	SPDX-FileCopyrightText: 2012-2022 Alexander Semke <alexander.semke@web.de>
	SPDX-FileCopyrightText: 2017-2022 Stefan Gerlach <stefan.gerlach@uni.kn>

	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "ColumnPrivate.h"
#include "Column.h"
#include "ColumnStringIO.h"
#include "backend/core/datatypes/filter.h"
#include "backend/gsl/ExpressionParser.h"
#include "backend/lib/trace.h"
#include "backend/spreadsheet/Spreadsheet.h"

#include "backend/nsl/nsl_stats.h"
#include <gsl/gsl_math.h>
#include <gsl/gsl_statistics.h>

#include "functions.h"

#include <array>
#include <unordered_map>

namespace {
template<typename T>
void determineNewIndices(T value, T reference, int index, int& lowerIndex, int& higherIndex, bool smaller, bool increase) {
	if (increase) {
		if (smaller) {
			if (value >= reference)
				higherIndex = index;
			else if (value < reference)
				lowerIndex = index;
		} else {
			if (value > reference)
				higherIndex = index;
			else if (value <= reference)
				lowerIndex = index;
		}
	} else {
		if (smaller) {
			if (value >= reference)
				lowerIndex = index;
			else if (value < reference)
				higherIndex = index;
		} else {
			if (value > reference)
				lowerIndex = index;
			else if (value <= reference)
				higherIndex = index;
		}
	}
}

template<typename T>
int finalIndex(T valueLowerIndex, T valueHigherIndex, T reference, int lowerIndex, int higherIndex, bool smaller, bool increase) {
	if (smaller) {
		if (increase) {
			if (std::abs(valueLowerIndex - reference) <= std::abs(valueHigherIndex - reference))
				return lowerIndex;
			return higherIndex;
		}
		if (std::abs(valueLowerIndex - reference) < std::abs(valueHigherIndex - reference))
			return lowerIndex;
		return higherIndex;
	}
	// larger index
	if (increase) {
		if (std::abs(valueLowerIndex - reference) < std::abs(valueHigherIndex - reference))
			return lowerIndex;
		return higherIndex;
	}

	if (std::abs(valueLowerIndex - reference) <= std::abs(valueHigherIndex - reference))
		return lowerIndex;
	return higherIndex;
}

template<typename T>
int indexForValueCommon(const T* obj,
						double x,
						const std::function<AbstractColumn::ColumnMode(const T*)> columnMode,
						const std::function<int(const T*)> rowCount,
						const std::function<double(const T*, int)> valueAt,
						const std::function<QDateTime(const T*, int)> dateTimeAt,
						const std::function<AbstractColumn::Properties(const T*)> properties,
						const std::function<bool(const T*, int)> isValid,
						const std::function<bool(const T*, int)> isMasked,
						bool smaller) {
	int rc = rowCount(obj);
	double prevValue = 0;
	qint64 prevValueDateTime = 0;
	auto mode = columnMode(obj);
	auto property = properties(obj);
	if (property == Column::Properties::MonotonicIncreasing || property == Column::Properties::MonotonicDecreasing) {
		// bisects the index every time, so it is possible to find the value in log_2(rowCount) steps
		bool increase = (property != Column::Properties::MonotonicDecreasing);

		int lowerIndex = 0;
		int higherIndex = rc - 1;

		unsigned int maxSteps = ColumnPrivate::calculateMaxSteps(static_cast<unsigned int>(rc)) + 1;

		switch (mode) {
		case Column::ColumnMode::Double:
		case Column::ColumnMode::Integer:
		case Column::ColumnMode::BigInt:
			for (unsigned int i = 0; i < maxSteps; i++) { // so no log_2(rowCount) needed
				int index = lowerIndex + round(static_cast<double>(higherIndex - lowerIndex) / 2);
				double value = valueAt(obj, index);

				if (higherIndex - lowerIndex < 2)
					return finalIndex(valueAt(obj, lowerIndex), valueAt(obj, higherIndex), x, lowerIndex, higherIndex, smaller, increase);

				determineNewIndices(value, x, index, lowerIndex, higherIndex, smaller, increase);
			}
			break;
		case Column::ColumnMode::Text:
			break;
		case Column::ColumnMode::DateTime:
		case Column::ColumnMode::Month:
		case Column::ColumnMode::Day: {
			qint64 xInt64 = static_cast<qint64>(x);
			for (unsigned int i = 0; i < maxSteps; i++) { // so no log_2(rowCount) needed
				int index = lowerIndex + round(static_cast<double>(higherIndex - lowerIndex) / 2);
				qint64 value = dateTimeAt(obj, index).toMSecsSinceEpoch();

				if (higherIndex - lowerIndex < 2)
					return finalIndex(dateTimeAt(obj, lowerIndex).toMSecsSinceEpoch(),
									  dateTimeAt(obj, higherIndex).toMSecsSinceEpoch(),
									  xInt64,
									  lowerIndex,
									  higherIndex,
									  smaller,
									  increase);

				determineNewIndices(value, xInt64, index, lowerIndex, higherIndex, smaller, increase);
			}
		}
		}

	} else if (property == Column::Properties::Constant) {
		if (rc > 0)
			return 0;
		else
			return -1;
	} else {
		// naiv way
		int index = 0;
		switch (mode) {
		case Column::ColumnMode::Double:
		case Column::ColumnMode::Integer:
		case Column::ColumnMode::BigInt:
			for (int row = 0; row < rc; row++) {
				if (!isValid(obj, row) || isMasked(obj, row))
					continue;
				if (row == 0)
					prevValue = valueAt(obj, row);

				double value = valueAt(obj, row);
				if (std::abs(value - x) <= std::abs(prevValue - x)) { // <= prevents also that row - 1 become < 0
					prevValue = value;
					index = row;
				}
			}
			return index;
		case Column::ColumnMode::Text:
			break;
		case Column::ColumnMode::DateTime:
		case Column::ColumnMode::Month:
		case Column::ColumnMode::Day: {
			qint64 xInt64 = static_cast<qint64>(x);
			for (int row = 0; row < rc; row++) {
				if (!isValid(obj, row) || isMasked(obj, row))
					continue;

				if (row == 0)
					prevValueDateTime = dateTimeAt(obj, row).toMSecsSinceEpoch();

				qint64 value = dateTimeAt(obj, row).toMSecsSinceEpoch();
				if (std::abs(value - xInt64) <= std::abs(prevValueDateTime - xInt64)) { // "<=" prevents also that row - 1 become < 0
					prevValueDateTime = value;
					index = row;
				}
			}
			return index;
		}
		}
	}
	return -1;
}
} // anonymous namespace

void ColumnPrivate::ValueLabels::setMode(AbstractColumn::ColumnMode mode) {
	if (!initialized())
		m_mode = mode;
	else
		migrateLabels(mode);
}

bool ColumnPrivate::ValueLabels::init(AbstractColumn::ColumnMode mode) {
	if (initialized())
		return false;

	invalidateStatistics();

	m_mode = mode;
	switch (m_mode) {
	case AbstractColumn::ColumnMode::Double:
		m_labels = new QVector<Column::ValueLabel<double>>();
		break;
	case AbstractColumn::ColumnMode::Integer:
		m_labels = new QVector<Column::ValueLabel<int>>();
		break;
	case AbstractColumn::ColumnMode::BigInt:
		m_labels = new QVector<Column::ValueLabel<qint64>>();
		break;
	case AbstractColumn::ColumnMode::Text:
		m_labels = new QVector<Column::ValueLabel<QString>>();
		break;
	case AbstractColumn::ColumnMode::DateTime:
	case AbstractColumn::ColumnMode::Month:
	case AbstractColumn::ColumnMode::Day:
		m_labels = new QVector<Column::ValueLabel<QDateTime>>();
		break;
	}
	return true;
}

void ColumnPrivate::ValueLabels::deinit() {
	invalidateStatistics();
	if (m_labels) {
		switch (m_mode) {
		case AbstractColumn::ColumnMode::Double:
			delete cast_vector<double>();
			break;
		case AbstractColumn::ColumnMode::Integer:
			delete cast_vector<int>();
			break;
		case AbstractColumn::ColumnMode::BigInt:
			delete cast_vector<qint64>();
			break;
		case AbstractColumn::ColumnMode::Text:
			delete cast_vector<QString>();
			break;
		case AbstractColumn::ColumnMode::DateTime:
		case AbstractColumn::ColumnMode::Month:
		case AbstractColumn::ColumnMode::Day:
			delete cast_vector<QDateTime>();
			break;
		}

		m_labels = nullptr;
	}
}

AbstractColumn::ColumnMode ColumnPrivate::ValueLabels::mode() const {
	return m_mode;
}
AbstractColumn::Properties ColumnPrivate::ValueLabels::properties() const {
	return AbstractColumn::Properties::No; // Performance improvements not yet implemented
}

double ColumnPrivate::ValueLabels::valueAt(int index) const {
	if (!initialized())
		return 0;

	switch (m_mode) {
	case AbstractColumn::ColumnMode::Double:
		return cast_vector<double>()->at(index).value;
	case AbstractColumn::ColumnMode::Integer:
		return cast_vector<int>()->at(index).value;
	case AbstractColumn::ColumnMode::BigInt:
		return cast_vector<qint64>()->at(index).value;
	case AbstractColumn::ColumnMode::Text:
		return std::nan("0");
	case AbstractColumn::ColumnMode::DateTime:
	case AbstractColumn::ColumnMode::Month:
	case AbstractColumn::ColumnMode::Day:
		return cast_vector<QDateTime>()->at(index).value.toMSecsSinceEpoch();
	}
	Q_ASSERT(false);
	return std::nan("0");
}

QDateTime ColumnPrivate::ValueLabels::dateTimeAt(int index) const {
	if (!initialized())
		return QDateTime();

	switch (m_mode) {
	case AbstractColumn::ColumnMode::DateTime:
	case AbstractColumn::ColumnMode::Month:
	case AbstractColumn::ColumnMode::Day:
		return cast_vector<QDateTime>()->at(index).value;
	case AbstractColumn::ColumnMode::Double:
	case AbstractColumn::ColumnMode::Integer:
	case AbstractColumn::ColumnMode::BigInt:
	case AbstractColumn::ColumnMode::Text:
		return QDateTime();
	}
	Q_ASSERT(false);
	return QDateTime();
}

void ColumnPrivate::ValueLabels::migrateLabels(AbstractColumn::ColumnMode newMode) {
	switch (mode()) {
	case AbstractColumn::ColumnMode::Double:
		migrateDoubleTo(newMode);
		break;
	case AbstractColumn::ColumnMode::Integer:
		migrateIntTo(newMode);
		break;
	case AbstractColumn::ColumnMode::BigInt:
		migrateBigIntTo(newMode);
		break;
	case AbstractColumn::ColumnMode::Text:
		migrateTextTo(newMode);
		break;
	case AbstractColumn::ColumnMode::DateTime:
	case AbstractColumn::ColumnMode::Month:
	case AbstractColumn::ColumnMode::Day:
		migrateDateTimeTo(newMode);
		break;
	}
}

void ColumnPrivate::ValueLabels::migrateDoubleTo(AbstractColumn::ColumnMode newMode) {
	if (newMode == AbstractColumn::ColumnMode::Double)
		return;

	auto vector = *cast_vector<double>();
	deinit();
	init(newMode);
	switch (newMode) {
	case AbstractColumn::ColumnMode::Double:
		break; // Nothing to do
	case AbstractColumn::ColumnMode::Integer:
		for (const auto& value : vector)
			add((int)value.value, value.label);
		break;
	case AbstractColumn::ColumnMode::BigInt:
		for (const auto& value : vector)
			add((qint64)value.value, value.label);
		break;
	case AbstractColumn::ColumnMode::Text:
		for (const auto& value : vector)
			add(QString::number(value.value), value.label);
		break;
	case AbstractColumn::ColumnMode::DateTime:
	case AbstractColumn::ColumnMode::Month:
	case AbstractColumn::ColumnMode::Day:
		// Not possible
		// All value labels deleted
		break;
	}
}

void ColumnPrivate::ValueLabels::migrateIntTo(AbstractColumn::ColumnMode newMode) {
	if (newMode == AbstractColumn::ColumnMode::Integer)
		return;

	auto vector = *cast_vector<int>();
	deinit();
	init(newMode);
	switch (newMode) {
	case AbstractColumn::ColumnMode::Double:
		for (const auto& value : vector)
			add((double)value.value, value.label);
		break;
	case AbstractColumn::ColumnMode::Integer:
		// nothing to do
		break;
	case AbstractColumn::ColumnMode::BigInt:
		for (const auto& value : vector)
			add((qint64)value.value, value.label);
		break;
	case AbstractColumn::ColumnMode::Text:
		for (const auto& value : vector)
			add(QString::number(value.value), value.label);
		break;
	case AbstractColumn::ColumnMode::DateTime:
	case AbstractColumn::ColumnMode::Month:
	case AbstractColumn::ColumnMode::Day:
		// Not possible
		// All value labels deleted
		break;
	}
}

void ColumnPrivate::ValueLabels::migrateBigIntTo(AbstractColumn::ColumnMode newMode) {
	if (newMode == AbstractColumn::ColumnMode::BigInt)
		return;

	auto vector = *cast_vector<qint64>();
	deinit();
	init(newMode);
	switch (newMode) {
	case AbstractColumn::ColumnMode::Double:
		for (const auto& value : vector)
			add((double)value.value, value.label);
		break;
	case AbstractColumn::ColumnMode::Integer:
		for (const auto& value : vector)
			add((int)value.value, value.label);
		break;
	case AbstractColumn::ColumnMode::BigInt:
		// Nothing to do
		break;
	case AbstractColumn::ColumnMode::Text:
		for (const auto& value : vector)
			add(QString::number(value.value), value.label);
		break;
	case AbstractColumn::ColumnMode::DateTime:
	case AbstractColumn::ColumnMode::Month:
	case AbstractColumn::ColumnMode::Day:
		// Not possible
		// All value labels deleted
		break;
	}
}

void ColumnPrivate::ValueLabels::migrateTextTo(AbstractColumn::ColumnMode newMode) {
	if (newMode == AbstractColumn::ColumnMode::Text)
		return;

	auto vector = *cast_vector<QString>();
	deinit();
	init(newMode);
	switch (newMode) {
	case AbstractColumn::ColumnMode::Double: {
		for (const auto& value : vector) {
			bool ok;
			double v = value.value.toDouble(&ok);
			if (ok)
				add(v, value.label);
		}
		break;
	}
	case AbstractColumn::ColumnMode::Integer: {
		for (const auto& value : vector) {
			bool ok;
			int v = value.value.toInt(&ok);
			if (ok)
				add(v, value.label);
		}
		break;
	}
	case AbstractColumn::ColumnMode::BigInt: {
		for (const auto& value : vector) {
			bool ok;
			qint64 v = value.value.toLongLong(&ok);
			if (ok)
				add(v, value.label);
		}
		break;
	}
	case AbstractColumn::ColumnMode::Text:
		// Nothing to do
		break;
	case AbstractColumn::ColumnMode::DateTime:
	case AbstractColumn::ColumnMode::Month:
	case AbstractColumn::ColumnMode::Day:
		// Not supported
		break;
	}
}

int ColumnPrivate::ValueLabels::indexForValue(double value, bool smaller) const {
	return indexForValueCommon<ValueLabels>(this,
											value,
											std::mem_fn(&ValueLabels::mode),
											std::mem_fn<int() const>(&ValueLabels::count),
											std::mem_fn(&ValueLabels::valueAt),
											std::mem_fn(&ValueLabels::dateTimeAt),
											std::mem_fn(&ValueLabels::properties),
											std::mem_fn<bool(int) const>(&ValueLabels::isValid),
											std::mem_fn<bool(int) const>(&ValueLabels::isMasked),
											smaller);
}

bool ColumnPrivate::ValueLabels::isValid(int) const {
	return true;
}

bool ColumnPrivate::ValueLabels::isMasked(int) const {
	return false;
}

QString ColumnPrivate::ValueLabels::labelAt(int index) const {
	if (!initialized())
		return {};

	switch (m_mode) {
	case AbstractColumn::ColumnMode::Double:
		return cast_vector<double>()->at(index).label;
	case AbstractColumn::ColumnMode::Integer:
		return cast_vector<int>()->at(index).label;
	case AbstractColumn::ColumnMode::BigInt:
		return cast_vector<qint64>()->at(index).label;
	case AbstractColumn::ColumnMode::Text:
		return cast_vector<QString>()->at(index).label;
	case AbstractColumn::ColumnMode::DateTime:
	case AbstractColumn::ColumnMode::Month:
	case AbstractColumn::ColumnMode::Day:
		return cast_vector<QDateTime>()->at(index).label;
	}
	Q_ASSERT(false);
	return {};
}

double ColumnPrivate::ValueLabels::minimum() {
	if (!m_statistics.available)
		recalculateStatistics();
	return m_statistics.minimum;
}

double ColumnPrivate::ValueLabels::maximum() {
	if (!m_statistics.available)
		recalculateStatistics();
	return m_statistics.maximum;
}

void ColumnPrivate::ValueLabels::recalculateStatistics() {
	m_statistics.available = false;
	m_statistics.minimum = INFINITY;
	m_statistics.maximum = -INFINITY;

	const int rowValuesSize = count();
	for (int row = 0; row < rowValuesSize; ++row) {
		const double val = valueAt(row);
		if (val < m_statistics.minimum)
			m_statistics.minimum = val;
		if (val > m_statistics.maximum)
			m_statistics.maximum = val;
	}

	m_statistics.available = true;
}

void ColumnPrivate::ValueLabels::invalidateStatistics() {
	m_statistics.available = false;
}

void ColumnPrivate::ValueLabels::migrateDateTimeTo(AbstractColumn::ColumnMode newMode) {
	if (newMode == AbstractColumn::ColumnMode::DateTime || newMode == AbstractColumn::ColumnMode::Day || newMode == AbstractColumn::ColumnMode::Month)
		return;

	// auto vector = *cast_vector<QDateTime>();
	deinit();
	init(newMode);
	// switch (newMode) {
	// case AbstractColumn::ColumnMode::Double: {
	//     // Not possible
	//     break;
	// }
	// case AbstractColumn::ColumnMode::Integer: {
	//     // Not possible
	//     break;
	// }
	// case AbstractColumn::ColumnMode::BigInt: {
	//     // Not possible
	//     break;
	// }
	// case AbstractColumn::ColumnMode::Text:
	//     // Not supported
	//     break;
	// case AbstractColumn::ColumnMode::DateTime:
	// case AbstractColumn::ColumnMode::Month:
	// case AbstractColumn::ColumnMode::Day:
	//     // Nothing to do
	//     break;
	// }
}

int ColumnPrivate::ValueLabels::count() const {
	if (!initialized())
		return 0;

	switch (m_mode) {
	case AbstractColumn::ColumnMode::Double:
		return cast_vector<double>()->count();
	case AbstractColumn::ColumnMode::Integer:
		return cast_vector<int>()->count();
	case AbstractColumn::ColumnMode::BigInt:
		return cast_vector<qint64>()->count();
	case AbstractColumn::ColumnMode::Text:
		return cast_vector<QString>()->count();
	case AbstractColumn::ColumnMode::DateTime:
	case AbstractColumn::ColumnMode::Month:
	case AbstractColumn::ColumnMode::Day:
		return cast_vector<QDateTime>()->count();
	}
	return 0;
}

int ColumnPrivate::ValueLabels::count(double min, double max) const {
	if (!initialized())
		return 0;

	min = qMin(min, max);
	max = qMax(min, max);

	int counter = 0;
	switch (m_mode) {
	case AbstractColumn::ColumnMode::Double: {
		const auto* data = cast_vector<double>();
		for (const auto& d : *data) {
			if (d.value >= min && d.value <= max)
				counter++;
		}
		break;
	}
	case AbstractColumn::ColumnMode::Integer: {
		const auto* data = cast_vector<int>();
		for (const auto& d : *data) {
			if (d.value >= min && d.value <= max)
				counter++;
		}
		break;
	}
	case AbstractColumn::ColumnMode::BigInt: {
		const auto* data = cast_vector<qint64>();
		for (const auto& d : *data) {
			if (d.value >= min && d.value <= max)
				counter++;
		}
		break;
	}
	case AbstractColumn::ColumnMode::Text:
		return 0;
	case AbstractColumn::ColumnMode::DateTime:
	case AbstractColumn::ColumnMode::Month:
	case AbstractColumn::ColumnMode::Day: {
		const auto* data = cast_vector<QDateTime>();
		for (const auto& d : *data) {
			const auto value = d.value.toMSecsSinceEpoch();
			if (value >= min && value <= max)
				counter++;
		}
		break;
	}
	}
	return counter;
}

void ColumnPrivate::ValueLabels::add(const QString& value, const QString& label) {
	if (initialized() && m_mode != AbstractColumn::ColumnMode::Text)
		return;

	init(AbstractColumn::ColumnMode::Text);
	invalidateStatistics();
	cast_vector<QString>()->append({value, label});
}

void ColumnPrivate::ValueLabels::add(const QDateTime& value, const QString& label) {
	if (initialized() && m_mode != AbstractColumn::ColumnMode::DateTime && m_mode != AbstractColumn::ColumnMode::Day
		&& m_mode != AbstractColumn::ColumnMode::Month)
		return;

	init(AbstractColumn::ColumnMode::Month);
	invalidateStatistics();
	cast_vector<QDateTime>()->append({value, label});
}

void ColumnPrivate::ValueLabels::add(double value, const QString& label) {
	if (initialized() && m_mode != AbstractColumn::ColumnMode::Double)
		return;

	init(AbstractColumn::ColumnMode::Double);
	invalidateStatistics();
	cast_vector<double>()->append({value, label});
}

void ColumnPrivate::ValueLabels::add(int value, const QString& label) {
	if (initialized() && m_mode != AbstractColumn::ColumnMode::Integer)
		return;

	init(AbstractColumn::ColumnMode::Integer);
	invalidateStatistics();
	cast_vector<int>()->append({value, label});
}

void ColumnPrivate::ValueLabels::add(qint64 value, const QString& label) {
	if (initialized() && m_mode != AbstractColumn::ColumnMode::BigInt)
		return;

	init(AbstractColumn::ColumnMode::BigInt);
	invalidateStatistics();
	cast_vector<qint64>()->append({value, label});
}

void ColumnPrivate::ValueLabels::removeAll() {
	if (!initialized())
		return;

	deinit();
	init(m_mode);
}

void ColumnPrivate::ValueLabels::remove(const QString& key) {
	if (!initialized())
		return;

	invalidateStatistics();
	bool ok;
	switch (m_mode) {
	case AbstractColumn::ColumnMode::Double: {
		double value = QLocale().toDouble(key, &ok);
		if (!ok)
			return;
		remove<double>(value);
		break;
	}
	case AbstractColumn::ColumnMode::Integer: {
		int value = QLocale().toInt(key, &ok);
		if (!ok)
			return;
		remove<int>(value);
		break;
	}
	case AbstractColumn::ColumnMode::BigInt: {
		qint64 value = QLocale().toLongLong(key, &ok);
		if (!ok)
			return;
		remove<qint64>(value);
		break;
	}
	case AbstractColumn::ColumnMode::Text: {
		remove<QString>(key);
		break;
	}
	case AbstractColumn::ColumnMode::Month:
	case AbstractColumn::ColumnMode::Day:
	case AbstractColumn::ColumnMode::DateTime: {
		DateTime2StringFilter f;
		if (m_mode == AbstractColumn::ColumnMode::Month) {
			f.setFormat(QStringLiteral("MMMM"));
		} else {
			f.setFormat(QStringLiteral("dddd"));
		}
		const auto ref = QDateTime::fromString(key, f.format());
		remove<QDateTime>(ref);
		break;
	}
	}
}

const QVector<Column::ValueLabel<QString>>* ColumnPrivate::ValueLabels::textValueLabels() const {
	if (!initialized() || m_mode != AbstractColumn::ColumnMode::Text)
		return nullptr;
	return cast_vector<QString>();
}

const QVector<Column::ValueLabel<QDateTime>>* ColumnPrivate::ValueLabels::dateTimeValueLabels() const {
	if (!initialized()
		|| (m_mode != AbstractColumn::ColumnMode::DateTime && m_mode != AbstractColumn::ColumnMode::Day && m_mode != AbstractColumn::ColumnMode::Month))
		return nullptr;
	return cast_vector<QDateTime>();
}

const QVector<Column::ValueLabel<double>>* ColumnPrivate::ValueLabels::valueLabels() const {
	if (!initialized() || m_mode != AbstractColumn::ColumnMode::Double)
		return nullptr;
	return cast_vector<double>();
}

const QVector<Column::ValueLabel<int>>* ColumnPrivate::ValueLabels::intValueLabels() const {
	if (!initialized() || m_mode != AbstractColumn::ColumnMode::Integer)
		return nullptr;
	return cast_vector<int>();
}

const QVector<Column::ValueLabel<qint64>>* ColumnPrivate::ValueLabels::bigIntValueLabels() const {
	if (!initialized() || m_mode != AbstractColumn::ColumnMode::BigInt)
		return nullptr;
	return cast_vector<qint64>();
}

// ######################################################################################################
// ######################################################################################################
// ######################################################################################################

ColumnPrivate::ColumnPrivate(Column* owner, AbstractColumn::ColumnMode mode)
	: AbstractColumnPrivate(owner)
	, q(owner)
	, m_columnMode(mode) {
	initIOFilters();
}

/**
 * \brief Special ctor (to be called from Column only!)
 */
ColumnPrivate::ColumnPrivate(Column* owner, AbstractColumn::ColumnMode mode, void* data)
	: AbstractColumnPrivate(owner)
	, q(owner)
	, m_columnMode(mode)
	, m_data(data) {
	initIOFilters();
}

/*!
 * initializes the interval vector for data. This is where the actual allocation on the heap is happening.
 * If \c resize is set to \false, the vector is not resized after its creation. This should be used
 * if there is already a vector created somewhere and the content of the column is going to be replaced
 * with the existing content where the memory was already allocated.
 */
bool ColumnPrivate::initDataContainer(bool resize) {
	switch (m_columnMode) {
	case AbstractColumn::ColumnMode::Double: {
		auto* vec = new QVector<double>();
		try {
			if (resize)
				vec->resize(m_rowCount);
		} catch (std::bad_alloc&) {
			return false;
		}
		vec->fill(std::numeric_limits<double>::quiet_NaN());
		m_data = vec;
		break;
	}
	case AbstractColumn::ColumnMode::Integer: {
		auto* vec = new QVector<int>();
		try {
			if (resize)
				vec->resize(m_rowCount);
		} catch (std::bad_alloc&) {
			return false;
		}
		m_data = vec;
		break;
	}
	case AbstractColumn::ColumnMode::BigInt: {
		auto* vec = new QVector<qint64>();
		try {
			if (resize)
				vec->resize(m_rowCount);
		} catch (std::bad_alloc&) {
			return false;
		}
		m_data = vec;
		break;
	}
	case AbstractColumn::ColumnMode::Text: {
		auto* vec = new QVector<QString>();
		try {
			if (resize)
				vec->resize(m_rowCount);
		} catch (std::bad_alloc&) {
			return false;
		}
		m_data = vec;
		break;
	}
	case AbstractColumn::ColumnMode::DateTime:
	case AbstractColumn::ColumnMode::Month:
	case AbstractColumn::ColumnMode::Day: {
		auto* vec = new QVector<QDateTime>();
		try {
			if (resize)
				vec->resize(m_rowCount);
		} catch (std::bad_alloc&) {
			return false;
		}
		m_data = vec;
		break;
	}
	}

	return true;
}

void ColumnPrivate::initIOFilters() {
	const auto numberLocale = QLocale();
	switch (m_columnMode) {
	case AbstractColumn::ColumnMode::Double:
		m_inputFilter = new String2DoubleFilter();
		m_inputFilter->setNumberLocale(numberLocale);
		m_outputFilter = new Double2StringFilter();
		m_outputFilter->setNumberLocale(numberLocale);
		break;
	case AbstractColumn::ColumnMode::Integer:
		m_inputFilter = new String2IntegerFilter();
		m_inputFilter->setNumberLocale(numberLocale);
		m_outputFilter = new Integer2StringFilter();
		m_outputFilter->setNumberLocale(numberLocale);
		break;
	case AbstractColumn::ColumnMode::BigInt:
		m_inputFilter = new String2BigIntFilter();
		m_inputFilter->setNumberLocale(numberLocale);
		m_outputFilter = new BigInt2StringFilter();
		m_outputFilter->setNumberLocale(numberLocale);
		break;
	case AbstractColumn::ColumnMode::Text:
		m_inputFilter = new SimpleCopyThroughFilter();
		m_outputFilter = new SimpleCopyThroughFilter();
		break;
	case AbstractColumn::ColumnMode::DateTime:
		m_inputFilter = new String2DateTimeFilter();
		m_outputFilter = new DateTime2StringFilter();
		break;
	case AbstractColumn::ColumnMode::Month:
		m_inputFilter = new String2MonthFilter();
		m_outputFilter = new DateTime2StringFilter();
		static_cast<DateTime2StringFilter*>(m_outputFilter)->setFormat(QStringLiteral("MMMM"));
		break;
	case AbstractColumn::ColumnMode::Day:
		m_inputFilter = new String2DayOfWeekFilter();
		m_outputFilter = new DateTime2StringFilter();
		static_cast<DateTime2StringFilter*>(m_outputFilter)->setFormat(QStringLiteral("dddd"));
		break;
	}

	connect(m_outputFilter, &AbstractSimpleFilter::formatChanged, q, &Column::handleFormatChange);
}

ColumnPrivate::~ColumnPrivate() {
	deleteData();
}

void ColumnPrivate::deleteData() {
	if (!m_data)
		return;

	switch (m_columnMode) {
	case AbstractColumn::ColumnMode::Double:
		delete static_cast<QVector<double>*>(m_data);
		break;
	case AbstractColumn::ColumnMode::Integer:
		delete static_cast<QVector<int>*>(m_data);
		break;
	case AbstractColumn::ColumnMode::BigInt:
		delete static_cast<QVector<qint64>*>(m_data);
		break;
	case AbstractColumn::ColumnMode::Text:
		delete static_cast<QVector<QString>*>(m_data);
		break;
	case AbstractColumn::ColumnMode::DateTime:
	case AbstractColumn::ColumnMode::Month:
	case AbstractColumn::ColumnMode::Day:
		delete static_cast<QVector<QDateTime>*>(m_data);
		break;
	}
	m_data = nullptr;
}

AbstractColumn::ColumnMode ColumnPrivate::columnMode() const {
	return m_columnMode;
}

/**
 * \brief Set the column mode
 *
 * This sets the column mode and, if
 * necessary, converts it to another datatype.
 * Remark: setting the mode back to undefined (the
 * initial value) is not supported.
 */
void ColumnPrivate::setColumnMode(AbstractColumn::ColumnMode mode) {
	DEBUG(Q_FUNC_INFO << ", " << ENUM_TO_STRING(AbstractColumn, ColumnMode, m_columnMode) << " -> " << ENUM_TO_STRING(AbstractColumn, ColumnMode, mode))
	if (mode == m_columnMode)
		return;

	void* old_data = m_data;
	// remark: the deletion of the old data will be done in the dtor of a command

	AbstractSimpleFilter *filter{nullptr}, *new_in_filter{nullptr}, *new_out_filter{nullptr};
	bool filter_is_temporary = false; // it can also become outputFilter(), which we may not delete here
	Column* temp_col = nullptr;

	Q_EMIT q->modeAboutToChange(q);

	// determine the conversion filter and allocate the new data vector
	switch (m_columnMode) { // old mode
	case AbstractColumn::ColumnMode::Double: {
		disconnect(static_cast<Double2StringFilter*>(m_outputFilter), &Double2StringFilter::formatChanged, q, &Column::handleFormatChange);
		switch (mode) {
		case AbstractColumn::ColumnMode::Double:
			break;
		case AbstractColumn::ColumnMode::Integer:
			filter = new Double2IntegerFilter();
			filter_is_temporary = true;
			if (m_data) {
				temp_col = new Column(QStringLiteral("temp_col"), *(static_cast<QVector<double>*>(old_data)));
				m_data = new QVector<int>();
			}
			break;
		case AbstractColumn::ColumnMode::BigInt:
			filter = new Double2BigIntFilter();
			filter_is_temporary = true;
			if (m_data) {
				temp_col = new Column(QStringLiteral("temp_col"), *(static_cast<QVector<double>*>(old_data)));
				m_data = new QVector<qint64>();
			}
			break;
		case AbstractColumn::ColumnMode::Text:
			filter = outputFilter();
			filter_is_temporary = false;
			if (m_data) {
				temp_col = new Column(QStringLiteral("temp_col"), *(static_cast<QVector<double>*>(old_data)));
				m_data = new QVector<QString>();
			}
			break;
		case AbstractColumn::ColumnMode::DateTime:
			filter = new Double2DateTimeFilter();
			filter_is_temporary = true;
			if (m_data) {
				temp_col = new Column(QStringLiteral("temp_col"), *(static_cast<QVector<double>*>(old_data)));
				m_data = new QVector<QDateTime>();
			}
			break;
		case AbstractColumn::ColumnMode::Month:
			filter = new Double2MonthFilter();
			filter_is_temporary = true;
			if (m_data) {
				temp_col = new Column(QStringLiteral("temp_col"), *(static_cast<QVector<double>*>(old_data)));
				m_data = new QVector<QDateTime>();
			}
			break;
		case AbstractColumn::ColumnMode::Day:
			filter = new Double2DayOfWeekFilter();
			filter_is_temporary = true;
			if (m_data) {
				temp_col = new Column(QStringLiteral("temp_col"), *(static_cast<QVector<double>*>(old_data)));
				m_data = new QVector<QDateTime>();
			}
			break;
		} // switch(mode)

		break;
	}
	case AbstractColumn::ColumnMode::Integer: {
		disconnect(static_cast<Integer2StringFilter*>(m_outputFilter), &Integer2StringFilter::formatChanged, q, &Column::handleFormatChange);
		switch (mode) {
		case AbstractColumn::ColumnMode::Integer:
			break;
		case AbstractColumn::ColumnMode::BigInt:
			filter = new Integer2BigIntFilter();
			filter_is_temporary = true;
			if (m_data) {
				temp_col = new Column(QStringLiteral("temp_col"), *(static_cast<QVector<int>*>(old_data)));
				m_data = new QVector<qint64>();
			}
			break;
		case AbstractColumn::ColumnMode::Double:
			filter = new Integer2DoubleFilter();
			filter_is_temporary = true;
			if (m_data) {
				temp_col = new Column(QStringLiteral("temp_col"), *(static_cast<QVector<int>*>(old_data)));
				m_data = new QVector<double>();
			}
			break;
		case AbstractColumn::ColumnMode::Text:
			filter = outputFilter();
			filter_is_temporary = false;
			if (m_data) {
				temp_col = new Column(QStringLiteral("temp_col"), *(static_cast<QVector<int>*>(old_data)));
				m_data = new QVector<QString>();
			}
			break;
		case AbstractColumn::ColumnMode::DateTime:
			DEBUG(Q_FUNC_INFO << ", int -> datetime")
			filter = new Integer2DateTimeFilter();
			filter_is_temporary = true;
			if (m_data) {
				temp_col = new Column(QStringLiteral("temp_col"), *(static_cast<QVector<int>*>(old_data)));
				m_data = new QVector<QDateTime>();
			}
			DEBUG(Q_FUNC_INFO << ", int -> datetime done")
			break;
		case AbstractColumn::ColumnMode::Month:
			filter = new Integer2MonthFilter();
			filter_is_temporary = true;
			if (m_data) {
				temp_col = new Column(QStringLiteral("temp_col"), *(static_cast<QVector<int>*>(old_data)));
				m_data = new QVector<QDateTime>();
			}
			break;
		case AbstractColumn::ColumnMode::Day:
			filter = new Integer2DayOfWeekFilter();
			filter_is_temporary = true;
			if (m_data) {
				temp_col = new Column(QStringLiteral("temp_col"), *(static_cast<QVector<int>*>(old_data)));
				m_data = new QVector<QDateTime>();
			}
			break;
		} // switch(mode)

		break;
	}
	case AbstractColumn::ColumnMode::BigInt: {
		disconnect(static_cast<BigInt2StringFilter*>(m_outputFilter), &BigInt2StringFilter::formatChanged, q, &Column::handleFormatChange);
		switch (mode) {
		case AbstractColumn::ColumnMode::BigInt:
			break;
		case AbstractColumn::ColumnMode::Integer: // TODO: timeUnit
			filter = new BigInt2IntegerFilter();
			filter_is_temporary = true;
			if (m_data) {
				temp_col = new Column(QStringLiteral("temp_col"), *(static_cast<QVector<qint64>*>(old_data)));
				m_data = new QVector<int>();
			}
			break;
		case AbstractColumn::ColumnMode::Double: // TODO: timeUnit
			filter = new BigInt2DoubleFilter();
			filter_is_temporary = true;
			if (m_data) {
				temp_col = new Column(QStringLiteral("temp_col"), *(static_cast<QVector<qint64>*>(old_data)));
				m_data = new QVector<double>();
			}
			break;
		case AbstractColumn::ColumnMode::Text:
			filter = outputFilter();
			filter_is_temporary = false;
			if (m_data) {
				temp_col = new Column(QStringLiteral("temp_col"), *(static_cast<QVector<qint64>*>(old_data)));
				m_data = new QVector<QString>();
			}
			break;
		case AbstractColumn::ColumnMode::DateTime: // TODO: timeUnit
			filter = new BigInt2DateTimeFilter();
			filter_is_temporary = true;
			if (m_data) {
				temp_col = new Column(QStringLiteral("temp_col"), *(static_cast<QVector<qint64>*>(old_data)));
				m_data = new QVector<QDateTime>();
			}
			break;
		case AbstractColumn::ColumnMode::Month:
			filter = new BigInt2MonthFilter();
			filter_is_temporary = true;
			if (m_data) {
				temp_col = new Column(QStringLiteral("temp_col"), *(static_cast<QVector<qint64>*>(old_data)));
				m_data = new QVector<QDateTime>();
			}
			break;
		case AbstractColumn::ColumnMode::Day:
			filter = new BigInt2DayOfWeekFilter();
			filter_is_temporary = true;
			if (m_data) {
				temp_col = new Column(QStringLiteral("temp_col"), *(static_cast<QVector<qint64>*>(old_data)));
				m_data = new QVector<QDateTime>();
			}
			break;
		} // switch(mode)

		break;
	}
	case AbstractColumn::ColumnMode::Text: {
		switch (mode) {
		case AbstractColumn::ColumnMode::Text:
			break;
		case AbstractColumn::ColumnMode::Double:
			filter = new String2DoubleFilter();
			filter->setNumberLocale(QLocale());
			filter_is_temporary = true;
			if (m_data) {
				temp_col = new Column(QStringLiteral("temp_col"), *(static_cast<QVector<QString>*>(old_data)));
				m_data = new QVector<double>();
			}
			break;
		case AbstractColumn::ColumnMode::Integer:
			filter = new String2IntegerFilter();
			filter->setNumberLocale(QLocale());
			filter_is_temporary = true;
			if (m_data) {
				temp_col = new Column(QStringLiteral("temp_col"), *(static_cast<QVector<QString>*>(old_data)));
				m_data = new QVector<int>();
			}
			break;
		case AbstractColumn::ColumnMode::BigInt:
			filter = new String2BigIntFilter();
			filter->setNumberLocale(QLocale());
			filter_is_temporary = true;
			if (m_data) {
				temp_col = new Column(QStringLiteral("temp_col"), *(static_cast<QVector<QString>*>(old_data)));
				m_data = new QVector<qint64>();
			}
			break;
		case AbstractColumn::ColumnMode::DateTime:
			filter = new String2DateTimeFilter();
			filter_is_temporary = true;
			if (m_data) {
				temp_col = new Column(QStringLiteral("temp_col"), *(static_cast<QVector<QString>*>(old_data)));
				m_data = new QVector<QDateTime>();
			}
			break;
		case AbstractColumn::ColumnMode::Month:
			filter = new String2MonthFilter();
			filter_is_temporary = true;
			if (m_data) {
				temp_col = new Column(QStringLiteral("temp_col"), *(static_cast<QVector<QString>*>(old_data)));
				m_data = new QVector<QDateTime>();
			}
			break;
		case AbstractColumn::ColumnMode::Day:
			filter = new String2DayOfWeekFilter();
			filter_is_temporary = true;
			if (m_data) {
				temp_col = new Column(QStringLiteral("temp_col"), *(static_cast<QVector<QString>*>(old_data)));
				m_data = new QVector<QDateTime>();
			}
			break;
		} // switch(mode)

		break;
	}
	case AbstractColumn::ColumnMode::DateTime:
	case AbstractColumn::ColumnMode::Month:
	case AbstractColumn::ColumnMode::Day: {
		disconnect(static_cast<DateTime2StringFilter*>(m_outputFilter), &DateTime2StringFilter::formatChanged, q, &Column::handleFormatChange);
		switch (mode) {
		case AbstractColumn::ColumnMode::DateTime:
		case AbstractColumn::ColumnMode::Month:
		case AbstractColumn::ColumnMode::Day:
			break;
		case AbstractColumn::ColumnMode::Text:
			filter = outputFilter();
			filter_is_temporary = false;
			if (m_data) {
				temp_col = new Column(QStringLiteral("temp_col"), *(static_cast<QVector<QDateTime>*>(old_data)), m_columnMode);
				m_data = new QStringList();
			}
			break;
		case AbstractColumn::ColumnMode::Double:
			if (m_columnMode == AbstractColumn::ColumnMode::Month)
				filter = new Month2DoubleFilter();
			else if (m_columnMode == AbstractColumn::ColumnMode::Day)
				filter = new DayOfWeek2DoubleFilter();
			else // TODO: timeUnit
				filter = new DateTime2DoubleFilter();
			filter_is_temporary = true;
			if (m_data) {
				temp_col = new Column(QStringLiteral("temp_col"), *(static_cast<QVector<QDateTime>*>(old_data)), m_columnMode);
				m_data = new QVector<double>();
			}
			break;
		case AbstractColumn::ColumnMode::Integer:
			if (m_columnMode == AbstractColumn::ColumnMode::Month)
				filter = new Month2IntegerFilter();
			else if (m_columnMode == AbstractColumn::ColumnMode::Day)
				filter = new DayOfWeek2IntegerFilter();
			else // TODO: timeUnit
				filter = new DateTime2IntegerFilter();
			filter_is_temporary = true;
			if (m_data) {
				temp_col = new Column(QStringLiteral("temp_col"), *(static_cast<QVector<QDateTime>*>(old_data)), m_columnMode);
				m_data = new QVector<int>();
			}
			break;
		case AbstractColumn::ColumnMode::BigInt:
			if (m_columnMode == AbstractColumn::ColumnMode::Month)
				filter = new Month2BigIntFilter();
			else if (m_columnMode == AbstractColumn::ColumnMode::Day)
				filter = new DayOfWeek2BigIntFilter();
			else // TODO: timeUnit
				filter = new DateTime2BigIntFilter();
			filter_is_temporary = true;
			if (m_data) {
				temp_col = new Column(QStringLiteral("temp_col"), *(static_cast<QVector<QDateTime>*>(old_data)), m_columnMode);
				m_data = new QVector<qint64>();
			}
			break;
		} // switch(mode)

		break;
	}
	}

	// determine the new input and output filters
	switch (mode) { // new mode
	case AbstractColumn::ColumnMode::Double:
		new_in_filter = new String2DoubleFilter();
		new_in_filter->setNumberLocale(QLocale());
		new_out_filter = new Double2StringFilter();
		new_out_filter->setNumberLocale(QLocale());
		connect(static_cast<Double2StringFilter*>(new_out_filter), &Double2StringFilter::formatChanged, q, &Column::handleFormatChange);
		break;
	case AbstractColumn::ColumnMode::Integer:
		new_in_filter = new String2IntegerFilter();
		new_in_filter->setNumberLocale(QLocale());
		new_out_filter = new Integer2StringFilter();
		new_out_filter->setNumberLocale(QLocale());
		connect(static_cast<Integer2StringFilter*>(new_out_filter), &Integer2StringFilter::formatChanged, q, &Column::handleFormatChange);
		break;
	case AbstractColumn::ColumnMode::BigInt:
		new_in_filter = new String2BigIntFilter();
		new_in_filter->setNumberLocale(QLocale());
		new_out_filter = new BigInt2StringFilter();
		new_out_filter->setNumberLocale(QLocale());
		connect(static_cast<BigInt2StringFilter*>(new_out_filter), &BigInt2StringFilter::formatChanged, q, &Column::handleFormatChange);
		break;
	case AbstractColumn::ColumnMode::Text:
		new_in_filter = new SimpleCopyThroughFilter();
		new_out_filter = new SimpleCopyThroughFilter();
		break;
	case AbstractColumn::ColumnMode::DateTime:
		new_in_filter = new String2DateTimeFilter();
		new_out_filter = new DateTime2StringFilter();
		connect(static_cast<DateTime2StringFilter*>(new_out_filter), &DateTime2StringFilter::formatChanged, q, &Column::handleFormatChange);
		break;
	case AbstractColumn::ColumnMode::Month:
		new_in_filter = new String2MonthFilter();
		new_out_filter = new DateTime2StringFilter();
		static_cast<DateTime2StringFilter*>(new_out_filter)->setFormat(QStringLiteral("MMMM"));
		// DEBUG("	Month out_filter format: " << STDSTRING(static_cast<DateTime2StringFilter*>(new_out_filter)->format()));
		connect(static_cast<DateTime2StringFilter*>(new_out_filter), &DateTime2StringFilter::formatChanged, q, &Column::handleFormatChange);
		break;
	case AbstractColumn::ColumnMode::Day:
		new_in_filter = new String2DayOfWeekFilter();
		new_out_filter = new DateTime2StringFilter();
		static_cast<DateTime2StringFilter*>(new_out_filter)->setFormat(QStringLiteral("dddd"));
		connect(static_cast<DateTime2StringFilter*>(new_out_filter), &DateTime2StringFilter::formatChanged, q, &Column::handleFormatChange);
		break;
	} // switch(mode)

	m_columnMode = mode;

	m_inputFilter = new_in_filter;
	m_outputFilter = new_out_filter;
	m_inputFilter->input(0, q->m_string_io);
	m_outputFilter->input(0, q);
	m_inputFilter->setHidden(true);
	m_outputFilter->setHidden(true);

	if (temp_col) { // if temp_col == 0, only the input/output filters need to be changed
		// copy the filtered, i.e. converted, column (mode is orig mode)
		DEBUG("	temp_col column mode = " << ENUM_TO_STRING(AbstractColumn, ColumnMode, temp_col->columnMode()));
		filter->input(0, temp_col);
		DEBUG("	filter->output size = " << filter->output(0)->rowCount());
		copy(filter->output(0));
		DEBUG(" DONE")
		delete temp_col;
	}

	if (filter_is_temporary)
		delete filter;

	Q_EMIT q->modeChanged(q);
}

/**
 * \brief Replace all mode related members
 *
 * Replace column mode, data type, data pointer and filters directly
 */
void ColumnPrivate::replaceModeData(AbstractColumn::ColumnMode mode, void* data, AbstractSimpleFilter* in_filter, AbstractSimpleFilter* out_filter) {
	Q_EMIT q->modeAboutToChange(q);
	// disconnect formatChanged()
	switch (m_columnMode) {
	case AbstractColumn::ColumnMode::Double:
		disconnect(static_cast<Double2StringFilter*>(m_outputFilter), &Double2StringFilter::formatChanged, q, &Column::handleFormatChange);
		break;
	case AbstractColumn::ColumnMode::Integer:
		disconnect(static_cast<Integer2StringFilter*>(m_outputFilter), &Integer2StringFilter::formatChanged, q, &Column::handleFormatChange);
		break;
	case AbstractColumn::ColumnMode::BigInt:
		disconnect(static_cast<BigInt2StringFilter*>(m_outputFilter), &BigInt2StringFilter::formatChanged, q, &Column::handleFormatChange);
		break;
	case AbstractColumn::ColumnMode::Text:
		break;
	case AbstractColumn::ColumnMode::DateTime:
	case AbstractColumn::ColumnMode::Month:
	case AbstractColumn::ColumnMode::Day:
		disconnect(static_cast<DateTime2StringFilter*>(m_outputFilter), &DateTime2StringFilter::formatChanged, q, &Column::handleFormatChange);
		break;
	}

	m_columnMode = mode;
	setLabelsMode(mode);
	m_data = data;

	m_inputFilter = in_filter;
	m_outputFilter = out_filter;
	m_inputFilter->input(0, q->m_string_io);
	m_outputFilter->input(0, q);

	// connect formatChanged()
	switch (m_columnMode) {
	case AbstractColumn::ColumnMode::Double:
		connect(static_cast<Double2StringFilter*>(m_outputFilter), &Double2StringFilter::formatChanged, q, &Column::handleFormatChange);
		break;
	case AbstractColumn::ColumnMode::Integer:
		connect(static_cast<Integer2StringFilter*>(m_outputFilter), &Integer2StringFilter::formatChanged, q, &Column::handleFormatChange);
		break;
	case AbstractColumn::ColumnMode::BigInt:
		connect(static_cast<BigInt2StringFilter*>(m_outputFilter), &BigInt2StringFilter::formatChanged, q, &Column::handleFormatChange);
		break;
	case AbstractColumn::ColumnMode::Text:
		break;
	case AbstractColumn::ColumnMode::DateTime:
	case AbstractColumn::ColumnMode::Month:
	case AbstractColumn::ColumnMode::Day:
		connect(static_cast<DateTime2StringFilter*>(m_outputFilter), &DateTime2StringFilter::formatChanged, q, &Column::handleFormatChange);
		break;
	}

	Q_EMIT q->modeChanged(q);
}

/**
 * \brief Replace data pointer
 */
void ColumnPrivate::replaceData(void* data) {
	Q_EMIT q->dataAboutToChange(q);

	m_data = data;
	q->setDataChanged();
}

/**
 * \brief Copy another column of the same type
 *
 * This function will return false if the data type
 * of 'other' is not the same as the type of 'this'.
 * Use a filter to convert a column to another type.
 */
bool ColumnPrivate::copy(const AbstractColumn* other) {
	DEBUG(Q_FUNC_INFO)
	if (other->columnMode() != columnMode())
		return false;
	// 	DEBUG(Q_FUNC_INFO << ", mode = " << ENUM_TO_STRING(AbstractColumn, ColumnMode, columnMode()));
	int num_rows = other->rowCount();
	// 	DEBUG(Q_FUNC_INFO << ", rows " << num_rows);

	Q_EMIT q->dataAboutToChange(q);
	resizeTo(num_rows);

	if (!m_data) {
		if (!initDataContainer())
			return false; // failed to allocate memory
	}

	// copy the data
	switch (m_columnMode) {
	case AbstractColumn::ColumnMode::Double: {
		double* ptr = static_cast<QVector<double>*>(m_data)->data();
		for (int i = 0; i < num_rows; ++i)
			ptr[i] = other->valueAt(i);
		break;
	}
	case AbstractColumn::ColumnMode::Integer: {
		int* ptr = static_cast<QVector<int>*>(m_data)->data();
		for (int i = 0; i < num_rows; ++i)
			ptr[i] = other->integerAt(i);
		break;
	}
	case AbstractColumn::ColumnMode::BigInt: {
		qint64* ptr = static_cast<QVector<qint64>*>(m_data)->data();
		for (int i = 0; i < num_rows; ++i)
			ptr[i] = other->bigIntAt(i);
		break;
	}
	case AbstractColumn::ColumnMode::Text: {
		auto* vec = static_cast<QVector<QString>*>(m_data);
		for (int i = 0; i < num_rows; ++i)
			vec->replace(i, other->textAt(i));
		break;
	}
	case AbstractColumn::ColumnMode::DateTime:
	case AbstractColumn::ColumnMode::Month:
	case AbstractColumn::ColumnMode::Day: {
		auto* vec = static_cast<QVector<QDateTime>*>(m_data);
		for (int i = 0; i < num_rows; ++i)
			vec->replace(i, other->dateTimeAt(i));
		break;
	}
	}

	q->setDataChanged();

	DEBUG(Q_FUNC_INFO << ", done")
	return true;
}

/**
 * \brief Copies a part of another column of the same type
 *
 * This function will return false if the data type
 * of 'other' is not the same as the type of 'this'.
 * \param source pointer to the column to copy
 * \param source_start first row to copy in the column to copy
 * \param dest_start first row to copy in
 * \param num_rows the number of rows to copy
 */
bool ColumnPrivate::copy(const AbstractColumn* source, int source_start, int dest_start, int num_rows) {
	if (source->columnMode() != m_columnMode)
		return false;
	if (num_rows == 0)
		return true;

	Q_EMIT q->dataAboutToChange(q);
	if (dest_start + num_rows > rowCount())
		resizeTo(dest_start + num_rows);

	if (!m_data) {
		if (!initDataContainer())
			return false; // failed to allocate memory
	}

	// copy the data
	switch (m_columnMode) {
	case AbstractColumn::ColumnMode::Double: {
		double* ptr = static_cast<QVector<double>*>(m_data)->data();
		for (int i = 0; i < num_rows; i++)
			ptr[dest_start + i] = source->valueAt(source_start + i);
		break;
	}
	case AbstractColumn::ColumnMode::Integer: {
		int* ptr = static_cast<QVector<int>*>(m_data)->data();
		for (int i = 0; i < num_rows; i++)
			ptr[dest_start + i] = source->integerAt(source_start + i);
		break;
	}
	case AbstractColumn::ColumnMode::BigInt: {
		qint64* ptr = static_cast<QVector<qint64>*>(m_data)->data();
		for (int i = 0; i < num_rows; i++)
			ptr[dest_start + i] = source->bigIntAt(source_start + i);
		break;
	}
	case AbstractColumn::ColumnMode::Text:
		for (int i = 0; i < num_rows; i++)
			static_cast<QVector<QString>*>(m_data)->replace(dest_start + i, source->textAt(source_start + i));
		break;
	case AbstractColumn::ColumnMode::DateTime:
	case AbstractColumn::ColumnMode::Month:
	case AbstractColumn::ColumnMode::Day:
		for (int i = 0; i < num_rows; i++)
			static_cast<QVector<QDateTime>*>(m_data)->replace(dest_start + i, source->dateTimeAt(source_start + i));
		break;
	}

	q->setDataChanged();

	return true;
}

/**
 * \brief Copy another column of the same type
 *
 * This function will return false if the data type
 * of 'other' is not the same as the type of 'this'.
 * Use a filter to convert a column to another type.
 */
bool ColumnPrivate::copy(const ColumnPrivate* other) {
	if (other->columnMode() != m_columnMode)
		return false;
	int num_rows = other->rowCount();

	Q_EMIT q->dataAboutToChange(q);
	resizeTo(num_rows);

	if (!m_data) {
		if (!initDataContainer())
			return false; // failed to allocate memory
	}

	// copy the data
	switch (m_columnMode) {
	case AbstractColumn::ColumnMode::Double: {
		double* ptr = static_cast<QVector<double>*>(m_data)->data();
		for (int i = 0; i < num_rows; ++i)
			ptr[i] = other->valueAt(i);
		break;
	}
	case AbstractColumn::ColumnMode::Integer: {
		int* ptr = static_cast<QVector<int>*>(m_data)->data();
		for (int i = 0; i < num_rows; ++i)
			ptr[i] = other->integerAt(i);
		break;
	}
	case AbstractColumn::ColumnMode::BigInt: {
		qint64* ptr = static_cast<QVector<qint64>*>(m_data)->data();
		for (int i = 0; i < num_rows; ++i)
			ptr[i] = other->bigIntAt(i);
		break;
	}
	case AbstractColumn::ColumnMode::Text:
		for (int i = 0; i < num_rows; ++i)
			static_cast<QVector<QString>*>(m_data)->replace(i, other->textAt(i));
		break;
	case AbstractColumn::ColumnMode::DateTime:
	case AbstractColumn::ColumnMode::Month:
	case AbstractColumn::ColumnMode::Day:
		for (int i = 0; i < num_rows; ++i)
			static_cast<QVector<QDateTime>*>(m_data)->replace(i, other->dateTimeAt(i));
		break;
	}

	q->setDataChanged();

	return true;
}

/**
 * \brief Copies a part of another column of the same type
 *
 * This function will return false if the data type
 * of 'other' is not the same as the type of 'this'.
 * \param source pointer to the column to copy
 * \param source_start first row to copy in the column to copy
 * \param dest_start first row to copy in
 * \param num_rows the number of rows to copy
 */
bool ColumnPrivate::copy(const ColumnPrivate* source, int source_start, int dest_start, int num_rows) {
	if (source->columnMode() != m_columnMode)
		return false;
	if (num_rows == 0)
		return true;

	Q_EMIT q->dataAboutToChange(q);
	if (dest_start + num_rows > rowCount())
		resizeTo(dest_start + num_rows);

	if (!m_data) {
		if (!initDataContainer())
			return false; // failed to allocate memory
	}

	// copy the data
	switch (m_columnMode) {
	case AbstractColumn::ColumnMode::Double: {
		double* ptr = static_cast<QVector<double>*>(m_data)->data();
		for (int i = 0; i < num_rows; ++i)
			ptr[dest_start + i] = source->valueAt(source_start + i);
		break;
	}
	case AbstractColumn::ColumnMode::Integer: {
		int* ptr = static_cast<QVector<int>*>(m_data)->data();
		for (int i = 0; i < num_rows; ++i)
			ptr[dest_start + i] = source->integerAt(source_start + i);
		break;
	}
	case AbstractColumn::ColumnMode::BigInt: {
		qint64* ptr = static_cast<QVector<qint64>*>(m_data)->data();
		for (int i = 0; i < num_rows; ++i)
			ptr[dest_start + i] = source->bigIntAt(source_start + i);
		break;
	}
	case AbstractColumn::ColumnMode::Text:
		for (int i = 0; i < num_rows; ++i)
			static_cast<QVector<QString>*>(m_data)->replace(dest_start + i, source->textAt(source_start + i));
		break;
	case AbstractColumn::ColumnMode::DateTime:
	case AbstractColumn::ColumnMode::Month:
	case AbstractColumn::ColumnMode::Day:
		for (int i = 0; i < num_rows; ++i)
			static_cast<QVector<QDateTime>*>(m_data)->replace(dest_start + i, source->dateTimeAt(source_start + i));
		break;
	}

	q->setDataChanged();

	return true;
}

/**
 * \brief Return the data vector size
 *
 * This returns the size of the column container
 */
int ColumnPrivate::rowCount() const {
	if (!m_data)
		return m_rowCount;

	switch (m_columnMode) {
	case AbstractColumn::ColumnMode::Double:
		return static_cast<QVector<double>*>(m_data)->size();
	case AbstractColumn::ColumnMode::Integer:
		return static_cast<QVector<int>*>(m_data)->size();
	case AbstractColumn::ColumnMode::BigInt:
		return static_cast<QVector<qint64>*>(m_data)->size();
	case AbstractColumn::ColumnMode::DateTime:
	case AbstractColumn::ColumnMode::Month:
	case AbstractColumn::ColumnMode::Day:
		return static_cast<QVector<QDateTime>*>(m_data)->size();
	case AbstractColumn::ColumnMode::Text:
		return static_cast<QVector<QString>*>(m_data)->size();
	}

	return 0;
}

int ColumnPrivate::rowCount(double min, double max) const {
	if (!m_data)
		return m_rowCount;

	int counter = 0;
	switch (m_columnMode) {
	case AbstractColumn::ColumnMode::Double: {
		const auto* data = static_cast<QVector<double>*>(m_data);
		for (const auto& d : *data) {
			if (d >= min && d <= max)
				counter++;
		}
		break;
	}
	case AbstractColumn::ColumnMode::Integer: {
		const auto* data = static_cast<QVector<int>*>(m_data);
		for (const auto& d : *data) {
			if (d >= min && d <= max)
				counter++;
		}
		break;
	}
	case AbstractColumn::ColumnMode::BigInt: {
		const auto* data = static_cast<QVector<qint64>*>(m_data);
		for (const auto& d : *data) {
			if (d >= min && d <= max)
				counter++;
		}
		break;
	}
	case AbstractColumn::ColumnMode::DateTime:
	case AbstractColumn::ColumnMode::Month:
	case AbstractColumn::ColumnMode::Day: {
		const auto* data = static_cast<QVector<QDateTime>*>(m_data);
		for (const auto& d : *data) {
			const auto value = d.toMSecsSinceEpoch();
			if (value >= min && value <= max)
				counter++;
		}
		break;
	}
	case AbstractColumn::ColumnMode::Text:
		break;
	}
	return counter;
}

/**
 * \brief Return the number of available rows
 *
 * This returns the number of rows that actually contain data.
 * Rows beyond this can be masked etc. but should be ignored by filters,
 * plots etc.
 */
int ColumnPrivate::availableRowCount(int max) const {
	int count = 0;
	for (int row = 0; row < rowCount(); row++) {
		if (q->isValid(row) && !q->isMasked(row)) {
			count++;
			if (count == max)
				return max;
		}
	}

	return count;
}

/**
 * \brief Resize the vector to the specified number of rows
 *
 * Since selecting and masking rows higher than the
 * real internal number of rows is supported, this
 * does not change the interval attributes. Also
 * no signal is emitted. If the new rows are filled
 * with values AbstractColumn::dataChanged()
 * must be emitted.
 */
void ColumnPrivate::resizeTo(int new_size) {
	int old_size = rowCount();
	if (new_size == old_size)
		return;

	// 	DEBUG("ColumnPrivate::resizeTo() " << old_size << " -> " << new_size);
	const int new_rows = new_size - old_size;

	if (!m_data) {
		m_rowCount += new_rows;
		return;
	}

	switch (m_columnMode) {
	case AbstractColumn::ColumnMode::Double: {
		auto* data = static_cast<QVector<double>*>(m_data);
		if (new_rows > 0)
			data->insert(data->end(), new_rows, NAN);
		else
			data->remove(old_size - 1 + new_rows, -new_rows);
		break;
	}
	case AbstractColumn::ColumnMode::Integer: {
		auto* data = static_cast<QVector<int>*>(m_data);
		if (new_rows > 0)
			data->insert(data->end(), new_rows, 0);
		else
			data->remove(old_size - 1 + new_rows, -new_rows);
		break;
	}
	case AbstractColumn::ColumnMode::BigInt: {
		auto* data = static_cast<QVector<qint64>*>(m_data);
		if (new_rows > 0)
			data->insert(data->end(), new_rows, 0);
		else
			data->remove(old_size - 1 + new_rows, -new_rows);
		break;
	}
	case AbstractColumn::ColumnMode::Text: {
		auto* data = static_cast<QVector<QString>*>(m_data);
		if (new_rows > 0)
			data->insert(data->end(), new_rows, QString());
		else
			data->remove(old_size - 1 + new_rows, -new_rows);
		break;
	}
	case AbstractColumn::ColumnMode::DateTime:
	case AbstractColumn::ColumnMode::Month:
	case AbstractColumn::ColumnMode::Day: {
		auto* data = static_cast<QVector<QDateTime>*>(m_data);
		if (new_rows > 0)
			data->insert(data->end(), new_rows, QDateTime());
		else
			data->remove(old_size - 1 + new_rows, -new_rows);
		break;
	}
	}

	invalidate();
}

/**
 * \brief Insert some empty (or initialized with zero) rows
 */
void ColumnPrivate::insertRows(int before, int count) {
	if (count == 0)
		return;

	m_formulas.insertRows(before, count);

	if (!m_data) {
		m_rowCount += count;
		return;
	}

	if (before <= rowCount()) {
		switch (m_columnMode) {
		case AbstractColumn::ColumnMode::Double:
			static_cast<QVector<double>*>(m_data)->insert(before, count, NAN);
			break;
		case AbstractColumn::ColumnMode::Integer:
			static_cast<QVector<int>*>(m_data)->insert(before, count, 0);
			break;
		case AbstractColumn::ColumnMode::BigInt:
			static_cast<QVector<qint64>*>(m_data)->insert(before, count, 0);
			break;
		case AbstractColumn::ColumnMode::DateTime:
		case AbstractColumn::ColumnMode::Month:
		case AbstractColumn::ColumnMode::Day:
			for (int i = 0; i < count; ++i)
				static_cast<QVector<QDateTime>*>(m_data)->insert(before, QDateTime());
			break;
		case AbstractColumn::ColumnMode::Text:
			for (int i = 0; i < count; ++i)
				static_cast<QVector<QString>*>(m_data)->insert(before, QString());
			break;
		}
	}

	invalidate();
}

/**
 * \brief Remove 'count' rows starting from row 'first'
 */
void ColumnPrivate::removeRows(int first, int count) {
	if (count == 0)
		return;

	m_formulas.removeRows(first, count);

	if (first < rowCount()) {
		int corrected_count = count;
		if (first + count > rowCount())
			corrected_count = rowCount() - first;

		if (!m_data) {
			m_rowCount -= corrected_count;
			return;
		}

		switch (m_columnMode) {
		case AbstractColumn::ColumnMode::Double:
			static_cast<QVector<double>*>(m_data)->remove(first, corrected_count);
			break;
		case AbstractColumn::ColumnMode::Integer:
			static_cast<QVector<int>*>(m_data)->remove(first, corrected_count);
			break;
		case AbstractColumn::ColumnMode::BigInt:
			static_cast<QVector<qint64>*>(m_data)->remove(first, corrected_count);
			break;
		case AbstractColumn::ColumnMode::DateTime:
		case AbstractColumn::ColumnMode::Month:
		case AbstractColumn::ColumnMode::Day:
			for (int i = 0; i < corrected_count; ++i)
				static_cast<QVector<QDateTime>*>(m_data)->removeAt(first);
			break;
		case AbstractColumn::ColumnMode::Text:
			for (int i = 0; i < corrected_count; ++i)
				static_cast<QVector<QString>*>(m_data)->removeAt(first);
			break;
		}
	}

	invalidate();
}

int ColumnPrivate::indexForValue(double x, bool smaller) const {
	return indexForValueCommon<Column>(q,
									   x,
									   std::mem_fn(&Column::columnMode),
									   std::mem_fn<int() const>(&Column::rowCount),
									   std::mem_fn(&Column::valueAt),
									   std::mem_fn(&Column::dateTimeAt),
									   std::mem_fn(&Column::properties),
									   std::mem_fn<bool(int) const>(&Column::isValid),
									   std::mem_fn<bool(int) const>(&Column::isMasked),
									   smaller);
}

/*!
 * calculates log2(x)+1 for an integer value.
 * Used in y(double x) to calculate the maximum steps
 * source: https://stackoverflow.com/questions/11376288/fast-computing-of-log2-for-64-bit-integers
 * source: https://graphics.stanford.edu/~seander/bithacks.html#IntegerLogLookup
 * @param value
 * @return returns calculated value
 */
// TODO: testing if it is faster than calculating log2.
// TODO: put into NSL when useful
int ColumnPrivate::calculateMaxSteps(unsigned int value) {
	const std::array<signed char, 256> LogTable256 = {
		-1, 0, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
		5,	5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
		6,	6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 7,
		7,	7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
		7,	7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
		7,	7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7};

	unsigned int r; // r will be lg(v)
	unsigned int t, tt; // temporaries
	if ((tt = value >> 16))
		r = (t = tt >> 8) ? 24 + LogTable256[t] : 16 + LogTable256[tt];
	else
		r = (t = value >> 8) ? 8 + LogTable256[t] : LogTable256[value];

	return r + 1;
}

/*!
 * Find index which corresponds to a @p x . In a vector of values
 * When monotonic increasing or decreasing a different algorithm will be used, which needs less steps (mean) (log_2(rowCount)) to find the value.
 * @param x
 * @return -1 if index not found, otherwise the index
 */
int ColumnPrivate::indexForValue(double x, QVector<double>& column, Column::Properties properties, bool smaller) {
	int rowCount = column.count();
	if (rowCount == 0)
		return -1;

	if (properties == AbstractColumn::Properties::MonotonicIncreasing || properties == AbstractColumn::Properties::MonotonicDecreasing) {
		// bisects the index every time, so it is possible to find the value in log_2(rowCount) steps
		bool increase = true;
		if (properties == AbstractColumn::Properties::MonotonicDecreasing)
			increase = false;

		int lowerIndex = 0;
		int higherIndex = rowCount - 1;

		unsigned int maxSteps = calculateMaxSteps(static_cast<unsigned int>(rowCount)) + 1;

		for (unsigned int i = 0; i < maxSteps; i++) { // so no log_2(rowCount) needed
			int index = lowerIndex + round(static_cast<double>(higherIndex - lowerIndex) / 2);
			double value = column.at(index);

			if (higherIndex - lowerIndex < 2)
				return finalIndex(column.at(lowerIndex), column.at(higherIndex), x, lowerIndex, higherIndex, smaller, increase);

			determineNewIndices(value, x, index, lowerIndex, higherIndex, smaller, increase);
		}
	} else if (properties == AbstractColumn::Properties::Constant) {
		return 0;
	} else { // AbstractColumn::Properties::No || AbstractColumn::Properties::NonMonotonic
		// simple way
		int index = 0;
		double prevValue = column.at(0);
		for (int row = 0; row < rowCount; row++) {
			double value = column.at(row);
			if (std::abs(value - x) <= std::abs(prevValue - x)) { // "<=" prevents also that row - 1 become < 0
				prevValue = value;
				index = row;
			}
		}
		return index;
	}
	return -1;
}

/*!
 * Find index which corresponds to a @p x . In a vector of values
 * When monotonic increasing or decreasing a different algorithm will be used, which needs less steps (mean) (log_2(rowCount)) to find the value.
 * @param x
 * @return -1 if index not found, otherwise the index
 */
int ColumnPrivate::indexForValue(const double x, const QVector<QPointF>& points, Column::Properties properties, bool smaller) {
	int rowCount = points.count();

	if (rowCount == 0)
		return -1;

	if (properties == AbstractColumn::Properties::MonotonicIncreasing || properties == AbstractColumn::Properties::MonotonicDecreasing) {
		// bisects the index every time, so it is possible to find the value in log_2(rowCount) steps
		bool increase = true;
		if (properties == AbstractColumn::Properties::MonotonicDecreasing)
			increase = false;

		int lowerIndex = 0;
		int higherIndex = rowCount - 1;

		unsigned int maxSteps = calculateMaxSteps(static_cast<unsigned int>(rowCount)) + 1;

		for (unsigned int i = 0; i < maxSteps; i++) { // so no log_2(rowCount) needed
			int index = lowerIndex + round(static_cast<double>(higherIndex - lowerIndex) / 2);
			double value = points.at(index).x();

			if (higherIndex - lowerIndex < 2)
				return finalIndex(points.at(lowerIndex).x(), points.at(higherIndex).x(), x, lowerIndex, higherIndex, smaller, increase);

			determineNewIndices(value, x, index, lowerIndex, higherIndex, smaller, increase);
		}

	} else if (properties == AbstractColumn::Properties::Constant) {
		return 0;
	} else {
		// AbstractColumn::Properties::No || AbstractColumn::Properties::NonMonotonic
		// naiv way
		double prevValue = points.at(0).x();
		int index = 0;
		for (int row = 0; row < rowCount; row++) {
			double value = points.at(row).x();
			if (std::abs(value - x) <= std::abs(prevValue - x)) { // "<=" prevents also that row - 1 become < 0
				prevValue = value;
				index = row;
			}
		}
		return index;
	}
	return -1;
}

/*!
 * Find index which corresponds to a @p x . In a vector of values
 * When monotonic increasing or decreasing a different algorithm will be used, which needs less steps (mean) (log_2(rowCount)) to find the value.
 * @param x
 * @return -1 if index not found, otherwise the index
 */
int ColumnPrivate::indexForValue(double x, QVector<QLineF>& lines, AbstractColumn::Properties properties, bool smaller) {
	int rowCount = lines.count();
	if (rowCount == 0)
		return -1;

	// use only p1 to find index
	if (properties == AbstractColumn::Properties::MonotonicIncreasing || properties == AbstractColumn::Properties::MonotonicDecreasing) {
		// bisects the index every time, so it is possible to find the value in log_2(rowCount) steps
		bool increase = true;
		if (properties == AbstractColumn::Properties::MonotonicDecreasing)
			increase = false;

		int lowerIndex = 0;
		int higherIndex = rowCount - 1;

		unsigned int maxSteps = calculateMaxSteps(static_cast<unsigned int>(rowCount)) + 1;

		for (unsigned int i = 0; i < maxSteps; i++) { // so no log_2(rowCount) needed
			int index = lowerIndex + round(static_cast<double>(higherIndex - lowerIndex) / 2);
			double value = lines.at(index).p1().x();

			if (higherIndex - lowerIndex < 2)
				return finalIndex(lines.at(lowerIndex).p1().x(), lines.at(higherIndex).p1().x(), x, lowerIndex, higherIndex, smaller, increase);

			determineNewIndices(value, x, index, lowerIndex, higherIndex, smaller, increase);
		}

	} else if (properties == AbstractColumn::Properties::Constant) {
		return 0;
	} else {
		// AbstractColumn::Properties::No || AbstractColumn::Properties::NonMonotonic
		// naiv way
		int index = 0;
		double prevValue = lines.at(0).p1().x();
		for (int row = 0; row < rowCount; row++) {
			double value = lines.at(row).p1().x();
			if (std::abs(value - x) <= std::abs(prevValue - x)) { // "<=" prevents also that row - 1 become < 0
				prevValue = value;
				index = row;
			}
		}
		return index;
	}
	return -1;
}

//! Return the column name
QString ColumnPrivate::name() const {
	return q->name();
}

/**
 * \brief Return the column plot designation
 */
AbstractColumn::PlotDesignation ColumnPrivate::plotDesignation() const {
	return m_plotDesignation;
}

/**
 * \brief Set the column plot designation
 */
void ColumnPrivate::setPlotDesignation(AbstractColumn::PlotDesignation pd) {
	Q_EMIT q->plotDesignationAboutToChange(q);
	m_plotDesignation = pd;
	Q_EMIT q->plotDesignationChanged(q);
}

/**
 * \brief Get width
 */
int ColumnPrivate::width() const {
	return m_width;
}

/**
 * \brief Set width
 */
void ColumnPrivate::setWidth(int value) {
	m_width = value;
}

/**
 * @brief ColumnPrivate::setData
 * Set new column data
 */
void ColumnPrivate::setData(void* data) {
	deleteData();
	m_data = data;
	invalidate();
}

/**
 * \brief Return the data pointer
 */
void* ColumnPrivate::data() const {
	if (!m_data)
		const_cast<ColumnPrivate*>(this)->initDataContainer();

	return m_data;
}

/**
 * \brief Return the input filter (for string -> data type conversion)
 */
AbstractSimpleFilter* ColumnPrivate::inputFilter() const {
	return m_inputFilter;
}

/**
 * \brief Return the output filter (for data type -> string  conversion)
 */
AbstractSimpleFilter* ColumnPrivate::outputFilter() const {
	return m_outputFilter;
}

//! \name Labels related functions
//@{
void ColumnPrivate::setLabelsMode(Column::ColumnMode mode) {
	m_labels.setMode(mode);
}

void ColumnPrivate::valueLabelsRemoveAll() {
	m_labels.removeAll();
}

bool ColumnPrivate::valueLabelsInitialized() const {
	return m_labels.initialized();
}

void ColumnPrivate::removeValueLabel(const QString& key) {
	m_labels.remove(key);
}

double ColumnPrivate::valueLabelsMinimum() {
	return m_labels.minimum();
}

double ColumnPrivate::valueLabelsMaximum() {
	return m_labels.maximum();
}

const QVector<Column::ValueLabel<QString>>* ColumnPrivate::textValueLabels() const {
	return m_labels.textValueLabels();
}

const QVector<Column::ValueLabel<QDateTime>>* ColumnPrivate::dateTimeValueLabels() const {
	return m_labels.dateTimeValueLabels();
}

int ColumnPrivate::valueLabelsCount() const {
	return m_labels.count();
}

int ColumnPrivate::valueLabelsCount(double min, double max) const {
	return m_labels.count(min, max);
}

int ColumnPrivate::valueLabelsIndexForValue(double value, bool smaller) const {
	return m_labels.indexForValue(value, smaller);
}

double ColumnPrivate::valueLabelsValueAt(int index) const {
	return m_labels.valueAt(index);
}

QString ColumnPrivate::valueLabelAt(int index) const {
	return m_labels.labelAt(index);
}

const QVector<Column::ValueLabel<double>>* ColumnPrivate::valueLabels() const {
	return m_labels.valueLabels();
}

const QVector<Column::ValueLabel<int>>* ColumnPrivate::intValueLabels() const {
	return m_labels.intValueLabels();
}

const QVector<Column::ValueLabel<qint64>>* ColumnPrivate::bigIntValueLabels() const {
	return m_labels.bigIntValueLabels();
}
//@}

//! \name Formula related functions
//@{
/**
 * \brief Return the formula last used to generate data for the column
 */
QString ColumnPrivate::formula() const {
	return m_formula;
}

const QVector<Column::FormulaData>& ColumnPrivate::formulaData() const {
	return m_formulaData;
}

bool ColumnPrivate::formulaAutoUpdate() const {
	return m_formulaAutoUpdate;
}

bool ColumnPrivate::formulaAutoResize() const {
	return m_formulaAutoResize;
}

/**
 * \brief Sets the formula used to generate column values
 */
void ColumnPrivate::setFormula(const QString& formula, const QVector<Column::FormulaData>& formulaData, bool autoUpdate, bool autoResize) {
	m_formula = formula;
	m_formulaData = formulaData; // TODO: disconnecting everything?
	m_formulaAutoUpdate = autoUpdate;
	m_formulaAutoResize = autoResize;

	for (auto& connection : m_connectionsUpdateFormula)
		if (static_cast<bool>(connection))
			disconnect(connection);

	for (const auto& data : m_formulaData) {
		const auto* column = data.column();
		Q_ASSERT(column);
		if (autoUpdate)
			connectFormulaColumn(column);
	}

	Q_EMIT q->formulaChanged(q);
}

/*!
 * called after the import of the project was done and all columns were loaded in \sa Project::load()
 * to establish the required slot-signal connections for the formula update
 */
void ColumnPrivate::finalizeLoad() {
	if (m_formulaAutoUpdate) {
		for (const auto& formulaData : m_formulaData) {
			const auto* column = formulaData.column();
			if (column)
				connectFormulaColumn(column);
		}
	}
}

/*!
 * \brief ColumnPrivate::connectFormulaColumn
 * This function is used to connect the columns to the needed slots for updating formulas
 * \param column
 */
void ColumnPrivate::connectFormulaColumn(const AbstractColumn* column) {
	if (!column)
		return;

	// avoid circular dependencies - the current column cannot be part of the variable columns.
	// this shouldn't actually happen because of the checks done when the formula is defined,
	// but in case we have bugs somewhere or somebody manipulated the project xml file we add
	// a sanity check to avoid recursive calls here and crash because of the stack overflow.
	if (column == q)
		return;

	DEBUG(Q_FUNC_INFO)
	m_connectionsUpdateFormula << connect(column, &AbstractColumn::dataChanged, q, &Column::updateFormula);
	connect(column->parentAspect(),
			QOverload<const AbstractAspect*>::of(&AbstractAspect::childAspectAboutToBeRemoved),
			this,
			&ColumnPrivate::formulaVariableColumnRemoved);
	connect(column, &AbstractColumn::aboutToReset, this, &ColumnPrivate::formulaVariableColumnRemoved);
	connect(column->parentAspect(), &AbstractAspect::childAspectAdded, this, &ColumnPrivate::formulaVariableColumnAdded);
}

/*!
 * helper function used in \c Column::load() to set parameters read from the xml file.
 * \param variableColumnPaths is used to restore the pointers to columns from paths
 * after the project was loaded in Project::load().
 */
void ColumnPrivate::setFormula(const QString& formula,
							   const QStringList& variableNames,
							   const QStringList& variableColumnPaths,
							   bool autoUpdate,
							   bool autoResize) {
	m_formula = formula;
	m_formulaData.clear();
	for (int i = 0; i < variableNames.count(); i++)
		m_formulaData.append(Column::FormulaData(variableNames.at(i), variableColumnPaths.at(i)));

	m_formulaAutoUpdate = autoUpdate;
	m_formulaAutoResize = autoResize;
}

void ColumnPrivate::setFormulVariableColumnsPath(int index, const QString& path) {
	if (!m_formulaData[index].setColumnPath(path))
		DEBUG(Q_FUNC_INFO << ": For some reason, there was already a column assigned");
}

void ColumnPrivate::setFormulVariableColumn(int index, Column* column) {
	if (m_formulaData.at(index).column()) // if there exists already a valid column, disconnect it first
		disconnect(m_formulaData.at(index).column(), nullptr, this, nullptr);
	m_formulaData[index].setColumn(column);
	connectFormulaColumn(column);
}

void ColumnPrivate::setFormulVariableColumn(Column* c) {
	for (auto& d : m_formulaData) {
		if (d.columnName() == c->path()) {
			d.setColumn(c);
			break;
		}
	}
}

struct PayloadColumn : public Parsing::Payload {
	PayloadColumn(const QVector<Column::FormulaData>& data, const QVector<double>& yValues)
		: Parsing::Payload(true)
		, formulaData(data)
		, y(yValues) {
	}
	const QVector<Column::FormulaData>& formulaData;
	const QVector<double>& y; // current values
};

#define COLUMN_FUNCTION(function_name, evaluation_function)                                                                                                    \
	double column##function_name(const std::string_view& variable, const std::weak_ptr<Parsing::Payload> payload) {                                            \
		const auto p = std::dynamic_pointer_cast<PayloadColumn>(payload.lock());                                                                               \
		if (!p) {                                                                                                                                              \
			Q_ASSERT(p);                                                                                                                                       \
			return NAN;                                                                                                                                        \
		}                                                                                                                                                      \
		for (const auto& formulaData : p->formulaData) {                                                                                                       \
			if (formulaData.variableName().compare(QLatin1String(variable)) == 0)                                                                              \
				return formulaData.column()->evaluation_function;                                                                                              \
		}                                                                                                                                                      \
		return NAN;                                                                                                                                            \
	}

// Constant functions, which always return the same value independent of the row index
COLUMN_FUNCTION(Size, statistics().size)
COLUMN_FUNCTION(Sum, statistics().sum)
COLUMN_FUNCTION(Min, minimum())
COLUMN_FUNCTION(Max, maximum())
COLUMN_FUNCTION(Mean, statistics().arithmeticMean)
COLUMN_FUNCTION(Median, statistics().median)
COLUMN_FUNCTION(Stdev, statistics().standardDeviation)
COLUMN_FUNCTION(Var, statistics().variance)
COLUMN_FUNCTION(Gm, statistics().geometricMean)
COLUMN_FUNCTION(Hm, statistics().harmonicMean)
COLUMN_FUNCTION(Chm, statistics().contraharmonicMean)
COLUMN_FUNCTION(StatisticsMode, statistics().mode)
COLUMN_FUNCTION(Quartile1, statistics().firstQuartile)
COLUMN_FUNCTION(Quartile3, statistics().thirdQuartile)
COLUMN_FUNCTION(Iqr, statistics().iqr)
COLUMN_FUNCTION(Percentile1, statistics().percentile_1)
COLUMN_FUNCTION(Percentile5, statistics().percentile_5)
COLUMN_FUNCTION(Percentile10, statistics().percentile_10)
COLUMN_FUNCTION(Percentile90, statistics().percentile_90)
COLUMN_FUNCTION(Percentile95, statistics().percentile_95)
COLUMN_FUNCTION(Percentile99, statistics().percentile_99)
COLUMN_FUNCTION(Trimean, statistics().trimean)
COLUMN_FUNCTION(Meandev, statistics().meanDeviation)
COLUMN_FUNCTION(Meandevmedian, statistics().meanDeviationAroundMedian)
COLUMN_FUNCTION(Mediandev, statistics().medianDeviation)
COLUMN_FUNCTION(Skew, statistics().skewness)
COLUMN_FUNCTION(Kurt, statistics().kurtosis)
COLUMN_FUNCTION(Entropy, statistics().entropy)

double cell_curr_column(double row, const std::weak_ptr<Parsing::Payload> payload) {
	const auto pd = std::dynamic_pointer_cast<PayloadColumn>(payload.lock());
	if (!pd) {
		Q_ASSERT(pd);
		return NAN;
	}
	int index = (int)row - 1;
	if (index >= 0 && pd->y.length() > index)
		return pd->y.at(index);
	return NAN;
}

double cell_curr_column_defaultvalue(double row, double defaultValue, const std::weak_ptr<Parsing::Payload> payload) {
	const auto pd = std::dynamic_pointer_cast<PayloadColumn>(payload.lock());
	if (!pd) {
		Q_ASSERT(pd);
		return NAN;
	}
	int index = (int)row - 1;
	if (index >= 0 && pd->y.length() > index)
		return pd->y.at(index);
	return defaultValue;
}

double columnQuantile(double p, const std::string_view& variable, const std::weak_ptr<Parsing::Payload> payload) {
	const auto pd = std::dynamic_pointer_cast<PayloadColumn>(payload.lock());
	if (!pd) {
		Q_ASSERT(pd);
		return NAN;
	}

	if (p < 0)
		return NAN;

	const Column* column = nullptr;
	for (const auto& formulaData : pd->formulaData) {
		if (formulaData.variableName().compare(QLatin1String(variable)) == 0) {
			column = formulaData.column();
			break;
		}
	}
	if (!column)
		return NAN;

	double value = 0.0;
	switch (column->columnMode()) { // all types
	case AbstractColumn::ColumnMode::Double: {
		auto data = reinterpret_cast<QVector<double>*>(column->data());
		value = nsl_stats_quantile(data->data(), 1, column->statistics().size, p, nsl_stats_quantile_type7);
		break;
	}
	case AbstractColumn::ColumnMode::Integer: {
		auto* intData = reinterpret_cast<QVector<int>*>(column->data());

		QVector<double> data = QVector<double>(); // copy data to double
		data.reserve(column->rowCount());
		for (auto v : *intData)
			data << static_cast<double>(v);
		value = nsl_stats_quantile(data.data(), 1, column->statistics().size, p, nsl_stats_quantile_type7);
		break;
	}
	case AbstractColumn::ColumnMode::BigInt: {
		auto* bigIntData = reinterpret_cast<QVector<qint64>*>(column->data());

		QVector<double> data = QVector<double>(); // copy data to double
		data.reserve(column->rowCount());
		for (auto v : *bigIntData)
			data << static_cast<double>(v);
		value = nsl_stats_quantile(data.data(), 1, column->statistics().size, p, nsl_stats_quantile_type7);
		break;
	}
	case AbstractColumn::ColumnMode::DateTime: // not supported yet
	case AbstractColumn::ColumnMode::Day:
	case AbstractColumn::ColumnMode::Month:
	case AbstractColumn::ColumnMode::Text:
		break;
	}
	return value;
}

double columnPercentile(double p, const std::string_view& variable, const std::weak_ptr<Parsing::Payload> payload) {
	return columnQuantile(p / 100., variable, payload);
}

/*!
 * \sa FunctionValuesDialog::generate()
 */
void ColumnPrivate::updateFormula() {
	if (m_formula.isEmpty())
		return;
	DEBUG(Q_FUNC_INFO)
	// determine variable names and the data vectors of the specified columns
	QVector<QVector<double>*> xVectors;

	bool valid = true;
	QStringList formulaVariableNames;
	int maxRowCount = 0;

	auto numberLocale = QLocale();
	// need to disable group separator since parser can't handle it
	numberLocale.setNumberOptions(QLocale::OmitGroupSeparator);

	for (const auto& formulaData : m_formulaData) {
		auto* column = formulaData.column();
		if (!column) {
			valid = false;
			break;
		}
		formulaVariableNames << formulaData.variableName();

		if (column->columnMode() == AbstractColumn::ColumnMode::Double)
			xVectors << static_cast<QVector<double>*>(column->data());
		else {
			// convert integers to doubles first
			auto* xVector = new QVector<double>(column->rowCount());
			for (int i = 0; i < column->rowCount(); ++i)
				(*xVector)[i] = column->valueAt(i);

			xVectors << xVector;
		}

		if (column->rowCount() > maxRowCount)
			maxRowCount = column->rowCount();
	}

	if (valid) {
		// resize the spreadsheet if one of the data vectors from
		// other spreadsheet(s) has more elements than the parent spreadsheet
		// and if the option "auto resize" is activated
		if (m_formulaAutoResize && rowCount() < maxRowCount) {
			auto* spreadsheet = static_cast<Spreadsheet*>(q->parentAspect());
			// In the tests spreadsheet might not exist, because directly the column was created
			if (spreadsheet)
				spreadsheet->setRowCount(maxRowCount);
		}

		// create new vector for storing the calculated values
		// the vectors with the variable data can be smaller then the result vector. So, not all values in the result vector might get initialized.
		//->"clean" the result vector first
		QVector<double> new_data(rowCount(), NAN);

		const auto payload = std::make_shared<PayloadColumn>(m_formulaData, new_data);

		// evaluate the expression for f(x_1, x_2, ...) and write the calculated values into a new vector.
		auto* parser = ExpressionParser::getInstance();
		parser->setSpecialFunctionVariablePayload(Parsing::colfun_size, columnSize, payload);
		parser->setSpecialFunctionVariablePayload(Parsing::colfun_sum, columnSum, payload);
		parser->setSpecialFunctionVariablePayload(Parsing::colfun_min, columnMin, payload);
		parser->setSpecialFunctionVariablePayload(Parsing::colfun_max, columnMax, payload);
		parser->setSpecialFunctionVariablePayload(Parsing::colfun_mean, columnMean, payload);
		parser->setSpecialFunctionVariablePayload(Parsing::colfun_median, columnMedian, payload);
		parser->setSpecialFunctionVariablePayload(Parsing::colfun_stdev, columnStdev, payload);
		parser->setSpecialFunctionVariablePayload(Parsing::colfun_var, columnVar, payload);
		parser->setSpecialFunctionVariablePayload(Parsing::colfun_gm, columnGm, payload);
		parser->setSpecialFunctionVariablePayload(Parsing::colfun_hm, columnHm, payload);
		parser->setSpecialFunctionVariablePayload(Parsing::colfun_chm, columnChm, payload);
		parser->setSpecialFunctionVariablePayload(Parsing::colfun_mode, columnStatisticsMode, payload);
		parser->setSpecialFunctionVariablePayload(Parsing::colfun_quartile1, columnQuartile1, payload);
		parser->setSpecialFunctionVariablePayload(Parsing::colfun_quartile3, columnQuartile3, payload);
		parser->setSpecialFunctionVariablePayload(Parsing::colfun_iqr, columnIqr, payload);
		parser->setSpecialFunctionVariablePayload(Parsing::colfun_percentile1, columnPercentile1, payload);
		parser->setSpecialFunctionVariablePayload(Parsing::colfun_percentile5, columnPercentile5, payload);
		parser->setSpecialFunctionVariablePayload(Parsing::colfun_percentile10, columnPercentile10, payload);
		parser->setSpecialFunctionVariablePayload(Parsing::colfun_percentile90, columnPercentile90, payload);
		parser->setSpecialFunctionVariablePayload(Parsing::colfun_percentile95, columnPercentile95, payload);
		parser->setSpecialFunctionVariablePayload(Parsing::colfun_percentile99, columnPercentile99, payload);
		parser->setSpecialFunctionVariablePayload(Parsing::colfun_trimean, columnTrimean, payload);
		parser->setSpecialFunctionVariablePayload(Parsing::colfun_meandev, columnMeandev, payload);
		parser->setSpecialFunctionVariablePayload(Parsing::colfun_meandevmedian, columnMeandevmedian, payload);
		parser->setSpecialFunctionVariablePayload(Parsing::colfun_mediandev, columnMediandev, payload);
		parser->setSpecialFunctionVariablePayload(Parsing::colfun_skew, columnSkew, payload);
		parser->setSpecialFunctionVariablePayload(Parsing::colfun_kurt, columnKurt, payload);
		parser->setSpecialFunctionVariablePayload(Parsing::colfun_entropy, columnEntropy, payload);
		parser->setSpecialFunctionValueVariablePayload(Parsing::colfun_percentile, columnPercentile, payload);
		parser->setSpecialFunctionValueVariablePayload(Parsing::colfun_quantile, columnQuantile, payload);
		parser->setSpecialFunctionValuePayload(Parsing::cell_curr_column, cell_curr_column, payload);
		parser->setSpecialFunction2ValuePayload(Parsing::cell_curr_column_default, cell_curr_column_defaultvalue, payload);

		QDEBUG(Q_FUNC_INFO << ", Calling evaluateCartesian(). formula: " << m_formula << ", var names: " << formulaVariableNames)
		bool validEval = parser->tryEvaluateCartesian(m_formula, formulaVariableNames, xVectors, &new_data);
		if (!validEval)
			DEBUG(Q_FUNC_INFO << ", Failed parsing formula!")
		DEBUG(Q_FUNC_INFO << ", Calling replaceValues()")
		replaceValues(-1, new_data);

		// initialize remaining rows with NAN
		// This will be done already in evaluateCartesian()
		// int remainingRows = rowCount() - maxRowCount;
		// if (remainingRows > 0) {
		//	QVector<double> emptyRows(remainingRows, NAN);
		//	replaceValues(maxRowCount, emptyRows);
		//}
	} else { // not valid
		QVector<double> new_data(rowCount(), NAN);
		replaceValues(-1, new_data);
	}

	DEBUG(Q_FUNC_INFO << " DONE")
	Q_EMIT q->formulaChanged(q);
}

void ColumnPrivate::formulaVariableColumnRemoved(const AbstractAspect* aspect) {
	const Column* column = dynamic_cast<const Column*>(aspect);
	if (!column)
		return;
	disconnect(column, nullptr, this, nullptr);
	int index = -1;
	for (int i = 0; i < formulaData().count(); i++) {
		auto& d = formulaData().at(i);
		if (d.column() == column) {
			index = i;
			break;
		}
	}
	if (index != -1) {
		m_formulaData[index].setColumn(nullptr);
		DEBUG(Q_FUNC_INFO << ", calling updateFormula()")
		updateFormula();
	}
}

void ColumnPrivate::formulaVariableColumnAdded(const AbstractAspect* aspect) {
	PERFTRACE(QLatin1String(Q_FUNC_INFO));
	auto* column = dynamic_cast<Column*>(const_cast<AbstractAspect*>(aspect));
	if (!column)
		return;

	const auto& path = aspect->path();
	for (int i = 0; i < formulaData().count(); i++) {
		if (formulaData().at(i).columnName() == path) {
			// m_formulaData[index].setColumn(const_cast<Column*>(column));
			// DEBUG(Q_FUNC_INFO << ", calling updateFormula()")
			setFormulVariableColumn(i, column);
			updateFormula();
			return;
		}
	}
}

/**
 * \brief Return the formula associated with row 'row'
 */
QString ColumnPrivate::formula(int row) const {
	return m_formulas.value(row);
}

/**
 * \brief Return the intervals that have associated formulas
 *
 * This can be used to make a list of formulas with their intervals.
 * Here is some example code:
 *
 * \code
 * QStringList list;
 * QVector< Interval<int> > intervals = my_column.formulaIntervals();
 * foreach(Interval<int> interval, intervals)
 * 	list << QString(interval.toString() + ": " + my_column.formula(interval.start()));
 * \endcode
 */
QVector<Interval<int>> ColumnPrivate::formulaIntervals() const {
	return m_formulas.intervals();
}

/**
 * \brief Set a formula string for an interval of rows
 */
void ColumnPrivate::setFormula(const Interval<int>& i, const QString& formula) {
	m_formulas.setValue(i, formula);
}

/**
 * \brief Overloaded function for convenience
 */
void ColumnPrivate::setFormula(int row, const QString& formula) {
	setFormula(Interval<int>(row, row), formula);
}

/**
 * \brief Clear all formulas
 */
void ColumnPrivate::clearFormulas() {
	m_formulas.clear();
}
//@}

////////////////////////////////////////////////////////////////////////////////
//! \name type specific functions
//@{
////////////////////////////////////////////////////////////////////////////////
void ColumnPrivate::setValueAt(int row, int new_value) {
	setIntegerAt(row, new_value);
}

void ColumnPrivate::setValueAt(int row, qint64 new_value) {
	setBigIntAt(row, new_value);
}

void ColumnPrivate::setValueAt(int row, QDateTime new_value) {
	setDateTimeAt(row, new_value);
}

void ColumnPrivate::setValueAt(int row, QString new_value) {
	setTextAt(row, new_value);
}

void ColumnPrivate::replaceValues(int first, const QVector<int>& new_values) {
	replaceInteger(first, new_values);
}

void ColumnPrivate::replaceValues(int first, const QVector<qint64>& new_values) {
	replaceBigInt(first, new_values);
}

void ColumnPrivate::replaceValues(int first, const QVector<QDateTime>& new_values) {
	replaceDateTimes(first, new_values);
}

void ColumnPrivate::replaceValues(int first, const QVector<QString>& new_values) {
	replaceTexts(first, new_values);
}

/**
 * \brief Return the content of row 'row'.
 *
 * Use this only when columnMode() is Text
 */
QString ColumnPrivate::textAt(int row) const {
	if (!m_data || m_columnMode != AbstractColumn::ColumnMode::Text)
		return {};
	return static_cast<QVector<QString>*>(m_data)->value(row);
}

/**
 * \brief Return the date part of row 'row'
 *
 * Use this only when columnMode() is DateTime, Month or Day
 */
QDate ColumnPrivate::dateAt(int row) const {
	if (!m_data
		|| (m_columnMode != AbstractColumn::ColumnMode::DateTime && m_columnMode != AbstractColumn::ColumnMode::Month
			&& m_columnMode != AbstractColumn::ColumnMode::Day))
		return QDate{};
	return dateTimeAt(row).date();
}

/**
 * \brief Return the time part of row 'row'
 *
 * Use this only when columnMode() is DateTime, Month or Day
 */
QTime ColumnPrivate::timeAt(int row) const {
	if (!m_data
		|| (m_columnMode != AbstractColumn::ColumnMode::DateTime && m_columnMode != AbstractColumn::ColumnMode::Month
			&& m_columnMode != AbstractColumn::ColumnMode::Day))
		return QTime{};
	return dateTimeAt(row).time();
}

/**
 * \brief Return the QDateTime in row 'row'
 *
 * Use this only when columnMode() is DateTime, Month or Day
 */
QDateTime ColumnPrivate::dateTimeAt(int row) const {
	if (!m_data
		|| (m_columnMode != AbstractColumn::ColumnMode::DateTime && m_columnMode != AbstractColumn::ColumnMode::Month
			&& m_columnMode != AbstractColumn::ColumnMode::Day))
		return QDateTime();
	return static_cast<QVector<QDateTime>*>(m_data)->value(row);
}

double ColumnPrivate::doubleAt(int index) const {
	if (!m_data)
		return NAN;

	return static_cast<QVector<double>*>(m_data)->value(index, NAN);
}

/**
 * \brief Return the double value at index 'index' for columns with type Numeric, Integer or BigInt.
 * This function has to be used everywhere where the exact type (double, int or qint64) is not relevant for numerical calculations.
 * For cases where the integer value is needed without any implicit conversions, \sa integerAt() has to be used.
 */
double ColumnPrivate::valueAt(int index) const {
	if (!m_data)
		return NAN;

	switch (m_columnMode) {
	case AbstractColumn::ColumnMode::Double:
		return static_cast<QVector<double>*>(m_data)->value(index, NAN);
	case AbstractColumn::ColumnMode::Integer:
		return static_cast<QVector<int>*>(m_data)->value(index, 0);
	case AbstractColumn::ColumnMode::BigInt:
		return static_cast<QVector<qint64>*>(m_data)->value(index, 0);
	case AbstractColumn::ColumnMode::DateTime:
		return static_cast<QVector<QDateTime>*>(m_data)->value(index).toMSecsSinceEpoch();
	case AbstractColumn::ColumnMode::Month: // Fall through
	case AbstractColumn::ColumnMode::Day: // Fall through
	case AbstractColumn::ColumnMode::Text: // Fall through
		break;
	}
	return NAN;
}

/**
 * \brief Return the int value in row 'row'
 */
int ColumnPrivate::integerAt(int row) const {
	if (!m_data || m_columnMode != AbstractColumn::ColumnMode::Integer)
		return 0;
	return static_cast<QVector<int>*>(m_data)->value(row, 0);
}

/**
 * \brief Return the bigint value in row 'row'
 */
qint64 ColumnPrivate::bigIntAt(int row) const {
	if (!m_data || m_columnMode != AbstractColumn::ColumnMode::BigInt)
		return 0;
	return static_cast<QVector<qint64>*>(m_data)->value(row, 0);
}

void ColumnPrivate::invalidate() {
	available.setUnavailable();
}

/**
 * \brief Set the content of row 'row'
 *
 * Use this only when columnMode() is Text
 */
void ColumnPrivate::setTextAt(int row, const QString& new_value) {
	if (m_columnMode != AbstractColumn::ColumnMode::Text)
		return;

	setValueAtPrivate<QString>(row, new_value);
}

/**
 * \brief Replace a range of values
 *
 * Use this only when columnMode() is Text
 */
void ColumnPrivate::replaceTexts(int first, const QVector<QString>& new_values) {
	if (m_columnMode != AbstractColumn::ColumnMode::Text)
		return;

	replaceValuePrivate<QString>(first, new_values);
}

int ColumnPrivate::dictionaryIndex(int row) const {
	if (!available.dictionary)
		const_cast<ColumnPrivate*>(this)->initDictionary();

	const auto& value = textAt(row);
	int index = 0;
	auto it = m_dictionary.constBegin();
	while (it != m_dictionary.constEnd()) {
		if (*it == value)
			break;
		++index;
		++it;
	}

	return index;
}

/*!
 * \brief Return the dictionary of the column.
 *
 * The dictionary is a list of all different values in the column (string representation)
 * together with their frequencies of occurrence.
 */
const QMap<QString, int>& ColumnPrivate::frequencies() const {
	if (!available.dictionary)
		const_cast<ColumnPrivate*>(this)->initDictionary();

	return m_dictionaryFrequencies;
}

void ColumnPrivate::initDictionary() {
	m_dictionary.clear();
	m_dictionaryFrequencies.clear();
	if (!m_data)
		return;

	QString value;
	const auto locale = QLocale();

	switch (columnMode()) {
	case AbstractColumn::ColumnMode::Text: {
		auto data = static_cast<QVector<QString>*>(m_data);
		for (auto& val : *data) {
			if (val.isEmpty())
				continue;
			value = val;
			if (!m_dictionary.contains(value))
				m_dictionary << value;
			if (m_dictionaryFrequencies.constFind(value) == m_dictionaryFrequencies.constEnd())
				m_dictionaryFrequencies[value] = 1;
			else
				m_dictionaryFrequencies[value]++;
		}
		break;
	}
	case AbstractColumn::ColumnMode::Double: {
		auto data = static_cast<QVector<double>*>(m_data);
		for (auto val : *data) {
			if (std::isnan(val))
				continue;
			value = locale.toString(val);
			if (!m_dictionary.contains(value))
				m_dictionary << value;
			if (m_dictionaryFrequencies.constFind(value) == m_dictionaryFrequencies.constEnd())
				m_dictionaryFrequencies[value] = 1;
			else
				m_dictionaryFrequencies[value]++;
		}
		break;
	}
	case AbstractColumn::ColumnMode::Integer: {
		auto data = static_cast<QVector<int>*>(m_data);
		for (auto val : *data) {
			value = locale.toString(val);
			if (!m_dictionary.contains(value))
				m_dictionary << value;
			if (m_dictionaryFrequencies.constFind(value) == m_dictionaryFrequencies.constEnd())
				m_dictionaryFrequencies[value] = 1;
			else
				m_dictionaryFrequencies[value]++;
		}
		break;
	}
	case AbstractColumn::ColumnMode::BigInt: {
		auto data = static_cast<QVector<qint64>*>(m_data);
		for (auto val : *data) {
			value = locale.toString(val);
			if (!m_dictionary.contains(value))
				m_dictionary << value;
			if (m_dictionaryFrequencies.constFind(value) == m_dictionaryFrequencies.constEnd())
				m_dictionaryFrequencies[value] = 1;
			else
				m_dictionaryFrequencies[value]++;
		}
		break;
	}
	case AbstractColumn::ColumnMode::DateTime: {
		auto data = static_cast<QVector<QDateTime>*>(m_data);
		for (const auto& val : *data) {
			if (!val.isValid())
				continue;
			value = val.toString(Qt::ISODate);
			if (!m_dictionary.contains(value))
				m_dictionary << value;
			if (m_dictionaryFrequencies.constFind(value) == m_dictionaryFrequencies.constEnd())
				m_dictionaryFrequencies[value] = 1;
			else
				m_dictionaryFrequencies[value]++;
		}
		break;
	}
	case AbstractColumn::ColumnMode::Month: {
		auto data = static_cast<QVector<QDateTime>*>(m_data);
		for (const auto& val : *data) {
			if (!val.isValid())
				continue;
			value = val.toString(QStringLiteral("MMMM")); // Full month name
			if (!m_dictionary.contains(value))
				m_dictionary << value;
			if (m_dictionaryFrequencies.constFind(value) == m_dictionaryFrequencies.constEnd())
				m_dictionaryFrequencies[value] = 1;
			else
				m_dictionaryFrequencies[value]++;
		}
		break;
	}
	case AbstractColumn::ColumnMode::Day: {
		auto data = static_cast<QVector<QDateTime>*>(m_data);
		for (const auto& val : *data) {
			if (!val.isValid())
				continue;
			value = val.toString(QStringLiteral("dddd")); // Full day name
			if (!m_dictionary.contains(value))
				m_dictionary << value;
			if (m_dictionaryFrequencies.constFind(value) == m_dictionaryFrequencies.constEnd())
				m_dictionaryFrequencies[value] = 1;
			else
				m_dictionaryFrequencies[value]++;
		}
		break;
	}
	}

	available.dictionary = true;
}

/**
 * \brief Set the content of row 'row'
 *
 * Use this only when columnMode() is DateTime, Month or Day
 */
void ColumnPrivate::setDateAt(int row, QDate new_value) {
	if (m_columnMode != AbstractColumn::ColumnMode::DateTime && m_columnMode != AbstractColumn::ColumnMode::Month
		&& m_columnMode != AbstractColumn::ColumnMode::Day)
		return;

	if (!m_data)
		initDataContainer();

	if (!m_data) // failed to allocate memory
		return;

	setDateTimeAt(row, QDateTime(new_value, timeAt(row)));
}

/**
 * \brief Set the content of row 'row'
 *
 * Use this only when columnMode() is DateTime, Month or Day
 */
void ColumnPrivate::setTimeAt(int row, QTime new_value) {
	if (m_columnMode != AbstractColumn::ColumnMode::DateTime && m_columnMode != AbstractColumn::ColumnMode::Month
		&& m_columnMode != AbstractColumn::ColumnMode::Day)
		return;

	if (!m_data)
		initDataContainer();

	if (!m_data) // failed to allocate memory
		return;

	setDateTimeAt(row, QDateTime(dateAt(row), new_value));
}

/**
 * \brief Set the content of row 'row'
 *
 * Use this only when columnMode() is DateTime, Month or Day
 */
void ColumnPrivate::setDateTimeAt(int row, const QDateTime& new_value) {
	if (m_columnMode != AbstractColumn::ColumnMode::DateTime && m_columnMode != AbstractColumn::ColumnMode::Month
		&& m_columnMode != AbstractColumn::ColumnMode::Day)
		return;

	setValueAtPrivate<QDateTime>(row, new_value);
}

/**
 * \brief Replace a range of values
 * \param first first index which should be replaced. If first < 0, the complete vector
 * will be replaced
 * \param new_values
 * Use this only when columnMode() is DateTime, Month or Day
 */
void ColumnPrivate::replaceDateTimes(int first, const QVector<QDateTime>& new_values) {
	if (m_columnMode != AbstractColumn::ColumnMode::DateTime && m_columnMode != AbstractColumn::ColumnMode::Month
		&& m_columnMode != AbstractColumn::ColumnMode::Day)
		return;

	replaceValuePrivate<QDateTime>(first, new_values);
}

/**
 * \brief Set the content of row 'row'
 *
 * Use this only when columnMode() is Numeric
 */
void ColumnPrivate::setValueAt(int row, double new_value) {
	// DEBUG(Q_FUNC_INFO);
	if (m_columnMode != AbstractColumn::ColumnMode::Double)
		return;

	setValueAtPrivate<double>(row, new_value);
}

/**
 * \brief Replace a range of values
 *
 * Use this only when columnMode() is Numeric
 */
void ColumnPrivate::replaceValues(int first, const QVector<double>& new_values) {
	// DEBUG(Q_FUNC_INFO);
	if (m_columnMode != AbstractColumn::ColumnMode::Double)
		return;

	if (!m_data) {
		const bool resize = (first >= 0);
		if (!initDataContainer(resize))
			return; // failed to allocate memory
	}

	Q_EMIT q->dataAboutToChange(q);

	if (first < 0)
		*static_cast<QVector<double>*>(m_data) = new_values;
	else {
		const int num_rows = new_values.size();
		resizeTo(first + num_rows);

		double* ptr = static_cast<QVector<double>*>(m_data)->data();
		for (int i = 0; i < num_rows; ++i)
			ptr[first + i] = new_values.at(i);
	}

	q->setDataChanged();
}

void ColumnPrivate::addValueLabel(const QString& value, const QString& label) {
	m_labels.add(value, label);
}

void ColumnPrivate::addValueLabel(const QDateTime& value, const QString& label) {
	m_labels.add(value, label);
}

void ColumnPrivate::addValueLabel(double value, const QString& label) {
	m_labels.add(value, label);
}

void ColumnPrivate::addValueLabel(int value, const QString& label) {
	m_labels.add(value, label);
}

void ColumnPrivate::addValueLabel(qint64 value, const QString& label) {
	m_labels.add(value, label);
}

/**
 * \brief Set the content of row 'row'
 *
 * Use this only when columnMode() is Integer
 */
void ColumnPrivate::setIntegerAt(int row, int new_value) {
	// DEBUG(Q_FUNC_INFO);
	if (m_columnMode != AbstractColumn::ColumnMode::Integer)
		return;

	setValueAtPrivate<int>(row, new_value);
}

/**
 * \brief Replace a range of values
 *
 * Use this only when columnMode() is Integer
 */
void ColumnPrivate::replaceInteger(int first, const QVector<int>& new_values) {
	// DEBUG(Q_FUNC_INFO);
	if (m_columnMode != AbstractColumn::ColumnMode::Integer)
		return;

	replaceValuePrivate<int>(first, new_values);
}

/**
 * \brief Set the content of row 'row'
 *
 * Use this only when columnMode() is BigInt
 */
void ColumnPrivate::setBigIntAt(int row, qint64 new_value) {
	// DEBUG(Q_FUNC_INFO);
	if (m_columnMode != AbstractColumn::ColumnMode::BigInt)
		return;

	setValueAtPrivate<qint64>(row, new_value);
}

/**
 * \brief Replace a range of values
 *
 * Use this only when columnMode() is BigInt
 */
void ColumnPrivate::replaceBigInt(int first, const QVector<qint64>& new_values) {
	// DEBUG(Q_FUNC_INFO);
	if (m_columnMode != AbstractColumn::ColumnMode::BigInt)
		return;

	replaceValuePrivate<qint64>(first, new_values);
}

/*!
 * Updates the properties. Will be called, when data in the column changed.
 * The properties will be used to speed up some algorithms.
 * See where variable properties will be used.
 */
void ColumnPrivate::updateProperties() {
	PERFTRACE(name() + QLatin1String(Q_FUNC_INFO));

	// TODO: for double Properties::Constant will never be used. Use an epsilon (difference smaller than epsilon is zero)
	const int rows = rowCount();
	if (rows == 0 || m_columnMode == AbstractColumn::ColumnMode::Text) {
		properties = AbstractColumn::Properties::No;
		available.properties = true;
		return;
	}

	double prevValue = NAN;
	int prevValueInt = 0;
	qint64 prevValueBigInt = 0;
	qint64 prevValueDatetime = 0;

	if (m_columnMode == AbstractColumn::ColumnMode::Integer)
		prevValueInt = integerAt(0);
	else if (m_columnMode == AbstractColumn::ColumnMode::BigInt)
		prevValueBigInt = bigIntAt(0);
	else if (m_columnMode == AbstractColumn::ColumnMode::Double)
		prevValue = doubleAt(0);
	else if (m_columnMode == AbstractColumn::ColumnMode::DateTime || m_columnMode == AbstractColumn::ColumnMode::Month
			 || m_columnMode == AbstractColumn::ColumnMode::Day)
		prevValueDatetime = dateTimeAt(0).toMSecsSinceEpoch();
	else {
		properties = AbstractColumn::Properties::No;
		available.properties = true;
		return;
	}

	int monotonic_decreasing = -1;
	int monotonic_increasing = -1;

	double value;
	int valueInt;
	qint64 valueBigInt;
	qint64 valueDateTime;
	for (int row = 1; row < rows; row++) {
		if (!q->isValid(row) || q->isMasked(row)) {
			// if there is one invalid or masked value, the property is No, because
			// otherwise it's difficult to find the correct index in indexForValue().
			// You don't know if you should increase the index or decrease it when
			// you hit an invalid value
			properties = AbstractColumn::Properties::No;
			available.properties = true;
			return;
		}

		switch (m_columnMode) {
		case AbstractColumn::ColumnMode::Integer: {
			valueInt = integerAt(row);

			if (valueInt > prevValueInt) {
				monotonic_decreasing = 0;
				if (monotonic_increasing < 0)
					monotonic_increasing = 1;
				else if (monotonic_increasing == 0)
					break; // when nor increasing, nor decreasing, break

			} else if (valueInt < prevValueInt) {
				monotonic_increasing = 0;
				if (monotonic_decreasing < 0)
					monotonic_decreasing = 1;
				else if (monotonic_decreasing == 0)
					break; // when nor increasing, nor decreasing, break

			} else {
				if (monotonic_increasing < 0 && monotonic_decreasing < 0) {
					monotonic_decreasing = 1;
					monotonic_increasing = 1;
				}
			}

			prevValueInt = valueInt;
			break;
		}
		case AbstractColumn::ColumnMode::BigInt: {
			valueBigInt = bigIntAt(row);

			if (valueBigInt > prevValueBigInt) {
				monotonic_decreasing = 0;
				if (monotonic_increasing < 0)
					monotonic_increasing = 1;
				else if (monotonic_increasing == 0)
					break; // when nor increasing, nor decreasing, break

			} else if (valueBigInt < prevValueBigInt) {
				monotonic_increasing = 0;
				if (monotonic_decreasing < 0)
					monotonic_decreasing = 1;
				else if (monotonic_decreasing == 0)
					break; // when nor increasing, nor decreasing, break

			} else {
				if (monotonic_increasing < 0 && monotonic_decreasing < 0) {
					monotonic_decreasing = 1;
					monotonic_increasing = 1;
				}
			}

			prevValueBigInt = valueBigInt;
			break;
		}
		case AbstractColumn::ColumnMode::Double: {
			value = doubleAt(row);

			if (std::isnan(value)) {
				monotonic_increasing = 0;
				monotonic_decreasing = 0;
				break;
			}

			if (value > prevValue) {
				monotonic_decreasing = 0;
				if (monotonic_increasing < 0)
					monotonic_increasing = 1;
				else if (monotonic_increasing == 0)
					break; // when nor increasing, nor decreasing, break

			} else if (value < prevValue) {
				monotonic_increasing = 0;
				if (monotonic_decreasing < 0)
					monotonic_decreasing = 1;
				else if (monotonic_decreasing == 0)
					break; // when nor increasing, nor decreasing, break

			} else {
				if (monotonic_increasing < 0 && monotonic_decreasing < 0) {
					monotonic_decreasing = 1;
					monotonic_increasing = 1;
				}
			}

			prevValue = value;
			break;
		}
		case AbstractColumn::ColumnMode::DateTime:
		case AbstractColumn::ColumnMode::Month:
		case AbstractColumn::ColumnMode::Day: {
			valueDateTime = dateTimeAt(row).toMSecsSinceEpoch();

			if (valueDateTime > prevValueDatetime) {
				monotonic_decreasing = 0;
				if (monotonic_increasing < 0)
					monotonic_increasing = 1;
				else if (monotonic_increasing == 0)
					break; // when nor increasing, nor decreasing, break

			} else if (valueDateTime < prevValueDatetime) {
				monotonic_increasing = 0;
				if (monotonic_decreasing < 0)
					monotonic_decreasing = 1;
				else if (monotonic_decreasing == 0)
					break; // when nor increasing, nor decreasing, break

			} else {
				if (monotonic_increasing < 0 && monotonic_decreasing < 0) {
					monotonic_decreasing = 1;
					monotonic_increasing = 1;
				}
			}

			prevValueDatetime = valueDateTime;
			break;
		}
		case AbstractColumn::ColumnMode::Text:
			break;
		}
	}

	properties = AbstractColumn::Properties::NonMonotonic;
	if (monotonic_increasing > 0 && monotonic_decreasing > 0) {
		properties = AbstractColumn::Properties::Constant;
		DEBUG("	setting column CONSTANT")
	} else if (monotonic_decreasing > 0) {
		properties = AbstractColumn::Properties::MonotonicDecreasing;
		DEBUG("	setting column MONOTONIC DECREASING")
	} else if (monotonic_increasing > 0) {
		properties = AbstractColumn::Properties::MonotonicIncreasing;
		DEBUG("	setting column MONOTONIC INCREASING")
	}

	available.properties = true;
}

////////////////////////////////////////////////////////////////////////////////
//@}
////////////////////////////////////////////////////////////////////////////////

/**
 * \brief Return the interval attribute representing the formula strings
 */
IntervalAttribute<QString> ColumnPrivate::formulaAttribute() const {
	return m_formulas;
}

/**
 * \brief Replace the interval attribute for the formula strings
 */
void ColumnPrivate::replaceFormulas(const IntervalAttribute<QString>& formulas) {
	m_formulas = formulas;
}

void ColumnPrivate::calculateStatistics() {
	PERFTRACE(QStringLiteral("calculate column statistics"));
	statistics = AbstractColumn::ColumnStatistics();

	if (q->columnMode() == AbstractColumn::ColumnMode::Text) {
		calculateTextStatistics();
		return;
	}

	if (!q->isNumeric()) {
		calculateDateTimeStatistics();
		return;
	}

	// ######  location measures  #######
	int rowValuesSize = rowCount();
	double columnSum = 0.0;
	double columnProduct = 1.0;
	double columnSumNeg = 0.0;
	double columnSumSquare = 0.0;
	statistics.minimum = INFINITY;
	statistics.maximum = -INFINITY;
	std::unordered_map<double, int> frequencyOfValues;
	QVector<double> rowData;
	rowData.reserve(rowValuesSize);

	for (int row = 0; row < rowValuesSize; ++row) {
		double val = valueAt(row);
		if (!std::isfinite(val) || q->isMasked(row))
			continue;

		if (val < statistics.minimum)
			statistics.minimum = val;
		if (val > statistics.maximum)
			statistics.maximum = val;
		columnSum += val;
		columnSumNeg += (1.0 / val); // will be Inf when val == 0
		columnSumSquare += val * val;
		columnProduct *= val;
		if (frequencyOfValues.find(val) != frequencyOfValues.end())
			frequencyOfValues.operator[](val)++;
		else
			frequencyOfValues.insert(std::make_pair(val, 1));
		rowData.push_back(val);
	}

	const int notNanCount = rowData.size();

	if (notNanCount == 0) {
		available.statistics = true;
		available.min = true;
		available.max = true;
		return;
	}

	if (rowData.size() < rowValuesSize)
		rowData.squeeze();

	statistics.size = notNanCount;
	statistics.sum = columnSum;
	statistics.arithmeticMean = columnSum / notNanCount;

	// geometric mean
	if (statistics.minimum <= -100.) // invalid
		statistics.geometricMean = NAN;
	else if (statistics.minimum < 0) { // interpret as percentage (/100) and add 1
		columnProduct = 1.; // recalculate
		for (auto val : rowData)
			columnProduct *= val / 100. + 1.;
		// n-th root and convert back to percentage changes
		statistics.geometricMean = 100. * (std::pow(columnProduct, 1.0 / notNanCount) - 1.);
	} else if (statistics.minimum == 0) { // replace zero values with 1
		columnProduct = 1.; // recalculate
		for (auto val : rowData)
			columnProduct *= (val == 0.) ? 1. : val;
		statistics.geometricMean = std::pow(columnProduct, 1.0 / notNanCount);
	} else
		statistics.geometricMean = std::pow(columnProduct, 1.0 / notNanCount);

	if (columnSumNeg != 0.)
		statistics.harmonicMean = notNanCount / columnSumNeg;
	if (columnSum != 0.)
		statistics.contraharmonicMean = columnSumSquare / columnSum;

	// calculate the mode, the most frequent value in the data set
	int maxFreq = 0;
	double mode = NAN;
	for (const auto& it : frequencyOfValues) {
		if (it.second > maxFreq) {
			maxFreq = it.second;
			mode = it.first;
		}
	}
	// check how many times the max frequency occurs in the data set.
	// if more than once, we have a multi-modal distribution and don't show any mode
	int maxFreqOccurance = 0;
	for (const auto& it : frequencyOfValues) {
		if (it.second == maxFreq)
			++maxFreqOccurance;

		if (maxFreqOccurance > 1) {
			mode = NAN;
			break;
		}
	}
	statistics.mode = mode;

	// sort the data to calculate the percentiles
	std::sort(rowData.begin(), rowData.end());
	statistics.firstQuartile = gsl_stats_quantile_from_sorted_data(rowData.constData(), 1, notNanCount, 0.25);
	statistics.median = gsl_stats_quantile_from_sorted_data(rowData.constData(), 1, notNanCount, 0.50);
	statistics.thirdQuartile = gsl_stats_quantile_from_sorted_data(rowData.constData(), 1, notNanCount, 0.75);
	statistics.percentile_1 = gsl_stats_quantile_from_sorted_data(rowData.constData(), 1, notNanCount, 0.01);
	statistics.percentile_5 = gsl_stats_quantile_from_sorted_data(rowData.constData(), 1, notNanCount, 0.05);
	statistics.percentile_10 = gsl_stats_quantile_from_sorted_data(rowData.constData(), 1, notNanCount, 0.1);
	statistics.percentile_90 = gsl_stats_quantile_from_sorted_data(rowData.constData(), 1, notNanCount, 0.9);
	statistics.percentile_95 = gsl_stats_quantile_from_sorted_data(rowData.constData(), 1, notNanCount, 0.95);
	statistics.percentile_99 = gsl_stats_quantile_from_sorted_data(rowData.constData(), 1, notNanCount, 0.99);
	statistics.iqr = statistics.thirdQuartile - statistics.firstQuartile;
	statistics.trimean = (statistics.firstQuartile + 2. * statistics.median + statistics.thirdQuartile) / 4.;

	// ######  dispersion and shape measures  #######
	statistics.variance = 0.;
	statistics.meanDeviation = 0.;
	statistics.meanDeviationAroundMedian = 0.;
	statistics.averageTwoPeriodMovingRange = 0.;
	double centralMoment_r3 = 0.;
	double centralMoment_r4 = 0.;
	QVector<double> absoluteMedianList;
	absoluteMedianList.reserve(notNanCount);
	absoluteMedianList.resize(notNanCount);

	for (int row = 0; row < notNanCount; ++row) {
		double val = rowData.value(row);
		statistics.variance += gsl_pow_2(val - statistics.arithmeticMean);
		statistics.meanDeviation += std::abs(val - statistics.arithmeticMean);

		absoluteMedianList[row] = std::abs(val - statistics.median);
		statistics.meanDeviationAroundMedian += absoluteMedianList[row];

		centralMoment_r3 += gsl_pow_3(val - statistics.arithmeticMean);
		centralMoment_r4 += gsl_pow_4(val - statistics.arithmeticMean);

		if (row != 0)
			statistics.averageTwoPeriodMovingRange += std::abs(val - rowData.value(row - 1));
	}

	double centralMoment_r2 = statistics.variance / notNanCount;

	// normalize
	statistics.variance = (notNanCount != 1) ? statistics.variance / (notNanCount - 1) : NAN;
	statistics.meanDeviationAroundMedian = statistics.meanDeviationAroundMedian / notNanCount;
	statistics.meanDeviation = statistics.meanDeviation / notNanCount;
	statistics.averageTwoPeriodMovingRange = statistics.averageTwoPeriodMovingRange / (notNanCount - 1);

	// standard deviation
	statistics.standardDeviation = std::sqrt(statistics.variance);

	//"median absolute deviation" - the median of the absolute deviations from the data's median.
	std::sort(absoluteMedianList.begin(), absoluteMedianList.end());
	statistics.medianDeviation = gsl_stats_quantile_from_sorted_data(absoluteMedianList.data(), 1, notNanCount, 0.50);

	// skewness and kurtosis
	centralMoment_r3 /= notNanCount;
	centralMoment_r4 /= notNanCount;
	statistics.skewness = centralMoment_r3 / gsl_pow_3(std::sqrt(centralMoment_r2));
	statistics.kurtosis = centralMoment_r4 / gsl_pow_2(centralMoment_r2);

	// entropy
	double entropy = 0.;
	for (const auto& v : frequencyOfValues) {
		const double frequencyNorm = static_cast<double>(v.second) / notNanCount;
		entropy += (frequencyNorm * std::log2(frequencyNorm));
	}

	statistics.entropy = -entropy;

	available.statistics = true;
	available.min = true;
	available.max = true;
}

void ColumnPrivate::calculateTextStatistics() {
	if (!available.dictionary)
		initDictionary();

	int valid = 0;
	for (int row = 0; row < rowCount(); ++row) {
		if (q->isMasked(row))
			continue;

		++valid;
	}

	statistics.size = valid;
	statistics.unique = m_dictionary.count();
	available.statistics = true;
}

void ColumnPrivate::calculateDateTimeStatistics() {
	statistics.minimum = INFINITY;
	statistics.maximum = -INFINITY;

	int valid = 0;
	for (int row = 0; row < rowCount(); ++row) {
		if (q->isMasked(row))
			continue;

		const auto& value = dateTimeAt(row);
		if (!value.isValid())
			continue;

		quint64 val = value.toMSecsSinceEpoch();
		if (val < statistics.minimum)
			statistics.minimum = val;
		if (val > statistics.maximum)
			statistics.maximum = val;

		++valid;
	}

	statistics.size = valid;
	available.statistics = true;
}
