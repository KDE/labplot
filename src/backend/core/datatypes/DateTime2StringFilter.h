/*
    File                 : DateTime2StringFilter.h
    Project              : LabPlot
    Description          : Conversion filter QDateTime -> QString.
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2007 Tilman Benkert <thzs@gmx.net>
    SPDX-FileCopyrightText: 2007 Knut Franke <knut.franke@gmx.de>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef DATE_TIME2STRING_FILTER_H
#define DATE_TIME2STRING_FILTER_H

#include "backend/core/AbstractSimpleFilter.h"

class DateTime2StringFilterSetFormatCmd;

//! Conversion filter QDateTime -> QString.
class DateTime2StringFilter : public AbstractSimpleFilter {
	Q_OBJECT

public:
	//! Standard constructor.
	explicit DateTime2StringFilter(const QString& format = QLatin1String("yyyy-MM-dd hh:mm:ss.zzz")) : m_format(format) {}
	//! Set the format string to be used for conversion.
	void setFormat(const QString& format);

	//! Return the format string
	/**
		* The default format string is "yyyy-MM-dd hh:mm:ss.zzz".
	* \sa QDate::toString()
		*/
	QString format() const { return m_format; }

	//! Return the data type of the column
	AbstractColumn::ColumnMode columnMode() const override { return AbstractColumn::ColumnMode::Text; }

private:
	friend class DateTime2StringFilterSetFormatCmd;
	//! The format string.
	QString m_format;

public:
	QString textAt(int row) const override;

	//! \name XML related functions
	//@{
	void writeExtraAttributes(QXmlStreamWriter*) const override;
	bool load(XmlStreamReader*, bool preview) override;
	//@}

protected:
	//! Using typed ports: only DateTime inputs are accepted.
	bool inputAcceptable(int, const AbstractColumn *source) override;
};

#endif // ifndef DATE_TIME2STRING_FILTER_H

