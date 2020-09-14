/***************************************************************************
    File                 : DateTime2StringFilter.cpp
    Project              : LabPlot
    --------------------------------------------------------------------
    Copyright            : (C) 2007 by Tilman Benkert (thzs@gmx.net)
    Copyright            : (C) 2007 by Knut Franke (knut.franke@gmx.de)
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
#include "backend/lib/XmlStreamReader.h"
#include <QDateTime>
#include <QUndoCommand>

#include <KLocalizedString>

class DateTime2StringFilterSetFormatCmd : public QUndoCommand {
public:
	DateTime2StringFilterSetFormatCmd(DateTime2StringFilter* target, const QString &new_format);

	void redo() override;
	void undo() override;

private:
	DateTime2StringFilter* m_target;
	QString m_other_format;
};

void DateTime2StringFilter::setFormat(const QString& format) {
	if (m_format != format)
		exec(new DateTime2StringFilterSetFormatCmd(this, format));
}

QString DateTime2StringFilter::textAt(int row) const {
	//DEBUG("DateTime2StringFilter::textAt()");
	if (!m_inputs.value(0)) return QString();
	QDateTime inputValue = m_inputs.value(0)->dateTimeAt(row);
	if (!inputValue.isValid()) return QString();

	//QDEBUG("	: " << inputValue << " -> " << m_format << " -> " << inputValue.toString(m_format));
	return inputValue.toString(m_format);
}

bool DateTime2StringFilter::inputAcceptable(int, const AbstractColumn *source) {
	auto mode = source->columnMode();
	return (mode == AbstractColumn::ColumnMode::DateTime)
		|| (mode == AbstractColumn::ColumnMode::Day)
		|| (mode == AbstractColumn::ColumnMode::Month);
}

DateTime2StringFilterSetFormatCmd::DateTime2StringFilterSetFormatCmd(DateTime2StringFilter* target, const QString &new_format)
	: m_target(target), m_other_format(new_format)
{
	if (m_target->parentAspect())
		setText(i18n("%1: set date-time format to %2", m_target->parentAspect()->name(), new_format));
	else
		setText(i18n("set date-time format to %1", new_format));
}

void DateTime2StringFilterSetFormatCmd::redo() {
	QString tmp = m_target->m_format;
	m_target->m_format = m_other_format;
	m_other_format = tmp;
	emit m_target->formatChanged();
}

void DateTime2StringFilterSetFormatCmd::undo() {
	redo();
}

void DateTime2StringFilter::writeExtraAttributes(QXmlStreamWriter * writer) const {
	writer->writeAttribute("format", format());
}

bool DateTime2StringFilter::load(XmlStreamReader* reader, bool preview) {
	if (preview)
		return true;

	QXmlStreamAttributes attribs = reader->attributes();
	QString str = attribs.value(reader->namespaceUri().toString(), "format").toString();

	if (AbstractSimpleFilter::load(reader, preview))
		setFormat(str);
	else
		return false;

	return !reader->hasError();
}

