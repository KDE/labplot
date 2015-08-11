/***************************************************************************
    File             : ColorMapManager.h
    Project          : LabPlot
    --------------------------------------------------------------------
    Copyright        : (C) 2015 Alexander Semke (alexander.semke@web.de)
    Description      : class providing access to all available color maps

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

#ifndef COLORMAPMANAGER_H
#define COLORMAPMANAGER_H

#include <QVector>
#include <QMap>
#include <QColor>
#include <QPixmap>

class ColorMapManager {

public:
	static ColorMapManager* getInstance();

	enum ColorMapId {BREWER_SPECTRAL_3, BREWER_SPECTRAL_4, BREWER_SPECTRAL_5, BREWER_SPECTRAL_6,
					BREWER_SPECTRAL_7, BREWER_SPECTRAL_8, BREWER_SPECTRAL_9, BREWER_SPECTRAL_10, BREWER_SPECTRAL_11,
					INVALID};


	const QVector<QRgb>& colorMap(const ColorMapId) const;
	const QVector<QRgb>& colorMap(const QString&) const;

	const QString& colorMapName(const ColorMapId) const;
	const ColorMapId colorMapId(const QString&) const;
	const QStringList colorMapNames() const;

	void fillPixmap(QPixmap&, ColorMapId, Qt::Orientation orientation = Qt::Horizontal);

private:
	ColorMapManager();
	~ColorMapManager();

	void init();

	static ColorMapManager* instance;
	QMap<ColorMapId, QVector<QRgb> > m_colorMaps;
	QMap<ColorMapId, QString> m_colorMapNames;
};

#endif
