/***************************************************************************
    File                 : ColumnPrivate.cpp
    Project              : SciDAVis
    Description          : Private data class of Column
    --------------------------------------------------------------------
    Copyright            : (C) 2007,2008 Tilman Benkert (thzs*gmx.net)
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

#include "ColumnPrivate.h"
#include "Column.h"
#include "core/AbstractSimpleFilter.h"
#include "core/datatypes/SimpleCopyThroughFilter.h"
#include "core/datatypes/String2DoubleFilter.h"
#include "core/datatypes/Double2StringFilter.h"
#include "core/datatypes/Double2DateTimeFilter.h"
#include "core/datatypes/Double2MonthFilter.h"
#include "core/datatypes/Double2DayOfWeekFilter.h"
#include "core/datatypes/String2DateTimeFilter.h"
#include "core/datatypes/DateTime2StringFilter.h"
#include "core/datatypes/String2MonthFilter.h"
#include "core/datatypes/String2DayOfWeekFilter.h"
#include "core/datatypes/DateTime2DoubleFilter.h"
#include "core/datatypes/DayOfWeek2DoubleFilter.h"
#include "core/datatypes/Month2DoubleFilter.h"
#include <QString>
#include <QStringList>
#include <QtDebug>

/**
 * \class Column::Private
 * \brief Private data class of Column
 *
 * The writing interface defined here is only to be used by column commands and Column contructors.
 */

/**
 * \var Column::Private::m_column_mode
 * \brief The column mode
 *
 * The column mode specifies how to interpret 
 * the values in the column additional to the data type.
 */

/**
 * \var Column::Private::m_data
 * \brief Pointer to the data vector
 *
 * This will point to a QVector<double>, QStringList or
 * QList<QDateTime> depending on the stored data type.
 */

/**
 * \var Column::Private::m_input_filter
 * \brief The input filter (for string -> data type conversion)
 */

/**
 * \var Column::Private::m_output_filter
 * \brief The output filter (for data type -> string conversion)
 */

/**
 * \var Column::Private::m_plot_designation
 * \brief The plot designation
 */

/**
 * \var Column::Private::m_width
 * \brief Width to be used by views
 */

/**
 * \var Column::Private::m_owner
 * \brief The owner column
 */

/**
 * \brief Ctor
 */
Column::Private::Private(Column * owner, SciDAVis::ColumnMode mode)
 : m_owner(owner)
{
	Q_ASSERT(owner != 0); // a Column::Private without owner is not allowed 
					      // because the owner must become the parent aspect of the input and output filters
	m_column_mode = mode;
	switch(mode)
	{		
		case SciDAVis::Numeric:
			m_input_filter = new String2DoubleFilter();
			m_output_filter = new Double2StringFilter();
			connect(static_cast<Double2StringFilter *>(m_output_filter), SIGNAL(formatChanged()),
				m_owner, SLOT(handleFormatChange()));
			m_data = new QVector<double>();
			break;
		case SciDAVis::Text:
			m_input_filter = new SimpleCopyThroughFilter();
			m_output_filter = new SimpleCopyThroughFilter();
			m_data = new QStringList();
			break;
		case SciDAVis::DateTime:
			m_input_filter = new String2DateTimeFilter();
			m_output_filter = new DateTime2StringFilter();
			connect(static_cast<DateTime2StringFilter *>(m_output_filter), SIGNAL(formatChanged()),
				m_owner, SLOT(handleFormatChange()));
			m_data = new QList<QDateTime>();
			break;
		case SciDAVis::Month:
			m_input_filter = new String2MonthFilter();
			m_output_filter = new DateTime2StringFilter();
			static_cast<DateTime2StringFilter *>(m_output_filter)->setFormat("MMMM");
			connect(static_cast<DateTime2StringFilter *>(m_output_filter), SIGNAL(formatChanged()),
				m_owner, SLOT(handleFormatChange()));
			m_data = new QList<QDateTime>();
			break;
		case SciDAVis::Day:
			m_input_filter = new String2DayOfWeekFilter();
			m_output_filter = new DateTime2StringFilter();
			static_cast<DateTime2StringFilter *>(m_output_filter)->setFormat("dddd");
			connect(static_cast<DateTime2StringFilter *>(m_output_filter), SIGNAL(formatChanged()),
				m_owner, SLOT(handleFormatChange()));
			m_data = new QList<QDateTime>();
			break;
	} // switch(mode)

	m_plot_designation = SciDAVis::noDesignation;
	m_input_filter->setName("InputFilter");
	m_output_filter->setName("OutputFilter");
}

