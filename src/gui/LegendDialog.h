/***************************************************************************
    File                 : LegendDialog.h
    Project              : LabPlot
    --------------------------------------------------------------------
    Copyright            : (C) 2008 by Alexander Semke
    Email (use @ for *)  : alexander.semke*web.de
    Description          : legend settings dialog
                           
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
#ifndef LEGENDDIALOG_H
#define LEGENDDIALOG_H

#include <KDialog>

class Legend;
class LegendWidget;
class Worksheet;

/**
 * @brief Provides a dialog for editing the legend settings.
 */
class LegendDialog: public KDialog{
	Q_OBJECT

public:
	LegendDialog(QWidget*);
	void setWorksheet(Worksheet*);

private:
	LegendWidget* legendWidget;
	Worksheet* worksheet;

private slots:
	void apply();
	void save();
};

#endif //LEGENDDIALOG_H
