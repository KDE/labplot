/*
    File                 : SpreadsheetItemDelegate.h
    Project              : SciDAVis
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2007 Tilman Benkert (thzs@gmx.net)
    SPDX-FileCopyrightText: 2010-2017 Alexander Semke (alexander.semke@web.de)

*/

/***************************************************************************
 *                                                                         *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *                                                                         *
 ***************************************************************************/

#ifndef SPREADSHEETITEMDELEGATE_H
#define SPREADSHEETITEMDELEGATE_H

#include <QItemDelegate>
class QAbstractItemModel;

class SpreadsheetItemDelegate : public QItemDelegate {
	Q_OBJECT

public:
	explicit SpreadsheetItemDelegate(QObject* parent = nullptr);

	void paint(QPainter*, const QStyleOptionViewItem&, const QModelIndex&) const override;

	void setEditorData(QWidget*, const QModelIndex&) const override;
	void setModelData(QWidget*, QAbstractItemModel*, const QModelIndex&) const override;

private:
	QColor m_maskingColor{0xff,0,0};
	bool eventFilter(QObject*, QEvent*) override;

signals:
	void returnPressed();
	void editorEntered();
};

#endif
