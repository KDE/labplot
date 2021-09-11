/*
    File                 : ThemeHandler.cpp
    Project              : LabPlot
    Description          : Widget for handling saving and loading of themes
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2016 Prakriti Bhardwaj <p_bhardwaj14@informatik.uni-kl.de>
    SPDX-FileCopyrightText: 2016-2017 Alexander Semke <alexander.semke@web.de>
    SPDX-FileCopyrightText: 2018 Stefan Gerlach <stefan.gerlach@uni.kn>
    SPDX-License-Identifier: GPL-2.0-or-later
*/


#include "ThemeHandler.h"
#include "widgets/ThemesWidget.h"
#include "backend/lib/macros.h"

#include <QDir>
#include <QDirIterator>
#include <QFileInfo>
#include <QHBoxLayout>
#include <QMenu>
#include <QPushButton>
#include <QWidgetAction>

#include <KConfig>
#include <KConfigGroup>
#include <KLocalizedString>

// #include <KMessageBox>
// #include <KNS3/UploadDialog>

/*!
  \class ThemeHandler
  \brief Provides a widget with buttons for loading of themes.

  Emits \c loadConfig() signal that have to be connected
  to the appropriate slots in the backend (plot widgets)

  \ingroup kdefrontend
*/

ThemeHandler::ThemeHandler(QWidget* parent) : QWidget(parent) {
	auto* horizontalLayout = new QHBoxLayout(this);
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

	connect(m_pbLoadTheme, &QPushButton::clicked, this, &ThemeHandler::showPanel);
// 	connect( pbSaveTheme, SIGNAL(clicked()), this, SLOT(saveMenu()));
// 	connect( pbPublishTheme, SIGNAL(clicked()), this, SLOT(publishThemes()));

	m_themeList = themeList();

	m_pbLoadTheme->setEnabled(!m_themeList.isEmpty());
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*!
 * get list of all theme files (full path)
 */
QStringList ThemeHandler::themeList() {
	DEBUG("ThemeHandler::themeList()");
	// find all available themes files (system wide and user specific local files)
	QStringList dirs = QStandardPaths::locateAll(QStandardPaths::DataLocation, "themes", QStandardPaths::LocateDirectory);

	QStringList themes;
	for (const auto& dir : dirs) {
		QDirIterator it(dir, QStringList() << QStringLiteral("*"), QDir::Files);
		while (it.hasNext())
			themes.append(it.next());
	}

	if (!themes.isEmpty())
		DEBUG("	first theme path: " << STDSTRING(themes.first()));

	return themes;
}

/*!
 * get list of all theme names
 */
QStringList ThemeHandler::themes() {
	DEBUG("ThemeHandler::themes()");
	QStringList themePaths = themeList();

	QStringList themes;
	for (int i = 0; i < themePaths.size(); ++i) {
		QFileInfo fileinfo(themePaths.at(i));
		themes.append(fileinfo.fileName().split('.').at(0));
	}

	if (!themes.isEmpty()) {
		DEBUG("	first theme: " << STDSTRING(themes.first()));
		QDEBUG("	themes = " << themes);
	}

	return themes;
}

/*!
 * get path for theme of name 'name'
 */
const QString ThemeHandler::themeFilePath(const QString& name) {
	DEBUG("ThemeHandler::themeFilePath() name = " << STDSTRING(name));
	QStringList themePaths = themeList();

	for (int i = 0; i < themePaths.size(); ++i) {
		const QString& path = themePaths.at(i);
		const QString& fileName = QFileInfo(path).fileName();
		if (fileName == name) {
			DEBUG("	theme \"" << STDSTRING(name) << "\" path: " << STDSTRING(path));
			return path;
		}
	}

	return QString();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

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
// 		m_currentLocalTheme = themeFilePath.right(themeFilePath.length() - themeFilePath.lastIndexOf(QLatin1String("/")) - 1);
// 	} else {
// 		pbPublishTheme->setEnabled(false);
// 		m_currentLocalTheme.clear();
// 	}
}

void ThemeHandler::showPanel() {
#ifndef SDK
	QMenu menu;
	ThemesWidget themeWidget(&menu);
	themeWidget.setFixedMode();
	connect(&themeWidget, &ThemesWidget::themeSelected, this, &ThemeHandler::loadSelected);
	connect(&themeWidget, &ThemesWidget::themeSelected, &menu, &QMenu::close);
	connect(&themeWidget, &ThemesWidget::canceled, &menu, &QMenu::close);

	auto* widgetAction = new QWidgetAction(this);
	widgetAction->setDefaultWidget(&themeWidget);
	menu.addAction(widgetAction);

	QPoint pos(-menu.sizeHint().width()+m_pbLoadTheme->width(),-menu.sizeHint().height());
	menu.exec(m_pbLoadTheme->mapToGlobal(pos));
#endif
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
// 	QLineEdit* leFilename = new QLineEdit(QString(), frame);
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
