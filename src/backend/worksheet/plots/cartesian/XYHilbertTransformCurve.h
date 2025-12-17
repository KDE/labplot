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

#ifdef SDK
#include "labplot_export.h"
class LABPLOT_EXPORT XYHilbertTransformCurve : public XYAnalysisCurve {
#else
class XYHilbertTransformCurve : public XYAnalysisCurve {
#endif
	Q_OBJECT

public:
	struct TransformData {
		TransformData() { };

		nsl_hilbert_result_type type{nsl_hilbert_result_imag};
		bool autoRange{true}; // use all data?
		// TODO: use Range
		QVector<double> xRange{0, 0}; // x range for transform
	};

	explicit XYHilbertTransformCurve(const QString& name);
	~XYHilbertTransformCurve() override;

	virtual const XYAnalysisCurve::Result& result() const override;
	QIcon icon() const override;
	void save(QXmlStreamWriter*) const override;
	bool load(XmlStreamReader*, bool preview) override;

	CLASS_D_ACCESSOR_DECL(TransformData, transformData, TransformData)

	typedef XYAnalysisCurve::Result TransformResult;

	typedef XYHilbertTransformCurvePrivate Private;

protected:
	XYHilbertTransformCurve(const QString& name, XYHilbertTransformCurvePrivate* dd);

private:
	Q_DECLARE_PRIVATE(XYHilbertTransformCurve)

Q_SIGNALS:
	void transformDataChanged(const XYHilbertTransformCurve::TransformData&);
};

#endif
