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

	void load();
	void loadConfig(KConfig&);

private Q_SLOTS:
	// SLOTs for changes triggered in ReferenceRangeDock
	void visibilityChanged(bool);

	// Position
	void orientationChanged(int);
	void positionLogicalStartChanged(const QString&);
	void positionLogicalEndChanged(const QString&);
	void positionLogicalDateTimeStartChanged(const QDateTime&);
	void positionLogicalDateTimeEndChanged(const QDateTime&);

	// Background
	void backgroundTypeChanged(int);
	void backgroundColorStyleChanged(int);
	void backgroundImageStyleChanged(int);
	void backgroundBrushStyleChanged(int);
	void backgroundFirstColorChanged(const QColor&);
	void backgroundSecondColorChanged(const QColor&);
	void selectFile();
	void fileNameChanged();
	void backgroundOpacityChanged(int);

	// Border
	void borderStyleChanged(int);
	void borderColorChanged(const QColor&);
	void borderWidthChanged(double);
	void borderOpacityChanged(int);

	// SLOTs for changes triggered in ReferenceRange
	void updatePlotRanges() override;
	void rangeVisibilityChanged(bool);

	// Position
	void rangePositionLogicalStartChanged(const QPointF&);
	void rangePositionLogicalEndChanged(const QPointF&);
	void rangeOrientationChanged(ReferenceRange::Orientation);

	// Background
	void rangeBackgroundTypeChanged(WorksheetElement::BackgroundType);
	void rangeBackgroundColorStyleChanged(WorksheetElement::BackgroundColorStyle);
	void rangeBackgroundImageStyleChanged(WorksheetElement::BackgroundImageStyle);
	void rangeBackgroundBrushStyleChanged(Qt::BrushStyle);
	void rangeBackgroundFirstColorChanged(QColor&);
	void rangeBackgroundSecondColorChanged(QColor&);
	void rangeBackgroundFileNameChanged(QString&);
	void rangeBackgroundOpacityChanged(float);

	// Border
	void rangeBorderPenChanged(QPen&);
	void rangeBorderOpacityChanged(double);
};

#endif
