/*
	File                 : EquidistantValuesDialog.h
	Project              : LabPlot
	Description          : Dialog for generating equidistant values
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2014-2023 Alexander Semke <alexander.semke@web.de>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

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
	enum class Type { FixedNumber, FixedIncrement };
	enum class DateTimeUnit { Year, Month, Day, Hour, Minute, Second, Millisecond };

	explicit EquidistantValuesDialog(Spreadsheet*, QWidget* parent = nullptr);
	~EquidistantValuesDialog() override;
	void setColumns(const QVector<Column*>&);

private:
	Ui::EquidistantValuesWidget ui;
	QVector<Column*> m_columns;
	Spreadsheet* m_spreadsheet;
	QPushButton* m_okButton;
	bool m_hasNumeric{false};
	bool m_hasDateTime{false};

	bool generateNumericData(QVector<double>&);
	bool generateDateTimeData(QVector<QDateTime>&);

private Q_SLOTS:
	void generate();
	void typeChanged(int);
	void checkValues();
};

#endif
