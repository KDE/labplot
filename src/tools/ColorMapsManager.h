/*
	File                 : ColorMapsManager.h
	Project              : LabPlot
	Description          : color maps manager
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2021 Alexander Semke (alexander.semke@web.de)

*/

/***************************************************************************
 *                                                                         *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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
