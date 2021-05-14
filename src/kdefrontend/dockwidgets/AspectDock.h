/***************************************************************************
    File                 : AspectDock.h
    Project              : LabPlot
    Description          : widget for aspect properties showing name and comments only
    --------------------------------------------------------------------
    Copyright            : (C) 2021 Alexander Semke (alexander.semke@web.de)

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

#ifndef ASPECTDOCK_H
#define ASPECTDOCK_H

#include "kdefrontend/dockwidgets/BaseDock.h"
#include "ui_aspectdock.h"

class AbstractAspect;
template <class T> class QList;

class AspectDock : public BaseDock {
	Q_OBJECT

public:
	explicit AspectDock(QWidget*);
	void setAspects(QList<AbstractAspect*>);

private:
	Ui::AspectDock ui;

private slots:
	void aspectDescriptionChanged(const AbstractAspect*);
};

#endif // ASPECT_H
