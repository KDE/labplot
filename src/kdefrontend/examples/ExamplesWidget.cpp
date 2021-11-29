/*
    File                 : ExamplesWidget.h
    Project              : LabPlot
    Description          : widget showing the available color maps
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2021 Alexander Semke <alexander.semke@web.de>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "kdefrontend/examples/ExamplesWidget.h"
#include "kdefrontend/examples/ExamplesManager.h"
#include "backend/lib/macros.h"

#include <QCompleter>
#include <QMessageBox>
#include <QStandardItemModel>
#include <QWhatsThis>

#include <KConfigGroup>
#include <KLocalizedString>

/*!
	\class ExamplesWidget
	\brief Widget showing all available example projects.

	\ingroup kdefrontend
 */
ExamplesWidget::ExamplesWidget(QWidget* parent) : QWidget(parent) {
	ui.setupUi(this);
	ui.bInfo->setIcon(QIcon::fromTheme(QLatin1String("help-about")));
	ui.bViewMode->setIcon(QIcon::fromTheme(QLatin1String("view-list-icons")));
	ui.bViewMode->setToolTip(i18n("Switch between icon and list views"));

	ui.lvExamples->setViewMode(QListView::IconMode);
	ui.lvExamples->setSelectionMode(QAbstractItemView::SingleSelection);
	ui.lvExamples->setWordWrap(true);
	ui.lvExamples->setResizeMode(QListWidget::Adjust);
	ui.lvExamples->setDragDropMode(QListView::NoDragDrop);
	ui.lvExamples->setEditTriggers(QAbstractItemView::NoEditTriggers);
	ui.lvExamples->setIconSize(QSize(256, 256));
	connect(ui.lvExamples, &QAbstractItemView::doubleClicked, this, &ExamplesWidget::doubleClicked);

	ui.lPreview->setScaledContents(false);
	ui.lPreview->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);

	const int size = ui.leSearch->height();
	ui.lSearch->setPixmap( QIcon::fromTheme(QLatin1String("edit-find")).pixmap(size, size) );

	QString info = i18n("Enter the keyword you want to search for");
	ui.lSearch->setToolTip(info);
	ui.leSearch->setToolTip(info);
	ui.leSearch->setPlaceholderText(i18n("Search..."));
	ui.leSearch->setFocus();

	m_manager = ExamplesManager::instance();
	ui.cbCollections->addItems(m_manager->collectionNames());

	connect(ui.cbCollections, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
			this, &ExamplesWidget::collectionChanged);
	connect(ui.bInfo, &QPushButton::clicked, this, &ExamplesWidget::showInfo);
	connect(ui.bViewMode, &QPushButton::clicked, this, &ExamplesWidget::toggleIconView);
	connect(ui.stackedWidget, &QStackedWidget::currentChanged, this, &ExamplesWidget::viewModeChanged);
	connect(ui.lwExamples, &QListWidget::itemSelectionChanged, this, &ExamplesWidget::exampleChanged);

	//select the last used collection
	KConfigGroup conf(KSharedConfig::openConfig(), "ExamplesWidget");
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

		const QString& colorMap = conf.readEntry("Example", QString());
		auto items = ui.lwExamples->findItems(colorMap, Qt::MatchExactly);
		if (items.count() == 1)
			ui.lwExamples->setCurrentItem(items.constFirst());
	}

	collectionChanged(ui.cbCollections->currentIndex());

	ui.stackedWidget->setCurrentIndex(conf.readEntry("ViewIndex", 0));
}

ExamplesWidget::~ExamplesWidget() {
	//save the selected collection
	KConfigGroup conf(KSharedConfig::openConfig(), "ExamplesWidget");
	conf.writeEntry("Collection", ui.cbCollections->currentText());
	conf.writeEntry("ViewIndex", ui.stackedWidget->currentIndex());
	if (ui.lwExamples->currentItem())
		conf.writeEntry("Example", ui.lwExamples->currentItem()->text());
}

