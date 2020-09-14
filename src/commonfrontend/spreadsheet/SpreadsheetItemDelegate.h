/***************************************************************************
    File                 : SpreadsheetItemDelegate.h
    Project              : SciDAVis
    --------------------------------------------------------------------
    Copyright            : (C) 2007 by Tilman Benkert (thzs@gmx.net)
    Copyright            : (C) 2010-2017 by Alexander Semke (alexander.semke@web.de)

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
