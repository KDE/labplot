/***************************************************************************
    File                 : TableModule.h
    Project              : SciDAVis
    Description          : Module providing the table Part and support classes.
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
#ifndef TABLE_MODULE_H
#define TABLE_MODULE_H

#include "core/interfaces.h"
#include "table/Table.h"
#include "table/TableView.h"
#include <QMenu>

class TableModule : public QObject, public PartMaker, public FileFormat, public ActionManagerOwner, public ConfigPageMaker, 
	public XmlElementAspectMaker, public NeedsStaticInit
{
	Q_OBJECT
	Q_INTERFACES(PartMaker FileFormat ActionManagerOwner ConfigPageMaker XmlElementAspectMaker NeedsStaticInit)

	public:
		virtual AbstractPart * makePart();
		virtual QAction * makeAction(QObject *parent);
		virtual AbstractImportFilter * makeImportFilter();
		virtual AbstractExportFilter * makeExportFilter();
		virtual ActionManager * actionManager() { return TableView::actionManager(); }
		virtual void initActionManager();
		virtual ConfigPageWidget * makeConfigPage();
		virtual QString configPageLabel();
		virtual void loadSettings();
		virtual void saveSettings();
		virtual bool canCreate(const QString & element_name);
		virtual AbstractAspect * createAspectFromXml(XmlStreamReader * reader);
		virtual void staticInit();
};

class Ui_TableConfigPage;

//! Helper class for TableModule
class TableConfigPage : public ConfigPageWidget
{
	Q_OBJECT

	public:
		TableConfigPage();
		~TableConfigPage();

	public slots:
		virtual void apply();

	private:
		Ui_TableConfigPage *ui;
};

#endif // ifndef TABLE_MODULE_H

