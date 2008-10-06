/***************************************************************************
    File                 : Project.cpp
    Project              : SciDAVis
    Description          : Represents a SciDAVis project.
    --------------------------------------------------------------------
    Copyright            : (C) 2007-2008 Tilman Benkert (thzs*gmx.net)
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
#include "core/ScriptingEngineManager.h"
#include "core/interfaces.h"
#include "core/globals.h"
#include "lib/XmlStreamReader.h"
#ifdef ACTIVATE_SCIDAVIS_SPECIFIC_CODE
#include "core/ProjectWindow.h"
#include "core/ProjectConfigPage.h"
#else
#include "MainWin.h"
#endif
#include <QUndoStack>
#include <QString>
#include <QKeySequence>
#include <QMenu>
#include <QSettings>
#include <QPluginLoader>
#include <QComboBox>
#include <QFile>
#include <QXmlStreamWriter>
#include <QDateTime>
#include <QtDebug>

#define NOT_IMPL (QMessageBox::information(0, "info", "not yet implemented"))

class Project::Private
{
	public:
#ifdef ACTIVATE_SCIDAVIS_SPECIFIC_CODE
		Private() :
#else
		Private(MainWin * mainWin) :
#endif
			mdi_window_visibility(static_cast<MdiWindowVisibility>(Project::global("default_mdi_window_visibility").toInt())),
#ifdef ACTIVATE_SCIDAVIS_SPECIFIC_CODE
			primary_view(0),
#else
			primary_view(mainWin),
#endif
			scripting_engine(0),
		 	file_name(QString()),

#ifdef ACTIVATE_SCIDAVIS_SPECIFIC_CODE
			version(SciDAVis::version()),
#else
			version(0),
			labPlot(QString(LVERSION)),
#endif
			author(QString()),
			modification_time(QDateTime::currentDateTime()),
			changed(false)
			{}
		~Private() {
#ifdef ACTIVATE_SCIDAVIS_SPECIFIC_CODE // in LabPlot, the MainWin deletes the Project
			delete static_cast<ProjectWindow *>(primary_view);
#endif
		}
		QUndoStack undo_stack;
		MdiWindowVisibility mdi_window_visibility;
		QWidget * primary_view;
		AbstractScriptingEngine * scripting_engine;
		QString file_name;
		int version;
#ifndef ACTIVATE_SCIDAVIS_SPECIFIC_CODE
		QString labPlot;
#endif
		QString  author;
		QDateTime modification_time;
		bool changed;
};

#ifdef ACTIVATE_SCIDAVIS_SPECIFIC_CODE
Project::Project()
	: Folder(tr("Unnamed")), d(new Private())
#else
Project::Project(MainWin * mainWin)
	: Folder(tr("Unnamed")), d(new Private(mainWin))
#endif
{
#ifndef SUPPRESS_SCRIPTING_INIT
	// TODO: intelligent engine choosing
	Q_ASSERT(ScriptingEngineManager::instance()->engineNames().size() > 0);
	QString engine_name = ScriptingEngineManager::instance()->engineNames()[0];
	d->scripting_engine = ScriptingEngineManager::instance()->engine(engine_name);
#endif
}

Project::~Project()
{
	delete d;
}

QUndoStack *Project::undoStack() const
{
	return &d->undo_stack;
}

QWidget *Project::view()
{
#ifdef ACTIVATE_SCIDAVIS_SPECIFIC_CODE
	if (!d->primary_view)
		d->primary_view = new ProjectWindow(this);
#endif
	return d->primary_view;
}

QMenu *Project::createContextMenu()
{
#ifdef ACTIVATE_SCIDAVIS_SPECIFIC_CODE
	QMenu * menu = new QMenu(); // no remove action from AbstractAspect in the project context menu
	emit requestProjectContextMenu(menu);
	return menu;
#else
	return NULL;
#endif
}

QMenu *Project::createFolderContextMenu(const Folder * folder)
{
#ifdef ACTIVATE_SCIDAVIS_SPECIFIC_CODE
	QMenu * menu = const_cast<Folder *>(folder)->AbstractAspect::createContextMenu();
	Q_ASSERT(menu);
	emit requestFolderContextMenu(folder, menu);
	return menu;
#else
	return NULL;
#endif
}

void Project::setMdiWindowVisibility(MdiWindowVisibility visibility)
{ 
	d->mdi_window_visibility = visibility; 
	emit mdiWindowVisibilityChanged();
}
		
Project::MdiWindowVisibility Project::mdiWindowVisibility() const 
{ 
	return d->mdi_window_visibility; 
}

AbstractScriptingEngine * Project::scriptingEngine() const
{
	return d->scripting_engine;
}

/* ================== static methods ======================= */
#ifdef ACTIVATE_SCIDAVIS_SPECIFIC_CODE
ConfigPageWidget * Project::makeConfigPage()
{
	 return new ProjectConfigPage();
}

QString Project::configPageLabel()
{
	return QObject::tr("General");
}
#endif

