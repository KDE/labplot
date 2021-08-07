/***************************************************************************
    File       	    : AspectTreeModel.h
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
#ifndef ASPECT_TREE_MODEL_H
#define ASPECT_TREE_MODEL_H

#include <QAbstractItemModel>

enum class AspectType : quint64;
class AbstractAspect;

class AspectTreeModel : public QAbstractItemModel {
	Q_OBJECT

public:
	explicit AspectTreeModel(AbstractAspect* root, QObject* parent=nullptr);

	QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
	QModelIndex parent(const QModelIndex &index) const override;
	int rowCount(const QModelIndex &parent = QModelIndex()) const override;
	int columnCount(const QModelIndex &parent = QModelIndex()) const override;

	QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
	QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
	bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
	Qt::ItemFlags flags(const QModelIndex &index) const override;

	QModelIndex modelIndexOfAspect(const AbstractAspect*, int column=0) const;
	QModelIndex modelIndexOfAspect(const QString& path, int column=0) const;

	void setSelectableAspects(const QList<AspectType>&);
	const QList<AspectType>& selectableAspects() const;

	void setReadOnly(bool);
	void enablePlottableColumnsOnly(bool);
	void enableNumericColumnsOnly(bool);
	void enableNonEmptyNumericColumnsOnly(bool);
	void enableShowPlotDesignation(bool);
	void setFilterString(const QString&);
	void setFilterCaseSensitivity(Qt::CaseSensitivity);
	void setFilterMatchCompleteWord(bool);

private slots:
	void aspectDescriptionChanged(const AbstractAspect*);
	void aspectAboutToBeAdded(const AbstractAspect* parent, const AbstractAspect* before, const AbstractAspect* child);
	void aspectAdded(const AbstractAspect* parent);
	void aspectAboutToBeRemoved(const AbstractAspect*);
	void aspectRemoved();
	void aspectHiddenAboutToChange(const AbstractAspect*);
	void aspectHiddenChanged(const AbstractAspect*);
	void aspectSelectedInView(const AbstractAspect*);
	void aspectDeselectedInView(const AbstractAspect*);
	void renameRequestedSlot();

private:
	AbstractAspect* m_root;
	bool m_readOnly{false};
	bool m_folderSelectable{true};
	bool m_plottableColumnsOnly{false};
	bool m_numericColumnsOnly{false};
	bool m_nonEmptyNumericColumnsOnly{false};
	bool m_showPlotDesignation{false};
	QList<AspectType> m_selectableAspects;

	QString m_filterString;
	Qt::CaseSensitivity m_filterCaseSensitivity{Qt::CaseInsensitive};
	bool m_matchCompleteWord{false};
	bool containsFilterString(const AbstractAspect*) const;

signals:
	void renameRequested(const QModelIndex&);
	void indexSelected(const QModelIndex&);
	void indexDeselected(const QModelIndex&);
	void hiddenAspectSelected(const AbstractAspect*);
	void statusInfo(const QString&);
};

#endif // ifndef ASPECT_TREE_MODEL_H
