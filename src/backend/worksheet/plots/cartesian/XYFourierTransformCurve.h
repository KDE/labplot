/***************************************************************************
    File                 : XYFourierTransformCurve.h
    Project              : LabPlot
    Description          : A xy-curve defined by a Fourier transform
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

#ifndef XYFOURIERTRANSFORMCURVE_H
#define XYFOURIERTRANSFORMCURVE_H

#include "backend/worksheet/plots/cartesian/XYAnalysisCurve.h"
extern "C" {
#include "backend/nsl/nsl_dft.h"
#include "backend/nsl/nsl_sf_window.h"
}

class XYFourierTransformCurvePrivate;
class XYFourierTransformCurve : public XYAnalysisCurve {
Q_OBJECT

public:
	struct TransformData {
		TransformData() : type(nsl_dft_result_magnitude), twoSided(false), shifted(false),
			xScale(nsl_dft_xscale_frequency), windowType(nsl_sf_window_uniform), autoRange(true), xRange(2) {};

		nsl_dft_result_type type;
		bool twoSided;
		bool shifted;
		nsl_dft_xscale xScale;
		nsl_sf_window_type windowType;
		bool autoRange;			// use all data?
		QVector<double> xRange;		// x range for transform
	};
	struct TransformResult {
		TransformResult() : available(false), valid(false), elapsedTime(0) {};

		bool available;
		bool valid;
		QString status;
		qint64 elapsedTime;
	};

	explicit XYFourierTransformCurve(const QString& name);
	~XYFourierTransformCurve() override;

	void recalculate() override;
	QIcon icon() const override;
	void save(QXmlStreamWriter*) const override;
	bool load(XmlStreamReader*, bool preview) override;

	CLASS_D_ACCESSOR_DECL(TransformData, transformData, TransformData)
	const TransformResult& transformResult() const;

	typedef XYFourierTransformCurvePrivate Private;

protected:
	XYFourierTransformCurve(const QString& name, XYFourierTransformCurvePrivate* dd);

private:
	Q_DECLARE_PRIVATE(XYFourierTransformCurve)

signals:
	void transformDataChanged(const XYFourierTransformCurve::TransformData&);
};

#endif
