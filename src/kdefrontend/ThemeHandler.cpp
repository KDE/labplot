/***************************************************************************
    File                 : ThemeHandler.cpp
    Project              : LabPlot
    Description          : Widget for handling saving and loading of themes
    --------------------------------------------------------------------
	Copyright            : (C) 2012 by Stefan Gerlach (stefan.gerlach@uni-konstanz.de)
	Copyright            : (C) 2012-2014 by Alexander Semke (alexander.semke@web.de)

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
#include <QtGui/QHBoxLayout>
#include <QtGui/QSpacerItem>
#include <QtGui/QToolButton>
#include <QLabel>
#include <QFileInfo>
#include <QWidgetAction>
#include <KLocale>
#include <KStandardDirs>
#include <KLineEdit>
#include <KIcon>
#include <KMenu>
#include <KConfig>
#include <KConfigGroup>

/*!
  \class ThemeHandler
  \brief Provides a widget with buttons for loading of themes.

  Emits \c loadConfig() signal that have to be connected
  to the appropriate slots in the backend (plot widgets)

  \ingroup kdefrontend
*/

ThemeHandler::ThemeHandler(QWidget *parent): QWidget(parent){
	horizontalLayout = new QHBoxLayout(this);
	horizontalLayout->setSpacing(0);

	lTheme = new QLabel(this);
	horizontalLayout->addWidget(lTheme);
	lTheme->setText("Theme Manager:");

	horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
	horizontalLayout->addItem(horizontalSpacer);

	pbLoadTheme = new QPushButton(this);
	horizontalLayout->addWidget(pbLoadTheme);
	pbLoadTheme->setText("Choose theme");

	horizontalSpacer2 = new QSpacerItem(10, 20, QSizePolicy::Fixed, QSizePolicy::Minimum);
	horizontalLayout->addItem(horizontalSpacer2);

	connect( pbLoadTheme, SIGNAL(clicked()), this, SLOT(loadMenu()));
	m_themeList = KGlobal::dirs()->findAllResources("data", "labplot2/themes/*");
	pbLoadTheme->setEnabled(!m_themeList.empty());
}


void ThemeHandler::loadMenu() {
	KMenu menu;
	menu.addTitle(i18n("Themes:"));

	for (int i = 0; i < m_themeList.size(); ++i) {
		QFileInfo fileinfo(m_themeList.at(i));
		QAction* action = menu.addAction(fileinfo.fileName());
		action->setData(QVariant(m_themeList.at(i)));
	}
	connect(&menu, SIGNAL(triggered(QAction*)), this, SLOT(loadSelected(QAction*)));

	QPoint pos(-menu.sizeHint().width()+pbLoadTheme->width(),-menu.sizeHint().height());
	menu.exec(pbLoadTheme->mapToGlobal(pos));
}

void ThemeHandler::loadSelected(QAction* action) {
	KConfig config(action->data().toString(), KConfig::SimpleConfig);
	emit (loadThemeRequested(config));

	emit info( i18n("Theme \"%1\" was loaded.", action->text().remove('&')) );
}

QStringList ThemeHandler::themes() {
	const QStringList pathList = KGlobal::dirs()->findAllResources("data", "labplot2/themes/*");
	QStringList themeList;
	for(int i = 0; i < pathList.size(); ++i) {
		QFileInfo fileinfo(pathList.at(i));
		themeList.append(fileinfo.fileName().split('.').at(0));
	}
	return themeList;
}

const QString ThemeHandler::themeConfigPath(const QString& name) {
	const QStringList themes = KGlobal::dirs()->findAllResources("data", "labplot2/themes/*");
	for (int i=0; i<themes.size(); ++i) {
		if (themes.at(i).indexOf(name + ".txt")!=-1)
			return themes.at(i);
	}

	return QString();
}