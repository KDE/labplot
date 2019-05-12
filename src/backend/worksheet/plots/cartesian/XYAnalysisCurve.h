/***************************************************************************
    File                 : XYAnalysisCurve.h
    Project              : LabPlot
    Description          : Base class for all analysis curves
    --------------------------------------------------------------------
    Copyright            : (C) 2017 Alexander Semke (alexander.semke@web.de)
    Copyright            : (C) 2018 Stefan Gerlach (stefan.gerlach@uni.kn)

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

#ifndef XYANALYSISCURVE_H
#define XYANALYSISCURVE_H

#include "backend/worksheet/plots/cartesian/XYCurve.h"

class XYAnalysisCurvePrivate;

class XYAnalysisCurve : public XYCurve {
	Q_OBJECT

public:
	enum DataSourceType {DataSourceSpreadsheet, DataSourceCurve};

	XYAnalysisCurve(const QString&, AspectType type);
	~XYAnalysisCurve() override;

	virtual void recalculate() = 0;
	void save(QXmlStreamWriter*) const override;
	bool load(XmlStreamReader*, bool preview) override;

	BASIC_D_ACCESSOR_DECL(DataSourceType, dataSourceType, DataSourceType)
	POINTER_D_ACCESSOR_DECL(const XYCurve, dataSourceCurve, DataSourceCurve)
	const QString& dataSourceCurvePath() const;

	POINTER_D_ACCESSOR_DECL(const AbstractColumn, xDataColumn, XDataColumn)
	POINTER_D_ACCESSOR_DECL(const AbstractColumn, yDataColumn, YDataColumn)
	POINTER_D_ACCESSOR_DECL(const AbstractColumn, y2DataColumn, Y2DataColumn)	// optional
	const QString& xDataColumnPath() const;
	const QString& yDataColumnPath() const;
	const QString& y2DataColumnPath() const;

	typedef XYAnalysisCurvePrivate Private;

protected:
	XYAnalysisCurve(const QString& name, XYAnalysisCurvePrivate* dd, AspectType type);

private:
	Q_DECLARE_PRIVATE(XYAnalysisCurve)
	void init();

public slots:
	void handleSourceDataChanged();

signals:
	void sourceDataChanged(); //emitted when the source data used in the analysis curves was changed to enable the recalculation in the dock widgets
	void dataSourceTypeChanged(XYAnalysisCurve::DataSourceType);
	void dataSourceCurveChanged(const XYCurve*);
	void xDataColumnChanged(const AbstractColumn*);
	void yDataColumnChanged(const AbstractColumn*);
	void y2DataColumnChanged(const AbstractColumn*);
};

#endif
