/***************************************************************************
    File                 : AxesDialog.h
    Project              : LabPlot
    --------------------------------------------------------------------
    Copyright            : (C) 2008 by Stefan Gerlach, Alexander Semke
    Email (use @ for *)  : stefan.gerlach*uni-konstanz.de, alexander.semke*web.de
    Description          : axes settings dialog
                           
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
#ifndef AXESDIALOG_H
#define AXESDIALOG_H

#include <KDialog>
#include "plots/Plot.h"

class AxesWidget;
class Axis;
class Worksheet;

/**
 * @brief Provides a dialog for editing the axis settings.
 */
class AxesDialog: public KDialog{
	Q_OBJECT

public:
	AxesDialog(QWidget*);
	~AxesDialog();

	void setWorksheet(Worksheet*);
	void setAxes(QList<Axis>* list_axes, const int axisNumber=0);

private:
	AxesWidget* axesWidget;
	Worksheet* worksheet;

private slots:
	void apply();
	void save();
};

#endif //AXESDIALOG_H
