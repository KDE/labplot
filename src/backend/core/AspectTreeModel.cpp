/*
	File       	    : AspectTreeModel.h
	Project         : LabPlot
	Description     : Represents a tree of AbstractAspect objects as a Qt item model.
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2007-2009 Knut Franke <knut.franke@gmx.de>
	SPDX-FileCopyrightText: 2007-2009 Tilman Benkert <thzs@gmx.net>
	SPDX-FileCopyrightText: 2011-2021 Alexander Semke <alexander.semke@web.de>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "backend/core/AspectTreeModel.h"
#include "backend/core/AbstractAspect.h"
#include "backend/core/column/Column.h"
#include "backend/worksheet/WorksheetElement.h"

#include <QApplication>
#include <QDateTime>
#include <QFontMetrics>
#include <QIcon>
#include <QMenu>

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
	: QAbstractItemModel(parent) {
	setRoot(root);
}

void AspectTreeModel::setRoot(AbstractAspect* root) {
	if (m_root)
		disconnect(m_root, nullptr, this, nullptr);

	m_root = root;
	connect(m_root, &AbstractAspect::renameRequested, this, &AspectTreeModel::renameRequestedSlot);
	connect(m_root, &AbstractAspect::aspectDescriptionChanged, this, &AspectTreeModel::aspectDescriptionChanged);
	connect(m_root,
			QOverload<const AbstractAspect*, const AbstractAspect*, const AbstractAspect*>::of(&AbstractAspect::childAspectAboutToBeAdded),
			this,
			QOverload<const AbstractAspect*, const AbstractAspect*, const AbstractAspect*>::of(&AspectTreeModel::aspectAboutToBeAdded));
	connect(m_root, &AbstractAspect::childAspectAboutToBeRemoved, this, &AspectTreeModel::aspectAboutToBeRemoved);
	connect(m_root, &AbstractAspect::childAspectAdded, this, &AspectTreeModel::aspectAdded);
	connect(m_root, &AbstractAspect::childAspectRemoved, this, &AspectTreeModel::aspectRemoved);
	connect(m_root, &AbstractAspect::aspectHiddenAboutToChange, this, &AspectTreeModel::aspectHiddenAboutToChange);
	connect(m_root, &AbstractAspect::aspectHiddenChanged, this, &AspectTreeModel::aspectHiddenChanged);
	connect(m_root, &AbstractAspect::childAspectAboutToBeMoved, this, &AspectTreeModel::aspectAboutToBeMoved);
	connect(m_root, &AbstractAspect::childAspectMoved, this, &AspectTreeModel::aspectMoved);
}

/*!
  \c list contains the class names of the aspects, that can be selected in the corresponding model view.
*/
void AspectTreeModel::setSelectableAspects(const QList<AspectType>& list) {
	m_selectableAspects = list;
}

