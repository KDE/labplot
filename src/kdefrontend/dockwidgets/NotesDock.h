/***************************************************************************
    File                 : NotesDock.h
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

#ifndef NOTESDOCK_H
#define NOTESDOCK_H

#include <QWidget>
#include "backend/notes/Notes.h"
#include "ui_notesdock.h"

class NotesDock : public QWidget {
	Q_OBJECT

	public:
		explicit NotesDock(QWidget *parent);
		~NotesDock();
		
		void setNotesList(QList<Notes*>);

	private:
		Ui::NotesDock ui;
		bool m_initializing;
		Notes* m_notes;
		QList< Notes* > m_notesList;

		void init();

	private slots:
		//SLOTs for changes triggered in WorksheetDock
		//"General"-tab
		void nameChanged(QString);
		void commentChanged(QString);
		void bgColorChanged(QColor);
		void textColorChanged(QColor);
		void loadConfigFromTemplate(KConfig&);
		void saveConfigAsTemplate(KConfig&);

		//SLOTs for changes triggered in Worksheet
// 		void worksheetDescriptionChanged(const AbstractAspect*);
	signals:
// 		void info(const QString&);
};

#endif // NOTESDOCK_H
