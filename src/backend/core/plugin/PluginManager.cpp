/*
    File                 : PluginManager.cpp
    Project              : LabPlot/SciDAVis
    Description          : This class manages all plugins.
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2009 Tilman Benkert (thzs*gmx.net)
    (replace * with @ in the email addresses)

*/

/***************************************************************************
 *                                                                         *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *                                                                         *
 ***************************************************************************/

#include "backend/core/plugin/PluginManager.h"
#include "backend/core/plugin/PluginLoader.h"
#include <QPluginLoader>
#include <QSettings>
#include <QtDebug>
#include <QSet>

/**
 * \class PluginManager
 * \brief This class manages all plugins.
 *
 * This class is used to persistently (using QSettings) enable/disable dynamic plugins and
 * to make the difference between static and dynamic plugins transparent
 * to the application. You typically do the following:
   \code
   // let the user select a plugin from a file dialog
   PluginManager::enablePlugin(absoluteFilePath);
   ...
   // somewhere else in the application
   foreach(QObject *plugin, PluginManager::plugins()) {
     MyInterface *myObject = qobject_cast<MyInterface *>(plugin);
        if (myObject)
           // found a suitable plugin
   }
   \endcode
 *
 */

#ifdef Q_OS_MAC // Mac
#define SETTINGS_FORMAT QSettings::IniFormat
#else
#define SETTINGS_FORMAT QSettings::NativeFormat
#endif

PluginManager::PluginManager() = default;

QList<PluginLoader *> PluginManager::m_loadedPlugins;
QList<PluginLoader *> PluginManager::m_pluginsWithErrors;
QObjectList PluginManager::m_staticPlugins;
bool PluginManager::m_loaded = false;
QObjectList PluginManager::m_allPlugins;

/**
 * \brief Load a plugin and append it to the plugin list in the settings.
 *
 * \param absolutePath Absolute path to the plugin file.
 * \return True, if the plugin was successfully loaded. Even if false is returned,
 * the path is still appended to the settings.
 */
bool PluginManager::enablePlugin(const QString &absolutePath) {
	loadAll();

	// save the plugin in the settings
	//TODO
// 	QSettings settings(SETTINGS_FORMAT, QSettings::UserScope, SciDAVis::appName, SciDAVis::appName);
// 	settings.beginGroup("PluginManager");
// 	QStringList pluginPaths = settings.value("enabledPlugins", QStringList()).toStringList();
// 	pluginPaths.removeDuplicates();
// 	if (!pluginPaths.contains(absolutePath)) {
// 		pluginPaths << absolutePath;
// 		settings.setValue("enabledPlugins", pluginPaths);
// 	}
// 	settings.endGroup();

	// check whether it's already loaded
	bool result = true;
	bool alreadyLoaded = false;
	for (auto* loader : m_loadedPlugins) {
		if (loader->fileName() == absolutePath) {
			alreadyLoaded = true;
			break;
		}
	}

	if (!alreadyLoaded) {
		PluginLoader *pluginLoader = nullptr;
		// check whether a loader for this plugin already exists
		for (auto* loader : m_pluginsWithErrors) {
			if (loader->fileName() == absolutePath) {
				pluginLoader = loader;
				m_pluginsWithErrors.removeAll(loader);
				break;
			}
		}

		if (!pluginLoader)
			pluginLoader = new PluginLoader(absolutePath);

		// try to load the plugin
		if (!pluginLoader->load()) {
			m_pluginsWithErrors << pluginLoader;
			result = false;
		} else {
			m_loadedPlugins << pluginLoader;
			m_allPlugins << pluginLoader->instance();
		}
	}

	return result;
}

/**
 * \brief Remove the plugin from the plugin list in the settings (and unload it if rightNow == true).
 *
 * \param rightNow Set this to true if you want to unload the plugin immediately (which is not
 * always a good idea). The default behavior is to disable the plugin for the next program startup.
 */
