/*
	File                 : NotebookDock.h
	Project              : LabPlot
	Description          : widget for Notebook properties
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2015 Garvit Khatri <garvitdelhi@gmail.com>
	SPDX-FileCopyrightText: 2015-2024 Alexander Semke <alexander.semke@web.de>

	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef CANTORWORKSHEETDOCK_H
#define CANTORWORKSHEETDOCK_H

#include "kdefrontend/dockwidgets/BaseDock.h"
#include "ui_notebookdock.h"

class Notebook;

class NotebookDock : public BaseDock {
	Q_OBJECT

public:
	explicit NotebookDock(QWidget*);
	void setNotebooks(QList<Notebook*>);

private:
	Ui::NotebookDock ui;
	QList<Notebook*> m_notebooks;
	Notebook* m_notebook{nullptr};
	QList<int> index;

	// in the old Cantor the help panel plugin is coming as second
	// in the new code we determine the position via the plugin name
	// TODO: improve this logic without hard-coding for a fixed index,
	// or remove the support for the old code in Cantor completely
	int m_helpPanelIndex{1};

	int m_documentationPanelIndex{0};

private Q_SLOTS:
	// SLOTs for changes triggered in NotebookDock
	//"General"-tab
	void evaluate();
	void restartBackend();
	void visibilityRequested();

Q_SIGNALS:
	void info(const QString&);
};

#endif // CANTORWORKSHEETDOCK_H
