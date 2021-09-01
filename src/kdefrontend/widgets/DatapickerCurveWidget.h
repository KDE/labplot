/*
    File                 : ImageWidget.h
    Project              : LabPlot
    Description          : widget for datapicker properties
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2015 Ankit Wagadre <wagadre.ankit@gmail.com>
    SPDX-FileCopyrightText: 2015-2021 Alexander Semke <alexander.semke@web.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/


#ifndef DATAPICKERCURVEWIDGET_H
#define DATAPICKERCURVEWIDGET_H

#include "ui_datapickercurvewidget.h"
#include "backend/datapicker/DatapickerCurve.h"
#include "kdefrontend/dockwidgets/BaseDock.h"

class SymbolWidget;

class DatapickerCurveWidget : public BaseDock {
	Q_OBJECT

public:
	explicit DatapickerCurveWidget(QWidget*);
	~DatapickerCurveWidget() override;

	void setCurves(QList<DatapickerCurve*>);
	void load();
	void updateLocale() override;

private:
	Ui::DatapickerCurveWidget ui;
	void hideErrorBarWidgets(bool);

	DatapickerCurve* m_curve{nullptr};
	QList<DatapickerCurve*> m_curveList;
	SymbolWidget* symbolWidget{nullptr};
	bool m_suppressTypeChange{false};

private slots:
	//SLOTs for changes triggered in DatapickerCurveDock
	void updateSymbolWidgets();
	void xErrorTypeChanged(int);
	void yErrorTypeChanged(int);
	void visibilityChanged(bool);
	void errorBarFillingStyleChanged(int);
	void errorBarFillingColorChanged(const QColor&);
	void errorBarSizeChanged(double);

	//SLOTs for changes triggered in DatapickerCurve
	void curveErrorsChanged(DatapickerCurve::Errors);
	void symbolVisibleChanged(bool);
	void symbolErrorBarSizeChanged(qreal);
	void symbolErrorBarBrushChanged(const QBrush&);
};

#endif // DATAPICKERCURVEWIDGET_H
