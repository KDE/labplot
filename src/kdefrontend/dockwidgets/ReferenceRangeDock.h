/*
	File                 : ReferenceRangeDock.h
	Project              : LabPlot
	Description          : Dock widget for the reference range on the plot
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2022 Alexander Semke <alexander.semke@web.de>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef REFERENCERANGEDOCK_H
#define REFERENCERANGEDOCK_H

#include "backend/worksheet/plots/cartesian/ReferenceRange.h"
#include "kdefrontend/dockwidgets/BaseDock.h"
#include "ui_referencerangedock.h"

class AbstractAspect;
class BackgroundWidget;
class LineWidget;
class ReferenceRange;
class KConfig;

class ReferenceRangeDock : public BaseDock {
	Q_OBJECT

public:
	explicit ReferenceRangeDock(QWidget*);
	void setReferenceRanges(QList<ReferenceRange*>);
	void updateLocale() override;

private:
	Ui::ReferenceRangeDock ui;
	QList<ReferenceRange*> m_rangeList;
	ReferenceRange* m_range{nullptr};
	BackgroundWidget* backgroundWidget{nullptr};
	LineWidget* lineWidget{nullptr};

	void load();
	void loadConfig(KConfig&);

private Q_SLOTS:
	// SLOTs for changes triggered in ReferenceRangeDock
	void visibilityChanged(bool);

	// Position
	void orientationChanged(int);
	void positionLogicalStartChanged(double);
	void positionLogicalEndChanged(double);
	void positionLogicalDateTimeStartChanged(qint64);
	void positionLogicalDateTimeEndChanged(qint64);

	// SLOTs for changes triggered in ReferenceRange
	void updatePlotRanges() override;
	void rangeVisibilityChanged(bool);

	// Position
	void rangePositionLogicalStartChanged(const QPointF&);
	void rangePositionLogicalEndChanged(const QPointF&);
	void rangeOrientationChanged(ReferenceRange::Orientation);
};

#endif
