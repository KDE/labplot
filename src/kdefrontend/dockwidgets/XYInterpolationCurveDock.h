/***************************************************************************
    File             : XYInterpolationCurveDock.h
    Project          : LabPlot
    --------------------------------------------------------------------
    Copyright        : (C) 2016 Stefan Gerlach (stefan.gerlach@uni.kn)
    Description      : widget for editing properties of interpolation curves

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

#ifndef XYINTERPOLATIONCURVEDOCK_H
#define XYINTERPOLATIONCURVEDOCK_H

#include "kdefrontend/dockwidgets/XYCurveDock.h"
#include "backend/worksheet/plots/cartesian/XYAnalysisCurve.h"
#include "backend/worksheet/plots/cartesian/XYInterpolationCurve.h"
#include "ui_xyinterpolationcurvedockgeneraltab.h"

class TreeViewComboBox;

class XYInterpolationCurveDock: public XYCurveDock {
	Q_OBJECT

public:
	explicit XYInterpolationCurveDock(QWidget *parent);
	void setCurves(QList<XYCurve*>);
	void setupGeneral() override;

private:
	void initGeneralTab() override;
	void showInterpolationResult();
	void updateSettings(const AbstractColumn*);

	Ui::XYInterpolationCurveDockGeneralTab uiGeneralTab;
	TreeViewComboBox* cbDataSourceCurve{nullptr};
	TreeViewComboBox* cbXDataColumn{nullptr};
	TreeViewComboBox* cbYDataColumn{nullptr};

	XYInterpolationCurve* m_interpolationCurve{nullptr};
	XYInterpolationCurve::InterpolationData m_interpolationData;
	unsigned int dataPoints{0};	// number of data points in selected column

protected:
	void setModel() override;

private slots:
	//SLOTs for changes triggered in XYInterpolationCurveDock
	//general tab
	void dataSourceTypeChanged(int);
	void dataSourceCurveChanged(const QModelIndex&);
	void xDataColumnChanged(const QModelIndex&);
	void yDataColumnChanged(const QModelIndex&);
	void autoRangeChanged();
	void xRangeMinChanged();
	void xRangeMaxChanged();
	void typeChanged();
	void variantChanged();
	void tensionChanged();
	void continuityChanged();
	void biasChanged();
	void evaluateChanged();
	void numberOfPointsChanged();
	void pointsModeChanged();

	void recalculateClicked();
	void enableRecalculate() const;

	//SLOTs for changes triggered in XYCurve
	//General-Tab
	void curveDescriptionChanged(const AbstractAspect*);
	void curveDataSourceTypeChanged(XYAnalysisCurve::DataSourceType);
	void curveDataSourceCurveChanged(const XYCurve*);
	void curveXDataColumnChanged(const AbstractColumn*);
	void curveYDataColumnChanged(const AbstractColumn*);
	void curveInterpolationDataChanged(const XYInterpolationCurve::InterpolationData&);
	void dataChanged();
};

#endif
