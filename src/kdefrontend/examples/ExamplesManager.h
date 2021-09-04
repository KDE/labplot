/***************************************************************************
	File                 : ExamplesManager.h
	Project              : LabPlot
	Description          : examples projects manager
	--------------------------------------------------------------------
	Copyright            : (C) 2021 by Alexander Semke (alexander.semke@web.de)

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

private:
	ExamplesManager();
	~ExamplesManager();

	void loadCollections();

	static ExamplesManager* m_instance;

	QMap<QString, QString> m_collections; //collections (key = collection name, value = description)
	QMap<QString, QStringList> m_examples; //names of the example projects in a collection (key = collection name, value = list of project names)
	QMap<QString, QString> m_descriptions; //example desciptions (key = example project name, value = description)
	QMap<QString, QPixmap> m_pixmaps; //preview pixmaps (key = example project name, value = pixmap)
	QString m_jsonDir;
};

#endif // COLORMAPSMANAGER_H
