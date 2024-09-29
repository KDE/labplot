/*
	File                 : ColorMapsWidget.h
	Project              : LabPlot
	Description          : widget showing the available color maps
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2021-2024 Alexander Semke <alexander.semke@web.de>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "kdefrontend/colormaps/ColorMapsWidget.h"
#include "backend/core/Settings.h"
#include "backend/lib/macros.h"
#include "tools/ColorMapsManager.h"

#include <QCompleter>
#include <QMenu>
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
	connect(ui.bViewMode, &QPushButton::clicked, this, &ColorMapsWidget::showViewModeMenu);
	connect(ui.lwColorMaps, &QListWidget::itemSelectionChanged, this, &ColorMapsWidget::colorMapChanged);
	connect(ui.lwColorMapsDetails, &QListWidget::itemSelectionChanged, this, &ColorMapsWidget::colorMapDetailsChanged);

	// select the last used collection
	KConfigGroup conf = Settings::group(QStringLiteral("ColorMapsWidget"));
	m_viewMode = static_cast<ViewMode>(conf.readEntry("ViewMode", static_cast<int>(ViewMode::IconView)));
	switchViewMode();

	// available collections
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

		collectionChanged(ui.cbCollections->currentIndex());

		// select the last selected color map in the current view
		const QString& colorMap = conf.readEntry("ColorMap", QString());
		switch (m_viewMode) {
		case ViewMode::IconView:
			activateIconViewItem(colorMap);
			break;
		case ViewMode::ListView:
			activateListViewItem(colorMap);
			break;
		case ViewMode::ListDetailsView:
			activateListDetailsViewItem(colorMap);
			break;
		}
	}
}

ColorMapsWidget::~ColorMapsWidget() {
	// save the selected collection
	KConfigGroup conf = Settings::group(QStringLiteral("ColorMapsWidget"));
	conf.writeEntry("ViewMode", static_cast<int>(m_viewMode));
	conf.writeEntry("Collection", ui.cbCollections->currentText());
	conf.writeEntry("ColorMap", colorMapName());
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

	// populate the list widget for the list details mode
	ui.lwColorMapsDetails->clear();
	ui.lwColorMapsDetails->addItems(colorMapNames);

	// select the first color map in the current collection
	ui.lvColorMaps->setCurrentIndex(ui.lvColorMaps->model()->index(0, 0));
	ui.lwColorMaps->setCurrentRow(0);
	ui.lwColorMapsDetails->setCurrentRow(0);

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

/*!
 * called when the current selected color map in the list of maps in the Details view was changed.
 * updates the detailed information in the table widget for the new selected map.
 */
void ColorMapsWidget::colorMapDetailsChanged() {
	const QString& name = ui.lwColorMapsDetails->currentItem()->text();
	m_manager->render(m_pixmap, name); // trigger this to get the list of colors below. TODO: redesign the API to get the colors directly.
	const auto& colors = m_manager->colors();

	ui.twColorMapDetails->clear();
	static QStringList
		columnNames{i18n("Color"), QLatin1String("Hex"), i18n("Red"), i18n("Green"), i18n("Blue"), i18n("Hue"), i18n("Saturation"), i18n("Value")};
	ui.twColorMapDetails->setHorizontalHeaderLabels(columnNames);
	ui.twColorMapDetails->setRowCount(colors.count());

	int row = 0;
	for (const auto& color : colors) {
		auto* item = new QTableWidgetItem();
		item->setBackground(color);
		item->setFlags(item->flags() ^ Qt::ItemIsEditable);
		ui.twColorMapDetails->setItem(row, 0, item);

		item = new QTableWidgetItem(color.name(QColor::HexRgb));
		item->setFlags(item->flags() ^ Qt::ItemIsEditable);
		ui.twColorMapDetails->setItem(row, 1, item);

		item = new QTableWidgetItem(QString::number(color.red()));
		item->setFlags(item->flags() ^ Qt::ItemIsEditable);
		ui.twColorMapDetails->setItem(row, 2, item);

		item = new QTableWidgetItem(QString::number(color.green()));
		item->setFlags(item->flags() ^ Qt::ItemIsEditable);
		ui.twColorMapDetails->setItem(row, 3, item);

		item = new QTableWidgetItem(QString::number(color.blue()));
		item->setFlags(item->flags() ^ Qt::ItemIsEditable);
		ui.twColorMapDetails->setItem(row, 4, item);

		item = new QTableWidgetItem(QString::number(color.hue()));
		item->setFlags(item->flags() ^ Qt::ItemIsEditable);
		ui.twColorMapDetails->setItem(row, 5, item);

		item = new QTableWidgetItem(QString::number(color.saturation()));
		item->setFlags(item->flags() ^ Qt::ItemIsEditable);
		ui.twColorMapDetails->setItem(row, 6, item);

		item = new QTableWidgetItem(QString::number(color.value()));
		item->setFlags(item->flags() ^ Qt::ItemIsEditable);
		ui.twColorMapDetails->setItem(row, 7, item);

		++row;
	}
}

