/***************************************************************************
    File                 : XYInterpolationCurve.h
    Project              : LabPlot
    Description          : A xy-curve defined by an interpolation
    --------------------------------------------------------------------
    Copyright            : (C) 2016 Stefan Gerlach (stefan.gerlach@uni.kn)
    Copyright            : (C) 2017 Alexander Semke (alexander.semke@web.de)

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
	enum PointsMode {Auto, Multiple, Custom};
	struct InterpolationData {
		InterpolationData() : type(nsl_interp_type_linear), variant(nsl_interp_pch_variant_finite_difference),
			tension(0.0), continuity(0.0), bias(0.0), evaluate(nsl_interp_evaluate_function), npoints(100),
			pointsMode(XYInterpolationCurve::Auto), autoRange(true), xRange(2) {};

		nsl_interp_type type;			// type of interpolation
		nsl_interp_pch_variant variant;		// variant of cubic Hermite interpolation
		double tension, continuity, bias;	// TCB values
		nsl_interp_evaluate evaluate;		// what to evaluate
		size_t npoints;				// nr. of points
		XYInterpolationCurve::PointsMode pointsMode;	// mode to interpret points
		bool autoRange;				// use all data?
		QVector<double> xRange;			// x range for interpolation
	};
	struct InterpolationResult {
		InterpolationResult() : available(false), valid(false), elapsedTime(0) {};

		bool available;
		bool valid;
		QString status;
		qint64 elapsedTime;
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
