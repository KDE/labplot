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

#include <QPushButton>
#include <QVBoxLayout>
#include <QtGui/QWidget>
class QHBoxLayout;
class QToolButton;
class QSpacerItem;
class QLabel;
class KConfig;

class ThemeHandler : public QWidget{
	Q_OBJECT

	public:
		enum ClassName {Spreadsheet, Matrix, Worksheet, CartesianPlot, CartesianPlotLegend, XYCurve, Axis, CustomPoint};

		ThemeHandler(QWidget* parent, ClassName);

	private:
        void retranslateUi();

        ClassName className;
        QList<QString> dirNames;

        QHBoxLayout *horizontalLayout;
        QSpacerItem *horizontalSpacer;
        QToolButton *tbLoad;
        QToolButton *tbSave;
        QToolButton *tbSaveDefault;
        QSpacerItem *horizontalSpacer2;
        QToolButton *tbCopy;
        QToolButton *tbPaste;
        QLabel *lTheme;

    private slots:
        void loadMenu();
        void saveMenu();
        void loadMenuSelected(QAction*);
        void saveMenuSelected(QAction*);
        void saveNewSelected(const QString&);
        void saveDefaults();

    signals:
        void loadConfigRequested(KConfig&);
        void saveConfigRequested(KConfig&);
        void info(const QString&);
};

#endif
