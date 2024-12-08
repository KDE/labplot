/*
	File                 : XYAnalysisCurve.h
	Project              : LabPlot
	Description          : Base class for all analysis curves
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2017-2024 Alexander Semke <alexander.semke@web.de>
	SPDX-FileCopyrightText: 2018-2022 Stefan Gerlach <stefan.gerlach@uni.kn>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef XYANALYSISCURVE_H
#define XYANALYSISCURVE_H

#include "backend/worksheet/plots/cartesian/XYCurve.h"

class XYAnalysisCurvePrivate;

class XYAnalysisCurve : public XYCurve {
	Q_OBJECT

public:
	enum class DataSourceType { Spreadsheet, Curve, Histogram };
	Q_ENUM(DataSourceType)
	enum class AnalysisAction {
		DataReduction,
		Differentiation,
		Integration,
		Interpolation,
		Smoothing,
		FitLinear,
		FitPower,
		FitExp1,
		FitExp2,
		FitInvExp,
		FitGauss,
		FitCauchyLorentz,
		FitTan,
		FitTanh,
		FitErrFunc,
		FitCustom,
		FourierFilter
	};
	Q_ENUM(AnalysisAction)

	struct Result {
		Result() {
		}

		bool available{false};
		bool valid{false};
		QString status;
		qint64 elapsedTime{0};
	};

	~XYAnalysisCurve() override;

	static void copyData(QVector<double>& xData,
						 QVector<double>& yData,
						 const AbstractColumn* xDataColumn,
						 const AbstractColumn* yDataColumn,
						 double xMin,
						 double xMax,
						 bool avgUniqueX = false);

	virtual void recalculate() = 0;
	bool resultAvailable() const;
	virtual const Result& result() const = 0;
	bool usingColumn(const AbstractColumn*, bool indirect = true) const override;
	virtual QVector<const Plot*> dependingPlots() const;

	void save(QXmlStreamWriter*) const override;
	bool load(XmlStreamReader*, bool preview) override;

	BASIC_D_ACCESSOR_DECL(DataSourceType, dataSourceType, DataSourceType)
	POINTER_D_ACCESSOR_DECL(const XYCurve, dataSourceCurve, DataSourceCurve)
	CLASS_D_ACCESSOR_DECL(QString, dataSourceCurvePath, DataSourceCurvePath)

	POINTER_D_ACCESSOR_DECL(const AbstractColumn, xDataColumn, XDataColumn)
	POINTER_D_ACCESSOR_DECL(const AbstractColumn, yDataColumn, YDataColumn)
	POINTER_D_ACCESSOR_DECL(const AbstractColumn, y2DataColumn, Y2DataColumn) // optional
	CLASS_D_ACCESSOR_DECL(QString, xDataColumnPath, XDataColumnPath)
	CLASS_D_ACCESSOR_DECL(QString, yDataColumnPath, YDataColumnPath)
	CLASS_D_ACCESSOR_DECL(QString, y2DataColumnPath, Y2DataColumnPath)

	bool saveCalculations() const;

	typedef XYAnalysisCurvePrivate Private;

protected:
	XYAnalysisCurve(const QString& name, XYAnalysisCurvePrivate*, AspectType);
	virtual void handleAspectUpdated(const QString& aspectPath, const AbstractAspect*) override;

private:
	Q_DECLARE_PRIVATE(XYAnalysisCurve)
	void init();

public Q_SLOTS:
	void handleSourceDataChanged();
	void createDataSpreadsheet();

private Q_SLOTS:
	void xDataColumnAboutToBeRemoved(const AbstractAspect*);
	void yDataColumnAboutToBeRemoved(const AbstractAspect*);
	void y2DataColumnAboutToBeRemoved(const AbstractAspect*);
	void xDataColumnNameChanged();
	void yDataColumnNameChanged();
	void y2DataColumnNameChanged();

	void dataSourceCurveAboutToBeRemoved(const AbstractAspect*);
	void dataSourceCurveNameChanged();

Q_SIGNALS:
	void sourceDataChanged(); // emitted when the source data used in the analysis curves was changed to enable the recalculation in the dock widgets
	void dataSourceTypeChanged(XYAnalysisCurve::DataSourceType);
	void dataSourceCurveChanged(const XYCurve*);
	void xDataColumnChanged(const AbstractColumn*);
	void yDataColumnChanged(const AbstractColumn*);
	void y2DataColumnChanged(const AbstractColumn*);
};

#endif
