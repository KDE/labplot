/*
    File                 : Double2StringFilter.cpp
    Project              : AbstractColumn
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2007 Knut Franke Tilman Benkert
    Email (use @ for *)  : knut.franke*gmx.de, thzs@gmx.net
    Description          : Locale-aware conversion filter double -> QString.

*/

/***************************************************************************
 *                                                                         *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *                                                                         *
 ***************************************************************************/

#include "Double2StringFilter.h"
#include "backend/lib/XmlStreamReader.h"
#include <QXmlStreamWriter>
#include <QUndoCommand>

#include <KLocalizedString>

class Double2StringFilterSetFormatCmd : public QUndoCommand {
public:
	Double2StringFilterSetFormatCmd(Double2StringFilter* target, char new_format);

	void redo() override;
	void undo() override;

private:
	Double2StringFilter* m_target;
	char m_other_format;
};

class Double2StringFilterSetDigitsCmd : public QUndoCommand {
public:
	Double2StringFilterSetDigitsCmd(Double2StringFilter* target, int new_digits);

	void redo() override;
	void undo() override;

private:
	Double2StringFilter* m_target;
	int m_other_digits;
};

void Double2StringFilter::writeExtraAttributes(QXmlStreamWriter * writer) const {
	writer->writeAttribute("format", QString(QChar(numericFormat())));
	writer->writeAttribute("digits", QString::number(numDigits()));
}

bool Double2StringFilter::load(XmlStreamReader* reader, bool preview) {
	if (preview)
		return true;

	QXmlStreamAttributes attribs = reader->attributes();
	QString format_str = attribs.value(reader->namespaceUri().toString(), "format").toString();
	QString digits_str = attribs.value(reader->namespaceUri().toString(), "digits").toString();

	if (AbstractSimpleFilter::load(reader, preview)) {
		bool ok;
		int digits = digits_str.toInt(&ok);
		if (ok && m_digits != digits)
			setNumDigits(digits);

		if (format_str.size() >= 1 && m_format != format_str)
			setNumericFormat(format_str.at(0).toLatin1());
	} else
		return false;

	return !reader->hasError();
}

void Double2StringFilter::setNumericFormat(char format) {
	exec(new Double2StringFilterSetFormatCmd(this, format));
}

void Double2StringFilter::setNumDigits(int digits) {
	exec(new Double2StringFilterSetDigitsCmd(this, digits));
}

Double2StringFilterSetFormatCmd::Double2StringFilterSetFormatCmd(Double2StringFilter* target, char new_format)
	: m_target(target), m_other_format(new_format) {
	if (m_target->parentAspect())
		setText(i18n("%1: set numeric format to '%2'", m_target->parentAspect()->name(), new_format));
	else
		setText(i18n("set numeric format to '%1'", new_format));
}

void Double2StringFilterSetFormatCmd::redo() {
	char tmp = m_target->m_format;
	m_target->m_format = m_other_format;
	m_other_format = tmp;
	emit m_target->formatChanged();
}

void Double2StringFilterSetFormatCmd::undo() {
	redo();
}

Double2StringFilterSetDigitsCmd::Double2StringFilterSetDigitsCmd(Double2StringFilter* target, int new_digits)
	: m_target(target), m_other_digits(new_digits) {
	if (m_target->parentAspect())
		setText(i18n("%1: set decimal digits to %2", m_target->parentAspect()->name(), new_digits));
	else
		setText(i18n("set decimal digits to %1", new_digits));
}

void Double2StringFilterSetDigitsCmd::redo() {
	int tmp = m_target->m_digits;
	m_target->m_digits = m_other_digits;
	m_other_digits = tmp;
	emit m_target->digitsChanged();
}

void Double2StringFilterSetDigitsCmd::undo() {
	redo();
}
