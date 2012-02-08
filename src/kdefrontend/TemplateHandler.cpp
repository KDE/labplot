/***************************************************************************
    File                 : TemplateHandler.cpp
    Project              : LabPlot/SciDAVis
    Description          : Widget for handling saving and loading of templates
    --------------------------------------------------------------------
	Copyright            : (C) 2012 by Stefan Gerlach (stefan.gerlach*uni-konstanz.de)
															Alexander Semke (alexander.semke*web.de)
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

#include "TemplateHandler.h"

#include <QMenu>
#include <QFileInfo>
#include <QWidgetAction>
#include <KLocale>
#include <KStandardDirs>
#include <KLineEdit>
#include <KIcon>

 /*!
  \class TemplateHandler
  \brief Provides a widget with buttons for saving and loading of templates.
  
  Emits \c loadConfig() and \c saveConfig() signals that have to be connected to the appropriate slots in the ui (mostly in the dock widgets)

  \ingroup kdefrontend
*/
 
 
TemplateHandler::TemplateHandler(QWidget *parent, ClassName name): QWidget(parent){
	horizontalLayout = new QHBoxLayout(this);
	horizontalLayout->setSpacing(0);
	horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
	horizontalLayout->setSizeConstraint(QLayout::SetDefaultConstraint);

	horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
	horizontalLayout->addItem(horizontalSpacer);

	tbLoad = new QToolButton(this);
	horizontalLayout->addWidget(tbLoad);

	tbSave = new QToolButton(this);
	horizontalLayout->addWidget(tbSave);

	tbSaveDefault = new QToolButton(this);
	horizontalLayout->addWidget(tbSaveDefault);

	horizontalSpacer2 = new QSpacerItem(10, 20, QSizePolicy::Fixed, QSizePolicy::Minimum);
	horizontalLayout->addItem(horizontalSpacer2);

	tbCopy = new QToolButton(this);
	tbCopy->setEnabled(false);
	horizontalLayout->addWidget(tbCopy);

	tbPaste = new QToolButton(this);
	tbPaste->setEnabled(false);
	horizontalLayout->addWidget(tbPaste);

	tbLoad->setIcon(KIcon("document-open"));
	tbSave->setIcon(KIcon("document-save"));
	tbSaveDefault->setIcon(KIcon("document-save-as"));
	tbCopy->setIcon(KIcon("edit-copy"));
	tbPaste->setIcon(KIcon("edit-paste"));

	connect( tbLoad, SIGNAL(clicked()), this, SLOT(loadMenu()));
	connect( tbSave, SIGNAL(clicked()), this, SLOT(saveMenu()));
	connect( tbSaveDefault, SIGNAL(clicked()), this, SLOT(saveDefaults()));

	className = name;
	
	//synchronize this with the ordering in TemplateHandler::ClassName
	dirNames<<"spreadsheet"<<"worksheet";
	
	this->retranslateUi();
}

TemplateHandler::~TemplateHandler(){};

void TemplateHandler::loadMenu(){
	QMenu menu;
	QStringList list = KGlobal::dirs()->findAllResources("appdata", "templates/" + dirNames.at(className) + "/*");
	for (int i = 0; i < list.size(); ++i) {
			QFileInfo fileinfo(list.at(i));
			QAction* action = menu.addAction(fileinfo.fileName());
			action->setData(QVariant(list.at(i)));
	}
	connect(&menu, SIGNAL(triggered(QAction*)), this, SLOT(loadMenuSelected(QAction*)));

	QPoint pos(-menu.sizeHint().width()+tbLoad->width(),-menu.sizeHint().height());
	menu.exec(tbLoad->mapToGlobal(pos));
}

void TemplateHandler::loadMenuSelected(QAction* action){
	KConfig config(action->data().toString(), KConfig::SimpleConfig);
	emit (loadConfigRequested(config));
}

void TemplateHandler::saveMenu(){
	QMenu menu;
	QStringList list = KGlobal::dirs()->findAllResources("appdata", "templates/"+ dirNames.at(className) + "/*");
	for (int i = 0; i < list.size(); ++i) {
			QFileInfo fileinfo(list.at(i));
			QAction* action = menu.addAction(fileinfo.fileName());
			action->setData(QVariant(fileinfo.path()));
	}
	connect(&menu, SIGNAL(triggered(QAction*)), this, SLOT(saveMenuSelected(QAction*)));

	// add editable action
	QWidgetAction *widgetAction = new QWidgetAction(this);
	KLineEdit *leFilename = new KLineEdit("");
	connect(leFilename, SIGNAL(returnPressed(QString)), this, SLOT(saveNewSelected(QString)));
	connect(leFilename, SIGNAL(returnPressed(QString)), &menu, SLOT(close()));
	widgetAction->setDefaultWidget(leFilename);
	menu.addAction(widgetAction);

	QPoint pos(-menu.sizeHint().width()+tbSave->width(),-menu.sizeHint().height());
	menu.exec(tbSave->mapToGlobal(pos));
}

/*!
	the receiver of the signal had to config.sync().
 */
void TemplateHandler::saveNewSelected(QString filename){
	KConfig config(KGlobal::dirs()->locateLocal("appdata", "templates")+"/" + dirNames.at(className) + "/"+filename, KConfig::SimpleConfig);
	emit (saveConfigRequested(config));
}

/*!
	the receiver of the signal had to config.sync().
 */
void TemplateHandler::saveMenuSelected(QAction* action){
	KConfig config(action->data().toString()+'/'+action->text(), KConfig::SimpleConfig);
	emit (saveConfigRequested(config));
}

/*!
	the receiver of the signal had to config.sync().
 */
void TemplateHandler::saveDefaults(){
	KConfig config;
	emit (saveConfigRequested(config));
}


void TemplateHandler::retranslateUi(){
	tbLoad->setToolTip(i18n("Load properties from a template"));
	tbSave->setToolTip(i18n("Save properties as a template"));
	tbSaveDefault->setToolTip(i18n("Save properties as default"));
	tbCopy->setToolTip(i18n("Copy properties"));
	tbPaste->setToolTip(i18n("Paste properties"));
}
