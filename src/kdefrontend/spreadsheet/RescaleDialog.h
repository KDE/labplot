/***************************************************************************
    File                 : RescaleDialog.h
    Project              : LabPlot
    Description          : Dialog to provide the rescale interval
    --------------------------------------------------------------------
    Copyright            : (C) 2020 by Alexander Semke (alexander.semke@web.de)

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
#ifndef RESCALEDIALOG_H
#define RESCALEDIALOG_H

#include <ui_rescalewidget.h>
#include <QDialog>
class Column;

class RescaleDialog : public QDialog {
	Q_OBJECT

public:
	explicit RescaleDialog(QWidget* parent = nullptr);
	~RescaleDialog() override;

	void setColumns(const QVector<Column*>&);
	double min() const;
	double max() const;

private:
	Ui::RescaleWidget ui;

private slots:
	void validateOkButton();
};

#endif
