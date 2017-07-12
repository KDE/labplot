/***************************************************************************
File                 : AbstractFileFilter.h
Project              : LabPlot
Description          : file I/O-filter related interface
--------------------------------------------------------------------
Copyright            : (C) 2009-2017 Alexander Semke (alexander.semke@web.de)
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

#include "backend/datasources/filters/AbstractFileFilter.h"
#include <QDateTime>
#include <QLocale>
#include <KLocale>

AbstractColumn::ColumnMode AbstractFileFilter::columnMode(const QString& valueString, const QString& dateTimeFormat, Locale locale) {
	AbstractColumn::ColumnMode mode = AbstractColumn::Numeric;

	//try to convert to a number first
	bool isNumber;
	if (locale == LocaleC) {
		QLocale l(QLocale::C);
		l.toDouble(valueString, &isNumber);
	} else {	//TODO: why German? Should be QLocale::system()
		QLocale l(QLocale::German);
		l.toDouble(valueString, &isNumber);
	}

	//if not a number, check datetime and string
	if (!isNumber) {
		QDateTime valueDateTime = QDateTime::fromString(valueString, dateTimeFormat);
		if (valueDateTime.isValid())
			mode = AbstractColumn::DateTime;
		else
			mode = AbstractColumn::Text;
	}

	return mode;
}

/*
returns the list of all supported locales for numeric data
*/
QStringList AbstractFileFilter::numberFormats() {
	return (QStringList() << i18n("System locale") << i18n("C format"));
}
