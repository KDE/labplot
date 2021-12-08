/*
	File                 : ColorMapsManager.h
	Project              : LabPlot
	Description          : widget showing the available color maps
	--------------------------------------------------------------------
    SPDX-FileCopyrightText: 2021 Alexander Semke <alexander.semke@web.de>
    SPDX-License-Identifier: GPL-2.0-or-later
*/


#include "tools/ColorMapsManager.h"
#include "backend/lib/macros.h"

#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QMessageBox>
#include <QPainter>
#include <QPixmap>
#include <QStandardPaths>

#include <KLocalizedString>

ColorMapsManager* ColorMapsManager::m_instance{nullptr};

/*!
	\class ColorMapsManager
	\brief color maps manager. singleton class holding the information about the data and metadata of the available color maps.

	\ingroup kdefrontend
 */
ColorMapsManager::ColorMapsManager() {
	m_jsonDir = QStandardPaths::locate(QStandardPaths::AppDataLocation, QLatin1String("colormaps"), QStandardPaths::LocateDirectory);
	loadCollections();
}

ColorMapsManager::~ColorMapsManager()  = default;

ColorMapsManager* ColorMapsManager::instance() {
	if (!m_instance)
		m_instance = new ColorMapsManager();

	return m_instance;
}

QStringList ColorMapsManager::collectionNames() const {
	return m_collections.keys();
}

QString ColorMapsManager::collectionInfo(const QString& name) const {
	return m_collections[name];
}

/*!
 * \brief returns the list of the color map names for the collection \c collecitonName
 */
QStringList ColorMapsManager::colorMapNames(const QString& collectionName) {
	//load the collection if not done yet
	if (!m_colorMaps.contains(collectionName)) {
		//color maps of the currently selected collection not loaded yet -> load them
		QString path = m_jsonDir + QLatin1Char('/') + collectionName + ".json";
		QFile collectionFile(path);
		if (collectionFile.open(QIODevice::ReadOnly)) {
			QJsonDocument doc = QJsonDocument::fromJson(collectionFile.readAll());
			if (!doc.isObject()) {
				QDEBUG("Invalid definition of " + path)
				return QStringList();
			}

			const QJsonObject& colorMaps = doc.object();
			const auto& keys = colorMaps.keys();
			m_colorMaps[collectionName] = keys;

			//load colors
			for (const auto& key : keys) {
				if (m_colors.find(key) == m_colors.end()) {
					QStringList colors;
					const auto& colorsArray = colorMaps.value(key).toArray();
					for (const auto& color : colorsArray)
						colors << color.toString();
					m_colors[key] = colors;
				}
			}
		}
	}

	return m_colorMaps[collectionName];
}

QVector<QColor> ColorMapsManager::colors() const {
	return m_colormap;
}

void ColorMapsManager::render(QPixmap& pixmap, const QString& name) {
	if (name.isEmpty())
		return;

	if (!m_colors.contains(name)) {
		for (const auto& name : collectionNames())
			colorMapNames(name);
	}

	//convert from the string RGB represetation to QColor
	m_colormap.clear();
	for (auto& rgb : m_colors[name]) {
		QStringList rgbValues = rgb.split(QLatin1Char(','));
		if (rgbValues.count() == 3)
			m_colormap << QColor(rgbValues.at(0).toInt(), rgbValues.at(1).toInt(), rgbValues.at(2).toInt());
		else if (rgbValues.count() == 4)
			m_colormap << QColor(rgbValues.at(1).toInt(), rgbValues.at(2).toInt(), rgbValues.at(3).toInt());
	}

	//render the preview pixmap
	int height = 80;
	int width = 200;
	int count = m_colormap.count();
	pixmap = QPixmap(width, height);
	QPainter p(&pixmap);
	int i = 0;
	for (auto& color : m_colormap) {
		p.setPen(color);
		p.setBrush(color);
		p.drawRect(i*width/count, 0, width/count, height);
		++i;
	}
}

/**
 * @brief Processes the json metadata file that contains the list of colormap collections.
 */
void ColorMapsManager::loadCollections() {
	const QString& fileName = m_jsonDir + QLatin1String("/ColormapCollections.json");
	QFile file(fileName);

	if (file.open(QIODevice::ReadOnly)) {
		QJsonDocument document = QJsonDocument::fromJson(file.readAll());
		file.close();
		if (!document.isArray()) {
			QDEBUG("Invalid definition of " + fileName)
			return;
		}

		for (const QJsonValueRef col : document.array()) {
			const QJsonObject& collection = col.toObject();
			const QString& name = collection[QLatin1String("name")].toString();
			const QString& desc = collection[QLatin1String("description")].toString();
			m_collections[name] = desc;
		}
	} else
		QMessageBox::critical(nullptr, i18n("File not found"),
							i18n("Couldn't open the color map collections file %1. Please check your installation.", fileName));
}
