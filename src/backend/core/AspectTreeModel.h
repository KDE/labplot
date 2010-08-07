/***************************************************************************
    File                 : AspectTreeModel.h
    Project              : SciDAVis
    --------------------------------------------------------------------
    Copyright            : (C) 2007-2009 by Knut Franke, Tilman Benkert
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
#include <QItemSelection>

class AbstractAspect;

class AspectTreeModel : public QAbstractItemModel{
	Q_OBJECT

	public:
		AspectTreeModel(AbstractAspect* root, QObject *parent=0);
		~AspectTreeModel();

		enum CustomDataRole {
			ContextMenuRole = Qt::UserRole,
		};

		QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
		QModelIndex parent(const QModelIndex &index) const;
		int rowCount(const QModelIndex &parent = QModelIndex()) const;
		int columnCount(const QModelIndex &parent = QModelIndex()) const;
		QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
		QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
		bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);
		Qt::ItemFlags flags(const QModelIndex &index) const;
		void setFolderSelectable(const bool);
		QModelIndex modelIndexOfAspect(const AbstractAspect *aspect, int column=0) const;
		
	private slots:
		void aspectDescriptionChanged(const AbstractAspect *aspect);
		void aspectAboutToBeAdded(const AbstractAspect *parent, const AbstractAspect *before, const AbstractAspect *child);
		void aspectAdded(const AbstractAspect *parent);
		void aspectAboutToBeRemoved(const AbstractAspect *aspect);
		void aspectRemoved();
		void aspectHiddenAboutToChange(const AbstractAspect * aspect);
		void aspectHiddenChanged(const AbstractAspect *aspect);
		void aspectSelectedInView(const AbstractAspect* aspect);
		void aspectDeselectedInView(const AbstractAspect* aspect);
		
	private:
		AbstractAspect* m_root;
		bool m_folderSelectable;
		
	signals:
	  void indexSelected(const QModelIndex&);
	  void indexDeselected(const QModelIndex&);
};

#endif // ifndef ASPECT_TREE_MODEL_H
