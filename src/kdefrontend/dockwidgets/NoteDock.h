/***************************************************************************
    File                 : NotesDock.h
    Project              : LabPlot
    Description          : Dock for configuring notes
    --------------------------------------------------------------------
    Copyright            : (C) 2016 Garvit Khatri (garvitdelhi@gmail.com)

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

#ifndef NOTEDOCK_H
#define NOTEDOCK_H

#include <QWidget>
#include "backend/note/Note.h"
#include "kdefrontend/dockwidgets/BaseDock.h"
#include "ui_notedock.h"

class KConfig;

class NoteDock : public BaseDock {
	Q_OBJECT

public:
	explicit NoteDock(QWidget *parent);
	void setNotesList(QList<Note*>);

private:
	Ui::NoteDock ui;
	Note* m_notes{nullptr};
	QList<Note*> m_notesList;

	void init();

private slots:
	void backgroundColorChanged(const QColor&);
	void textColorChanged(const QColor&);
	void textFontChanged(const QFont&);

	void loadConfigFromTemplate(KConfig&);
	void saveConfigAsTemplate(KConfig&);
};

#endif // NOTEDOCK_H
