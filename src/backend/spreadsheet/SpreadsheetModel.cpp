/*
	File                 : SpreadsheetModel.cpp
	Project              : LabPlot
	Description          : Model for the access to a Spreadsheet
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2007 Tilman Benkert <thzs@gmx.net>
	SPDX-FileCopyrightText: 2009 Knut Franke <knut.franke@gmx.de>
	SPDX-FileCopyrightText: 2013-2022 Alexander Semke <alexander.semke@web.de>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "backend/spreadsheet/SpreadsheetModel.h"
#include "backend/core/Settings.h"
#include "backend/core/datatypes/Double2StringFilter.h"
#include "backend/lib/macros.h"
#include "backend/lib/trace.h"
#include "backend/spreadsheet/Spreadsheet.h"
#include "frontend/spreadsheet/SpreadsheetSparkLineHeaderModel.h"

#include <KConfigGroup>
#include <KLocalizedString>

#include <QBrush>
#include <QIcon>
#include <QPalette>

/*!
	\class SpreadsheetModel
	\brief  Model for the access to a Spreadsheet

	This is a model in the sense of Qt4 model/view framework which is used
	to access a Spreadsheet object from any of Qt4s view classes, typically a QTableView.
	Its main purposes are translating Spreadsheet signals into QAbstractItemModel signals
	and translating calls to the QAbstractItemModel read/write API into calls
	in the public API of Spreadsheet. In many cases a pointer to the addressed column
	is obtained by calling Spreadsheet::column() and the manipulation is done using the
	public API of column.

	\ingroup backend
*/
SpreadsheetModel::SpreadsheetModel(Spreadsheet* spreadsheet)
	: QAbstractItemModel(nullptr)
	, m_spreadsheet(spreadsheet)
	, m_rowCount(spreadsheet->rowCount())
	, m_verticalHeaderCount(spreadsheet->rowCount())
	, m_columnCount(spreadsheet->columnCount()) {
	updateVerticalHeader();
	updateHorizontalHeader(false);
	connect(m_spreadsheet, &Spreadsheet::aspectDescriptionChanged, this, &SpreadsheetModel::handleDescriptionChange);

	// Used when single columns get deleted or added
	connect(m_spreadsheet,
			QOverload<const AbstractAspect*, int, const AbstractAspect*>::of(&Spreadsheet::childAspectAboutToBeAdded),
			this,
			&SpreadsheetModel::handleAspectAboutToBeAdded);
	connect(m_spreadsheet, &Spreadsheet::childAspectAdded, this, &SpreadsheetModel::handleAspectAdded);
	connect(m_spreadsheet, &Spreadsheet::childAspectAboutToBeRemoved, this, &SpreadsheetModel::handleAspectAboutToBeRemoved);
	connect(m_spreadsheet, &Spreadsheet::childAspectRemoved, this, &SpreadsheetModel::handleAspectRemoved);

	// Used when changing the column count
	connect(m_spreadsheet, &Spreadsheet::aspectsAboutToBeInserted, this, &SpreadsheetModel::handleAspectsAboutToBeInserted);
	connect(m_spreadsheet, &Spreadsheet::aspectsAboutToBeRemoved, this, &SpreadsheetModel::handleAspectsAboutToBeRemoved);
	connect(m_spreadsheet, &Spreadsheet::aspectsInserted, this, &SpreadsheetModel::handleAspectsInserted);
	connect(m_spreadsheet, &Spreadsheet::aspectsRemoved, this, &SpreadsheetModel::handleAspectsRemoved);

	connect(m_spreadsheet, &Spreadsheet::rowsAboutToBeInserted, this, &SpreadsheetModel::handleRowsAboutToBeInserted);
	connect(m_spreadsheet, &Spreadsheet::rowsAboutToBeRemoved, this, &SpreadsheetModel::handleRowsAboutToBeRemoved);
	connect(m_spreadsheet, &Spreadsheet::rowsInserted, this, &SpreadsheetModel::handleRowsInserted);
	connect(m_spreadsheet, &Spreadsheet::rowsRemoved, this, &SpreadsheetModel::handleRowsRemoved);

	m_suppressSignals = true;
	handleAspectsAboutToBeInserted(0, spreadsheet->columnCount() - 1);
	handleAspectsInserted(0, spreadsheet->columnCount() - 1); // make connections
	m_suppressSignals = false;

	m_spreadsheet->setModel(this);
}

