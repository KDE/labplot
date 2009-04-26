/***************************************************************************
    File                 : Column.cpp
    Project              : SciDAVis
    Description          : Aspect that manages a column
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

#include "core/column/Column.h"
#include "core/column/ColumnPrivate.h"
#include "core/column/columncommands.h"
#include "lib/XmlStreamReader.h"
#include <QIcon>
#include <QXmlStreamWriter>
#include <QtDebug>
#include <QVariant>

/**
 * \class Column
 * \brief Aspect that manages a column
 *
 * This class represents a column, i.e., (mathematically) a 1D vector of 
 * values with a header. It provides a public reading and (undo aware) writing 
 * interface as defined in AbstractColumn. It manages special attributes
 * of column rows such as masking and a validity flag. A column
 * can have one of currently three data types: double, QString, or
 * QDateTime. The string representation of the values can differ depending
 * on the mode of the column.
 *
 * Column inherits from AbstractAspect and is intended to be a child
 * of the corresponding Spreadsheet in the aspect hierarchy. Columns don't
 * have a view as they are intended to be displayed inside a spreadsheet.
 */

void Column::staticInit()
{
	setGlobalDefault("default_width", 120);
}

/**
 * \brief Ctor
 *
 * \param name the column name (= aspect name)
 * \param mode initial column mode
 */
Column::Column(const QString& name, SciDAVis::ColumnMode mode)
 : AbstractColumn(name)
{
	m_column_private = new Private(this, mode);
	init();
}

/**
 * \brief Ctor
 *
 * \param name the column name (= aspect name)
 * \param data initial data vector
 * \param validity a list of invalid intervals (optional)
 */
Column::Column(const QString& name, QVector<double> data, IntervalAttribute<bool> validity)
 : AbstractColumn(name)
{
	m_column_private = new Private(this, SciDAVis::Numeric, new QVector<double>(data), validity);
	init();
}

/**
 * \brief Ctor
 *
 * \param name the column name (= aspect name)
 * \param data initial data vector
 * \param validity a list of invalid intervals (optional)
 */
Column::Column(const QString& name, QStringList data, IntervalAttribute<bool> validity)
 : AbstractColumn(name)
{
	m_column_private = new Private(this, SciDAVis::Text, new QStringList(data), validity);
	init();
}

/**
 * \brief Ctor
 *
 * \param name the column name (= aspect name)
 * \param data initial data vector
 * \param validity a list of invalid intervals (optional)
 */
Column::Column(const QString& name, QList<QDateTime> data, IntervalAttribute<bool> validity)
 : AbstractColumn(name)
{
	m_column_private = new Private(this, SciDAVis::DateTime, new QList<QDateTime>(data), validity);
	init();
}

/**
 * \brief Common part of ctors
 */
void Column::init()
{
	m_string_io = new ColumnStringIO(this);
	m_column_private->inputFilter()->input(0,m_string_io);
	m_column_private->outputFilter()->input(0,this);
	m_column_private->inputFilter()->setHidden(true);
	m_column_private->outputFilter()->setHidden(true);
	addChild(m_column_private->inputFilter());
	addChild(m_column_private->outputFilter());
	m_column_private->setWidth(global("default_width").toInt());
}

/**
 * \brief Dtor
 */
Column::~Column()
{
	delete m_column_private;
}

/**
 * \brief Set the column mode
 *
 * This sets the column mode and, if
 * necessary, converts it to another datatype.
 */
void Column::setColumnMode(SciDAVis::ColumnMode mode)
{
	if(mode == columnMode()) return;
	beginMacro(QObject::tr("%1: change column type").arg(name()));
	AbstractSimpleFilter * old_input_filter = m_column_private->inputFilter();
	AbstractSimpleFilter * old_output_filter = m_column_private->outputFilter();
	exec(new ColumnSetModeCmd(m_column_private, mode));
	if (m_column_private->inputFilter() != old_input_filter) 
	{
		removeChild(old_input_filter);
		addChild(m_column_private->inputFilter());
		m_column_private->inputFilter()->input(0,m_string_io);
	}
	if (m_column_private->outputFilter() != old_output_filter) 
	{
		removeChild(old_output_filter);
		addChild(m_column_private->outputFilter());
		m_column_private->outputFilter()->input(0, this);
	}
	endMacro();
}


