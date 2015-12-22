/***************************************************************************
	File       		: AspectTreeModel.h
    Project         : LabPlot
    Description     : Represents a tree of AbstractAspect objects as a Qt item model.
    --------------------------------------------------------------------
	Copyright            : (C) 2007-2009 by Knut Franke (knut.franke@gmx.de)
    Copyright            : (C) 2007-2009 by Tilman Benkert (thzs@gmx.net)
	Copyright            : (C) 2011-2014 Alexander Semke (alexander.semke@web.de)

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
#include "backend/core/AbstractAspect.h"
#include "backend/worksheet/WorksheetElement.h"
#include "backend/core/AspectTreeModel.h"

#include <QDateTime>
#include <QIcon>
#include <QMenu>
#include <QApplication>
#include <QFontMetrics>

#include <KLocale>

/**
 * \class AspectTreeModel
 * \brief Represents a tree of AbstractAspect objects as a Qt item model.
 *
 * This class is an adapter between an AbstractAspect hierarchy and Qt's view classes.
 *
 * It represents children of an Aspect as rows in the model, with the fixed columns
 * Name (AbstractAspect::name()), Type (the class name), Created (AbstractAspect::creationTime())
 * and Comment (AbstractAspect::comment()). Name is decorated using AbstractAspect::icon().
 * The tooltip for all columns is generated from AbstractAspect::caption().
 *
 * Name and Comment are editable.
 *
 * For views which support this (currently ProjectExplorer), the menu created by
 * AbstractAspect::createContextMenu() is made available via the custom role ContextMenuRole.
 */

/**
 * \enum AspectTreeModel::CustomDataRole
 * \brief Custom data roles used in addition to Qt::ItemDataRole
 */
/**
 * \var AspectTreeModel::ContextMenuRole
 * \brief pointer to a new context menu for an Aspect
 */

/**
 * \fn QModelIndex AspectTreeModel::modelIndexOfAspect(const AbstractAspect *aspect, int column=0) const
 * \brief Convenience wrapper around QAbstractItemModel::createIndex().
 */

AspectTreeModel::AspectTreeModel(AbstractAspect* root, QObject* parent)
	: QAbstractItemModel(parent),
	  m_root(root),
	  m_folderSelectable(true),
	  m_filterCaseSensitivity(Qt::CaseInsensitive),
	  m_matchCompleteWord(false) {

	QFont font;
	font.setFamily(font.defaultFamily());
	QFontMetrics fm(font);
	m_defaultHeaderHeight = fm.height();

	connect(m_root, SIGNAL(aspectDescriptionChanged(const AbstractAspect*)),
		this, SLOT(aspectDescriptionChanged(const AbstractAspect*)));
	connect(m_root, SIGNAL(aspectAboutToBeAdded(const AbstractAspect*,const AbstractAspect*,const AbstractAspect*)),
		this, SLOT(aspectAboutToBeAdded(const AbstractAspect*,const AbstractAspect*,const AbstractAspect*)));
	connect(m_root, SIGNAL(aspectAboutToBeRemoved(const AbstractAspect*)),
		this, SLOT(aspectAboutToBeRemoved(const AbstractAspect*)));
	connect(m_root, SIGNAL(aspectAdded(const AbstractAspect*)),
		this, SLOT(aspectAdded(const AbstractAspect*)));
	connect(m_root, SIGNAL(aspectRemoved(const AbstractAspect*,const AbstractAspect*,const AbstractAspect*)),
		this, SLOT(aspectRemoved()));
	connect(m_root, SIGNAL(aspectHiddenAboutToChange(const AbstractAspect*)),
		this, SLOT(aspectHiddenAboutToChange(const AbstractAspect*)));
	connect(m_root, SIGNAL(aspectHiddenChanged(const AbstractAspect*)),
		this, SLOT(aspectHiddenChanged(const AbstractAspect*)));
}

/*!
  \c list contains the class names of the aspects, that can be selected in the corresponding model view.
*/
void AspectTreeModel::setSelectableAspects(QList<const char*> list){
	m_selectableAspects=list;
}

QModelIndex AspectTreeModel::index(int row, int column, const QModelIndex &parent) const{
	if (!hasIndex(row, column, parent))
	  return QModelIndex();

	if(!parent.isValid()){
		if(row != 0) return QModelIndex();
		return createIndex(row, column, m_root);
	}

	AbstractAspect *parent_aspect = static_cast<AbstractAspect*>(parent.internalPointer());
	AbstractAspect *child_aspect = parent_aspect->child<AbstractAspect>(row);
	if (!child_aspect) return QModelIndex();
	return createIndex(row, column, child_aspect);
}

QModelIndex AspectTreeModel::parent(const QModelIndex &index) const{
	if (!index.isValid())
	  return QModelIndex();

	AbstractAspect *parent_aspect = static_cast<AbstractAspect*>(index.internalPointer())->parentAspect();
	if (!parent_aspect) return QModelIndex();
	  return modelIndexOfAspect(parent_aspect);
}

