/***************************************************************************
    File                 : Folder.cpp
    Project              : LabPlot
    Description          : Folder in a project
    --------------------------------------------------------------------
    Copyright            : (C) 2009-2013 Alexander Semke (alexander.semke@web.de)
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
#include "backend/core/Project.h"
#include "backend/lib/XmlStreamReader.h"
#include "backend/core/column/Column.h"
#include "backend/worksheet/Worksheet.h"
#include "backend/datasources/FileDataSource.h"
#include "backend/spreadsheet/Spreadsheet.h"

#include <QXmlStreamWriter>

#ifdef ACTIVATE_SCIDAVIS_SPECIFIC_CODE
#include <QIcon>
#include "core/plugin/PluginManager.h"
#else
#include <QIcon>
#include <klocalizedstring.h>
#endif

/**
 * \class Folder
 * \brief Folder in a project
 */

Folder::Folder(const QString &name) : AbstractAspect(name) {}

Folder::~Folder(){}

QIcon Folder::icon() const
{
	QIcon result;
#ifdef ACTIVATE_SCIDAVIS_SPECIFIC_CODE
	result.addFile(":/folder_closed.xpm", QSize(), QIcon::Normal, QIcon::Off);
	result.addFile(":/folder_open.xpm", QSize(), QIcon::Normal, QIcon::On);
#else
    result=QIcon("folder");
#endif
	return result;
}

/**
 * \brief Return a new context menu.
 *
 * The caller takes ownership of the menu.
 */
QMenu *Folder::createContextMenu()
{
	if (project())
		return project()->createFolderContextMenu(this);
	return 0;
}

////////////////////////////////////////////////////////////////////////////////
//! \name serialize/deserialize
//@{
////////////////////////////////////////////////////////////////////////////////

/**
 * \brief Save as XML
 */
void Folder::save(QXmlStreamWriter * writer) const
{
	writer->writeStartElement("folder");
	writeBasicAttributes(writer);
	writeCommentElement(writer);

	foreach(AbstractAspect * child, children<AbstractAspect>(IncludeHidden)) {
		writer->writeStartElement("child_aspect");
		child->save(writer);
		writer->writeEndElement(); // "child_aspect"
	}
	writer->writeEndElement(); // "folder"
}

/**
 * \brief Load from XML
 */
bool Folder::load(XmlStreamReader * reader)
{
	if(reader->isStartElement() && reader->name() == "folder")
	{
		setComment("");
		removeAllChildren();

		if (!readBasicAttributes(reader)) return false;

		// read child elements
		while (!reader->atEnd())
		{
			reader->readNext();

			if (reader->isEndElement()) break;

			if (reader->isStartElement())
			{
				if (reader->name() == "comment")
				{
					if (!readCommentElement(reader))
						return false;
				}
				else if(reader->name() == "child_aspect")
				{
					if (!readChildAspectElement(reader))
						return false;
				}
				else // unknown element
				{
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
bool Folder::readChildAspectElement(XmlStreamReader * reader)
{
	bool loaded = false;
	Q_ASSERT(reader->isStartElement() && reader->name() == "child_aspect");

	if (!reader->skipToNextTag()) return false;
	if (reader->isEndElement() && reader->name() == "child_aspect") return true; // empty element tag
	QString element_name = reader->name().toString();
	if (element_name == "folder")
	{
		Folder * folder = new Folder("");
		if (!folder->load(reader))
		{
			delete folder;
			return false;
		}
		addChild(folder);
		loaded = true;
	}
	else if (element_name == "column")
	{
		Column * column = new Column("", AbstractColumn::Text);
		if (!column->load(reader))
		{
			delete column;
			return false;
		}
		addChild(column);
		loaded = true;
	}
#ifdef ACTIVATE_SCIDAVIS_SPECIFIC_CODE
	else
	{
		foreach(QObject *plugin, PluginManager::plugins())
		{
			XmlElementAspectMaker * maker = qobject_cast<XmlElementAspectMaker *>(plugin);
			if (maker && maker->canCreate(element_name))
			{
				AbstractAspect * aspect = maker->createAspectFromXml(reader);
				if (aspect)
				{
					addChild(aspect);
					loaded = true;
					break;
				}
				else
				{
					reader->raiseError(i18n("creation of aspect from element '%1' failed", element_name));
					return false;
				}
			}
		}
	}
	if (!loaded)
	{
		reader->raiseWarning(i18n("no plugin to load element '%1' found", element_name));
		if (!reader->skipToEndElement()) return false;
	}
#else
	else if (element_name == "spreadsheet")
	{
		Spreadsheet * spreadsheet = new Spreadsheet(0, "");
		if (!spreadsheet->load(reader)){
			delete spreadsheet;
			return false;
		}
		addChild(spreadsheet);
		loaded = true;
	}else if (element_name == "worksheet"){
		Worksheet * worksheet = new Worksheet(0, "");
		if (!worksheet->load(reader)){
			delete worksheet;
			return false;
		}
		addChild(worksheet);
		loaded = true;
	} else if (element_name == "fileDataSource") {
		FileDataSource* fileDataSource = new FileDataSource(0, "");
		if (!fileDataSource->load(reader)){
			delete fileDataSource;
			return false;
		}
		addChild(fileDataSource);
		loaded = true;
	}

	if (!loaded)
	{
		reader->raiseWarning(i18n("unknown element '%1' found", element_name));
		if (!reader->skipToEndElement()) return false;
	}
#endif
	if (!reader->skipToNextTag()) return false;
	Q_ASSERT(reader->isEndElement() && reader->name() == "child_aspect");
	return !reader->hasError();
}

////////////////////////////////////////////////////////////////////////////////
//@}
////////////////////////////////////////////////////////////////////////////////
