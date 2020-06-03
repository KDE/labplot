/***************************************************************************
    File                 : ColumnDock.h
    Project              : LabPlot
    Description          : widget for column properties
    --------------------------------------------------------------------
    Copyright            : (C) 2011-2018 Alexander Semke (alexander.semke@web.de)
    Copyright            : (C) 2017 Stefan Gerlach (stefan.gerlach@uni.kn)

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
#include "kdefrontend/dockwidgets/BaseDock.h"
#include "ui_columndock.h"

template <class T> class QList;

class ColumnDock : public BaseDock {
	Q_OBJECT

public:
	explicit ColumnDock(QWidget*);
	void setColumns(QList<Column*>);

private:
	Ui::ColumnDock ui;
	QList<Column*> m_columnsList;
	Column* m_column{nullptr};

	void updateTypeWidgets(AbstractColumn::ColumnMode);

private slots:
	void retranslateUi();

	void typeChanged(int);
	void numericFormatChanged(int);
	void precisionChanged(int);
	void dateTimeFormatChanged(const QString&);
	void plotDesignationChanged(int);

	//SLOTs for changes triggered in Column
	void columnDescriptionChanged(const AbstractAspect*);
	void columnModeChanged(const AbstractAspect*);
	void columnFormatChanged();
	void columnPrecisionChanged();
	void columnPlotDesignationChanged(const AbstractColumn*);

signals:
	void info(const QString&);
};

#endif // COLUMNDOCK_H
