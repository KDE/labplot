/*
    File                 : NotesDock.h
    Project              : LabPlot
    Description          : Dock for configuring notes
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2016 Garvit Khatri <garvitdelhi@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

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
