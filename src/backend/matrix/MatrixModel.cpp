/*
	File                 : MatrixModel.cpp
	Project              : LabPlot
	Description          : Matrix data model
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2015-2016 Alexander Semke <alexander.semke@web.de>
	SPDX-FileCopyrightText: 2008-2009 Tilman Benkert <thzs@gmx.net>
	SPDX-FileCopyrightText: 2018-2020 Stefan Gerlach <stefan.gerlach@uni.kn>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "backend/matrix/MatrixModel.h"
#include "backend/matrix/Matrix.h"

#include <QBrush>

/*!
  \class MatrixModel
  \brief Model for the access to data of a Matrix-object.

	This is a model in the sense of Qt4 model/view framework which is used
	to access a Matrix object from any of Qt4s view classes, typically a QMatrixView.
	Its main purposes are translating Matrix signals into QAbstractItemModel signals
	and translating calls to the QAbstractItemModel read/write API into calls
	in the public API of Matrix.

	\ingroup backend
*/
MatrixModel::MatrixModel(Matrix* matrix)
	: QAbstractItemModel(nullptr)
	, m_matrix(matrix) {
	connect(m_matrix, &Matrix::columnsAboutToBeInserted, this, &MatrixModel::handleColumnsAboutToBeInserted);
	connect(m_matrix, &Matrix::columnsInserted, this, &MatrixModel::handleColumnsInserted);
	connect(m_matrix, &Matrix::columnsAboutToBeRemoved, this, &MatrixModel::handleColumnsAboutToBeRemoved);
	connect(m_matrix, &Matrix::columnsRemoved, this, &MatrixModel::handleColumnsRemoved);
	connect(m_matrix, &Matrix::rowsAboutToBeInserted, this, &MatrixModel::handleRowsAboutToBeInserted);
	connect(m_matrix, &Matrix::rowsInserted, this, &MatrixModel::handleRowsInserted);
	connect(m_matrix, &Matrix::rowsAboutToBeRemoved, this, &MatrixModel::handleRowsAboutToBeRemoved);
	connect(m_matrix, &Matrix::rowsRemoved, this, &MatrixModel::handleRowsRemoved);
	connect(m_matrix, &Matrix::dataChanged, this, &MatrixModel::handleDataChanged);
	connect(m_matrix, &Matrix::coordinatesChanged, this, &MatrixModel::handleCoordinatesChanged);
	connect(m_matrix, &Matrix::numericFormatChanged, this, &MatrixModel::handleFormatChanged);
	connect(m_matrix, &Matrix::precisionChanged, this, &MatrixModel::handleFormatChanged);
}

void MatrixModel::setSuppressDataChangedSignal(bool b) {
	m_suppressDataChangedSignal = b;
}

void MatrixModel::setChanged() {
	Q_EMIT changed();
}

Qt::ItemFlags MatrixModel::flags(const QModelIndex& index) const {
	if (index.isValid())
		return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable;
	else
		return Qt::ItemIsEnabled;
}

QVariant MatrixModel::data(const QModelIndex& index, int role) const {
	if (!index.isValid())
		return {};

	int row = index.row();
	int col = index.column();

	switch (role) {
	case Qt::ToolTipRole:
	case Qt::EditRole:
	case Qt::DisplayRole: {
		auto mode = m_matrix->mode();
		// DEBUG(Q_FUNC_INFO << ", DisplayRole, mode = " << mode);
		switch (mode) {
		case AbstractColumn::ColumnMode::Double:
			return {m_matrix->text<double>(row, col)};
		case AbstractColumn::ColumnMode::Integer:
			return {m_matrix->text<int>(row, col)};
		case AbstractColumn::ColumnMode::BigInt:
			return {m_matrix->text<qint64>(row, col)};
		case AbstractColumn::ColumnMode::DateTime:
		case AbstractColumn::ColumnMode::Month:
		case AbstractColumn::ColumnMode::Day:
			return {m_matrix->text<QDateTime>(row, col)};
		case AbstractColumn::ColumnMode::Text: // should not happen
			return {m_matrix->text<QString>(row, col)};
		}
		break;
	}
	case Qt::BackgroundRole:
		// use bluish background color to distinguish Matrix from Spreadsheet
		return {QColor(192, 255, 255)};
	case Qt::ForegroundRole:
		// ignore current theme settings and always use black foreground color so Matrix is usable with dark themes, too.
		return {QColor(Qt::black)};
	}

	return {};
}

