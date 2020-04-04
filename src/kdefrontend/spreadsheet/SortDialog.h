/***************************************************************************
    File                 : SortDialog.h
    Project              : LabPlot
    Description          : Sorting options dialog
    --------------------------------------------------------------------
    Copyright            : (C) 2011 by Alexander Semke (alexander.semke@web.de)

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

#include <ui_sortdialogwidget.h>
#include <QDialog>
class Column;

class SortDialog : public QDialog {
	Q_OBJECT

public:
	explicit SortDialog(QWidget* parent = nullptr);
	~SortDialog() override;

	void setColumns(const QVector<Column*>&);

	enum {Separately = 0, Together = 1};
	enum {Ascending = 0, Descending = 1};

private slots:
	void sortColumns();
	void changeType(int index);

signals:
	void sort(Column*, QVector<Column*>, bool ascending);

private:
	Ui::SortDialogWidget ui;
	QVector<Column*> m_columns;
};

#endif