/**
 * \brief Copy another column of the same type
 *
 * This function will return false if the data type
 * of 'other' is not the same as the type of 'this'.
 * The validity information for the rows is also copied.
 * Use a filter to convert a column to another type.
 */
bool Column::copy(const AbstractColumn * other)
{
	Q_CHECK_PTR(other);
	if(other->columnMode() != columnMode()) return false;
	exec(new ColumnFullCopyCmd(m_column_private, other));
	return true;
}

/**
 * \brief Copies a part of another column of the same type
 *
 * This function will return false if the data type
 * of 'other' is not the same as the type of 'this'.
 * The validity information for the rows is also copied.
 * \param other pointer to the column to copy
 * \param src_start first row to copy in the column to copy
 * \param dest_start first row to copy in
 * \param num_rows the number of rows to copy
 */ 
bool Column::copy(const AbstractColumn * source, int source_start, int dest_start, int num_rows)
{
	Q_CHECK_PTR(source);
	if(source->columnMode() != columnMode()) return false;
	exec(new ColumnPartialCopyCmd(m_column_private, source, source_start, dest_start, num_rows));
	return true;
}

/**
 * \brief Insert some empty (or initialized with zero) rows
 */
void Column::insertRows(int before, int count)
{
	if(count > 0)
		exec(new ColumnInsertEmptyRowsCmd(m_column_private, before, count));
}

/**
 * \brief Remove 'count' rows starting from row 'first'
 */
void Column::removeRows(int first, int count)
{
	if(count > 0)
		exec(new ColumnRemoveRowsCmd(m_column_private, first, count));
}

/**
 * \brief Set the column plot designation
 */
void Column::setPlotDesignation(SciDAVis::PlotDesignation pd)
{
	if(pd != plotDesignation())
		exec(new ColumnSetPlotDesignationCmd(m_column_private, pd));
}

/**
 * \brief Get width
 */
int Column::width() const
{
	return m_column_private->width();
}

/**
 * \brief Set width
 */
void Column::setWidth(int value)
{
	if (value != m_column_private->width())
		exec(new ColumnSetWidthCmd(m_column_private, value));
}

/**
 * \brief Clear the whole column
 */
void Column::clear()
{
	exec(new ColumnClearCmd(m_column_private));
}

/**
 * \brief Clear all validity information
 */
void Column::clearValidity()
{
	exec(new ColumnClearValidityCmd(m_column_private));
}

/**
 * \brief Clear all masking information
 */
void Column::clearMasks()
{
	exec(new ColumnClearMasksCmd(m_column_private));
}

/**
 * \brief Set an interval invalid or valid
 *
 * \param i the interval
 * \param invalid true: set invalid, false: set valid
 */ 
void Column::setInvalid(Interval<int> i, bool invalid)
{
	exec(new ColumnSetInvalidCmd(m_column_private, i, invalid));
}

/**
 * \brief Overloaded function for convenience
 */
void Column::setInvalid(int row, bool invalid)
{
	setInvalid(Interval<int>(row,row), invalid);
}

/**
 * \brief Set an interval masked
 *
 * \param i the interval
 * \param mask true: mask, false: unmask
 */ 
void Column::setMasked(Interval<int> i, bool mask)
{
	exec(new ColumnSetMaskedCmd(m_column_private, i, mask));
}

/**
 * \brief Overloaded function for convenience
 */
void Column::setMasked(int row, bool mask)
{
	setMasked(Interval<int>(row,row), mask);
}

////////////////////////////////////////////////////////////////////////////////
//@}
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
//! \name Formula related functions
//@{
////////////////////////////////////////////////////////////////////////////////

/**
 * \brief Set a formula string for an interval of rows
 */
void Column::setFormula(Interval<int> i, QString formula)
{
	exec(new ColumnSetFormulaCmd(m_column_private, i, formula));
}

/**
 * \brief Overloaded function for convenience
 */
void Column::setFormula(int row, QString formula)
{
	setFormula(Interval<int>(row, row), formula);
}

/**
 * \brief Clear all formulas
 */
void Column::clearFormulas()
{
	exec(new ColumnClearFormulasCmd(m_column_private));
}

