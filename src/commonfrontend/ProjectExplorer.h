/***************************************************************************
    File                 : ProjectExplorer.cpp
    Project              : SciDAVis/Labplot2
    Description       	 : A tree view for displaying and editing an AspectTreeModel.
    --------------------------------------------------------------------
    Copyright            : (C) 2007 by Knut Franke (knut.franke*gmx.de)
    Copyright            : (C) 2007-2008 by Tilman Benkert (thzs*gmx.net)
    Copyright            : (C) 2011-2013 Alexander Semke (alexander.semke*web.de)
							(replace * with @ in the email addresses)
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
#ifndef PROJECT_EXPLORER_H
#define PROJECT_EXPLORER_H

#include <QTreeView>

class AbstractAspect;
class QPushButton;
class QLabel;
class QSignalMapper;
class Project;

class ProjectExplorer : public QWidget{
	Q_OBJECT

	public:
		ProjectExplorer(QWidget* parent = 0);

		void setCurrentAspect(const AbstractAspect*);
		void setModel(QAbstractItemModel*);
		void setProject(const Project*);
		QModelIndex currentIndex() const;
		QAbstractItemModel* model() const;
		
	private:
		void createActions();
	  	void contextMenuEvent(QContextMenuEvent*);
		bool eventFilter(QObject*, QEvent*);
		int m_columnToHide;
		QTreeView* m_treeView;
		
		QAction* caseSensitiveAction;
		QAction* matchCompleteWordAction;
		QAction* expandTreeAction;
		QAction* collapseTreeAction;
		QAction* toggleFilterAction;
		QAction* showAllColumnsAction;
		QList<QAction*> list_showColumnActions;
		QSignalMapper* showColumnsSignalMapper;
		
		QFrame* frameFilter;
		QLabel* lFilter;
		QLineEdit* leFilter;
		QPushButton* bClearFilter;
		QPushButton* bFilterOptions;
	
	private slots:
		void aspectAdded(const AbstractAspect*);
		void toggleColumn(int);
		void showAllColumns();
		void filterTextChanged(const QString&);
		void toggleFilterCaseSensitivity();
		void toggleFilterMatchCompleteWord();
		void toggleFilterWidgets();
		void toggleFilterOptionsMenu(bool);
		
		void currentChanged(const QModelIndex& current, const QModelIndex& previous);
		void selectIndex(const QModelIndex&);
		void deselectIndex(const QModelIndex&);
		void selectionChanged(const QItemSelection&, const QItemSelection&);
		
	signals:
		void currentAspectChanged(AbstractAspect*);
		void selectedAspectsChanged(QList<AbstractAspect*>&);
		void hiddenAspectSelected(const AbstractAspect*);
};

#endif // ifndef PROJECT_EXPLORER_H