/**
 * \brief Special ctor (to be called from Column only!)
 */
Column::Private::Private(Column * owner, SciDAVis::ColumnMode mode, void * data) 
	: m_owner(owner)
{
	m_column_mode = mode;
	m_data = data;

	switch(mode)
	{		
		case SciDAVis::Numeric:
			m_input_filter = new String2DoubleFilter();
			m_output_filter = new Double2StringFilter();
			connect(static_cast<Double2StringFilter *>(m_output_filter), SIGNAL(formatChanged()),
				m_owner, SLOT(handleFormatChange()));
			break;
		case SciDAVis::Text:
			m_input_filter = new SimpleCopyThroughFilter();
			m_output_filter = new SimpleCopyThroughFilter();
			break;
		case SciDAVis::DateTime:
			m_input_filter = new String2DateTimeFilter();
			m_output_filter = new DateTime2StringFilter();
			connect(static_cast<DateTime2StringFilter *>(m_output_filter), SIGNAL(formatChanged()),
				m_owner, SLOT(handleFormatChange()));
			break;
		case SciDAVis::Month:
			m_input_filter = new String2MonthFilter();
			m_output_filter = new DateTime2StringFilter();
			static_cast<DateTime2StringFilter *>(m_output_filter)->setFormat("MMMM");
			connect(static_cast<DateTime2StringFilter *>(m_output_filter), SIGNAL(formatChanged()),
				m_owner, SLOT(handleFormatChange()));
			break;
		case SciDAVis::Day:
			m_input_filter = new String2DayOfWeekFilter();
			m_output_filter = new DateTime2StringFilter();
			static_cast<DateTime2StringFilter *>(m_output_filter)->setFormat("dddd");
			connect(static_cast<DateTime2StringFilter *>(m_output_filter), SIGNAL(formatChanged()),
				m_owner, SLOT(handleFormatChange()));
			break;
	} // switch(mode)

	m_plot_designation = SciDAVis::noDesignation;
	m_input_filter->setName("InputFilter");
	m_output_filter->setName("OutputFilter");
}

/**
 * \brief Dtor
 */
Column::Private::~Private()
{
	if (!m_data) return;

	switch(m_column_mode) {
		case SciDAVis::Numeric:
			delete static_cast< QVector<double>* >(m_data);
			break;

		case SciDAVis::Text:
			delete static_cast< QStringList* >(m_data);
			break;
			
		case SciDAVis::DateTime:
		case SciDAVis::Month:
		case SciDAVis::Day:
			delete static_cast< QList<QDateTime>* >(m_data);
			break;
	} // switch(m_column_mode)
}

/**
 * \brief Return the column mode
 *
 * This function is most used by spreadsheets but can also be used
 * by plots. The column mode specifies how to interpret 
 * the values in the column additional to the data type.
 */ 
