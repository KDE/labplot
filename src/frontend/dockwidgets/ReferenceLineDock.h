/*
	File                 : ReferenceLineDock.h
	Project              : LabPlot
	Description          : Dock widget for the reference line on the plot
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2020-2025 Alexander Semke <alexander.semke@web.de>
	SPDX-FileCopyrightText: 2021 Stefan Gerlach <stefan.gerlach@uni.kn>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef REFERENCELINEDOCK_H
#define REFERENCELINEDOCK_H

#include "backend/worksheet/plots/cartesian/ReferenceLine.h"
#include "frontend/dockwidgets/BaseDock.h"
#include "ui_referencelinedock.h"

class LineWidget;
class ReferenceLine;
class KConfig;

class ReferenceLineDock : public BaseDock {
	Q_OBJECT

public:
	explicit ReferenceLineDock(QWidget*);
	void setReferenceLines(QList<ReferenceLine*>);
	void updateLocale() override;
	void retranslateUi() override;

private:
	Ui::ReferenceLineDock ui;
	QList<ReferenceLine*> m_linesList;
	ReferenceLine* m_line{nullptr};
	LineWidget* lineWidget{nullptr};

	void load();
	void updateWidgetsOrientation(ReferenceLine::Orientation);

private Q_SLOTS:
	// SLOTs for changes triggered in ReferenceLineDock
	void lockChanged(bool);
	void orientationChanged(int);
	void positionLogicalChanged(double);
	void positionLogicalDateTimeChanged(qint64);

	// SLOTs for changes triggered in ReferenceLine
	void lineLockChanged(bool);
	void linePositionLogicalChanged(const QPointF&);
	void lineOrientationChanged(ReferenceLine::Orientation);

	// load and save
	void loadConfigFromTemplate(KConfig&);
	void saveConfigAsTemplate(KConfig&);

Q_SIGNALS:
	void info(const QString&);
};

#endif
