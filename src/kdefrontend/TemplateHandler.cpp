/***************************************************************************
    File                 : TemplateHandler.cpp
    Project              : LabPlot
    Description          : Widget for handling saving and loading of templates
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

#include "TemplateHandler.h"
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QToolButton>
#include <QLabel>
#include <QFileInfo>
#include <QWidgetAction>

#include <KLocalizedString>
#include <KLineEdit>
#include <QMenu>
#include <KConfig>
 /*!
  \class TemplateHandler
  \brief Provides a widget with buttons for saving and loading of templates.

  Emits \c loadConfig() and \c saveConfig() signals that have to be connected
  to the appropriate slots in the ui (mostly in the dock widgets)

  \ingroup kdefrontend
*/

TemplateHandler::TemplateHandler(QWidget *parent, ClassName name): QWidget(parent){
	horizontalLayout = new QHBoxLayout(this);
	horizontalLayout->setSpacing(0);

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

    tbLoad->setIcon(QIcon::fromTheme("document-open"));
    tbSave->setIcon(QIcon::fromTheme("document-save"));
    tbSaveDefault->setIcon(QIcon::fromTheme("document-save-as"));
    tbCopy->setIcon(QIcon::fromTheme("edit-copy"));
    tbPaste->setIcon(QIcon::fromTheme("edit-paste"));

	connect( tbLoad, SIGNAL(clicked()), this, SLOT(loadMenu()));
	connect( tbSave, SIGNAL(clicked()), this, SLOT(saveMenu()));
	connect( tbSaveDefault, SIGNAL(clicked()), this, SLOT(saveDefaults()));

	className = name;

	//synchronize this with the ordering in TemplateHandler::ClassName
	dirNames<<"spreadsheet"<<"matrix"<<"worksheet"<<"cartesianplot"<<"cartesianplotlegend"<<"xycurve"<<"histogram"<<"axis"<<"custompoint";

	this->retranslateUi();

	//disable the load-button if no templates are available yet
    QStringList list = QStandardPaths::locateAll(QStandardPaths::ApplicationsLocation, "templates/" + dirNames.at(className) + "/*");
	tbLoad->setEnabled(list.size());
}

void TemplateHandler::retranslateUi(){
	tbLoad->setToolTip(i18n("Load properties from a template"));
	tbSave->setToolTip(i18n("Save current properties as a template"));
	tbSaveDefault->setToolTip(i18n("Save current properties as default"));
	tbCopy->setToolTip(i18n("Copy properties"));
	tbPaste->setToolTip(i18n("Paste properties"));
}

//##############################################################################
//##################################  Slots ####################################
//##############################################################################
void TemplateHandler::loadMenu() {
    QMenu menu;
    menu.addSection(i18n("Load from"));

    QStringList list = QStandardPaths::locateAll(QStandardPaths::ApplicationsLocation, "templates/" + dirNames.at(className) + "/*");
	for (int i = 0; i < list.size(); ++i) {
			QFileInfo fileinfo(list.at(i));
			QAction* action = menu.addAction(fileinfo.fileName());
			action->setData(QVariant(list.at(i)));
	}
	connect(&menu, SIGNAL(triggered(QAction*)), this, SLOT(loadMenuSelected(QAction*)));

	QPoint pos(-menu.sizeHint().width()+tbLoad->width(),-menu.sizeHint().height());
	menu.exec(tbLoad->mapToGlobal(pos));
}

void TemplateHandler::loadMenuSelected(QAction* action) {
	KConfig config(action->data().toString(), KConfig::SimpleConfig);
	emit (loadConfigRequested(config));

	emit info( i18n("Template \"%1\" was loaded.", action->text().remove('&')) );
}

void TemplateHandler::saveMenu() {
    QMenu menu;
    menu.addSection(i18n("Save as"));

    QStringList list = QStandardPaths::locateAll(QStandardPaths::ApplicationsLocation, "templates/"+ dirNames.at(className) + "/*");
	for (int i = 0; i < list.size(); ++i) {
			QFileInfo fileinfo(list.at(i));
			QAction* action = menu.addAction(fileinfo.fileName());
			menu.addAction(action);
			action->setShortcut(QKeySequence());
	}
	connect(&menu, SIGNAL(triggered(QAction*)), this, SLOT(saveMenuSelected(QAction*)));

	// add editable action
	QWidgetAction* widgetAction = new QWidgetAction(this);
	QFrame* frame = new QFrame(this);
	QHBoxLayout* layout = new QHBoxLayout(frame);

	QLabel* label = new QLabel(i18n("new:"), frame);
	layout->addWidget(label);

	KLineEdit* leFilename = new KLineEdit("", frame);
	layout->addWidget(leFilename);
	connect(leFilename, SIGNAL(returnPressed(QString)), this, SLOT(saveNewSelected(QString)));
	connect(leFilename, SIGNAL(returnPressed(QString)), &menu, SLOT(close()));

	widgetAction->setDefaultWidget(frame);
	menu.addAction(widgetAction);

	QPoint pos(-menu.sizeHint().width()+tbSave->width(),-menu.sizeHint().height());
	menu.exec(tbSave->mapToGlobal(pos));

	//TODO: focus is not set. why?
	leFilename->setFocus();
}

/*!
 * Is called when the current properties are going to be saved as a new template.
 * Emits \c saveConfigRequested, the receiver of the signal has to config.sync().
 */
void TemplateHandler::saveNewSelected(const QString& filename) {
    KConfig config(QStandardPaths::locate(QStandardPaths::ApplicationsLocation, "templates") + '/' + dirNames.at(className) + '/' + filename, KConfig::SimpleConfig);
	emit (saveConfigRequested(config));

	//we have at least one saved template now -> enable the load button
	tbLoad->setEnabled(true);

	emit info( i18n("New template \"%1\" was saved.", filename) );
}

/*!
 * Is called when the current properties are going to be saved in an already available template.
 * Emits \c saveConfigRequested, the receiver of the signal has to config.sync().
 */
void TemplateHandler::saveMenuSelected(QAction* action) {
	KConfig config(action->data().toString()+'/'+action->text(), KConfig::SimpleConfig);
	emit (saveConfigRequested(config));

	emit info( i18n("Template \"%1\" was saved.", action->text()) );
}

/*!
 * Is called when the current properties are going to be saved as new default properties.
 * Emits \c saveConfigRequested, the receiver of the signal has to config.sync().
 */
void TemplateHandler::saveDefaults() {
	KConfig config;
	emit (saveConfigRequested(config));

	emit info( i18n("New default template was saved.") );
}
