/*
	File                 : ColorMapsWidget.h
	Project              : LabPlot
	Description          : widget showing the available color maps
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2021 Alexander Semke <alexander.semke@web.de>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "kdefrontend/colormaps/ColorMapsWidget.h"
#include "backend/core/Settings.h"
#include "backend/lib/macros.h"
#include "tools/ColorMapsManager.h"

#include <QCompleter>
#include <QMessageBox>
#include <QPainter>
#include <QStandardItemModel>
#include <QWhatsThis>

#include <KConfigGroup>
#include <KLocalizedString>

/*!
	\class ColorMapsWidget
	\brief Widget for importing data from a dataset.

	\ingroup kdefrontend
 */
ColorMapsWidget::ColorMapsWidget(QWidget* parent)
	: QWidget(parent) {
	ui.setupUi(this);
	ui.bInfo->setIcon(QIcon::fromTheme(QLatin1String("help-about")));
	ui.bViewMode->setIcon(QIcon::fromTheme(QLatin1String("view-list-icons")));
	ui.bViewMode->setToolTip(i18n("Switch between icon and list views"));

	ui.lvColorMaps->setViewMode(QListView::IconMode);
	ui.lvColorMaps->setSelectionMode(QAbstractItemView::SingleSelection);
	ui.lvColorMaps->setWordWrap(true);
	ui.lvColorMaps->setResizeMode(QListWidget::Adjust);
	ui.lvColorMaps->setDragDropMode(QListView::NoDragDrop);
	ui.lvColorMaps->setEditTriggers(QAbstractItemView::NoEditTriggers);
	ui.lvColorMaps->setIconSize(QSize(128, 128));
	connect(ui.lvColorMaps, &QAbstractItemView::doubleClicked, this, &ColorMapsWidget::doubleClicked);

	const int size = ui.leSearch->height();
	ui.lSearch->setPixmap(QIcon::fromTheme(QLatin1String("edit-find")).pixmap(size, size));

	QString info = i18n("Enter the keyword you want to search for");
	ui.lSearch->setToolTip(info);
	ui.leSearch->setToolTip(info);
	ui.leSearch->setPlaceholderText(i18n("Search..."));
	ui.leSearch->setFocus();

	m_manager = ColorMapsManager::instance();
	ui.cbCollections->addItems(m_manager->collectionNames());

	connect(ui.cbCollections, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &ColorMapsWidget::collectionChanged);
	connect(ui.bInfo, &QPushButton::clicked, this, &ColorMapsWidget::showInfo);
	connect(ui.bViewMode, &QPushButton::clicked, this, &ColorMapsWidget::toggleIconView);
	connect(ui.stackedWidget, &QStackedWidget::currentChanged, this, &ColorMapsWidget::viewModeChanged);
	connect(ui.lwColorMaps, &QListWidget::itemSelectionChanged, this, &ColorMapsWidget::colorMapChanged);

	// select the last used collection
	KConfigGroup conf = Settings::group(QStringLiteral("ColorMapsWidget"));
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

	collectionChanged(ui.cbCollections->currentIndex());

	ui.stackedWidget->setCurrentIndex(conf.readEntry("ViewIndex", 0));
}

ColorMapsWidget::~ColorMapsWidget() {
	// save the selected collection
	KConfigGroup conf = Settings::group(QStringLiteral("ColorMapsWidget"));
	conf.writeEntry("Collection", ui.cbCollections->currentText());
	conf.writeEntry("ViewIndex", ui.stackedWidget->currentIndex());
	if (ui.lwColorMaps->currentItem())
		conf.writeEntry("ColorMap", ui.lwColorMaps->currentItem()->text());
}

/**
 * Shows all categories and sub-categories for the currently selected collection
 */
void ColorMapsWidget::collectionChanged(int) {
	const QString& collection = ui.cbCollections->currentText();

	// populate the list view for the icon mode
	if (m_model)
		delete m_model;

	m_model = new QStandardItemModel(this);
	const auto& colorMapNames = m_manager->colorMapNames(collection);
	for (const auto& name : colorMapNames) {
		auto* item = new QStandardItem();
		QPixmap pixmap;
		m_manager->render(pixmap, name);
		item->setIcon(QIcon(pixmap));
		item->setText(name);
		m_model->appendRow(item);
	}

	ui.lvColorMaps->setModel(m_model);

	// populate the list widget for the list mode
	ui.lwColorMaps->clear();
	ui.lwColorMaps->addItems(colorMapNames);

	// select the first color map in the current collection
	ui.lvColorMaps->setCurrentIndex(ui.lvColorMaps->model()->index(0, 0));
	ui.lwColorMaps->setCurrentRow(0);

	// update the completer
	if (m_completer)
		delete m_completer;

	m_completer = new QCompleter(colorMapNames, this);
	connect(m_completer, QOverload<const QString&>::of(&QCompleter::activated), this, &ColorMapsWidget::activated);
	m_completer->setCompletionMode(QCompleter::PopupCompletion);
	m_completer->setCaseSensitivity(Qt::CaseInsensitive);
	m_completer->setFilterMode(Qt::MatchContains);
	ui.leSearch->setCompleter(m_completer);
}

void ColorMapsWidget::colorMapChanged() {
	const QString& name = ui.lwColorMaps->currentItem()->text();
	m_manager->render(m_pixmap, name);
	ui.lPreview->setPixmap(m_pixmap);
}

void ColorMapsWidget::showInfo() {
	const QString& collection = ui.cbCollections->currentText();
	const QString& info = m_manager->collectionInfo(collection);
	QWhatsThis::showText(ui.bInfo->mapToGlobal(QPoint(0, 0)), info, ui.bInfo);
}

void ColorMapsWidget::toggleIconView() {
	if (ui.bViewMode->isChecked())
		ui.stackedWidget->setCurrentIndex(0);
	else
		ui.stackedWidget->setCurrentIndex(1);
}

void ColorMapsWidget::viewModeChanged(int index) {
	if (index == 0) {
		// switching form list to icon view mode
		if (ui.lwColorMaps->currentItem()) {
			const auto& name = ui.lwColorMaps->currentItem()->text();
			activateIconViewItem(name);
		}
	} else {
		// switching form icon to list view mode
		if (ui.lvColorMaps->currentIndex().isValid()) {
			const auto& name = ui.lvColorMaps->currentIndex().data(Qt::DisplayRole).toString();
			activateListViewItem(name);
		}
	}
}

void ColorMapsWidget::activated(const QString& name) {
	if (ui.bViewMode->isChecked())
		activateIconViewItem(name);
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
	if (ui.stackedWidget->currentIndex() == 0 && ui.lvColorMaps->currentIndex().isValid()) {
		const QString& name = ui.lvColorMaps->currentIndex().data(Qt::DisplayRole).toString();
		m_manager->render(m_pixmap, name);
	}

	return m_pixmap;
}

/*!
 * returns the name of the currently selected color map.
 */
QString ColorMapsWidget::name() const {
	if (ui.stackedWidget->currentIndex() == 0) {
		if (ui.lvColorMaps->currentIndex().isValid())
			return ui.lvColorMaps->currentIndex().data(Qt::DisplayRole).toString();
	} else {
		if (ui.lwColorMaps->currentItem())
			return ui.lwColorMaps->currentItem()->text();
	}

	return {};
}

/*!
 * returns the vector with the colors of the currently selected color map.
 */
QVector<QColor> ColorMapsWidget::colors() const {
	return m_manager->colors();
}
