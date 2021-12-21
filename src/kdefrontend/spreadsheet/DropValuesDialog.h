/*
    File                 : DropValuesDialog.h
    Project              : LabPlot
    Description          : Dialog for droping and masking values in columns
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2015 Alexander Semke <alexander.semke@web.de>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef DROPVALUESDIALOG_H
#define DROPVALUESDIALOG_H

#include "ui_dropvalueswidget.h"
#include <QDialog>

class Column;
class Spreadsheet;
class QPushButton;

class DropValuesDialog : public QDialog {
	Q_OBJECT

public:
	explicit DropValuesDialog(Spreadsheet* s, bool mask = false, QWidget* parent = nullptr);
	~DropValuesDialog() override;
	void setColumns(const QVector<Column*>&);

private:
	Ui::DropValuesWidget ui;
	QVector<Column*> m_columns;
	Spreadsheet* m_spreadsheet;
	bool m_mask;

	void dropValues() const;
	void maskValues() const;

	QPushButton* m_okButton;
private Q_SLOTS:
	void operatorChanged(int) const;
	void okClicked() const;
};

#endif
