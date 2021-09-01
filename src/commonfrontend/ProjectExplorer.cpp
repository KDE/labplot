
/***************************************************************************
    File                 : ProjectExplorer.cpp
    Project              : LabPlot
    Description       	 : A tree view for displaying and editing an AspectTreeModel.
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2007-2008 Tilman Benkert <thzs@gmx.net>
    SPDX-FileCopyrightText: 2010-2021 Alexander Semke <alexander.semke@web.de>

 **************************************************************************
    SPDX-License-Identifier: GPL-2.0-or-later
*/
#include "ProjectExplorer.h"
#include "backend/core/AspectTreeModel.h"
#include "backend/core/AbstractPart.h"
#include "backend/core/Project.h"
#include "backend/lib/XmlStreamReader.h"
#include "backend/worksheet/plots/cartesian/CartesianPlot.h"
#include "commonfrontend/core/PartMdiView.h"

#include <QContextMenuEvent>
#include <QDrag>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QMenu>
#include <QMimeData>
#include <QPushButton>
#include <QToolButton>
#include <QTreeView>
#include <QVBoxLayout>

#include <KConfig>
#include <KConfigGroup>
#include <KSharedConfig>
#include <KLocalizedString>
#include <KMessageWidget>
#include <KMessageBox>

/*!
  \class ProjectExplorer
  \brief A tree view for displaying and editing an AspectTreeModel.

  In addition to the functionality of QTreeView, ProjectExplorer allows
  the usage of the context menus provided by AspectTreeModel
  and propagates the item selection in the view to the model.
  Furthermore, features for searching and filtering in the model are provided.

  \ingroup commonfrontend
*/

ProjectExplorer::ProjectExplorer(QWidget* parent) :
	m_treeView(new QTreeView(parent)),
	m_frameFilter(new QFrame(this)) {

	auto* layout = new QVBoxLayout(this);
	layout->setSpacing(0);
	layout->setContentsMargins(0, 0, 0, 0);

	auto* layoutFilter = new QHBoxLayout(m_frameFilter);
	layoutFilter->setSpacing(0);
	layoutFilter->setContentsMargins(0, 0, 0, 0);

	m_leFilter = new QLineEdit(m_frameFilter);
	m_leFilter->setClearButtonEnabled(true);
	m_leFilter->setPlaceholderText(i18n("Search/Filter"));
	layoutFilter->addWidget(m_leFilter);

	bFilterOptions = new QToolButton(m_frameFilter);
	bFilterOptions->setIcon(QIcon::fromTheme("configure"));
	bFilterOptions->setCheckable(true);
	layoutFilter->addWidget(bFilterOptions);

	layout->addWidget(m_frameFilter);

	m_treeView->setAnimated(true);
	m_treeView->setAlternatingRowColors(true);
	m_treeView->setSelectionBehavior(QAbstractItemView::SelectRows);
	m_treeView->setSelectionMode(QAbstractItemView::ExtendedSelection);
	m_treeView->setUniformRowHeights(true);
	m_treeView->viewport()->installEventFilter(this);
	m_treeView->header()->setStretchLastSection(true);
	m_treeView->header()->installEventFilter(this);
	m_treeView->setDragEnabled(true);
	m_treeView->setAcceptDrops(true);
	m_treeView->setDropIndicatorShown(true);
	m_treeView->setDragDropMode(QAbstractItemView::InternalMove);

	layout->addWidget(m_treeView);

	this->createActions();

	connect(m_leFilter, &QLineEdit::textChanged, this, &ProjectExplorer::filterTextChanged);
	connect(bFilterOptions, &QPushButton::toggled, this, &ProjectExplorer::toggleFilterOptionsMenu);
}

ProjectExplorer::~ProjectExplorer() {
	// save the visible columns
	QString status;
	for (int i = 0; i < list_showColumnActions.size(); ++i) {
		if (list_showColumnActions.at(i)->isChecked()) {
			if (!status.isEmpty())
				status += QLatin1Char(' ');
			status += QString::number(i);
		}
	}
	KConfigGroup group(KSharedConfig::openConfig(), QLatin1String("ProjectExplorer"));
	group.writeEntry("VisibleColumns", status);
}

