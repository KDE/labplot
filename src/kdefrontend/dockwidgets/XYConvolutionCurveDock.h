/***************************************************************************
    File             : XYConvolutionCurveDock.h
    Project          : LabPlot
    --------------------------------------------------------------------
    Copyright        : (C) 2018 Stefan Gerlach (stefan.gerlach@uni.kn)
    Description      : widget for editing properties of convolution curves

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

#ifndef XYCONVOLUTIONCURVEDOCK_H
#define XYCONVOLUTIONCURVEDOCK_H

#include "kdefrontend/dockwidgets/XYCurveDock.h"
#include "backend/worksheet/plots/cartesian/XYConvolutionCurve.h"
#include "ui_xyconvolutioncurvedockgeneraltab.h"

class TreeViewComboBox;

class XYConvolutionCurveDock: public XYCurveDock {
	Q_OBJECT

public:
	explicit XYConvolutionCurveDock(QWidget*);
	void setCurves(QList<XYCurve*>);
	void setupGeneral() override;

private:
	void initGeneralTab() override;
	void showConvolutionResult();

	Ui::XYConvolutionCurveDockGeneralTab uiGeneralTab;
	TreeViewComboBox* cbDataSourceCurve{nullptr};
	TreeViewComboBox* cbXDataColumn{nullptr};
	TreeViewComboBox* cbYDataColumn{nullptr};
	TreeViewComboBox* cbY2DataColumn{nullptr};

	XYConvolutionCurve* m_convolutionCurve{nullptr};
	XYConvolutionCurve::ConvolutionData m_convolutionData;

protected:
	void setModel() override;

private slots:
	//SLOTs for changes triggered in XYConvolutionCurveDock
	//general tab
	void nameChanged();
	void commentChanged();
	void dataSourceTypeChanged(int);
	void dataSourceCurveChanged(const QModelIndex&);
	void xDataColumnChanged(const QModelIndex&);
	void yDataColumnChanged(const QModelIndex&);
	void y2DataColumnChanged(const QModelIndex&);
	void samplingIntervalChanged();
	void kernelChanged();
	void kernelSizeChanged();
	void autoRangeChanged();
	void xRangeMinChanged();
	void xRangeMaxChanged();
	void directionChanged();
	void typeChanged();
	void normChanged();
	void wrapChanged();

	void recalculateClicked();
	void enableRecalculate() const;

	//SLOTs for changes triggered in XYCurve
	//General-Tab
	void curveDescriptionChanged(const AbstractAspect*);
	void curveDataSourceTypeChanged(XYAnalysisCurve::DataSourceType);
	void curveDataSourceCurveChanged(const XYCurve*);
	void curveXDataColumnChanged(const AbstractColumn*);
	void curveYDataColumnChanged(const AbstractColumn*);
	void curveY2DataColumnChanged(const AbstractColumn*);
	void curveConvolutionDataChanged(const XYConvolutionCurve::ConvolutionData&);
	void dataChanged();
};

#endif