void ColorMapsWidget::showInfo() {
	const QString& collection = ui.cbCollections->currentText();
	const QString& info = m_manager->collectionInfo(collection);
	QWhatsThis::showText(ui.bInfo->mapToGlobal(QPoint(0, 0)), info, ui.bInfo);
}

void ColorMapsWidget::showViewModeMenu() {
	QMenu menu;

	auto* action = new QAction(QIcon::fromTheme(QLatin1String("view-list-icons")), i18n("Icon View"), &menu);
	action->setData(static_cast<int>(ViewMode::IconView));
	action->setCheckable(true);
	if (m_viewMode == ViewMode::IconView)
		action->setChecked(true);
	connect(action, &QAction::triggered, this, &ColorMapsWidget::viewModeChanged);
	menu.addAction(action);

	action = new QAction(QIcon::fromTheme(QLatin1String("view-list-details")), i18n("List View"), &menu);
	action->setData(static_cast<int>(ViewMode::ListView));
	action->setCheckable(true);
	if (m_viewMode == ViewMode::ListView)
		action->setChecked(true);
	connect(action, &QAction::triggered, this, &ColorMapsWidget::viewModeChanged);
	menu.addAction(action);

	action = new QAction(QIcon::fromTheme(QLatin1String("table")), i18n("List View with Details"), &menu);
	action->setData(static_cast<int>(ViewMode::ListDetailsView));
	action->setCheckable(true);
	if (m_viewMode == ViewMode::ListDetailsView)
		action->setChecked(true);
	connect(action, &QAction::triggered, this, &ColorMapsWidget::viewModeChanged);
	menu.addAction(action);

	QPoint pos(-menu.sizeHint().width() + ui.bViewMode->width(), -menu.sizeHint().height());
	menu.exec(ui.bViewMode->mapToGlobal(pos));
}

void ColorMapsWidget::viewModeChanged() {
	auto* action = static_cast<QAction*>(QObject::sender());
	m_viewMode = static_cast<ViewMode>(action->data().toInt());
	switchViewMode();
}

void ColorMapsWidget::switchViewMode() {
	// switch the view and show the information for the current color map
	switch (m_viewMode) {
	case ViewMode::IconView: {
		activateIconViewItem(colorMapName());
		ui.stackedWidget->setCurrentIndex(0);
		break;
	}
	case ViewMode::ListView: {
		activateListViewItem(colorMapName());
		ui.stackedWidget->setCurrentIndex(1);
		break;
	}
	case ViewMode::ListDetailsView: {
		activateListDetailsViewItem(colorMapName());
		ui.stackedWidget->setCurrentIndex(2);
		break;
	}
	}
}

/*!
 * returns the name of the currently selected color map
 */
QString ColorMapsWidget::colorMapName() const {
	QString name;
	if (ui.stackedWidget->currentIndex() == 0) {
		if (ui.lvColorMaps->currentIndex().isValid())
			name = ui.lvColorMaps->currentIndex().data(Qt::DisplayRole).toString();
	} else if (ui.stackedWidget->currentIndex() == 1) {
		if (ui.lwColorMaps->currentItem())
			name = ui.lwColorMaps->currentItem()->text();
	} else {
		if (ui.lwColorMapsDetails->currentIndex().isValid())
			name = ui.lwColorMapsDetails->currentItem()->text();
	}

	return name;
}

void ColorMapsWidget::activated(const QString& name) {
	switch (m_viewMode) {
	case ViewMode::IconView:
		activateIconViewItem(name);
		break;
	case ViewMode::ListView:
		activateListViewItem(name);
		break;
	case ViewMode::ListDetailsView:
		activateListDetailsViewItem(name);
		break;
	}
}

void ColorMapsWidget::activateIconViewItem(const QString& name) {
	auto* model = ui.lvColorMaps->model();
	if (!model)
		return;

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

void ColorMapsWidget::activateListDetailsViewItem(const QString& name) {
	const auto& items = ui.lwColorMapsDetails->findItems(name, Qt::MatchExactly);
	if (items.count())
		ui.lwColorMapsDetails->setCurrentItem(items.constFirst());
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
	switch (m_viewMode) {
	case ViewMode::IconView:
		if (ui.lvColorMaps->currentIndex().isValid())
			return ui.lvColorMaps->currentIndex().data(Qt::DisplayRole).toString();
		break;
	case ViewMode::ListView:
		if (ui.lwColorMaps->currentItem())
			return ui.lwColorMaps->currentItem()->text();
		break;
	case ViewMode::ListDetailsView:
		if (ui.lwColorMapsDetails->currentItem())
			return ui.lwColorMapsDetails->currentItem()->text();
		break;
	}

	return {};
}

/*!
 * returns the vector with the colors of the currently selected color map.
 */
QVector<QColor> ColorMapsWidget::colors() const {
	return m_manager->colors();
}