void ProjectExplorer::createActions() {
	caseSensitiveAction = new QAction(i18n("Case Sensitive"), this);
	caseSensitiveAction->setCheckable(true);
	caseSensitiveAction->setChecked(false);
	connect(caseSensitiveAction, &QAction::triggered, this, &ProjectExplorer::toggleFilterCaseSensitivity);

	matchCompleteWordAction = new QAction(i18n("Match Complete Word"), this);
	matchCompleteWordAction->setCheckable(true);
	matchCompleteWordAction->setChecked(false);
	connect(matchCompleteWordAction, &QAction::triggered, this, &ProjectExplorer::toggleFilterMatchCompleteWord);

	expandTreeAction = new QAction(QIcon::fromTheme(QLatin1String("expand-all")), i18n("Expand All"), this);
	connect(expandTreeAction, &QAction::triggered, m_treeView, &QTreeView::expandAll);

	expandSelectedTreeAction = new QAction(QIcon::fromTheme(QLatin1String("expand-all")), i18n("Expand Selected"), this);
	connect(expandSelectedTreeAction, &QAction::triggered, this, &ProjectExplorer::expandSelected);

	collapseTreeAction = new QAction(QIcon::fromTheme(QLatin1String("collapse-all")), i18n("Collapse All"), this);
	connect(collapseTreeAction, &QAction::triggered, m_treeView, &QTreeView::collapseAll);

	collapseSelectedTreeAction = new QAction(QIcon::fromTheme(QLatin1String("collapse-all")), i18n("Collapse Selected"), this);
	connect(collapseSelectedTreeAction, &QAction::triggered, this, &ProjectExplorer::collapseSelected);

	deleteSelectedTreeAction = new QAction(QIcon::fromTheme("edit-delete"), i18n("Delete Selected"), this);
	connect(deleteSelectedTreeAction, &QAction::triggered, this, &ProjectExplorer::deleteSelected);

	toggleFilterAction = new QAction(QIcon::fromTheme(QLatin1String("view-filter")), i18n("Search/Filter Options"), this);
	toggleFilterAction->setCheckable(true);
	toggleFilterAction->setChecked(true);
	connect(toggleFilterAction, &QAction::triggered, this, [=]() {m_frameFilter->setVisible(!m_frameFilter->isVisible());});

	showAllColumnsAction = new QAction(i18n("Show All"),this);
	showAllColumnsAction->setCheckable(true);
	showAllColumnsAction->setChecked(true);
	showAllColumnsAction->setEnabled(false);
	connect(showAllColumnsAction, &QAction::triggered, this, &ProjectExplorer::showAllColumns);
}

/*!
  shows the context menu in the tree. In addition to the context menu of the currently selected aspect,
  treeview specific options are added.
*/
void ProjectExplorer::contextMenuEvent(QContextMenuEvent *event) {
	if (!m_treeView->model())
		return;

	const QModelIndex& index = m_treeView->indexAt(m_treeView->viewport()->mapFrom(this, event->pos()));
	if (!index.isValid())
		m_treeView->clearSelection();

	const QModelIndexList& items = m_treeView->selectionModel()->selectedIndexes();
	QMenu* menu = nullptr;
	if (items.size()/4 == 1) {
		auto* aspect = static_cast<AbstractAspect*>(index.internalPointer());
		menu = aspect->createContextMenu();

		if (aspect == m_project) {
			auto* firstAction = menu->actions().at(2);
			menu->insertSeparator(firstAction);
			menu->insertAction(firstAction, expandTreeAction);
			menu->insertAction(firstAction, collapseTreeAction);
		}
	} else {
		menu = new QMenu();

		if (items.size()/4 > 1) {
			menu->addAction(expandSelectedTreeAction);
			menu->addAction(collapseSelectedTreeAction);
			menu->addSeparator();
			menu->addAction(deleteSelectedTreeAction);
		} else {
			QMenu* projectMenu = m_project->createContextMenu();
			projectMenu->setTitle(m_project->name());
			menu->addMenu(projectMenu);

			menu->addSeparator();
			menu->addAction(expandTreeAction);
			menu->addAction(collapseTreeAction);
			menu->addSeparator();
			menu->addAction(toggleFilterAction);

			//Menu for showing/hiding the columns in the tree view
			QMenu* columnsMenu = menu->addMenu(i18n("Columns"));
			columnsMenu->addAction(showAllColumnsAction);
			columnsMenu->addSeparator();
			for (auto* action : qAsConst(list_showColumnActions))
				columnsMenu->addAction(action);

			//TODO
			//Menu for showing/hiding the top-level aspects (Worksheet, Spreadhsheet, etc) in the tree view
			// QMenu* objectsMenu = menu->addMenu(i18n("Show/Hide objects"));
		}
	}

	if (menu)
		menu->exec(event->globalPos());

	delete menu;
}

void ProjectExplorer::setCurrentAspect(const AbstractAspect* aspect) {
	//HACK: when doing redo/undo in MainWin and an object is being deleted,
	//we don't want to jump to another object in the project explorer.
	//we reuse the aspectAddedSignalSuppressed also for the deletion of aspects.
	if (m_project->aspectAddedSignalSuppressed())
		return;

	const auto* tree_model = dynamic_cast<AspectTreeModel*>(m_treeView->model());
	if (tree_model)
		m_treeView->setCurrentIndex(tree_model->modelIndexOfAspect(aspect));
}

