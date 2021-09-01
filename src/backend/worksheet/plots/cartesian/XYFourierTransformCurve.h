/*
    File                 : XYFourierTransformCurve.h
    Project              : LabPlot
    Description          : A xy-curve defined by a Fourier transform
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2016-2021 Stefan Gerlach <stefan.gerlach@uni.kn>
    SPDX-FileCopyrightText: 2017 Alexander Semke <alexander.semke@web.de>
    SPDX-License-Identifier: GPL-2.0-or-later
*/


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
		TransformData() {};

		nsl_dft_result_type type{nsl_dft_result_magnitude};
		bool twoSided{false};
		bool shifted{false};
		nsl_dft_xscale xScale{nsl_dft_xscale_frequency};
		nsl_sf_window_type windowType{nsl_sf_window_uniform};
		bool autoRange{true};		// use all data?
		//TODO: use Range
		QVector<double> xRange{0, 0};	// x range for transform
	};
	struct TransformResult {
		TransformResult() {};

		bool available{false};
		bool valid{false};
		QString status;
		qint64 elapsedTime{0};
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
