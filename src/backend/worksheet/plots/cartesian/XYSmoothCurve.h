/***************************************************************************
    File                 : XYSmoothCurve.h
    Project              : LabPlot
    Description          : A xy-curve defined by a smooth
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

#ifndef XYSMOOTHCURVE_H
#define XYSMOOTHCURVE_H

#include "backend/worksheet/plots/cartesian/XYAnalysisCurve.h"

extern "C" {
#include "backend/nsl/nsl_smooth.h"
}

class XYSmoothCurvePrivate;

class XYSmoothCurve : public XYAnalysisCurve {
	Q_OBJECT

public:
	struct SmoothData {
		SmoothData() : xRange(2) {};

		nsl_smooth_type type{nsl_smooth_type_moving_average};		// type of smoothing
		size_t points{5};			// number of points
		nsl_smooth_weight_type weight{nsl_smooth_weight_uniform};	// type of weight
		double percentile{0.5};			// percentile for percentile filter (0.0 .. 1.0)
		int order{2};				// order for Savitzky-Golay filter
		nsl_smooth_pad_mode mode{nsl_smooth_pad_none};		// mode of padding for edges
		double lvalue{0.0}, rvalue{0.0};	// values for constant padding
		bool autoRange{true};			// use all data?
		QVector<double> xRange;			// x range for integration
	};
	struct SmoothResult {
		SmoothResult() {};

		bool available{false};
		bool valid{false};
		QString status;
		qint64 elapsedTime{0};
	};

	explicit XYSmoothCurve(const QString& name);
	~XYSmoothCurve() override;

	void recalculate() override;
	QIcon icon() const override;
	void save(QXmlStreamWriter*) const override;
	bool load(XmlStreamReader*, bool preview) override;

	CLASS_D_ACCESSOR_DECL(SmoothData, smoothData, SmoothData)
	const SmoothResult& smoothResult() const;

	typedef XYSmoothCurvePrivate Private;

protected:
	XYSmoothCurve(const QString& name, XYSmoothCurvePrivate* dd);

private:
	Q_DECLARE_PRIVATE(XYSmoothCurve)

signals:
	void smoothDataChanged(const XYSmoothCurve::SmoothData&);
};

#endif