/*!
  Sets the \c model for the tree view to present.
*/
void ProjectExplorer::setModel(AspectTreeModel* treeModel) {
	m_treeView->setModel(treeModel);

	connect(treeModel, &AspectTreeModel::renameRequested,
			m_treeView,  static_cast<void (QAbstractItemView::*)(const QModelIndex&)>(&QAbstractItemView::edit));
	connect(treeModel, &AspectTreeModel::indexSelected, this, &ProjectExplorer::selectIndex);
	connect(treeModel, &AspectTreeModel::indexDeselected, this, &ProjectExplorer::deselectIndex);
	connect(treeModel, &AspectTreeModel::hiddenAspectSelected, this, &ProjectExplorer::hiddenAspectSelected);

	connect(m_treeView->selectionModel(), &QItemSelectionModel::selectionChanged, this, &ProjectExplorer::selectionChanged);

	//create action for showing/hiding the columns in the tree.
	//this is done here since the number of columns is  not available in createActions() yet.
	if (list_showColumnActions.isEmpty()) {
		//read the status of the column actions if available
		KConfigGroup group(KSharedConfig::openConfig(), QLatin1String("ProjectExplorer"));
		const QString& status = group.readEntry(QLatin1String("VisibleColumns"), QString());
		QVector<int> checkedActions;
		if (!status.isEmpty()) {
			QStringList strList = status.split(QLatin1Char(' '));
			for (int i = 0; i < strList.size(); ++i)
				checkedActions << strList.at(i).toInt();
		}

		if (checkedActions.size() != m_treeView->model()->columnCount()) {
			showAllColumnsAction->setEnabled(true);
			showAllColumnsAction->setChecked(false);
		} else {
			showAllColumnsAction->setEnabled(false);
			showAllColumnsAction->setChecked(true);
		}

		//create an action for every available column in the model
		for (int i = 0; i < m_treeView->model()->columnCount(); i++) {
			QAction* showColumnAction =  new QAction(treeModel->headerData(i, Qt::Horizontal).toString(), this);
			showColumnAction->setCheckable(true);

			//restore the status, if available
			if (!checkedActions.isEmpty()) {
				if (checkedActions.indexOf(i) != -1)
					showColumnAction->setChecked(true);
				else
					m_treeView->hideColumn(i);
			} else
				showColumnAction->setChecked(true);

			list_showColumnActions.append(showColumnAction);

			connect(showColumnAction, &QAction::triggered,
					this, [=] { ProjectExplorer::toggleColumn(i); });
		}
	} else {
		for (int i = 0; i < list_showColumnActions.size(); ++i) {
			if (!list_showColumnActions.at(i)->isChecked())
				m_treeView->hideColumn(i);
		}
	}
}

void ProjectExplorer::setProject(Project* project) {
	connect(project, &Project::aspectAdded, this, &ProjectExplorer::aspectAdded);
	connect(project, &Project::requestSaveState, this, &ProjectExplorer::save);
	connect(project, &Project::requestLoadState, this, &ProjectExplorer::load);
	connect(project, &Project::requestNavigateTo, this, &ProjectExplorer::navigateTo);
	connect(project, &Project::loaded, this, &ProjectExplorer::projectLoaded);
	m_project = project;

	//for newly created projects, resize the header to fit the size of the header section names.
	//for projects loaded from a file, this function will be called laterto fit the sizes
	//of the content once the project is loaded
	resizeHeader();
}

QModelIndex ProjectExplorer::currentIndex() const {
	return m_treeView->currentIndex();
}

AbstractAspect* ProjectExplorer::currentAspect() const {
	if (!currentIndex().isValid())
		return nullptr;
	return static_cast<AbstractAspect*>(currentIndex().internalPointer());
}

void ProjectExplorer::search() {
	m_leFilter->setFocus();
}

