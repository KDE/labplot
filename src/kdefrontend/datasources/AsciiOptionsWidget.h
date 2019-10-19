/***************************************************************************
    File                 : AsciiOptionsWidget.h
    Project              : LabPlot
    Description          : widget providing options for the import of ascii data
    --------------------------------------------------------------------
    Copyright            : (C) 2009-2017 by Stefan Gerlach (stefan.gerlach@uni.kn)
    Copyright            : (C) 2009-2017 by Alexander Semke (alexander.semke@web.de)

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
#ifndef ASCIIOPTIONSWIDGET_H
#define ASCIIOPTIONSWIDGET_H

#include "ui_asciioptionswidget.h"

class AsciiFilter;

class AsciiOptionsWidget : public QWidget {
	Q_OBJECT

public:
	explicit AsciiOptionsWidget(QWidget*);
	void showAsciiHeaderOptions(bool);
	void showTimestampOptions(bool);
	void applyFilterSettings(AsciiFilter*) const;
	void setSeparatingCharacter(QLatin1Char);
	void loadSettings() const;
	void saveSettings();

public slots:
	void headerChanged(int state);

private:
	Ui::AsciiOptionsWidget ui;
};

#endif
