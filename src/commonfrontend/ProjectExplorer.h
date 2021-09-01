/*
    File                 : ProjectExplorer.cpp
    Project              : LabPlot
    Description       	 : A tree view for displaying and editing an AspectTreeModel.
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2007-2008 Tilman Benkert (thzs@gmx.net)
    SPDX-FileCopyrightText: 2011-2016 Alexander Semke (alexander.semke@web.de)

    SPDX-License-Identifier: GPL-2.0-or-later
*/
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
class QToolButton;
class QSignalMapper;
class QTreeView;
class QXmlStreamWriter;
class QItemSelection;
class QMenu;

class KMessageWidget;

class ProjectExplorer : public QWidget {
	Q_OBJECT

public:
	explicit ProjectExplorer(QWidget* parent = nullptr);
	~ProjectExplorer() override;

	void setCurrentAspect(const AbstractAspect*);
	void setModel(AspectTreeModel*);
	void setProject(Project*);
	QModelIndex currentIndex() const;
	AbstractAspect* currentAspect() const;
	void updateSelectedAspects();
	void search();

private:
	void createActions();
	void contextMenuEvent(QContextMenuEvent*) override;
	bool eventFilter(QObject*, QEvent*) override;
	void keyPressEvent(QKeyEvent*) override;
	void collapseParents(const QModelIndex&, const QList<QModelIndex>& expanded);
	bool filter(const QModelIndex&, const QString&);
	void showErrorMessage(const QString&);

	int m_columnToHide{0};
	QTreeView* m_treeView;
	Project* m_project{nullptr};
	KMessageWidget* m_messageWidget{nullptr};
	QPoint m_dragStartPos;
	bool m_dragStarted{false};
	bool m_changeSelectionFromView{false};

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
	QToolButton* bFilterOptions;

private slots:
	void projectLoaded();
	void aspectAdded(const AbstractAspect*);
	void toggleColumn(int);
	void showAllColumns();
	void filterTextChanged(const QString&);
	void toggleFilterCaseSensitivity();
	void toggleFilterMatchCompleteWord();
	void toggleFilterOptionsMenu(bool);
	void resizeHeader();
	void expandSelected();
	void collapseSelected();
	void deleteSelected();

	void navigateTo(const QString& path);
	void selectIndex(const QModelIndex&);
	void deselectIndex(const QModelIndex&);
	void selectionChanged(const QItemSelection& selected, const QItemSelection& deselected);

	void save(QXmlStreamWriter*) const;
	bool load(XmlStreamReader*);

signals:
	void currentAspectChanged(AbstractAspect*);
	void activateView(AbstractAspect*);
	void selectedAspectsChanged(QList<AbstractAspect*>&);
	void hiddenAspectSelected(const AbstractAspect*);
};

#endif // ifndef PROJECT_EXPLORER_H
