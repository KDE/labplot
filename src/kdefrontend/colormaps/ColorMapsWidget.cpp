/***************************************************************************
	File                 : ColorMapsWidget.h
	Project              : LabPlot
	Description          : widget showing the available color maps
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

#include "backend/datasources/DatasetHandler.h"
#include "kdefrontend/colormaps/ColorMapsWidget.h"
#include "backend/lib/macros.h"

#include <QCompleter>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QMessageBox>
#include <QPainter>
#include <QStandardPaths>
#include <QWhatsThis>

#include <KConfigGroup>
#include <KLocalizedString>

/*!
	\class ColorMapsWidget
	\brief Widget for importing data from a dataset.

	\ingroup kdefrontend
 */
ColorMapsWidget::ColorMapsWidget(QWidget* parent) : QWidget(parent) {

	ui.setupUi(this);
	ui.bInfo->setIcon(QIcon::fromTheme(QLatin1String("help-about")));

	const int size = ui.leSearch->height();
	ui.lSearch->setPixmap( QIcon::fromTheme(QLatin1String("edit-find")).pixmap(size, size) );

	QString info = i18n("Enter the keyword you want to search for");
	ui.lSearch->setToolTip(info);
	ui.leSearch->setToolTip(info);
	ui.leSearch->setPlaceholderText(i18n("Search..."));
	ui.leSearch->setFocus();

	loadCollections();

	connect(ui.cbCollections, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
			this, &ColorMapsWidget::collectionChanged);
	connect(ui.bInfo, &QPushButton::clicked, this, &ColorMapsWidget::showInfo);
	connect(ui.lwColorMaps, &QListWidget::itemSelectionChanged, this, &ColorMapsWidget::colorMapChanged);
	connect(ui.leSearch, &QLineEdit::textChanged, this, &ColorMapsWidget::updateColorMapsList);

	//select the last used collection
	KConfigGroup conf(KSharedConfig::openConfig(), "ColorMapsWidget");
	const QString& collection = conf.readEntry("Collection", QString());
	if (collection.isEmpty())
		ui.cbCollections->setCurrentIndex(0);
	else {
		for (int i = 0; i < ui.cbCollections->count(); ++i) {
			if (ui.cbCollections->itemText(i) == collection) {
				ui.cbCollections->setCurrentIndex(i);
				break;
			}
		}

		const QString& colorMap = conf.readEntry("ColorMap", QString());
		auto items = ui.lwColorMaps->findItems(colorMap, Qt::MatchExactly);
		if (items.count() == 1)
			ui.lwColorMaps->setCurrentItem(items.constFirst());
	}
}

ColorMapsWidget::~ColorMapsWidget() {
	//save the selected collection
	KConfigGroup conf(KSharedConfig::openConfig(), "ColorMapsWidget");
	conf.writeEntry("Collection", ui.cbCollections->currentText());
	if (ui.lwColorMaps->currentItem())
		conf.writeEntry("ColorMap", ui.lwColorMaps->currentItem()->text());
}

/**
 * @brief Processes the json metadata file that contains the list of colormap collections.
 */
void ColorMapsWidget::loadCollections() {
	m_jsonDir = QStandardPaths::locate(QStandardPaths::AppDataLocation, QLatin1String("colormaps"), QStandardPaths::LocateDirectory);

	const QString& fileName = m_jsonDir + QLatin1String("/ColormapCollections.json");
	QFile file(fileName);

	if (file.open(QIODevice::ReadOnly)) {
		QJsonDocument document = QJsonDocument::fromJson(file.readAll());
		file.close();
		if (!document.isArray()) {
			QDEBUG("Invalid definition of " + fileName);
			return;
		}

		for (const QJsonValueRef col : document.array()) {
			const QJsonObject& collection = col.toObject();
			const QString& name = collection[QLatin1String("name")].toString();
			const QString& desc = collection[QLatin1String("description")].toString();
			ui.cbCollections->addItem(name);
			m_collections[name] = desc;
		}

		collectionChanged(ui.cbCollections->currentIndex());
	} else
		QMessageBox::critical(this, i18n("File not found"),
							  i18n("Couldn't open the color map collections file %1. Please check your installation.", fileName));
}

/**
 * Shows all categories and sub-categories for the currently selected collection
 */
void ColorMapsWidget::collectionChanged(int) {
	const QString& collection = ui.cbCollections->currentText();

	ui.lwColorMaps->clear();

	if (m_colorMaps.find(collection) == m_colorMaps.end()) {
		//color maps of the currently selected collection not loaded yet -> load them
		QString path = m_jsonDir + QLatin1Char('/') + collection + ".json";
		QFile collectionFile(path);
		QStringList keys;
		if (collectionFile.open(QIODevice::ReadOnly)) {
			QJsonDocument doc = QJsonDocument::fromJson(collectionFile.readAll());
			if (!doc.isObject()) {
				QDEBUG("Invalid definition of " + path);
				return;
			}

			const QJsonObject& colorMaps = doc.object();
			keys = colorMaps.keys();
			m_colorMaps[collection] = keys;

			//load colors
			for (auto key : keys) {
				if (m_colors.find(key) == m_colors.end()) {
					QStringList colors;
					auto colorsArray = colorMaps.value(key).toArray();
					for (auto color : colorsArray)
						colors << color.toString();
					m_colors[key] = colors;
				}
			}
		}
	}

	ui.lwColorMaps->addItems(m_colorMaps[collection]);
	ui.lwColorMaps->setCurrentRow(0);

	//update the completer
	if (m_completer)
		delete m_completer;

	m_completer = new QCompleter(m_colorMaps[collection], this);
	m_completer->setCompletionMode(QCompleter::PopupCompletion);
	m_completer->setCaseSensitivity(Qt::CaseSensitive);
	ui.leSearch->setCompleter(m_completer);
}

void ColorMapsWidget::colorMapChanged() {
	//convert from the string RGB represetation to QColor
	const QString& colorMap = ui.lwColorMaps->currentItem()->text();
	QVector<QColor> colors;
	for (auto& rgb : m_colors[colorMap]) {
		QStringList rgbValues = rgb.split(QLatin1Char(','));
		if (rgbValues.count() == 3)
			colors << QColor(rgbValues.at(0).toInt(), rgbValues.at(1).toInt(), rgbValues.at(2).toInt());
		else if (rgbValues.count() == 4)
			colors << QColor(rgbValues.at(1).toInt(), rgbValues.at(2).toInt(), rgbValues.at(3).toInt());
	}

	//render the preview pixmap
	int height = 200;
	int width = 80;
	int count = colors.count();
	QPixmap pixmap(width, height);
	QPainter p(&pixmap);
	int i = 0;
	for (auto& color : colors) {
		p.setPen(color);
		p.setBrush(color);
		p.drawRect(0, i*height/count,width, height/count);
		++i;
	}
	ui.lPreview->setPixmap(pixmap);
}

void ColorMapsWidget::showInfo() {
	const QString& collection = ui.cbCollections->currentText();
	const QString& info = m_collections[collection];
	QWhatsThis::showText(ui.bInfo->mapToGlobal(QPoint(0, 0)), info, ui.bInfo);
}

void ColorMapsWidget::updateColorMapsList() {
	//TODO
}
