/*
    File                 : CantorWorksheetView.h
    Project              : LabPlot
    Description          : View class for CantorWorksheet
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2015 Garvit Khatri <garvitdelhi@gmail.com>
    SPDX-FileCopyrightText: 2016-2021 Alexander Semke <alexander.semke@web.de>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef CANTORWORKSHEETVIEW_H
#define CANTORWORKSHEETVIEW_H

#include <QWidget>
#include <cantor/session.h>

class QActionGroup;
class QMenu;
class QToolBar;

class CantorWorksheet;
namespace KParts {
class ReadWritePart;
}

class CantorWorksheetView : public QWidget {
	Q_OBJECT

public:
	explicit CantorWorksheetView(CantorWorksheet*);
	~CantorWorksheetView() override;

public Q_SLOTS:
	void createContextMenu(QMenu*);
	void fillToolBar(QToolBar*);

private Q_SLOTS:
	void triggerAction(QAction*);

private:
	CantorWorksheet* m_worksheet;
	KParts::ReadWritePart* m_part{nullptr};

	QActionGroup* m_actionGroup{nullptr};
	QAction* m_evaluateEntryAction{nullptr};
	QAction* m_removeCurrentEntryAction{nullptr};
	QAction* m_restartBackendAction{nullptr};
	QAction* m_evaluateWorsheetAction{nullptr};
	QAction* m_zoomIn{nullptr};
	QAction* m_zoomOut{nullptr};
	QAction* m_find{nullptr};
	QAction* m_replace{nullptr};

	QMenu* m_addNewMenu{nullptr};
	QMenu* m_linearAlgebraMenu{nullptr};
	QMenu* m_calculateMenu{nullptr};
	QMenu* m_settingsMenu{nullptr};

	void initActions();
	void initMenus();

private Q_SLOTS:
	void statusChanged(Cantor::Session::Status);
};

#endif // CANTORWORKSHEETVIEW_H