CLASS_D_ACCESSOR_IMPL(Project, QString, fileName, FileName, file_name)
BASIC_D_ACCESSOR_IMPL(Project, int, version, Version, version)
#ifndef ACTIVATE_SCIDAVIS_SPECIFIC_CODE
CLASS_D_ACCESSOR_IMPL(Project, QString, labPlot, LabPlot, labPlot)
#endif
// TODO: add support for these in the SciDAVis UI
CLASS_D_ACCESSOR_IMPL(Project, QString, author, Author, author)  
CLASS_D_ACCESSOR_IMPL(Project, QDateTime, modificationTime, ModificationTime, modification_time)
FLAG_D_ACCESSOR_IMPL(Project, Changed, changed)

void Project::save(QXmlStreamWriter * writer) const
{
	writer->writeStartDocument();
#ifdef ACTIVATE_SCIDAVIS_SPECIFIC_CODE
	writer->writeDTD("<!DOCTYPE SciDAVisProject>");
#else
	writer->writeDTD("<!DOCTYPE LabPlotXML>");
#endif

#ifdef ACTIVATE_SCIDAVIS_SPECIFIC_CODE
	writer->writeStartElement("scidavis_project");
#else
	writer->writeStartElement("labplot_project");
#endif

	writer->writeAttribute("version", QString::number(version()));
	writer->writeAttribute("file_name", fileName());
	writer->writeAttribute("modification_time" , modificationTime().toString("yyyy-dd-MM hh:mm:ss:zzz"));
	writer->writeAttribute("author", author());
#ifndef ACTIVATE_SCIDAVIS_SPECIFIC_CODE
	writer->writeAttribute("LabPlot" , labPlot());
#endif
	writeBasicAttributes(writer);

	writeCommentElement(writer);

	int child_count = childCount();
	for (int i=0; i<child_count; i++)
	{
		writer->writeStartElement("child_aspect");
		child(i)->save(writer);
		writer->writeEndElement();
	}

	writer->writeEndElement();
	writer->writeEndDocument();
}

bool Project::load(XmlStreamReader * reader)
{
	while (!(reader->isStartDocument() || reader->atEnd()))
		reader->readNext();
	if(!(reader->atEnd()))
	{
		if (!reader->skipToNextTag()) return false;

#ifdef ACTIVATE_SCIDAVIS_SPECIFIC_CODE
		if (reader->name() == "scidavis_project") 
#else
		if (reader->name() == "labplot_project") 
#endif
		{
			setComment("");
			removeAllChildAspects();

			bool ok;
			int version = reader->readAttributeInt("version", &ok);
			if(!ok) 
			{
				reader->raiseError(tr("invalid or missing project version"));
				return false;
			}
			setVersion(version);
			// version dependent staff goes here
			
			if (!readBasicAttributes(reader)) return false;
			if (!readProjectAttributes(reader)) return false;
			
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
		else // no project element
#ifdef ACTIVATE_SCIDAVIS_SPECIFIC_CODE
			reader->raiseError(tr("no scidavis_project element found"));
#else
			reader->raiseError(tr("no labplot_project element found"));
#endif
	}
	else // no start document
		reader->raiseError(tr("no valid XML document found"));

	return !reader->hasError();
}

bool Project::readProjectAttributes(XmlStreamReader * reader)
{
	QString prefix(tr("XML read error: ","prefix for XML error messages"));
	QString postfix(tr(" (loading failed)", "postfix for XML error messages"));

	QXmlStreamAttributes attribs = reader->attributes();
	QString str;

	str = attribs.value(reader->namespaceUri().toString(), "file_name").toString();
	if(str.isEmpty())
	{
		reader->raiseError(prefix+tr("project file name missing")+postfix);
		return false;
	}
	setFileName(str);

	str = attribs.value(reader->namespaceUri().toString(), "modification_time").toString();
	QDateTime modification_time = QDateTime::fromString(str, "yyyy-dd-MM hh:mm:ss:zzz");
	if(str.isEmpty() || !modification_time.isValid())
	{
		reader->raiseWarning(tr("Invalid project modification time. Using current time."));
		setModificationTime(QDateTime::currentDateTime());
	}
	else
		setModificationTime(modification_time);

	str = attribs.value(reader->namespaceUri().toString(), "author").toString();
	if(str.isEmpty())
	{
		reader->raiseError(prefix+tr("author attribute missing")+postfix);
		return false;
	}
	setAuthor(str);

#ifndef ACTIVATE_SCIDAVIS_SPECIFIC_CODE
	str = attribs.value(reader->namespaceUri().toString(), "LabPlot").toString();
	if(str.isEmpty())
	{
		reader->raiseError(prefix+tr("LabPlot attribute missing")+postfix);
		return false;
	}
	setLabPlot(str);
#endif

	return true;
}

void Project::staticInit()
{
	// defaults for global settings
	Project::setGlobalDefault("default_mdi_window_visibility", Project::folderOnly);
	Project::setGlobalDefault("auto_save", true);
	Project::setGlobalDefault("auto_save_interval", 15);
	Project::setGlobalDefault("default_scripting_language", QString("muParser"));
	// TODO: not really Project-specific; maybe put these somewhere else:
	Project::setGlobalDefault("language", QString("en"));
	Project::setGlobalDefault("auto_search_updates", false);
	Project::setGlobalDefault("locale_use_group_separator", false);
}