/*!
	handles the contextmenu-event of the horizontal header in the tree view.
	Provides a menu for selective showing and hiding of columns.
*/
bool ProjectExplorer::eventFilter(QObject* obj, QEvent* event) {
	if (obj == m_treeView->header() && event->type() == QEvent::ContextMenu) {
		//Menu for showing/hiding the columns in the tree view
		QMenu* columnsMenu = new QMenu(m_treeView->header());
		columnsMenu->addSection(i18n("Columns"));
		columnsMenu->addAction(showAllColumnsAction);
		columnsMenu->addSeparator();
		for (auto* action : qAsConst(list_showColumnActions))
			columnsMenu->addAction(action);

		auto* e = static_cast<QContextMenuEvent*>(event);
		columnsMenu->exec(e->globalPos());
		delete columnsMenu;

		return true;
	} else if (obj == m_treeView->viewport()) {
		if (event->type() == QEvent::MouseButtonPress) {
			auto* e = static_cast<QMouseEvent*>(event);
			if (e->button() == Qt::LeftButton) {
				QModelIndex index = m_treeView->indexAt(e->pos());
				if (!index.isValid())
					return false;

				auto* aspect = static_cast<AbstractAspect*>(index.internalPointer());
				if (aspect->isDraggable()) {
					m_dragStartPos = e->globalPos();
					m_dragStarted = false;
				}
			}
		} else if (event->type() == QEvent::MouseMove) {
			auto* e = static_cast<QMouseEvent*>(event);
			if ( !m_dragStarted && m_treeView->selectionModel()->selectedIndexes().size() > 0
				&& (e->globalPos() - m_dragStartPos).manhattanLength() >= QApplication::startDragDistance()) {

				m_dragStarted = true;
				auto* drag = new QDrag(this);
				auto* mimeData = new QMimeData;

				//determine the selected objects and serialize the pointers to QMimeData
				QVector<quintptr> vec;
				QModelIndexList items = m_treeView->selectionModel()->selectedIndexes();
				//there are four model indices in each row -> divide by 4 to obtain the number of selected rows (=aspects)
				for (int i = 0; i < items.size()/4; ++i) {
					const QModelIndex& index = items.at(i*4);
					auto* aspect = static_cast<AbstractAspect*>(index.internalPointer());
					vec << (quintptr)aspect;
				}

				QByteArray data;
				QDataStream stream(&data, QIODevice::WriteOnly);
				stream << (quintptr)m_project; //serialize the project pointer first, will be used as the unique identifier
				stream << vec;

				mimeData->setData("labplot-dnd", data);
				drag->setMimeData(mimeData);
				drag->exec();
			}
		} else if (event->type() == QEvent::DragEnter) {
			//ignore events not related to internal drags of columns etc., e.g. dropping of external files onto LabPlot
			auto* dragEnterEvent = static_cast<QDragEnterEvent*>(event);
			const QMimeData* mimeData = dragEnterEvent->mimeData();
			if (!mimeData) {
				event->ignore();
				return false;
			}

			if (mimeData->formats().at(0) != QLatin1String("labplot-dnd")) {
				event->ignore();
				return false;
			} else {
				//drad&drop between the different project windows is not supported yet.
				//check whether we're dragging inside of the same project.
				QByteArray data = mimeData->data(QLatin1String("labplot-dnd"));
				QDataStream stream(&data, QIODevice::ReadOnly);
				quintptr ptr = 0;
				stream >> ptr;
				auto* project = reinterpret_cast<Project*>(ptr);
				if (project != m_project) {
					event->ignore();
					return false;
				}
			}

			event->setAccepted(true);
		} else if (event->type() == QEvent::DragMove) {
			auto* dragMoveEvent = static_cast<QDragEnterEvent*>(event);
			const QMimeData* mimeData = dragMoveEvent->mimeData();
			QVector<quintptr> vec = m_project->droppedAspects(mimeData);

			AbstractAspect* sourceAspect{nullptr};
			if (!vec.isEmpty())
				sourceAspect = reinterpret_cast<AbstractAspect*>(vec.at(0));

			if (!sourceAspect)
				return false;

			//determine the aspect under the cursor
			QModelIndex index = m_treeView->indexAt(dragMoveEvent->pos());
			if (!index.isValid())
				return false;

			//accept only the events when the aspect being dragged is dropable onto the aspect under the cursor
			//and the aspect under the cursor is not already the parent of the dragged aspect
			auto* destinationAspect = static_cast<AbstractAspect*>(index.internalPointer());
			bool accept = sourceAspect->dropableOn().indexOf(destinationAspect->type()) != -1
						&& sourceAspect->parentAspect() != destinationAspect;
			event->setAccepted(accept);
		} else if (event->type() == QEvent::Drop) {
			auto* dropEvent = static_cast<QDropEvent*>(event);
			const QMimeData* mimeData = dropEvent->mimeData();
			if (!mimeData)
				return false;

			QVector<quintptr> vec = m_project->droppedAspects(mimeData);
			if (vec.isEmpty())
				return false;

			QModelIndex index = m_treeView->indexAt(dropEvent->pos());
			if (!index.isValid())
				return false;

			//process the dropped objects
			auto* aspect = static_cast<AbstractAspect*>(index.internalPointer());
			aspect->processDropEvent(vec);

			//expand the current aspect to see the dropped objects
			const auto* model = static_cast<const AspectTreeModel*>(m_treeView->model());
			m_treeView->setExpanded(model->modelIndexOfAspect(aspect), true);
		}
	}

	return QObject::eventFilter(obj, event);
}

void ProjectExplorer::keyPressEvent(QKeyEvent* event) {
	//current selected aspect
	auto* aspect = static_cast<AbstractAspect*>(m_treeView->currentIndex().internalPointer());

	if (event->matches(QKeySequence::Delete))
		deleteSelected();
	else if (event->matches(QKeySequence::Copy)) {
		//copy
		if (aspect != m_project) {
			aspect->copy();
			showErrorMessage(QString());
		}
	} else if (event->matches(QKeySequence::Paste)) {
		//paste
		QString name;
		auto t = AbstractAspect::clipboardAspectType(name);
		if (t != AspectType::AbstractAspect && aspect->pasteTypes().indexOf(t) != -1) {
			aspect->paste();
			showErrorMessage(QString());
		} else {
			QString msg = i18n("'%1' cannot be pasted in %2.", name, aspect->name());
			showErrorMessage(msg);

		}
	} else if ( (event->modifiers() & Qt::ControlModifier) && (event->key() == Qt::Key_D)) {
		//duplicate
		if (aspect != m_project) {
			aspect->copy();
			aspect->parentAspect()->paste(true);
			showErrorMessage(QString());
		}
	} else if (event->key() == 32) {
		//space key - hide/show the current object
		auto* we = dynamic_cast<WorksheetElement*>(aspect);
		if (we)
			we->setVisible(!we->isVisible());
	}
}

void ProjectExplorer::showErrorMessage(const QString& message) {
	if (message.isEmpty()) {
		if (m_messageWidget && m_messageWidget->isVisible())
			m_messageWidget->close();
	} else {
		if (!m_messageWidget) {
			m_messageWidget = new KMessageWidget(this);
			m_messageWidget->setMessageType(KMessageWidget::Error);
			layout()->addWidget(m_messageWidget);
		}
		m_messageWidget->setText(message);
		m_messageWidget->animatedShow();
	}
}

//##############################################################################
//#################################  SLOTS  ####################################
//##############################################################################
/*!
 * called after the project was loaded.
 * resize the header of the view to adjust to the content
 * and re-select the currently selected object to show
 * its final properties in the dock widget after in Project::load() all
 * pointers were restored, relative paths replaced by absolute, etc.
 */
