/*
    File                 : XYAnalysisCurve.h
    Project              : LabPlot
    Description          : Base class for all analysis curves
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2017 Alexander Semke (alexander.semke@web.de)
    SPDX-FileCopyrightText: 2018 Stefan Gerlach (stefan.gerlach@uni.kn)
    SPDX-License-Identifier: GPL-2.0-or-later
*/


#ifndef XYANALYSISCURVE_H
#define XYANALYSISCURVE_H

#include "backend/worksheet/plots/cartesian/XYCurve.h"

class XYAnalysisCurvePrivate;

class XYAnalysisCurve : public XYCurve {
	Q_OBJECT

public:
	enum class DataSourceType {Spreadsheet, Curve};

	XYAnalysisCurve(const QString&, AspectType type);
	~XYAnalysisCurve() override;

	static void copyData(QVector<double>& xData, QVector<double>& yData, const AbstractColumn* xDataColumn, const AbstractColumn* yDataColumn, double xMin, double xMax);

	virtual void recalculate() = 0;
	void save(QXmlStreamWriter*) const override;
	bool load(XmlStreamReader*, bool preview) override;

	BASIC_D_ACCESSOR_DECL(DataSourceType, dataSourceType, DataSourceType)
	POINTER_D_ACCESSOR_DECL(const XYCurve, dataSourceCurve, DataSourceCurve)
	const QString& dataSourceCurvePath() const;

	POINTER_D_ACCESSOR_DECL(const AbstractColumn, xDataColumn, XDataColumn)
	POINTER_D_ACCESSOR_DECL(const AbstractColumn, yDataColumn, YDataColumn)
	POINTER_D_ACCESSOR_DECL(const AbstractColumn, y2DataColumn, Y2DataColumn)	// optional
	CLASS_D_ACCESSOR_DECL(QString, xDataColumnPath, XDataColumnPath)
	CLASS_D_ACCESSOR_DECL(QString, yDataColumnPath, YDataColumnPath)
	CLASS_D_ACCESSOR_DECL(QString, y2DataColumnPath, Y2DataColumnPath)

	typedef XYAnalysisCurvePrivate Private;

protected:
	XYAnalysisCurve(const QString& name, XYAnalysisCurvePrivate* dd, AspectType type);

private:
	Q_DECLARE_PRIVATE(XYAnalysisCurve)
	void init();

public slots:
	void handleSourceDataChanged();
private slots:
	void xDataColumnAboutToBeRemoved(const AbstractAspect*);
	void yDataColumnAboutToBeRemoved(const AbstractAspect*);
	void y2DataColumnAboutToBeRemoved(const AbstractAspect*);
	void xDataColumnNameChanged();
	void yDataColumnNameChanged();
	void y2DataColumnNameChanged();

signals:
	void sourceDataChanged(); //emitted when the source data used in the analysis curves was changed to enable the recalculation in the dock widgets
	void dataSourceTypeChanged(XYAnalysisCurve::DataSourceType);
	void dataSourceCurveChanged(const XYCurve*);
	void xDataColumnChanged(const AbstractColumn*);
	void yDataColumnChanged(const AbstractColumn*);
	void y2DataColumnChanged(const AbstractColumn*);
};

#endif
