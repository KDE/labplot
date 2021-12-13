/*
    File                 : CustomPointDock.h
    Project              : LabPlot
    Description          : Dock widget for the custom point on the plot
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2015-2020 Alexander Semke <alexander.semke@web.de>
    SPDX-FileCopyrightText: 2021 Stefan Gerlach <stefan.gerlach@uni.kn>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef CUSTOMPOINTDOCK_H
#define CUSTOMPOINTDOCK_H

#include "backend/worksheet/plots/cartesian/Symbol.h"
#include "kdefrontend/dockwidgets/BaseDock.h"
#include "ui_custompointdock.h"

class AbstractAspect;
class CustomPoint;
class SymbolWidget;
class KConfig;

class CustomPointDock : public BaseDock {
	Q_OBJECT

public:
	explicit CustomPointDock(QWidget *);
	void setPoints(QList<CustomPoint*>);
	void updateLocale() override;

private:
	Ui::CustomPointDock ui;
	QList<CustomPoint*> m_points;
	CustomPoint* m_point{nullptr};
	SymbolWidget* symbolWidget{nullptr};

	void load();
	void loadConfig(KConfig&);

private slots:
	//SLOTs for changes triggered in CustomPointDock
	//General-Tab
	void positionXChanged();
	void positionXDateTimeChanged(const QDateTime&);
	void positionYChanged();
	void visibilityChanged(bool);

	//SLOTs for changes triggered in CustomPoint
	//General-Tab
	void pointPositionChanged(const WorksheetElement::PositionWrapper &);
	void updatePlotRanges() const override;
	void pointVisibilityChanged(bool);

	//load and save
	void loadConfigFromTemplate(KConfig&);
	void saveConfigAsTemplate(KConfig&);

signals:
	void info(const QString&);
};

#endif
