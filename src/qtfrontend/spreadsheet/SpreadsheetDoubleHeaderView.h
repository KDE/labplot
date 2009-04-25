/***************************************************************************
    File                 : SpreadsheetDoubleHeaderView.h
    Project              : SciDAVis
    --------------------------------------------------------------------
    Copyright            : (C) 2007 by Tilman Benkert,
    Email (use @ for *)  : thzs*gmx.net
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

#ifndef TABLEDOUBLEHEADERVIEW_H
#define TABLEDOUBLEHEADERVIEW_H

#include <QHeaderView>
#include <spreadsheet/SpreadsheetModel.h>

//! Slave header for SpreadsheetDoubleHeaderView
/**
 * This class is only to be used by SpreadsheetDoubleHeaderView.
 * It allows for displaying two horizontal headers in a SpreadsheetView.
 * A SpreadsheetCommentsHeaderView displays the column comments
 * in a second header below the normal header. It is completely
 * controlled by a SpreadsheetDoubleHeaderView object and thus has
 * a master-slave relationship to it. This would be an inner class 
 * of SpreadsheetDoubleHeaderView if Qt allowed this.
 */
class SpreadsheetCommentsHeaderView : public QHeaderView
{
	Q_OBJECT

	public:
		SpreadsheetCommentsHeaderView(QWidget *parent = 0);
		virtual ~SpreadsheetCommentsHeaderView();

	virtual void setModel(QAbstractItemModel * model);

	friend class SpreadsheetDoubleHeaderView; // access to paintSection (protected)
};

//! Horizontal header for SpreadsheetView displaying comments in a second header
/*
 * This class is only to be used by SpreadsheetView.
 * It allows for displaying two horizontal headers.
 * A SpreadsheetDoubleHeaderView displays the column name, plot designation, and
 * type icon in a normal QHeaderView and below that a second header
 * which displays the column comments. 
 * 
 * \sa SpreadsheetCommentsHeaderView
 * \sa QHeaderView
 */
class SpreadsheetDoubleHeaderView : public QHeaderView
{
	Q_OBJECT

	private:
		SpreadsheetCommentsHeaderView * m_slave;

	public:
		SpreadsheetDoubleHeaderView(QWidget * parent = 0);
		~SpreadsheetDoubleHeaderView();

		virtual void setModel(QAbstractItemModel * model);
		virtual QSize sizeHint () const;
		//! Show or hide (if on = false) the column comments
		void showComments(bool on = true);
		//! Return whether comments are show currently
		bool areCommentsShown() const;

	public slots:
		// adjust geometry and repaint header 
		void refresh();
		// React to a header data change
		void headerDataChanged(Qt::Orientation orientation, int logicalFirst, int logicalLast);

	protected slots:
		void sectionsInserted(const QModelIndex & parent, int logicalFirst, int logicalLast);

	protected:
		virtual void paintSection(QPainter * painter, const QRect & rect, int logicalIndex) const;
	
		//! Flag: show/high column comments
		bool m_show_comments;
};




#endif // #ifndef TABLEDOUBLEHEADERVIEW_H
