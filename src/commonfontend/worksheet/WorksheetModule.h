/***************************************************************************
    File                 : WorksheetModule.h
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

#ifndef WORKSHEETMODULE.H
#define WORKSHEETMODULE.H

#include "core/interfaces.h"

class WorksheetModule: public QObject, public PartMaker, public ActionManagerOwner, public ConfigPageMaker, 
	public XmlElementAspectMaker, public NeedsStaticInit {
	Q_OBJECT
	Q_INTERFACES(PartMaker FileFormat ActionManagerOwner ConfigPageMaker XmlElementAspectMaker NeedsStaticInit)

	public:
		virtual AbstractPart * makePart();
		virtual QAction * makeAction(QObject *parent);
		virtual ActionManager * actionManager();
		virtual void initActionManager();
		virtual ConfigPageWidget * makeConfigPage();
		virtual QString configPageLabel();
		virtual void loadSettings();
		virtual void saveSettings();
		virtual bool canCreate(const QString & element_name);
		virtual AbstractAspect * createAspectFromXml(XmlStreamReader * reader);
		virtual void staticInit();
};

class Ui_WorksheetConfigPage;

class WorksheetConfigPage : public ConfigPageWidget {
	Q_OBJECT

	public:
		WorksheetConfigPage();
		~WorksheetConfigPage();

	public slots:
		virtual void apply();

	private:
		Ui_WorksheetConfigPage *ui;
};

#endif


