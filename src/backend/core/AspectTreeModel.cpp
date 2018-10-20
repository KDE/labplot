/***************************************************************************
	File       		: AspectTreeModel.h
    Project         : LabPlot
    Description     : Represents a tree of AbstractAspect objects as a Qt item model.
    --------------------------------------------------------------------
	Copyright            : (C) 2007-2009 by Knut Franke (knut.franke@gmx.de)
    Copyright            : (C) 2007-2009 by Tilman Benkert (thzs@gmx.net)
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
#include "backend/core/AbstractAspect.h"
#include "backend/core/column/Column.h"
#include "backend/worksheet/WorksheetElement.h"
#include "backend/core/AspectTreeModel.h"

#include <QDateTime>
#include <QIcon>
#include <QMenu>
#include <QApplication>
#include <QFontMetrics>

#include <KLocalizedString>

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
	  m_readOnly(false),
	  m_folderSelectable(true),
	  m_plottableColumnsOnly(false),
	  m_numericColumnsOnly(false),
	  m_nonEmptyNumericColumnsOnly(false),
	  m_showPlotDesignation(false),
	  m_filterCaseSensitivity(Qt::CaseInsensitive),
	  m_matchCompleteWord(false) {

	connect(m_root, &AbstractAspect::aspectDescriptionChanged, this, &AspectTreeModel::aspectDescriptionChanged);
	connect(m_root, &AbstractAspect::aspectAboutToBeAdded, this, &AspectTreeModel::aspectAboutToBeAdded);
	connect(m_root, &AbstractAspect::aspectAboutToBeRemoved, this, &AspectTreeModel::aspectAboutToBeRemoved);
	connect(m_root, &AbstractAspect::aspectAdded, this, &AspectTreeModel::aspectAdded);
	connect(m_root, &AbstractAspect::aspectRemoved, this, &AspectTreeModel::aspectRemoved);
	connect(m_root, &AbstractAspect::aspectHiddenAboutToChange, this, &AspectTreeModel::aspectHiddenAboutToChange);
	connect(m_root, &AbstractAspect::aspectHiddenChanged, this, &AspectTreeModel::aspectHiddenChanged);
}

/*!
  \c list contains the class names of the aspects, that can be selected in the corresponding model view.
*/
void AspectTreeModel::setSelectableAspects(QList<const char*> list) {
	m_selectableAspects=list;
}

void AspectTreeModel::setReadOnly(bool readOnly) {
	m_readOnly = readOnly;
}

void AspectTreeModel::enablePlottableColumnsOnly(bool value) {
	m_plottableColumnsOnly = value;
}

void AspectTreeModel::enableNumericColumnsOnly(bool value) {
	m_numericColumnsOnly = value;
}

void AspectTreeModel::enableNonEmptyNumericColumnsOnly(bool value) {
	m_nonEmptyNumericColumnsOnly = value;
}

void AspectTreeModel::enableShowPlotDesignation(bool value) {
	m_showPlotDesignation = value;
}

QModelIndex AspectTreeModel::index(int row, int column, const QModelIndex &parent) const {
	if (!hasIndex(row, column, parent))
		return QModelIndex{};

	if(!parent.isValid()) {
		if(row != 0)
			return QModelIndex{};
		return createIndex(row, column, m_root);
	}

	AbstractAspect *parent_aspect = static_cast<AbstractAspect*>(parent.internalPointer());
	AbstractAspect *child_aspect = parent_aspect->child<AbstractAspect>(row);
	if (!child_aspect)
		return QModelIndex{};
	return createIndex(row, column, child_aspect);
}

QModelIndex AspectTreeModel::parent(const QModelIndex &index) const {
	if (!index.isValid())
		return QModelIndex{};

	AbstractAspect *parent_aspect = static_cast<AbstractAspect*>(index.internalPointer())->parentAspect();
	if (!parent_aspect)
		return QModelIndex{};
	return modelIndexOfAspect(parent_aspect);
}

int AspectTreeModel::rowCount(const QModelIndex &parent) const {
	if (!parent.isValid())
		return 1;

	AbstractAspect *parent_aspect =  static_cast<AbstractAspect*>(parent.internalPointer());
	return parent_aspect->childCount<AbstractAspect>();
}

