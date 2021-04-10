/***************************************************************************
    File                 : FormattingHeatmapDialog.h
    Project              : LabPlot
    Description          : Dialog for the conditional formatting according to a heatmap
    --------------------------------------------------------------------
    Copyright            : (C) 2021 by Alexander Semke (alexander.semke@web.de)

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
