/***************************************************************************
    File                 : NotesDock.cpp
    Project              : LabPlot
    Description          : Notes Dock for configuring notes
    --------------------------------------------------------------------
    Copyright            : (C) 2016-2016 Garvit Khatri (garvitdelhi@gmail.com)

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

#include "NotesDock.h"
#include "kdefrontend/TemplateHandler.h"

#include <QDir>

NotesDock::NotesDock(QWidget *parent) : QWidget(parent), m_initializing(false) {
	ui.setupUi(this);
	
	connect(ui.leName, SIGNAL(returnPressed(QString)), this, SLOT(nameChanged(QString)));
	connect(ui.leComment, SIGNAL(returnPressed(QString)), this, SLOT(commentChanged(QString)));
	
	connect(ui.kcbBgColor, SIGNAL(changed(QColor)), this, SLOT(bgColorChanged(QColor)));
	connect(ui.kcbTextColor, SIGNAL(changed(QColor)), this, SLOT(textColorChanged(QColor)));

	TemplateHandler* templateHandler = new TemplateHandler(this, TemplateHandler::Worksheet);
	ui.gridLayout->addWidget(templateHandler, 7, 0);
	templateHandler->show();
	connect(templateHandler, SIGNAL(loadConfigRequested(KConfig&)), this, SLOT(loadConfigFromTemplate(KConfig&)));
	connect(templateHandler, SIGNAL(saveConfigRequested(KConfig&)), this, SLOT(saveConfigAsTemplate(KConfig&)));
}

NotesDock::~NotesDock() {
}

void NotesDock::init() {
	ui.kcbBgColor->setColor(m_notes->bgColor());
	ui.kcbTextColor->setColor(m_notes->textColor());
}


void NotesDock::bgColorChanged(QColor color) {
	if(m_initializing)
		return;
	m_notes->changeBgColor(color);
}

void NotesDock::textColorChanged(QColor color) {
	if(m_initializing)
		return;
	m_notes->changeTextColor(color);
}

void NotesDock::setNotesList(QList< Notes* > list) {
	m_initializing=true;
	m_notesList = list;
	m_notes = list.first();
	ui.leName->setText(m_notes->name());
	m_initializing=false;
	init();
}

void NotesDock::nameChanged(QString name) {
	if(m_initializing)
		return;
	m_notes->setName(name);
}

void NotesDock::commentChanged(QString name) {
	if(m_initializing)
		return;
	m_notes->setComment(name);
}

void NotesDock::loadConfigFromTemplate(KConfig& config) {

	QString name;
	int index = config.name().lastIndexOf(QDir::separator());
	if (index!=-1)
		name = config.name().right(config.name().size() - index - 1);
	else
		name = config.name();

	int size = m_notesList.size();
	if (size>1)
		m_notes->beginMacro(i18n("%1 Notes: template \"%2\" loaded", size, name));
	else
		m_notes->beginMacro(i18n("%1: template \"%2\" loaded", m_notes->name(), name));

	KConfigGroup group = config.group( "Notes" );
	ui.kcbBgColor->setColor(group.readEntry("bgColor", m_notes->bgColor()));
	ui.kcbTextColor->setColor(group.readEntry("textColor", m_notes->textColor()));

	m_notes->endMacro();
}

void NotesDock::saveConfigAsTemplate(KConfig& config) {
	KConfigGroup group = config.group( "Notes" );

	group.writeEntry("bgColor", ui.kcbBgColor->color());
	group.writeEntry("textColor", ui.kcbTextColor->color());
}
