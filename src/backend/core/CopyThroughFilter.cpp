/*
	File                 : CopyThroughFilter.cpp
	Project              : SciDAVis
	Description          : Filter which copies all provided inputs unaltered
	to an equal number of outputs.
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2007 Knut Franke <knut.franke*gmx.de (use @ for *)>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "CopyThroughFilter.h"
#include "AbstractColumn.h"
#include "backend/lib/XmlStreamReader.h"

/**
 * \class CopyThroughFilter
 * \brief Filter which copies all provided inputs unaltered to an equal number of outputs.
 *
 * This is probably the simplest filter you can possibly write.
 * It accepts an arbitrary number of inputs and provides the same AbstractColumn objects
 * as outputs again.
 */

/**
 * \brief Accept any number of inputs.
 */
int CopyThroughFilter::inputCount() const {
	return -1;
}

/**
 * \brief Provide as many output ports as inputs have been connected.
 */
int CopyThroughFilter::outputCount() const {
	return m_inputs.size();
}

void CopyThroughFilter::save(QXmlStreamWriter* writer) const {
	writer->writeStartElement(QStringLiteral("copy_through_filter"));
	writeBasicAttributes(writer);
	writeCommentElement(writer);
	writer->writeEndElement();
}

bool CopyThroughFilter::load(XmlStreamReader* reader, bool /*preview*/) {
	if (!readBasicAttributes(reader))
		return false;

	while (!reader->atEnd()) {
		reader->readNext();
		if (reader->isEndElement())
			break;
		if (reader->isStartElement()) {
			if (reader->name() == QLatin1String("comment")) {
				if (!readCommentElement(reader))
					return false;
			} else if (!reader->skipToEndElement())
				return false;
		}
	}

	return !reader->hasError();
}

/**
 * \brief When asked for an output port, just return the corresponding input port.
 */
AbstractColumn* CopyThroughFilter::output(int port) {
	return const_cast<AbstractColumn*>(m_inputs.value(port));
}

AbstractColumn* CopyThroughFilter::output(int port) const {
	return const_cast<AbstractColumn*>(m_inputs.value(port));
}
