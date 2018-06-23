/***************************************************************************
    File                 : ThemeHandler.cpp
    Project              : LabPlot
    Description          : Widget for handling saving and loading of themes
    --------------------------------------------------------------------
    Copyright            : (C) 2016 Prakriti Bhardwaj (p_bhardwaj14@informatik.uni-kl.de)
    Copyright            : (C) 2016-2017 Alexander Semke (alexander.semke@web.de)

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
#include <QDirIterator>
#include <QFileInfo>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMenu>
#include <QPushButton>
#include <QWidgetAction>

#include <KConfig>
#include <KConfigGroup>
#include <KLocale>

#include <KMessageBox>
// #include <KNS3/UploadDialog>

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
	horizontalLayout->setMargin(0);

    m_pbLoadTheme = new QPushButton(this);
    horizontalLayout->addWidget(m_pbLoadTheme);
    m_pbLoadTheme->setText(i18n("Apply Theme"));

// 	pbSaveTheme = new QPushButton(this);
// 	horizontalLayout->addWidget(pbSaveTheme);
// 	pbSaveTheme->setText(i18n("Save Theme"));

/*
	pbPublishTheme = new QPushButton(this);
	horizontalLayout->addWidget(pbPublishTheme);
	pbPublishTheme->setText("Publish Theme");
	pbPublishTheme->setEnabled(false);
*/

    connect( m_pbLoadTheme, SIGNAL(clicked()), this, SLOT(showPanel()));
// 	connect( pbSaveTheme, SIGNAL(clicked()), this, SLOT(saveMenu()));
// 	connect( pbPublishTheme, SIGNAL(clicked()), this, SLOT(publishThemes()));

	//find all available themes files (system wide and user specific local files)
	//the list m_themeList contains full paths (path + file name)
	QStringList dirs = QStandardPaths::locateAll(QStandardPaths::DataLocation, "themes", QStandardPaths::LocateDirectory);
	for (const auto& dir : dirs) {
		QDirIterator it(dir, QStringList() << QStringLiteral("*"), QDir::Files);
		while (it.hasNext())
			m_themeList.append(it.next());
	}

    m_pbLoadTheme->setEnabled(!m_themeList.isEmpty());
}

void ThemeHandler::setCurrentTheme(const QString& name) {
	if (!name.isEmpty()) {
        m_pbLoadTheme->setText(i18n("Apply theme [active '%1']", name));
        m_pbLoadTheme->setToolTip(i18n("Theme '%1' is active. Click on the button to change the theme.", name));
	} else {
        m_pbLoadTheme->setText(i18n("Apply Theme"));
        m_pbLoadTheme->setToolTip(i18n("No theme is active. Click on the button to select a theme."));
	}

	m_currentTheme = name;
}

void ThemeHandler::loadSelected(const QString& name) {
	emit loadThemeRequested(name);
	this->setCurrentTheme(name);

	if (!name.isEmpty())
		emit info( i18n("Theme \"%1\" was loaded.", name) );
	else
		emit info( i18n("Theming deactivated.") );

	//in case a local theme file was loaded (we have write access), allow to publish it
	//TODO: activate this later
// 	if (KStandardDirs::checkAccess(themeFilePath, W_OK)) {
// 		pbPublishTheme->setEnabled(true);
// 		m_currentLocalTheme = themeFilePath.right(themeFilePath.length() - themeFilePath.lastIndexOf(QDir::separator()) - 1);
// 	} else {
// 		pbPublishTheme->setEnabled(false);
// 		m_currentLocalTheme.clear();
// 	}
}

QStringList ThemeHandler::themes() {
	QStringList dirs = QStandardPaths::locateAll(QStandardPaths::DataLocation, "themes", QStandardPaths::LocateDirectory);
	QStringList pathList;
	for (const auto& dir : dirs) {
		QDirIterator it(dir, QStringList() << QStringLiteral("*"), QDir::Files);
		while (it.hasNext())
			pathList.append(it.next());
	}

	QStringList themeList;
	for (int i = 0; i < pathList.size(); ++i) {
		QFileInfo fileinfo(pathList.at(i));
		themeList.append(fileinfo.fileName().split('.').at(0));
	}
	return themeList;
}

const QString ThemeHandler::themeFilePath(const QString& name) {
	QStringList dirs = QStandardPaths::locateAll(QStandardPaths::DataLocation, "themes", QStandardPaths::LocateDirectory);

	QStringList themes;
	for (const auto& dir : dirs) {
		QDirIterator it(dir, QStringList() << QStringLiteral("*"), QDir::Files);
		while (it.hasNext())
			themes.append(it.next());
	}

	for (int i = 0; i < themes.size(); ++i) {
		if (themes.at(i).indexOf(name) != -1)
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

    QPoint pos(-menu.sizeHint().width()+m_pbLoadTheme->width(),-menu.sizeHint().height());
	menu.setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Preferred);
    menu.exec(m_pbLoadTheme->mapToGlobal(pos));
}

// void ThemeHandler::saveMenu() {
// 	QMenu menu;
// 	menu.addSection(i18n("Save As"));
//
// 	// add editable action
// 	QWidgetAction* widgetAction = new QWidgetAction(this);
// 	QFrame* frame = new QFrame(this);
// 	QHBoxLayout* layout = new QHBoxLayout(frame);
//
// 	QLabel* label = new QLabel(i18n("Enter name:"), frame);
// 	layout->addWidget(label);
//
// 	QLineEdit* leFilename = new QLineEdit("", frame);
// 	layout->addWidget(leFilename);
// 	connect(leFilename, SIGNAL(returnPressed(QString)), this, SLOT(saveNewSelected(QString)));
// 	connect(leFilename, SIGNAL(returnPressed(QString)), &menu, SLOT(close()));
//
// 	widgetAction->setDefaultWidget(frame);
// 	menu.addAction(widgetAction);
//
//     QPoint pos(-menu.sizeHint().width() + m_pbSaveTheme->width(), -menu.sizeHint().height());
//     menu.exec(m_pbSaveTheme->mapToGlobal(pos));
// 	leFilename->setFocus();
// }

// void ThemeHandler::saveNewSelected(const QString& filename) {
// 	KConfig config(QStandardPaths::writableLocation(QStandardPaths::DataLocation) + '/' + "themes" + '/' + filename, KConfig::SimpleConfig);
// 	emit saveThemeRequested(config);
// 	emit info( i18n("New theme \"%1\" was saved.", filename) );
//
// 	m_currentLocalTheme = filename;
// 	m_themeList.append(config.name());
//
// 	//enable the publish button so the newly created theme can be published
// 	//TODO: enable this later
// // 	pbPublishTheme->setEnabled(true);
// }

/*!
	opens the dialog to upload the currently selected local theme.
	The publish button is only enabled if a local theme was loaded or one of the themes was modified and saved locally.
 */
// void ThemeHandler::publishThemes() {
// 	int ret = KMessageBox::questionYesNo(this,
// 			i18n("Do you want to upload your theme %1 to public web server?", m_currentLocalTheme),
// 			i18n("Publish Theme"));
// 	if (ret != KMessageBox::Yes)
// 		return;
//
// 	// creating upload dialog
// 	KNS3::UploadDialog dialog("labplot2_themes.knsrc", this);
// 	dialog.setUploadFile(QStandardPaths::writableLocation(QStandardPaths::DataLocation) + '/' + "themes" + '/' + m_currentLocalTheme);
// 	dialog.setUploadName(m_currentLocalTheme);
// 	//dialog.setDescription(); TODO: allow the user to provide a short description for the theme to be uploaded
// 	dialog.exec();
// }