void ProjectExplorer::projectLoaded() {
	resizeHeader();
	selectionChanged(m_treeView->selectionModel()->selection(), QItemSelection());
}

/*!
  expand the aspect \c aspect (the tree index corresponding to it) in the tree view
  and makes it visible and selected. Called when a new aspect is added to the project.
 */
void ProjectExplorer::aspectAdded(const AbstractAspect* aspect) {
	if (m_project->isLoading() ||m_project->aspectAddedSignalSuppressed())
		return;

	//don't do anything if hidden aspects were added
	if (aspect->hidden())
		return;


	//don't do anything for newly added data spreadsheets of data picker curves
	if (aspect->inherits(AspectType::Spreadsheet) &&
		aspect->parentAspect()->inherits(AspectType::DatapickerCurve))
		return;

	const auto* tree_model = qobject_cast<AspectTreeModel*>(m_treeView->model());
	const auto& index =  tree_model->modelIndexOfAspect(aspect);

	//expand and make the aspect visible
	m_treeView->setExpanded(index, true);

	// newly added columns are only expanded but not selected, return here
	if (aspect->inherits(AspectType::Column)) {
		m_treeView->setExpanded(tree_model->modelIndexOfAspect(aspect->parentAspect()), true);
		return;
	}

	m_treeView->scrollTo(index);
	m_treeView->setCurrentIndex(index);
	m_treeView->header()->resizeSections(QHeaderView::ResizeToContents);
	m_treeView->header()->resizeSection(0, m_treeView->header()->sectionSize(0)*1.2);
}

void ProjectExplorer::navigateTo(const QString& path) {
	const auto* tree_model = dynamic_cast<AspectTreeModel*>(m_treeView->model());
	if (tree_model) {
		const auto& index = tree_model->modelIndexOfAspect(path);
		if (!index.isValid())
			return;
		m_treeView->scrollTo(index);
		m_treeView->setCurrentIndex(index);
		auto* aspect = static_cast<AbstractAspect*>(index.internalPointer());
		aspect->setSelected(true);
	}
}

void ProjectExplorer::toggleColumn(int index) {
	//determine the total number of checked column actions
	int checked = 0;
	for (const auto* action : qAsConst(list_showColumnActions)) {
		if (action->isChecked())
			checked++;
	}

	if (list_showColumnActions.at(index)->isChecked()) {
		m_treeView->showColumn(index);
		m_treeView->header()->resizeSection(0,0 );
		m_treeView->header()->resizeSections(QHeaderView::ResizeToContents);

		for (auto* action : qAsConst(list_showColumnActions))
			action->setEnabled(true);

		//deactivate the "show all column"-action, if all actions are checked
		if ( checked == list_showColumnActions.size() ) {
			showAllColumnsAction->setEnabled(false);
			showAllColumnsAction->setChecked(true);
		}
	} else {
		m_treeView->hideColumn(index);
		showAllColumnsAction->setEnabled(true);
		showAllColumnsAction->setChecked(false);

		//if there is only one checked column-action, deactivated it.
		//It should't be possible to hide all columns
		if ( checked == 1 ) {
			int i = 0;
			while ( !list_showColumnActions.at(i)->isChecked() )
				i++;

			list_showColumnActions.at(i)->setEnabled(false);
		}
	}
}

void ProjectExplorer::showAllColumns() {
	for (int i = 0; i < m_treeView->model()->columnCount(); i++) {
		m_treeView->showColumn(i);
		m_treeView->header()->resizeSection(0,0 );
		m_treeView->header()->resizeSections(QHeaderView::ResizeToContents);
	}
	showAllColumnsAction->setEnabled(false);

	for (auto* action : qAsConst(list_showColumnActions)) {
		action->setEnabled(true);
		action->setChecked(true);
	}
}

/*!
  toggles the menu for the filter/search options
*/
void ProjectExplorer::toggleFilterOptionsMenu(bool checked) {
	if (checked) {
		QMenu menu;
		menu.addAction(caseSensitiveAction);
		menu.addAction(matchCompleteWordAction);
		connect(&menu, &QMenu::aboutToHide, bFilterOptions, &QPushButton::toggle);
		menu.exec(bFilterOptions->mapToGlobal(QPoint(0,bFilterOptions->height())));
	}
}

void ProjectExplorer::resizeHeader() {
	m_treeView->header()->resizeSections(QHeaderView::ResizeToContents);
	m_treeView->header()->resizeSection(0, m_treeView->header()->sectionSize(0)*1.2); //make the column "Name" somewhat bigger
}

/*!
  called when the filter/search text was changend.
*/
void ProjectExplorer::filterTextChanged(const QString& text) {
	QModelIndex root = m_treeView->model()->index(0,0);
	filter(root, text);
}

