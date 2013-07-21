/***************************************************************************
    File                 : Project.cpp
    Project              : SciDAVis
    Description          : Represents a SciDAVis project.
    --------------------------------------------------------------------
    Copyright            : (C) 2011-2013 Alexander Semke (alexander.semke*web.de)
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
#include "backend/core/Project.h"
#include "backend/lib/XmlStreamReader.h"
#include "backend/spreadsheet/Spreadsheet.h"
#include "backend/worksheet/plots/cartesian/XYCurve.h"

#include <QUndoStack>
#include <QMenu>
#include <QDateTime>

/**
 * \class Project
 * \brief Represents a SciDAVis project.
 *  \ingroup core
 * Project manages an undo stack and is responsible for creating ProjectWindow instances
 * as views on itself.
 */

/**
 * \enum Project::MdiWindowVisibility
 * \brief MDI subwindow visibility setting
 */
/**
 * \var Project::folderOnly
 * \brief only show MDI windows corresponding to Parts in the current folder
 */
/**
 * \var Project::foldAndSubfolders
 * \brief show MDI windows corresponding to Parts in the current folder and its subfolders
 */
/**
 * \var Project::allMdiWindows
 * \brief show MDI windows for all Parts in the project simultaneously
 */

class Project::Private {
	public:
		Private() :
			mdiWindowVisibility(Project::folderOnly),
			scriptingEngine(0),
			version(LVERSION),
			author(QString(getenv("USER"))),
			modificationTime(QDateTime::currentDateTime()),
			changed(false)
			{}

		QUndoStack undo_stack;
		MdiWindowVisibility mdiWindowVisibility;
		AbstractScriptingEngine* scriptingEngine;
		QString fileName;
		QString version;
		QString author;
		QDateTime modificationTime;
		bool changed;
};

Project::Project() : Folder(tr("Project")), d(new Private()) {
#ifndef SUPPRESS_SCRIPTING_INIT
	// TODO: intelligent engine choosing
	Q_ASSERT(ScriptingEngineManager::instance()->engineNames().size() > 0);
	QString engine_name = ScriptingEngineManager::instance()->engineNames()[0];
	d->scriptingEngine = ScriptingEngineManager::instance()->engine(engine_name);
#endif
}

Project::~Project() {
	delete d;
}

QUndoStack* Project::undoStack() const {
	return &d->undo_stack;
}

QMenu* Project::createContextMenu() {
	QMenu* menu = new QMenu(); // no remove action from AbstractAspect in the project context menu
	emit requestProjectContextMenu(menu);
	return menu;
}

QMenu* Project::createFolderContextMenu(const Folder* folder) {
	QMenu* menu = const_cast<Folder*>(folder)->AbstractAspect::createContextMenu();
	Q_ASSERT(menu);
	emit requestFolderContextMenu(folder, menu);
	return menu;
}

void Project::setMdiWindowVisibility(MdiWindowVisibility visibility) {
	d->mdiWindowVisibility = visibility;
	emit mdiWindowVisibilityChanged();
}

Project::MdiWindowVisibility Project::mdiWindowVisibility() const {
	return d->mdiWindowVisibility;
}

AbstractScriptingEngine* Project::scriptingEngine() const {
	return d->scriptingEngine;
}

CLASS_D_ACCESSOR_IMPL(Project, QString, fileName, FileName, fileName)
BASIC_D_ACCESSOR_IMPL(Project, QString, version, Version, version)
CLASS_D_ACCESSOR_IMPL(Project, QString, author, Author, author)
CLASS_D_ACCESSOR_IMPL(Project, QDateTime, modificationTime, ModificationTime, modificationTime)

void Project::setChanged(const bool value) {
	if ( value && !d->changed )
		emit changed();
	
	d->changed = value;
}

bool Project ::hasChanged() const {
	return d->changed ;
} 

//##############################################################################
//##################  Serialization/Deserialization  ###########################
//##############################################################################
/**
 * \brief Save as XML
 */
