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
	bool m_hasDouble{false};
	bool m_hasInteger{false};
	bool m_hasBigInteger{false};
	bool m_hasDateTime{false};

	bool generateDouble(QVector<double>&, double start, double increment, int number);
	bool generateInt(QVector<int>&, int start, int increment, int number);
	bool generateBigInt(QVector<qint64>&, int start, int increment, int number);
	bool generateDateTime(QVector<QDateTime>&, Type, const QDateTime& start, const QDateTime& end, int number, int increment, DateTimeUnit);

private Q_SLOTS:
	void generate();
	void typeChanged(int);
	void checkValues();
};

#endif