void SpreadsheetModel::suppressSignals(bool value) {
	m_suppressSignals = value;

	// update the headers after all the data was added to the model
	// and we start listening to signals again
	if (m_suppressSignals)
		beginResetModel();
	else {
		m_rowCount = m_spreadsheet->rowCount();
		m_columnCount = m_spreadsheet->columnCount();
		updateHorizontalHeader(false);
		endResetModel();
	}
}

Qt::ItemFlags SpreadsheetModel::flags(const QModelIndex& index) const {
	if (index.isValid())
		return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable;
	else
		return Qt::ItemIsEnabled;
}

void SpreadsheetModel::setSearchText(const QString& text) {
	m_searchText = text;
}

QModelIndex SpreadsheetModel::index(const QString& text) const {
	const int colCount = m_spreadsheet->columnCount();
	const int rowCount = m_spreadsheet->rowCount();
	for (int col = 0; col < colCount; ++col) {
		auto* column = m_spreadsheet->column(col)->asStringColumn();
		for (int row = 0; row < rowCount; ++row) {
			if (column->textAt(row).indexOf(text) != -1)
				return createIndex(row, col);
		}
	}

	return createIndex(-1, -1);
}

Spreadsheet* SpreadsheetModel::spreadsheet() {
	return m_spreadsheet;
}

QVariant SpreadsheetModel::data(const QModelIndex& index, int role) const {
	if (!index.isValid())
		return {};

	const int row = index.row();
	const int col = index.column();
	const Column* col_ptr = m_spreadsheet->column(col);

	if (!col_ptr)
		return {};

	switch (role) {
	case Qt::ToolTipRole:
		if (col_ptr->isValid(row)) {
			if (col_ptr->isMasked(row))
				return {i18n("%1, masked (ignored in all operations)", col_ptr->asStringColumn()->textAt(row))};
			else
				return {col_ptr->asStringColumn()->textAt(row)};
		} else {
			if (col_ptr->isMasked(row))
				return {i18n("invalid cell, masked (ignored in all operations)")};
			else
				return {i18n("invalid cell (ignored in all operations)")};
		}
	case Qt::EditRole:
		if (col_ptr->columnMode() == AbstractColumn::ColumnMode::Double) {
			double value = col_ptr->valueAt(row);
			if (std::isnan(value))
				return {QStringLiteral("-")};
			else if (std::isinf(value))
				return {QStringLiteral("inf")};
			else
				return {col_ptr->asStringColumn()->textAt(row)};
		}

		if (col_ptr->isValid(row))
			return {col_ptr->asStringColumn()->textAt(row)};

		// m_formula_mode is not used at the moment
		// if (m_formula_mode)
		//	return QVariant(col_ptr->formula(row));

		break;
	case Qt::DisplayRole:
		if (col_ptr->columnMode() == AbstractColumn::ColumnMode::Double) {
			double value = col_ptr->valueAt(row);
			if (std::isnan(value))
				return {QStringLiteral("-")};
			else if (std::isinf(value))
				return {UTF8_QSTRING("∞")};
			else
				return {col_ptr->asStringColumn()->textAt(row)};
		}

		if (!col_ptr->isValid(row))
			return {QStringLiteral("-")};

		// m_formula_mode is not used at the moment
		// if (m_formula_mode)
		//	return QVariant(col_ptr->formula(row));

		return {col_ptr->asStringColumn()->textAt(row)};
	case Qt::ForegroundRole:
		if (!col_ptr->isValid(row))
			return {QBrush(Qt::red)};
		return color(col_ptr, row, AbstractColumn::Formatting::Foreground);
	case Qt::BackgroundRole:
		if (m_searchText.isEmpty())
			return color(col_ptr, row, AbstractColumn::Formatting::Background);
		else {
			if (col_ptr->asStringColumn()->textAt(row).indexOf(m_searchText) == -1)
				return color(col_ptr, row, AbstractColumn::Formatting::Background);
			else
				return {QApplication::palette().color(QPalette::Highlight)};
		}
	case static_cast<int>(CustomDataRole::MaskingRole):
		return {col_ptr->isMasked(row)};
	case static_cast<int>(CustomDataRole::FormulaRole):
		return {col_ptr->formula(row)};
	case Qt::DecorationRole:
		return color(col_ptr, row, AbstractColumn::Formatting::Icon);
		// 		if (m_formula_mode)
		// 			return QIcon(QPixmap(":/equals.png")); //TODO
	}

	return {};
}