bool ProjectExplorer::filter(const QModelIndex& index, const QString& text) {
	Qt::CaseSensitivity sensitivity = caseSensitiveAction->isChecked() ? Qt::CaseSensitive : Qt::CaseInsensitive;
	bool matchCompleteWord = matchCompleteWordAction->isChecked();

	bool childVisible = false;
	const int rows = index.model()->rowCount(index);
	for (int i = 0; i < rows; i++) {
		QModelIndex child = index.model()->index(i, 0, index);
		auto* aspect =  static_cast<AbstractAspect*>(child.internalPointer());
		bool visible;
		if (text.isEmpty())
			visible = true;
		else if (matchCompleteWord)
			visible = aspect->name().startsWith(text, sensitivity);
		else
			visible = aspect->name().contains(text, sensitivity);

		if (visible) {
			//current item is visible -> make all its children visible without applying the filter
			for (int j = 0; j < child.model()->rowCount(child); ++j) {
				m_treeView->setRowHidden(j, child, false);
				if (text.isEmpty())
					filter(child, text);
			}

			childVisible = true;
		} else {
			//check children items. if one of the children is visible, make the parent (current) item visible too.
			visible = filter(child, text);
			if (visible)
				childVisible = true;
		}

		m_treeView->setRowHidden(i, index, !visible);
	}

	return childVisible;
}

void ProjectExplorer::toggleFilterCaseSensitivity() {
	filterTextChanged(m_leFilter->text());
}

void ProjectExplorer::toggleFilterMatchCompleteWord() {
	filterTextChanged(m_leFilter->text());
}

void ProjectExplorer::selectIndex(const QModelIndex&  index) {
	DEBUG(Q_FUNC_INFO)
	if (m_project->isLoading())
		return;

	if ( !m_treeView->selectionModel()->isSelected(index) ) {
		m_changeSelectionFromView = true;
		m_treeView->selectionModel()->select(index, QItemSelectionModel::Select | QItemSelectionModel::Rows);
		m_treeView->setExpanded(index, true);
		m_treeView->scrollTo(index);
	}
}

void ProjectExplorer::deselectIndex(const QModelIndex & index) {
	if (m_project->isLoading())
		return;

	if ( m_treeView->selectionModel()->isSelected(index) ) {
		m_changeSelectionFromView = true;
		m_treeView->selectionModel()->select(index, QItemSelectionModel::Deselect | QItemSelectionModel::Rows);
	}
}

void ProjectExplorer::selectionChanged(const QItemSelection &selected, const QItemSelection &deselected) {
	DEBUG(Q_FUNC_INFO)
	if (m_project->isLoading())
		return;

	QModelIndex index;
	AbstractAspect* aspect = nullptr;

	//there are four model indices in each row
	//-> divide by 4 to obtain the number of selected rows (=aspects)
	QModelIndexList items = selected.indexes();
	for (int i = 0; i < items.size()/4; ++i) {
		index = items.at(i*4);
		aspect = static_cast<AbstractAspect*>(index.internalPointer());
		aspect->setSelected(true);
	}

	items = deselected.indexes();
	for (int i = 0; i < items.size()/4; ++i) {
		index = items.at(i*4);
		aspect = static_cast<AbstractAspect*>(index.internalPointer());
		aspect->setSelected(false);
	}

	items = m_treeView->selectionModel()->selectedRows();
	QList<AbstractAspect*> selectedAspects;
	for (const QModelIndex& index : qAsConst(items)) {
		aspect = static_cast<AbstractAspect*>(index.internalPointer());
		selectedAspects << aspect;
	}

	//notify GuiObserver about the new selection
	emit selectedAspectsChanged(selectedAspects);

	//notify MainWin about the new current aspect (last selected aspect).
	if (!selectedAspects.isEmpty())
		emit currentAspectChanged(selectedAspects.last());

	//emitting the signal above is done to show the properties widgets for the selected aspect(s).
	//with this the project explorer looses the focus and don't react on the key events like DEL key press, etc.
	//If we explicitly select an item in the project explorer (not via a selection in the view), we want to keep the focus here.
	//TODO: after the focus is set again we react on DEL in the event filter, but navigation with the arrow keys in the table
	//is still not possible. Looks like we need to set the selection again...
	if (!m_changeSelectionFromView)
		setFocus();
	else
		m_changeSelectionFromView = false;
}

/*!
 * Used to udpate the cursor Dock
 */
void ProjectExplorer::updateSelectedAspects() {
	const auto& items = m_treeView->selectionModel()->selectedRows();
	QList<AbstractAspect*> selectedAspects;
	for (const QModelIndex& index : items)
		selectedAspects << static_cast<AbstractAspect*>(index.internalPointer());

	emit selectedAspectsChanged(selectedAspects);
}

void ProjectExplorer::expandSelected() {
	const auto& items = m_treeView->selectionModel()->selectedIndexes();
	for (const auto& index : items)
		m_treeView->setExpanded(index, true);
}

void ProjectExplorer::collapseSelected() {
	const auto& items = m_treeView->selectionModel()->selectedIndexes();
	for (const auto& index : items)
		m_treeView->setExpanded(index, false);
}

