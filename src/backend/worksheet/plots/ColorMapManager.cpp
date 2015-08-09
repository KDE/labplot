/***************************************************************************
    File             : ColorMapManager.cpp
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

#include "ColorMapManager.h"
#include <QColor>
#include <QPainter>
#include <QDebug>

ColorMapManager* ColorMapManager::instance = NULL;

ColorMapManager::ColorMapManager() {
	init();
}

ColorMapManager* ColorMapManager::getInstance() {
	if (!instance)
		instance = new ColorMapManager();

	return instance;
}

const QVector<QRgb>& ColorMapManager::colorMap(const ColorMapId id) const {
	return m_colorMaps[id];
}

const QVector<QRgb>& ColorMapManager::colorMap(const QString& name) const {
	QMap<ColorMapId, QString>::const_iterator it = m_colorMapNames.begin();
	while (it != m_colorMapNames.end()) {
		if (it.value()==name)
			break;
		++it;
	}

	if (it!=m_colorMapNames.end())
		return m_colorMaps[it.key()];
	else
		return QVector<QRgb>();
}

const QString& ColorMapManager::colorMapName(ColorMapId id) const {
	return m_colorMapNames[id];
}

const ColorMapManager::ColorMapId ColorMapManager::colorMapId(const QString& name) const {
	QMap<ColorMapId, QString>::const_iterator it = m_colorMapNames.begin();
	while (it != m_colorMapNames.end()) {
		if (it.value()==name)
			break;
		++it;
	}

	if (it!=m_colorMapNames.end())
		return it.key();
	else
		return INVALID;
}


const QStringList ColorMapManager::colorMapNames() const {
	QStringList names;
	QMap<ColorMapId, QString>::const_iterator it = m_colorMapNames.begin();
	while (it != m_colorMapNames.end()) {
		names << it.value();
		++it;
	}

	return names;
}

void ColorMapManager::fillPixmap(QPixmap& pix, ColorMapManager::ColorMapId id, Qt::Orientation orientation) {
	if (id==INVALID) {
		pix.fill(Qt::transparent);
		return;
	}

	QVector<QRgb> colorMap = m_colorMaps[id];
	QPainter painter(&pix);

	const float h = pix.height()/colorMap.size();
	const float w = pix.width();

	for (int i=0; i<colorMap.size(); ++i) {
		const QRgb& rgb = colorMap.at(i);
		painter.setBrush(QColor(rgb));
		if (orientation == Qt::Horizontal)
			painter.drawRect(QRectF(i*h+2, 0.0, h, w));
		else
			painter.drawRect(QRectF(0.0, i*h+2, w, h));
	}
}

void ColorMapManager::init() {
	//Brewer color maps
	m_colorMapNames[BREWER_SPECTRAL_3] = "Spectral 3";
	m_colorMaps[BREWER_SPECTRAL_3] = QVector<QRgb>()
		<< qRgb(252,141,89)
		<< qRgb(255,255,191)
		<< qRgb(153,213,148);

	m_colorMapNames[BREWER_SPECTRAL_4] = "Spectral 4";
	m_colorMaps[BREWER_SPECTRAL_4] = QVector<QRgb>()
		<< qRgb(215,25,28)
		<< qRgb(253,174,97)
		<< qRgb(171,221,164)
		<< qRgb(43,131,186);


	m_colorMapNames[BREWER_SPECTRAL_5] = "Spectral 5";
	m_colorMaps[BREWER_SPECTRAL_5] = QVector<QRgb>()
		<< qRgb(215,25,28)
		<< qRgb(253,174,97)
		<< qRgb(255,255,191)
		<< qRgb(171,221,164)
		<< qRgb(43,131,186);

	m_colorMapNames[BREWER_SPECTRAL_6] = "Spectral 6";
	m_colorMaps[BREWER_SPECTRAL_6] = QVector<QRgb>()
		<< qRgb(213,62,79)
		<< qRgb(252,141,89)
		<< qRgb(254,224,139)
		<< qRgb(230,245,152)
		<< qRgb(153,213,148)
		<< qRgb(50,136,189);

	m_colorMapNames[BREWER_SPECTRAL_7] = "Spectral 7";
	m_colorMaps[BREWER_SPECTRAL_7] = QVector<QRgb>()
		<< qRgb(213,62,79)
		<< qRgb(252,141,89)
		<< qRgb(254,224,139)
		<< qRgb(255,255,191)
		<< qRgb(230,245,152)
		<< qRgb(153,213,148)
		<< qRgb(50,136,189);

	m_colorMapNames[BREWER_SPECTRAL_8] = "Spectral 8";
	m_colorMaps[BREWER_SPECTRAL_8] = QVector<QRgb>()
		<< qRgb(213,62,79)
		<< qRgb(244,109,67)
		<< qRgb(253,174,97)
		<< qRgb(254,224,139)
		<< qRgb(230,245,152)
		<< qRgb(171,221,164)
		<< qRgb(102,194,165)
		<< qRgb(50,136,189);

	m_colorMapNames[BREWER_SPECTRAL_9] = "Spectral 9";
	m_colorMaps[BREWER_SPECTRAL_9] = QVector<QRgb>()
		<< qRgb(213,62,79)
		<< qRgb(244,109,67)
		<< qRgb(253,174,97)
		<< qRgb(254,224,139)
		<< qRgb(255,255,191)
		<< qRgb(230,245,152)
		<< qRgb(171,221,164)
		<< qRgb(102,194,165)
		<< qRgb(50,136,189);

	m_colorMapNames[BREWER_SPECTRAL_10] = "Spectral 10";
	m_colorMaps[BREWER_SPECTRAL_10] = QVector<QRgb>()
		<< qRgb(158,1,66)
		<< qRgb(213,62,79)
		<< qRgb(244,109,67)
		<< qRgb(253,174,97)
		<< qRgb(254,224,139)
		<< qRgb(230,245,152)
		<< qRgb(171,221,164)
		<< qRgb(102,194,165)
		<< qRgb(50,136,189)
		<< qRgb(94,79,162);

	m_colorMapNames[BREWER_SPECTRAL_11] = "Spectral 11";
	m_colorMaps[BREWER_SPECTRAL_11] = QVector<QRgb>()
		<< qRgb(158,1,66)
		<< qRgb(213,62,79)
		<< qRgb(244,109,67)
		<< qRgb(253,174,97)
		<< qRgb(254,224,139)
		<< qRgb(255,255,191)
		<< qRgb(230,245,152)
		<< qRgb(171,221,164)
		<< qRgb(102,194,165)
		<< qRgb(50,136,189)
		<< qRgb(94,79,162);


}
