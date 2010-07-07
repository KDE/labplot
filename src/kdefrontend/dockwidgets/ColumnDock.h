/***************************************************************************
    File                 : ColumnDock.h
    Project              : LabPlot
    --------------------------------------------------------------------
    Copyright            : (C) 2008 by Stefan Gerlach
    Email (use @ for *)  : stefan.gerlach*uni-konstanz.de
    Description          : widget for column properties
                           
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

#ifndef COLUMNDOCK_H
#define COLUMNDOCK_H

#include <QList>
#include "ui_columndock.h"
class Column;

class ColumnDock: public QWidget{
	Q_OBJECT
	
  public:
	  ColumnDock(QWidget *parent);
	  void setColumns(QList<Column*>);
	  
  private:
	  Ui::ColumnDock ui;
	  QList<Column*> m_columnList;
	  
  public slots:
	  void columnTypeChanged(int);
};

#endif // COLUMNDOCK_H
