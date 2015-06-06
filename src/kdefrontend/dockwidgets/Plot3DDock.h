/***************************************************************************
    File                 : Plot3DDock.h
    Project              : LabPlot
    Description          : widget for 3D plot properties
    --------------------------------------------------------------------
    Copyright            : (C) 2015 Minh Ngo (minh@fedoraproject.org)

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

#ifndef PLOT3DDOCK_H
#define PLOT3DDOCK_H

#include <QWidget>
#include "ui_plot3ddock.h"

class Plot3D;
class KUrl;
class AbstractColumn;

class Plot3DDock: public QWidget{
	Q_OBJECT

	public:
		explicit Plot3DDock(QWidget* parent);
		void setPlots(const QList<Plot3D*>& plots);

	private:
		void hideDataSource(bool hide = true);
		void hideFileUrl(bool hide = true);
		void hideCoordinates(bool hide = true);

		AbstractColumn* getColumn(const QModelIndex& index) const;

	private slots:
		void onVisualizationTypeChanged(int index);
		void onDataSourceChanged(int index);
		void onFileChanged(const KUrl& path);

		void onXCoordinateSourceChanged(const QModelIndex& index);
		void onYCoordinateSourceChanged(const QModelIndex& index);
		void onZCoordinatedSourceChanged(const QModelIndex& index);

	private:
		Ui::Plot3DDock ui;
		QList<Plot3D*> plots;

	signals:
		void needRepaint();
};

#endif