////////////////////////////////////////////////////////////////////////////////
//@}
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
//! \name type specific functions
//@{
////////////////////////////////////////////////////////////////////////////////

/**
 * \brief Set the content of row 'row'
 *
 * Use this only when columnMode() is Text
 */
void Column::setTextAt(int row, const QString& new_value)
{
	exec(new ColumnSetTextCmd(m_column_private, row, new_value));
}

/**
 * \brief Replace a range of values 
 *
 * Use this only when columnMode() is Text
 */
void Column::replaceTexts(int first, const QStringList& new_values)
{
	if (!new_values.isEmpty())
		exec(new ColumnReplaceTextsCmd(m_column_private, first, new_values));
}

/**
 * \brief Set the content of row 'row'
 *
 * Use this only when columnMode() is DateTime, Month or Day
 */
void Column::setDateAt(int row, const QDate& new_value)
{
	setDateTimeAt(row, QDateTime(new_value, timeAt(row)));
}

/**
 * \brief Set the content of row 'row'
 *
 * Use this only when columnMode() is DateTime, Month or Day
 */
void Column::setTimeAt(int row,const QTime& new_value)
{
	setDateTimeAt(row, QDateTime(dateAt(row), new_value));
}

/**
 * \brief Set the content of row 'row'
 *
 * Use this only when columnMode() is DateTime, Month or Day
 */
void Column::setDateTimeAt(int row, const QDateTime& new_value)
{
	exec(new ColumnSetDateTimeCmd(m_column_private, row, new_value));
}

/**
 * \brief Replace a range of values 
 *
 * Use this only when columnMode() is DateTime, Month or Day
 */
void Column::replaceDateTimes(int first, const QList<QDateTime>& new_values)
{
	if (!new_values.isEmpty())
		exec(new ColumnReplaceDateTimesCmd(m_column_private, first, new_values));
}

/**
 * \brief Set the content of row 'row'
 *
 * Use this only when columnMode() is Numeric
 */
void Column::setValueAt(int row, double new_value)
{
	exec(new ColumnSetValueCmd(m_column_private, row, new_value));
}

/**
 * \brief Replace a range of values 
 *
 * Use this only when columnMode() is Numeric
 */
void Column::replaceValues(int first, const QVector<double>& new_values)
{
	if (!new_values.isEmpty())
		exec(new ColumnReplaceValuesCmd(m_column_private, first, new_values));
}

/**
 * \brief Return the content of row 'row'.
 *
 * Use this only when columnMode() is Text
 */
QString Column::textAt(int row) const
{
	return m_column_private->textAt(row);
}

/**
 * \brief Return the date part of row 'row'
 *
 * Use this only when columnMode() is DateTime, Month or Day
 */
QDate Column::dateAt(int row) const
{
	return m_column_private->dateAt(row);
}

/**
 * \brief Return the time part of row 'row'
 *
 * Use this only when columnMode() is DateTime, Month or Day
 */
QTime Column::timeAt(int row) const
{
	return m_column_private->timeAt(row);
}

/**
 * \brief Return the QDateTime in row 'row'
 *
 * Use this only when columnMode() is DateTime, Month or Day
 */
QDateTime Column::dateTimeAt(int row) const
{
	return m_column_private->dateTimeAt(row);
}

/**
 * \brief Return the double value in row 'row'
 */
double Column::valueAt(int row) const
{
	return m_column_private->valueAt(row);
}

////////////////////////////////////////////////////////////////////////////////
//@}
////////////////////////////////////////////////////////////////////////////////

/**
 * \brief Return an icon to be used for decorating the views and spreadsheet column headers
 */
