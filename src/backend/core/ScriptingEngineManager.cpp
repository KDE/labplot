/***************************************************************************
    File                 : ScriptingEngineManager.cpp
    Project              : SciDAVis
    --------------------------------------------------------------------
    Copyright            : (C) 2008 Knut Franke
    Email (use @ for *)  : Knut.Franke*gmx.net
    Description          : Entry point for dealing with scripting.

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

#include "backend/core/ScriptingEngineManager.h"
#include "backend/core/AbstractScriptingEngine.h"
#include "backend/core/plugin/PluginManager.h"

#include <QStringList>

ScriptingEngineManager* ScriptingEngineManager::instance() {
	static ScriptingEngineManager the_instance;
	return &the_instance;
}

ScriptingEngineManager::ScriptingEngineManager() {
	for (auto* plugin : PluginManager::plugins()) {
		AbstractScriptingEngine* engine = qobject_cast<AbstractScriptingEngine*>(plugin);
		if (engine) m_engines << engine;
	}
}

ScriptingEngineManager::~ScriptingEngineManager() {
	qDeleteAll(m_engines);
}

QStringList ScriptingEngineManager::engineNames() const {
	QStringList result;
	for (auto* engine : m_engines)
		result << engine->objectName();
	return result;
}

AbstractScriptingEngine * ScriptingEngineManager::engine(const QString &name) {
	for (auto* engine : m_engines)
		if (engine->objectName() == name) {
			if (!engine->initialized())
				engine->initialize();
			return engine->initialized() ? engine : nullptr;
		}
	return nullptr;
}
