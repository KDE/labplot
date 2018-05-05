/***************************************************************************
    File                 : GuiObserver.h
    Project              : LabPlot
    Description          : GUI observer
    --------------------------------------------------------------------
    Copyright            : (C) 2010-2016 Alexander Semke (alexander.semke@web.de)

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
#ifndef GUIOBSERVER_H
#define GUIOBSERVER_H

#include <QObject>

class MainWin;
class AbstractAspect;

class GuiObserver : public QObject {
	Q_OBJECT

public:
	explicit GuiObserver(MainWin*);

private:
    MainWin* m_mainWindow;

private slots:
	void selectedAspectsChanged(QList<AbstractAspect*>&) const;
	void hiddenAspectSelected(const AbstractAspect*) const;
};

#endif
