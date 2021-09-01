/*
    File                 : AddSubtractDialog.h
    Project              : LabPlot
    Description          : Dialog for adding/subtracting a value from column values
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2018 Alexander Semke (alexander.semke@web.de)

*/

/***************************************************************************
 *                                                                         *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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
	void setColumns(const QVector<Column*>&);
	void setMatrices();

private:
	void init();
	void generateForColumns();
	void generateForMatrices();
	QString getMessage(const QString&);

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