void PluginManager::disablePlugin(const QString &absolutePath, bool rightNow) {
	loadAll();

	// remove the plugin from the settings
	//TODO
// 	QSettings settings(SETTINGS_FORMAT, QSettings::UserScope, SciDAVis::appName, SciDAVis::appName);
// 	settings.beginGroup("PluginManager");
// 	QStringList pluginPaths = settings.value("enabledPlugins", QStringList()).toStringList();
// 	pluginPaths.removeDuplicates();
// 	int index = pluginPaths.indexOf(absolutePath);
// 	if (index > -1) {
// 		pluginPaths.removeAt(index);
// 		settings.setValue("enabledPlugins", pluginPaths);
// 	}
// 	settings.endGroup();

	// remove the related loader if it is in the list of failed loaders
	for (auto* loader : m_pluginsWithErrors) {
		if (loader->fileName() == absolutePath) {
			m_pluginsWithErrors.removeAll(loader);
			delete loader;
			break;
		}
	}

	if (rightNow) {
		// unload the plugin and remove the loader
		for (auto* loader : m_loadedPlugins) {
			if (loader->fileName() == absolutePath) {
				m_allPlugins.removeAll(loader->instance());
				m_loadedPlugins.removeAll(loader);
				loader->unload();
				delete loader;
				break;
			}
		}
	}
}

/**
 * \brief Get all plugin root instances.
 *
 * This function will return a list of all static plugin root instances
 * as well as those from all successfully loaded dynamic plugins.
 */
QObjectList PluginManager::plugins() {
	loadAll();
	return m_allPlugins;
}

/**
 * \brief Load all plugins contained in the "enabledPlugins" setting.
 *
 * This method also fetches a list of all static plugins.
 */
void PluginManager::loadAll() {
	if (!m_loaded) {
		m_staticPlugins = QPluginLoader::staticInstances();
		m_allPlugins << m_staticPlugins;

		//TODO:
// 		QSettings settings(SETTINGS_FORMAT, QSettings::UserScope, SciDAVis::appName, SciDAVis::appName);
// 		settings.beginGroup("PluginManager");
// 		QStringList plugins = settings.value("enabledPlugins", QStringList()).toStringList();
// 		plugins.removeDuplicates();
// 		foreach (const QString& plugin, plugins) {
// 			PluginLoader *pluginLoader = new PluginLoader(plugin);
// 			if (!pluginLoader->load()) {
// 				m_pluginsWithErrors << pluginLoader;
// 			} else {
// 				m_loadedPlugins << pluginLoader;
// 				m_allPlugins << pluginLoader->instance();
// 			}
// 		}
// 		settings.endGroup();

		m_loaded = true;
	}
}

/**
 * \brief Get the file names of all loaded plugins.
 */
QStringList PluginManager::loadedPluginFileNames() {
	QStringList result;
	for (auto* loader : m_loadedPlugins)
		result << loader->fileName();
	return result;
}

/**
 * \brief Get the file names of all plugins that failed to load.
 */
QStringList PluginManager::failedPluginFileNames() {
	QStringList result;
	for (auto* loader : m_pluginsWithErrors)
		result << loader->fileName();
	return result;
}

/**
 * \brief Get the error messages of a plugin that failed to load.
 */
QString PluginManager::errorOfPlugin(const QString &fileName) {
	QString result;
	for (auto* loader : m_pluginsWithErrors)
		if (loader->fileName() == fileName)
			result = loader->statusString();
	return result;
}

/**
 * \brief Get the plugin root instance for a given file name.
 */
QObject* PluginManager::instanceOfPlugin(const QString &fileName) {
	QObject* result = nullptr;
	for (auto* loader : m_loadedPlugins)
		if (loader->fileName() == fileName)
			result = loader->instance();
	return result;
}

#ifdef QT_DEBUG
void PluginManager::printAll() {
	loadAll();
	for (auto* loader : m_loadedPlugins) {
		qDebug() << "Plugin" << loader->fileName() << "loaded";
	}

	for (auto* loader : m_pluginsWithErrors) {
		qDebug() << loader->statusString();
	}

}
#endif

