/***************************************************************************
    File                 : AspectTreeModel.h
    Project              : SciDAVis
    --------------------------------------------------------------------
    Copyright            : (C) 2007 by Knut Franke, Tilman Benkert
    Email (use @ for *)  : knut.franke*gmx.de, thzs*gmx.net
    Description          : Represents a tree of AbstractAspect objects as a
                           Qt item model.

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
#ifndef ASPECT_TREE_MODEL_H
#define ASPECT_TREE_MODEL_H

#include <QAbstractItemModel>
#include "core/AbstractAspect.h"

//! Represents a tree of AbstractAspect objects as a Qt item model.
/**
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
 * AbstractAspect::createContextMenu() is made availabel via the custom role ContextMenuRole.
 */
class AspectTreeModel : public QAbstractItemModel
{
	Q_OBJECT

	public:
		AspectTreeModel(AbstractAspect* root, QObject *parent=0);
		~AspectTreeModel();

		//! Custom data roles used in addition to Qt::ItemDataRole
		enum CustomDataRole {
			ContextMenuRole = Qt::UserRole, //!< pointer to a new context menu for an Aspect
		};

		QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
		QModelIndex parent(const QModelIndex &index) const;
		int rowCount(const QModelIndex &parent = QModelIndex()) const;
		int columnCount(const QModelIndex &parent = QModelIndex()) const;
		QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
		QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
		bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);
		Qt::ItemFlags flags(const QModelIndex &index) const;

		//! Convenience wrapper around QAbstractItemModel::createIndex().
		QModelIndex modelIndexOfAspect(const AbstractAspect *aspect, int column=0) const 
		{
			return createIndex(aspect->index(), column, const_cast<AbstractAspect*>(aspect));
		}

	public slots:
		void aspectDescriptionChanged(const AbstractAspect *aspect);
		void aspectAboutToBeAdded(const AbstractAspect *parent, int index);
		void aspectAdded(const AbstractAspect *parent, int index);
		void aspectAboutToBeRemoved(const AbstractAspect *parent, int index);
		void aspectRemoved(const AbstractAspect *parent, int index);


	private:
		AbstractAspect* m_root;
};

#endif // ifndef ASPECT_TREE_MODEL_H
