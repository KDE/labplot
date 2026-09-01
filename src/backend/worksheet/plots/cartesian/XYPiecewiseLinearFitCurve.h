/*
	File                 : XYPiecewiseLinearFitCurve.h
	Project              : LabPlot
	Description          : A xy-curve defined by piecewise linear regression
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2026 Alexander Semke <alexander.semke@web.de>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef XYPIECEWISELINEARFITCURVE_H
#define XYPIECEWISELINEARFITCURVE_H

#include "backend/worksheet/plots/cartesian/XYAnalysisCurve.h"

extern "C" {
#include "backend/nsl/nsl_changepoint.h"
}

class ReferenceLine;
class XYPiecewiseLinearFitCurvePrivate;

#ifdef SDK
#include "labplot_export.h"
class LABPLOT_EXPORT XYPiecewiseLinearFitCurve : public XYAnalysisCurve {
#else
class XYPiecewiseLinearFitCurve : public XYAnalysisCurve {
#endif
	Q_OBJECT

public:
	struct FitData {
		FitData() {
		}

		nsl_changepoint_method changepointMethod{nsl_changepoint_method_binary_segmentation};
		double penalty{1.0}; // Penalty for adding changepoints - higher values = fewer segments
		size_t minSegmentSize{2}; // Minimum number of points per segment
		size_t maxChangepoints{1}; // Maximum number of changepoints to detect
	};

	struct FitResult : public XYAnalysisCurve::Result {
		FitResult() {
		}

		size_t numSegments{0};
		QVector<double> changepoints; // X-values of changepoints
		QVector<double> slopes;
		QVector<double> intercepts;
		QVector<double> rsquares;
		double overallRsquare{0.};
		double sse{0.};
	};

	explicit XYPiecewiseLinearFitCurve(const QString& name);
	~XYPiecewiseLinearFitCurve() override;

	virtual const XYAnalysisCurve::Result& result() const override;

	QIcon icon() const override;
	void save(QXmlStreamWriter*) const override;
	bool load(XmlStreamReader*, bool preview) override;

	CLASS_D_ACCESSOR_DECL(FitData, fitData, FitData)
	const FitResult& fitResult() const;

	BASIC_D_ACCESSOR_DECL(bool, changepointLinesEnabled, ChangepointLinesEnabled)
	QVector<ReferenceLine*> changepointLines() const;

	typedef XYPiecewiseLinearFitCurvePrivate Private;

protected:
	XYPiecewiseLinearFitCurve(const QString& name, XYPiecewiseLinearFitCurvePrivate* dd);

private:
	Q_DECLARE_PRIVATE(XYPiecewiseLinearFitCurve)

Q_SIGNALS:
	void fitDataChanged(const XYPiecewiseLinearFitCurve::FitData&);
	void changepointLinesEnabledChanged(bool);
};

#endif
