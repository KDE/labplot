/***************************************************************************
    File                 : CustomPointDock.h
    Project              : LabPlot
    Description          : Dock widget for the custom point on the plot
    --------------------------------------------------------------------
    Copyright            : (C) 2015 Alexander Semke (alexander.semke@web.de)
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

#ifndef CUSTOMPOINTDOCK_H
#define CUSTOMPOINTDOCK_H

#include "backend/worksheet/plots/cartesian/Symbol.h"
#include "kdefrontend/dockwidgets/BaseDock.h"
#include "ui_custompointdock.h"

class AbstractAspect;
class CustomPoint;
class KConfig;

class CustomPointDock : public BaseDock {
	Q_OBJECT

public:
	explicit CustomPointDock(QWidget *);
	void setPoints(QList<CustomPoint*>);
	void updateLocale() override;

private:
	Ui::CustomPointDock ui;
	QList<CustomPoint*> m_pointsList;
	CustomPoint* m_point{nullptr};

	void load();
	void loadConfig(KConfig&);

private slots:
	void init();

	//SLOTs for changes triggered in CustomPointDock
	//General-Tab
	void positionXChanged();
	void positionYChanged();
	void visibilityChanged(bool);

	//Symbol-tab
	void symbolStyleChanged(int);
	void symbolSizeChanged(double);
	void symbolRotationChanged(int);
	void symbolOpacityChanged(int);
	void symbolFillingStyleChanged(int);
	void symbolFillingColorChanged(const QColor&);
	void symbolBorderStyleChanged(int);
	void symbolBorderColorChanged(const QColor&);
	void symbolBorderWidthChanged(double);

	//SLOTs for changes triggered in CustomPoint
	//General-Tab
	void pointDescriptionChanged(const AbstractAspect*);
	void pointPositionChanged(QPointF);
	void pointVisibilityChanged(bool);

	//Symbol-Tab
	void pointSymbolStyleChanged(Symbol::Style);
	void pointSymbolSizeChanged(qreal);
	void pointSymbolRotationAngleChanged(qreal);
	void pointSymbolOpacityChanged(qreal);
	void pointSymbolBrushChanged(const QBrush&);
	void pointSymbolPenChanged(const QPen&);

	//load and save
	void loadConfigFromTemplate(KConfig&);
	void saveConfigAsTemplate(KConfig&);

signals:
	void info(const QString&);
};

#endif