int AspectTreeModel::columnCount(const QModelIndex &parent) const {
	Q_UNUSED(parent);
	return 4;
}

QVariant AspectTreeModel::headerData(int section, Qt::Orientation orientation, int role) const {
	if(orientation != Qt::Horizontal)
		return QVariant();

	switch(role) {
	case Qt::DisplayRole:
		switch(section) {
		case 0:
			return i18n("Name");
		case 1:
			return i18n("Type");
		case 2:
			return i18n("Created");
		case 3:
			return i18n("Comment");
		default:
			return QVariant();
		}
	default:
		return QVariant();
	}
}

QVariant AspectTreeModel::data(const QModelIndex &index, int role) const {
	if (!index.isValid())
		return QVariant();

	AbstractAspect* aspect = static_cast<AbstractAspect*>(index.internalPointer());
	switch(role) {
	case Qt::DisplayRole:
	case Qt::EditRole:
		switch(index.column()) {
		case 0: {
			const Column* column = dynamic_cast<const Column*>(aspect);
			if (column) {
				QString name = aspect->name();
				if (m_plottableColumnsOnly && !column->isPlottable())
					name = i18n("%1   (non-plottable data)", name);
				else if (m_numericColumnsOnly && !column->isNumeric())
					name = i18n("%1   (non-numeric data)", name);
				else if (m_nonEmptyNumericColumnsOnly && !column->hasValues())
					name = i18n("%1   (no values)", name);

				if (m_showPlotDesignation) {
					QString designation;
					switch(column->plotDesignation()) {
						case AbstractColumn::NoDesignation:
							break;
						case AbstractColumn::X:
							designation = QLatin1String(" [X]");
							break;
						case AbstractColumn::Y:
							designation = QLatin1String(" [Y]");
							break;
						case AbstractColumn::Z:
							designation = QLatin1String(" [Z]");
							break;
						case AbstractColumn::XError:
							designation = QLatin1String(" [") + i18n("X-error") + QLatin1Char(']');
							break;
						case AbstractColumn::XErrorPlus:
							designation = QLatin1String(" [") + i18n("X-error +") + QLatin1Char(']');
							break;
						case AbstractColumn::XErrorMinus:
							designation = QLatin1String(" [") + i18n("X-error -") + QLatin1Char(']');
							break;
						case AbstractColumn::YError:
							designation = QLatin1String(" [") + i18n("Y-error") + QLatin1Char(']');
							break;
						case AbstractColumn::YErrorPlus:
							designation = QLatin1String(" [") + i18n("Y-error +") + QLatin1Char(']');
							break;
						case AbstractColumn::YErrorMinus:
							designation = QLatin1String(" [") + i18n("Y-error -") + QLatin1Char(']');
							break;
					}
					name += QLatin1Char('\t') + designation;
				}

				return name;
			} else
				return aspect->name();
		}
		case 1:
			if (aspect->metaObject()->className() != QLatin1String("CantorWorksheet"))
				return aspect->metaObject()->className();
			else
				return QLatin1String("CAS Worksheet");
		case 2:
			return aspect->creationTime().toString();
		case 3:
			return aspect->comment().replace('\n', ' ').simplified();
		default:
			return QVariant();
		}
	case Qt::ToolTipRole:
		if (aspect->comment().isEmpty())
			return QLatin1String("<b>") + aspect->name() + QLatin1String("</b>");
		else
			return QLatin1String("<b>") + aspect->name() + QLatin1String("</b><br><br>") + aspect->comment().replace(QLatin1Char('\n'), QLatin1String("<br>"));
	case Qt::DecorationRole:
		return index.column() == 0 ? aspect->icon() : QIcon();
	case Qt::ForegroundRole: {
			const WorksheetElement* we = qobject_cast<WorksheetElement*>(aspect);
			if (we) {
				if (!we->isVisible())
					return QVariant(  QApplication::palette().color(QPalette::Disabled,QPalette::Text ) );
			}
			return QVariant( QApplication::palette().color(QPalette::Active,QPalette::Text ) );
		}
	default:
		return QVariant();
	}
}

