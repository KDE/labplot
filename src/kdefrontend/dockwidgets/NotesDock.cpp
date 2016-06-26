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

NotesDock::NotesDock(QWidget *parent) : QWidget(parent), m_initializing(false) {
	ui.setupUi(this);
	
	connect(ui.leName, SIGNAL(returnPressed(QString)), this, SLOT(nameChanged(QString)));
	connect(ui.leComment, SIGNAL(returnPressed(QString)), this, SLOT(commentChanged(QString)));
	
	connect(ui.kcbBgColor, SIGNAL(changed(QColor)), this, SLOT(bgColorChanged(QColor)));
	connect(ui.kcbTextColor, SIGNAL(changed(QColor)), this, SLOT(textColorChanged(QColor)));
}

NotesDock::~NotesDock() {
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
	m_notes = list.first();
	ui.leName->setText(m_notes->name());
	m_initializing=false;
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
