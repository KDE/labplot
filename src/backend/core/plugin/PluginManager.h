/*
    File                 : PluginManager.h
    Project              : LabPlot/SciDAVis
    Description          : This class manages all plugins.
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2009 Tilman Benkert (thzs*gmx.net)
    (replace * with @ in the email addresses)
    SPDX-License-Identifier: GPL-2.0-or-later
*/


#ifndef PLUGINMANAGER_H
#define PLUGINMANAGER_H

#include <QList>
#include <QObjectList>
class QString;
class QStringList;
class PluginLoader;

class PluginManager {
private:
	PluginManager();

public:
	static bool enablePlugin(const QString &absolutePath);
	static void disablePlugin(const QString &absolutePath, bool rightNow = false);
	static QObjectList plugins();
	static QStringList loadedPluginFileNames();
	static QStringList failedPluginFileNames();
	static QString errorOfPlugin(const QString &fileName);
	static QObject *instanceOfPlugin(const QString &fileName);

#ifdef QT_DEBUG
	static void printAll();
#endif

private:
	static void loadAll();
	static QList<PluginLoader *> m_loadedPlugins;
	static QList<PluginLoader *> m_pluginsWithErrors;
	static QObjectList m_staticPlugins;
	static bool m_loaded;
	static QObjectList m_allPlugins;
};


#endif


