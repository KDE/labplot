/*
	File                 : NotesView.cpp
	Project              : LabPlot
	Description          : Notes View for taking notes
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2016 Garvit Khatri <garvitdelhi@gmail.com>
	SPDX-FileCopyrightText: 2016-2026 Alexander Semke <alexander.semke@web.de>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef NOTEVIEW_H
#define NOTEVIEW_H

#include "backend/note/Note.h"
#include <QWidget>

class QTextEdit;
class QTextBrowser;
class QToolButton;
class QStackedWidget;
class QToolBar;
class QPrinter;

class NoteView : public QWidget {
	Q_OBJECT

public:
	explicit NoteView(Note*);
	bool eventFilter(QObject*, QEvent*) override;

public Q_SLOTS:
	void print(QPrinter*) const;

private Q_SLOTS:
	// SLOTs for changes triggered in Note
	void noteTextChanged(const QString&);
	void noteBackgroundColorChanged(const QColor&);
	void noteTextColorChanged(const QColor&);
	void noteTextFontChanged(const QFont&);
	void noteModeChanged(Note::Mode);

	void showEditMode();
	void showPreviewMode();
	void textEditChanged();

private:
	Note* m_note;
	QTextEdit* m_textEdit;
	QTextBrowser* m_preview;
	QToolButton* m_editButton;
	QToolButton* m_previewButton;
	QStackedWidget* m_stack;
	QToolBar* m_toolbar;
	bool m_initializing{false};
	bool m_viewMode{false}; // false = edit, true = preview

	int m_textChangedTimerId{-1};
	void timerEvent(QTimerEvent*) override;

	void setupToolbar();
	QString renderMarkdown(const QString& markdown);
	void updatePreview();
};

#endif // NOTEVIEW_H
