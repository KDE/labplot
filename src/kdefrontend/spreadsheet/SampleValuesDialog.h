/*
	File                 : SampleValuesDialog.h
	Project              : LabPlot
	Description          : Dialog for sampling values in columns
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2015-2022 Alexander Semke <alexander.semke@web.de>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef SAMPLEVALUESDIALOG_H
#define SAMPLEVALUESDIALOG_H

#include "ui_samplevalueswidget.h"
#include <QDialog>

class Column;
class Spreadsheet;
class QPushButton;

class SampleValuesDialog : public QDialog {
	Q_OBJECT

public:
	explicit SampleValuesDialog(Spreadsheet* s, QWidget* parent = nullptr);
	~SampleValuesDialog() override;
	void setColumns(const QVector<Column*>&);

private:
	Ui::SampleValuesWidget ui;
	QVector<Column*> m_columns;
	Spreadsheet* m_spreadsheet;
	QPushButton* m_okButton;
	bool m_mask{false};
	bool m_hasNumeric{false};
	bool m_hasText{false};

private Q_SLOTS:
	void methodChanged(int) const;
	void sampleValues() const;
};

#endif
