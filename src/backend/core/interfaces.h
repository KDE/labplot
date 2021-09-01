/*
    File                 : interfaces.h
    Project              : SciDAVis
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2008 Knut Franke <knut.franke*gmx.de (use @ for *)>
    Description          : Interfaces the kernel uses to talk to modules
    SPDX-License-Identifier: GPL-2.0-or-later
*/

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