SciDAVis::ColumnMode Column::Private::columnMode() const {
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
void Column::Private::setColumnMode(SciDAVis::ColumnMode mode)
{
	if (mode == m_column_mode) return;

	void * old_data = m_data;
	// remark: the deletion of the old data will be done in the dtor of a command

	AbstractSimpleFilter *filter, *new_in_filter, *new_out_filter;
	bool filter_is_temporary = false; // it can also become outputFilter(), which we may not delete here
	Column* temp_col = 0;

	emit m_owner->modeAboutToChange(m_owner);

	// determine the conversion filter and allocate the new data vector
	switch(m_column_mode)
	{
		case SciDAVis::Numeric:
			disconnect(static_cast<Double2StringFilter *>(m_output_filter), SIGNAL(formatChanged()),
				m_owner, SLOT(handleFormatChange()));
			switch(mode)
			{		
				case SciDAVis::Numeric:
					break;
				case SciDAVis::Text:
					filter = outputFilter(); filter_is_temporary = false;
					temp_col = new Column("temp_col", *(static_cast< QVector<double>* >(old_data)));
					m_data = new QStringList();
					break;
				case SciDAVis::DateTime:
					filter = new Double2DateTimeFilter(); filter_is_temporary = true;
					temp_col = new Column("temp_col", *(static_cast< QVector<double>* >(old_data)));
					m_data = new QList<QDateTime>();
					break;
				case SciDAVis::Month:
					filter = new Double2MonthFilter(); filter_is_temporary = true;
					temp_col = new Column("temp_col", *(static_cast< QVector<double>* >(old_data)));
					m_data = new QList<QDateTime>();
					break;
				case SciDAVis::Day:
					filter = new Double2DayOfWeekFilter(); filter_is_temporary = true;
					temp_col = new Column("temp_col", *(static_cast< QVector<double>* >(old_data)));
					m_data = new QList<QDateTime>();
					break;
			} // switch(mode)
			break;

		case SciDAVis::Text:
			switch(mode)
			{		
				case SciDAVis::Text:
					break;
				case SciDAVis::Numeric:
					filter = new String2DoubleFilter(); filter_is_temporary = true;
					temp_col = new Column("temp_col", *(static_cast< QStringList* >(old_data)));
					m_data = new QVector<double>();
					break;
				case SciDAVis::DateTime:
					filter = new String2DateTimeFilter(); filter_is_temporary = true;
					temp_col = new Column("temp_col", *(static_cast< QStringList* >(old_data)));
					m_data = new QList<QDateTime>();
					break;
				case SciDAVis::Month:
					filter = new String2MonthFilter(); filter_is_temporary = true;
					temp_col = new Column("temp_col", *(static_cast< QStringList* >(old_data)));
					m_data = new QList<QDateTime>();
					break;
				case SciDAVis::Day:
					filter = new String2DayOfWeekFilter(); filter_is_temporary = true;
					temp_col = new Column("temp_col", *(static_cast< QStringList* >(old_data)));
					m_data = new QList<QDateTime>();
					break;
			} // switch(mode)
			break;

		case SciDAVis::DateTime:
		case SciDAVis::Month:
		case SciDAVis::Day:
			disconnect(static_cast<DateTime2StringFilter *>(m_output_filter), SIGNAL(formatChanged()),
				m_owner, SLOT(handleFormatChange()));
			switch(mode)
			{		
				case SciDAVis::DateTime:
					break;
				case SciDAVis::Text:
					filter = outputFilter(); filter_is_temporary = false;
					temp_col = new Column("temp_col", *(static_cast< QList<QDateTime>* >(old_data)));
					m_data = new QStringList();
					break;
				case SciDAVis::Numeric:
					if (m_column_mode == SciDAVis::Month)
						filter = new Month2DoubleFilter();
					else if (m_column_mode == SciDAVis::Day)
						filter = new DayOfWeek2DoubleFilter();
					else
						filter = new DateTime2DoubleFilter();
					filter_is_temporary = true;
					temp_col = new Column("temp_col", *(static_cast< QList<QDateTime>* >(old_data)));
					m_data = new QVector<double>();
					break;
				case SciDAVis::Month:
				case SciDAVis::Day:
					break;
			} // switch(mode)
			break;

	}

	// determine the new input and output filters
	switch(mode) {		
		case SciDAVis::Numeric:
			new_in_filter = new String2DoubleFilter();
			new_out_filter = new Double2StringFilter();
			connect(static_cast<Double2StringFilter *>(new_out_filter), SIGNAL(formatChanged()),
				m_owner, SLOT(handleFormatChange()));
			break;
		case SciDAVis::Text:
			new_in_filter = new SimpleCopyThroughFilter();
			new_out_filter = new SimpleCopyThroughFilter();
			break;
		case SciDAVis::DateTime:
			new_in_filter = new String2DateTimeFilter();
			new_out_filter = new DateTime2StringFilter();
			connect(static_cast<DateTime2StringFilter *>(new_out_filter), SIGNAL(formatChanged()),
				m_owner, SLOT(handleFormatChange()));
			break;
		case SciDAVis::Month:
			new_in_filter = new String2MonthFilter();
			new_out_filter = new DateTime2StringFilter();
			static_cast<DateTime2StringFilter *>(new_out_filter)->setFormat("MMMM");
			connect(static_cast<DateTime2StringFilter *>(new_out_filter), SIGNAL(formatChanged()),
				m_owner, SLOT(handleFormatChange()));
			break;
		case SciDAVis::Day:
			new_in_filter = new String2DayOfWeekFilter();
			new_out_filter = new DateTime2StringFilter();
			static_cast<DateTime2StringFilter *>(new_out_filter)->setFormat("dddd");
			connect(static_cast<DateTime2StringFilter *>(new_out_filter), SIGNAL(formatChanged()),
				m_owner, SLOT(handleFormatChange()));
			break;
	} // switch(mode)

	m_column_mode = mode;

	new_in_filter->setName("InputFilter");
	new_out_filter->setName("OutputFilter");
	m_input_filter = new_in_filter;
	m_output_filter = new_out_filter;
	m_input_filter->input(0, m_owner->m_string_io);
	m_output_filter->input(0, m_owner);
	m_input_filter->setHidden(true);
	m_output_filter->setHidden(true);

	if (temp_col) // if temp_col == 0, only the input/output filters need to be changed
	{
		// copy the filtered, i.e. converted, column
		filter->input(0, temp_col);
		copy(filter->output(0));
		delete temp_col;
	}

	if (filter_is_temporary) delete filter;

	emit m_owner->modeChanged(m_owner);
}

/**
 * \brief Replace all mode related members
 *
 * Replace column mode, data type, data pointer and filters directly 
 */
void Column::Private::replaceModeData(SciDAVis::ColumnMode mode, void * data, 
	AbstractSimpleFilter * in_filter, AbstractSimpleFilter * out_filter)
{
	emit m_owner->modeAboutToChange(m_owner);
	// disconnect formatChanged()
	switch(m_column_mode)
	{
		case SciDAVis::Numeric:
			disconnect(static_cast<Double2StringFilter *>(m_output_filter), SIGNAL(formatChanged()),
				m_owner, SLOT(handleFormatChange()));
			break;
		case SciDAVis::DateTime:
		case SciDAVis::Month:
		case SciDAVis::Day:
			disconnect(static_cast<DateTime2StringFilter *>(m_output_filter), SIGNAL(formatChanged()),
				m_owner, SLOT(handleFormatChange()));
			break;
		default:
			break;
	}

	m_column_mode = mode;
	m_data = data;

	in_filter->setName("InputFilter");
	out_filter->setName("OutputFilter");
	m_input_filter = in_filter;
	m_output_filter = out_filter;
	m_input_filter->input(0, m_owner->m_string_io);
	m_output_filter->input(0, m_owner);

	// connect formatChanged()
	switch(m_column_mode) {		
		case SciDAVis::Numeric:
			connect(static_cast<Double2StringFilter *>(m_output_filter), SIGNAL(formatChanged()),
				m_owner, SLOT(handleFormatChange()));
			break;
		case SciDAVis::DateTime:
		case SciDAVis::Month:
		case SciDAVis::Day:
			connect(static_cast<DateTime2StringFilter *>(m_output_filter), SIGNAL(formatChanged()),
				m_owner, SLOT(handleFormatChange()));
			break;
		default:
			break;
	} 

	emit m_owner->modeChanged(m_owner);
}

/**
 * \brief Replace data pointer
 */
void Column::Private::replaceData(void * data)
{
	emit m_owner->dataAboutToChange(m_owner);
	m_data = data;
	emit m_owner->dataChanged(m_owner);
}

/**
 * \brief Copy another column of the same type
 *
 * This function will return false if the data type
 * of 'other' is not the same as the type of 'this'.
 * Use a filter to convert a column to another type.
 */
bool Column::Private::copy(const AbstractColumn * other)
{
	if (other->columnMode() != columnMode()) return false;
	int num_rows = other->rowCount();

	emit m_owner->dataAboutToChange(m_owner);
	resizeTo(num_rows); 

	// copy the data
	switch(m_column_mode) {
		case SciDAVis::Numeric:
			{
				double * ptr = static_cast< QVector<double>* >(m_data)->data();
				for(int i=0; i<num_rows; i++)
					ptr[i] = other->valueAt(i);
				break;
			}
		case SciDAVis::Text:
			{
				for(int i=0; i<num_rows; i++)
					static_cast< QStringList* >(m_data)->replace(i, other->textAt(i));
				break;
			}
		case SciDAVis::DateTime:
		case SciDAVis::Month:
		case SciDAVis::Day:
			{
				for(int i=0; i<num_rows; i++)
					static_cast< QList<QDateTime>* >(m_data)->replace(i, other->dateTimeAt(i));
				break;
			}
	}

	emit m_owner->dataChanged(m_owner);

	return true;
}

/**
 * \brief Copies a part of another column of the same type
 *
 * This function will return false if the data type
 * of 'other' is not the same as the type of 'this'.
 * \param other pointer to the column to copy
 * \param src_start first row to copy in the column to copy
 * \param dest_start first row to copy in
 * \param num_rows the number of rows to copy
 */ 
bool Column::Private::copy(const AbstractColumn * source, int source_start, int dest_start, int num_rows)
{
	if (source->columnMode() != m_column_mode) return false;
	if (num_rows == 0) return true;

	emit m_owner->dataAboutToChange(m_owner);
	if (dest_start + num_rows > rowCount())
		resizeTo(dest_start + num_rows); 

	// copy the data
	switch(m_column_mode) {
		case SciDAVis::Numeric:
			{
				double * ptr = static_cast< QVector<double>* >(m_data)->data();
				for(int i=0; i<num_rows; i++)
					ptr[dest_start+i] = source->valueAt(source_start + i);
				break;
			}
		case SciDAVis::Text:
				for(int i=0; i<num_rows; i++)
					static_cast< QStringList* >(m_data)->replace(dest_start+i, source->textAt(source_start + i));
				break;
		case SciDAVis::DateTime:
		case SciDAVis::Month:
		case SciDAVis::Day:
				for(int i=0; i<num_rows; i++)
					static_cast< QList<QDateTime>* >(m_data)->replace(dest_start+i, source->dateTimeAt(source_start + i));
				break;
	}

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
bool Column::Private::copy(const Private * other)
{
	if (other->columnMode() != m_column_mode) return false;
	int num_rows = other->rowCount();

	emit m_owner->dataAboutToChange(m_owner);
	resizeTo(num_rows); 

	// copy the data
	switch(m_column_mode) {
		case SciDAVis::Numeric:
			{
				double * ptr = static_cast< QVector<double>* >(m_data)->data();
				for(int i=0; i<num_rows; i++)
					ptr[i] = other->valueAt(i);
				break;
			}
		case SciDAVis::Text:
			{
				for(int i=0; i<num_rows; i++)
					static_cast< QStringList* >(m_data)->replace(i, other->textAt(i));
				break;
			}
		case SciDAVis::DateTime:
		case SciDAVis::Month:
		case SciDAVis::Day:
			{
				for(int i=0; i<num_rows; i++)
					static_cast< QList<QDateTime>* >(m_data)->replace(i, other->dateTimeAt(i));
				break;
			}
	}

	emit m_owner->dataChanged(m_owner);

	return true;
}

/**
 * \brief Copies a part of another column of the same type
 *
 * This function will return false if the data type
 * of 'other' is not the same as the type of 'this'.
 * \param other pointer to the column to copy
 * \param src_start first row to copy in the column to copy
 * \param dest_start first row to copy in
 * \param num_rows the number of rows to copy
 */ 
bool Column::Private::copy(const Private * source, int source_start, int dest_start, int num_rows)
{
	if (source->columnMode() != m_column_mode) return false;
	if (num_rows == 0) return true;

	emit m_owner->dataAboutToChange(m_owner);
	if (dest_start + num_rows > rowCount())
		resizeTo(dest_start + num_rows); 

	// copy the data
	switch(m_column_mode) {
		case SciDAVis::Numeric:
			{
				double * ptr = static_cast< QVector<double>* >(m_data)->data();
				for(int i=0; i<num_rows; i++)
					ptr[dest_start+i] = source->valueAt(source_start + i);
				break;
			}
		case SciDAVis::Text:
				for(int i=0; i<num_rows; i++)
					static_cast< QStringList* >(m_data)->replace(dest_start+i, source->textAt(source_start + i));
				break;
		case SciDAVis::DateTime:
		case SciDAVis::Month:
		case SciDAVis::Day:
				for(int i=0; i<num_rows; i++)
					static_cast< QList<QDateTime>* >(m_data)->replace(dest_start+i, source->dateTimeAt(source_start + i));
				break;
	}

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
int Column::Private::rowCount() const
{
	switch(m_column_mode) {
		case SciDAVis::Numeric:
			return static_cast< QVector<double>* >(m_data)->size();
		case SciDAVis::DateTime:
		case SciDAVis::Month:
		case SciDAVis::Day:
			return static_cast< QList<QDateTime>* >(m_data)->size();
		case SciDAVis::Text:
			return static_cast< QStringList* >(m_data)->size();
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
void Column::Private::resizeTo(int new_size)
{
	int old_size = rowCount();
	if (new_size == old_size) return;

	switch(m_column_mode) {
		case SciDAVis::Numeric:
			{
				QVector<double> *numeric_data = static_cast< QVector<double>* >(m_data);
				numeric_data->insert(numeric_data->end(), new_size-old_size, NAN);
				break;
			}
		case SciDAVis::DateTime:
		case SciDAVis::Month:
		case SciDAVis::Day:
			{
				int new_rows = new_size - old_size;
				if (new_rows > 0) {
					for(int i=0; i<new_rows; i++)
						static_cast< QList<QDateTime>* >(m_data)->append(QDateTime());
				} else {
					for(int i=0; i<-new_rows; i++)
						static_cast< QList<QDateTime>* >(m_data)->removeLast();
				}
				break;
			}
		case SciDAVis::Text:
			{
				int new_rows = new_size - old_size;
				if (new_rows > 0) {
					for(int i=0; i<new_rows; i++)
						static_cast< QStringList* >(m_data)->append(QString());
				} else {
					for(int i=0; i<-new_rows; i++)
						static_cast< QStringList* >(m_data)->removeLast();
				}
				break;
			}
	}
}

/**
 * \brief Insert some empty (or initialized with zero) rows
 */
void Column::Private::insertRows(int before, int count)
{
	if (count == 0) return;

	m_formulas.insertRows(before, count);

	if (before <= rowCount()) {
		switch(m_column_mode) {
			case SciDAVis::Numeric:
				static_cast< QVector<double>* >(m_data)->insert(before, count, NAN);
				break;
			case SciDAVis::DateTime:
			case SciDAVis::Month:
			case SciDAVis::Day:
				for(int i=0; i<count; i++)
					static_cast< QList<QDateTime>* >(m_data)->insert(before, QDateTime());
				break;
			case SciDAVis::Text:
				for(int i=0; i<count; i++)
					static_cast< QStringList* >(m_data)->insert(before, QString());
				break;
		}
	}
}

/**
 * \brief Remove 'count' rows starting from row 'first'
 */
void Column::Private::removeRows(int first, int count)
{
	if (count == 0) return;

	m_formulas.removeRows(first, count);

	if (first < rowCount()) 
	{
		int corrected_count = count;
		if (first + count > rowCount()) 
			corrected_count = rowCount() - first;

		switch(m_column_mode) {
			case SciDAVis::Numeric:
				static_cast< QVector<double>* >(m_data)->remove(first, corrected_count);
				break;
			case SciDAVis::DateTime:
			case SciDAVis::Month:
			case SciDAVis::Day:
				for(int i=0; i<corrected_count; i++)
					static_cast< QList<QDateTime>* >(m_data)->removeAt(first);
				break;
			case SciDAVis::Text:
				for(int i=0; i<corrected_count; i++)
					static_cast< QStringList* >(m_data)->removeAt(first);
				break;
		}
	}
}

//! Return the column name
QString Column::Private::name() const {
	return m_owner->name();
}

/**
 * \brief Return the column plot designation
 */
SciDAVis::PlotDesignation Column::Private::plotDesignation() const {
	return m_plot_designation;
}

/**
 * \brief Set the column plot designation
 */
void Column::Private::setPlotDesignation(SciDAVis::PlotDesignation pd)
{
	emit m_owner->plotDesignationAboutToChange(m_owner);
	m_plot_designation = pd; 
	emit m_owner->plotDesignationChanged(m_owner);
}

/**
 * \brief Get width
 */
int Column::Private::width() const {
	return m_width;
}

/**
 * \brief Set width
 */
void Column::Private::setWidth(int value)
{
	emit m_owner->widthAboutToChange(m_owner);
	m_width = value;
	emit m_owner->widthChanged(m_owner);
}

/**
 * \brief Return the data pointer
 */
void *Column::Private::dataPointer() const {
	return m_data;
}

/**
 * \brief Return the input filter (for string -> data type conversion)
 */
AbstractSimpleFilter *Column::Private::inputFilter() const {
	return m_input_filter;
}

/**
 * \brief Return the output filter (for data type -> string  conversion)
 */
AbstractSimpleFilter *Column::Private::outputFilter() const {
	return m_output_filter;
}

////////////////////////////////////////////////////////////////////////////////
//! \name Formula related functions
//@{
////////////////////////////////////////////////////////////////////////////////

/**
 * \brief Return the formula associated with row 'row' 	 
 */
QString Column::Private::formula(int row) const {
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
 * QList< Interval<int> > intervals = my_column.formulaIntervals();
 * foreach(Interval<int> interval, intervals)
 * 	list << QString(interval.toString() + ": " + my_column.formula(interval.start()));
 * \endcode
 */
QList< Interval<int> > Column::Private::formulaIntervals() const {
	return m_formulas.intervals();
}

/**
 * \brief Set a formula string for an interval of rows
 */
void Column::Private::setFormula(Interval<int> i, QString formula)
{
	m_formulas.setValue(i, formula);
}

/**
 * \brief Overloaded function for convenience
 */
void Column::Private::setFormula(int row, QString formula)
{
	setFormula(Interval<int>(row,row), formula);
}

/**
 * \brief Clear all formulas
 */
void Column::Private::clearFormulas()
{
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
QString Column::Private::textAt(int row) const
{
	if (m_column_mode != SciDAVis::Text) return QString();
	return static_cast< QStringList* >(m_data)->value(row);
}

/**
 * \brief Return the date part of row 'row'
 *
 * Use this only when columnMode() is DateTime, Month or Day
 */
QDate Column::Private::dateAt(int row) const
{
	return dateTimeAt(row).date();
}

/**
 * \brief Return the time part of row 'row'
 *
 * Use this only when columnMode() is DateTime, Month or Day
 */
QTime Column::Private::timeAt(int row) const
{
	return dateTimeAt(row).time();
}

/**
 * \brief Return the QDateTime in row 'row'
 *
 * Use this only when columnMode() is DateTime, Month or Day
 */
QDateTime Column::Private::dateTimeAt(int row) const
{
	if (m_column_mode != SciDAVis::DateTime &&
			m_column_mode != SciDAVis::Month &&
			m_column_mode != SciDAVis::Day)
		return QDateTime();
	return static_cast< QList<QDateTime>* >(m_data)->value(row);
}

/**
 * \brief Return the double value in row 'row'
 */
double Column::Private::valueAt(int row) const
{
	if (m_column_mode != SciDAVis::Numeric) return NAN;
	return static_cast< QVector<double>* >(m_data)->value(row, NAN);
}

/**
 * \brief Set the content of row 'row'
 *
 * Use this only when columnMode() is Text
 */
void Column::Private::setTextAt(int row, const QString& new_value)
{
	if (m_column_mode != SciDAVis::Text) return;

	emit m_owner->dataAboutToChange(m_owner);
	if (row >= rowCount())
		resizeTo(row+1); 

	static_cast< QStringList* >(m_data)->replace(row, new_value);
	emit m_owner->dataChanged(m_owner);
}

/**
 * \brief Replace a range of values 
 *
 * Use this only when columnMode() is Text
 */
void Column::Private::replaceTexts(int first, const QStringList& new_values)
{
	if (m_column_mode != SciDAVis::Text) return;
	
	emit m_owner->dataAboutToChange(m_owner);
	int num_rows = new_values.size();
	if (first + num_rows > rowCount())
		resizeTo(first + num_rows);

	for(int i=0; i<num_rows; i++)
		static_cast< QStringList* >(m_data)->replace(first+i, new_values.at(i));
	emit m_owner->dataChanged(m_owner);
}

/**
 * \brief Set the content of row 'row'
 *
 * Use this only when columnMode() is DateTime, Month or Day
 */
void Column::Private::setDateAt(int row, const QDate& new_value)
{
	if (m_column_mode != SciDAVis::DateTime &&
			m_column_mode != SciDAVis::Month &&
			m_column_mode != SciDAVis::Day)
		return;

	setDateTimeAt(row, QDateTime(new_value, timeAt(row)));
}

/**
 * \brief Set the content of row 'row'
 *
 * Use this only when columnMode() is DateTime, Month or Day
 */
void Column::Private::setTimeAt(int row, const QTime& new_value)
{
	if (m_column_mode != SciDAVis::DateTime &&
			m_column_mode != SciDAVis::Month &&
			m_column_mode != SciDAVis::Day)
		return;
	
	setDateTimeAt(row, QDateTime(dateAt(row), new_value));
}

/**
 * \brief Set the content of row 'row'
 *
 * Use this only when columnMode() is DateTime, Month or Day
 */
void Column::Private::setDateTimeAt(int row, const QDateTime& new_value)
{
	if (m_column_mode != SciDAVis::DateTime &&
			m_column_mode != SciDAVis::Month &&
			m_column_mode != SciDAVis::Day)
		return;

	emit m_owner->dataAboutToChange(m_owner);
	if (row >= rowCount())
		resizeTo(row+1); 

	static_cast< QList<QDateTime>* >(m_data)->replace(row, new_value);
	emit m_owner->dataChanged(m_owner);
}

/**
 * \brief Replace a range of values 
 *
 * Use this only when columnMode() is DateTime, Month or Day
 */
void Column::Private::replaceDateTimes(int first, const QList<QDateTime>& new_values)
{
	if (m_column_mode != SciDAVis::DateTime &&
			m_column_mode != SciDAVis::Month &&
			m_column_mode != SciDAVis::Day)
		return;
	
	emit m_owner->dataAboutToChange(m_owner);
	int num_rows = new_values.size();
	if (first + num_rows > rowCount())
		resizeTo(first + num_rows);

	for(int i=0; i<num_rows; i++)
		static_cast< QList<QDateTime>* >(m_data)->replace(first+i, new_values.at(i));
	emit m_owner->dataChanged(m_owner);
}

/**
 * \brief Set the content of row 'row'
 *
 * Use this only when columnMode() is Numeric
 */
void Column::Private::setValueAt(int row, double new_value)
{
	if (m_column_mode != SciDAVis::Numeric) return;

	emit m_owner->dataAboutToChange(m_owner);
	if (row >= rowCount())
		resizeTo(row+1); 

	static_cast< QVector<double>* >(m_data)->replace(row, new_value);
	emit m_owner->dataChanged(m_owner);
}

/**
 * \brief Replace a range of values 
 *
 * Use this only when columnMode() is Numeric
 */
void Column::Private::replaceValues(int first, const QVector<double>& new_values)
{
	if (m_column_mode != SciDAVis::Numeric) return;
	
	emit m_owner->dataAboutToChange(m_owner);
	int num_rows = new_values.size();
	if (first + num_rows > rowCount())
		resizeTo(first + num_rows);

	double * ptr = static_cast< QVector<double>* >(m_data)->data();
	for(int i=0; i<num_rows; i++)
		ptr[first+i] = new_values.at(i);
	emit m_owner->dataChanged(m_owner);
}

////////////////////////////////////////////////////////////////////////////////
//@}
////////////////////////////////////////////////////////////////////////////////

/**
 * \brief Return the interval attribute representing the formula strings
 */
IntervalAttribute<QString> Column::Private::formulaAttribute() const {
	return m_formulas;
}

/**
 * \brief Replace the interval attribute for the formula strings
 */
void Column::Private::replaceFormulas(IntervalAttribute<QString> formulas)
{
	m_formulas = formulas;
}

