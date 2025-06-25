/*
	File                 : NotesView.cpp
	Project              : LabPlot
	Description          : Notes View for taking notes
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2016 Garvit Khatri <garvitdelhi@gmail.com>
	SPDX-FileCopyrightText: 2016-2024 Alexander Semke <alexander.semke@web.de>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef NOTEVIEW_H
#define NOTEVIEW_H

#include <QWidget>

class Note;
class QTextEdit;
class QPrinter;

class NoteView : public QWidget {
	Q_OBJECT

public:
	explicit NoteView(Note*);

public Q_SLOTS:
	void print(QPrinter*) const;

private Q_SLOTS:
	// SLOTs for changes triggered in Note
	void noteTextChanged(const QString&);
	void noteBackgroundColorChanged(const QColor&);
	void noteTextColorChanged(const QColor&);
	void noteTextFontChanged(const QFont&);

private:
	Note* m_note;
	QTextEdit* m_textEdit;
	bool m_initializing{false};

	int m_textChangedTimerId{-1};
	void timerEvent(QTimerEvent*) override;
};

#endif // NOTEVIEW_H
