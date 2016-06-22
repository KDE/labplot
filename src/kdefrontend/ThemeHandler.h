/***************************************************************************
    File                 : ThemeHandler.h
    Project              : LabPlot
    Description          : Widget for handling saving and loading of themes
    --------------------------------------------------------------------
	Copyright            : (C) 2012 by Stefan Gerlach (stefan.gerlach@uni-konstanz.de)
	Copyright            : (C) 2012-2013 by Alexander Semke (alexander.semke@web.de)

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

#ifndef THEMEHANDLER_H
#define THEMEHANDLER_H

#include <QtGui/QWidget>
#include <QPushButton>
#include "backend/worksheet/plots/cartesian/CartesianPlot.h"
class QHBoxLayout;
class QToolButton;
class QSpacerItem;
class QLabel;
class KConfig;

class ThemeHandler : public QWidget{
	Q_OBJECT

	public:
        ThemeHandler(QWidget* parent);

	private:
        QList<QString> dirNames;

        QHBoxLayout *horizontalLayout;
        QSpacerItem *horizontalSpacer;
        QSpacerItem *horizontalSpacer2;
        QLabel *lTheme;
        QPushButton *pbLoadTheme;
        void setThemePalette(KConfig& config);

    private slots:
        void loadMenu();
        void loadSelected(QAction*);

    signals:
        void loadThemeRequested(KConfig& config);
        void info(const QString&);
        void setThemePalette(QList<QColor> color);
};

#endif
