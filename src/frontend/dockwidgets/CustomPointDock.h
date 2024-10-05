/*
	File                 : CustomPointDock.h
	Project              : LabPlot
	Description          : Dock widget for the custom point on the plot
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2015-2023 Alexander Semke <alexander.semke@web.de>
	SPDX-FileCopyrightText: 2021 Stefan Gerlach <stefan.gerlach@uni.kn>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef CUSTOMPOINTDOCK_H
#define CUSTOMPOINTDOCK_H

#include "frontend/dockwidgets/BaseDock.h"
#include "ui_custompointdock.h"

class CustomPoint;
class SymbolWidget;
class KConfig;

class CustomPointDock : public BaseDock {
	Q_OBJECT

public:
	explicit CustomPointDock(QWidget*);
	void setPoints(QList<CustomPoint*>);
	void updateLocale() override;

private:
	Ui::CustomPointDock ui;
	QList<CustomPoint*> m_points;
	CustomPoint* m_point{nullptr};
	SymbolWidget* symbolWidget{nullptr};

	void load();
	void initConnections() const;

private Q_SLOTS:
	// SLOTs for changes triggered in CustomPointDock
	// General-Tab
	void positionXChanged(int);
	void positionYChanged(int);
	void customPositionXChanged(double);
	void customPositionYChanged(double);
	void lockChanged(bool);
	void bindingChanged(bool checked);

	void positionXLogicalChanged(double);
	void positionXLogicalDateTimeChanged(qint64);
	void positionYLogicalChanged(double);
	void positionYLogicalDateTimeChanged(qint64);

	// SLOTs for changes triggered in CustomPoint
	// General-Tab
	void pointLockChanged(bool);
	void pointPositionChanged(const WorksheetElement::PositionWrapper&);
	void pointPositionLogicalChanged(QPointF);
	void pointCoordinateBindingEnabledChanged(bool);

	// load and save
	void loadConfigFromTemplate(KConfig&);
	void saveConfigAsTemplate(KConfig&);

Q_SIGNALS:
	void info(const QString&);

	friend class WorksheetElementTest;
};

#endif
