/***************************************************************************
    File                 : SpreadsheetModel.cpp
    Project              : LabPlot
    Description          : Model for the access to a Spreadsheet
    --------------------------------------------------------------------
    Copyright            : (C) 2007 Tilman Benkert (thzs@gmx.net)
    Copyright            : (C) 2009 Knut Franke (knut.franke@gmx.de)
    Copyright            : (C) 2013-2017 Alexander Semke (alexander.semke@web.de)

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

#include "backend/spreadsheet/Spreadsheet.h"
#include "backend/spreadsheet/SpreadsheetModel.h"
#include "backend/core/datatypes/Double2StringFilter.h"

#include <QBrush>
#include <QIcon>

#include <KLocalizedString>
#include <cmath>

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
SpreadsheetModel::SpreadsheetModel(Spreadsheet* spreadsheet) : QAbstractItemModel(nullptr),
	m_spreadsheet(spreadsheet),
	m_rowCount(spreadsheet->rowCount()),
	m_columnCount(spreadsheet->columnCount()) {

	updateVerticalHeader();
	updateHorizontalHeader();

	connect(m_spreadsheet, &Spreadsheet::aspectAdded, this, &SpreadsheetModel::handleAspectAdded);
	connect(m_spreadsheet, &Spreadsheet::aspectAboutToBeRemoved, this, &SpreadsheetModel::handleAspectAboutToBeRemoved);
	connect(m_spreadsheet, &Spreadsheet::aspectRemoved, this, &SpreadsheetModel::handleAspectRemoved);
	connect(m_spreadsheet, &Spreadsheet::aspectDescriptionChanged, this, &SpreadsheetModel::handleDescriptionChange);

	for (int i = 0; i < spreadsheet->columnCount(); ++i) {
		beginInsertColumns(QModelIndex(), i, i);
		handleAspectAdded(spreadsheet->column(i));
	}

	m_spreadsheet->setModel(this);
}

void SpreadsheetModel::suppressSignals(bool value) {
	m_suppressSignals = value;

	//update the headers after all the data was added to the model
	//and we start listening to signals again
	if (!m_suppressSignals) {
		m_rowCount = m_spreadsheet->rowCount();
		m_columnCount = m_spreadsheet->columnCount();
		m_spreadsheet->emitColumnCountChanged();
		updateVerticalHeader();
		updateHorizontalHeader();
		beginResetModel();
		endResetModel();
	}
}

Qt::ItemFlags SpreadsheetModel::flags(const QModelIndex& index) const {
	if (index.isValid())
		return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable;
	else
		return Qt::ItemIsEnabled;
}

QVariant SpreadsheetModel::data(const QModelIndex& index, int role) const {
	if ( !index.isValid() )
		return QVariant();

	const int row = index.row();
	const int col = index.column();
	const Column* col_ptr = m_spreadsheet->column(col);

	if (!col_ptr)
		return QVariant();

	switch (role) {
	case Qt::ToolTipRole:
		if (col_ptr->isValid(row)) {
			if (col_ptr->isMasked(row))
				return QVariant(i18n("%1, masked (ignored in all operations)", col_ptr->asStringColumn()->textAt(row)));
			else
				return QVariant(col_ptr->asStringColumn()->textAt(row));
		} else {
			if (col_ptr->isMasked(row))
				return QVariant(i18n("invalid cell, masked (ignored in all operations)"));
			else
				return QVariant(i18n("invalid cell (ignored in all operations)"));
		}
	case Qt::EditRole:
		if (col_ptr->columnMode() == AbstractColumn::Numeric) {
			double value = col_ptr->valueAt(row);
			if (std::isnan(value))
				return QVariant("-");
			else if (std::isinf(value))
				return QVariant(QLatin1String("inf"));
			else
				return QVariant(col_ptr->asStringColumn()->textAt(row));
		}

		if (col_ptr->isValid(row))
			return QVariant(col_ptr->asStringColumn()->textAt(row));

		//m_formula_mode is not used at the moment
		//if (m_formula_mode)
		//	return QVariant(col_ptr->formula(row));

		break;
	case Qt::DisplayRole:
		if (col_ptr->columnMode() == AbstractColumn::Numeric) {
			double value = col_ptr->valueAt(row);
			if (std::isnan(value))
				return QVariant("-");
			else if (std::isinf(value))
				return QVariant(UTF8_QSTRING("∞"));
			else
				return QVariant(col_ptr->asStringColumn()->textAt(row));
		}

		if (!col_ptr->isValid(row))
			return QVariant("-");

		//m_formula_mode is not used at the moment
		//if (m_formula_mode)
		//	return QVariant(col_ptr->formula(row));

		return QVariant(col_ptr->asStringColumn()->textAt(row));
	case Qt::ForegroundRole:
		if (!col_ptr->isValid(row))
			return QVariant(QBrush(Qt::red));
		break;
	case MaskingRole:
		return QVariant(col_ptr->isMasked(row));
	case FormulaRole:
		return QVariant(col_ptr->formula(row));
// 	case Qt::DecorationRole:
// 		if (m_formula_mode)
// 			return QIcon(QPixmap(":/equals.png")); //TODO
	}

	return QVariant();
}

QVariant SpreadsheetModel::headerData(int section, Qt::Orientation orientation, int role) const {
	if ( (orientation == Qt::Horizontal && section > m_columnCount-1)
		|| (orientation == Qt::Vertical && section > m_rowCount-1) )
		return QVariant();

	switch (orientation) {
	case Qt::Horizontal:
		switch (role) {
		case Qt::DisplayRole:
		case Qt::ToolTipRole:
		case Qt::EditRole:
			return m_horizontal_header_data.at(section);
		case Qt::DecorationRole:
			return m_spreadsheet->child<Column>(section)->icon();
		case SpreadsheetModel::CommentRole:
			return m_spreadsheet->child<Column>(section)->comment();
		}
		break;
	case Qt::Vertical:
		switch (role) {
		case Qt::DisplayRole:
		case Qt::ToolTipRole:
			return m_vertical_header_data.at(section);
		}
	}

	return QVariant();
}

int SpreadsheetModel::rowCount(const QModelIndex& parent) const {
	Q_UNUSED(parent)
	return m_rowCount;
}

int SpreadsheetModel::columnCount(const QModelIndex& parent) const {
	Q_UNUSED(parent)
	return m_columnCount;
}

bool SpreadsheetModel::setData(const QModelIndex& index, const QVariant& value, int role) {
	if (!index.isValid())
		return false;

	int row = index.row();
	Column* column = m_spreadsheet->column(index.column());

	//don't do anything if no new value was provided
	if (column->columnMode() == AbstractColumn::Numeric) {
		bool ok;
		QLocale locale;
		double new_value = locale.toDouble(value.toString(), &ok);
		if (ok) {
			if (column->valueAt(row) == new_value )
				return false;
		} else {
			//an empty (non-numeric value) was provided
			if (std::isnan(column->valueAt(row)))
				return false;
		}
	} else {
		if (column->asStringColumn()->textAt(row) == value.toString())
			return false;
	}

	switch (role) {
	case Qt::EditRole: {
		// remark: the validity of the cell is determined by the input filter
		if (m_formula_mode)
			column->setFormula(row, value.toString());
		else
			column->asStringColumn()->setTextAt(row, value.toString());

		return true;
	}
	case MaskingRole: {
		m_spreadsheet->column(index.column())->setMasked(row, value.toBool());
		return true;
	}
	case FormulaRole: {
		m_spreadsheet->column(index.column())->setFormula(row, value.toString());
		return true;
	}
	}

	return false;
}

QModelIndex SpreadsheetModel::index(int row, int column, const QModelIndex& parent) const {
	Q_UNUSED(parent)
	return createIndex(row, column);
}

QModelIndex SpreadsheetModel::parent(const QModelIndex& child) const {
	Q_UNUSED(child)
	return QModelIndex{};
}

bool SpreadsheetModel::hasChildren(const QModelIndex& parent) const {
	Q_UNUSED(parent)
	return false;
}

void SpreadsheetModel::handleAspectAdded(const AbstractAspect* aspect) {
	const Column* col = dynamic_cast<const Column*>(aspect);
	if (!col || aspect->parentAspect() != m_spreadsheet)
		return;

	connect(col, &Column::plotDesignationChanged, this, &SpreadsheetModel::handlePlotDesignationChange);
	connect(col, &Column::modeChanged, this, &SpreadsheetModel::handleDataChange);
	connect(col, &Column::dataChanged, this, &SpreadsheetModel::handleDataChange);
	connect(col, &Column::formatChanged, this, &SpreadsheetModel::handleDataChange);
	connect(col, &Column::modeChanged, this, &SpreadsheetModel::handleModeChange);
	connect(col, &Column::rowsInserted, this, &SpreadsheetModel::handleRowsInserted);
	connect(col, &Column::rowsRemoved, this, &SpreadsheetModel::handleRowsRemoved);
	connect(col, &Column::maskingChanged, this, &SpreadsheetModel::handleDataChange);
	connect(col->outputFilter(), &AbstractSimpleFilter::digitsChanged, this, &SpreadsheetModel::handleDigitsChange);

	if (!m_suppressSignals) {
		beginResetModel();
		updateVerticalHeader();
		updateHorizontalHeader();
		endResetModel();

		m_columnCount = m_spreadsheet->columnCount();
		m_spreadsheet->emitColumnCountChanged();
		emit headerDataChanged(Qt::Horizontal, 0, m_columnCount-1);
	}
}

void SpreadsheetModel::handleAspectAboutToBeRemoved(const AbstractAspect* aspect) {
	if (m_suppressSignals)
		return;

	const Column* col = dynamic_cast<const Column*>(aspect);
	if (!col || aspect->parentAspect() != m_spreadsheet)
		return;

	beginResetModel();
	disconnect(col, nullptr, this, nullptr);
}

void SpreadsheetModel::handleAspectRemoved(const AbstractAspect* parent, const AbstractAspect* before, const AbstractAspect* child) {
	Q_UNUSED(before)
	const Column* col = dynamic_cast<const Column*>(child);
	if (!col || parent != m_spreadsheet)
		return;

	updateVerticalHeader();
	updateHorizontalHeader();

	m_columnCount = m_spreadsheet->columnCount();
	m_spreadsheet->emitColumnCountChanged();

	endResetModel();
}

void SpreadsheetModel::handleDescriptionChange(const AbstractAspect* aspect) {
	if (m_suppressSignals)
		return;

	const Column* col = dynamic_cast<const Column*>(aspect);
	if (!col || aspect->parentAspect() != m_spreadsheet)
		return;

	if (!m_suppressSignals) {
		updateHorizontalHeader();
		int index = m_spreadsheet->indexOfChild<Column>(col);
		emit headerDataChanged(Qt::Horizontal, index, index);
	}
}

void SpreadsheetModel::handleModeChange(const AbstractColumn* col) {
	if (m_suppressSignals)
		return;

	updateHorizontalHeader();
	int index = m_spreadsheet->indexOfChild<Column>(col);
	emit headerDataChanged(Qt::Horizontal, index, index);
	handleDataChange(col);

	//output filter was changed after the mode change, update the signal-slot connection
	disconnect(nullptr, SIGNAL(digitsChanged()), this, SLOT(handledigitsChange()));
	connect(dynamic_cast<const Column*>(col)->outputFilter(), &AbstractSimpleFilter::digitsChanged, this, &SpreadsheetModel::handleDigitsChange);
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

	updateHorizontalHeader();
	int index = m_spreadsheet->indexOfChild<Column>(col);
	emit headerDataChanged(Qt::Horizontal, index, m_columnCount-1);
}

void SpreadsheetModel::handleDataChange(const AbstractColumn* col) {
	if (m_suppressSignals)
		return;

	int i = m_spreadsheet->indexOfChild<Column>(col);
	emit dataChanged(index(0, i), index(m_rowCount-1, i));
}

void SpreadsheetModel::handleRowsInserted(const AbstractColumn* col, int before, int count) {
	if (m_suppressSignals)
		return;

	Q_UNUSED(before) Q_UNUSED(count)
	updateVerticalHeader();
	int i = m_spreadsheet->indexOfChild<Column>(col);
	m_rowCount = col->rowCount();
	emit dataChanged(index(0, i), index(m_rowCount-1, i));
	m_spreadsheet->emitRowCountChanged();
}

void SpreadsheetModel::handleRowsRemoved(const AbstractColumn* col, int first, int count) {
	if (m_suppressSignals)
		return;

	Q_UNUSED(first) Q_UNUSED(count)
	updateVerticalHeader();
	int i = m_spreadsheet->indexOfChild<Column>(col);
	m_rowCount = col->rowCount();
	emit dataChanged(index(0, i), index(m_rowCount-1, i));
	m_spreadsheet->emitRowCountChanged();
}

void SpreadsheetModel::updateVerticalHeader() {
	int old_rows = m_vertical_header_data.size();
	int new_rows = m_rowCount;

	if (new_rows > old_rows) {
		beginInsertRows(QModelIndex(), old_rows, new_rows-1);

		for (int i = old_rows+1; i <= new_rows; i++)
			m_vertical_header_data << i;

		endInsertRows();
	} else if (new_rows < old_rows) {
		beginRemoveRows(QModelIndex(), new_rows, old_rows-1);

		while (m_vertical_header_data.size() > new_rows)
			m_vertical_header_data.removeLast();

		endRemoveRows();
	}
}

void SpreadsheetModel::updateHorizontalHeader() {
	int column_count = m_spreadsheet->childCount<Column>();

	while (m_horizontal_header_data.size() < column_count)
		m_horizontal_header_data << QString();

	while (m_horizontal_header_data.size() > column_count)
		m_horizontal_header_data.removeLast();

	for (int i = 0; i < column_count; i++) {
		Column* col = m_spreadsheet->child<Column>(i);

		QString type;
		switch (col->columnMode()) {
		case AbstractColumn::Numeric:
			type = QLatin1String(" {") + i18n("Numeric") + QLatin1Char('}');
			break;
		case AbstractColumn::Integer:
			type = QLatin1String(" {") + i18n("Integer") + QLatin1Char('}');
			break;
		case AbstractColumn::BigInt:
			type = QLatin1String(" {") + i18n("Big Integer") + QLatin1Char('}');
			break;
		case AbstractColumn::Text:
			type = QLatin1String(" {") + i18n("Text") + QLatin1Char('}');
			break;
		case AbstractColumn::Month:
			type = QLatin1String(" {") + i18n("Month Names") + QLatin1Char('}');
			break;
		case AbstractColumn::Day:
			type = QLatin1String(" {") + i18n("Day Names") + QLatin1Char('}');
			break;
		case AbstractColumn::DateTime:
			type = QLatin1String(" {") + i18n("Date and Time") + QLatin1Char('}');
			break;
		}

		QString designation;
		switch (col->plotDesignation()) {
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
		m_horizontal_header_data.replace(i, col->name() + type + designation);
	}
}

Column* SpreadsheetModel::column(int index) {
	return m_spreadsheet->column(index);
}

void SpreadsheetModel::activateFormulaMode(bool on) {
	if (m_formula_mode == on) return;

	m_formula_mode = on;
	if (m_rowCount > 0 && m_columnCount > 0)
		emit dataChanged(index(0,0), index(m_rowCount - 1, m_columnCount - 1));
}

bool SpreadsheetModel::formulaModeActive() const {
	return m_formula_mode;
}