int AspectTreeModel::rowCount(const QModelIndex &parent) const{
	if (!parent.isValid())
	  return 1;

	AbstractAspect *parent_aspect =  static_cast<AbstractAspect*>(parent.internalPointer());
	return parent_aspect->childCount<AbstractAspect>();
}

int AspectTreeModel::columnCount(const QModelIndex &parent) const{
	Q_UNUSED(parent);
	return 4;
}

QVariant AspectTreeModel::headerData(int section, Qt::Orientation orientation, int role) const{
	if(orientation != Qt::Horizontal)
	  return QVariant();

	switch(role) {
		case Qt::DisplayRole:
			switch(section) {
				case 0: return i18n("Name");
				case 1: return i18n("Type");
				case 2: return i18n("Created");
				case 3: return i18n("Comment");
				default: return QVariant();
			}
		case Qt::SizeHintRole: {
			switch(section) {
				case 0: return QSize(300, m_defaultHeaderHeight);
				case 1: return QSize(80, m_defaultHeaderHeight);
				case 2: return QSize(160, m_defaultHeaderHeight);
				case 3: return QSize(400, m_defaultHeaderHeight);
				default: return QVariant();
			}
		}
		default:
			return QVariant();
	}
}

QVariant AspectTreeModel::data(const QModelIndex &index, int role) const{
	if (!index.isValid())
	  return QVariant();

	AbstractAspect *aspect = static_cast<AbstractAspect*>(index.internalPointer());
	switch(role) {
		case Qt::DisplayRole:
		case Qt::EditRole:
			switch(index.column()) {
				case 0: return aspect->name();
				case 1: return aspect->metaObject()->className();
				case 2: return aspect->creationTime().toString();
				case 3: return aspect->comment().replace('\n', ' ').simplified();
				default: return QVariant();
			}
		case Qt::ToolTipRole:
			if (aspect->comment().isEmpty())
				return aspect->name();
			else
				return aspect->name() + ", " + aspect->comment();
		case Qt::DecorationRole:
			return index.column() == 0 ? aspect->icon() : QIcon();
		case ContextMenuRole:
			return QVariant::fromValue(static_cast<QWidget*>(aspect->createContextMenu()));
		case Qt::ForegroundRole:{
			const WorksheetElement* we = qobject_cast<WorksheetElement*>(aspect);
			if (we){
				if (!we->isVisible())
					return QVariant(  QApplication::palette().color(QPalette::Disabled,QPalette::Text ) );
			}
			return QVariant( QApplication::palette().color(QPalette::Active,QPalette::Text ) );
		}
		default:
			return QVariant();
	}
}

Qt::ItemFlags AspectTreeModel::flags(const QModelIndex &index) const{
	if (!index.isValid())
	  return 0;

 	Qt::ItemFlags result;
	AbstractAspect *aspect = static_cast<AbstractAspect*>(index.internalPointer());

	if (m_selectableAspects.size() != 0){
		foreach(const char * classString, m_selectableAspects){
			if (aspect->inherits(classString)){
			  result = Qt::ItemIsEnabled | Qt::ItemIsSelectable;
			  break;
			}else{
				result &= ~Qt::ItemIsEnabled;
			}
		}
	}else{
	  //default case: the list for the selectable aspects is empty and all aspects are selectable.
	  // Apply filter, if available. Indices, that don't match the filter are not selectable.
	  //Don't apply any filter to the very first index in the model  - this top index corresponds to the project item.
	  if ( index!=this->index(0,0,QModelIndex()) &&  !m_filterString.isEmpty() ) {
		  if (this->containsFilterString(aspect))
			  result = Qt::ItemIsEnabled | Qt::ItemIsSelectable;
		  else
			  result = Qt::ItemIsSelectable;
	  }else{
		  result = Qt::ItemIsEnabled | Qt::ItemIsSelectable;
	  }
	}

	 //the columns "name" and "description" are editable
	if (index.column() == 0 || index.column() == 3)
		result |= Qt::ItemIsEditable;

	return result;
}

void AspectTreeModel::aspectDescriptionChanged(const AbstractAspect *aspect){
	emit dataChanged(modelIndexOfAspect(aspect), modelIndexOfAspect(aspect, 3));
}

void AspectTreeModel::aspectAboutToBeAdded(const AbstractAspect *parent, const AbstractAspect *before, const AbstractAspect *child){
    Q_UNUSED(child);
        int index = parent->indexOfChild<AbstractAspect>(before);
	if (index == -1)
	  index = parent->childCount<AbstractAspect>();

	beginInsertRows(modelIndexOfAspect(parent), index, index);
}

void AspectTreeModel::aspectAdded(const AbstractAspect *aspect){
	endInsertRows();
	AbstractAspect * parent = aspect->parentAspect();
	emit dataChanged(modelIndexOfAspect(parent), modelIndexOfAspect(parent, 3));

	connect(aspect, SIGNAL(renameRequested()), this, SLOT(renameRequested()));
	foreach(const AbstractAspect* child, aspect->children<AbstractAspect>())
		connect(child, SIGNAL(renameRequested()), this, SLOT(renameRequested()));

	connect(aspect, SIGNAL(childAspectSelectedInView(const AbstractAspect*)), this, SLOT(aspectSelectedInView(const AbstractAspect*)));
	connect(aspect, SIGNAL(childAspectDeselectedInView(const AbstractAspect*)), this, SLOT(aspectDeselectedInView(const AbstractAspect*)));
}

