/***************************************************************************
    File                 : interfaces.h
    Project              : SciDAVis
    --------------------------------------------------------------------
    Copyright            : (C) 2008 by Knut Franke
    Email (use @ for *)  : knut.franke*gmx.de
    Description          : Interfaces the kernel uses to talk to modules

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
#ifndef INTERFACES_H
#define INTERFACES_H

#include "lib/ConfigPageWidget.h"
#include "core/AbstractAspect.h"
#include "lib/XmlStreamReader.h"

#include <QtPlugin>

class AbstractPart;
class QAction;
class QMenu;
class ProjectWindow;
class AbstractFilter;
class AbstractImportFilter;
class AbstractExportFilter;
class ActionManager;

class PartMaker
{
	public:
		virtual ~PartMaker() {}
		virtual AbstractPart *makePart() = 0;
		virtual QAction *makeAction(QObject *parent) = 0;
};

Q_DECLARE_INTERFACE(PartMaker, "net.sf.scidavis.partmaker/0.1")

class FilterMaker
{
	public:
		virtual ~FilterMaker() {}
		virtual AbstractFilter * makeFilter(int id=0) = 0;
		virtual int filterCount() const { return 1; }
		virtual QAction *makeAction(QObject *parent, int id=0) = 0;
};

Q_DECLARE_INTERFACE(FilterMaker, "net.sf.scidavis.filtermaker/0.1")

class FileFormat
{
	public:
		virtual ~FileFormat() {}
		virtual AbstractImportFilter * makeImportFilter() = 0;
		virtual AbstractExportFilter * makeExportFilter() = 0;
};

Q_DECLARE_INTERFACE(FileFormat, "net.sf.scidavis.fileformat/0.1")

#ifdef ACTIVATE_SCIDAVIS_SPECIFIC_CODE
class ActionManagerOwner
{
	public:
		virtual ~ActionManagerOwner() {}
		virtual ActionManager * actionManager() = 0;
		virtual void initActionManager() {}
};

Q_DECLARE_INTERFACE(ActionManagerOwner, "net.sf.scidavis.actionmanagerowner/0.1")

class ConfigPageMaker {
	public:
		virtual ~ConfigPageMaker() {}
		virtual ConfigPageWidget * makeConfigPage() = 0;
		virtual QString configPageLabel() = 0;
		// TODO (maybe): icons instead of tabs to select the pages
		//		virtual QIcon icon() = 0;
};

Q_DECLARE_INTERFACE(ConfigPageMaker, "net.sf.scidavis.configpagemaker/0.1")
#endif

class XmlElementAspectMaker
{
	public:
		virtual ~XmlElementAspectMaker() {}
		virtual bool canCreate(const QString & element_name) = 0;
		virtual AbstractAspect * createAspectFromXml(XmlStreamReader * reader) = 0;
};

Q_DECLARE_INTERFACE(XmlElementAspectMaker, "net.sf.scidavis.xmlelementaspectmaker/0.1")

class NeedsStaticInit
{
	public:
		virtual ~NeedsStaticInit() {}
		virtual void staticInit() = 0;
};

Q_DECLARE_INTERFACE(NeedsStaticInit, "net.sf.scidavis.needsstaticinit/0.1")

class VersionedPlugin
{
	public:
		virtual ~VersionedPlugin() {}
		virtual int pluginTargetAppVersion() const = 0;
		virtual QString pluginTargetAppName() const = 0;
		virtual QString pluginName() const = 0;
};

Q_DECLARE_INTERFACE(VersionedPlugin, "net.sf.scidavis.versionedplugin/0.1")

#endif // ifndef INTERFACES_H
