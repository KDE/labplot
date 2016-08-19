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

#include <KMessageBox>
#include <knewstuff3/uploaddialog.h>
#include <kapplication.h>
#include <kdebug.h>
#include <klocale.h>
#include <kcmdlineargs.h>
#include <kaboutdata.h>
#include <knewstuff3/downloaddialog.h>

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
	QString themeImgPath = KGlobal::dirs()->findDirs("data", "labplot2/themes/screenshots/").first();
	QString tempPath;

	for (int i = 0; i < themeList.size(); ++i) {
		QStandardItem* listItem = new QStandardItem();

		tempPath = themeImgPath + themeList.at(i) + ".png";
		if(!QFile::exists(tempPath))
			tempPath = themeImgPath + "Unavailable.png";

		listItem->setIcon(QIcon(QPixmap(tempPath)));
		listItem->setText(themeList.at(i));
		listItem->setData(themeList.at(i), Qt::UserRole);
		mContentItemModel->appendRow(listItem);
	}
	//adding upload themes option in list
	QStandardItem* listItem1 = new QStandardItem();
	tempPath = themeImgPath+"Unavailable.png";
	listItem1->setIcon(QIcon(QPixmap(tempPath)));
	listItem1->setText("Publish Themes");
	listItem1->setData("file_publish_theme", Qt::UserRole);
	mContentItemModel->appendRow(listItem1);

	//adding download themes option in list
	QStandardItem* listItem2 = new QStandardItem();
	listItem2->setIcon(QIcon(QPixmap(tempPath)));
	listItem2->setText("Download Themes");
	listItem2->setData("file_download_theme", Qt::UserRole);
	mContentItemModel->appendRow(listItem2);

	setModel(mContentItemModel);

	//SLOTS
	connect( this, SIGNAL(clicked(QModelIndex)), this, SLOT(applyClicked()) );
}


void ThemesWidget::applyClicked() {
	QString themeName = currentIndex().data(Qt::UserRole).value<QString>();
	if(themeName=="file_publish_theme")
		this->publishThemes();
	else if(themeName=="file_download_theme")
		this->downloadThemes();
	else
		emit(themeSelected(themeName));
}

void ThemesWidget::publishThemes() {
	this->parentWidget()->close();
	QStringList localThemeFiles = KGlobal::dirs()->findAllResources("appdata", "themes/local/*");

	int ret = KMessageBox::questionYesNo(this->parentWidget(),
					     i18n("Do you want to upload your themes to public web server?"),
					     i18n("Question - Labplot"));
	if (ret != KMessageBox::Yes) return;


	// upload

	if(!localThemeFiles.empty()) {
		foreach(QString fileName, localThemeFiles) {
			qDebug()<<"uploading file "<<fileName;
			KNS3::UploadDialog dialog("labplot2.knsrc", this);
			dialog.setUploadFile(KUrl(fileName));
			dialog.exec();
		}
	}
	else {
		ret = KMessageBox::warningContinueCancel(this,
							 i18n("There are no locally saved themes to be uploaded. Please create new themes."),
							 i18n("Warning - Labplot"),  KStandardGuiItem::ok());
		if (ret != KMessageBox::Continue) return;
	}


}

void ThemesWidget::downloadThemes() {
	this->parentWidget()->close();
	KNS3::DownloadDialog dialog;
	dialog.exec();
//	foreach (const KNS3::Entry& e,  dialog.changedEntries())
//	{
//	    qDebug() << "Changed Entry: " << e.name();
//	}

}
