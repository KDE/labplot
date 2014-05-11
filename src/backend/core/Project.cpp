/***************************************************************************
    File                 : Project.cpp
    Project              : LabPlot
    Description          : Represents a LabPlot project.
    --------------------------------------------------------------------
    Copyright            : (C) 2011-2014 Alexander Semke (alexander.semke*web.de)
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
#include "backend/worksheet/plots/cartesian/XYEquationCurve.h"
#include "backend/worksheet/plots/cartesian/Axis.h"

#include <QUndoStack>
#include <QMenu>
#include <QDateTime>

#include <KConfig>
#include <KConfigGroup>

/**
 * \class Project
 * \brief Represents a project.
 * \ingroup core
 * Project represents the root node of all objects created during the runtime of the program.
 * Manages also the undo stack.
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
			author(QString(qgetenv("USER"))),
			modificationTime(QDateTime::currentDateTime()),
			changed(false),
			loading(false)
			{}

		QUndoStack undo_stack;
		MdiWindowVisibility mdiWindowVisibility;
		AbstractScriptingEngine* scriptingEngine;
		QString fileName;
		QString version;
		QString author;
		QDateTime modificationTime;
		bool changed;
		bool loading;
};

Project::Project() : Folder(i18n("Project")), d(new Private()) {
	//load default values for name, comment and author from config
	KConfig config;
	KConfigGroup group = config.group("Project");

	d->author = group.readEntry("Author", QString());

	//we don't have direct access to the members name and comment
	//->temporaly disable the undo stack and call the setters
	setUndoAware(false);
	d->loading = true;
	setName(group.readEntry("Name", i18n("Project")));
	setComment(group.readEntry("Comment", QString()));
	setUndoAware(true);
	d->loading = false;
	d->changed = false;

#ifndef SUPPRESS_SCRIPTING_INIT
	// TODO: intelligent engine choosing
	Q_ASSERT(ScriptingEngineManager::instance()->engineNames().size() > 0);
	QString engine_name = ScriptingEngineManager::instance()->engineNames()[0];
	d->scriptingEngine = ScriptingEngineManager::instance()->engine(engine_name);
#endif

	connect(this, SIGNAL(aspectDescriptionChanged(const AbstractAspect*)),this, SLOT(descriptionChanged(const AbstractAspect*)));
}

Project::~Project() {
	d->undo_stack.clear();
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
	if (d->loading)
		return;

	if ( value && !d->changed )
		emit changed();

	d->changed = value;
}

bool Project ::hasChanged() const {
	return d->changed ;
}

void Project::descriptionChanged(const AbstractAspect* aspect) {
	if (d->loading)
		return;

	if (this!=aspect)
		return;

	d->changed = true;
	emit changed();
}

//##############################################################################
//##################  Serialization/Deserialization  ###########################
//##############################################################################
/**
 * \brief Save as XML
 */
void Project::save(QXmlStreamWriter* writer) const {
	//set the version and the modification time to the current values
	d->version = LVERSION;
	d->modificationTime = QDateTime::currentDateTime();

	writer->setAutoFormatting(true);
	writer->writeStartDocument();
	writer->writeDTD("<!DOCTYPE LabPlotXML>");

	writer->writeStartElement("project");
	writer->writeAttribute("version", version());
	writer->writeAttribute("fileName", fileName());
	writer->writeAttribute("modificationTime", modificationTime().toString("yyyy-dd-MM hh:mm:ss:zzz"));
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
	d->loading = true;
	emit loadStarted();

	while (!(reader->isStartDocument() || reader->atEnd()))
		reader->readNext();

	if(!(reader->atEnd())) {
		if (!reader->skipToNextTag())
			return false;

		if (reader->name() == "project") {
			QString version = reader->attributes().value("version").toString();
			if(version.isEmpty())
				reader->raiseWarning(i18n("Attribute 'version' is missing."));
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
						reader->raiseWarning(i18n("unknown element '%1'", reader->name().toString()));
						if (!reader->skipToEndElement()) return false;
					}
				}
			}

			//everything is read now.
			//restore the pointer to the data sets (columns) in xy-curves etc.
			QList<AbstractAspect*> curves = children("XYCurve", AbstractAspect::Recursive);
			QList<AbstractAspect*> axes = children("Axes", AbstractAspect::Recursive);
			if (curves.size()!=0 || axes.size()!=0) {
				QList<AbstractAspect*> columns = children("Column", AbstractAspect::Recursive);

				//XY-curves
				foreach (AbstractAspect* aspect, curves) {
					XYCurve* curve = dynamic_cast<XYCurve*>(aspect);
					if (!curve) continue;

					XYEquationCurve* equationCurve = dynamic_cast<XYEquationCurve*>(aspect);
					if (equationCurve) {
						//curves defined by a mathematical equations recalculate their own columns on load again.
						equationCurve->recalculate();
					} else {
						RESTORE_COLUMN_POINTER(curve, xColumn, XColumn);
						RESTORE_COLUMN_POINTER(curve, yColumn, YColumn);
						RESTORE_COLUMN_POINTER(curve, valuesColumn, ValuesColumn);
						RESTORE_COLUMN_POINTER(curve, xErrorPlusColumn, XErrorPlusColumn);
						RESTORE_COLUMN_POINTER(curve, xErrorMinusColumn, XErrorMinusColumn);
						RESTORE_COLUMN_POINTER(curve, yErrorPlusColumn, YErrorPlusColumn);
						RESTORE_COLUMN_POINTER(curve, yErrorMinusColumn, YErrorMinusColumn);
					}
				}

				//Axes
				foreach (AbstractAspect* aspect, axes) {
					Axis* axis = dynamic_cast<Axis*>(aspect);
					if (!axis) continue;

					RESTORE_COLUMN_POINTER(axis, majorTicksColumn, MajorTicksColumn);
					RESTORE_COLUMN_POINTER(axis, minorTicksColumn, MinorTicksColumn);
				}
			}
		} else {// no project element
			reader->raiseError(i18n("no project element found"));
		}
	} else {// no start document
		reader->raiseError(i18n("no valid XML document found"));
	}

	d->loading = false;
	emit loadFinished();
	return !reader->hasError();
}

bool Project::readProjectAttributes(XmlStreamReader* reader) {
	QXmlStreamAttributes attribs = reader->attributes();
	QString str = attribs.value(reader->namespaceUri().toString(), "fileName").toString();
	if(str.isEmpty()) {
		reader->raiseError(i18n("Project file name missing."));
		return false;
	}
	d->fileName = str;

	str = attribs.value(reader->namespaceUri().toString(), "modificationTime").toString();
	QDateTime modificationTime = QDateTime::fromString(str, "yyyy-dd-MM hh:mm:ss:zzz");
	if(str.isEmpty() || !modificationTime.isValid()) {
		reader->raiseWarning(i18n("Invalid project modification time. Using current time."));
		d->modificationTime = QDateTime::currentDateTime();
	} else {
		d->modificationTime = modificationTime;
	}

	str = attribs.value(reader->namespaceUri().toString(), "author").toString();
	d->author = str;

	return true;
}
