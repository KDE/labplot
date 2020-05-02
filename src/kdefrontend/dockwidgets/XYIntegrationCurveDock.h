/***************************************************************************
    File             : XYIntegrationCurveDock.h
    Project          : LabPlot
    --------------------------------------------------------------------
    Copyright        : (C) 2016 Stefan Gerlach (stefan.gerlach@uni.kn)
    Description      : widget for editing properties of integration curves

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

#ifndef XYINTEGRATIONCURVEDOCK_H
#define XYINTEGRATIONCURVEDOCK_H

#include "kdefrontend/dockwidgets/XYCurveDock.h"
#include "backend/worksheet/plots/cartesian/XYIntegrationCurve.h"
#include "ui_xyintegrationcurvedockgeneraltab.h"

class TreeViewComboBox;

class XYIntegrationCurveDock: public XYCurveDock {
	Q_OBJECT

public:
	explicit XYIntegrationCurveDock(QWidget*);
	void setCurves(QList<XYCurve*>);
	void setupGeneral() override;

private:
	void initGeneralTab() override;
	void showIntegrationResult();
	void updateSettings(const AbstractColumn*);

	Ui::XYIntegrationCurveDockGeneralTab uiGeneralTab;
	TreeViewComboBox* cbDataSourceCurve{nullptr};
	TreeViewComboBox* cbXDataColumn{nullptr};
	TreeViewComboBox* cbYDataColumn{nullptr};

	XYIntegrationCurve* m_integrationCurve{nullptr};
	XYIntegrationCurve::IntegrationData m_integrationData;
	bool m_dateTimeRange{false};

protected:
	void setModel() override;

private slots:
	//SLOTs for changes triggered in XYIntegrationCurveDock
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
	void methodChanged(int);
	void absoluteChanged();

	void recalculateClicked();
	void enableRecalculate() const;

	//SLOTs for changes triggered in XYCurve
	//General-Tab
	void curveDescriptionChanged(const AbstractAspect*);
	void curveDataSourceTypeChanged(XYAnalysisCurve::DataSourceType);
	void curveDataSourceCurveChanged(const XYCurve*);
	void curveXDataColumnChanged(const AbstractColumn*);
	void curveYDataColumnChanged(const AbstractColumn*);
	void curveIntegrationDataChanged(const XYIntegrationCurve::IntegrationData&);
	void dataChanged();
};

#endif