/**
 * Shows all categories and sub-categories for the currently selected collection
 */
void ExamplesWidget::collectionChanged(int) {
	const QString& collection = ui.cbCollections->currentText();

	//populate the list view for the icon mode
	if (m_model)
		delete m_model;

	m_model = new QStandardItemModel(this);
	const auto& exampleNames = m_manager->exampleNames(collection);
	for (const auto& name : exampleNames) {
		auto* item = new QStandardItem();
		item->setIcon(QIcon(m_manager->pixmap(name)));
		item->setText(name);
		item->setToolTip(m_manager->description(name));
		m_model->appendRow(item);
	}

	ui.lvExamples->setModel(m_model);

	//populate the list widget for the list mode
	ui.lwExamples->clear();
	ui.lwExamples->addItems(exampleNames);

	//select the first example in the current collection
	ui.lvExamples->setCurrentIndex(ui.lvExamples->model()->index(0, 0));
	ui.lwExamples->setCurrentRow(0);

	//update the completer
	if (m_completer)
		delete m_completer;

	m_completer = new QCompleter(exampleNames, this);
	connect(m_completer, QOverload<const QString &>::of(&QCompleter::activated), this, &ExamplesWidget::activated);
	m_completer->setCompletionMode(QCompleter::PopupCompletion);
	m_completer->setCaseSensitivity(Qt::CaseSensitive);
	ui.leSearch->setCompleter(m_completer);
}

void ExamplesWidget::exampleChanged() {
	const QString& name = ui.lwExamples->currentItem()->text();
	ui.lPreview->setPixmap(m_manager->pixmap(name));
}

void ExamplesWidget::showInfo() {
	const QString& collection = ui.cbCollections->currentText();
	const QString& info = m_manager->collectionInfo(collection);
	QWhatsThis::showText(ui.bInfo->mapToGlobal(QPoint(0, 0)), info, ui.bInfo);
}

void ExamplesWidget::toggleIconView() {
	if (ui.bViewMode->isChecked())
		ui.stackedWidget->setCurrentIndex(0);
	else
		ui.stackedWidget->setCurrentIndex(1);
}

void ExamplesWidget::viewModeChanged(int index) {
	if (index == 0) {
		//switching form list to icon view mode
		if (ui.lwExamples->currentItem()) {
			const auto& name = ui.lwExamples->currentItem()->text();
			activateIconViewItem(name);
		}
	} else {
		//switching form icon to list view mode
		if (ui.lvExamples->currentIndex().isValid()) {
			const auto& name = ui.lvExamples->currentIndex().data(Qt::DisplayRole).toString();
			activateListViewItem(name);
		}
	}
}

void ExamplesWidget::activated(const QString& name) {
	if (ui.bViewMode->isChecked())
		activateIconViewItem(name);
	 else
		activateListViewItem(name);
}

void ExamplesWidget::activateIconViewItem(const QString& name) {
	auto* model = ui.lvExamples->model();
	for (int i = 0; i < model->rowCount(); ++i) {
		const auto& index = model->index(i, 0);
		if (index.data(Qt::DisplayRole).toString() == name) {
			ui.lvExamples->setCurrentIndex(index);
			ui.lvExamples->scrollTo(index);
		}
	}
}

void ExamplesWidget::activateListViewItem(const QString& name) {
	const auto& items = ui.lwExamples->findItems(name, Qt::MatchExactly);
	if (items.count())
		ui.lwExamples->setCurrentItem(items.constFirst());
}

/*!
 * returns the path of the currently selected example project.
 */
QString ExamplesWidget::path() const {
	QString name;
	if (ui.stackedWidget->currentIndex() == 0)
		name = ui.lvExamples->currentIndex().data(Qt::DisplayRole).toString();
	else
		name = ui.lwExamples->currentItem()->text();

	return m_manager->path(name);
}
