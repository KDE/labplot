/*
    File                 : XYHilbertTransformCurve.h
    Project              : LabPlot
    Description          : A xy-curve defined by a Hilbert transform
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2021 Stefan Gerlach <stefan.gerlach@uni.kn>
    SPDX-License-Identifier: GPL-2.0-or-later
*/


#ifndef XYHILBERTTRANSFORMCURVE_H
#define XYHILBERTTRANSFORMCURVE_H

#include "backend/worksheet/plots/cartesian/XYAnalysisCurve.h"
extern "C" {
#include "backend/nsl/nsl_hilbert.h"
}

class XYHilbertTransformCurvePrivate;

class XYHilbertTransformCurve : public XYAnalysisCurve {
	Q_OBJECT

public:
	struct TransformData {
		TransformData() {};

		nsl_hilbert_result_type type{nsl_hilbert_result_imag};
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

	explicit XYHilbertTransformCurve(const QString& name);
	~XYHilbertTransformCurve() override;

	void recalculate() override;
	QIcon icon() const override;
	void save(QXmlStreamWriter*) const override;
	bool load(XmlStreamReader*, bool preview) override;

	CLASS_D_ACCESSOR_DECL(TransformData, transformData, TransformData)
	const TransformResult& transformResult() const;

	typedef XYHilbertTransformCurvePrivate Private;

protected:
	XYHilbertTransformCurve(const QString& name, XYHilbertTransformCurvePrivate* dd);

private:
	Q_DECLARE_PRIVATE(XYHilbertTransformCurve)

Q_SIGNALS:
	void transformDataChanged(const XYHilbertTransformCurve::TransformData&);
};

#endif
