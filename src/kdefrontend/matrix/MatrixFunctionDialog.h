/***************************************************************************
    File                 : MatrixFunctionDialog.h
    Project              : LabPlot
    Description          : Dialog for generating matrix values from a mathematical function
    --------------------------------------------------------------------
    Copyright            : (C) 2015 by Alexander Semke (alexander.semke@web.de)

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
#ifndef MATRIXFUNCTIONDIALOG_H
#define MATRIXFUNCTIONDIALOG_H

#include "ui_matrixfunctionwidget.h"
#include <QDialog>

class Matrix;
class QPushButton;
class MatrixFunctionDialog : public QDialog {
	Q_OBJECT

public:
	explicit MatrixFunctionDialog(Matrix*, QWidget* parent = nullptr);

private:
	Ui::MatrixFunctionWidget ui;
	Matrix* m_matrix;
	QPushButton* m_okButton;
private slots:
	void generate();
	void checkValues();
	void showConstants();
	void showFunctions();
	void insertFunction(const QString&);
	void insertConstant(const QString&);
};

#endif
