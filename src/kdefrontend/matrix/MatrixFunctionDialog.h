/*
    File                 : MatrixFunctionDialog.h
    Project              : LabPlot
    Description          : Dialog for generating matrix values from a mathematical function
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2015-2019 Alexander Semke <alexander.semke@web.de>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

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
	~MatrixFunctionDialog();

private:
	Ui::MatrixFunctionWidget ui;
	Matrix* m_matrix;
	QPushButton* m_okButton;

private Q_SLOTS:
	void generate();
	void checkValues();
	void showConstants();
	void showFunctions();
	void insertFunction(const QString&) const;
	void insertConstant(const QString&) const;
};

#endif
