/***************************************************************************
	File                 : ColorMapsManager.h
	Project              : LabPlot
	Description          : color maps manager
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

class ColorMapsManager {

public:
	static ColorMapsManager* instance();
	QStringList collectionNames() const;
	QString collectionInfo(const QString& collectionName) const;
	QStringList colorMapNames(const QString& collectionName);
	QVector<QColor> colors() const;
	void render(QPixmap&, const QString& name);

private:
	ColorMapsManager();
	~ColorMapsManager();

	void loadCollections();

	static ColorMapsManager* m_instance;

	QMap<QString, QString> m_collections; //collections (key = collection name, value = description)
	QMap<QString, QStringList> m_colorMaps; //color maps in a collection (key = collection name, value = list of color map names)
	QMap<QString, QStringList> m_colors; //colors (key = color map name, value = list of colors in the string representation)
	QString m_jsonDir;
	QVector<QColor> m_colormap;
};

#endif // COLORMAPSMANAGER_H
