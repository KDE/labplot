/*
	File                 : NotesDock.h
	Project              : LabPlot
	Description          : Dock for configuring notes
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2016 Garvit Khatri <garvitdelhi@gmail.com>
	SPDX-FileCopyrightText: 2016-2025 Alexander Semke <alexander.semke@web.de>

	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef NOTEDOCK_H
#define NOTEDOCK_H

#include "backend/note/Note.h"
#include "frontend/dockwidgets/BaseDock.h"
#include "ui_notedock.h"

class KConfig;

class NoteDock : public BaseDock {
	Q_OBJECT

public:
	explicit NoteDock(QWidget* parent);
	void setNotesList(QList<Note*>);
	void retranslateUi() override;

private:
	Ui::NoteDock ui;
	Note* m_note{nullptr};
	QList<Note*> m_notesList;

	void init();

private Q_SLOTS:
	// SLOTs for changes triggered in NoteDock
	void backgroundColorChanged(const QColor&);
	void textColorChanged(const QColor&);
	void textFontChanged(const QFont&);

	// SLOTs for changes triggered in Note
	void noteBackgroundColorChanged(const QColor&);
	void noteTextColorChanged(const QColor&);
	void noteTextFontChanged(const QFont&);

	void loadConfigFromTemplate(KConfig&);
	void saveConfigAsTemplate(KConfig&);
};

#endif // NOTEDOCK_H
