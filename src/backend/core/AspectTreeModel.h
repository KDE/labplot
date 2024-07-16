/*
	File       	    : AspectTreeModel.h
	Project         : LabPlot
	Description     : Represents a tree of AbstractAspect objects as a Qt item model.
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2007-2009 Knut Franke <knut.franke@gmx.de>
	SPDX-FileCopyrightText: 2007-2009 Tilman Benkert <thzs@gmx.net>
	SPDX-FileCopyrightText: 2011-2016 Alexander Semke <alexander.semke@web.de>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef ASPECT_TREE_MODEL_H
#define ASPECT_TREE_MODEL_H

#include <QAbstractItemModel>

enum class AspectType : quint64;
class AbstractAspect;

class AspectTreeModel : public QAbstractItemModel {
	Q_OBJECT

public:
	explicit AspectTreeModel(AbstractAspect* root, QObject* parent = nullptr);
	void setRoot(AbstractAspect*);

	QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const override;
	QModelIndex parent(const QModelIndex& index) const override;
	int rowCount(const QModelIndex& parent = QModelIndex()) const override;
	int columnCount(const QModelIndex& parent = QModelIndex()) const override;

	QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
	QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
	bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) override;
	Qt::ItemFlags flags(const QModelIndex& index) const override;

	QModelIndex modelIndexOfAspect(const AbstractAspect*, int column = 0) const;
	QModelIndex modelIndexOfAspect(const QString& path, int column = 0) const;

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

private Q_SLOTS:
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
	void aspectAboutToBeMoved(const AbstractAspect*, int destinationRow);
	void aspectMoved();

private:
	AbstractAspect* m_root{nullptr};
	bool m_readOnly{false};
	bool m_folderSelectable{true};
	bool m_plottableColumnsOnly{false};
	bool m_numericColumnsOnly{false};
	bool m_nonEmptyNumericColumnsOnly{false};
	bool m_showPlotDesignation{false};
	/*!
	 * \brief m_selectableAspects
	 * Determines the types of selected aspects. If empty all aspects are selectable
	 */
	QList<AspectType> m_selectableAspects;

	QString m_filterString;
	Qt::CaseSensitivity m_filterCaseSensitivity{Qt::CaseInsensitive};
	bool m_matchCompleteWord{false};
	bool containsFilterString(const AbstractAspect*) const;
	bool m_aspectAboutToBeRemovedCalled{false};
	bool m_aspectAboutToBeMovedCalled{false};

Q_SIGNALS:
	void renameRequested(const QModelIndex&);
	void indexSelected(const QModelIndex&);
	void indexDeselected(const QModelIndex&);
	void hiddenAspectSelected(const AbstractAspect*);
	void statusInfo(const QString&);
};

#endif // ifndef ASPECT_TREE_MODEL_H
