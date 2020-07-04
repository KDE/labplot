/***************************************************************************
    File                 : FitOptionsWidget.h
    Project              : LabPlot
    Description          : widget for editing advanced fit parameters
    --------------------------------------------------------------------
    Copyright            : (C) 2014-2020 by Alexander Semke (alexander.semke@web.de)
    Copyright            : (C) 2017-2018 by Stefan Gerlach (stefan.gerlach@uni.kn)

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
#ifndef FITOPTIONSWIDGET_H
#define FITOPTIONSWIDGET_H

#include "backend/worksheet/plots/cartesian/XYFitCurve.h"
#include "ui_fitoptionswidget.h"

class FitOptionsWidget: public QWidget {
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

signals:
	void finished();
	void optionsChanged();

private slots:
	void autoRangeChanged();
	void autoEvalRangeChanged();
	void fitRangeMinChanged();
	void fitRangeMaxChanged();
	void fitRangeMinDateTimeChanged(const QDateTime&);
	void fitRangeMaxDateTimeChanged(const QDateTime&);
	void evalRangeMinChanged();
	void evalRangeMaxChanged();
	void evalRangeMinDateTimeChanged(const QDateTime&);
	void evalRangeMaxDateTimeChanged(const QDateTime&);
	void applyClicked();
	void changed();
};

#endif //FITOPTIONSWIDGET_H
