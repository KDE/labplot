/*
	File                 : XYSmoothCurve.h
	Project              : LabPlot
	Description          : A xy-curve defined by a smooth
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2016-2021 Stefan Gerlach <stefan.gerlach@uni.kn>
	SPDX-FileCopyrightText: 2017 Alexander Semke <alexander.semke@web.de>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef XYSMOOTHCURVE_H
#define XYSMOOTHCURVE_H

#include "backend/worksheet/plots/cartesian/XYAnalysisCurve.h"

extern "C" {
#include "backend/nsl/nsl_smooth.h"
}

class XYSmoothCurvePrivate;

#ifdef SDK
#include "labplot_export.h"
class LABPLOT_EXPORT XYSmoothCurve : public XYAnalysisCurve {
#else
class XYSmoothCurve : public XYAnalysisCurve {
#endif
	Q_OBJECT

public:
	struct SmoothData {
		SmoothData() {};

		nsl_smooth_type type{nsl_smooth_type_moving_average}; // type of smoothing
		size_t points{5}; // number of points
		nsl_smooth_weight_type weight{nsl_smooth_weight_uniform}; // type of weight
		double percentile{0.5}; // percentile for percentile filter (0.0 .. 1.0)
		int order{2}; // order for Savitzky-Golay filter
		nsl_smooth_pad_mode mode{nsl_smooth_pad_none}; // mode of padding for edges
		double lvalue{0.0}, rvalue{0.0}; // values for constant padding
		bool autoRange{true}; // use all data?
		// TODO: use Range
		QVector<double> xRange{0., 0.}; // x range for integration
	};

	explicit XYSmoothCurve(const QString& name);
	~XYSmoothCurve() override;

	QIcon icon() const override;
	void save(QXmlStreamWriter*) const override;
	bool load(XmlStreamReader*, bool preview) override;

	const AbstractColumn* roughsColumn() const;
	CLASS_D_ACCESSOR_DECL(SmoothData, smoothData, SmoothData)

	typedef XYAnalysisCurve::Result SmoothResult;
	virtual const XYAnalysisCurve::Result& result() const override;

	typedef XYSmoothCurvePrivate Private;

protected:
	XYSmoothCurve(const QString& name, XYSmoothCurvePrivate* dd);

private:
	Q_DECLARE_PRIVATE(XYSmoothCurve)

Q_SIGNALS:
	void smoothDataChanged(const XYSmoothCurve::SmoothData&);
};

#endif
