/***************************************************************************
    File                 : ThemesWidget.cpp
    Project              : LabPlot
    Description          : widget for selecting themes
    --------------------------------------------------------------------
    Copyright            : (C) 2014 Alexander Semke (alexander.semke@web.de)

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
#include <QListWidget>
#include <QListWidgetItem>
#include <QStandardItemModel>
#include <QDebug>
/*!
	\class ThemesWidget
	\brief Widget for showing theme previews and for selecting a theme.

	\ingroup kdefrontend
 */
ThemesWidget::ThemesWidget(QWidget *parent): QWidget(parent) {
	ui.setupUi(this);
	ui.bApply->setIcon(KIcon("edit-paste"));
	ui.bCancel->setIcon(KIcon("dialog-cancel"));

	//SLOTS
	connect( ui.bApply, SIGNAL(clicked(bool)), this, SLOT(applyClicked()) );
	connect( ui.bCancel, SIGNAL(clicked(bool)), this, SIGNAL(canceled()) );
	connect( ui.lvThemes, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(applyClicked()) );

}

void ThemesWidget::setupPreview(QStringList themeList, QString themeImgPath) {
	QStandardItemModel* mContentItemModel = new QStandardItemModel(this);
	QString tempPath = themeImgPath;
	for (int i = 0; i < themeList.size(); ++i) {
		QStandardItem* listItem = new QStandardItem();
		tempPath = tempPath+"themes/"+QVariant(themeList.at(i)).toString()+".png";
		QPixmap placeHolderMap(tempPath);
		listItem->setIcon(QIcon(placeHolderMap));
		listItem->setText(QVariant(themeList.at(i)).toString());
		listItem->setData(QVariant(themeList.at(i)).toString(), Qt::UserRole);
		mContentItemModel->appendRow(listItem);
		tempPath = themeImgPath;
	}

	ui.lvThemes->setModel(mContentItemModel);
	ui.lvThemes->clearSelection();
	ui.lvThemes->setSelectionMode(QAbstractItemView::SingleSelection);
	ui.lvThemes->setWordWrap(true);
	ui.lvThemes->setViewMode(QListWidget::IconMode);
	ui.lvThemes->setIconSize(QSize(200,200));
	ui.lvThemes->setResizeMode(QListWidget::Adjust);
	ui.lvThemes->setMaximumWidth(225);
	ui.lvThemes->setMinimumHeight(300);
}
void ThemesWidget::applyClicked() {

	QModelIndex m = ui.lvThemes->currentIndex();
	QString themeName = m.data(Qt::UserRole).value<QString>();
	emit(themeSelected(themeName));
}
