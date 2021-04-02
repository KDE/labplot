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
#include <QStandardItemModel>
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
	ui.bViewMode->setIcon(QIcon::fromTheme(QLatin1String("view-list-icons")));
	ui.bViewMode->setToolTip(i18n("Switch between icon and list views"));

	ui.lvColorMaps->setViewMode(QListView::IconMode);
	ui.lvColorMaps->setSelectionMode(QAbstractItemView::SingleSelection);
	ui.lvColorMaps->setWordWrap(true);
	ui.lvColorMaps->setResizeMode(QListWidget::Adjust);
	ui.lvColorMaps->setDragDropMode(QListView::NoDragDrop);

	ui.lvColorMaps->setIconSize(QSize(128, 128));

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
	connect(ui.bViewMode, &QPushButton::clicked, this, &ColorMapsWidget::toggleIconView);
	connect(ui.stackedWidget, &QStackedWidget::currentChanged, this, &ColorMapsWidget::viewModeChanged);
	connect(ui.lwColorMaps, &QListWidget::itemSelectionChanged, this, &ColorMapsWidget::colorMapChanged);

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

	ui.stackedWidget->setCurrentIndex(conf.readEntry("ViewIndex", 0));
}

ColorMapsWidget::~ColorMapsWidget() {
	//save the selected collection
	KConfigGroup conf(KSharedConfig::openConfig(), "ColorMapsWidget");
	conf.writeEntry("Collection", ui.cbCollections->currentText());
	conf.writeEntry("ViewIndex", ui.stackedWidget->currentIndex());
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
			QDEBUG("Invalid definition of " + fileName)
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

	//load the collection
	if (m_colorMaps.find(collection) == m_colorMaps.end()) {
		//color maps of the currently selected collection not loaded yet -> load them
		QString path = m_jsonDir + QLatin1Char('/') + collection + ".json";
		QFile collectionFile(path);
		QStringList keys;
		if (collectionFile.open(QIODevice::ReadOnly)) {
			QJsonDocument doc = QJsonDocument::fromJson(collectionFile.readAll());
			if (!doc.isObject()) {
				QDEBUG("Invalid definition of " + path)
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

	//populate the list view for the icon mode
	if (m_model)
		delete m_model;

	m_model = new QStandardItemModel(this);
	for (const auto& name : m_colorMaps[collection]) {
		auto* item = new QStandardItem();
		QPixmap pixmap;
		render(pixmap, name);
		item->setIcon(QIcon(pixmap));
		item->setText(name);
		m_model->appendRow(item);
	}

	ui.lvColorMaps->setModel(m_model);

	//populate the list widget for the list mode
	ui.lwColorMaps->clear();
	ui.lwColorMaps->addItems(m_colorMaps[collection]);

	//select the first color map in the current collection
	ui.lvColorMaps->setCurrentIndex(ui.lvColorMaps->model()->index(0, 0));
	ui.lwColorMaps->setCurrentRow(0);

	//update the completer
	if (m_completer)
		delete m_completer;

	m_completer = new QCompleter(m_colorMaps[collection], this);
	connect(m_completer, QOverload<const QString &>::of(&QCompleter::activated), this, &ColorMapsWidget::activated);
	m_completer->setCompletionMode(QCompleter::PopupCompletion);
	m_completer->setCaseSensitivity(Qt::CaseSensitive);
	ui.leSearch->setCompleter(m_completer);
}

void ColorMapsWidget::colorMapChanged() {
	const QString& name = ui.lwColorMaps->currentItem()->text();
	render(m_pixmap, name);
	ui.lPreview->setPixmap(m_pixmap);
}

void ColorMapsWidget::render(QPixmap& pixmap, const QString& name) {
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

void ColorMapsWidget::showInfo() {
	const QString& collection = ui.cbCollections->currentText();
	const QString& info = m_collections[collection];
	QWhatsThis::showText(ui.bInfo->mapToGlobal(QPoint(0, 0)), info, ui.bInfo);
}

void ColorMapsWidget::toggleIconView() {
	if (ui.bViewMode->isChecked())
		ui.stackedWidget->setCurrentIndex(0);
	else
		ui.stackedWidget->setCurrentIndex(1);
}

void ColorMapsWidget::viewModeChanged(int index) {
	//TODO:
	if (index == 0) {
		//switching form list to icon view mode
		const auto& name = ui.lwColorMaps->currentItem()->text();
		activateIconViewItem(name);
	} else {
		//switching form icon to list view mode
		const auto& name = ui.lvColorMaps->currentIndex().data(Qt::DisplayRole).toString();
		activateListViewItem(name);
	}
}

void ColorMapsWidget::activated(const QString& name) {
	if (ui.bViewMode->isChecked())
		activateListViewItem(name);
	 else
		activateListViewItem(name);
}

void ColorMapsWidget::activateIconViewItem(const QString& name) {
	auto* model = ui.lvColorMaps->model();
	for (int i = 0; i < model->rowCount(); ++i) {
		const auto& index = model->index(i, 0);
		if (index.data(Qt::DisplayRole).toString() == name) {
			ui.lvColorMaps->setCurrentIndex(index);
			ui.lvColorMaps->scrollTo(index);
		}
	}
}

void ColorMapsWidget::activateListViewItem(const QString& name) {
	const auto& items = ui.lwColorMaps->findItems(name, Qt::MatchExactly);
	if (items.count())
		ui.lwColorMaps->setCurrentItem(items.constFirst());
}

QPixmap ColorMapsWidget::previewPixmap() {
	if (ui.stackedWidget->currentIndex() == 0) {
		const QString& name = ui.lvColorMaps->currentIndex().data(Qt::DisplayRole).toString();
		render(m_pixmap, name);
	}

	return m_pixmap;
}

QString ColorMapsWidget::name() const {
	if (ui.stackedWidget->currentIndex() == 0)
		return ui.lvColorMaps->currentIndex().data(Qt::DisplayRole).toString();
	else
		return ui.lwColorMaps->currentItem()->text();
}

QVector<QColor> ColorMapsWidget::colors() const {
	return m_colormap;
}
