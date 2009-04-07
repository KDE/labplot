/***************************************************************************
    File                 : WorksheetModule.cpp
    Project              : LabPlot/SciDAVis
    Description          : Module providing the worksheet Part and support classes.
    --------------------------------------------------------------------
    Copyright            : (C) 2009 Tilman Benkert (thzs*gmx.net)
    Copyright            : (C) 2008-2009 Knut Franke (knut.franke*gmx.de)
                           (replace * with @ in the email addresses) 
                           
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

#include "WorksheetModule.h"
#include "worksheet/Worksheet.h"
#include "worksheet/WorksheetView.h"
#include "core/Project.h"
#include "core/ProjectWindow.h"
#include "lib/ActionManager.h"
#include <QAction>
#include <QPixmap>
#include <QtDebug>
#include <QSettings>
// TODO #include "ui_WorksheetConfigPage.h"


/**
 * \class WorksheetModule
 * \brief Module providing the worksheet Part and support classes.
 *
 *
 */

/**
 * \class WorksheetConfigPage
 * \brief Helper class for TableModule.
 *
 *
 */

ActionManager * WorksheetModule::actionManager() { 
	return WorksheetView::actionManager(); 
}


WorksheetConfigPage::WorksheetConfigPage() {
// TODO	ui = new Ui_WorksheetConfigPage();
// TODO	ui->setupUi(this);
}

WorksheetConfigPage::~WorksheetConfigPage() {
// TODO	delete ui;
}

void WorksheetConfigPage::apply() {
	// TODO: read settings from ui and change them in Worksheet
}

AbstractPart * WorksheetModule::makePart() {
	return new Worksheet(NULL, tr("Worksheet %1").arg(1));
}

QAction * WorksheetModule::makeAction(QObject *parent) {
	QAction *new_worksheet = new QAction(tr("New &Worksheet"), parent);
// TODO:	new_worksheet->setShortcut(tr("...", "new worksheet shortcut"));
	new_worksheet->setIcon(QIcon(QPixmap(":/worksheet.xpm")));
	WorksheetView::actionManager()->addAction(new_worksheet, "new_worksheet");
	return new_worksheet;
}

void WorksheetModule::initActionManager() {
	WorksheetView::initActionManager();
}

ConfigPageWidget * WorksheetModule::makeConfigPage() {
	return new WorksheetConfigPage();
}
		
QString WorksheetModule::configPageLabel() {
	return QObject::tr("Worksheet");
}

void WorksheetModule::loadSettings() {
#ifdef Q_OS_MAC // Mac
	QSettings settings(QSettings::IniFormat,QSettings::UserScope, "SciDAVis", "SciDAVis");
#else
	QSettings settings(QSettings::NativeFormat,QSettings::UserScope, "SciDAVis", "SciDAVis");
#endif

	settings.beginGroup("Worksheet");
	settings.endGroup();
}

void WorksheetModule::saveSettings() {
#ifdef Q_OS_MAC // Mac
	QSettings settings(QSettings::IniFormat,QSettings::UserScope, "SciDAVis", "SciDAVis");
#else
	QSettings settings(QSettings::NativeFormat,QSettings::UserScope, "SciDAVis", "SciDAVis");
#endif

	settings.beginGroup("Worksheet");
	settings.endGroup();
}

bool WorksheetModule::canCreate(const QString & element_name) {	
	return element_name == "worksheet";
}

AbstractAspect * WorksheetModule::createAspectFromXml(XmlStreamReader * reader) {
	Worksheet * worksheet = new Worksheet(NULL, tr("Worksheet %1").arg(1));
	if (!(worksheet->load(reader)))
	{
		delete worksheet;
		return NULL;
	}
	else
		return worksheet;
}

void WorksheetModule::staticInit() {
	// TODO: Worksheet::setGlobalDefault(...);
}

Q_EXPORT_PLUGIN2(scidavis_worksheet, WorksheetModule)

