/*
    File                 : ReferenceLineDock.h
    Project              : LabPlot
    Description          : Dock widget for the reference line on the plot
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2020 Alexander Semke <alexander.semke@web.de>
    SPDX-FileCopyrightText: 2021 Stefan Gerlach <stefan.gerlach@uni.kn>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

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

private Q_SLOTS:
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
	void linePositionChanged(const WorksheetElement::PositionWrapper&);
	void lineOrientationChanged(ReferenceLine::Orientation);

	//Line
	void linePenChanged(const QPen&);
	void lineOpacityChanged(qreal);
};

#endif
