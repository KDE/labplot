/*
    File                 : XYDifferentiationCurve.h
    Project              : LabPlot
    Description          : A xy-curve defined by an differentiation
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2016-2021 Stefan Gerlach (stefan.gerlach@uni.kn)
    SPDX-FileCopyrightText: 2017 Alexander Semke (alexander.semke@web.de)
    SPDX-License-Identifier: GPL-2.0-or-later
*/


#ifndef XYDIFFERENTIATIONCURVE_H
#define XYDIFFERENTIATIONCURVE_H

#include "backend/worksheet/plots/cartesian/XYAnalysisCurve.h"

extern "C" {
#include "backend/nsl/nsl_diff.h"
}

class XYDifferentiationCurvePrivate;

class XYDifferentiationCurve : public XYAnalysisCurve {
	Q_OBJECT

public:
	struct DifferentiationData {
		DifferentiationData() {};

		nsl_diff_deriv_order_type derivOrder{nsl_diff_deriv_order_first};	// order of differentiation
		int accOrder{2};							// order of accuracy
		bool autoRange{true};							// use all data?
		//TODO: use Range
		QVector<double> xRange{0., 0.};						// x range for integration
	};
	struct DifferentiationResult {
		DifferentiationResult() {};

		bool available{false};
		bool valid{false};
		QString status;
		qint64 elapsedTime{0};
	};

	explicit XYDifferentiationCurve(const QString& name);
	~XYDifferentiationCurve() override;

	void recalculate() override;
	QIcon icon() const override;
	void save(QXmlStreamWriter*) const override;
	bool load(XmlStreamReader*, bool preview) override;

	CLASS_D_ACCESSOR_DECL(DifferentiationData, differentiationData, DifferentiationData)
	const DifferentiationResult& differentiationResult() const;

	typedef XYDifferentiationCurvePrivate Private;

protected:
	XYDifferentiationCurve(const QString& name, XYDifferentiationCurvePrivate* dd);

private:
	Q_DECLARE_PRIVATE(XYDifferentiationCurve)

signals:
	void differentiationDataChanged(const XYDifferentiationCurve::DifferentiationData&);
};

#endif
