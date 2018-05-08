/***************************************************************************
    File                 : FitParametersWidget.h
    Project              : LabPlot
    Description          : widget for editing the fit parameters
    --------------------------------------------------------------------
    Copyright            : (C) 2014-2016 by Alexander Semke (alexander.semke@web.de)
    Copyright            : (C) 2016 by Stefan Gerlach (stefan.gerlach@uni-konstanz.de)

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
#ifndef FITPARAMETERSWIDGET_H
#define FITPARAMETERSWIDGET_H

#include <QWidget>

#include "backend/worksheet/plots/cartesian/XYFitCurve.h"
#include "ui_fitparameterswidget.h"

class FitParametersWidget : public QWidget {
	Q_OBJECT

public:
	explicit FitParametersWidget(QWidget*, XYFitCurve::FitData*);

private:
	Ui::FitParametersWidget ui;
	XYFitCurve::FitData* m_fitData;
	bool m_changed;
	bool m_rehighlighting;
	bool m_invalidRanges;

	bool eventFilter(QObject*, QEvent*);
	void highlightInvalid(int row, int col, bool invalid) const;

signals:
	void finished();
	void parametersChanged();

private slots:
	void applyClicked();
	void startValueChanged();
	void lowerLimitChanged();
	void upperLimitChanged();
	void addParameter();
	void removeParameter();
	void changed();
};

#endif //FITPARAMETERSWIDGET_H
