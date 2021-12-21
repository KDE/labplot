/*
    File             : XYFourierFilterCurveDock.h
    Project          : LabPlot
    Description      : widget for editing properties of Fourier filter curves
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2016-2021 Stefan Gerlach <stefan.gerlach@uni.kn>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef XYFOURIERFILTERCURVEDOCK_H
#define XYFOURIERFILTERCURVEDOCK_H

#include "kdefrontend/dockwidgets/XYCurveDock.h"
#include "backend/worksheet/plots/cartesian/XYFourierFilterCurve.h"
#include "ui_xyfourierfiltercurvedockgeneraltab.h"

class TreeViewComboBox;

class XYFourierFilterCurveDock: public XYCurveDock {
	Q_OBJECT

public:
	explicit XYFourierFilterCurveDock(QWidget *parent);
	void setCurves(QList<XYCurve*>);
	void setupGeneral() override;

private:
	void initGeneralTab() override;
	void updatePlotRanges() const override;
	void showFilterResult();

	Ui::XYFourierFilterCurveDockGeneralTab uiGeneralTab;
	TreeViewComboBox* cbDataSourceCurve{nullptr};
	TreeViewComboBox* cbXDataColumn{nullptr};
	TreeViewComboBox* cbYDataColumn{nullptr};

	XYFourierFilterCurve* m_filterCurve{nullptr};
	XYFourierFilterCurve::FilterData m_filterData;

protected:
	void setModel() override;

private Q_SLOTS:
	//SLOTs for changes triggered in XYFourierFilterCurveDock
	//general tab
	void dataSourceTypeChanged(int);
	void dataSourceCurveChanged(const QModelIndex&);
	void xDataColumnChanged(const QModelIndex&);
	void yDataColumnChanged(const QModelIndex&);
	void autoRangeChanged();
	void xRangeMinChanged();
	void xRangeMaxChanged();
	void typeChanged();
	void formChanged();
	void orderChanged();
	void unitChanged();
	void unit2Changed();

	void recalculateClicked();
	void enableRecalculate() const;

	//SLOTs for changes triggered in XYCurve
	//General-Tab
	void curveDataSourceTypeChanged(XYAnalysisCurve::DataSourceType);
	void curveDataSourceCurveChanged(const XYCurve*);
	void curveXDataColumnChanged(const AbstractColumn*);
	void curveYDataColumnChanged(const AbstractColumn*);
	void curveFilterDataChanged(const XYFourierFilterCurve::FilterData&);
	void dataChanged();
	void curveVisibilityChanged(bool);
};

#endif
