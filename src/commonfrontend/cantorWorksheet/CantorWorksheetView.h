/*
    File                 : CantorWorksheetView.h
    Project              : LabPlot
    Description          : View class for CantorWorksheet
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2015 Garvit Khatri (garvitdelhi@gmail.com)
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef CANTORWORKSHEETVIEW_H
#define CANTORWORKSHEETVIEW_H

#include <QWidget>
#include <cantor/session.h>

class QToolBar;
class QMenu;

class CantorWorksheet;
namespace KParts {
class ReadWritePart;
}

class CantorWorksheetView : public QWidget {
	Q_OBJECT

public:
	explicit CantorWorksheetView(CantorWorksheet*);
	~CantorWorksheetView() override;

public slots:
	void createContextMenu(QMenu*);
	void fillToolBar(QToolBar*);

private slots:
	void triggerCantorAction(QAction*);

private:
	CantorWorksheet* m_worksheet;
	KParts::ReadWritePart* m_part{nullptr};

	QAction* m_evaluateEntryAction{nullptr};
	QAction* m_insertCommandEntryAction{nullptr};
	QAction* m_insertTextEntryAction{nullptr};
	QAction* m_insertMarkdownEntryAction{nullptr};
	QAction* m_insertLatexEntryAction{nullptr};
	QAction* m_insertPageBreakAction{nullptr};
	QAction* m_removeCurrentEntryAction{nullptr};
	QAction* m_computeEigenvectorsAction{nullptr};
	QAction* m_createMatrixAction{nullptr};
	QAction* m_computeEigenvaluesAction{nullptr};
	QAction* m_invertMatrixAction{nullptr};
	QAction* m_differentiationAction{nullptr};
	QAction* m_integrationAction{nullptr};
	QAction* m_solveEquationsAction{nullptr};
	QAction* m_restartBackendAction{nullptr};
	QAction* m_evaluateWorsheetAction{nullptr};
	QAction* m_zoomIn{nullptr};
	QAction* m_zoomOut{nullptr};
	QAction* m_find{nullptr};
	QAction* m_replace{nullptr};
	QAction* m_syntaxHighlighting{nullptr};
	QAction* m_lineNumbers{nullptr};
	QAction* m_animateWorksheet{nullptr};
	QAction* m_latexTypesetting{nullptr};
	QAction* m_showCompletion{nullptr};

	QMenu* m_worksheetMenu{nullptr};
	QMenu* m_linearAlgebraMenu{nullptr};
	QMenu* m_calculateMenu{nullptr};
	QMenu* m_settingsMenu{nullptr};

	void initActions();
	void initMenus();

private slots:
	void statusChanged(Cantor::Session::Status);
};

#endif // CANTORWORKSHEETVIEW_H