QVariant MatrixModel::headerData(int section, Qt::Orientation orientation, int role) const {
	QString result;
	auto headerFormat = m_matrix->headerFormat();
	const auto numberLocale = QLocale();
	switch (orientation) {
	case Qt::Horizontal:
		switch (role) {
		case Qt::DisplayRole:
		case Qt::ToolTipRole:
			if (headerFormat == Matrix::HeaderFormat::HeaderRowsColumns) {
				result = QString::number(section + 1);
			} else if (headerFormat == Matrix::HeaderFormat::HeaderValues) {
				double diff = m_matrix->xEnd() - m_matrix->xStart();
				double step = 0.0;
				if (m_matrix->columnCount() > 1)
					step = diff / double(m_matrix->columnCount() - 1);
				result = numberLocale.toString(m_matrix->xStart() + double(section) * step, m_matrix->numericFormat(), m_matrix->precision());
			} else {
				result = QString::number(section + 1) + QLatin1String(" (");
				double diff = m_matrix->xEnd() - m_matrix->xStart();
				double step = 0.0;
				if (m_matrix->columnCount() > 1)
					step = diff / double(m_matrix->columnCount() - 1);
				result += numberLocale.toString(m_matrix->xStart() + double(section) * step, m_matrix->numericFormat(), m_matrix->precision());

				result += QLatin1Char(')');
			}
			return {result};
		}
		break;
	case Qt::Vertical:
		switch (role) {
		case Qt::DisplayRole:
		case Qt::ToolTipRole:
			if (headerFormat == Matrix::HeaderFormat::HeaderRowsColumns) {
				result = QString::number(section + 1);
			} else if (headerFormat == Matrix::HeaderFormat::HeaderValues) {
				double diff = m_matrix->yEnd() - m_matrix->yStart();
				double step = 0.0;
				if (m_matrix->rowCount() > 1)
					step = diff / double(m_matrix->rowCount() - 1);
				// TODO: implement decent double == 0 check
				// 						if (diff < 1e-10)
				// 							result += numberLocale.toString(m_matrix->yStart(),
				// 									m_matrix->numericFormat(), m_matrix->displayedDigits());
				result += numberLocale.toString(m_matrix->yStart() + double(section) * step, m_matrix->numericFormat(), m_matrix->precision());
			} else {
				result = QString::number(section + 1) + QStringLiteral(" (");
				double diff = m_matrix->yEnd() - m_matrix->yStart();
				double step = 0.0;
				if (m_matrix->rowCount() > 1)
					step = diff / double(m_matrix->rowCount() - 1);

				result += numberLocale.toString(m_matrix->yStart() + double(section) * step, m_matrix->numericFormat(), m_matrix->precision());
				result += QLatin1Char(')');
			}
			return {result};
		}
	}
	return {};
}

int MatrixModel::rowCount(const QModelIndex& /*parent*/) const {
	return m_matrix->rowCount();
}

int MatrixModel::columnCount(const QModelIndex& /*parent*/) const {
	return m_matrix->columnCount();
}

bool MatrixModel::setData(const QModelIndex& index, const QVariant& value, int role) {
	if (!index.isValid())
		return false;

	int row = index.row();
	int column = index.column();

	if (role == Qt::EditRole) {
		const auto mode = m_matrix->mode();
		switch (mode) {
		case AbstractColumn::ColumnMode::Double:
			m_matrix->setCell(row, column, value.toDouble());
			break;
		case AbstractColumn::ColumnMode::Integer:
			m_matrix->setCell(row, column, value.toInt());
			break;
		case AbstractColumn::ColumnMode::BigInt:
			m_matrix->setCell(row, column, value.toLongLong());
			break;
		case AbstractColumn::ColumnMode::DateTime:
		case AbstractColumn::ColumnMode::Month:
		case AbstractColumn::ColumnMode::Day:
			DEBUG("	WARNING: DateTime format not supported yet"); // should not happen
			// TODO: m_matrix->setCell(row, column, value.toDateTime());
			break;
		case AbstractColumn::ColumnMode::Text:
			DEBUG("	WARNING: Text format not supported yet"); // should not happen
			m_matrix->setCell(row, column, value.toString());
			break;
		}

		if (!m_suppressDataChangedSignal)
			Q_EMIT changed();
		return true;
	}
	return false;
}

QModelIndex MatrixModel::index(int row, int column, const QModelIndex& /*parent*/) const {
	return createIndex(row, column);
}

QModelIndex MatrixModel::parent(const QModelIndex& /*child*/) const {
	return QModelIndex{};
}

void MatrixModel::updateHeader() {
	Q_EMIT headerDataChanged(Qt::Horizontal, 0, m_matrix->columnCount());
	Q_EMIT headerDataChanged(Qt::Vertical, 0, m_matrix->rowCount());
}

void MatrixModel::handleColumnsAboutToBeInserted(int before, int count) {
	beginInsertColumns(QModelIndex(), before, before + count - 1);
}

void MatrixModel::handleColumnsInserted(int /*first*/, int /*count*/) {
	endInsertColumns();
	if (!m_suppressDataChangedSignal)
		Q_EMIT changed();
}

void MatrixModel::handleColumnsAboutToBeRemoved(int first, int count) {
	beginRemoveColumns(QModelIndex(), first, first + count - 1);
}

void MatrixModel::handleColumnsRemoved(int /*first*/, int /*count*/) {
	endRemoveColumns();
	if (!m_suppressDataChangedSignal)
		Q_EMIT changed();
}

void MatrixModel::handleRowsAboutToBeInserted(int before, int count) {
	beginInsertRows(QModelIndex(), before, before + count - 1);
}

void MatrixModel::handleRowsInserted(int /*first*/, int /*count*/) {
	endInsertRows();
	if (!m_suppressDataChangedSignal)
		Q_EMIT changed();
}

void MatrixModel::handleRowsAboutToBeRemoved(int first, int count) {
	beginRemoveRows(QModelIndex(), first, first + count - 1);
}

void MatrixModel::handleRowsRemoved(int /*first*/, int /*count*/) {
	endRemoveRows();
	if (!m_suppressDataChangedSignal)
		Q_EMIT changed();
}

void MatrixModel::handleDataChanged(int top, int left, int bottom, int right) {
	Q_EMIT dataChanged(index(top, left), index(bottom, right));
	if (!m_suppressDataChangedSignal)
		Q_EMIT changed();
}

void MatrixModel::handleCoordinatesChanged() {
	Q_EMIT headerDataChanged(Qt::Horizontal, 0, columnCount() - 1);
	Q_EMIT headerDataChanged(Qt::Vertical, 0, rowCount() - 1);
}

void MatrixModel::handleFormatChanged() {
	handleCoordinatesChanged();
	handleDataChanged(0, 0, rowCount() - 1, columnCount() - 1);
}
