/***************************************************************************
    File                 : SortDialog.h
    Project              : SciDAVis
    --------------------------------------------------------------------
    Copyright            : (C) 2006 by Ion Vasilief, Tilman Benkert
    Email (use @ for *)  : ion_vasilief*yahoo.fr, thzs*gmx.net
    Description          : Sorting options dialog

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
#ifndef SORTDIALOG_H
#define SORTDIALOG_H

#include "core/column/Column.h"
#include <QDialog>

class QPushButton;
class QComboBox;

//! Sorting options dialog
class SortDialog : public QDialog
{
	Q_OBJECT

	public:
		SortDialog( QWidget* parent = 0, Qt::WFlags fl = 0 );
		void setColumnsList(QList<Column*> list);

		enum { Separately=0, Together=1 };
		enum { Ascending=0, Descending=1 };

	private slots:
		void accept();
		void changeType(int index);

	signals:
		void sort(Column *leading, QList<Column*> cols, bool ascending);

	private:
		QList<Column*> m_columns_list;
		struct {
		QPushButton* button_ok;
		QPushButton* button_cancel;
		QPushButton* button_help;
		QComboBox* box_type;
		QComboBox* box_order;
		QComboBox *columns_list;
		} ui;
};

#endif
