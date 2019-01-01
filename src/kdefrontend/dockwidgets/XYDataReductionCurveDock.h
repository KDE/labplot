/***************************************************************************
    File             : XYDataReductionCurveDock.h
    Project          : LabPlot
    --------------------------------------------------------------------
    Copyright        : (C) 2016 Stefan Gerlach (stefan.gerlach@uni.kn)
    Copyright        : (C) 2017 Alexander Semke (alexander.semke@web.de)
    Description      : widget for editing properties of data reduction curves

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

#ifndef XYDATAREDUCTIONCURVEDOCK_H
#define XYDATAREDUCTIONCURVEDOCK_H

#include "kdefrontend/dockwidgets/XYCurveDock.h"
#include "backend/worksheet/plots/cartesian/XYDataReductionCurve.h"
#include "ui_xydatareductioncurvedockgeneraltab.h"

class XYAnalysisCurve;
class TreeViewComboBox;
class QStatusBar;

class XYDataReductionCurveDock : public XYCurveDock {
	Q_OBJECT

public:
	explicit XYDataReductionCurveDock(QWidget *parent, QStatusBar *sb);
	void setCurves(QList<XYCurve*>);
	void setupGeneral() override;

private:
	void initGeneralTab() override;
	void showDataReductionResult();
	void updateTolerance();
	void updateTolerance2();

	Ui::XYDataReductionCurveDockGeneralTab uiGeneralTab;
	QStatusBar* statusBar;	// main status bar to display progress
	TreeViewComboBox* cbDataSourceCurve{nullptr};
	TreeViewComboBox* cbXDataColumn{nullptr};
	TreeViewComboBox* cbYDataColumn{nullptr};

	XYDataReductionCurve* m_dataReductionCurve{nullptr};
	XYDataReductionCurve::DataReductionData m_dataReductionData;

protected:
	void setModel() override;

private slots:
	//SLOTs for changes triggered in XYDataReductionCurveDock
	//general tab
	void nameChanged();
	void commentChanged();
	void dataSourceTypeChanged(int);
	void dataSourceCurveChanged(const QModelIndex&);
	void xDataColumnChanged(const QModelIndex&);
	void yDataColumnChanged(const QModelIndex&);
	void autoRangeChanged();
	void xRangeMinChanged();
	void xRangeMaxChanged();
	void typeChanged();
	void autoToleranceChanged();
	void toleranceChanged();
	void autoTolerance2Changed();
	void tolerance2Changed();

	void recalculateClicked();
	void enableRecalculate() const;

	//SLOTs for changes triggered in XYCurve
	//General-Tab
	void curveDescriptionChanged(const AbstractAspect*);
	void curveDataSourceTypeChanged(XYAnalysisCurve::DataSourceType);
	void curveDataSourceCurveChanged(const XYCurve*);
	void curveXDataColumnChanged(const AbstractColumn*);
	void curveYDataColumnChanged(const AbstractColumn*);
	void curveDataReductionDataChanged(const XYDataReductionCurve::DataReductionData&);
	void dataChanged();
};

#endif
