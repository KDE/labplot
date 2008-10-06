/***************************************************************************
    File                 : DateTime2StringFilter.cpp
    Project              : SciDAVis
    --------------------------------------------------------------------
    Copyright            : (C) 2007 by Tilman Benkert,
                           Knut Franke
    Email (use @ for *)  : thzs*gmx.net, knut.franke*gmx.de
    Description          : Conversion filter QDateTime -> QString.
                           
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

#include "DateTime2StringFilter.h"
#include "lib/XmlStreamReader.h"
#include <QXmlStreamWriter>

void DateTime2StringFilter::setFormat(const QString& format) 
{ 
	exec(new DateTime2StringFilterSetFormatCmd(static_cast<DateTime2StringFilter*>(this), format));
}

DateTime2StringFilterSetFormatCmd::DateTime2StringFilterSetFormatCmd(DateTime2StringFilter* target, const QString &new_format)
	: m_target(target), m_other_format(new_format) 
{
	if(m_target->parentAspect())
		setText(QObject::tr("%1: set date-time format to %2").arg(m_target->parentAspect()->name()).arg(new_format));
	else
		setText(QObject::tr("set date-time format to %1").arg(new_format));
}

void DateTime2StringFilterSetFormatCmd::redo() 
{
	QString tmp = m_target->m_format;
	m_target->m_format = m_other_format;
	m_other_format = tmp;
	emit m_target->formatChanged();
}

void DateTime2StringFilterSetFormatCmd::undo() 
{ 
	redo(); 
}

void DateTime2StringFilter::writeExtraAttributes(QXmlStreamWriter * writer) const
{
	writer->writeAttribute("format", format());
}

bool DateTime2StringFilter::load(XmlStreamReader * reader)
{
	QXmlStreamAttributes attribs = reader->attributes();
	QString str = attribs.value(reader->namespaceUri().toString(), "format").toString();

	if (AbstractSimpleFilter::load(reader))
		setFormat(str);
	else
		return false;

	return !reader->hasError();
}

