/***************************************************************************
    File                 : ColumnPrivate.cpp
    Project              : AbstractColumn
    Description          : Private data class of Column
    --------------------------------------------------------------------
    Copyright            : (C) 2007-2008 Tilman Benkert (thzs@gmx.net)
    Copyright            : (C) 2012-2019 Alexander Semke (alexander.semke@web.de)
    Copyright            : (C) 2017-2020 Stefan Gerlach (stefan.gerlach@uni.kn)
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

#include "ColumnPrivate.h"
#include "ColumnStringIO.h"
#include "Column.h"
#include "backend/spreadsheet/Spreadsheet.h"
#include "backend/core/datatypes/filter.h"
#include "backend/gsl/ExpressionParser.h"

ColumnPrivate::ColumnPrivate(Column* owner, AbstractColumn::ColumnMode mode) :
	m_column_mode(mode), m_owner(owner) {
	Q_ASSERT(owner != nullptr);

	switch (mode) {
	case AbstractColumn::Numeric:
		m_input_filter = new String2DoubleFilter();
		m_output_filter = new Double2StringFilter('g');
		m_data = new QVector<double>();
		break;
	case AbstractColumn::Integer:
		m_input_filter = new String2IntegerFilter();
		m_output_filter = new Integer2StringFilter();
		m_data = new QVector<int>();
		break;
	case AbstractColumn::BigInt:
		//TODO
		//m_input_filter = new String2BigIntFilter();
		//m_output_filter = new BigInt2StringFilter();
		m_data = new QVector<qint64>();
		break;
	case AbstractColumn::Text:
		m_input_filter = new SimpleCopyThroughFilter();
		m_output_filter = new SimpleCopyThroughFilter();
		m_data = new QStringList();
		break;
	case AbstractColumn::DateTime:
		m_input_filter = new String2DateTimeFilter();
		m_output_filter = new DateTime2StringFilter();
		m_data = new QVector<QDateTime>();
		break;
	case AbstractColumn::Month:
		m_input_filter = new String2MonthFilter();
		m_output_filter = new DateTime2StringFilter();
		static_cast<DateTime2StringFilter*>(m_output_filter)->setFormat("MMMM");
		m_data = new QVector<QDateTime>();
		break;
	case AbstractColumn::Day:
		m_input_filter = new String2DayOfWeekFilter();
		m_output_filter = new DateTime2StringFilter();
		static_cast<DateTime2StringFilter*>(m_output_filter)->setFormat("dddd");
		m_data = new QVector<QDateTime>();
		break;
	}

	connect(m_output_filter, &AbstractSimpleFilter::formatChanged, m_owner, &Column::handleFormatChange);

	//m_input_filter->setName("InputFilter");
	//m_output_filter->setName("OutputFilter");
}

/**
 * \brief Special ctor (to be called from Column only!)
 */
ColumnPrivate::ColumnPrivate(Column* owner, AbstractColumn::ColumnMode mode, void* data) :
	m_column_mode(mode), m_data(data), m_owner(owner) {

	switch (mode) {
	case AbstractColumn::Numeric:
		m_input_filter = new String2DoubleFilter();
		m_output_filter = new Double2StringFilter();
		connect(static_cast<Double2StringFilter *>(m_output_filter), &Double2StringFilter::formatChanged,
				m_owner, &Column::handleFormatChange);
		break;
	case AbstractColumn::Integer:
		m_input_filter = new String2IntegerFilter();
		m_output_filter = new Integer2StringFilter();
		connect(static_cast<Integer2StringFilter *>(m_output_filter), &Integer2StringFilter::formatChanged,
				m_owner, &Column::handleFormatChange);
		break;
	case AbstractColumn::BigInt:
		//TODO
		//m_input_filter = new String2BigIntFilter();
		//m_output_filter = new BigInt2StringFilter();
		//connect(static_cast<BigInt2StringFilter *>(m_output_filter), &BigInt2StringFilter::formatChanged,
		//		m_owner, &Column::handleFormatChange);
		break;
	case AbstractColumn::Text:
		m_input_filter = new SimpleCopyThroughFilter();
		m_output_filter = new SimpleCopyThroughFilter();
		break;
	case AbstractColumn::DateTime:
		m_input_filter = new String2DateTimeFilter();
		m_output_filter = new DateTime2StringFilter();
		connect(static_cast<DateTime2StringFilter *>(m_output_filter), &DateTime2StringFilter::formatChanged,
				m_owner, &Column::handleFormatChange);
		break;
	case AbstractColumn::Month:
		m_input_filter = new String2MonthFilter();
		m_output_filter = new DateTime2StringFilter();
		static_cast<DateTime2StringFilter *>(m_output_filter)->setFormat("MMMM");
		connect(static_cast<DateTime2StringFilter *>(m_output_filter), &DateTime2StringFilter::formatChanged,
				m_owner, &Column::handleFormatChange);
		break;
	case AbstractColumn::Day:
		m_input_filter = new String2DayOfWeekFilter();
		m_output_filter = new DateTime2StringFilter();
		static_cast<DateTime2StringFilter *>(m_output_filter)->setFormat("dddd");
		connect(static_cast<DateTime2StringFilter *>(m_output_filter), &DateTime2StringFilter::formatChanged,
				m_owner, &Column::handleFormatChange);
		break;
	}

	//m_input_filter->setName("InputFilter");
	//m_output_filter->setName("OutputFilter");
}

ColumnPrivate::~ColumnPrivate() {
	if (!m_data) return;

	switch (m_column_mode) {
	case AbstractColumn::Numeric:
		delete static_cast<QVector<double>*>(m_data);
		break;
	case AbstractColumn::Integer:
		delete static_cast<QVector<int>*>(m_data);
		break;
	case AbstractColumn::BigInt:
		delete static_cast<QVector<qint64>*>(m_data);
		break;
	case AbstractColumn::Text:
		delete static_cast<QVector<QString>*>(m_data);
		break;
	case AbstractColumn::DateTime:
	case AbstractColumn::Month:
	case AbstractColumn::Day:
		delete static_cast<QVector<QDateTime>*>(m_data);
		break;
	}
}

