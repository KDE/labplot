/***************************************************************************
    File                 : PluginLoader.cpp
    Project              : LabPlot/SciDAVis
    Description          : Loader for VersionedPlugins.
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

#include "backend/core/plugin/PluginLoader.h"
#include "backend/core/interfaces.h"

#include <KLocalizedString>

/**
 * \class PluginLoader
 * \brief Loader for VersionedPlugins.
 *
 *  This class wraps a QPluginLoader object to support
 *  custom error/status strings. 
 */

PluginLoader::PluginLoader(QString fileName) 
		: m_loader(nullptr), m_fileName(std::move(fileName)) {
	m_status = NotYetLoaded;
	m_statusString = i18n("Not yet loaded.");
}
 
PluginLoader::~PluginLoader() {
	unload();
}

QString PluginLoader::statusString () const {
	return m_statusString;
}

PluginLoader::PluginStatus PluginLoader::status () const {
	return m_status;
}

QString PluginLoader::fileName() const {
	return m_fileName;
}

QObject *PluginLoader::instance() {
	return isActive() ? m_loader->instance() : nullptr;
}

bool PluginLoader::isActive() const {
	return (Active == m_status);
}

bool PluginLoader::load() {
	if (!m_loader)
		m_loader = new QPluginLoader(m_fileName);
	if (!m_loader->isLoaded()) {
		if (m_loader->load()) {
			//TODO
// 			VersionedPlugin *plugin = qobject_cast<VersionedPlugin *>(m_loader->instance());
// 			if (plugin) {
// 				int version = plugin->pluginTargetAppVersion();
// 				QString appName = plugin->pluginTargetAppName();
// 				if (SciDAVis::appName == appName && 
// 						(SciDAVis::version() & 0xFFFF00) == (version & 0xFFFF00)) {
// 					m_statusString = i18n("Plugin '%1' successfully loaded.", m_fileName);
// 					m_status = Active;
// 				} else {
// 					m_statusString = i18n("Plugin '%1' was created for incompatible version: %2 %3.%4.x",
// 							m_fileName, appName, (version & 0xFF0000) >> 16, (version & 0x00FF00) >> 8);
// 					m_status = IncompatibleApp;
// 				}
// 			} else {
// 				m_statusString = i18n("Plugin '%1' is not a %2 plugin.", m_fileName, SciDAVis::appName);
// 				m_status = NoVersionedPlugin;
// 			}
		} else {
			m_statusString = m_loader->errorString();
			m_status = ErrorFromQt;
		}
	}
	return (Active == m_status);
}

bool PluginLoader::unload() {
	if (m_loader && m_loader->isLoaded())
		m_loader->unload();
	delete m_loader;
	m_loader = nullptr;
	m_status = NotYetLoaded;
	m_statusString = i18n("Not yet loaded.");

	return true;
}

