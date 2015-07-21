/***************************************************************************
    File                 : XmlAttributeReader.h
    Project              : LabPlot
    Description          : Xml Attribute Reader class
    --------------------------------------------------------------------
    Copyright            : (C) 2015 by Minh Ngo (minh@fedoraproject.org)

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

#ifndef XMLATTRIBUTEREADER_H
#define XMLATTRIBUTEREADER_H

#include "backend/lib/XmlStreamReader.h"

#include <QDebug>
#include <QColor>
#include <QXmlStreamWriter>

#include <KUrl>
#include <KLocale>

namespace AttributeReaderHelper{
	template<class TAttribute>
	inline TAttribute convertQStringToAttributeType(const QString& str) {
		return static_cast<TAttribute>(str.toInt());
	}

	template<>
	inline int convertQStringToAttributeType<int>(const QString& str) {
		return str.toInt();
	}

	template<>
	inline bool convertQStringToAttributeType<bool>(const QString& str) {
		return str.toInt() == 1;
	}

	template<>
	inline QColor convertQStringToAttributeType<QColor>(const QString& str) {
		return QColor(str);
	}

	template<>
	inline KUrl convertQStringToAttributeType<KUrl>(const QString& str) {
		return KUrl(str);
	}
}

class XmlAttributeReader{
	public:
		XmlAttributeReader(XmlStreamReader* reader, const QXmlStreamAttributes& attribs)
			: reader(reader)
			, attribs(attribs) {
		}

		template<class TAttribute>
		void checkAndLoadAttribute(const QString& attributeName, TAttribute& result) {
			const QString attributeWarning = i18n("Attribute '%1' missing or empty, default value is used");

			const QString& attr = attribs.value(attributeName).toString();
			if(attr.isEmpty()) {
				reader->raiseWarning(attributeWarning.arg(attributeName));
			} else {
				result = AttributeReaderHelper::convertQStringToAttributeType<TAttribute>(attr);
			}
		}
	
	private:
		XmlStreamReader* const reader;
		const QXmlStreamAttributes& attribs;
};

#endif