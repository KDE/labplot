/***************************************************************************
    File                 : GuiObserver.h
    Project              : LabPlot/SciDAVis
    Description 		: GUI observer
    --------------------------------------------------------------------
    Copyright		            : (C) 2010 Alexander Semke
    Email (use @ for *) 	: alexander.semke*web.de

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

#include <QModelIndex>
#include <QItemSelection>
class MainWin;
class AbstractAspect;

class GuiObserver:public QObject{
  Q_OBJECT
  
  public:
	GuiObserver(MainWin*);
	~GuiObserver();

  private:
	MainWin* mainWindow;
	void updateGui(const QString&);
	
  private slots:
	void selectedAspectsChanged(QList<AbstractAspect*>&);
};

#endif
