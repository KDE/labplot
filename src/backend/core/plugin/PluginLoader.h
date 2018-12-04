/***************************************************************************
    File                 : PluginLoader.h
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

#ifndef PLUGINLOADER_H
#define PLUGINLOADER_H

#include <QPluginLoader>

class PluginLoader: public QObject {
	Q_OBJECT

	enum PluginStatus {
		NotYetLoaded,
		Active,
		ErrorFromQt,
		NoVersionedPlugin,
		IncompatibleApp,
	};

	public:
		explicit PluginLoader(QString fileName);
		~PluginLoader() override;
		QString statusString() const;
		PluginStatus status() const;
		QString fileName() const;
		QObject *instance();
		bool isActive() const;
		bool load();
		bool unload();

	private:
		QPluginLoader *m_loader;
		QString m_fileName;
		QString m_statusString;
		PluginStatus m_status;
};

#endif


