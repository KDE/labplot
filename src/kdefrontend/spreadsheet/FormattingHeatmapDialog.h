/*
    File                 : FormattingHeatmapDialog.h
    Project              : LabPlot
    Description          : Dialog for the conditional formatting according to a heatmap
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2021 Alexander Semke <alexander.semke@web.de>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef FORMATTINGHEATMAPDIALOG_H
#define FORMATTINGHEATMAPDIALOG_H

#include "ui_formattingheatmapwidget.h"
#include "backend/core/AbstractColumn.h"
#include <QDialog>

class Column;
class Spreadsheet;
class QPushButton;

class FormattingHeatmapDialog : public QDialog {
	Q_OBJECT

public:
	explicit FormattingHeatmapDialog(Spreadsheet*, QWidget* parent = nullptr);
	~FormattingHeatmapDialog() override;
	void setColumns(const QVector<Column*>&);
	AbstractColumn::HeatmapFormat format();

private:
	Ui::FormattingHeatmapWidget ui;
	QVector<Column*> m_columns;
	Spreadsheet* m_spreadsheet;
	QPushButton* m_okButton;
	QString m_name;
	QVector<QColor> m_colors;

private slots:
	void autoRangeChanged(int);
	void selectColorMap();
	void checkValues();
};

#endif
