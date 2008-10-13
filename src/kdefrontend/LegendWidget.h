/***************************************************************************
    File                 : LegendWidget.h
    Project              : LabPlot
    --------------------------------------------------------------------
    Copyright            : (C) 2008 by Alexander Semke
    Email (use @ for *)  : alexander.semke*web.de
    Description          : legend settings widget
                           
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
#ifndef LEGENDWIDGET_H
#define LEGENDWIDGET_H

#include <QtGui>
#include "../ui_legendwidget.h"

class Legend;

/**
 * @brief Represents a widget where all the legend settings can be modified
 * This widget is embedded in \c LegendDialog
 */
class LegendWidget : public QWidget{
    Q_OBJECT

public:
	LegendWidget(QWidget*);
	~LegendWidget();

	void setLegend(const Legend*);
	void saveLegend(Legend*) const;

private:
	Ui::LegendWidget ui;
	Legend* legend;
	bool initializing;

signals:
	void dataChanged(bool);

private slots:
	void fillingChanged(bool);
	void slotDataChanged();
};

#endif
