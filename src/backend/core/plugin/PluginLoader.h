/*
    File                 : PluginLoader.h
    Project              : LabPlot/SciDAVis
    Description          : Loader for VersionedPlugins.
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2009 Tilman Benkert <thzs*gmx.net  (use @ for *)>
    SPDX-License-Identifier: GPL-2.0-or-later
*/


#ifndef PLUGINLOADER_H
#define PLUGINLOADER_H

#include <QPluginLoader>

class PluginLoader: public QObject {
	Q_OBJECT

	enum class PluginStatus {
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
	QPluginLoader *m_loader{nullptr};
	QString m_fileName;
	QString m_statusString;
	PluginStatus m_status{PluginStatus::NotYetLoaded};
};

#endif


