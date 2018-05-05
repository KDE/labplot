/***************************************************************************
    File                 : SpreadsheetHeaderView.h
    Project              : LabPlot
    --------------------------------------------------------------------
    Copyright            : (C) 2016 Alexander Semke (alexander.semke@web.de)
    Copyright            : (C) 2007 by Tilman Benkert (thzs@gmx.net)
    Description          : Horizontal header for SpreadsheetView displaying comments in a second header

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

#ifndef SPREADSHEETHEADERVIEW_H
#define SPREADSHEETHEADERVIEW_H

#include <QHeaderView>

class SpreadsheetCommentsHeaderView : public QHeaderView {
	Q_OBJECT

public:
	explicit SpreadsheetCommentsHeaderView(QWidget* parent = 0);
	virtual ~SpreadsheetCommentsHeaderView();

	virtual void setModel(QAbstractItemModel*);
	friend class SpreadsheetHeaderView; // access to paintSection (protected)
};

class SpreadsheetHeaderView : public QHeaderView {
	Q_OBJECT

public:
	explicit SpreadsheetHeaderView(QWidget* parent = 0);
	~SpreadsheetHeaderView();

	virtual void setModel(QAbstractItemModel*);
	virtual QSize sizeHint () const;

	void showComments(bool on = true);
	bool areCommentsShown() const;

private:
	SpreadsheetCommentsHeaderView* m_slave;
	bool m_showComments;

private slots:
	void refresh();
	void headerDataChanged(Qt::Orientation, int logicalFirst, int logicalLast);

protected:
	virtual void paintSection(QPainter*, QRect, int logicalIndex) const;
};

#endif
