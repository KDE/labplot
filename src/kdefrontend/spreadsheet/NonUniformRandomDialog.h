/***************************************************************************
    File                 : NonUniformRandomDialog.h
    Project              : LabPlot
    Description          : Dialog for generating non-uniformly distributed random numbers
    --------------------------------------------------------------------
    Copyright            : (C) 2014 by Alexander Semke (alexander.semke@web.de)

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
#ifndef NONUNIFORMRANDOMDIALOG_H
#define NONUNIFORMRANDOMDIALOG_H

#include "ui_nonuniformrandomwidget.h"
#include "backend/core/column/Column.h"
#include <KDialog>

class Spreadsheet;

class NonUniformRandomDialog : public KDialog{
	Q_OBJECT

	public:
		explicit NonUniformRandomDialog(Spreadsheet* s, QWidget* parent = 0, Qt::WFlags fl = 0);
		void setColumns(QList<Column*>);

	private:
		Ui::NonUniformRandomWidget ui;
		QList<Column*> m_columns;
		Spreadsheet* m_spreadsheet;

	private slots:
		void generate();
		void distributionChanged(int index);
};

#endif
