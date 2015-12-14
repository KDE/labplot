/***************************************************************************
    File                 : DatapickerPointWidget.h
    Project              : LabPlot
    Description          : widget for Datapicker-Point properties
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

#ifndef DATAPICKERPOINTWIDGET_H
#define DATAPICKERPOINTWIDGET_H

#include "ui_datapickerpointwidget.h"
#include "backend/datapicker/DatapickerPoint.h"

class KConfig;

class DatapickerPointWidget: public QWidget {
	Q_OBJECT

public:
	explicit DatapickerPointWidget(QWidget *);

	void setDatapickerPoints(QList<DatapickerPoint*>);
	void load();

	void hidePositionWidgets();

private:
	Ui::DatapickerPointWidget ui;
	DatapickerPoint *m_point;
	QList<DatapickerPoint*> m_pointsList;
	bool m_initializing;

	void init();
	void initConnections();

signals:
	void dataChanged(bool);

private slots:
	void positionXChanged(int);
	void positionYChanged(int);
	void customPositionXChanged(double);
	void customPositionYChanged(double);
	void styleChanged(int);
	void sizeChanged(double);
	void rotationChanged(int);
	void opacityChanged(int);
	void fillingStyleChanged(int);
	void fillingColorChanged(const QColor&);
	void borderStyleChanged(int);
	void borderColorChanged(const QColor&);
	void borderWidthChanged(double);
	void visibilityChanged(bool);
	void errorBarFillingStyleChanged(int);
	void errorBarFillingColorChanged(const QColor&);
	void errorBarSizeChanged(double);

	void pointPositionChanged(const DatapickerPoint::PositionWrapper&);
	void pointStyleChanged(DatapickerPoint::PointsStyle);
	void pointSizeChanged(qreal);
	void pointRotationAngleChanged(qreal);
	void pointOpacityChanged(qreal);
	void pointBrushChanged(QBrush);
	void pointPenChanged(const QPen&);
	void pointVisibleChanged(bool);
	void pointErrorBarSizeChanged(qreal);
	void pointErrorBarBrushChanged(QBrush);

	void loadConfigFromTemplate(KConfig&);
	void loadConfig(KConfig&);
	void saveConfig(KConfig&);
};

#endif
