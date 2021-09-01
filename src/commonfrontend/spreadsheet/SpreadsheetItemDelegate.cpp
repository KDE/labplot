/*
    File                 : SpreadsheetItemDelegate.cpp
    Project              : LabPlot
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2007 Tilman Benkert (thzs@gmx.net)
    SPDX-FileCopyrightText: 2010-2020 Alexander Semke (alexander.semke@web.de)
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "SpreadsheetItemDelegate.h"
#include "backend/spreadsheet/SpreadsheetModel.h"

#include <QAbstractItemModel>
#include <QKeyEvent>
#include <QMetaProperty>
#include <QPainter>

/*!
\class SpreadsheetItemDelegate
\brief Item delegate for SpreadsheetView.

Overides QItemDelegate::paint() and provides shaded representation
of masked cells used in SpreadsheetView.

\ingroup commonfrontend
*/

SpreadsheetItemDelegate::SpreadsheetItemDelegate(QObject* parent) : QItemDelegate(parent) {
	installEventFilter(this);
}

void SpreadsheetItemDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option,
		const QModelIndex& index) const {

	QItemDelegate::paint(painter, option, index);
	if (!index.data(static_cast<int>(SpreadsheetModel::CustomDataRole::MaskingRole)).toBool())
		return;

	painter->save();
	painter->fillRect(option.rect, QBrush(m_maskingColor, Qt::BDiagPattern));
	painter->restore();
}

void SpreadsheetItemDelegate::setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index ) const {
	model->setData(index, editor->metaObject()->userProperty().read(editor), Qt::EditRole);
}

void SpreadsheetItemDelegate::setEditorData(QWidget* editor, const QModelIndex& index ) const {
	editor->metaObject()->userProperty().write(editor, index.data(Qt::EditRole));
}

bool SpreadsheetItemDelegate::eventFilter(QObject* editor, QEvent* event) {
	if (event->type() == QEvent::KeyPress) {
		auto* keyEvent = static_cast<QKeyEvent*>(event);
		if (keyEvent->key() == Qt::Key_Return || keyEvent->key() == Qt::Key_Enter) {
			emit commitData((QWidget*)editor);
			emit closeEditor((QWidget*)editor, QAbstractItemDelegate::NoHint);
			emit returnPressed();
			return true;
		}
	} else if (event->type() == QEvent::InputMethodQuery) {
		emit editorEntered();
		return true;
	}

	return QItemDelegate::eventFilter(editor, event);
}
