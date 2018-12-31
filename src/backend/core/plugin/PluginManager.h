/***************************************************************************
    File                 : PluginManager.h
    Project              : LabPlot/SciDAVis
    Description          : This class manages all plugins.
    --------------------------------------------------------------------
    Copyright            : (C) 2009 Tilman Benkert (thzs*gmx.net)
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


