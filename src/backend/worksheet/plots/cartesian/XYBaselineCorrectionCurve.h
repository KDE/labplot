/*
	Fil              : XYBaselineCorrectionCurve.h
	Project          : LabPlot
	Description      : A xy-curve defined by baseline correction
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2025 Alexander Semke <alexander.semke@web.de>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef XYBASELINESUBTRACTIONCURVE_H
#define XYBASELINESUBTRACTIONCURVE_H

#include "backend/nsl/nsl_baseline.h"
#include "backend/worksheet/plots/cartesian/XYAnalysisCurve.h"

class XYBaselineCorrectionCurvePrivate;

#ifdef SDK
#include "labplot_export.h"
class LABPLOT_EXPORT XYBaselineCorrectionCurve : public XYAnalysisCurve {
#else
class XYBaselineCorrectionCurve : public XYAnalysisCurve {
#endif
	Q_OBJECT

public:
	struct BaselineData {
		BaselineData() { }

		nsl_baseline_correction_method method{nsl_diff_baseline_correction_arpls};
		bool autoRange{true};
		QVector<double> xRange{0., 0.};

		/* arPLS parameters */
		double arPLSTerminationRatio{0.1}; // termination ratio
		double arPLSSmoothness{6}; // smoothness parameter, power of 10
		int arPLSIterations{10}; // number of iteratiosn
	};

	typedef XYAnalysisCurve::Result BaselineResult;
	virtual const XYAnalysisCurve::Result& result() const override;

	explicit XYBaselineCorrectionCurve(const QString& name);
	~XYBaselineCorrectionCurve() override;

	QIcon icon() const override;
	void save(QXmlStreamWriter*) const override;
	bool load(XmlStreamReader*, bool preview) override;

	CLASS_D_ACCESSOR_DECL(BaselineData, baselineData, BaselineData)
	const BaselineResult& baselineResult() const;

	typedef XYBaselineCorrectionCurvePrivate Private;

protected:
	XYBaselineCorrectionCurve(const QString& name, XYBaselineCorrectionCurvePrivate* dd);

private:
	Q_DECLARE_PRIVATE(XYBaselineCorrectionCurve)

Q_SIGNALS:
	void baselineDataChanged(const XYBaselineCorrectionCurve::BaselineData&);
};

#endif
