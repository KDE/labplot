/***************************************************************************
    File                 : ColumnDock.h
    Project              : LabPlot
    --------------------------------------------------------------------
    Copyright            : (C) 2011 Alexander Semke
    Email (use @ for *)  : alexander.semke*web.de
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

#include "backend/core/column/Column.h"
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
	QList<Column*> m_columnsList;
	Column* m_column;
	bool m_initializing;
	QStringList dateStrings;
	QStringList timeStrings;

	void updateFormatWidgets(const AbstractColumn::ColumnMode);

  private slots:
	void retranslateUi();

	void nameChanged();
	void commentChanged();
	void typeChanged(int);
	void formatChanged(int);
	void precisionChanged(int);
	void plotDesignationChanged(int);

	//SLOTs for changes triggered in Column
	void columnDescriptionChanged(const AbstractAspect*);
	void columnFormatChanged();
	void columnPrecisionChanged();
	void columnPlotDesignationChanged(const AbstractColumn*);
signals:
	void info(const QString&);
};

#endif // COLUMNDOCK_H
