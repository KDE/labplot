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

#include "backend/core/AbstractAspect.h"
#include "backend/lib/XmlStreamReader.h"

#include <QtPlugin>

class AbstractPart;
class QAction;
class QMenu;
class ProjectWindow;
class AbstractFilter;
class AbstractImportFilter;
class AbstractExportFilter;
class ActionManager;

class PartMaker {
public:
	virtual ~PartMaker() = default;
	virtual AbstractPart *makePart() = 0;
	virtual QAction *makeAction(QObject *parent) = 0;
};

Q_DECLARE_INTERFACE(PartMaker, "net.sf.scidavis.partmaker/0.1")

class FilterMaker {
public:
	virtual ~FilterMaker() = default;
	virtual AbstractFilter * makeFilter(int id=0) = 0;
	virtual int filterCount() const { return 1; }
	virtual QAction *makeAction(QObject *parent, int id=0) = 0;
};

Q_DECLARE_INTERFACE(FilterMaker, "net.sf.scidavis.filtermaker/0.1")

class FileFormat {
public:
	virtual ~FileFormat() = default;
	virtual AbstractImportFilter * makeImportFilter() = 0;
	virtual AbstractExportFilter * makeExportFilter() = 0;
};

Q_DECLARE_INTERFACE(FileFormat, "net.sf.scidavis.fileformat/0.1")

class XmlElementAspectMaker {
public:
	virtual ~XmlElementAspectMaker() = default;
	virtual bool canCreate(const QString & element_name) = 0;
	virtual AbstractAspect * createAspectFromXml(XmlStreamReader * reader) = 0;
};

Q_DECLARE_INTERFACE(XmlElementAspectMaker, "net.sf.scidavis.xmlelementaspectmaker/0.1")

class NeedsStaticInit {
public:
	virtual ~NeedsStaticInit() = default;
	virtual void staticInit() = 0;
};

Q_DECLARE_INTERFACE(NeedsStaticInit, "net.sf.scidavis.needsstaticinit/0.1")

class VersionedPlugin {
public:
	virtual ~VersionedPlugin() = default;
	virtual int pluginTargetAppVersion() const = 0;
	virtual QString pluginTargetAppName() const = 0;
	virtual QString pluginName() const = 0;
};

Q_DECLARE_INTERFACE(VersionedPlugin, "net.sf.scidavis.versionedplugin/0.1")

#endif // ifndef INTERFACES_H
