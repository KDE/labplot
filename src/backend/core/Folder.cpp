/***************************************************************************
    File                 : Folder.cpp
    Project              : SciDAVis
    Description          : Folder in a project
    --------------------------------------------------------------------
    Copyright            : (C) 2007 Tilman Benkert (thzs*gmx.net)
    Copyright            : (C) 2007 Knut Franke (knut.franke*gmx.de)
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
#include "core/Project.h"
#include "core/Folder.h"
#include "lib/XmlStreamReader.h"
#include "core/column/Column.h"

#include <QIcon>
#include <QApplication>
#include <QStyle>
#include <QXmlStreamWriter>
#ifdef ACTIVATE_SCIDAVIS_SPECIFIC_CODE
#include <QPluginLoader>
#else
#include "table/Table.h"
#include <klocalizedstring.h>
#endif
#include <QtDebug>

Folder::Folder(const QString &name)
	: AbstractAspect(name)
{
}

Folder::~Folder()
{
}

QIcon Folder::icon() const
{
	QIcon result;
	result.addFile(":/folder_closed.xpm", QSize(), QIcon::Normal, QIcon::Off);
	result.addFile(":/folder_open.xpm", QSize(), QIcon::Normal, QIcon::On);	
	return result;
}

QMenu *Folder::createContextMenu()
{
	if (project())
		return project()->createFolderContextMenu(this);
	return 0;
}

void Folder::save(QXmlStreamWriter * writer) const
{
	writer->writeStartElement("folder");
	writeBasicAttributes(writer);
	writeCommentElement(writer);

	int child_count = childCount();
	for (int i=0; i<child_count; i++)
	{
		writer->writeStartElement("child_aspect");
		child(i)->save(writer);
		writer->writeEndElement(); // "child_aspect"
	}
	writer->writeEndElement(); // "folder"
}

bool Folder::load(XmlStreamReader * reader)
{
	if(reader->isStartElement() && reader->name() == "folder") 
	{
		setComment("");
		removeAllChildAspects();

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
					reader->raiseWarning(tr("unknown element '%1'").arg(reader->name().toString()));
					if (!reader->skipToEndElement()) return false;
				}
			} 
		}
	}
	else // no folder element
		reader->raiseError(tr("no folder element found"));

	return !reader->hasError();
}

bool Folder::readChildAspectElement(XmlStreamReader * reader)
{
	bool loaded = false;
	Q_ASSERT(reader->isStartElement() && reader->name() == "child_aspect");

	if (!reader->skipToNextTag()) return false;
	if (reader->isEndElement() && reader->name() == "child_aspect") return true; // empty element tag
	QString element_name = reader->name().toString();
	if (element_name == "folder")
	{
		Folder * folder = new Folder(tr("Folder %1").arg(1));
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
		Column * column = new Column(tr("Column %1").arg(1), SciDAVis::Text);
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
		foreach(QObject * plugin, QPluginLoader::staticInstances()) 
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
					reader->raiseError(tr("creation of aspect from element '%1' failed").arg(element_name));
					return false;
				}
			}
		}
	}
	if (!loaded)
	{
		reader->raiseWarning(tr("no plugin to load element '%1' found").arg(element_name));
		if (!reader->skipToEndElement()) return false;
	}
#else
	else if (element_name == "table")
	{
		Table * table = new Table(0, 0, 0, tr("Table %1").arg(1));
		if (!table->load(reader))
		{
			delete table;
			return false;
		}
		addChild(table);
		loaded = true;
	}
	if (!loaded)
	{
		reader->raiseWarning(i18n("unknown element '%1' found").arg(element_name));
		if (!reader->skipToEndElement()) return false;
	}
#endif
	if (!reader->skipToNextTag()) return false;
	Q_ASSERT(reader->isEndElement() && reader->name() == "child_aspect");
	return !reader->hasError();
}