AbstractColumn::ColumnMode ColumnPrivate::columnMode() const {
	return m_column_mode;
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
	DEBUG("ColumnPrivate::setColumnMode() " << ENUM_TO_STRING(AbstractColumn, ColumnMode, m_column_mode)
		<< " -> " << ENUM_TO_STRING(AbstractColumn, ColumnMode, mode))
	if (mode == m_column_mode) return;

	void* old_data = m_data;
	// remark: the deletion of the old data will be done in the dtor of a command

	AbstractSimpleFilter* filter = nullptr, *new_in_filter = nullptr, *new_out_filter = nullptr;
	bool filter_is_temporary = false; // it can also become outputFilter(), which we may not delete here
	Column* temp_col = nullptr;

	emit m_owner->modeAboutToChange(m_owner);

	// determine the conversion filter and allocate the new data vector
	switch (m_column_mode) {	// old mode
	case AbstractColumn::Numeric: {
		disconnect(static_cast<Double2StringFilter*>(m_output_filter), &Double2StringFilter::formatChanged,
				   m_owner, &Column::handleFormatChange);
		switch (mode) {
		case AbstractColumn::Numeric:
			break;
		case AbstractColumn::Integer:
			filter = new Double2IntegerFilter();
			filter_is_temporary = true;
			temp_col = new Column("temp_col", *(static_cast<QVector<double>*>(old_data)));
			m_data = new QVector<int>();
			break;
		case AbstractColumn::BigInt:
			//TODO
			//filter = new Double2BigIntFilter();
			filter_is_temporary = true;
			temp_col = new Column("temp_col", *(static_cast<QVector<double>*>(old_data)));
			m_data = new QVector<qint64>();
			break;
		case AbstractColumn::Text:
			filter = outputFilter();
			filter_is_temporary = false;
			temp_col = new Column("temp_col", *(static_cast< QVector<double>* >(old_data)));
			m_data = new QVector<QString>();
			break;
		case AbstractColumn::DateTime:
			filter = new Double2DateTimeFilter();
			filter_is_temporary = true;
			temp_col = new Column("temp_col", *(static_cast< QVector<double>* >(old_data)));
			m_data = new QVector<QDateTime>();
			break;
		case AbstractColumn::Month:
			filter = new Double2MonthFilter();
			filter_is_temporary = true;
			temp_col = new Column("temp_col", *(static_cast< QVector<double>* >(old_data)));
			m_data = new QVector<QDateTime>();
			break;
		case AbstractColumn::Day:
			filter = new Double2DayOfWeekFilter();
			filter_is_temporary = true;
			temp_col = new Column("temp_col", *(static_cast< QVector<double>* >(old_data)));
			m_data = new QVector<QDateTime>();
			break;
		} // switch(mode)

		break;
	}
	case AbstractColumn::Integer: {
		disconnect(static_cast<Integer2StringFilter*>(m_output_filter), &Integer2StringFilter::formatChanged,
				   m_owner, &Column::handleFormatChange);
		switch (mode) {
		case AbstractColumn::Integer:
			break;
		case AbstractColumn::BigInt:
			filter = new Integer2BigIntFilter();
			filter_is_temporary = true;
			temp_col = new Column("temp_col", *(static_cast<QVector<int>*>(old_data)), m_column_mode);
			m_data = new QVector<qint64>();
			break;
		case AbstractColumn::Numeric:
			filter = new Integer2DoubleFilter();
			filter_is_temporary = true;
			temp_col = new Column("temp_col", *(static_cast<QVector<int>*>(old_data)), m_column_mode);
			m_data = new QVector<double>();
			break;
		case AbstractColumn::Text:
			filter = outputFilter();
			filter_is_temporary = false;
			temp_col = new Column("temp_col", *(static_cast< QVector<int>* >(old_data)), m_column_mode);
			m_data = new QVector<QString>();
			break;
		case AbstractColumn::DateTime:
			filter = new Integer2DateTimeFilter();
			filter_is_temporary = true;
			temp_col = new Column("temp_col", *(static_cast< QVector<int>* >(old_data)), m_column_mode);
			m_data = new QVector<QDateTime>();
			break;
		case AbstractColumn::Month:
			filter = new Integer2MonthFilter();
			filter_is_temporary = true;
			temp_col = new Column("temp_col", *(static_cast< QVector<int>* >(old_data)), m_column_mode);
			m_data = new QVector<QDateTime>();
			break;
		case AbstractColumn::Day:
			filter = new Integer2DayOfWeekFilter();
			filter_is_temporary = true;
			temp_col = new Column("temp_col", *(static_cast< QVector<int>* >(old_data)), m_column_mode);
			m_data = new QVector<QDateTime>();
			break;
		} // switch(mode)

		break;
	}
	case AbstractColumn::BigInt: {
		//TODO
		//disconnect(static_cast<BigInt2StringFilter*>(m_output_filter), &BigInt2StringFilter::formatChanged,
		//		   m_owner, &Column::handleFormatChange);
		switch (mode) {
		case AbstractColumn::BigInt:
			break;
		case AbstractColumn::Integer:
			//TODO
			//filter = new BigInt2IntegerFilter();
			filter_is_temporary = true;
			temp_col = new Column("temp_col", *(static_cast<QVector<qint64>*>(old_data)), m_column_mode);
			m_data = new QVector<int>();
			break;
		case AbstractColumn::Numeric:
			//TODO
			//filter = new BigInt2DoubleFilter();
			filter_is_temporary = true;
			temp_col = new Column("temp_col", *(static_cast<QVector<qint64>*>(old_data)), m_column_mode);
			m_data = new QVector<double>();
			break;
		case AbstractColumn::Text:
			filter = outputFilter();
			filter_is_temporary = false;
			temp_col = new Column("temp_col", *(static_cast< QVector<qint64>* >(old_data)), m_column_mode);
			m_data = new QVector<QString>();
			break;
		case AbstractColumn::DateTime:
			//TODO
			//filter = new BigInt2DateTimeFilter();
			filter_is_temporary = true;
			temp_col = new Column("temp_col", *(static_cast< QVector<qint64>* >(old_data)), m_column_mode);
			m_data = new QVector<QDateTime>();
			break;
		case AbstractColumn::Month:
			//TODO
			//filter = new BigInt2MonthFilter();
			filter_is_temporary = true;
			temp_col = new Column("temp_col", *(static_cast< QVector<qint64>* >(old_data)), m_column_mode);
			m_data = new QVector<QDateTime>();
			break;
		case AbstractColumn::Day:
			//TODO
			//filter = new BigInt2DayOfWeekFilter();
			filter_is_temporary = true;
			temp_col = new Column("temp_col", *(static_cast< QVector<qint64>* >(old_data)), m_column_mode);
			m_data = new QVector<QDateTime>();
			break;
		} // switch(mode)

		break;
	}
	case AbstractColumn::Text: {
		switch (mode) {
		case AbstractColumn::Text:
			break;
		case AbstractColumn::Numeric:
			filter = new String2DoubleFilter();
			filter_is_temporary = true;
			temp_col = new Column("temp_col", *(static_cast<QVector<QString>*>(old_data)), m_column_mode);
			m_data = new QVector<double>();
			break;
		case AbstractColumn::Integer:
			filter = new String2IntegerFilter();
			filter_is_temporary = true;
			temp_col = new Column("temp_col", *(static_cast<QVector<QString>*>(old_data)), m_column_mode);
			m_data = new QVector<int>();
			break;
		case AbstractColumn::BigInt:
			//TODO
			//filter = new String2BigIntFilter();
			filter_is_temporary = true;
			temp_col = new Column("temp_col", *(static_cast<QVector<QString>*>(old_data)), m_column_mode);
			m_data = new QVector<qint64>();
			break;
		case AbstractColumn::DateTime:
			filter = new String2DateTimeFilter();
			filter_is_temporary = true;
			temp_col = new Column("temp_col", *(static_cast<QVector<QString>*>(old_data)), m_column_mode);
			m_data = new QVector<QDateTime>();
			break;
		case AbstractColumn::Month:
			filter = new String2MonthFilter();
			filter_is_temporary = true;
			temp_col = new Column("temp_col", *(static_cast<QVector<QString>*>(old_data)), m_column_mode);
			m_data = new QVector<QDateTime>();
			break;
		case AbstractColumn::Day:
			filter = new String2DayOfWeekFilter();
			filter_is_temporary = true;
			temp_col = new Column("temp_col", *(static_cast<QVector<QString>*>(old_data)), m_column_mode);
			m_data = new QVector<QDateTime>();
			break;
		} // switch(mode)

		break;
	}
	case AbstractColumn::DateTime:
	case AbstractColumn::Month:
	case AbstractColumn::Day: {
		disconnect(static_cast<DateTime2StringFilter*>(m_output_filter), &DateTime2StringFilter::formatChanged,
				m_owner, &Column::handleFormatChange);
		switch (mode) {
		case AbstractColumn::DateTime:
		case AbstractColumn::Month:
		case AbstractColumn::Day:
			break;
		case AbstractColumn::Text:
			filter = outputFilter();
			filter_is_temporary = false;
			temp_col = new Column("temp_col", *(static_cast< QVector<QDateTime>* >(old_data)), m_column_mode);
			m_data = new QStringList();
			break;
		case AbstractColumn::Numeric:
			if (m_column_mode == AbstractColumn::Month)
				filter = new Month2DoubleFilter();
			else if (m_column_mode == AbstractColumn::Day)
				filter = new DayOfWeek2DoubleFilter();
			else
				filter = new DateTime2DoubleFilter();
			filter_is_temporary = true;
			temp_col = new Column("temp_col", *(static_cast< QVector<QDateTime>* >(old_data)), m_column_mode);
			m_data = new QVector<double>();
			break;
		case AbstractColumn::Integer:
			if (m_column_mode == AbstractColumn::Month)
				filter = new Month2IntegerFilter();
			else if (m_column_mode == AbstractColumn::Day)
				filter = new DayOfWeek2IntegerFilter();
			else
				filter = new DateTime2IntegerFilter();
			filter_is_temporary = true;
			temp_col = new Column("temp_col", *(static_cast< QVector<QDateTime>* >(old_data)), m_column_mode);
			m_data = new QVector<int>();
			break;
		case AbstractColumn::BigInt:
			/* TODO
			if (m_column_mode == AbstractColumn::Month)
				filter = new Month2BigIntFilter();
			else if (m_column_mode == AbstractColumn::Day)
				filter = new DayOfWeek2BigIntFilter();
			else
				filter = new DateTime2BigIntFilter();
			*/
			filter_is_temporary = true;
			temp_col = new Column("temp_col", *(static_cast< QVector<QDateTime>* >(old_data)), m_column_mode);
			m_data = new QVector<qint64>();
			break;
		} // switch(mode)

		break;
	}
	}

	// determine the new input and output filters
	switch (mode) {	// new mode
	case AbstractColumn::Numeric:
		new_in_filter = new String2DoubleFilter();
		new_out_filter = new Double2StringFilter();
		connect(static_cast<Double2StringFilter*>(new_out_filter), &Double2StringFilter::formatChanged,
				m_owner, &Column::handleFormatChange);
		break;
	case AbstractColumn::Integer:
		new_in_filter = new String2IntegerFilter();
		new_out_filter = new Integer2StringFilter();
		connect(static_cast<Integer2StringFilter*>(new_out_filter), &Integer2StringFilter::formatChanged,
				m_owner, &Column::handleFormatChange);
		break;
	case AbstractColumn::BigInt:
		//TODO
		//new_in_filter = new String2BigIntFilter();
		//new_out_filter = new BigInt2StringFilter();
		//connect(static_cast<BigInt2StringFilter*>(new_out_filter), &BigInt2StringFilter::formatChanged,
		//		m_owner, &Column::handleFormatChange);
		break;
	case AbstractColumn::Text:
		new_in_filter = new SimpleCopyThroughFilter();
		new_out_filter = new SimpleCopyThroughFilter();
		break;
	case AbstractColumn::DateTime:
		new_in_filter = new String2DateTimeFilter();
		new_out_filter = new DateTime2StringFilter();
		connect(static_cast<DateTime2StringFilter*>(new_out_filter), &DateTime2StringFilter::formatChanged,
				m_owner, &Column::handleFormatChange);
		break;
	case AbstractColumn::Month:
		new_in_filter = new String2MonthFilter();
		new_out_filter = new DateTime2StringFilter();
		static_cast<DateTime2StringFilter*>(new_out_filter)->setFormat("MMMM");
		DEBUG("	Month out_filter format: " << static_cast<DateTime2StringFilter*>(new_out_filter)->format().toStdString());
		connect(static_cast<DateTime2StringFilter*>(new_out_filter), &DateTime2StringFilter::formatChanged,
				m_owner, &Column::handleFormatChange);
		break;
	case AbstractColumn::Day:
		new_in_filter = new String2DayOfWeekFilter();
		new_out_filter = new DateTime2StringFilter();
		static_cast<DateTime2StringFilter*>(new_out_filter)->setFormat("dddd");
		connect(static_cast<DateTime2StringFilter*>(new_out_filter), &DateTime2StringFilter::formatChanged,
				m_owner, &Column::handleFormatChange);
		break;
	} // switch(mode)

	m_column_mode = mode;

	//new_in_filter->setName("InputFilter");
	//new_out_filter->setName("OutputFilter");
	m_input_filter = new_in_filter;
	m_output_filter = new_out_filter;
	m_input_filter->input(0, m_owner->m_string_io);
	m_output_filter->input(0, m_owner);
	m_input_filter->setHidden(true);
	m_output_filter->setHidden(true);

	if (temp_col) { // if temp_col == 0, only the input/output filters need to be changed
		// copy the filtered, i.e. converted, column (mode is orig mode)
		DEBUG("	temp_col column mode = " << ENUM_TO_STRING(AbstractColumn, ColumnMode, temp_col->columnMode()));
		filter->input(0, temp_col);
		DEBUG("	filter->output size = " << filter->output(0)->rowCount());
		copy(filter->output(0));
		delete temp_col;
	}

	if (filter_is_temporary) delete filter;

	emit m_owner->modeChanged(m_owner);
	DEBUG("ColumnPrivate::setColumnMode() DONE");
}