QIcon Column::icon() const
{
	switch(columnMode()) {
		case SciDAVis::Numeric:
			return QIcon(QPixmap(":/numerictype.png"));
		case SciDAVis::Text:
			return QIcon(QPixmap(":/texttype.png"));
		case SciDAVis::DateTime:
		case SciDAVis::Month:
		case SciDAVis::Day:
			return QIcon(QPixmap(":/datetype.png"));
	}
	return QIcon();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//! \name serialize/deserialize
//@{
////////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * \brief Save the column as XML
 */
void Column::save(QXmlStreamWriter * writer) const
{
	writer->writeStartElement("column");
	writeBasicAttributes(writer);
	writer->writeAttribute("mode", SciDAVis::enumValueToString(columnMode(), "ColumnMode"));
	writer->writeAttribute("plot_designation", SciDAVis::enumValueToString(plotDesignation(), "PlotDesignation"));
	writer->writeAttribute("width", QString::number(width()));
	writeCommentElement(writer);
	writer->writeStartElement("input_filter");
	m_column_private->inputFilter()->save(writer);
	writer->writeEndElement();
	writer->writeStartElement("output_filter");
	m_column_private->outputFilter()->save(writer);
	writer->writeEndElement();
	QList< Interval<int> > masks = maskedIntervals();
	foreach(Interval<int> interval, masks)
	{
		writer->writeStartElement("mask");
		writer->writeAttribute("start_row", QString::number(interval.start()));
		writer->writeAttribute("end_row", QString::number(interval.end()));
		writer->writeEndElement();
	}
	QList< Interval<int> > formulas = formulaIntervals();
	foreach(Interval<int> interval, formulas)
	{
		writer->writeStartElement("formula");
		writer->writeAttribute("start_row", QString::number(interval.start()));
		writer->writeAttribute("end_row", QString::number(interval.end()));
		writer->writeCharacters(formula(interval.start()));
		writer->writeEndElement();
	}
	int i;
	switch(columnMode()) {
		case SciDAVis::Numeric:
			for(i=0; i<rowCount(); i++)
			{
				writer->writeStartElement("row");
				writer->writeAttribute("index", QString::number(i));
				writer->writeAttribute("invalid", isInvalid(i) ? "yes" : "no");
				writer->writeCharacters(QString::number(valueAt(i), 'e', 16));
				writer->writeEndElement();
			}
			break;
		case SciDAVis::Text:
			for(i=0; i<rowCount(); i++)
			{
				writer->writeStartElement("row");
				writer->writeAttribute("index", QString::number(i));
				writer->writeAttribute("invalid", isInvalid(i) ? "yes" : "no");
				writer->writeCharacters(textAt(i));
				writer->writeEndElement();
			}
			break;

		case SciDAVis::DateTime:
		case SciDAVis::Month:
		case SciDAVis::Day:
			for(i=0; i<rowCount(); i++)
			{
				writer->writeStartElement("row");
				writer->writeAttribute("index", QString::number(i));
				writer->writeAttribute("invalid", isInvalid(i) ? "yes" : "no");
#if QT_VERSION < 0x040400  // avoid a bug in Qt < 4.4
				QString str = dateTimeAt(i).toString("yyyy-dd-MM hh:mm:ss:zzz");
				int should_be_length = 4;
				int actual_length = dateTimeAt(i).toString("yyyy").length();
				while (actual_length < should_be_length)
				{
					str.prepend("0");
					actual_length++;
				}
				writer->writeCharacters(str);
#else
				writer->writeCharacters(dateTimeAt(i).toString("yyyy-dd-MM hh:mm:ss:zzz"));
#endif
				writer->writeEndElement();
			}
			break;
	}
	writer->writeEndElement(); // "column"
}

/**
 * \brief Load the column from XML
 */
bool Column::load(XmlStreamReader * reader)
{
	if(reader->isStartElement() && reader->name() == "column") 
	{
		if (!readBasicAttributes(reader)) return false;

		QXmlStreamAttributes attribs = reader->attributes();
		QString str;

		// read mode
		str = attribs.value(reader->namespaceUri().toString(), "mode").toString();
		if(str.isEmpty())
		{
			reader->raiseError(tr("column mode missing"));
			return false;
		}
		int mode_code = SciDAVis::enumStringToValue(str, "ColumnMode");
		if(mode_code == -1)
		{
			reader->raiseError(tr("column mode invalid"));
			return false;
		}
		setColumnMode((SciDAVis::ColumnMode)mode_code);
		// read plot designation
		str = attribs.value(reader->namespaceUri().toString(), "plot_designation").toString();
		int pd_code = SciDAVis::enumStringToValue(str, "PlotDesignation");
		if(str.isEmpty())
			setPlotDesignation(SciDAVis::noDesignation);
		else if(pd_code == -1)
		{
			reader->raiseError(tr("column plot designation invalid"));
			return false;
		}
		else
			setPlotDesignation((SciDAVis::PlotDesignation)pd_code);
		bool ok;
		int width = attribs.value(reader->namespaceUri().toString(), "width").toString().toInt(&ok);
		if (ok)
			setWidth(width);
		else {
			reader->raiseError(tr("missing or invalid column width"));
			return false;
		}

		setComment("");
		if (rowCount() > 0)
			removeRows(0, rowCount());		
		clearValidity();
		clearMasks();
		clearFormulas();
		// read child elements
		while (!reader->atEnd()) 
		{
			reader->readNext();

			if (reader->isEndElement()) break;

			if (reader->isStartElement()) 
			{
				bool ret_val = true;
				if (reader->name() == "comment")
					ret_val = readCommentElement(reader);
				else if(reader->name() == "input_filter")
					ret_val = XmlReadInputFilter(reader);
				else if(reader->name() == "output_filter")
					ret_val = XmlReadOutputFilter(reader);
				else if(reader->name() == "mask")
					ret_val = XmlReadMask(reader);
				else if(reader->name() == "formula")
					ret_val = XmlReadFormula(reader);
				else if(reader->name() == "row")
					ret_val = XmlReadRow(reader);
				else // unknown element
				{
					reader->raiseWarning(tr("unknown element '%1'").arg(reader->name().toString()));
					if (!reader->skipToEndElement()) return false;
				}
				if(!ret_val)
					return false;
			} 
		}
	}
	else // no column element
		reader->raiseError(tr("no column element found"));

	return !reader->error();
}
					
/**
 * \brief Read XML input filter element
 */
bool Column::XmlReadInputFilter(XmlStreamReader * reader)
{
	Q_ASSERT(reader->isStartElement() && reader->name() == "input_filter");
	if (!reader->skipToNextTag()) return false;
	if (!m_column_private->inputFilter()->load(reader)) return false;
	if (!reader->skipToNextTag()) return false;
	Q_ASSERT(reader->isEndElement() && reader->name() == "input_filter");
	return true;
}

/**
 * \brief Read XML output filter element
 */
bool Column::XmlReadOutputFilter(XmlStreamReader * reader)
{
	Q_ASSERT(reader->isStartElement() && reader->name() == "output_filter");
	if (!reader->skipToNextTag()) return false;
	if (!m_column_private->outputFilter()->load(reader)) return false;
	if (!reader->skipToNextTag()) return false;
	Q_ASSERT(reader->isEndElement() && reader->name() == "output_filter");
	return true;
}

/**
 * \brief Read XML mask element
 */
bool Column::XmlReadMask(XmlStreamReader * reader)
{
	Q_ASSERT(reader->isStartElement() && reader->name() == "mask");

	bool ok1, ok2;
	int start, end;
	start = reader->readAttributeInt("start_row", &ok1);
	end = reader->readAttributeInt("end_row", &ok2);
	if(!ok1 || !ok2) 
	{
		reader->raiseError(tr("invalid or missing start or end row"));
		return false;
	}
	setMasked(Interval<int>(start,end));
	if (!reader->skipToEndElement()) return false;

	return true;
}

/**
 * \brief Read XML formula element
 */
bool Column::XmlReadFormula(XmlStreamReader * reader)
{
	Q_ASSERT(reader->isStartElement() && reader->name() == "formula");

	bool ok1, ok2;
	int start, end;
	start = reader->readAttributeInt("start_row", &ok1);
	end = reader->readAttributeInt("end_row", &ok2);
	if(!ok1 || !ok2) 
	{
		reader->raiseError(tr("invalid or missing start or end row"));
		return false;
	}
	setFormula(Interval<int>(start,end), reader->readElementText());

	return true;
}

/**
 * \brief Read XML row element
 */
bool Column::XmlReadRow(XmlStreamReader * reader)
{
	Q_ASSERT(reader->isStartElement() && reader->name() == "row");
	
	QString str;

	QXmlStreamAttributes attribs = reader->attributes();
	
	bool ok;
	int index = reader->readAttributeInt("index", &ok);
	if(!ok) {
		reader->raiseError(tr("invalid or missing row index"));
		return false;
	}

	str = attribs.value(reader->namespaceUri().toString(), "invalid").toString();
	if(str == "yes") setInvalid(index);

	str = reader->readElementText();
	switch(columnMode()) {
		case SciDAVis::Numeric:
			{
				double value = str.toDouble(&ok);
				if(!ok) {
					reader->raiseError(tr("invalid row value"));
					return false;
				}
				setValueAt(index, value);
				break;
			}
		case SciDAVis::Text:
			setTextAt(index, str);
			break;

		case SciDAVis::DateTime:
		case SciDAVis::Month:
		case SciDAVis::Day:
			QDateTime date_time = QDateTime::fromString(str,"yyyy-dd-MM hh:mm:ss:zzz");
			setDateTimeAt(index, date_time);
			break;
	}

	return true;
}

////////////////////////////////////////////////////////////////////////////////
//@}
////////////////////////////////////////////////////////////////////////////////

/**
 * \brief Return whether the object is read-only
 */
bool Column::isReadOnly() const {
	return false;
}

/**
 * \brief Return the column mode
 *
 * This function is mostly used by spreadsheets but can also be used
 * by plots. The column mode specifies how to interpret 
 * the values in the column additional to the data type.
 */ 
SciDAVis::ColumnMode Column::columnMode() const 
{ 
	return m_column_private->columnMode(); 
}

/**
 * \brief Return the data vector size
 *
 * This returns the number of rows that actually contain data. 
 * Rows beyond this can be masked etc. but should be ignored by filters,
 * plots etc.
 */
int Column::rowCount() const 
{ 
	return m_column_private->rowCount(); 
}

/**
 * \brief Return the column plot designation
 */
SciDAVis::PlotDesignation Column::plotDesignation() const
{ 
	return m_column_private->plotDesignation(); 
}

AbstractSimpleFilter * Column::outputFilter() const 
{ 
	return m_column_private->outputFilter(); 
}

/**
 * \brief Return a wrapper column object used for String I/O.
 */
ColumnStringIO *Column::asStringColumn() const {
	return m_string_io;
}

////////////////////////////////////////////////////////////////////////////////
//! \name IntervalAttribute related functions
//@{
////////////////////////////////////////////////////////////////////////////////

/**
 * \brief Return whether a certain row contains an invalid value 	 
 */
bool Column::isInvalid(int row) const 
{ 
	return m_column_private->isInvalid(row); 
}

/**
 * \brief Return whether a certain interval of rows contains only invalid values 	 
 */
bool Column::isInvalid(Interval<int> i) const 
{ 
	return m_column_private->isInvalid(i); 
}

/**
 * \brief Return all intervals of invalid rows
 */
QList< Interval<int> > Column::invalidIntervals() const 
{ 
	return m_column_private->invalidIntervals(); 
}

/**
 * \brief Return whether a certain row is masked 	 
 */
bool Column::isMasked(int row) const 
{ 
	return m_column_private->isMasked(row); 
}

/**
 * \brief Return whether a certain interval of rows is fully masked 	 
 */
bool Column::isMasked(Interval<int> i) const 
{ 
	return m_column_private->isMasked(i); 
}

/**
 * \brief Return all intervals of masked rows
 */
QList< Interval<int> > Column::maskedIntervals() const 
{ 
	return m_column_private->maskedIntervals(); 
}

/**
 * \brief Return the formula associated with row 'row' 	 
 */
QString Column::formula(int row) const 
{ 
	return m_column_private->formula(row); 
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
QList< Interval<int> > Column::formulaIntervals() const 
{ 
	return m_column_private->formulaIntervals(); 
}

void Column::notifyDisplayChange()
{
	emit dataChanged(this); // all cells must be repainted
	emit aspectDescriptionChanged(this); // the icon for the type changed
}

/**
 * \class ColumnStringIO
 * \brief String-IO interface of Column.
 */
void ColumnStringIO::setTextAt(int row, const QString &value)
{
	m_setting = true;
	m_to_set = value;
	m_owner->copy(m_owner->m_column_private->inputFilter()->output(0), 0, row, 1);
	m_setting = false;
	m_to_set.clear();
}

QString ColumnStringIO::textAt(int row) const
{
	if (m_setting)
		return m_to_set;
	else
		return m_owner->m_column_private->outputFilter()->output(0)->textAt(row);
}