const QList<AspectType>& AspectTreeModel::selectableAspects() const {
	return m_selectableAspects;
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

QModelIndex AspectTreeModel::index(int row, int column, const QModelIndex& parent) const {
	if (!m_root || !hasIndex(row, column, parent))
		return QModelIndex{};

	if (!parent.isValid()) {
		if (row != 0)
			return QModelIndex{};
		return createIndex(row, column, m_root);
	}

	auto* parent_aspect = static_cast<AbstractAspect*>(parent.internalPointer());
	auto* child_aspect = parent_aspect->child<AbstractAspect>(row);
	if (!child_aspect)
		return QModelIndex{};
	return createIndex(row, column, child_aspect);
}

QModelIndex AspectTreeModel::parent(const QModelIndex& index) const {
	if (!index.isValid())
		return QModelIndex{};

	auto* aspect = static_cast<AbstractAspect*>(index.internalPointer());
	if (!aspect)
		return QModelIndex{};

	auto* parent = aspect->parentAspect();
	if (!parent)
		return QModelIndex{};

	return modelIndexOfAspect(parent);
}

int AspectTreeModel::rowCount(const QModelIndex& parent) const {
	if (!parent.isValid())
		return 1;

	auto* parent_aspect = static_cast<AbstractAspect*>(parent.internalPointer());
	return parent_aspect->childCount<AbstractAspect>();
}

int AspectTreeModel::columnCount(const QModelIndex& /*parent*/) const {
	return 4;
}

QVariant AspectTreeModel::headerData(int section, Qt::Orientation orientation, int role) const {
	if (orientation != Qt::Horizontal)
		return {};

	switch (role) {
	case Qt::DisplayRole:
		switch (section) {
		case 0:
			return i18n("Name");
		case 1:
			return i18n("Type");
		case 2:
			return i18n("Created");
		case 3:
			return i18n("Comment");
		default:
			return {};
		}
	default:
		return {};
	}
}

QVariant AspectTreeModel::data(const QModelIndex& index, int role) const {
	if (!index.isValid())
		return {};

	auto* aspect = static_cast<AbstractAspect*>(index.internalPointer());
	switch (role) {
	case Qt::DisplayRole:
	case Qt::EditRole:
		switch (index.column()) {
		case 0: {
			const auto* column = dynamic_cast<const Column*>(aspect);
			if (column) {
				QString name = aspect->name();
				if (m_plottableColumnsOnly && !column->isPlottable())
					name = i18n("%1   (non-plottable data)", name);
				else if (m_numericColumnsOnly && !column->isNumeric())
					name = i18n("%1   (non-numeric data)", name);
				else if (m_nonEmptyNumericColumnsOnly && !column->hasValues())
					name = i18n("%1   (no values)", name);

				if (m_showPlotDesignation)
					name += QLatin1Char('\t') + column->plotDesignationString();

				return name;
			} else if (aspect)
				return aspect->name();
			else
				return {};
		}
		case 1:
			if (QLatin1String(aspect->metaObject()->className()) == QLatin1String("CantorWorksheet"))
				return QLatin1String("Notebook");
			else if (QLatin1String(aspect->metaObject()->className()) == QLatin1String("Datapicker"))
				return QLatin1String("DataExtractor");
			else if (QLatin1String(aspect->metaObject()->className()) == QLatin1String("CartesianPlot"))
				return QLatin1String("Plot Area");
			else
				return QLatin1String(aspect->metaObject()->className());
		case 2:
			return QLocale::system().toString(aspect->creationTime(), QLocale::ShortFormat);
		case 3:
			return aspect->comment().replace(QLatin1Char('\n'), QLatin1Char(' ')).simplified();
		default:
			return {};
		}
	case Qt::ToolTipRole: {
		QString toolTip;
		if (aspect->comment().isEmpty())
			toolTip = QLatin1String("<b>") + aspect->name() + QLatin1String("</b>");
		else
			toolTip =
				QLatin1String("<b>") + aspect->name() + QLatin1String("</b><br><br>") + aspect->comment().replace(QLatin1Char('\n'), QLatin1String("<br>"));

		const auto* col = dynamic_cast<const Column*>(aspect);
		if (col) {
			toolTip += QLatin1String("<br>");
			toolTip += QLatin1String("<br>") + i18n("Size: %1", col->rowCount());
			// TODO: active this once we have a more efficient implementation of this function
			// toolTip += QLatin1String("<br>") + i18n("Values: %1", col->availableRowCount());
			toolTip += QLatin1String("<br>") + i18n("Type: %1", col->columnModeString());
			toolTip += QLatin1String("<br>") + i18n("Plot Designation: %1", col->plotDesignationString());

			// in case it's a calculated column, add additional information
			// about the formula and parameters
			if (!col->formula().isEmpty()) {
				toolTip += QLatin1String("<br><br>") + i18n("Formula:");
				QString f(QStringLiteral("f("));
				QString parameters;
				for (int i = 0; i < col->formulaData().size(); ++i) {
					auto& data = col->formulaData().at(i);

					// string for the function definition like f(x,y), etc.
					f += data.variableName();
					if (i != col->formulaData().size() - 1)
						f += QStringLiteral(", ");

					// string for the parameters and the references to the used columns for them
					if (!parameters.isEmpty())
						parameters += QLatin1String("<br>");
					parameters += data.variableName();
					if (data.column())
						parameters += QStringLiteral(" = ") + data.column()->path();
				}

				toolTip += QStringLiteral("<br>") + f + QStringLiteral(") = ") + col->formula();
				toolTip += QStringLiteral("<br>") + parameters;
				if (col->formulaAutoUpdate())
					toolTip += QStringLiteral("<br>") + i18n("auto update: true");
				else
					toolTip += QStringLiteral("<br>") + i18n("auto update: false");
			}
		}

		return toolTip;
	}
	case Qt::DecorationRole:
		return index.column() == 0 ? aspect->icon() : QIcon();
	case Qt::ForegroundRole: {
		const WorksheetElement* we = dynamic_cast<WorksheetElement*>(aspect);
		if (we) {
			if (!we->isVisible())
				return QVariant(QApplication::palette().color(QPalette::Disabled, QPalette::Text));
		}
		return QVariant(QApplication::palette().color(QPalette::Active, QPalette::Text));
	}
	default:
		return {};
	}
}

Qt::ItemFlags AspectTreeModel::flags(const QModelIndex& index) const {
	if (!index.isValid())
		return Qt::NoItemFlags;

	Qt::ItemFlags result;
	auto* aspect = static_cast<AbstractAspect*>(index.internalPointer());

	if (!m_selectableAspects.isEmpty()) {
		for (AspectType type : m_selectableAspects) {
			if (aspect->inherits(type)) {
				result = Qt::ItemIsEnabled | Qt::ItemIsSelectable;
				if (index != this->index(0, 0, QModelIndex()) && !m_filterString.isEmpty()) {
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
		// default case: the list for the selectable aspects is empty and all aspects are selectable.
		//  Apply filter, if available. Indices, that don't match the filter are not selectable.
		// Don't apply any filter to the very first index in the model  - this top index corresponds to the project item.
		if (index != this->index(0, 0, QModelIndex()) && !m_filterString.isEmpty()) {
			if (this->containsFilterString(aspect))
				result = Qt::ItemIsEnabled | Qt::ItemIsSelectable;
			else
				result = Qt::ItemIsSelectable;
		} else
			result = Qt::ItemIsEnabled | Qt::ItemIsSelectable;
	}

	// the columns "name" and "description" are editable
	if (!m_readOnly) {
		if (index.column() == 0 || index.column() == 3)
			result |= Qt::ItemIsEditable;
	}

	const auto* column = dynamic_cast<const Column*>(aspect);
	if (column) {
		// allow to drag and drop columns for the faster creation of curves in the plots.
		// TODO: allow drag&drop later for other objects too, once we implement copy and paste in the project explorer
		result = result | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled;

		if (m_plottableColumnsOnly && !column->isPlottable())
			result &= ~Qt::ItemIsEnabled;

		if (m_numericColumnsOnly && !column->isNumeric())
			result &= ~Qt::ItemIsEnabled;

		if (m_nonEmptyNumericColumnsOnly && !(column->isNumeric() && column->hasValues()))
			result &= ~Qt::ItemIsEnabled;
	}

	return result;
}

void AspectTreeModel::aspectDescriptionChanged(const AbstractAspect* aspect) {
	Q_EMIT dataChanged(modelIndexOfAspect(aspect), modelIndexOfAspect(aspect, 3));
}

void AspectTreeModel::aspectAboutToBeAdded(const AbstractAspect* parent, const AbstractAspect* before, const AbstractAspect* /*child*/) {
	int index = parent->indexOfChild<AbstractAspect>(before);
	if (index == -1)
		index = parent->childCount<AbstractAspect>();

	beginInsertRows(modelIndexOfAspect(parent), index, index);
}

void AspectTreeModel::aspectAdded(const AbstractAspect* aspect) {
	endInsertRows();
	AbstractAspect* parent = aspect->parentAspect();
	Q_EMIT dataChanged(modelIndexOfAspect(parent), modelIndexOfAspect(parent, 3));

	connect(aspect, &AbstractAspect::renameRequested, this, &AspectTreeModel::renameRequestedSlot);
	connect(aspect, &AbstractAspect::childAspectSelectedInView, this, &AspectTreeModel::aspectSelectedInView);
	connect(aspect, &AbstractAspect::childAspectDeselectedInView, this, &AspectTreeModel::aspectDeselectedInView);

	// add signal-slot connects for all children, too
	const auto& children = aspect->children<AbstractAspect>(AbstractAspect::ChildIndexFlag::Recursive);
	for (const auto* child : children) {
		connect(child, &AbstractAspect::renameRequested, this, &AspectTreeModel::renameRequestedSlot);
		connect(child, &AbstractAspect::childAspectSelectedInView, this, &AspectTreeModel::aspectSelectedInView);
		connect(child, &AbstractAspect::childAspectDeselectedInView, this, &AspectTreeModel::aspectDeselectedInView);
	}
}

void AspectTreeModel::aspectAboutToBeRemoved(const AbstractAspect* aspect) {
	AbstractAspect* parent = aspect->parentAspect();
	int index = parent->indexOfChild<AbstractAspect>(aspect);
	m_aspectAboutToBeRemovedCalled = true;
	beginRemoveRows(modelIndexOfAspect(parent), index, index);
}

void AspectTreeModel::aspectRemoved() {
	// make sure aspectToBeRemoved(), and with this beginRemoveRows() in the model, was called
	// prior to calling endRemoveRows() further below.
	// see https://invent.kde.org/education/labplot/-/merge_requests/278 for more information.
	if (!m_aspectAboutToBeRemovedCalled)
		return;

	m_aspectAboutToBeRemovedCalled = false;
	endRemoveRows();
}

void AspectTreeModel::aspectAboutToBeMoved(const AbstractAspect* aspect, int destinationRow) {
	AbstractAspect* parent = aspect->parentAspect();
	int index = parent->indexOfChild<AbstractAspect>(aspect);
	const auto& parentIndex = modelIndexOfAspect(parent);
	m_aspectAboutToBeMovedCalled = true;
	if (!beginMoveRows(parentIndex, index, index, parentIndex, destinationRow)) {
		Q_ASSERT(false); // Must be done like this, because otherwise in release build the assert will not be executed
	}
}

void AspectTreeModel::aspectMoved() {
	Q_ASSERT(m_aspectAboutToBeMovedCalled == true);
	m_aspectAboutToBeMovedCalled = false;
	endMoveRows();
}

void AspectTreeModel::aspectHiddenAboutToChange(const AbstractAspect* aspect) {
	for (AbstractAspect* i = aspect->parentAspect(); i; i = i->parentAspect())
		if (i->hidden())
			return;
	if (aspect->hidden())
		aspectAboutToBeAdded(aspect->parentAspect(), aspect, aspect);
	else
		aspectAboutToBeRemoved(aspect);
}

void AspectTreeModel::aspectHiddenChanged(const AbstractAspect* aspect) {
	for (AbstractAspect* i = aspect->parentAspect(); i; i = i->parentAspect())
		if (i->hidden())
			return;
	if (aspect->hidden())
		aspectRemoved();
	else
		aspectAdded(aspect);
}

bool AspectTreeModel::setData(const QModelIndex& index, const QVariant& value, int role) {
	if (!index.isValid() || role != Qt::EditRole)
		return false;
	auto* aspect = static_cast<AbstractAspect*>(index.internalPointer());
	switch (index.column()) {
	case 0: {
		if (!aspect->setName(value.toString(), AbstractAspect::NameHandling::UniqueRequired)) {
			Q_EMIT statusInfo(i18n("The name \"%1\" is already in use. Choose another name.", value.toString()));
			return false;
		}
		break;
	}
	case 3:
		aspect->setComment(value.toString());
		break;
	default:
		return false;
	}
	Q_EMIT dataChanged(index, index);
	return true;
}

QModelIndex AspectTreeModel::modelIndexOfAspect(const AbstractAspect* aspect, int column) const {
	if (!aspect)
		return QModelIndex();
	AbstractAspect* parent = aspect->parentAspect();
	return createIndex(parent ? parent->indexOfChild<AbstractAspect>(aspect) : 0, column, const_cast<AbstractAspect*>(aspect));
}

/*!
	returns the model index of an aspect defined via its path.
 */
QModelIndex AspectTreeModel::modelIndexOfAspect(const QString& path, int column) const {
	// determine the aspect out of aspect path
	if (!m_root)
		return QModelIndex();
	AbstractAspect* aspect = nullptr;
	if (m_root->path() != path) {
		const auto& children = m_root->children<AbstractAspect>(AbstractAspect::ChildIndexFlag::Recursive);
		for (auto* child : children) {
			if (child->path() == path) {
				aspect = child;
				break;
			}
		}
	} else
		aspect = m_root;

	// return the model index of the aspect
	if (aspect)
		return modelIndexOfAspect(aspect, column);

	return QModelIndex{};
}

void AspectTreeModel::setFilterString(const QString& s) {
	m_filterString = s;
	QModelIndex topLeft = this->index(0, 0, QModelIndex());
	QModelIndex bottomRight = this->index(this->rowCount() - 1, 3, QModelIndex());
	Q_EMIT dataChanged(topLeft, bottomRight);
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

	// check for the occurrence of the filter string in the names of the parents
	if (aspect->parentAspect())
		return this->containsFilterString(aspect->parentAspect());
	else
		return false;

	// TODO make this optional
	//  	//check for the occurrence of the filter string in the names of the children
	// 	foreach(const AbstractAspect * child, aspect->children<AbstractAspect>()) {
	// 	  if ( this->containsFilterString(child) )
	// 		return true;
	// 	}
}

// ##############################################################################
// #################################  SLOTS  ####################################
// ##############################################################################
void AspectTreeModel::renameRequestedSlot() {
	auto* aspect = dynamic_cast<AbstractAspect*>(QObject::sender());
	if (aspect)
		Q_EMIT renameRequested(modelIndexOfAspect(aspect));
}

void AspectTreeModel::aspectSelectedInView(const AbstractAspect* aspect) {
	if (aspect->hidden()) {
		// a hidden aspect was selected in the view (e.g. plot title in WorksheetView)
		// select the parent aspect first, if available
		AbstractAspect* parent = aspect->parentAspect();
		if (parent)
			Q_EMIT indexSelected(modelIndexOfAspect(parent));

		// Q_EMIT also this signal, so the GUI can handle this selection.
		Q_EMIT hiddenAspectSelected(aspect);
	} else
		Q_EMIT indexSelected(modelIndexOfAspect(aspect));

	// deselect the root item when one of the children was selected in the view
	// in order to avoid multiple selection with the project item (if selected) in the project explorer
	Q_EMIT indexDeselected(modelIndexOfAspect(m_root));
}

void AspectTreeModel::aspectDeselectedInView(const AbstractAspect* aspect) {
	if (aspect->hidden()) {
		AbstractAspect* parent = aspect->parentAspect();
		if (parent)
			Q_EMIT indexDeselected(modelIndexOfAspect(parent));
	} else
		Q_EMIT indexDeselected(modelIndexOfAspect(aspect));
}