/**
 * \brief Replace all mode related members
 *
 * Replace column mode, data type, data pointer and filters directly
 */
void ColumnPrivate::replaceModeData(AbstractColumn::ColumnMode mode, void* data,
				AbstractSimpleFilter* in_filter, AbstractSimpleFilter* out_filter) {
	DEBUG("ColumnPrivate::replaceModeData()");
	emit m_owner->modeAboutToChange(m_owner);
	// disconnect formatChanged()
	switch (m_column_mode) {
	case AbstractColumn::Numeric:
		disconnect(static_cast<Double2StringFilter*>(m_output_filter), &Double2StringFilter::formatChanged,
				   m_owner, &Column::handleFormatChange);
		break;
	case AbstractColumn::Integer:
		disconnect(static_cast<Integer2StringFilter*>(m_output_filter), &Integer2StringFilter::formatChanged,
				   m_owner, &Column::handleFormatChange);
		break;
	case AbstractColumn::BigInt:
		//TODO disconnect(static_cast<BigInt2StringFilter*>(m_output_filter), &BigInt2StringFilter::formatChanged,
		//		   m_owner, &Column::handleFormatChange);
		break;
	case AbstractColumn::Text:
		break;
	case AbstractColumn::DateTime:
	case AbstractColumn::Month:
	case AbstractColumn::Day:
		disconnect(static_cast<DateTime2StringFilter*>(m_output_filter), &DateTime2StringFilter::formatChanged,
				   m_owner, &Column::handleFormatChange);
		break;
	}

	m_column_mode = mode;
	m_data = data;

	//in_filter->setName("InputFilter");
	//out_filter->setName("OutputFilter");
	m_input_filter = in_filter;
	m_output_filter = out_filter;
	m_input_filter->input(0, m_owner->m_string_io);
	m_output_filter->input(0, m_owner);

	// connect formatChanged()
	switch (m_column_mode) {
	case AbstractColumn::Numeric:
		connect(static_cast<Double2StringFilter*>(m_output_filter), &Double2StringFilter::formatChanged,
				m_owner, &Column::handleFormatChange);
		break;
	case AbstractColumn::Integer:
		connect(static_cast<Integer2StringFilter*>(m_output_filter), &Integer2StringFilter::formatChanged,
				m_owner, &Column::handleFormatChange);
		break;
	case AbstractColumn::BigInt:
		//TODO connect(static_cast<BigInt2StringFilter*>(m_output_filter), &BigInt2StringFilter::formatChanged,
		//		m_owner, &Column::handleFormatChange);
		break;
	case AbstractColumn::Text:
		break;
	case AbstractColumn::DateTime:
	case AbstractColumn::Month:
	case AbstractColumn::Day:
		connect(static_cast<DateTime2StringFilter*>(m_output_filter), &DateTime2StringFilter::formatChanged,
				m_owner, &Column::handleFormatChange);
		break;
	}

	emit m_owner->modeChanged(m_owner);
}

