/*
    File                 : ThemesWidget.cpp
    Project              : LabPlot
    Description          : widget for selecting themes
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2016 Prakriti Bhardwaj <p_bhardwaj14@informatik.uni-kl.de>
    SPDX-FileCopyrightText: 2016 Alexander Semke <alexander.semke@web.de>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "ThemesWidget.h"
#include "kdefrontend/ThemeHandler.h"

#include <QApplication>
#include <QDesktopWidget>
#include <QFile>
#include <QListWidgetItem>
#include <QPainter>
#include <QStandardItemModel>
#include <QStandardPaths>

#include <KLocalizedString>
#include <KMessageBox>
// #include <knewstuff3/downloaddialog.h>

#include <cmath>

/*!
	\class ThemesWidget
	\brief Widget for showing theme previews and for selecting a theme.

	\ingroup kdefrontend
 */
ThemesWidget::ThemesWidget(QWidget* parent) : QListView(parent) {
	setSelectionMode(QAbstractItemView::SingleSelection);
	setWordWrap(true);
	setViewMode(QListWidget::IconMode);
	setResizeMode(QListWidget::Adjust);
	setDragDropMode(QListView::NoDragDrop);

	//make the icon 3x3cm big and show two of them in the height
	static const int themeIconSize = std::ceil(3.0/2.54 * QApplication::desktop()->physicalDpiX());
	setIconSize(QSize(themeIconSize, themeIconSize));
	setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

	//show preview pixmaps
	auto* mContentItemModel = new QStandardItemModel(this);
	QStringList themeList = ThemeHandler::themes();
	QStringList themeImgPathList = QStandardPaths::locateAll(QStandardPaths::DataLocation, "themes/screenshots/", QStandardPaths::LocateDirectory);
	if (themeImgPathList.isEmpty()) {
		delete mContentItemModel;
		return;
	}

	const QString& themeImgPath = themeImgPathList.first();
	QString tempPath;

	for (int i = 0; i < themeList.size(); ++i) {
		auto* listItem = new QStandardItem();

		tempPath = themeImgPath + themeList.at(i) + ".png";
		if (!QFile::exists(tempPath))
			tempPath = themeImgPath + "Unavailable.png";

		listItem->setIcon(QIcon(QPixmap(tempPath)));
		if (themeList.at(i) == QLatin1String("Default")) {
			listItem->setText(i18n("Default"));
			mContentItemModel->insertRow(0, listItem);
		} else {
			listItem->setText(themeList.at(i));
			mContentItemModel->appendRow(listItem);
		}
	}

	//adding download themes option
	//TODO: activate this later
// 	QStandardItem* listItem = new QStandardItem();
// 	listItem->setIcon(QIcon::fromTheme("get-hot-new-stuff"));
// 	listItem->setText("Download Themes");
// 	listItem->setData("file_download_theme", Qt::UserRole);
// 	mContentItemModel->appendRow(listItem);

	QListView::setModel(mContentItemModel);

	//SLOTS
	connect(this, &ThemesWidget::clicked, this, &ThemesWidget::applyClicked);
}

void ThemesWidget::applyClicked(const QModelIndex& index) {
	const QString& themeName = index.data(Qt::DisplayRole).toString();

	//TODO: activate this later
// 	if (themeName == "file_download_theme")
// 		this->downloadThemes();
// 	else

	if (index.row() == 0)
		Q_EMIT themeSelected(QString()); //item with the string "None" was selected -> no theme
	else
		Q_EMIT themeSelected(themeName);
}

//TODO: activate this later
// void ThemesWidget::downloadThemes() {
// 	KNS3::DownloadDialog dialog("labplot2_themes.knsrc", this);
// 	dialog.exec();
// 	foreach (const KNS3::Entry& e, dialog.changedEntries()) {
// 	    kDebug() << "Changed Entry: " << e.name();
// 	}
// }

void ThemesWidget::setFixedMode() {
	//resize the widget to show three items only
	int frameWidth = style()->pixelMetric(QStyle::PM_DefaultFrameWidth);
	QFont font;
	QFontMetrics fm(font);
	static const int themeIconSize = std::ceil(3.0/2.54 * QApplication::desktop()->physicalDpiX());
	QSize widgetSize(themeIconSize + style()->pixelMetric(QStyle::PM_ScrollBarExtent) + frameWidth*2,
					 3*(themeIconSize + fm.height() + 2* frameWidth) + fm.height() + frameWidth);
	setMinimumSize(widgetSize);
	setMaximumSize(widgetSize);
}
