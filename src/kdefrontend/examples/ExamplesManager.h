/*
    File                 : ExamplesManager.h
    Project              : LabPlot
    Description          : examples projects manager
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2021 Alexander Semke <alexander.semke@web.de>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef COLORMAPSMANAGER_H
#define COLORMAPSMANAGER_H

#include <QColor>
#include <QMap>
#include <QVector>

class QPixmap;

class ExamplesManager {

public:
	static ExamplesManager* instance();
	QStringList collectionNames() const;
	QString collectionInfo(const QString& collectionName) const;
	QStringList exampleNames(const QString& collectionName);
	QPixmap pixmap(const QString&) const;
	QString description(const QString&) const;
	QString path(const QString&) const;

private:
	ExamplesManager();
	~ExamplesManager();

	void loadCollections();

	static ExamplesManager* m_instance;

	QMap<QString, QString> m_collections; //collections (key = collection name, value = description)
	QMap<QString, QStringList> m_examples; //names of the example projects in a collection (key = collection name, value = list of project names)
	QMap<QString, QString> m_descriptions; //example desciptions (key = example project name, value = description)
	QMap<QString, QPixmap> m_pixmaps; //preview pixmaps (key = example project name, value = pixmap)
	QMap<QString, QString> m_paths; //paths for the example projects (key = example project name, value = path)
	QString m_jsonDir;
};

#endif // COLORMAPSMANAGER_H
