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
#include <QListWidgetItem>
#include <QStandardItemModel>
#include <QFile>

#include <KGlobal>
#include <KStandardDirs>
#include <KMessageBox>
#include <kdebug.h>
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

	//make the icon 3x3cm big and show two of them in the height
	int size = 3.0/2.54 * QApplication::desktop()->physicalDpiX();
	setIconSize(QSize(size, size));
	setMinimumSize(1.1*size, 2.1*size); //add some offset here to take care of potential scrollbars, etc. (not very precise...)
	setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

	//show preview pixmaps
	QStandardItemModel* mContentItemModel = new QStandardItemModel(this);
	QStringList themeList = ThemeHandler::themes();
	QStringList themeImgPathList = QStandardPaths::locateAll(QStandardPaths::GenericDataLocation, "labplot2/themes/screenshots/", QStandardPaths::LocateDirectory);
	themeImgPathList.append(QStandardPaths::locateAll(QStandardPaths::DataLocation, "themes/screenshots/", QStandardPaths::LocateDirectory));
	if (themeImgPathList.isEmpty())
		return;
	QString themeImgPath = themeImgPathList.first();
	QString tempPath;

	for (int i = 0; i < themeList.size(); ++i) {
		QStandardItem* listItem = new QStandardItem();

		tempPath = themeImgPath + themeList.at(i) + ".png";
		if (!QFile::exists(tempPath))
			tempPath = themeImgPath + "Unavailable.png";

		listItem->setIcon(QIcon(QPixmap(tempPath)));
		listItem->setText(themeList.at(i));
		listItem->setData(themeList.at(i), Qt::UserRole);
		mContentItemModel->appendRow(listItem);
	}

	//adding download themes option
	//TODO: activate this later
// 	QStandardItem* listItem = new QStandardItem();
// 	listItem->setIcon(QIcon::fromTheme("get-hot-new-stuff"));
// 	listItem->setText("Download Themes");
// 	listItem->setData("file_download_theme", Qt::UserRole);
// 	mContentItemModel->appendRow(listItem);

	setModel(mContentItemModel);

	//SLOTS
	connect(this, SIGNAL(clicked(QModelIndex)), this, SLOT(applyClicked()));
}

void ThemesWidget::applyClicked() {
	QString themeName = currentIndex().data(Qt::UserRole).value<QString>();
	//TODO: activate this later
// 	if(themeName=="file_download_theme")
// 		this->downloadThemes();
// 	else
		emit(themeSelected(themeName));
}

//TODO: activate this later
// void ThemesWidget::downloadThemes() {
// 	KNS3::DownloadDialog dialog("labplot2_themes.knsrc", this);
// 	dialog.exec();
// 	foreach (const KNS3::Entry& e, dialog.changedEntries()) {
// 	    kDebug() << "Changed Entry: " << e.name();
// 	}
// }
