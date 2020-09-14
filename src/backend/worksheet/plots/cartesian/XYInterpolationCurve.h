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
	enum class PointsMode {Auto, Multiple, Custom};
	struct InterpolationData {
		InterpolationData() : xRange(2) {};

		nsl_interp_type type{nsl_interp_type_linear};			// type of interpolation
		nsl_interp_pch_variant variant{nsl_interp_pch_variant_finite_difference};		// variant of cubic Hermite interpolation
		double tension{0.0}, continuity{0.0}, bias{0.0};		// TCB values
		nsl_interp_evaluate evaluate{nsl_interp_evaluate_function};	// what to evaluate
		size_t npoints{100};						// nr. of points
		XYInterpolationCurve::PointsMode pointsMode{PointsMode::Auto};	// mode to interpret points
		bool autoRange{true};						// use all data?
		QVector<double> xRange;						// x range for interpolation
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
