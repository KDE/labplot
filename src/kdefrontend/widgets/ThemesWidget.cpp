/***************************************************************************
    File                 : ThemesWidget.cpp
    Project              : LabPlot
    Description          : widget for selecting themes
    --------------------------------------------------------------------
    Copyright            : (C) 2016 Prakriti Bhardwaj (p_bhardwaj14@informatik.uni-kl.de)
    Copyright            : (C) 2016 Alexander Semke (alexander.semke@web.de)

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
	int size = 3.0/2.54 * QApplication::desktop()->physicalDpiX();
	setIconSize(QSize(size, size));
	setMinimumSize(1.1*size, 2.1*size); //add some offset here to take care of potential scrollbars, etc. (not very precise...)
	setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

	//show preview pixmaps
	auto* mContentItemModel = new QStandardItemModel(this);
	QStringList themeList = ThemeHandler::themes();
	QStringList themeImgPathList = QStandardPaths::locateAll(QStandardPaths::DataLocation, "themes/screenshots/", QStandardPaths::LocateDirectory);
	if (themeImgPathList.isEmpty())
		return;

	const QString& themeImgPath = themeImgPathList.first();
	QString tempPath;

	for (int i = 0; i < themeList.size(); ++i) {
		auto* listItem = new QStandardItem();

		tempPath = themeImgPath + themeList.at(i) + ".png";
		if (!QFile::exists(tempPath))
			tempPath = themeImgPath + "Unavailable.png";

		listItem->setIcon(QIcon(QPixmap(tempPath)));
		listItem->setText(themeList.at(i));
		mContentItemModel->appendRow(listItem);
	}

	//create and add the icon for "None"
	QPixmap pm(size, size);
	QPen pen(Qt::SolidPattern, 1);
	const QColor& color = (palette().color(QPalette::Base).lightness() < 128) ? Qt::white : Qt::black;
	pen.setColor(color);
	QPainter pa;
	pm.fill(Qt::transparent);
	pa.begin(&pm);
	pa.setPen(pen);
	pa.drawRect(5, 5, size-10, size-10);
	pa.end();

	auto* listItem = new QStandardItem();
	listItem->setIcon(pm);
	listItem->setText(i18n("None"));
	mContentItemModel->appendRow(listItem);

	//adding download themes option
	//TODO: activate this later
// 	QStandardItem* listItem = new QStandardItem();
// 	listItem->setIcon(QIcon::fromTheme("get-hot-new-stuff"));
// 	listItem->setText("Download Themes");
// 	listItem->setData("file_download_theme", Qt::UserRole);
// 	mContentItemModel->appendRow(listItem);

	setModel(mContentItemModel);

	//SLOTS
	connect(this, &ThemesWidget::clicked, this, &ThemesWidget::applyClicked);
}

void ThemesWidget::applyClicked(const QModelIndex& index) {
	const QString& themeName = index.data(Qt::DisplayRole).toString();

	//TODO: activate this later
// 	if (themeName == "file_download_theme")
// 		this->downloadThemes();
// 	else

	if (index.row() == model()->rowCount()-1)
		emit themeSelected(QString()); //item with the string "None" was selected -> no theme
	else
		emit themeSelected(themeName);
}

//TODO: activate this later
// void ThemesWidget::downloadThemes() {
// 	KNS3::DownloadDialog dialog("labplot2_themes.knsrc", this);
// 	dialog.exec();
// 	foreach (const KNS3::Entry& e, dialog.changedEntries()) {
// 	    kDebug() << "Changed Entry: " << e.name();
// 	}
// }