void Project::save(QXmlStreamWriter* writer) const {
    writer->setAutoFormatting(true);
	writer->writeStartDocument();
#ifdef ACTIVATE_SCIDAVIS_SPECIFIC_CODE
	writer->writeDTD("<!DOCTYPE SciDAVisProject>");
#else
	writer->writeDTD("<!DOCTYPE LabPlotXML>");
#endif

	writer->writeStartElement("project");
	writer->writeAttribute("version", version());
	writer->writeAttribute("fileName", fileName());
	writer->writeAttribute("modificationTime" , modificationTime().toString("yyyy-dd-MM hh:mm:ss:zzz"));
	writer->writeAttribute("author", author());
	writeBasicAttributes(writer);

	writeCommentElement(writer);

	//save all children
	foreach(AbstractAspect* child, children<AbstractAspect>(IncludeHidden)) {
		writer->writeStartElement("child_aspect");
		child->save(writer);
		writer->writeEndElement();
	}

	//save the state of the views (visible, maximized/minimized/geometry)
	//and the state of the project explorer (expanded items, currently selected item)
	emit requestSaveState(writer);
	
	writer->writeEndElement();
	writer->writeEndDocument();
}

/**
 * \brief Load from XML
 */
bool Project::load(XmlStreamReader* reader) {
	emit loadStarted();

	while (!(reader->isStartDocument() || reader->atEnd()))
		reader->readNext();

	if(!(reader->atEnd())) {
		if (!reader->skipToNextTag())
			return false;

		if (reader->name() == "project") {
			QString version = reader->attributes().value("version").toString();
			if(version.isEmpty())
				reader->raiseWarning(tr("Attribute 'version' is missing."));
			else
				d->version = version;

			if (!readBasicAttributes(reader)) return false;
			if (!readProjectAttributes(reader)) return false;

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
					} else if(reader->name() == "state") {
						//load the state of the views (visible, maximized/minimized/geometry)
						//and the state of the project explorer (expanded items, currently selected item)						
						emit requestLoadState(reader);
					} else {
						reader->raiseWarning(tr("unknown element '%1'").arg(reader->name().toString()));
						if (!reader->skipToEndElement()) return false;
					}
				}
			}

			//everything is read now.
			//restore the pointer to the data sets (columns) in xy-curves etc.
			QList<AbstractAspect*> curves = children("XYCurve", AbstractAspect::Recursive);
			if (curves.size()!=0) {
				QList<AbstractAspect*> spreadsheets = children("Spreadsheet", AbstractAspect::Recursive);
				XYCurve* curve;
				Spreadsheet* sheet;
				QString name;
				foreach (AbstractAspect* aspect, curves) {
					curve = dynamic_cast<XYCurve*>(aspect);
					if (!curve) continue;

					RESTORE_COLUMN_POINTER(xColumn, XColumn);
					RESTORE_COLUMN_POINTER(yColumn, YColumn);
					RESTORE_COLUMN_POINTER(valuesColumn, ValuesColumn);
					RESTORE_COLUMN_POINTER(xErrorPlusColumn, XErrorPlusColumn);
					RESTORE_COLUMN_POINTER(xErrorMinusColumn, XErrorMinusColumn);
					RESTORE_COLUMN_POINTER(yErrorPlusColumn, YErrorPlusColumn);
					RESTORE_COLUMN_POINTER(yErrorMinusColumn, YErrorMinusColumn);					
				}
			}
		} else {// no project element
			reader->raiseError(tr("no project element found"));
		}
	} else {// no start document
		reader->raiseError(tr("no valid XML document found"));
	}

	emit loadFinished();
	return !reader->hasError();
}

bool Project::readProjectAttributes(XmlStreamReader* reader) {
	QXmlStreamAttributes attribs = reader->attributes();
	QString str = attribs.value(reader->namespaceUri().toString(), "fileName").toString();
	if(str.isEmpty()) {
		reader->raiseError(tr("Project file name missing."));
		return false;
	}
	d->fileName = str;

	str = attribs.value(reader->namespaceUri().toString(), "modificationTime").toString();
	QDateTime modificationTime = QDateTime::fromString(str, "yyyy-dd-MM hh:mm:ss:zzz");
	if(str.isEmpty() || !modificationTime.isValid()) {
		reader->raiseWarning(tr("Invalid project modification time. Using current time."));
		d->modificationTime = QDateTime::currentDateTime();
	} else {
		d->modificationTime = modificationTime;
	}

	str = attribs.value(reader->namespaceUri().toString(), "author").toString();
	d->author = str;

	return true;
}
