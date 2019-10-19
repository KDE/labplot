/***************************************************************************
    File                 : GuiTools.h
    Project              : LabPlot
    --------------------------------------------------------------------
    Copyright            : (C) 2011 Alexander Semke (alexander.semke*web.de)
                           (replace * with @ in the email addresses)
    Description          :  contains several static functions which are used on frequently throughout the kde frontend.

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

#ifndef GUITOOLS_H
#define GUITOOLS_H

#include <QPen>

class QComboBox;
class QColor;
class QMenu;
class QActionGroup;
class QAction;

class GuiTools {
public:
	static void updateBrushStyles(QComboBox*, const QColor&);
	static void updatePenStyles(QComboBox*, const QColor&);
	static void updatePenStyles(QMenu*, QActionGroup*, const QColor&);
	static void selectPenStyleAction(QActionGroup*, Qt::PenStyle);
	static Qt::PenStyle penStyleFromAction(QActionGroup*, QAction*);

	static void fillColorMenu(QMenu*, QActionGroup*);
	static void selectColorAction(QActionGroup*, const QColor&);
	static QColor& colorFromAction(QActionGroup*, QAction*);
};

#endif // GUITOOLS_H
