/***************************************************************************
    File                 : AddSubtractDialog.h
    Project              : LabPlot
    Description          : Dialog for adding/subtracting a value from column values
    --------------------------------------------------------------------
    Copyright            : (C) 2018 by Alexander Semke (alexander.semke@web.de)

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
#ifndef ADDSUBTRACTVALUEDIALOG_H
#define ADDSUBTRACTVALUEDIALOG_H

#include "ui_addsubtractvaluewidget.h"
#include <QDialog>

class Column;
class Spreadsheet;
class Matrix;
class QPushButton;

class AddSubtractValueDialog : public QDialog {
	Q_OBJECT

public:
	enum Operation {Add, Subtract, Multiply, Divide};

	explicit AddSubtractValueDialog(Spreadsheet*, Operation, QWidget* parent = nullptr);
	explicit AddSubtractValueDialog(Matrix*, Operation, QWidget* parent = nullptr);
	~AddSubtractValueDialog() override;
	void setColumns(QVector<Column*>);
	void setMatrices();

private:
	void init();
	void generateForColumns();
	void generateForMatrices();
	QString getMessage(QString);

	Ui::AddSubtractValueWidget ui;
	QVector<Column*> m_columns;
	Spreadsheet* m_spreadsheet = nullptr;
	Matrix* m_matrix = nullptr;
	QPushButton* m_okButton;
	Operation m_operation;

private slots:
	void generate();
};

#endif
