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

class Column;
class Spreadsheet;
class AbstractAspect;

class SpreadsheetModel : public QAbstractItemModel {
	Q_OBJECT

public:
	explicit SpreadsheetModel(Spreadsheet*);

	enum class CustomDataRole {
		MaskingRole = Qt::UserRole, //!< bool determining whether the cell is masked
		FormulaRole = Qt::UserRole + 1, //!< the cells formula
		CommentRole = Qt::UserRole + 2, //!< the column comment (for headerData())
		SparkLineRole = Qt::UserRole + 3, // the sparkline comment ( for headerData())
	};

	Qt::ItemFlags flags(const QModelIndex&) const override;
	QVariant data(const QModelIndex&, int role) const override;
	QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
	int rowCount(const QModelIndex& parent = QModelIndex()) const override;
	int columnCount(const QModelIndex& parent = QModelIndex()) const override;
	bool setData(const QModelIndex&, const QVariant& value, int role) override;
	QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const override;
	QModelIndex parent(const QModelIndex& child) const override;
	bool hasChildren(const QModelIndex& parent = QModelIndex()) const override;

	Column* column(int index);

	void activateFormulaMode(bool on);
	bool formulaModeActive() const;

	void updateHorizontalHeader(bool sendSignal = true);
	void suppressSignals(bool);

	void setSearchText(const QString&);
	QModelIndex index(const QString&) const;

	Spreadsheet* spreadsheet();

private Q_SLOTS:
	void handleAspectsAboutToBeInserted(int first, int last);
	void handleAspectsInserted(int first, int last);
	void handleAspectsAboutToBeRemoved(int first, int last);
	void handleAspectsRemoved();
	void handleAspectCountChanged();

	void handleAspectAboutToBeAdded(const AbstractAspect*, int index, const AbstractAspect*);
	void handleAspectAdded(const AbstractAspect* aspect);
	void handleAspectAboutToBeRemoved(const AbstractAspect* aspect);
	void handleAspectRemoved(const AbstractAspect* parent, const AbstractAspect* before, const AbstractAspect* child);

	void handleDescriptionChange(const AbstractAspect*);
	void handleModeChange(const AbstractColumn*);
	void handleDigitsChange();
	void handlePlotDesignationChange(const AbstractColumn*);
	void handleDataChange(const AbstractColumn*);
	void handleRowsInserted(int newRowCount);
	void handleRowsRemoved(int newRowCount);
	void handleRowsAboutToBeInserted(int before, int last);
	void handleRowsAboutToBeRemoved(int first, int last);

	void handleRowCountChanged(int newRowCount);

protected:
	void updateVerticalHeader();

private:
	Spreadsheet* m_spreadsheet;
	bool m_formula_mode{false};
	QStringList m_horizontal_header_data;
	int m_defaultHeaderHeight;
	bool m_suppressSignals{false};
	bool m_spreadsheetColumnCountChanging{false};
	int m_rowCount{0};
	int m_verticalHeaderCount{0};
	int m_columnCount{0};
	QString m_searchText;

	QVariant color(const AbstractColumn*, int row, AbstractColumn::Formatting) const;
};

#endif
