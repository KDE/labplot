/*
    File                 : XYInterpolationCurve.h
    Project              : LabPlot
    Description          : A xy-curve defined by an interpolation
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2016-2021 Stefan Gerlach (stefan.gerlach@uni.kn)
    SPDX-FileCopyrightText: 2017 Alexander Semke (alexander.semke@web.de)
    SPDX-License-Identifier: GPL-2.0-or-later
*/


#ifndef XYINTERPOLATIONCURVE_H
#define XYINTERPOLATIONCURVE_H

#include "backend/worksheet/plots/cartesian/XYAnalysisCurve.h"
extern "C" {
#include <gsl/gsl_version.h>
#include "backend/nsl/nsl_interp.h"
}

class XYInterpolationCurvePrivate;

class XYInterpolationCurve : public XYAnalysisCurve {
	Q_OBJECT

public:
	enum class PointsMode {Auto, Multiple, Custom};
	struct InterpolationData {
		InterpolationData() {};

		nsl_interp_type type{nsl_interp_type_linear};			// type of interpolation
		nsl_interp_pch_variant variant{nsl_interp_pch_variant_finite_difference};		// variant of cubic Hermite interpolation
		double tension{0.0}, continuity{0.0}, bias{0.0};		// TCB values
		nsl_interp_evaluate evaluate{nsl_interp_evaluate_function};	// what to evaluate
		size_t npoints{100};						// nr. of points
		XYInterpolationCurve::PointsMode pointsMode{PointsMode::Auto};	// mode to interpret points
		bool autoRange{true};						// use all data?
		//TODO: use Range
		QVector<double> xRange{0, 0};					// x range for interpolation
	};
	struct InterpolationResult {
		InterpolationResult() {};

		bool available{false};
		bool valid{false};
		QString status;
		qint64 elapsedTime{0};
	};

	explicit XYInterpolationCurve(const QString& name);
	~XYInterpolationCurve() override;

	void recalculate() override;
	QIcon icon() const override;
	void save(QXmlStreamWriter*) const override;
	bool load(XmlStreamReader*, bool preview) override;

	CLASS_D_ACCESSOR_DECL(InterpolationData, interpolationData, InterpolationData)
	const InterpolationResult& interpolationResult() const;

	typedef XYInterpolationCurvePrivate Private;

protected:
	XYInterpolationCurve(const QString& name, XYInterpolationCurvePrivate* dd);

private:
	Q_DECLARE_PRIVATE(XYInterpolationCurve)

signals:
	void interpolationDataChanged(const XYInterpolationCurve::InterpolationData&);
};

#endif