void AspectTreeModel::aspectAboutToBeRemoved(const AbstractAspect *aspect){
	AbstractAspect * parent = aspect->parentAspect();
	int index = parent->indexOfChild<AbstractAspect>(aspect);
	beginRemoveRows(modelIndexOfAspect(parent), index, index);
}

void AspectTreeModel::aspectRemoved(){
	endRemoveRows();
}

void AspectTreeModel::aspectHiddenAboutToChange(const AbstractAspect * aspect){
	for (AbstractAspect * i = aspect->parentAspect(); i; i = i->parentAspect())
		if (i->hidden())
			return;
	if (aspect->hidden())
		aspectAboutToBeAdded(aspect->parentAspect(), aspect, aspect);
	else
		aspectAboutToBeRemoved(aspect);
}

void AspectTreeModel::aspectHiddenChanged(const AbstractAspect *aspect){
	for (AbstractAspect * i = aspect->parentAspect(); i; i = i->parentAspect())
		if (i->hidden())
			return;
	if (aspect->hidden())
		aspectRemoved();
	else
		aspectAdded(aspect);
}

bool AspectTreeModel::setData(const QModelIndex &index, const QVariant &value, int role){
	if (!index.isValid() || role != Qt::EditRole) return false;
	AbstractAspect *aspect = static_cast<AbstractAspect*>(index.internalPointer());
	switch (index.column()) {
		case 0:
			aspect->setName(value.toString());
			break;
		case 3:
			aspect->setComment(value.toString());
			break;
		default:
			return false;
	}
	emit dataChanged(index, index);
	return true;
}

QModelIndex AspectTreeModel::modelIndexOfAspect(const AbstractAspect *aspect, int column) const{
  AbstractAspect * parent = aspect->parentAspect();
  return createIndex(parent ? parent->indexOfChild<AbstractAspect>(aspect) : 0,
					  column, const_cast<AbstractAspect*>(aspect));
}

void AspectTreeModel::setFilterString(const QString & s){
    m_filterString=s;

	QModelIndex  topLeft = this->index(0,0, QModelIndex());
	QModelIndex  bottomRight =  this->index(this->rowCount()-1,3, QModelIndex());
	emit dataChanged(topLeft, bottomRight);
}

void AspectTreeModel::setFilterCaseSensitivity(Qt::CaseSensitivity cs){
    m_filterCaseSensitivity = cs;
}

void AspectTreeModel::setFilterMatchCompleteWord(bool b){
	m_matchCompleteWord = b;
}

bool AspectTreeModel::containsFilterString(const AbstractAspect* aspect) const{
	if (m_matchCompleteWord){
		if (aspect->name().compare(m_filterString, m_filterCaseSensitivity) == 0)
			return true;
	}else{
		if (aspect->name().contains(m_filterString, m_filterCaseSensitivity))
			return true;
	}

	//check for the occurrence of the filter string in the names of the parents
	if ( aspect->parentAspect() )
		return this->containsFilterString(aspect->parentAspect());
	else
		return false;

	//TODO make this optional
	// 	//check for the occurrence of the filter string in the names of the children
// 	foreach(const AbstractAspect * child, aspect->children<AbstractAspect>()){
// 	  if ( this->containsFilterString(child) )
// 		return true;
// 	}
}

//##############################################################################
//#################################  SLOTS  ####################################
//##############################################################################
void AspectTreeModel::renameRequested() {
	AbstractAspect* aspect = qobject_cast<AbstractAspect*>(QObject::sender());
	if (aspect)
		emit renameRequested(modelIndexOfAspect(aspect));
}

void AspectTreeModel::aspectSelectedInView(const AbstractAspect* aspect){
	if (aspect->hidden()){
		//a hidden aspect was selected in the view (e.g. plot title in WorksheetView)
		//select the parent aspect first, if available
		AbstractAspect* parent = aspect->parentAspect();
		if (parent){
			emit indexSelected(modelIndexOfAspect(parent));
		}

		//emit also this signal, so the GUI can handle this selection.
		emit hiddenAspectSelected(aspect);
	}else{
		emit indexSelected(modelIndexOfAspect(aspect));
	}

	//deselect the root item when one of the children was selected in the view
	//in order to avoid multiple selection with the project item (if selected) in the project explorer
	emit indexDeselected(modelIndexOfAspect(m_root));
}

void AspectTreeModel::aspectDeselectedInView(const AbstractAspect* aspect){
	if (aspect->hidden()){
		AbstractAspect* parent = aspect->parentAspect();
		if (parent)
			emit indexDeselected(modelIndexOfAspect(parent));
	}else{
		emit indexDeselected(modelIndexOfAspect(aspect));
	}
}