void ProjectExplorer::deleteSelected() {
	const auto& items = m_treeView->selectionModel()->selectedIndexes();
	if (!items.size())
		return;

	//determine all selected aspect
	QVector<AbstractAspect*> aspects;
	for (int i = 0; i < items.size()/4; ++i) {
		const QModelIndex& index = items.at(i*4);
		auto* aspect = static_cast<AbstractAspect*>(index.internalPointer());
		aspects << aspect;
	}

	QString msg;
	if (aspects.size() > 1)
		msg = i18n("Do you really want to delete the selected %1 objects?", aspects.size());
	else
		 msg = i18n("Do you really want to delete %1?", aspects.constFirst()->name());

	int rc = KMessageBox::warningYesNo(this, msg, i18np("Delete selected object", "Delete selected objects", aspects.size()));

	if (rc == KMessageBox::No)
		return;

	m_project->beginMacro(i18np("Project Explorer: delete %1 selected object", "Project Explorer: delete %1 selected objects", items.size()/4));

	//determine aspects to be deleted:
	//it's enough to delete parent items in the selection only,
	//skip all selected aspects where one of the parents is also in the selection
	//for example selected columns of a selected spreadsheet, etc.
	QVector<AbstractAspect*> aspectsToDelete;
	for (auto* aspect : aspects) {
		auto* parent = aspect->parentAspect();
		int parentSelected = false;
		while (parent) {
			if (aspects.indexOf(parent) != -1) {
				parentSelected = true;
				break;
			}

			parent = parent->parentAspect();
		}

		if (!parentSelected)
			aspectsToDelete << aspect; //parent is not in the selection
	}

	for (auto* aspect : aspectsToDelete)
		aspect->remove();

	m_project->endMacro();
}

//##############################################################################
//##################  Serialization/Deserialization  ###########################
//##############################################################################
struct ViewState {
	Qt::WindowStates state;
	QRect geometry;
};

/**
 * \brief Save the current state of the tree view
 * (expanded items and the currently selected item) as XML
 */
void ProjectExplorer::save(QXmlStreamWriter* writer) const {
	const auto* model = static_cast<AspectTreeModel*>(m_treeView->model());
	const auto& selectedRows = m_treeView->selectionModel()->selectedRows();

	writer->writeStartElement("state");

	//check whether the project node itself is expanded
	if (m_treeView->isExpanded(m_treeView->model()->index(0,0))) {
		writer->writeStartElement("expanded");
		writer->writeAttribute("path", m_project->path());
		writer->writeEndElement();
	}

	const auto& children = m_project->children<AbstractAspect>(AbstractAspect::ChildIndexFlag::Recursive);
	for (const auto* aspect : children) {
		const QString& path = aspect->path();
		const auto* part = dynamic_cast<const AbstractPart*>(aspect);

		if (part && part->hasMdiSubWindow()) {
			writer->writeStartElement("view");
			const auto& geometry = part->mdiSubWindow()->geometry();
			writer->writeAttribute("path", path);
			writer->writeAttribute("state", QString::number(part->view()->windowState()) );
			writer->writeAttribute("x", QString::number(geometry.x()) );
			writer->writeAttribute("y", QString::number(geometry.y()) );
			writer->writeAttribute("width", QString::number(geometry.width()) );
			writer->writeAttribute("height", QString::number(geometry.height()) );
			writer->writeEndElement();
		}

		const auto& index = model->modelIndexOfAspect(aspect);
		if (model->rowCount(index)>0 && m_treeView->isExpanded(index)) {
			writer->writeStartElement("expanded");
			writer->writeAttribute("path", path);
			writer->writeEndElement();
		}

		if (selectedRows.indexOf(index) != -1) {
			writer->writeStartElement("selected");
			writer->writeAttribute("path", path);
			writer->writeEndElement();
		}

		if (index == m_treeView->currentIndex()) {
			writer->writeStartElement("current");
			writer->writeAttribute("path", path);
			writer->writeEndElement();
		}
	}

	writer->writeEndElement(); //"state"
}

/**
 * \brief Load from XML
 */
