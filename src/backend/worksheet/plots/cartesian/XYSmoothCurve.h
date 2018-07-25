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
		SmoothData() : type(nsl_smooth_type_moving_average), points(5), weight(nsl_smooth_weight_uniform), percentile(0.5), order(2),
			mode(nsl_smooth_pad_none), lvalue(0.0), rvalue(0.0), autoRange(true), xRange(2) {};

		nsl_smooth_type type;			// type of smoothing
		size_t points;				// number of points
		nsl_smooth_weight_type weight;		// type of weight
		double percentile;			// percentile for percentile filter (0.0 .. 1.0)
		int order;				// order for Savitzky-Golay filter
		nsl_smooth_pad_mode mode;		// mode of padding for edges
		double lvalue, rvalue;			// values for constant padding
		bool autoRange;				// use all data?
		QVector<double> xRange;			// x range for integration
	};
	struct SmoothResult {
		SmoothResult() : available(false), valid(false), elapsedTime(0) {};

		bool available;
		bool valid;
		QString status;
		qint64 elapsedTime;
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