/**
 * \brief Replace data pointer
 */
void ColumnPrivate::replaceData(void* data) {
	DEBUG("ColumnPrivate::replaceData()");
	emit m_owner->dataAboutToChange(m_owner);
	m_data = data;
	if (!m_owner->m_suppressDataChangedSignal)
		emit m_owner->dataChanged(m_owner);
}

/**
 * \brief Copy another column of the same type
 *
 * This function will return false if the data type
 * of 'other' is not the same as the type of 'this'.
 * Use a filter to convert a column to another type.
 */
bool ColumnPrivate::copy(const AbstractColumn* other) {
	DEBUG("ColumnPrivate::copy(other)");
	if (other->columnMode() != columnMode()) return false;
	DEBUG("	mode = " << ENUM_TO_STRING(AbstractColumn, ColumnMode, columnMode()));
	int num_rows = other->rowCount();
	DEBUG("	rows " << num_rows);

	emit m_owner->dataAboutToChange(m_owner);
	resizeTo(num_rows);

	// copy the data
	switch (m_column_mode) {
	case AbstractColumn::Numeric: {
		double* ptr = static_cast<QVector<double>*>(m_data)->data();
		for (int i = 0; i < num_rows; ++i)
			ptr[i] = other->valueAt(i);
		break;
	}
	case AbstractColumn::Integer: {
		int* ptr = static_cast<QVector<int>*>(m_data)->data();
		for (int i = 0; i < num_rows; ++i)
			ptr[i] = other->integerAt(i);
		break;
	}
	case AbstractColumn::BigInt: {
		qint64* ptr = static_cast<QVector<qint64>*>(m_data)->data();
		for (int i = 0; i < num_rows; ++i)
			ptr[i] = other->bigIntAt(i);
		break;
	}
	case AbstractColumn::Text: {
		auto* vec = static_cast<QVector<QString>*>(m_data);
		for (int i = 0; i < num_rows; ++i)
			vec->replace(i, other->textAt(i));
		break;
	}
	case AbstractColumn::DateTime:
	case AbstractColumn::Month:
	case AbstractColumn::Day: {
		auto* vec = static_cast<QVector<QDateTime>*>(m_data);
		for (int i = 0; i < num_rows; ++i)
			vec->replace(i, other->dateTimeAt(i));
		break;
	}
	}

	if (!m_owner->m_suppressDataChangedSignal)
		emit m_owner->dataChanged(m_owner);

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
	DEBUG("ColumnPrivate::copy()");
	if (source->columnMode() != m_column_mode) return false;
	if (num_rows == 0) return true;

	emit m_owner->dataAboutToChange(m_owner);
	if (dest_start + num_rows > rowCount())
		resizeTo(dest_start + num_rows);

	// copy the data
	switch (m_column_mode) {
	case AbstractColumn::Numeric: {
		double* ptr = static_cast<QVector<double>*>(m_data)->data();
		for (int i = 0; i < num_rows; i++)
			ptr[dest_start+i] = source->valueAt(source_start + i);
		break;
	}
	case AbstractColumn::Integer: {
		int* ptr = static_cast<QVector<int>*>(m_data)->data();
		for (int i = 0; i < num_rows; i++)
			ptr[dest_start+i] = source->integerAt(source_start + i);
		break;
	}
	case AbstractColumn::BigInt: {
		qint64* ptr = static_cast<QVector<qint64>*>(m_data)->data();
		for (int i = 0; i < num_rows; i++)
			ptr[dest_start+i] = source->bigIntAt(source_start + i);
		break;
	}
	case AbstractColumn::Text:
		for (int i = 0; i < num_rows; i++)
			static_cast<QVector<QString>*>(m_data)->replace(dest_start+i, source->textAt(source_start + i));
		break;
	case AbstractColumn::DateTime:
	case AbstractColumn::Month:
	case AbstractColumn::Day:
		for (int i = 0; i < num_rows; i++)
			static_cast<QVector<QDateTime>*>(m_data)->replace(dest_start+i, source->dateTimeAt(source_start + i));
		break;
	}

	if (!m_owner->m_suppressDataChangedSignal)
		emit m_owner->dataChanged(m_owner);
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
	if (other->columnMode() != m_column_mode) return false;
	int num_rows = other->rowCount();

	emit m_owner->dataAboutToChange(m_owner);
	resizeTo(num_rows);

	// copy the data
	switch (m_column_mode) {
	case AbstractColumn::Numeric: {
		double* ptr = static_cast<QVector<double>*>(m_data)->data();
		for (int i = 0; i < num_rows; ++i)
			ptr[i] = other->valueAt(i);
		break;
	}
	case AbstractColumn::Integer: {
		int* ptr = static_cast<QVector<int>*>(m_data)->data();
		for (int i = 0; i < num_rows; ++i)
			ptr[i] = other->integerAt(i);
		break;
	}
	case AbstractColumn::BigInt: {
		qint64* ptr = static_cast<QVector<qint64>*>(m_data)->data();
		for (int i = 0; i < num_rows; ++i)
			ptr[i] = other->bigIntAt(i);
		break;
	}
	case AbstractColumn::Text:
		for (int i = 0; i < num_rows; ++i)
			static_cast<QVector<QString>*>(m_data)->replace(i, other->textAt(i));
		break;
	case AbstractColumn::DateTime:
	case AbstractColumn::Month:
	case AbstractColumn::Day:
		for (int i = 0; i < num_rows; ++i)
			static_cast<QVector<QDateTime>*>(m_data)->replace(i, other->dateTimeAt(i));
		break;
	}

	if (!m_owner->m_suppressDataChangedSignal)
		emit m_owner->dataChanged(m_owner);

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
	if (source->columnMode() != m_column_mode) return false;
	if (num_rows == 0) return true;

	emit m_owner->dataAboutToChange(m_owner);
	if (dest_start + num_rows > rowCount())
		resizeTo(dest_start + num_rows);

	// copy the data
	switch (m_column_mode) {
	case AbstractColumn::Numeric: {
		double* ptr = static_cast<QVector<double>*>(m_data)->data();
		for (int i = 0; i < num_rows; ++i)
			ptr[dest_start+i] = source->valueAt(source_start + i);
		break;
	}
	case AbstractColumn::Integer: {
		int* ptr = static_cast<QVector<int>*>(m_data)->data();
		for (int i = 0; i < num_rows; ++i)
			ptr[dest_start+i] = source->integerAt(source_start + i);
		break;
	}
	case AbstractColumn::BigInt: {
		qint64* ptr = static_cast<QVector<qint64>*>(m_data)->data();
		for (int i = 0; i < num_rows; ++i)
			ptr[dest_start+i] = source->bigIntAt(source_start + i);
		break;
	}
	case AbstractColumn::Text:
		for (int i = 0; i < num_rows; ++i)
			static_cast<QVector<QString>*>(m_data)->replace(dest_start+i, source->textAt(source_start + i));
		break;
	case AbstractColumn::DateTime:
	case AbstractColumn::Month:
	case AbstractColumn::Day:
		for (int i = 0; i  <num_rows; ++i)
			static_cast<QVector<QDateTime>*>(m_data)->replace(dest_start+i, source->dateTimeAt(source_start + i));
		break;
	}

	if (!m_owner->m_suppressDataChangedSignal)
		emit m_owner->dataChanged(m_owner);

	return true;
}

