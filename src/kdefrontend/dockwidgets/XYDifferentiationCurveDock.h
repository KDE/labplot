/***************************************************************************
    File             : XYDifferentiationCurveDock.h
    Project          : LabPlot
    --------------------------------------------------------------------
    Copyright        : (C) 2016 Stefan Gerlach (stefan.gerlach@uni.kn)
    Copyright        : (C) 2017 Alexander Semke (alexander.semke@web.de)
    Description      : widget for editing properties of differentiation curves

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

#ifndef XYDIFFERENTIATIONCURVEDOCK_H
#define XYDIFFERENTIATIONCURVEDOCK_H

#include "kdefrontend/dockwidgets/XYCurveDock.h"
#include "backend/worksheet/plots/cartesian/XYDifferentiationCurve.h"
#include "ui_xydifferentiationcurvedockgeneraltab.h"

class TreeViewComboBox;

class XYDifferentiationCurveDock : public XYCurveDock {
	Q_OBJECT

public:
	explicit XYDifferentiationCurveDock(QWidget*);
	void setCurves(QList<XYCurve*>);
	void setupGeneral() override;

private:
	void initGeneralTab() override;
	void showDifferentiationResult();
	void updateSettings(const AbstractColumn*);

	Ui::XYDifferentiationCurveDockGeneralTab uiGeneralTab;
	TreeViewComboBox* cbDataSourceCurve{nullptr};
	TreeViewComboBox* cbXDataColumn{nullptr};
	TreeViewComboBox* cbYDataColumn{nullptr};

	XYDifferentiationCurve* m_differentiationCurve{nullptr};
	XYDifferentiationCurve::DifferentiationData m_differentiationData;
	bool m_dateTimeRange{false};

protected:
	void setModel() override;

private slots:
	//SLOTs for changes triggered in XYDifferentiationCurveDock
	//general tab
	void dataSourceTypeChanged(int);
	void dataSourceCurveChanged(const QModelIndex&);
	void xDataColumnChanged(const QModelIndex&);
	void yDataColumnChanged(const QModelIndex&);
	void autoRangeChanged();
	void xRangeMinChanged(double);
	void xRangeMaxChanged(double);
	void xRangeMinDateTimeChanged(const QDateTime&);
	void xRangeMaxDateTimeChanged(const QDateTime&);
	void derivOrderChanged(int);
	void accOrderChanged(int);

	void recalculateClicked();
	void enableRecalculate() const;

	//SLOTs for changes triggered in XYDifferentiationCurve
	//General-Tab
	void curveDescriptionChanged(const AbstractAspect*);
	void curveDataSourceTypeChanged(XYAnalysisCurve::DataSourceType);
	void curveDataSourceCurveChanged(const XYCurve*);
	void curveXDataColumnChanged(const AbstractColumn*);
	void curveYDataColumnChanged(const AbstractColumn*);
	void curveDifferentiationDataChanged(const XYDifferentiationCurve::DifferentiationData&);
	void dataChanged();
};

#endif
