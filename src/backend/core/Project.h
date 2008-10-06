/***************************************************************************
    File                 : Project.h
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
#ifndef PROJECT_H
#define PROJECT_H

#include "core/Folder.h"
#include "core/interfaces.h"
#include "lib/macros.h"

class QString;
#ifdef ACTIVATE_SCIDAVIS_SPECIFIC_CODE
class ProjectWindow;
#else
class MainWin;
#endif
class QAction;
class AbstractScriptingEngine;


/* remarks by thzs about the Labplot/SciDAVis integration of class Project:

- save/open: the stream based XML reading and writing is faster than using DOM.
  If you want, you can of course use DOM instead.
- QString m_filename: now d->file_name
- int m_version: now d->version
- QString m_labPlot: now d->labPlot
- QString m_title: use AbstractAspect::name()
- QString m_author: now d->author
- QDateTime m_created: use AbstractAspect::creationTime()
- QDateTime m_modified: now d->modification_time
- QString m_notes: use AbstractAspect::comment()
- bool m_changed: now d->changed
*/

//! Represents a SciDAVis project.
/**
 * Project manages an undo stack and is responsible for creating ProjectWindow instances
 * as views on itself.
 */
class Project : public Folder
{
	Q_OBJECT

	public:
		//! MDI subwindow visibility setting
		enum MdiWindowVisibility
		{
			folderOnly,
			folderAndSubfolders,
			allMdiWindows
		};

	public:
#ifdef ACTIVATE_SCIDAVIS_SPECIFIC_CODE
		Project();
#else
		Project(MainWin * mainWin);
#endif
		~Project();

		//!\name Reimplemented from AbstractAspect
		//@{
		virtual const Project *project() const { return this; }
		virtual Project *project() { return this; }
		virtual QUndoStack *undoStack() const;
		virtual QString path() const { return name(); }
		virtual QWidget *view();
		virtual QMenu *createContextMenu();
		//@}
		virtual QMenu *createFolderContextMenu(const Folder * folder);

		AbstractScriptingEngine * scriptingEngine() const;

		void setMdiWindowVisibility(MdiWindowVisibility visibility);
		MdiWindowVisibility mdiWindowVisibility() const;
		CLASS_D_ACCESSOR_DECL(QString, fileName, FileName)
		BASIC_D_ACCESSOR_DECL(int, version, Version)
#ifndef ACTIVATE_SCIDAVIS_SPECIFIC_CODE
		CLASS_D_ACCESSOR_DECL(QString, labPlot, LabPlot)
#endif
		CLASS_D_ACCESSOR_DECL(QString, author, Author)  
		CLASS_D_ACCESSOR_DECL(QDateTime, modificationTime, ModificationTime)
		FLAG_D_ACCESSOR_DECL(Changed)
	
#ifdef ACTIVATE_SCIDAVIS_SPECIFIC_CODE
		static ConfigPageWidget * makeConfigPage();
		static QString configPageLabel();
#endif
		static void staticInit();
	
		//! \name serialize/deserialize
		//@{
		//! Save as XML
		virtual void save(QXmlStreamWriter *) const;
		//! Load from XML
		virtual bool load(XmlStreamReader *);
		//@}

	signals:
		void requestProjectContextMenu(QMenu*);
		void requestFolderContextMenu(const Folder * folder, QMenu * menu);
		void mdiWindowVisibilityChanged();

	private:
		class Private;
		Private *d;
		bool readProjectAttributes(XmlStreamReader * reader);

#ifdef ACTIVATE_SCIDAVIS_SPECIFIC_CODE
		friend class ProjectConfigPage;
#endif
};

#endif // ifndef PROJECT_H
