/***************************************************************************
    File                 : Folder.cpp
    Project              : LabPlot
    Description          : Folder in a project
    --------------------------------------------------------------------
    Copyright            : (C) 2009-2015 Alexander Semke (alexander.semke@web.de)
    Copyright            : (C) 2007 Tilman Benkert (thzs@gmx.net)
    Copyright            : (C) 2007 Knut Franke (knut.franke@gmx.de)

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

#include "backend/core/Folder.h"
#include "backend/datapicker/Datapicker.h"
#include "backend/core/Project.h"
#include "backend/core/Workbook.h"
#include "backend/core/column/Column.h"
#include "backend/datasources/FileDataSource.h"
#include "backend/matrix/Matrix.h"
#include "backend/spreadsheet/Spreadsheet.h"
#include "backend/worksheet/Worksheet.h"

#include <KIcon>
#include <KLocale>

/**
 * \class Folder
 * \brief Folder in a project
 */

Folder::Folder(const QString &name) : AbstractAspect(name) {}

Folder::~Folder(){}

QIcon Folder::icon() const {
	return KIcon("folder");
}

/**
 * \brief Return a new context menu.
 *
 * The caller takes ownership of the menu.
 */
QMenu* Folder::createContextMenu() {
	if (project())
		return project()->createFolderContextMenu(this);
	return 0;
}

/**
 * \brief Save as XML
 */
void Folder::save(QXmlStreamWriter* writer) const {
	writer->writeStartElement("folder");
	writeBasicAttributes(writer);
	writeCommentElement(writer);

	foreach(AbstractAspect* child, children<AbstractAspect>(IncludeHidden)) {
		writer->writeStartElement("child_aspect");
		child->save(writer);
		writer->writeEndElement(); // "child_aspect"
	}
	writer->writeEndElement(); // "folder"
}

/**
 * \brief Load from XML
 */
bool Folder::load(XmlStreamReader* reader) {
	if(reader->isStartElement() && reader->name() == "folder") {
		setComment("");
		removeAllChildren();

		if (!readBasicAttributes(reader)) return false;

		// read child elements
		while (!reader->atEnd()) {
			reader->readNext();

			if (reader->isEndElement()) break;

			if (reader->isStartElement()) {
				if (reader->name() == "comment") {
					if (!readCommentElement(reader))
						return false;
				} else if(reader->name() == "child_aspect") {
					if (!readChildAspectElement(reader))
						return false;
				} else {// unknown element
					reader->raiseWarning(i18n("unknown element '%1'", reader->name().toString()));
					if (!reader->skipToEndElement()) return false;
				}
			}
		}
	}
	else // no folder element
		reader->raiseError(i18n("no folder element found"));

	return !reader->hasError();
}

/**
 * \brief Read child aspect from XML
 */
bool Folder::readChildAspectElement(XmlStreamReader* reader) {
	bool loaded = false;
	Q_ASSERT(reader->isStartElement() && reader->name() == "child_aspect");

	if (!reader->skipToNextTag()) return false;
	if (reader->isEndElement() && reader->name() == "child_aspect") return true; // empty element tag

	QString element_name = reader->name().toString();
	if (element_name == "folder") {
		Folder* folder = new Folder("");
		if (!folder->load(reader)) {
			delete folder;
			return false;
		}
		addChild(folder);
		loaded = true;
	} else if (element_name == "workbook") {
		Workbook* workbook = new Workbook(0, "");
		if (!workbook->load(reader)) {
			delete workbook;
			return false;
		}
		addChild(workbook);
		loaded = true;
	} else if (element_name == "spreadsheet") {
		Spreadsheet* spreadsheet = new Spreadsheet(0, "", true);
		if (!spreadsheet->load(reader)) {
			delete spreadsheet;
			return false;
		}
		addChild(spreadsheet);
		loaded = true;
	} else if (element_name == "matrix") {
		Matrix* matrix = new Matrix(0, "", true);
		if (!matrix->load(reader)) {
			delete matrix;
			return false;
		}
		addChild(matrix);
		loaded = true;
	} else if (element_name == "worksheet") {
		Worksheet* worksheet = new Worksheet(0, "");
		if (!worksheet->load(reader)){
			delete worksheet;
			return false;
		}
		addChild(worksheet);
		loaded = true;
	} else if (element_name == "fileDataSource") {
		FileDataSource* fileDataSource = new FileDataSource(0, "", true);
		if (!fileDataSource->load(reader)){
			delete fileDataSource;
			return false;
		}
		addChild(fileDataSource);
		loaded = true;
	} else if (element_name == "datapicker") {
		Datapicker* datapicker = new Datapicker(0, "", true);
		if (!datapicker->load(reader)){
			delete datapicker;
			return false;
		}
		addChild(datapicker);
		loaded = true;
	}

	if (!loaded) {
		reader->raiseWarning(i18n("unknown element '%1' found", element_name));
		if (!reader->skipToEndElement()) return false;
	}

	if (!reader->skipToNextTag()) return false;
	Q_ASSERT(reader->isEndElement() && reader->name() == "child_aspect");
	return !reader->hasError();
}
