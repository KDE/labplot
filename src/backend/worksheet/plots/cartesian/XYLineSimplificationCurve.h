/*
	File                 : XYLineSimplificationCurve.h
	Project              : LabPlot
	Description          : A xy-curve defined by a line simplification
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2016 Stefan Gerlach <stefan.gerlach@uni.kn>
	SPDX-FileCopyrightText: 2017 Alexander Semke <alexander.semke@web.de>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef XYLINESIMPLIFICATIONCURVE_H
#define XYLINESIMPLIFICATIONCURVE_H

#include "backend/worksheet/plots/cartesian/XYAnalysisCurve.h"

extern "C" {
#include "backend/nsl/nsl_geom_linesim.h"
}

class XYLineSimplificationCurvePrivate;

#ifdef SDK
#include "labplot_export.h"
class LABPLOT_EXPORT XYLineSimplificationCurve : public XYAnalysisCurve {
#else
class XYLineSimplificationCurve : public XYAnalysisCurve {
#endif
	Q_OBJECT

public:
	struct LineSimplificationData {
		LineSimplificationData() { };

		nsl_geom_linesim_type type{nsl_geom_linesim_type_douglas_peucker_variant}; // type of simplification
		bool autoTolerance{true}; // automatic tolerance
		double tolerance{0.0}; // tolerance
		bool autoTolerance2{true}; // automatic tolerance2
		double tolerance2{0.0}; // tolerance2
		bool autoRange{true}; // use all data?
		// TODO: use Range
		QVector<double> xRange{0., 0.}; // x range for integration
	};
	struct LineSimplificationResult : public XYAnalysisCurve::Result {
		LineSimplificationResult() { };

		size_t npoints{0};
		double posError{0};
		double areaError{0};
	};

	explicit XYLineSimplificationCurve(const QString& name);
	~XYLineSimplificationCurve() override;

	const XYAnalysisCurve::Result& result() const override;
	QIcon icon() const override;
	void save(QXmlStreamWriter*) const override;
	bool load(XmlStreamReader*, bool preview) override;

	CLASS_D_ACCESSOR_DECL(LineSimplificationData, lineSimplificationData, LineSimplificationData)
	const LineSimplificationResult& lineSimplificationResult() const;

	typedef XYLineSimplificationCurvePrivate Private;

protected:
	XYLineSimplificationCurve(const QString& name, XYLineSimplificationCurvePrivate* dd);

private:
	Q_DECLARE_PRIVATE(XYLineSimplificationCurve)

Q_SIGNALS:
	void lineSimplificationDataChanged(const XYLineSimplificationCurve::LineSimplificationData&);
	void completed(int); //!< int ranging from 0 to 100 notifies about the status of the analysis process
};

#endif
