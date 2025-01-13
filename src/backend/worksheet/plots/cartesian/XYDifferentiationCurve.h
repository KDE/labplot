/*
	File                 : XYDifferentiationCurve.h
	Project              : LabPlot
	Description          : A xy-curve defined by an differentiation
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2016-2021 Stefan Gerlach <stefan.gerlach@uni.kn>
	SPDX-FileCopyrightText: 2017 Alexander Semke <alexander.semke@web.de>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef XYDIFFERENTIATIONCURVE_H
#define XYDIFFERENTIATIONCURVE_H

#include "backend/worksheet/plots/cartesian/XYAnalysisCurve.h"

extern "C" {
#include "backend/nsl/nsl_diff.h"
}

class XYDifferentiationCurvePrivate;

#ifdef SDK
#include "labplot_export.h"
class LABPLOT_EXPORT XYDifferentiationCurve : public XYAnalysisCurve {
#else
class XYDifferentiationCurve : public XYAnalysisCurve {
#endif
	Q_OBJECT

public:
	struct DifferentiationData {
		DifferentiationData() {};

		nsl_diff_deriv_order_type derivOrder{nsl_diff_deriv_order_first}; // order of differentiation
		int accOrder{2}; // order of accuracy
		bool autoRange{true}; // use all data?
		// TODO: use Range
		QVector<double> xRange{0., 0.}; // x range for integration
	};

	explicit XYDifferentiationCurve(const QString& name);
	~XYDifferentiationCurve() override;

	virtual const Result& result() const override;
	QIcon icon() const override;
	void save(QXmlStreamWriter*) const override;
	bool load(XmlStreamReader*, bool preview) override;

	CLASS_D_ACCESSOR_DECL(DifferentiationData, differentiationData, DifferentiationData)

	typedef XYAnalysisCurve::Result DifferentiationResult;
	const DifferentiationResult& differentiationResult() const;

	typedef XYDifferentiationCurvePrivate Private;

protected:
	XYDifferentiationCurve(const QString& name, XYDifferentiationCurvePrivate* dd);

private:
	Q_DECLARE_PRIVATE(XYDifferentiationCurve)

Q_SIGNALS:
	void differentiationDataChanged(const XYDifferentiationCurve::DifferentiationData&);
};

#endif
