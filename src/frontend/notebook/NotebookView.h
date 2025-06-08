/*
	File                 : NotebookView.h
	Project              : LabPlot
	Description          : View class for Notebeook
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2015 Garvit Khatri <garvitdelhi@gmail.com>
	SPDX-FileCopyrightText: 2016-2025 Alexander Semke <alexander.semke@web.de>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef NOTEBOOKVIEW_H
#define NOTEBOOKVIEW_H

#include <QWidget>
#ifdef HAVE_CANTOR_LIBS
#include <cantor/session.h>
#endif

class Column;
class Notebook;

class QActionGroup;
class QMenu;

namespace KParts {
class ReadWritePart;
}

class NotebookView : public QWidget {
	Q_OBJECT

public:
	explicit NotebookView(Notebook*);
	~NotebookView() override;

public Q_SLOTS:
	void createContextMenu(QMenu*);
	void fillColumnsContextMenu(QMenu*, const QVector<Column*>&);

	void evaluate();
	void restart();
	void zoomIn();
	void zoomOut();
	void find();

private Q_SLOTS:
	void triggerAction(QAction*);

private:
	Notebook* m_notebook;
	KParts::ReadWritePart* m_part{nullptr};

	QActionGroup* m_actionGroup{nullptr};
	QAction* m_evaluateEntryAction{nullptr};
	QAction* m_removeCurrentEntryAction{nullptr};
	QAction* m_restartAction{nullptr};
	QAction* m_evaluateAction{nullptr};
	QAction* m_zoomInAction{nullptr};
	QAction* m_zoomOutAction{nullptr};
	QAction* m_findAction{nullptr};
	QAction* m_replaceAction{nullptr};
	QAction* m_statisticsAction{nullptr};
	QVector<Column*> m_contextMenuColumns;

	QMenu* m_addNewMenu{nullptr};
	QMenu* m_plotDataMenu{nullptr};
	QMenu* m_linearAlgebraMenu{nullptr};
	QMenu* m_calculateMenu{nullptr};
	QMenu* m_settingsMenu{nullptr};

	void initActions();
	void initMenus();

private Q_SLOTS:
	void statusChanged(Cantor::Session::Status);
	void plotData(QAction*);
	void showStatistics();
};

#endif // NOTEBOOKVIEW_H
