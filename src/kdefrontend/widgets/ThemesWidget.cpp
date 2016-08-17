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

#include <QListWidgetItem>
#include <QStandardItemModel>
#include <QFile>
#include <KStandardDirs>

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

	//TODO: make this pixel-independent
	setIconSize(QSize(200,200));
	setMaximumWidth(225);

	//show preview pixmaps
	QStandardItemModel* mContentItemModel = new QStandardItemModel(this);
	QStringList themeList = ThemeHandler::themes();
	QString themeImgPath = KGlobal::dirs()->findDirs("data", "labplot2/themes/").first();
	QString tempPath;
	for (int i = 0; i < themeList.size(); ++i) {
		QStandardItem* listItem = new QStandardItem();
		tempPath = themeImgPath + "screenshots/" + themeList.at(i) + ".png";
		if(!QFile::exists(tempPath))
			tempPath = themeImgPath + "screenshots/Unavailable.png";

		listItem->setIcon(QIcon(QPixmap(tempPath)));
		listItem->setText(themeList.at(i));
		listItem->setData(themeList.at(i), Qt::UserRole);
		mContentItemModel->appendRow(listItem);
	}

	setModel(mContentItemModel);

	//SLOTS
	connect( this, SIGNAL(clicked(QModelIndex)), this, SLOT(applyClicked()) );
}

void ThemesWidget::applyClicked() {
	QString themeName = currentIndex().data(Qt::UserRole).value<QString>();
	emit(themeSelected(themeName));
}
