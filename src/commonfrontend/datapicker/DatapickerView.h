/***************************************************************************
    File                 : DatapickerView.h
    Project              : LabPlot
    Description          : View class for Datapicker
    --------------------------------------------------------------------
    Copyright            : (C) 2015 by Ankit Wagadre (wagadre.ankit@gmail.com)

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

#ifndef DATAPICKERVIEW_H
#define DATAPICKERVIEW_H

#include <QWidget>

class AbstractAspect;
class Datapicker;
class QAction;
class QMenu;
class QPrinter;
class QToolBar;
class TabWidget;

class DatapickerView : public QWidget {
    Q_OBJECT

    public:
        explicit DatapickerView(Datapicker*);
        virtual ~DatapickerView();

        int currentIndex() const;

    private:
        TabWidget* m_tabWidget;
        Datapicker* m_datapicker;
        int lastSelectedIndex;
        bool m_initializing;

    private  slots:
        void showTabContextMenu(const QPoint&);
        void itemSelected(int);
        void tabChanged(int);
        void tabMoved(int,int);
        void handleDescriptionChanged(const AbstractAspect*);
        void handleAspectAdded(const AbstractAspect*);
        void handleAspectAboutToBeRemoved(const AbstractAspect*);

};

#endif