QVariant SpreadsheetModel::headerData(int section, Qt::Orientation orientation, int role) const {
	if ((orientation == Qt::Horizontal && section > m_columnCount - 1) || (orientation == Qt::Vertical && section > m_rowCount - 1))
		return {};

	switch (orientation) {
	case Qt::Horizontal:
		switch (role) {
		case Qt::DisplayRole:
		case Qt::ToolTipRole:
		case Qt::EditRole:
			return m_horizontal_header_data.at(section);
		case Qt::DecorationRole:
			return m_spreadsheet->child<Column>(section)->icon();
		case static_cast<int>(CustomDataRole::CommentRole):
			// Return the comment associated with the column
			return m_spreadsheet->child<Column>(section)->comment();

		case static_cast<int>(CustomDataRole::SparkLineRole): {
			// Return the sparkline associated with the column
			return m_spreadsheet->child<Column>(section)->sparkline();
		}
		}
		break;
	case Qt::Vertical:
		switch (role) {
		case Qt::DisplayRole:
		case Qt::ToolTipRole:
			return section + 1;
		}
	}

	return {};
}

int SpreadsheetModel::rowCount(const QModelIndex& /*parent*/) const {
	return m_rowCount;
}

int SpreadsheetModel::columnCount(const QModelIndex& /*parent*/) const {
	return m_columnCount;
}

bool SpreadsheetModel::setData(const QModelIndex& index, const QVariant& value, int role) {
	if (!index.isValid())
		return false;

	int row = index.row();
	auto* column = m_spreadsheet->column(index.column());

	// DEBUG("SpreadsheetModel::setData() value = " << STDSTRING(value.toString()))

	// don't do anything if no new value was provided
	if (column->columnMode() == AbstractColumn::ColumnMode::Double) {
		bool ok;
		double new_value = QLocale().toDouble(value.toString(), &ok);
		if (ok) {
			if (column->valueAt(row) == new_value)
				return false;
		} else {
			// an empty (non-numeric value) was provided
			if (std::isnan(column->valueAt(row)))
				return false;
		}
	} else {
		if (column->asStringColumn()->textAt(row) == value.toString())
			return false;
	}

	switch (role) {
	case Qt::EditRole:
		// remark: the validity of the cell is determined by the input filter
		if (m_formula_mode)
			column->setFormula(row, value.toString());
		else
			column->asStringColumn()->setTextAt(row, value.toString());
		return true;
	case static_cast<int>(CustomDataRole::MaskingRole):
		m_spreadsheet->column(index.column())->setMasked(row, value.toBool());
		return true;
	case static_cast<int>(CustomDataRole::FormulaRole):
		m_spreadsheet->column(index.column())->setFormula(row, value.toString());
		return true;
	}

	return false;
}

QModelIndex SpreadsheetModel::index(int row, int column, const QModelIndex& /*parent*/) const {
	return createIndex(row, column);
}

QModelIndex SpreadsheetModel::parent(const QModelIndex& /*child*/) const {
	return QModelIndex{};
}

bool SpreadsheetModel::hasChildren(const QModelIndex& /*parent*/) const {
	return false;
}

void SpreadsheetModel::handleAspectsAboutToBeInserted(int first, int last) {
	if (m_suppressSignals)
		return;
	m_spreadsheetColumnCountChanging = true;
	beginInsertColumns(QModelIndex(), first, last);
}

void SpreadsheetModel::handleAspectAboutToBeAdded(const AbstractAspect* parent, int index, const AbstractAspect* aspect) {
	if (m_spreadsheetColumnCountChanging || m_suppressSignals)
		return;
	const Column* col = dynamic_cast<const Column*>(aspect);
	if (!col || parent != m_spreadsheet)
		return;
	beginInsertColumns(QModelIndex(), index, index);
}

void SpreadsheetModel::handleAspectsInserted(int first, int last) {
	const auto& children = m_spreadsheet->children<Column>();
	if (first < 0 || first >= children.count() || last >= children.count() || first > last)
		return;

	for (int i = first; i <= last; i++) {
		const auto* col = children.at(i);
		connect(col, &Column::plotDesignationChanged, this, &SpreadsheetModel::handlePlotDesignationChange);
		connect(col, &Column::modeChanged, this, &SpreadsheetModel::handleDataChange);
		connect(col, &Column::dataChanged, this, &SpreadsheetModel::handleDataChange);
		connect(col, &Column::formatChanged, this, &SpreadsheetModel::handleDataChange);
		connect(col, &Column::modeChanged, this, &SpreadsheetModel::handleModeChange);
		connect(col, &Column::maskingChanged, this, &SpreadsheetModel::handleDataChange);
		connect(col, &Column::formulaChanged, this, &SpreadsheetModel::handlePlotDesignationChange); // we can re-use the same slot to update the header here
		connect(col->outputFilter(), &AbstractSimpleFilter::digitsChanged, this, &SpreadsheetModel::handleDigitsChange);
	}

	handleAspectCountChanged();
	if (!m_suppressSignals)
		endInsertColumns();
	m_spreadsheetColumnCountChanging = false;
}