/**
 * \brief Return the data vector size
 *
 * This returns the number of rows that actually contain data.
 * Rows beyond this can be masked etc. but should be ignored by filters,
 * plots etc.
 */
int ColumnPrivate::rowCount() const {
	switch (m_column_mode) {
	case AbstractColumn::Numeric:
		return static_cast<QVector<double>*>(m_data)->size();
	case AbstractColumn::Integer:
		return static_cast<QVector<int>*>(m_data)->size();
	case AbstractColumn::BigInt:
		return static_cast<QVector<qint64>*>(m_data)->size();
	case AbstractColumn::DateTime:
	case AbstractColumn::Month:
	case AbstractColumn::Day:
		return static_cast<QVector<QDateTime>*>(m_data)->size();
	case AbstractColumn::Text:
		return static_cast<QVector<QString>*>(m_data)->size();
	}

	return 0;
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

	DEBUG("ColumnPrivate::resizeTo() " << old_size << " -> " << new_size);

	switch (m_column_mode) {
	case AbstractColumn::Numeric: {
		auto* numeric_data = static_cast<QVector<double>*>(m_data);
		numeric_data->insert(numeric_data->end(), new_size - old_size, NAN);
		break;
	}
	case AbstractColumn::Integer: {
		auto* numeric_data = static_cast<QVector<int>*>(m_data);
		numeric_data->insert(numeric_data->end(), new_size - old_size, 0);
		break;
	}
	case AbstractColumn::BigInt: {
		auto* numeric_data = static_cast<QVector<qint64>*>(m_data);
		numeric_data->insert(numeric_data->end(), new_size - old_size, 0);
		break;
	}
	case AbstractColumn::Text: {
		int new_rows = new_size - old_size;
		if (new_rows > 0) {
			for (int i = 0; i < new_rows; ++i)
				static_cast<QVector<QString>*>(m_data)->append(QString());
		} else {
			for (int i = 0; i < -new_rows; ++i)
				static_cast<QVector<QString>*>(m_data)->removeLast();
		}
		break;
	}
	case AbstractColumn::DateTime:
	case AbstractColumn::Month:
	case AbstractColumn::Day: {
		int new_rows = new_size - old_size;
		if (new_rows > 0) {
			for (int i = 0; i < new_rows; ++i)
				static_cast<QVector<QDateTime>*>(m_data)->append(QDateTime());
		} else {
			for (int i = 0; i < -new_rows; ++i)
				static_cast<QVector<QDateTime>*>(m_data)->removeLast();
		}
		break;
	}
	}
}

/**
 * \brief Insert some empty (or initialized with zero) rows
 */
void ColumnPrivate::insertRows(int before, int count) {
	if (count == 0) return;

	m_formulas.insertRows(before, count);

	if (before <= rowCount()) {
		switch (m_column_mode) {
		case AbstractColumn::Numeric:
			static_cast<QVector<double>*>(m_data)->insert(before, count, NAN);
			break;
		case AbstractColumn::Integer:
			static_cast<QVector<int>*>(m_data)->insert(before, count, 0);
			break;
		case AbstractColumn::BigInt:
			static_cast<QVector<qint64>*>(m_data)->insert(before, count, 0);
			break;
		case AbstractColumn::DateTime:
		case AbstractColumn::Month:
		case AbstractColumn::Day:
			for (int i = 0; i < count; ++i)
				static_cast<QVector<QDateTime>*>(m_data)->insert(before, QDateTime());
			break;
		case AbstractColumn::Text:
			for (int i = 0; i < count; ++i)
				static_cast<QVector<QString>*>(m_data)->insert(before, QString());
			break;
		}
	}
}

/**
 * \brief Remove 'count' rows starting from row 'first'
 */
void ColumnPrivate::removeRows(int first, int count) {
	if (count == 0) return;

	m_formulas.removeRows(first, count);

	if (first < rowCount()) {
		int corrected_count = count;
		if (first + count > rowCount())
			corrected_count = rowCount() - first;

		switch (m_column_mode) {
		case AbstractColumn::Numeric:
			static_cast<QVector<double>*>(m_data)->remove(first, corrected_count);
			break;
		case AbstractColumn::Integer:
			static_cast<QVector<int>*>(m_data)->remove(first, corrected_count);
			break;
		case AbstractColumn::BigInt:
			static_cast<QVector<qint64>*>(m_data)->remove(first, corrected_count);
			break;
		case AbstractColumn::DateTime:
		case AbstractColumn::Month:
		case AbstractColumn::Day:
			for (int i = 0; i < corrected_count; ++i)
				static_cast<QVector<QDateTime>*>(m_data)->removeAt(first);
			break;
		case AbstractColumn::Text:
			for (int i = 0; i < corrected_count; ++i)
				static_cast<QVector<QString>*>(m_data)->removeAt(first);
			break;
		}
	}
}