bool ProjectExplorer::load(XmlStreamReader* reader) {
	const auto* model = static_cast<AspectTreeModel*>(m_treeView->model());
	QList<QModelIndex> selected;
	QList<QModelIndex> expanded;
	QModelIndex currentIndex;
	QString str;
	KLocalizedString attributeWarning = ki18n("Attribute '%1' missing or empty, default value is used");

	//xmlVersion < 3: old logic where the "rows" for the selected and expanded items were saved
	//ignoring the sub-folder structure. Remove it later.
	if (Project::xmlVersion() < 3) {

	const auto& aspects = m_project->children<AbstractAspect>(AbstractAspect::ChildIndexFlag::Recursive);
	bool expandedItem = false;
	bool selectedItem = false;
	bool viewItem = false;
	(void)viewItem; // because of a strange g++-warning about unused viewItem
	bool currentItem = false;
	int row;
	QXmlStreamAttributes attribs;

	while (!reader->atEnd()) {
		reader->readNext();
		if (reader->isEndElement() && reader->name() == "state")
			break;

		if (!reader->isStartElement())
			continue;

		if (reader->name() == "expanded") {
			expandedItem = true;
			selectedItem = false;
			viewItem = false;
			currentItem = false;
		} else if (reader->name() == "selected") {
			expandedItem = false;
			selectedItem = true;
			viewItem = false;
			currentItem = false;
		} else 	if (reader->name() == "view") {
			expandedItem = false;
			selectedItem = false;
			viewItem = true;
			currentItem = false;
		} else 	if (reader->name() == "current") {
			expandedItem = false;
			selectedItem = false;
			viewItem = false;
			currentItem = true;
		} else if (reader->name() == "row") {
			//we need to read the attributes first and before readElementText() otherwise they are empty
			attribs = reader->attributes();
			row = reader->readElementText().toInt();

			QModelIndex index;
			if (row == -1)
				index = model->modelIndexOfAspect(m_project); //-1 corresponds to the project-item (s.a. ProjectExplorer::save())
			else if (row >= aspects.size() || row < 0) //checking for <0 to protect against wrong values in the XML
				continue;
			else
				index = model->modelIndexOfAspect(aspects.at(row));

			if (expandedItem)
				expanded.push_back(index);
			else if (selectedItem)
				selected.push_back(index);
			else if (currentItem)
				currentIndex = index;
			else if (viewItem) {
				if (row < 0) //should never happen, but we need to handle the corrupted file
					continue;

				auto* part = dynamic_cast<AbstractPart*>(aspects.at(row));
				if (!part)
					continue; //TODO: add error/warning message here?

				emit activateView(part); //request to show the view in MainWin

				str = attribs.value("state").toString();
				if (str.isEmpty())
					reader->raiseWarning(attributeWarning.subs("state").toString());
				else {
					part->view()->setWindowState(Qt::WindowStates(str.toInt()));
					part->mdiSubWindow()->setWindowState(Qt::WindowStates(str.toInt()));
				}

				if (str != "0")
					continue; //no geometry settings required for maximized/minimized windows

				QRect geometry;
				str = attribs.value("x").toString();
				if (str.isEmpty())
					reader->raiseWarning(attributeWarning.subs("x").toString());
				else
					geometry.setX(str.toInt());

				str = attribs.value("y").toString();
				if (str.isEmpty())
					reader->raiseWarning(attributeWarning.subs("y").toString());
				else
					geometry.setY(str.toInt());

				str = attribs.value("width").toString();
				if (str.isEmpty())
					reader->raiseWarning(attributeWarning.subs("width").toString());
				else
					geometry.setWidth(str.toInt());

				str = attribs.value("height").toString();
				if (str.isEmpty())
					reader->raiseWarning(attributeWarning.subs("height").toString());
				else
					geometry.setHeight(str.toInt());

				part->mdiSubWindow()->setGeometry(geometry);
			}
		}
	}

	} else {
		while (!reader->atEnd()) {
			reader->readNext();
			if (reader->isEndElement() && reader->name() == "state")
				break;

			if (!reader->isStartElement())
				continue;

			const auto& attribs = reader->attributes();
			const auto& path = attribs.value("path").toString();
			const auto& index = model->modelIndexOfAspect(path);

			if (reader->name() == "expanded")
				expanded << index;
			else if (reader->name() == "selected")
				selected << index;
			else if (reader->name() == "current")
				currentIndex = index;
			else if (reader->name() == "view") {
				auto* aspect = static_cast<AbstractAspect*>(index.internalPointer());
				auto* part = dynamic_cast<AbstractPart*>(aspect);
				if (!part)
					continue; //TODO: add error/warning message here?

				emit activateView(part); //request to show the view in MainWin

				str = attribs.value("state").toString();
				if (str.isEmpty())
					reader->raiseWarning(attributeWarning.subs("state").toString());
				else {
					part->view()->setWindowState(Qt::WindowStates(str.toInt()));
					part->mdiSubWindow()->setWindowState(Qt::WindowStates(str.toInt()));
				}

				if (str != "0")
					continue; //no geometry settings required for maximized/minimized windows

				QRect geometry;
				str = attribs.value("x").toString();
				if (str.isEmpty())
					reader->raiseWarning(attributeWarning.subs("x").toString());
				else
					geometry.setX(str.toInt());

				str = attribs.value("y").toString();
				if (str.isEmpty())
					reader->raiseWarning(attributeWarning.subs("y").toString());
				else
					geometry.setY(str.toInt());

				str = attribs.value("width").toString();
				if (str.isEmpty())
					reader->raiseWarning(attributeWarning.subs("width").toString());
				else
					geometry.setWidth(str.toInt());

				str = attribs.value("height").toString();
				if (str.isEmpty())
					reader->raiseWarning(attributeWarning.subs("height").toString());
				else
					geometry.setHeight(str.toInt());

				part->mdiSubWindow()->setGeometry(geometry);
			}
		}
	}

	for (const auto& index : expanded) {
		m_treeView->setExpanded(index, true);
		collapseParents(index, expanded);//collapse all parent indices if they are not expanded
	}

	for (const auto& index : selected)
		m_treeView->selectionModel()->select(index, QItemSelectionModel::Select | QItemSelectionModel::Rows);

	m_treeView->setCurrentIndex(currentIndex);
	m_treeView->scrollTo(currentIndex);
	auto* aspect = static_cast<AbstractAspect*>(currentIndex.internalPointer());
	emit currentAspectChanged(aspect);

	//when setting the current index above it gets expanded, collapse all parent indices if they are were not expanded when saved
	collapseParents(currentIndex, expanded);

	return true;
}

void ProjectExplorer::collapseParents(const QModelIndex& index, const QList<QModelIndex>& expanded) {
	//root index doesn't have any parents - this case is not caught by the second if-statement below
	if (index.column() == 0 && index.row() == 0)
		return;

	const auto& parent = index.parent();
	if (parent == QModelIndex())
		return;

	if (expanded.indexOf(parent) == -1)
		m_treeView->collapse(parent);
}
