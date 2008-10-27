/***************************************************************************
    File                 : ColumnDialoh.h
    Project              : LabPlot
    --------------------------------------------------------------------
    Copyright            : (C) 2008 by Stefan Gerlach
    Email (use @ for *)  : stefan.gerlach*uni-konstanz.de
    Description          : dialog for column properties
                           
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

#ifndef COLUMNDIALOG_H
#define COLUMNDIALOG_H

#include "ui_columndialog.h"
class Spreadsheet;

/**
 * @brief Provides a dialog for editing the properties of the current spreadsheet column.
 */
class ColumnDialog: public KDialog {
	Q_OBJECT
public:
	ColumnDialog(QWidget *parent, Spreadsheet *s);
private:
	Ui::ColumnDialog ui;
	Spreadsheet *s;
	void setupGUI();
private slots:
	void apply();
};

#endif // COLUMNDIALOG_H