//! Return the column name
QString ColumnPrivate::name() const {
	return m_owner->name();
}

/**
 * \brief Return the column plot designation
 */
AbstractColumn::PlotDesignation ColumnPrivate::plotDesignation() const {
	return m_plot_designation;
}

/**
 * \brief Set the column plot designation
 */
void ColumnPrivate::setPlotDesignation(AbstractColumn::PlotDesignation pd) {
	emit m_owner->plotDesignationAboutToChange(m_owner);
	m_plot_designation = pd;
	emit m_owner->plotDesignationChanged(m_owner);
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
 * \brief Return the data pointer
 */
void* ColumnPrivate::data() const {
	return m_data;
}

/**
 * \brief Return the input filter (for string -> data type conversion)
 */
AbstractSimpleFilter *ColumnPrivate::inputFilter() const {
	return m_input_filter;
}

/**
 * \brief Return the output filter (for data type -> string  conversion)
 */
AbstractSimpleFilter *ColumnPrivate::outputFilter() const {
	return m_output_filter;
}

////////////////////////////////////////////////////////////////////////////////
//! \name Formula related functions
//@{
////////////////////////////////////////////////////////////////////////////////
/**
 * \brief Return the formula last used to generate data for the column
 */
QString ColumnPrivate::formula() const {
	return m_formula;
}

bool ColumnPrivate::formulaAutoUpdate() const {
	return m_formulaAutoUpdate;
}

/**
 * \brief Sets the formula used to generate column values
 */
void ColumnPrivate::setFormula(const QString& formula, const QStringList& variableNames,
							   const QVector<Column*>& variableColumns, bool autoUpdate) {
	m_formula = formula;
	m_formulaVariableNames = variableNames;
	m_formulaVariableColumns = variableColumns;
	m_formulaAutoUpdate = autoUpdate;

	for (auto connection: m_connectionsUpdateFormula)
		disconnect(connection);

	m_formulaVariableColumnPaths.clear();

	for (auto column : variableColumns) {
		m_formulaVariableColumnPaths << column->path();
		if (autoUpdate)
			connectFormulaColumn(column);
	}
}

/*!
 * called after the import of the project was done and all columns were loaded in \sa Project::load()
 * to establish the required slot-signal connections for the formula update
 */
void ColumnPrivate::finalizeLoad() {
	if (m_formulaAutoUpdate) {
		for (auto column : m_formulaVariableColumns)
			connectFormulaColumn(column);
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

	m_connectionsUpdateFormula << connect(column, &Column::dataChanged, m_owner, &Column::updateFormula);
	connect(column->parentAspect(), &AbstractAspect::aspectAboutToBeRemoved, this, &ColumnPrivate::formulaVariableColumnRemoved);
	connect(column, &AbstractColumn::reset, this, &ColumnPrivate::formulaVariableColumnRemoved);
	connect(column->parentAspect(), &AbstractAspect::aspectAdded, this, &ColumnPrivate::formulaVariableColumnAdded);
}

/*!
 * helper function used in \c Column::load() to set parameters read from the xml file.
 * \param variableColumnPaths is used to restore the pointers to columns from pathes
 * after the project was loaded in Project::load().
 */
 void ColumnPrivate::setFormula(const QString& formula, const QStringList& variableNames,
								const QStringList& variableColumnPaths, bool autoUpdate) {
	m_formula = formula;
	m_formulaVariableNames = variableNames;
	m_formulaVariableColumnPaths = variableColumnPaths;
	m_formulaVariableColumns.resize(variableColumnPaths.length());
	m_formulaAutoUpdate = autoUpdate;
}

const QStringList& ColumnPrivate::formulaVariableNames() const {
	return m_formulaVariableNames;
}

const QVector<Column*>& ColumnPrivate::formulaVariableColumns() const {
	return m_formulaVariableColumns;
}

const QStringList& ColumnPrivate::formulaVariableColumnPaths() const {
	return m_formulaVariableColumnPaths;
}

void ColumnPrivate::setformulVariableColumnsPath(int index, QString path) {
	m_formulaVariableColumnPaths[index] = path;
}

void ColumnPrivate::setformulVariableColumn(int index, Column* column) {
	if (m_formulaVariableColumns[index]) // if there exists already a valid column, disconnect it first
		disconnect(m_formulaVariableColumns[index], nullptr, this, nullptr);
	m_formulaVariableColumns[index] = column;
	connectFormulaColumn(column);
}

/*!
 * \sa FunctionValuesDialog::generate()
 */
void ColumnPrivate::updateFormula() {
	//determine variable names and the data vectors of the specified columns
	QVector<QVector<double>*> xVectors;
	QVector<QVector<double>*> xNewVectors;
	int maxRowCount = 0;

	bool valid = true;
	for (auto column : m_formulaVariableColumns) {
		if (!column) {
			valid = false;
			break;
		}

		if (column->columnMode() == AbstractColumn::Integer) {
			//convert integers to doubles first
			auto* xVector = new QVector<double>(column->rowCount());
			for (int i = 0; i<column->rowCount(); ++i)
				xVector->operator[](i) = column->valueAt(i);

			xNewVectors << xVector;
			xVectors << xVector;
		} else
			xVectors << static_cast<QVector<double>* >(column->data());

		if (column->rowCount() > maxRowCount)
			maxRowCount = column->rowCount();
	}

	if (valid) {
		//resize the spreadsheet if one of the data vectors from
		//other spreadsheet(s) has more elements than the parent spreadsheet
		Spreadsheet* spreadsheet = dynamic_cast<Spreadsheet*>(m_owner->parentAspect());
		Q_ASSERT(spreadsheet);
		if (spreadsheet->rowCount() < maxRowCount)
			spreadsheet->setRowCount(maxRowCount);

		//create new vector for storing the calculated values
		//the vectors with the variable data can be smaller then the result vector. So, not all values in the result vector might get initialized.
		//->"clean" the result vector first
		QVector<double> new_data(rowCount(), NAN);

		//evaluate the expression for f(x_1, x_2, ...) and write the calculated values into a new vector.
		ExpressionParser* parser = ExpressionParser::getInstance();
		parser->evaluateCartesian(m_formula, m_formulaVariableNames, xVectors, &new_data);
		replaceValues(0, new_data);

		// initialize remaining rows with NAN
		int remainingRows = rowCount() - maxRowCount;
		if (remainingRows > 0) {
			QVector<double> emptyRows(remainingRows, NAN);
			replaceValues(maxRowCount, emptyRows);
		}
	} else {
		QVector<double> new_data(rowCount(), NAN);
		replaceValues(0, new_data);
	}

	//delete help vectors created for the conversion from int to double
	for (auto* vector : xNewVectors)
		delete vector;
}

void ColumnPrivate::formulaVariableColumnRemoved(const AbstractAspect* aspect) {
	const Column* column = dynamic_cast<const Column*>(aspect);
	disconnect(column, nullptr, this, nullptr);
	//TODO: why is const_cast required here?!?
	int index = m_formulaVariableColumns.indexOf(const_cast<Column*>(column));
	if (index != -1) {
		m_formulaVariableColumns[index] = nullptr;
		updateFormula();
	}
}

void ColumnPrivate::formulaVariableColumnAdded(const AbstractAspect* aspect) {
	int index = m_formulaVariableColumnPaths.indexOf(aspect->path());
	if (index != -1) {
		const Column* column = dynamic_cast<const Column*>(aspect);
		m_formulaVariableColumns[index] = const_cast<Column*>(column);
		updateFormula();
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
QVector< Interval<int> > ColumnPrivate::formulaIntervals() const {
	return m_formulas.intervals();
}

/**
 * \brief Set a formula string for an interval of rows
 */
void ColumnPrivate::setFormula(Interval<int> i, QString formula) {
	m_formulas.setValue(i, formula);
}

/**
 * \brief Overloaded function for convenience
 */
void ColumnPrivate::setFormula(int row, QString formula) {
	setFormula(Interval<int>(row,row), formula);
}

/**
 * \brief Clear all formulas
 */
void ColumnPrivate::clearFormulas() {
	m_formulas.clear();
}

////////////////////////////////////////////////////////////////////////////////
//@}
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
//! \name type specific functions
//@{
////////////////////////////////////////////////////////////////////////////////

/**
 * \brief Return the content of row 'row'.
 *
 * Use this only when columnMode() is Text
 */
QString ColumnPrivate::textAt(int row) const {
	if (m_column_mode != AbstractColumn::Text) return QString();
	return static_cast<QVector<QString>*>(m_data)->value(row);
}

/**
 * \brief Return the date part of row 'row'
 *
 * Use this only when columnMode() is DateTime, Month or Day
 */
QDate ColumnPrivate::dateAt(int row) const {
	if (m_column_mode != AbstractColumn::DateTime &&
	        m_column_mode != AbstractColumn::Month &&
	        m_column_mode != AbstractColumn::Day)
		return QDate{};
	return dateTimeAt(row).date();
}

/**
 * \brief Return the time part of row 'row'
 *
 * Use this only when columnMode() is DateTime, Month or Day
 */
QTime ColumnPrivate::timeAt(int row) const {
	if (m_column_mode != AbstractColumn::DateTime &&
	        m_column_mode != AbstractColumn::Month &&
	        m_column_mode != AbstractColumn::Day)
		return QTime{};
	return dateTimeAt(row).time();
}

/**
 * \brief Return the QDateTime in row 'row'
 *
 * Use this only when columnMode() is DateTime, Month or Day
 */
QDateTime ColumnPrivate::dateTimeAt(int row) const {
	if (m_column_mode != AbstractColumn::DateTime &&
	        m_column_mode != AbstractColumn::Month &&
	        m_column_mode != AbstractColumn::Day)
		return QDateTime();
	return static_cast<QVector<QDateTime>*>(m_data)->value(row);
}

/**
 * \brief Return the double value in row 'row' for columns with type Numeric and Integer.
 * This function has to be used everywhere where the exact type (double or int) is not relevant for numerical calculations.
 * For cases where the integer value is needed without any implicit conversions, \sa integerAt() has to be used.
 */
double ColumnPrivate::valueAt(int row) const {
	if (m_column_mode == AbstractColumn::Numeric)
		return static_cast<QVector<double>*>(m_data)->value(row, NAN);
	else if (m_column_mode == AbstractColumn::Integer)
		return static_cast<QVector<int>*>(m_data)->value(row, 0);
	else
		 return NAN;
}

/**
 * \brief Return the int value in row 'row'
 */
int ColumnPrivate::integerAt(int row) const {
	if (m_column_mode != AbstractColumn::Integer) return 0;
	return static_cast<QVector<int>*>(m_data)->value(row, 0);
}

/**
 * \brief Return the bigint value in row 'row'
 */
qint64 ColumnPrivate::bigIntAt(int row) const {
	if (m_column_mode != AbstractColumn::BigInt) return 0;
	return static_cast<QVector<qint64>*>(m_data)->value(row, 0);
}

/**
 * \brief Set the content of row 'row'
 *
 * Use this only when columnMode() is Text
 */
void ColumnPrivate::setTextAt(int row, const QString& new_value) {
	if (m_column_mode != AbstractColumn::Text) return;

	emit m_owner->dataAboutToChange(m_owner);
	if (row >= rowCount())
		resizeTo(row + 1);

	static_cast<QVector<QString>*>(m_data)->replace(row, new_value);
	if (!m_owner->m_suppressDataChangedSignal)
		emit m_owner->dataChanged(m_owner);
}

/**
 * \brief Replace a range of values
 *
 * Use this only when columnMode() is Text
 */
void ColumnPrivate::replaceTexts(int first, const QVector<QString>& new_values) {
	if (m_column_mode != AbstractColumn::Text) return;

	emit m_owner->dataAboutToChange(m_owner);
	int num_rows = new_values.size();
	if (first + num_rows > rowCount())
		resizeTo(first + num_rows);

	for (int i = 0; i < num_rows; ++i)
		static_cast<QVector<QString>*>(m_data)->replace(first+i, new_values.at(i));

	if (!m_owner->m_suppressDataChangedSignal)
		emit m_owner->dataChanged(m_owner);
}

/**
 * \brief Set the content of row 'row'
 *
 * Use this only when columnMode() is DateTime, Month or Day
 */
void ColumnPrivate::setDateAt(int row, QDate new_value) {
	if (m_column_mode != AbstractColumn::DateTime &&
	        m_column_mode != AbstractColumn::Month &&
	        m_column_mode != AbstractColumn::Day)
		return;

	setDateTimeAt(row, QDateTime(new_value, timeAt(row)));
}

/**
 * \brief Set the content of row 'row'
 *
 * Use this only when columnMode() is DateTime, Month or Day
 */
void ColumnPrivate::setTimeAt(int row, QTime new_value) {
	if (m_column_mode != AbstractColumn::DateTime &&
	        m_column_mode != AbstractColumn::Month &&
	        m_column_mode != AbstractColumn::Day)
		return;

	setDateTimeAt(row, QDateTime(dateAt(row), new_value));
}

/**
 * \brief Set the content of row 'row'
 *
 * Use this only when columnMode() is DateTime, Month or Day
 */
void ColumnPrivate::setDateTimeAt(int row, const QDateTime& new_value) {
	if (m_column_mode != AbstractColumn::DateTime &&
	        m_column_mode != AbstractColumn::Month &&
	        m_column_mode != AbstractColumn::Day)
		return;

	emit m_owner->dataAboutToChange(m_owner);
	if (row >= rowCount())
		resizeTo(row+1);

	static_cast< QVector<QDateTime>* >(m_data)->replace(row, new_value);
	if (!m_owner->m_suppressDataChangedSignal)
		emit m_owner->dataChanged(m_owner);
}

/**
 * \brief Replace a range of values
 *
 * Use this only when columnMode() is DateTime, Month or Day
 */
void ColumnPrivate::replaceDateTimes(int first, const QVector<QDateTime>& new_values) {
	if (m_column_mode != AbstractColumn::DateTime &&
	        m_column_mode != AbstractColumn::Month &&
	        m_column_mode != AbstractColumn::Day)
		return;

	emit m_owner->dataAboutToChange(m_owner);
	int num_rows = new_values.size();
	if (first + num_rows > rowCount())
		resizeTo(first + num_rows);

	for (int i = 0; i < num_rows; ++i)
		static_cast<QVector<QDateTime>*>(m_data)->replace(first+i, new_values.at(i));

	if (!m_owner->m_suppressDataChangedSignal)
		emit m_owner->dataChanged(m_owner);
}

/**
 * \brief Set the content of row 'row'
 *
 * Use this only when columnMode() is Numeric
 */
void ColumnPrivate::setValueAt(int row, double new_value) {
//	DEBUG("ColumnPrivate::setValueAt()");
	if (m_column_mode != AbstractColumn::Numeric) return;

	emit m_owner->dataAboutToChange(m_owner);
	if (row >= rowCount())
		resizeTo(row+1);

	static_cast<QVector<double>*>(m_data)->replace(row, new_value);
	if (!m_owner->m_suppressDataChangedSignal)
		emit m_owner->dataChanged(m_owner);
}

/**
 * \brief Replace a range of values
 *
 * Use this only when columnMode() is Numeric
 */
void ColumnPrivate::replaceValues(int first, const QVector<double>& new_values) {
	DEBUG("ColumnPrivate::replaceValues()");
	if (m_column_mode != AbstractColumn::Numeric) return;

	emit m_owner->dataAboutToChange(m_owner);
	int num_rows = new_values.size();
	if (first + num_rows > rowCount())
		resizeTo(first + num_rows);

	double* ptr = static_cast<QVector<double>*>(m_data)->data();
	for (int i = 0; i < num_rows; ++i)
		ptr[first+i] = new_values.at(i);

	if (!m_owner->m_suppressDataChangedSignal)
		emit m_owner->dataChanged(m_owner);
}

/**
 * \brief Set the content of row 'row'
 *
 * Use this only when columnMode() is Integer
 */
void ColumnPrivate::setIntegerAt(int row, int new_value) {
	DEBUG("ColumnPrivate::setIntegerAt()");
	if (m_column_mode != AbstractColumn::Integer) return;

	emit m_owner->dataAboutToChange(m_owner);
	if (row >= rowCount())
		resizeTo(row+1);

	static_cast<QVector<int>*>(m_data)->replace(row, new_value);
	if (!m_owner->m_suppressDataChangedSignal)
		emit m_owner->dataChanged(m_owner);
}

/**
 * \brief Replace a range of values
 *
 * Use this only when columnMode() is Integer
 */
void ColumnPrivate::replaceInteger(int first, const QVector<int>& new_values) {
	DEBUG("ColumnPrivate::replaceInteger()");
	if (m_column_mode != AbstractColumn::Integer) return;

	emit m_owner->dataAboutToChange(m_owner);
	int num_rows = new_values.size();
	if (first + num_rows > rowCount())
		resizeTo(first + num_rows);

	int* ptr = static_cast<QVector<int>*>(m_data)->data();
	for (int i = 0; i < num_rows; ++i)
		ptr[first+i] = new_values.at(i);

	if (!m_owner->m_suppressDataChangedSignal)
		emit m_owner->dataChanged(m_owner);
}

/**
 * \brief Set the content of row 'row'
 *
 * Use this only when columnMode() is BigInt
 */
void ColumnPrivate::setBigIntAt(int row, qint64 new_value) {
	DEBUG("ColumnPrivate::setBigIntAt()");
	if (m_column_mode != AbstractColumn::BigInt) return;

	emit m_owner->dataAboutToChange(m_owner);
	if (row >= rowCount())
		resizeTo(row+1);

	static_cast<QVector<qint64>*>(m_data)->replace(row, new_value);
	if (!m_owner->m_suppressDataChangedSignal)
		emit m_owner->dataChanged(m_owner);
}

/**
 * \brief Replace a range of values
 *
 * Use this only when columnMode() is BigInt
 */
void ColumnPrivate::replaceBigInt(int first, const QVector<qint64>& new_values) {
	DEBUG("ColumnPrivate::replaceBigInt()");
	if (m_column_mode != AbstractColumn::BigInt) return;

	emit m_owner->dataAboutToChange(m_owner);
	int num_rows = new_values.size();
	if (first + num_rows > rowCount())
		resizeTo(first + num_rows);

	qint64* ptr = static_cast<QVector<qint64>*>(m_data)->data();
	for (int i = 0; i < num_rows; ++i)
		ptr[first+i] = new_values.at(i);

	if (!m_owner->m_suppressDataChangedSignal)
		emit m_owner->dataChanged(m_owner);
}

/*!
 * Updates the properties. Will be called, when data in the column changed.
 * The properties will be used to speed up some algorithms.
 * See where variable properties will be used.
 */
void ColumnPrivate::updateProperties() {

	// TODO: for double Properties::Constant will never be used. Use an epsilon (difference smaller than epsilon is zero)
	if (rowCount() == 0) {
		properties = AbstractColumn::Properties::No;
		propertiesAvailable = true;
		return;
	}

	double prevValue = NAN;
	int prevValueInt = 0;
	qint64 prevValueDatetime = 0;

	if (m_column_mode == AbstractColumn::Integer)
		prevValueInt = integerAt(0);
	else if (m_column_mode == AbstractColumn::Numeric)
		prevValue = valueAt(0);
	else if (m_column_mode == AbstractColumn::DateTime ||
			m_column_mode == AbstractColumn::Month ||
			m_column_mode == AbstractColumn::Day)
		prevValueDatetime = dateTimeAt(0).toMSecsSinceEpoch();
	else {
		properties = AbstractColumn::Properties::No;
		propertiesAvailable = true;
		return;
	}


	int monotonic_decreasing = -1;
	int monotonic_increasing = -1;

	double value;
	int valueInt;
	qint64 valueDateTime;

	for (int row = 1; row < rowCount(); row++) {
		if (!m_owner->isValid(row) || m_owner->isMasked(row)) {
			// if there is one invalid or masked value, the property is No, because
			// otherwise it's difficult to find the correct index in indexForValue().
			// You don't know if you should increase the index or decrease it when
			// you hit an invalid value
			properties = AbstractColumn::Properties::No;
			propertiesAvailable = true;
			return;
		}

		if (m_column_mode == AbstractColumn::Integer) {
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

			} else if (m_column_mode == AbstractColumn::Numeric) {
				value = valueAt(row);

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

			} else if (m_column_mode == AbstractColumn::DateTime ||
					   m_column_mode == AbstractColumn::Month ||
					   m_column_mode == AbstractColumn::Day) {

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
			}
	}

	properties = AbstractColumn::Properties::No;
	if (monotonic_increasing > 0 && monotonic_decreasing > 0)
		properties = AbstractColumn::Properties::Constant;
	else if (monotonic_decreasing > 0)
		properties = AbstractColumn::Properties::MonotonicDecreasing;
	else if (monotonic_increasing > 0)
		properties = AbstractColumn::Properties::MonotonicIncreasing;

	propertiesAvailable = true;
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
