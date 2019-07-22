/***************************************************************************
    File                 : ProjectExplorer.cpp
    Project              : LabPlot
    Description       	 : A tree view for displaying and editing an AspectTreeModel.
    --------------------------------------------------------------------
    Copyright            : (C) 2007-2008 by Tilman Benkert (thzs@gmx.net)
    Copyright            : (C) 2011-2016 Alexander Semke (alexander.semke@web.de)
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

#include <QWidget>

class AbstractAspect;
class AspectTreeModel;
class Project;
class XmlStreamReader;

class QFrame;
class QLabel;
class QLineEdit;
class QModelIndex;
class QPushButton;
class QSignalMapper;
class QTreeView;
class QXmlStreamWriter;
class QItemSelection;
class QMenu;

class ProjectExplorer : public QWidget {
	Q_OBJECT

public:
	explicit ProjectExplorer(QWidget* parent = nullptr);

	void setCurrentAspect(const AbstractAspect*);
	void setModel(AspectTreeModel*);
	void setProject(Project*);
	QModelIndex currentIndex() const;
	void updateSelectedAspects();

private:
	void createActions();
	void contextMenuEvent(QContextMenuEvent*) override;
	bool eventFilter(QObject*, QEvent*) override;
	void collapseParents(const QModelIndex&, const QList<QModelIndex>& expanded);
	bool filter(const QModelIndex&, const QString&);
	int m_columnToHide{0};
	QTreeView* m_treeView;
	Project* m_project{nullptr};
	QPoint m_dragStartPos;
	bool m_dragStarted{false};

	QAction* caseSensitiveAction;
	QAction* matchCompleteWordAction;
	QAction* expandTreeAction;
	QAction* expandSelectedTreeAction;
	QAction* collapseTreeAction;
	QAction* collapseSelectedTreeAction;
	QAction* deleteSelectedTreeAction;
	QAction* toggleFilterAction;
	QAction* showAllColumnsAction;
	QList<QAction*> list_showColumnActions;
	QSignalMapper* showColumnsSignalMapper;

	QFrame* m_frameFilter;
	QLineEdit* m_leFilter;
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
	void resizeHeader();
	void expandSelected();
	void collapseSelected();
	void deleteSelected();

	void navigateTo(const QString& path);
	void currentChanged(const QModelIndex& current, const QModelIndex& previous);
	void selectIndex(const QModelIndex&);
	void deselectIndex(const QModelIndex&);
	void selectionChanged(const QItemSelection&, const QItemSelection&);

	void save(QXmlStreamWriter*) const;
	bool load(XmlStreamReader*);

signals:
	void currentAspectChanged(AbstractAspect*);
	void selectedAspectsChanged(QList<AbstractAspect*>&);
	void hiddenAspectSelected(const AbstractAspect*);
};

#endif // ifndef PROJECT_EXPLORER_H