void SpreadsheetModel::handleAspectAdded(const AbstractAspect* aspect) {
	// PERFTRACE(Q_FUNC_INFO);
	if (m_spreadsheetColumnCountChanging)
		return;
	const Column* col = dynamic_cast<const Column*>(aspect);
	if (!col || aspect->parentAspect() != m_spreadsheet)
		return;
	int index = m_spreadsheet->indexOfChild<Column>(aspect);
	handleAspectsInserted(index, index);
}

void SpreadsheetModel::handleAspectsAboutToBeRemoved(int first, int last) {
	if (m_suppressSignals)
		return;

	const auto& children = m_spreadsheet->children<Column>();
	if (first < 0 || first >= children.count() || last >= children.count() || first > last)
		return;

	m_spreadsheetColumnCountChanging = true;

	beginRemoveColumns(QModelIndex(), first, last);
	for (int i = first; i <= last; i++)
		disconnect(children.at(i), nullptr, this, nullptr);
}

void SpreadsheetModel::handleAspectAboutToBeRemoved(const AbstractAspect* aspect) {
	if (m_suppressSignals || m_spreadsheetColumnCountChanging)
		return;

	const Column* col = dynamic_cast<const Column*>(aspect);
	if (!col || aspect->parentAspect() != m_spreadsheet)
		return;

	const int index = m_spreadsheet->indexOfChild<AbstractAspect>(aspect);
	beginRemoveColumns(QModelIndex(), index, index);
	disconnect(col, nullptr, this, nullptr);
}

void SpreadsheetModel::handleAspectsRemoved() {
	if (m_suppressSignals)
		return;
	handleAspectCountChanged();
	endRemoveColumns();
	m_spreadsheetColumnCountChanging = false;
}

void SpreadsheetModel::handleAspectRemoved(const AbstractAspect* parent, const AbstractAspect* /*before*/, const AbstractAspect* child) {
	// same conditions as in handleAspectAboutToBeRemoved()
	if (m_spreadsheetColumnCountChanging || child->type() != AspectType::Column || parent != m_spreadsheet)
		return;

	handleAspectsRemoved();
}

void SpreadsheetModel::handleAspectCountChanged() {
	if (m_suppressSignals)
		return;

	m_columnCount = m_spreadsheet->columnCount();
	updateHorizontalHeader(false);
}

void SpreadsheetModel::handleDescriptionChange(const AbstractAspect* aspect) {
	if (m_suppressSignals)
		return;

	const Column* col = dynamic_cast<const Column*>(aspect);
	if (!col || aspect->parentAspect() != m_spreadsheet)
		return;

	if (!m_suppressSignals) {
		updateHorizontalHeader(false);
		int index = m_spreadsheet->indexOfChild<Column>(col);
		Q_EMIT headerDataChanged(Qt::Horizontal, index, index);
	}
}

void SpreadsheetModel::handleModeChange(const AbstractColumn* col) {
	if (m_suppressSignals)
		return;

	updateHorizontalHeader(false);
	int index = m_spreadsheet->indexOfChild<Column>(col);
	Q_EMIT headerDataChanged(Qt::Horizontal, index, index);
	handleDataChange(col);

	// output filter was changed after the mode change, update the signal-slot connection
	disconnect(nullptr, SIGNAL(digitsChanged()), this, SLOT(handledigitsChange()));
	connect(static_cast<const Column*>(col)->outputFilter(), &AbstractSimpleFilter::digitsChanged, this, &SpreadsheetModel::handleDigitsChange);
}

void SpreadsheetModel::handleDigitsChange() {
	if (m_suppressSignals)
		return;

	const auto* filter = dynamic_cast<const Double2StringFilter*>(QObject::sender());
	if (!filter)
		return;

	const AbstractColumn* col = filter->output(0);
	handleDataChange(col);
}

