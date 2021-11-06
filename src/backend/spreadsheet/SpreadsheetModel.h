/*
    File                 : SpreadsheetModel.h
    Project              : LabPlot
    Description          : Model for the access to a Spreadsheet
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2007 Tilman Benkert <thzs@gmx.net>
    SPDX-FileCopyrightText: 2009 Knut Franke <knut.franke@gmx.de>
    SPDX-FileCopyrightText: 2013-2021 Alexander Semke <alexander.semke@web.de>
    SPDX-License-Identifier: GPL-2.0-or-later
*/


#ifndef SPREADSHEETMODEL_H
#define SPREADSHEETMODEL_H

#include "backend/core/AbstractColumn.h"
#include <QAbstractItemModel>

class QStringList;
class Column;
class Spreadsheet;
class AbstractAspect;

class SpreadsheetModel : public QAbstractItemModel {
	Q_OBJECT

public:
	explicit SpreadsheetModel(Spreadsheet*);

	enum class CustomDataRole {
		MaskingRole = Qt::UserRole, //!< bool determining whether the cell is masked
		FormulaRole = Qt::UserRole+1, //!< the cells formula
		CommentRole = Qt::UserRole+2, //!< the column comment (for headerData())
	};

	Qt::ItemFlags flags( const QModelIndex & index ) const override;
	QVariant data(const QModelIndex& index, int role) const override;
	QVariant headerData(int section, Qt::Orientation orientation,int role) const override;
	int rowCount(const QModelIndex& parent = QModelIndex()) const override;
	int columnCount(const QModelIndex& parent = QModelIndex()) const override;
	bool setData(const QModelIndex& index, const QVariant& value, int role) override;
	QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const override;
	QModelIndex parent(const QModelIndex& child) const override;
	bool hasChildren (const QModelIndex& parent = QModelIndex() ) const override;

	Column* column(int index);

	void activateFormulaMode(bool on);
	bool formulaModeActive() const;

	void updateHorizontalHeader();
	void suppressSignals(bool);

	void setSearchText(const QString&);

private slots:
	void handleAspectAdded(const AbstractAspect*);
	void handleAspectAboutToBeRemoved(const AbstractAspect*);
	void handleAspectRemoved(const AbstractAspect* parent, const AbstractAspect* before, const AbstractAspect* child);

	void handleDescriptionChange(const AbstractAspect*);
	void handleModeChange(const AbstractColumn*);
	void handleDigitsChange();
	void handlePlotDesignationChange(const AbstractColumn*);
	void handleDataChange(const AbstractColumn*);
	void handleRowsInserted(const AbstractColumn*, int before, int count);
	void handleRowsRemoved(const AbstractColumn*, int first, int count);

protected:
	void updateVerticalHeader();

private:
	Spreadsheet* m_spreadsheet;
	bool m_formula_mode{false};
	QVector<int> m_vertical_header_data;
	QStringList m_horizontal_header_data;
	int m_defaultHeaderHeight;
	bool m_suppressSignals{false};
	int m_rowCount{0};
	int m_columnCount{0};
	QString m_searchText;

	QVariant color(const AbstractColumn*, int row, AbstractColumn::Formatting type) const;
};

#endif