Qt::ItemFlags AspectTreeModel::flags(const QModelIndex &index) const {
	if (!index.isValid())
		return nullptr;

	Qt::ItemFlags result;
	AbstractAspect *aspect = static_cast<AbstractAspect*>(index.internalPointer());

	if (!m_selectableAspects.isEmpty()) {
		foreach(const char * classString, m_selectableAspects) {
			if (aspect->inherits(classString)) {
				result = Qt::ItemIsEnabled | Qt::ItemIsSelectable;
				if( index!=this->index(0,0,QModelIndex()) &&  !m_filterString.isEmpty() ) {
					if (this->containsFilterString(aspect))
						result = Qt::ItemIsEnabled | Qt::ItemIsSelectable;
					else
						result &= ~Qt::ItemIsEnabled;
				}
				break;
			} else
				result &= ~Qt::ItemIsEnabled;
		}
	} else {
		//default case: the list for the selectable aspects is empty and all aspects are selectable.
		// Apply filter, if available. Indices, that don't match the filter are not selectable.
		//Don't apply any filter to the very first index in the model  - this top index corresponds to the project item.
		if ( index!=this->index(0,0,QModelIndex()) &&  !m_filterString.isEmpty() ) {
			if (this->containsFilterString(aspect))
				result = Qt::ItemIsEnabled | Qt::ItemIsSelectable;
			else
				result = Qt::ItemIsSelectable;
		} else
			result = Qt::ItemIsEnabled | Qt::ItemIsSelectable;
	}

	//the columns "name" and "description" are editable
	if (!m_readOnly) {
		if (index.column() == 0 || index.column() == 3)
			result |= Qt::ItemIsEditable;
	}

	const Column* column = dynamic_cast<const Column*>(aspect);
	if (column) {
		//allow to drag and drop columns for the faster creation of curves in the plots.
		//TODO: allow drag&drop later for other objects too, once we implement copy and paste in the project explorer
		result = result |Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled;

		if (m_plottableColumnsOnly && !column->isPlottable())
			result &= ~Qt::ItemIsEnabled;

		if (m_numericColumnsOnly && !column->isNumeric())
			result &= ~Qt::ItemIsEnabled;

		if (m_nonEmptyNumericColumnsOnly && !(column->isNumeric() && column->hasValues()))
			result &= ~Qt::ItemIsEnabled;
	}

	return result;
}

void AspectTreeModel::aspectDescriptionChanged(const AbstractAspect *aspect) {
	emit dataChanged(modelIndexOfAspect(aspect), modelIndexOfAspect(aspect, 3));
}

void AspectTreeModel::aspectAboutToBeAdded(const AbstractAspect *parent, const AbstractAspect *before, const AbstractAspect *child) {
	Q_UNUSED(child);
	int index = parent->indexOfChild<AbstractAspect>(before);
	if (index == -1)
		index = parent->childCount<AbstractAspect>();

	beginInsertRows(modelIndexOfAspect(parent), index, index);
}

void AspectTreeModel::aspectAdded(const AbstractAspect *aspect) {
	endInsertRows();
	AbstractAspect * parent = aspect->parentAspect();
	emit dataChanged(modelIndexOfAspect(parent), modelIndexOfAspect(parent, 3));

	connect(aspect, &AbstractAspect::renameRequested, this, &AspectTreeModel::renameRequestedSlot);
	connect(aspect, &AbstractAspect::childAspectSelectedInView, this, &AspectTreeModel::aspectSelectedInView);
	connect(aspect, &AbstractAspect::childAspectDeselectedInView, this, &AspectTreeModel::aspectDeselectedInView);

	//add signal-slot connects for all children, too
	for (const auto* child : aspect->children<AbstractAspect>(AbstractAspect::Recursive)) {
		connect(child, &AbstractAspect::renameRequested, this, &AspectTreeModel::renameRequestedSlot);
		connect(child, &AbstractAspect::childAspectSelectedInView, this, &AspectTreeModel::aspectSelectedInView);
		connect(child, &AbstractAspect::childAspectDeselectedInView, this, &AspectTreeModel::aspectDeselectedInView);
	}
}

