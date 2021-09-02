/*
    File             : XYHilbertTransformCurveDock.h
    Project          : LabPlot
    Description      : widget for editing properties of Hilbert transform curves
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2021 Stefan Gerlach <stefan.gerlach@uni.kn>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef XYHILBERTTRANSFORMCURVEDOCK_H
#define XYHILBERTTRANSFORMCURVEDOCK_H

#include "kdefrontend/dockwidgets/XYCurveDock.h"
#include "backend/worksheet/plots/cartesian/XYHilbertTransformCurve.h"
#include "ui_xyhilberttransformcurvedockgeneraltab.h"

class TreeViewComboBox;

class XYHilbertTransformCurveDock: public XYCurveDock {
	Q_OBJECT

public:
	explicit XYHilbertTransformCurveDock(QWidget *parent);
	void setCurves(QList<XYCurve*>);
	void setupGeneral() override;

private:
	void initGeneralTab() override;
	void updatePlotRanges() const override;
	void showTransformResult();

	Ui::XYHilbertTransformCurveDockGeneralTab uiGeneralTab;
	TreeViewComboBox* cbXDataColumn{nullptr};
	TreeViewComboBox* cbYDataColumn{nullptr};

	XYHilbertTransformCurve* m_transformCurve{nullptr};
	XYHilbertTransformCurve::TransformData m_transformData;

protected:
	void setModel() override;

private slots:
	//SLOTs for changes triggered in XYHilbertTransformCurveDock
	//general tab
	void xDataColumnChanged(const QModelIndex&);
	void yDataColumnChanged(const QModelIndex&);
	void autoRangeChanged();
	void xRangeMinChanged();
	void xRangeMaxChanged();
	void typeChanged();

//	void showOptions();
	void recalculateClicked();
	void enableRecalculate() const;

	//SLOTs for changes triggered in XYCurve
	//General-Tab
	void curveXDataColumnChanged(const AbstractColumn*);
	void curveYDataColumnChanged(const AbstractColumn*);
	void curveTransformDataChanged(const XYHilbertTransformCurve::TransformData&);
	void dataChanged();
	void curveVisibilityChanged(bool);
};

#endif
