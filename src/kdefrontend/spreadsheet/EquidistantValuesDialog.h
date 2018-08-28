/***************************************************************************
    File                 : EquidistantValuesDialog.h
    Project              : LabPlot
    Description          : Dialog for generating equidistant values
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
#ifndef EQUIDISTANTVALUESDIALOG_H
#define EQUIDISTANTVALUESDIALOG_H

#include "ui_equidistantvalueswidget.h"
#include <QDialog>

class Column;
class Spreadsheet;
class QPushButton;

class EquidistantValuesDialog : public QDialog {
Q_OBJECT

public:
	explicit EquidistantValuesDialog(Spreadsheet* s, QWidget* parent = nullptr);
	void setColumns(const QVector<Column*>&);

private:
	Ui::EquidistantValuesWidget ui;
	QVector<Column*> m_columns;
	Spreadsheet* m_spreadsheet;
	QPushButton* m_okButton;

private slots:
	void generate();
	void typeChanged(int index);
	void checkValues();
};

#endif
