/*
	File                 : FitOptionsWidget.h
	Project              : LabPlot
	Description          : widget for editing advanced fit parameters
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2014-2020 Alexander Semke <alexander.semke@web.de>
	SPDX-FileCopyrightText: 2017-2018 Stefan Gerlach <stefan.gerlach@uni.kn>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef FITOPTIONSWIDGET_H
#define FITOPTIONSWIDGET_H

#include "backend/worksheet/plots/cartesian/XYFitCurve.h"
#include "ui_fitoptionswidget.h"

class FitOptionsWidget : public QWidget {
	Q_OBJECT

public:
	explicit FitOptionsWidget(QWidget*, XYFitCurve::FitData*, XYFitCurve*);

private:
	Ui::FitOptionsWidget ui;
	XYFitCurve::FitData* m_fitData;
	XYFitCurve* m_fitCurve;
	bool m_changed{false};
	bool m_dateTimeRange{false};
	const QLocale locale;

Q_SIGNALS:
	void finished();
	void optionsChanged();

private Q_SLOTS:
	void autoRangeChanged();
	void autoEvalRangeChanged();
	void fitRangeMinChanged();
	void fitRangeMaxChanged();
	void fitRangeMinDateTimeChanged(qint64);
	void fitRangeMaxDateTimeChanged(qint64);
	void evalRangeMinChanged();
	void evalRangeMaxChanged();
	void evalRangeMinDateTimeChanged(qint64);
	void evalRangeMaxDateTimeChanged(qint64);
	void applyClicked();
	void changed();
};

#endif // FITOPTIONSWIDGET_H