void AspectTreeModel::aspectAboutToBeRemoved(const AbstractAspect *aspect) {
	AbstractAspect * parent = aspect->parentAspect();
	int index = parent->indexOfChild<AbstractAspect>(aspect);
	beginRemoveRows(modelIndexOfAspect(parent), index, index);
}

void AspectTreeModel::aspectRemoved() {
	endRemoveRows();
}

void AspectTreeModel::aspectHiddenAboutToChange(const AbstractAspect * aspect) {
	for (AbstractAspect * i = aspect->parentAspect(); i; i = i->parentAspect())
		if (i->hidden())
			return;
	if (aspect->hidden())
		aspectAboutToBeAdded(aspect->parentAspect(), aspect, aspect);
	else
		aspectAboutToBeRemoved(aspect);
}

void AspectTreeModel::aspectHiddenChanged(const AbstractAspect *aspect) {
	for (AbstractAspect * i = aspect->parentAspect(); i; i = i->parentAspect())
		if (i->hidden())
			return;
	if (aspect->hidden())
		aspectRemoved();
	else
		aspectAdded(aspect);
}

bool AspectTreeModel::setData(const QModelIndex &index, const QVariant &value, int role) {
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

QModelIndex AspectTreeModel::modelIndexOfAspect(const AbstractAspect* aspect, int column) const {
	AbstractAspect* parent = aspect->parentAspect();
	return createIndex(parent ? parent->indexOfChild<AbstractAspect>(aspect) : 0,
	                   column, const_cast<AbstractAspect*>(aspect));
}

/*!
	returns the model index of an aspect defined via its path.
 */
QModelIndex AspectTreeModel::modelIndexOfAspect(const QString& path, int column) const {
	//determine the aspect out of aspect path
	AbstractAspect* aspect = nullptr;
	auto children = m_root->children("AbstractAspect", AbstractAspect::Recursive);
	for (auto* child: children) {
		if (child->path() == path) {
			aspect = child;
			break;
		}
	}

	//return the model index of the aspect
	if (aspect)
		return modelIndexOfAspect(aspect, column);

	return QModelIndex{};
}

void AspectTreeModel::setFilterString(const QString & s) {
	m_filterString=s;
	QModelIndex  topLeft = this->index(0,0, QModelIndex());
	QModelIndex  bottomRight =  this->index(this->rowCount()-1,3, QModelIndex());
	emit dataChanged(topLeft, bottomRight);
}

void AspectTreeModel::setFilterCaseSensitivity(Qt::CaseSensitivity cs) {
	m_filterCaseSensitivity = cs;
}

void AspectTreeModel::setFilterMatchCompleteWord(bool b) {
	m_matchCompleteWord = b;
}

bool AspectTreeModel::containsFilterString(const AbstractAspect* aspect) const {
	if (m_matchCompleteWord) {
		if (aspect->name().compare(m_filterString, m_filterCaseSensitivity) == 0)
			return true;
	} else {
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
void AspectTreeModel::renameRequestedSlot() {
	AbstractAspect* aspect = qobject_cast<AbstractAspect*>(QObject::sender());
	if (aspect)
		emit renameRequested(modelIndexOfAspect(aspect));
}

void AspectTreeModel::aspectSelectedInView(const AbstractAspect* aspect) {
	if (aspect->hidden()) {
		//a hidden aspect was selected in the view (e.g. plot title in WorksheetView)
		//select the parent aspect first, if available
		AbstractAspect* parent = aspect->parentAspect();
		if (parent)
			emit indexSelected(modelIndexOfAspect(parent));

		//emit also this signal, so the GUI can handle this selection.
		emit hiddenAspectSelected(aspect);
	} else
		emit indexSelected(modelIndexOfAspect(aspect));

	//deselect the root item when one of the children was selected in the view
	//in order to avoid multiple selection with the project item (if selected) in the project explorer
	emit indexDeselected(modelIndexOfAspect(m_root));
}

void AspectTreeModel::aspectDeselectedInView(const AbstractAspect* aspect) {
	if (aspect->hidden()) {
		AbstractAspect* parent = aspect->parentAspect();
		if (parent)
			emit indexDeselected(modelIndexOfAspect(parent));
	} else
		emit indexDeselected(modelIndexOfAspect(aspect));
}
