/***************************************************************************
    File                 : CantorWorksheetView.h
    Project              : LabPlot
    Description          : View class for CantorWorksheet
    --------------------------------------------------------------------
    Copyright            : (C) 2015 Garvit Khatri (garvitdelhi@gmail.com)

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

	QAction* m_evaluateEntryAction;
	QAction* m_insertCommandEntryAction;
	QAction* m_insertTextEntryAction;
	QAction* m_insertMarkdownEntryAction{nullptr};
	QAction* m_insertLatexEntryAction;
	QAction* m_insertPageBreakAction;
	QAction* m_removeCurrentEntryAction;
	QAction* m_computeEigenvectorsAction{nullptr};
	QAction* m_createMatrixAction{nullptr};
	QAction* m_computeEigenvaluesAction{nullptr};
	QAction* m_invertMatrixAction{nullptr};
	QAction* m_differentiationAction{nullptr};
	QAction* m_integrationAction{nullptr};
	QAction* m_solveEquationsAction{nullptr};
	QAction* m_restartBackendAction;
	QAction* m_evaluateWorsheetAction;
	QAction* m_zoomIn;
	QAction* m_zoomOut;
	QAction* m_find;
	QAction* m_replace;
	QAction* m_syntaxHighlighting;
	QAction* m_lineNumbers;
	QAction* m_animateWorksheet;
	QAction* m_latexTypesetting;
	QAction* m_showCompletion;

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
