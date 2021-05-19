/***************************************************************************
    File                 : ReferenceLineDock.h
    Project              : LabPlot
    Description          : Dock widget for the reference line on the plot
    --------------------------------------------------------------------
    Copyright            : (C) 2020 Alexander Semke (alexander.semke@web.de)
    Copyright            : (C) 2021 Stefan Gerlach (stefan.gerlach@uni.kn)
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

#ifndef REFERENCELINEDOCK_H
#define REFERENCELINEDOCK_H

#include "kdefrontend/dockwidgets/BaseDock.h"
#include "backend/worksheet/plots/cartesian/ReferenceLine.h"
#include "ui_referencelinedock.h"

class AbstractAspect;
class ReferenceLine;
class KConfig;

class ReferenceLineDock : public BaseDock {
	Q_OBJECT

public:
	explicit ReferenceLineDock(QWidget *);
	void setReferenceLines(QList<ReferenceLine*>);
	void updateLocale() override;

private:
	Ui::ReferenceLineDock ui;
	QList<ReferenceLine*> m_linesList;
	ReferenceLine* m_line{nullptr};

	void load();
	void loadConfig(KConfig&);

private slots:
	//SLOTs for changes triggered in ReferenceLineDock
	void visibilityChanged(bool);

	//Position
	void orientationChanged(int);
	void positionChanged();

	//Line
	void styleChanged(int);
	void colorChanged(const QColor&);
	void widthChanged(double);
	void opacityChanged(int);

	//SLOTs for changes triggered in ReferenceLine
	void updatePlotRanges() const override;
	void lineVisibilityChanged(bool);

	//Position
	void linePositionChanged(double);
	void lineOrientationChanged(ReferenceLine::Orientation);

	//Line
	void linePenChanged(const QPen&);
	void lineOpacityChanged(qreal);
};

#endif
