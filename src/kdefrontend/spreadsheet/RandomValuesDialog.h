/*
	File                 : RandomValuesDialog.h
	Project              : LabPlot
	Description          : Dialog for generating uniformly and non-uniformly distributed random numbers
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2014 Alexander Semke <alexander.semke@web.de>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef RANDOMVALUESDIALOG_H
#define RANDOMVALUESDIALOG_H

#include "ui_randomvalueswidget.h"
#include <QDialog>

class Column;
class Spreadsheet;
class QPushButton;

class RandomValuesDialog : public QDialog {
	Q_OBJECT

public:
	explicit RandomValuesDialog(Spreadsheet*, QWidget* parent = nullptr);
	~RandomValuesDialog() override;
	void setColumns(const QVector<Column*>&);

private:
	Ui::RandomValuesWidget ui;
	QVector<Column*> m_columns;
	Spreadsheet* m_spreadsheet;
	QPushButton* m_okButton;

private Q_SLOTS:
	void generate();
	void distributionChanged(int index = 0);
	void checkValues();
};

#endif
