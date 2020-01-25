/***************************************************************************
    File                 : DropValuesDialog.h
    Project              : LabPlot
    Description          : Dialog for droping and masking values in columns
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
	void setColumns(QVector<Column*>);

private:
	Ui::DropValuesWidget ui;
	QVector<Column*> m_columns;
	Spreadsheet* m_spreadsheet;
	bool m_mask;

	void dropValues() const;
	void maskValues() const;

	QPushButton* m_okButton;
private slots:
	void operatorChanged(int) const;
	void okClicked() const;
};

#endif