void SpreadsheetModel::handlePlotDesignationChange(const AbstractColumn* col) {
	if (m_suppressSignals)
		return;

	updateHorizontalHeader(false);
	int index = m_spreadsheet->indexOfChild<Column>(col);
	Q_EMIT headerDataChanged(Qt::Horizontal, index, m_columnCount - 1);
}

void SpreadsheetModel::handleDataChange(const AbstractColumn* col) {
	if (m_suppressSignals)
		return;

	int i = m_spreadsheet->indexOfChild<Column>(col);
	Q_EMIT dataChanged(index(0, i), index(m_rowCount - 1, i));
}

void SpreadsheetModel::handleRowsAboutToBeInserted(int before, int last) {
	if (m_suppressSignals)
		return;
	beginInsertRows(QModelIndex(), before, last);
}

void SpreadsheetModel::handleRowsAboutToBeRemoved(int first, int last) {
	if (m_suppressSignals)
		return;
	beginRemoveRows(QModelIndex(), first, last);
}

void SpreadsheetModel::handleRowsInserted(int newRowCount) {
	handleRowCountChanged(newRowCount);
	if (m_suppressSignals)
		return;
	endInsertRows();
}

void SpreadsheetModel::handleRowsRemoved(int newRowCount) {
	handleRowCountChanged(newRowCount);
	if (m_suppressSignals)
		return;
	endRemoveRows();
}

void SpreadsheetModel::handleRowCountChanged(int newRowCount) {
	if (m_suppressSignals)
		return;
	m_rowCount = newRowCount;
	updateVerticalHeader();
}

void SpreadsheetModel::updateVerticalHeader() {
	m_verticalHeaderCount = m_rowCount;
}

void SpreadsheetModel::updateHorizontalHeader(bool sendSignal) {
	int column_count = m_spreadsheet->childCount<Column>();

	while (m_horizontal_header_data.size() < column_count)
		m_horizontal_header_data << QString();

	while (m_horizontal_header_data.size() > column_count)
		m_horizontal_header_data.removeLast();

	KConfigGroup group = Settings::group(QStringLiteral("Settings_Spreadsheet"));
	bool showColumnType = group.readEntry(QLatin1String("ShowColumnType"), true);
	bool showPlotDesignation = group.readEntry(QLatin1String("ShowPlotDesignation"), true);

	for (int i = 0; i < column_count; i++) {
		Column* col = m_spreadsheet->child<Column>(i);
		QString header;
		if (!col->formula().isEmpty() && col->formulaAutoUpdate())
			header += QLatin1String("*");
		header += col->name();

		if (showColumnType)
			header += QLatin1String(" {") + col->columnModeString() + QLatin1Char('}');

		if (showPlotDesignation) {
			if (col->plotDesignation() != AbstractColumn::PlotDesignation::NoDesignation)
				header += QLatin1String(" ") + col->plotDesignationString();
		}

		m_horizontal_header_data.replace(i, header);
	}

	if (sendSignal)
		Q_EMIT headerDataChanged(Qt::Horizontal, 0, column_count - 1);
}

Column* SpreadsheetModel::column(int index) {
	return m_spreadsheet->column(index);
}

void SpreadsheetModel::activateFormulaMode(bool on) {
	if (m_formula_mode == on)
		return;

	m_formula_mode = on;
	if (m_rowCount > 0 && m_columnCount > 0)
		Q_EMIT dataChanged(index(0, 0), index(m_rowCount - 1, m_columnCount - 1));
}

bool SpreadsheetModel::formulaModeActive() const {
	return m_formula_mode;
}

QVariant SpreadsheetModel::color(const AbstractColumn* column, int row, AbstractColumn::Formatting type) const {
	if (!column->hasHeatmapFormat() || (!column->isNumeric() && column->columnMode() != AbstractColumn::ColumnMode::Text) || !column->isValid(row))
		return {};

	const auto& format = column->heatmapFormat();
	if (format.type != type || format.colors.isEmpty())
		return {};

	int index = 0;
	if (column->isNumeric()) {
		double value = column->valueAt(row);
		double range = (format.max - format.min) / format.colors.count();

		if (value > format.max)
			index = format.colors.count() - 1;
		else {
			for (int i = 0; i < format.colors.count(); ++i) {
				if (value <= format.min + (i + 1) * range) {
					index = i;
					break;
				}
			}
		}
	} else {
		index = column->dictionaryIndex(row);
	}

	if (index < format.colors.count())
		return {QColor(format.colors.at(index))};
	else
		return {QColor(format.colors.constLast())};
}
