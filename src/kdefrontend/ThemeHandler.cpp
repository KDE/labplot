/***************************************************************************
    File                 : ThemeHandler.cpp
    Project              : LabPlot
    Description          : Widget for handling saving and loading of themes
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

#include "ThemeHandler.h"
#include "widgets/ThemesWidget.h"

#include <QDir>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QFileInfo>
#include <QWidgetAction>

#include <KLocale>
#include <KStandardDirs>
#include <KLineEdit>
#include <KMenu>
#include <KConfig>
#include <KConfigGroup>

#include <KMessageBox>
// #include <knewstuff3/uploaddialog.h>

#include <unistd.h>

/*!
  \class ThemeHandler
  \brief Provides a widget with buttons for loading of themes.

  Emits \c loadConfig() signal that have to be connected
  to the appropriate slots in the backend (plot widgets)

  \ingroup kdefrontend
*/

ThemeHandler::ThemeHandler(QWidget* parent) : QWidget(parent) {
	QHBoxLayout* horizontalLayout = new QHBoxLayout(this);
	horizontalLayout->setSpacing(0);

	pbLoadTheme = new QPushButton(this);
	horizontalLayout->addWidget(pbLoadTheme);
	pbLoadTheme->setText(i18n("Apply theme"));

	pbSaveTheme = new QPushButton(this);
	horizontalLayout->addWidget(pbSaveTheme);
	pbSaveTheme->setText(i18n("Save theme"));

/*
	pbPublishTheme = new QPushButton(this);
	horizontalLayout->addWidget(pbPublishTheme);
	pbPublishTheme->setText("Publish theme");
	pbPublishTheme->setEnabled(false);
*/
	QSpacerItem* horizontalSpacer2 = new QSpacerItem(10, 20, QSizePolicy::Fixed, QSizePolicy::Minimum);
	horizontalLayout->addItem(horizontalSpacer2);

	connect( pbLoadTheme, SIGNAL(clicked()), this, SLOT(showPanel()));
	connect( pbSaveTheme, SIGNAL(clicked()), this, SLOT(saveMenu()));
// 	connect( pbPublishTheme, SIGNAL(clicked()), this, SLOT(publishThemes()));

	//find all available themes files (system wide and user specific local files)
	//the list m_themeList contains full pathes (path + file name)
	m_themeList = KGlobal::dirs()->findAllResources("appdata", "themes/*");
	pbLoadTheme->setEnabled(!m_themeList.isEmpty());
}

void ThemeHandler::loadSelected(QString name) {
	QString themeFilePath;
	foreach (QString filePath, m_themeList) {
		if ( filePath.indexOf(name)!=-1 ) {
			themeFilePath = filePath;
			break;
		}
	}
	KConfig config(themeFilePath, KConfig::SimpleConfig);
	emit (loadThemeRequested(config));

	emit info( i18n("Theme \"%1\" was loaded.", name) );

	//in case a local theme file was loaded (we have write access), allow to publish it
	//TODO: activate this later
// 	if (KGlobal::dirs()->checkAccess(themeFilePath, W_OK)) {
// 		pbPublishTheme->setEnabled(true);
// 		m_currentLocalTheme = themeFilePath.right(themeFilePath.length() - themeFilePath.lastIndexOf(QDir::separator()) - 1);
// 	} else {
// 		pbPublishTheme->setEnabled(false);
// 		m_currentLocalTheme.clear();
// 	}
}

QStringList ThemeHandler::themes() {
	QStringList pathList = KGlobal::dirs()->findAllResources("data", "labplot2/themes/*");
	pathList.append(KGlobal::dirs()->findAllResources("appdata", "themes/*"));
	QStringList themeList;
	for (int i = 0; i < pathList.size(); ++i) {
		QFileInfo fileinfo(pathList.at(i));
		themeList.append(fileinfo.fileName().split('.').at(0));
	}
	return themeList;
}

const QString ThemeHandler::themeFilePath(const QString& name) {
	QStringList themes = KGlobal::dirs()->findAllResources("data", "labplot2/themes/*");
	themes.append(KGlobal::dirs()->findAllResources("appdata", "themes/*"));
	for (int i = 0; i < themes.size(); ++i) {
		if ( themes.at(i).indexOf(name) != -1 )
			return themes.at(i);
	}

	return QString();
}

void ThemeHandler::showPanel() {
	QMenu menu;
	ThemesWidget themeWidget(&menu);
	connect(&themeWidget, SIGNAL(themeSelected(QString)), this, SLOT(loadSelected(QString)));
	connect(&themeWidget, SIGNAL(themeSelected(QString)), &menu, SLOT(close()));
	connect(&themeWidget, SIGNAL(canceled()), &menu, SLOT(close()));

	QWidgetAction* widgetAction = new QWidgetAction(this);
	widgetAction->setDefaultWidget(&themeWidget);
	menu.addAction(widgetAction);

	QPoint pos(-menu.sizeHint().width()+pbLoadTheme->width(),-menu.sizeHint().height());
	menu.setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Preferred);
	menu.exec(pbLoadTheme->mapToGlobal(pos));
}

void ThemeHandler::saveMenu() {
	KMenu menu;
	menu.addTitle(i18n("Save as"));

	// add editable action
	QWidgetAction* widgetAction = new QWidgetAction(this);
	QFrame* frame = new QFrame(this);
	QHBoxLayout* layout = new QHBoxLayout(frame);

	QLabel* label = new QLabel(i18n("Enter name:"), frame);
	layout->addWidget(label);

	KLineEdit* leFilename = new KLineEdit("", frame);
	layout->addWidget(leFilename);
	connect(leFilename, SIGNAL(returnPressed(QString)), this, SLOT(saveNewSelected(QString)));
	connect(leFilename, SIGNAL(returnPressed(QString)), &menu, SLOT(close()));

	widgetAction->setDefaultWidget(frame);
	menu.addAction(widgetAction);

	QPoint pos(-menu.sizeHint().width()+pbSaveTheme->width(),-menu.sizeHint().height());
	menu.exec(pbSaveTheme->mapToGlobal(pos));
	leFilename->setFocus();
}

void ThemeHandler::saveNewSelected(const QString& filename) {
	KConfig config(KGlobal::dirs()->locateLocal("appdata", "themes") + '/' + filename, KConfig::SimpleConfig);
	emit (saveThemeRequested(config));
	emit info( i18n("New theme \"%1\" was saved.", filename) );

	m_currentLocalTheme = filename;
	m_themeList.append(config.name());

	//enable the publish button so the newly created theme can be published
	//TODO: enable this later
// 	pbPublishTheme->setEnabled(true);
}

/*!
	opens the dialog to upload the currently selected local theme.
	The publish button is only enabled if a local theme was loaded or one of the themes was modified and saved localy.
 */
// void ThemeHandler::publishThemes() {
// 	int ret = KMessageBox::questionYesNo(this,
// 					     i18n("Do you want to upload your theme %1 to public web server?").arg(m_currentLocalTheme),
// 					     i18n("Publish Theme"));
// 	if (ret != KMessageBox::Yes)
// 		return;
// 
// 	// creating upload dialog
// 	KNS3::UploadDialog dialog("labplot2_themes.knsrc", this);
// 	dialog.setUploadFile(KGlobal::dirs()->locateLocal("appdata", "themes") + '/' + m_currentLocalTheme);
// 	dialog.setUploadName(m_currentLocalTheme);
// 	//dialog.setDescription(); TODO: allow the user to provide a short description for the theme to be uploaded
// 	dialog.exec();
// }
