/*
	File                 : ProjectExplorer.cpp
	Project              : LabPlot
	Description       	 : A tree view for displaying and editing an AspectTreeModel.
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2007-2008 Tilman Benkert <thzs@gmx.net>
	SPDX-FileCopyrightText: 2011-2021 Alexander Semke <alexander.semke@web.de>

	SPDX-License-Identifier: GPL-2.0-or-later
*/
#ifndef PROJECT_EXPLORER_H
#define PROJECT_EXPLORER_H

#include <QWidget>

#include <backend/core/column/Column.h>

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
	void search();
	// show sparkLine of respective column
        // show sparkLine of respective column
        static QPixmap showSparkLines(const Column*);


private:
	void createActions();
	void contextMenuEvent(QContextMenuEvent*) override;
	bool eventFilter(QObject*, QEvent*) override;
	void keyPressEvent(QKeyEvent*) override;
	void collapseParents(const QModelIndex&, const QList<QModelIndex>& expanded);
	bool filter(const QModelIndex&, const QString&);
	void showErrorMessage(const QString&);

	int m_columnToHide{0};
	QTreeView* m_treeView{nullptr};
	Project* m_project{nullptr};
	KMessageWidget* m_messageWidget{nullptr};
	QPoint m_dragStartPos;
	bool m_dragStarted{false};
	bool m_changeSelectionFromView{false};

	QAction* fuzzyMatchingAction{nullptr};
	QAction* caseSensitiveAction{nullptr};
	QAction* matchCompleteWordAction{nullptr};
	QAction* expandTreeAction{nullptr};
	QAction* expandSelectedTreeAction{nullptr};
	QAction* collapseTreeAction{nullptr};
	QAction* collapseSelectedTreeAction{nullptr};
	QAction* deleteSelectedTreeAction{nullptr};
	QAction* toggleFilterAction{nullptr};
	QAction* showAllColumnsAction{nullptr};
	QList<QAction*> list_showColumnActions;
	QSignalMapper* showColumnsSignalMapper{nullptr};

	QFrame* m_frameFilter{nullptr};
	QLineEdit* m_leFilter{nullptr};
	QPushButton* bFilterOptions{nullptr};

private Q_SLOTS:
	void aspectAdded(const AbstractAspect*);
	void toggleColumn(int);
	void showAllColumns();
	void filterTextChanged(const QString&);
	void toggleFilterOptionsMenu(bool);
	void resizeHeader();
	void expandSelected();
	void collapseSelected();
	void deleteSelected();
	void changeSelectedVisible();

	void navigateTo(const QString& path);
	void selectIndex(const QModelIndex&);
	void deselectIndex(const QModelIndex&);
	void selectionChanged(const QItemSelection& selected, const QItemSelection& deselected);

	void save(QXmlStreamWriter*) const;
	bool load(XmlStreamReader*);

Q_SIGNALS:
	void currentAspectChanged(AbstractAspect*);
	void activateView(AbstractAspect*);
	void selectedAspectsChanged(QList<AbstractAspect*>&);
	void hiddenAspectSelected(const AbstractAspect*);
};

#endif // ifndef PROJECT_EXPLORER_H
