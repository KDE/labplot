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

class QAbstractItemModel;
class QToolBar;
class QMenu;

class CantorWorksheet;
namespace KParts {
class ReadWritePart;
}

class CantorWorksheetView : public QWidget {
	Q_OBJECT
    
	public:
		CantorWorksheetView(CantorWorksheet*);
		~CantorWorksheetView();
	
	public slots:
		void createContextMenu(QMenu*) const;
		void fillToolBar(QToolBar*);

	private slots:
		void triggerCantorAction(QAction*);

	private:
		CantorWorksheet* m_worksheet;
		QAction* m_restartBackendAction;
		QAction* m_evaluateWorsheetAction;
		QAction* m_evaluateEntryAction;
		QAction* m_insertCommandEntryAction;
		QAction* m_insertTextEntryAction;
		QAction* m_insertLatexEntryAction;
		QAction* m_insertPageBreakAction;
		QAction* m_removeCurrentEntryAction;
		QAction* m_computeEigenvectorsAction;
		QAction* m_createMattrixAction;
		QAction* m_computeEigenvaluesAction;
		QAction* m_invertMattrixAction;
		QAction* m_differentiationAction;
		QAction* m_integrationAction;
		QAction* m_solveEquationsAction;
		QAction* m_zoomIn;
		QAction* m_zoomOut;
		QAction* m_find;
		QAction* m_replace;
		QAction* m_syntaxHighlighting;
		QAction* m_completion;
		QAction* m_lineNumbers;
		QAction* m_animateWorksheet;
		QMenu* m_worksheetMenu;
		QMenu* m_linearAlgebraMenu;
		QMenu* m_calculateMenu;
		QMenu* m_settingsMenu;
		KParts::ReadWritePart* part;

		void initActions();
		void initMenus();
};

#endif // CANTORWORKSHEETVIEW_H
