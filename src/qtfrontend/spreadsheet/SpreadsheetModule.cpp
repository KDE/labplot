/***************************************************************************
    File                 : SpreadsheetModule.cpp
    Project              : SciDAVis
    Description          : Module providing the spreadsheet Part and support classes.
    --------------------------------------------------------------------
    Copyright            : (C) 2008 Knut Franke (knut.franke*gmx.de)
                           (replace * with @ in the email address)

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
#include "SpreadsheetModule.h"

#include "spreadsheet/Spreadsheet.h"
#include "spreadsheet/SpreadsheetView.h"
#include "spreadsheet/AsciiSpreadsheetImportFilter.h"
#include "core/Project.h"
#include "core/ProjectWindow.h"
#include "lib/ActionManager.h"
#include <QAction>
#include <QPixmap>
#include <QtDebug>
#include <QSettings>
#include "ui_SpreadsheetConfigPage.h"

SpreadsheetConfigPage::SpreadsheetConfigPage() 
{
	ui = new Ui_SpreadsheetConfigPage();
	ui->setupUi(this);
}

SpreadsheetConfigPage::~SpreadsheetConfigPage() 
{
	delete ui;
}

void SpreadsheetConfigPage::apply()
{
	// TODO: read settings from ui and change them in Spreadsheet
}

AbstractPart * SpreadsheetModule::makePart()
{
	return new Spreadsheet(0, 30, 2, tr("Spreadsheet %1").arg(1));
}

QAction * SpreadsheetModule::makeAction(QObject *parent)
{
	QAction *new_spreadsheet = new QAction(tr("New &Spreadsheet"), parent);
	new_spreadsheet->setShortcut(tr("Ctrl+T", "new spreadsheet shortcut"));
	new_spreadsheet->setIcon(QIcon(QPixmap(":/table.xpm")));
	SpreadsheetView::actionManager()->addAction(new_spreadsheet, "new_spreadsheet");
	return new_spreadsheet;
}

AbstractImportFilter * SpreadsheetModule::makeImportFilter()
{
	return new AsciiSpreadsheetImportFilter();
}

AbstractExportFilter * SpreadsheetModule::makeExportFilter()
{
	// TODO
	return 0;
}

void SpreadsheetModule::initActionManager()
{
	SpreadsheetView::initActionManager();
}

ConfigPageWidget * SpreadsheetModule::makeConfigPage()
{
	return new SpreadsheetConfigPage();
}
		
QString SpreadsheetModule::configPageLabel()
{
	return QObject::tr("Spreadsheet");
}

void SpreadsheetModule::loadSettings()
{
#ifdef Q_OS_MAC // Mac
	QSettings settings(QSettings::IniFormat,QSettings::UserScope, "SciDAVis", "SciDAVis");
#else
	QSettings settings(QSettings::NativeFormat,QSettings::UserScope, "SciDAVis", "SciDAVis");
#endif

	settings.beginGroup("Spreadsheet");
	settings.endGroup();
}

void SpreadsheetModule::saveSettings()
{
#ifdef Q_OS_MAC // Mac
	QSettings settings(QSettings::IniFormat,QSettings::UserScope, "SciDAVis", "SciDAVis");
#else
	QSettings settings(QSettings::NativeFormat,QSettings::UserScope, "SciDAVis", "SciDAVis");
#endif

	settings.beginGroup("Spreadsheet");
	settings.endGroup();
}

bool SpreadsheetModule::canCreate(const QString & element_name)
{	
	return element_name == "spreadsheet";
}

AbstractAspect * SpreadsheetModule::createAspectFromXml(XmlStreamReader * reader)
{
	Spreadsheet * spreadsheet = new Spreadsheet(0, 0, 0, tr("Spreadsheet %1").arg(1));
	if (!(spreadsheet->load(reader)))
	{
		delete spreadsheet;
		return NULL;
	}
	else
		return spreadsheet;
}

void SpreadsheetModule::staticInit()
{
	Spreadsheet::setGlobalDefault("default_comment_visibility", false);
}

Q_EXPORT_PLUGIN2(scidavis_spreadsheet, SpreadsheetModule)


