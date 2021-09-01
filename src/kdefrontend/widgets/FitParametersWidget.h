/*
    File                 : FitParametersWidget.h
    Project              : LabPlot
    Description          : widget for editing the fit parameters
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2014-2016 Alexander Semke (alexander.semke@web.de)
    SPDX-FileCopyrightText: 2016-2018 Stefan Gerlach (stefan.gerlach@uni-konstanz.de)

*/

/***************************************************************************
 *                                                                         *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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
	explicit FitParametersWidget(QWidget*);
	void setFitData(XYFitCurve::FitData*);

private:
	Ui::FitParametersWidget ui;
	XYFitCurve::FitData* m_fitData{nullptr};
	bool m_initializing{false};
	bool m_rehighlighting{false};
	bool m_invalidRanges{false};
	bool m_resizing{false};

	bool eventFilter(QObject*, QEvent*) override;
	void resizeEvent(QResizeEvent*) override;

	void highlightInvalid(int row, int col, bool invalid);
	void updateTableSize();

signals:
	void parametersChanged(bool);
	void parametersValid(bool);

private slots:
	void changed();
	void apply();
	void startValueChanged();
	void lowerLimitChanged();
	void upperLimitChanged();
};

#endif //FITPARAMETERSWIDGET_H
