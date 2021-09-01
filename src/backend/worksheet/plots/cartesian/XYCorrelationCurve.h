/*
    File                 : XYCorrelationCurve.h
    Project              : LabPlot
    Description          : A xy-curve defined by a correlation
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2018 Stefan Gerlach (stefan.gerlach@uni.kn)
    SPDX-License-Identifier: GPL-2.0-or-later
*/


#ifndef XYCORRELATIONCURVE_H
#define XYCORRELATIONCURVE_H

#include "backend/worksheet/plots/cartesian/XYAnalysisCurve.h"

extern "C" {
#include "backend/nsl/nsl_corr.h"
}

class XYCorrelationCurvePrivate;

class XYCorrelationCurve : public XYAnalysisCurve {
	Q_OBJECT

public:
	struct CorrelationData {
		CorrelationData() {};

		double samplingInterval{1.};			// sampling interval used when no x-axis is present
		nsl_corr_type_type type{nsl_corr_type_linear};	// linear or circular
		nsl_corr_norm_type normalize{nsl_corr_norm_none};	// normalization
		bool autoRange{true};				// use all data?
		//TODO: use Range
		QVector<double> xRange{0., 0.};			// x range for correlation
	};
	struct CorrelationResult {
		CorrelationResult() {};

		bool available{false};
		bool valid{false};
		QString status;
		qint64 elapsedTime{0};
	};

	explicit XYCorrelationCurve(const QString& name);
	~XYCorrelationCurve() override;

	void recalculate() override;
	QIcon icon() const override;
	void save(QXmlStreamWriter*) const override;
	bool load(XmlStreamReader*, bool preview) override;

	CLASS_D_ACCESSOR_DECL(CorrelationData, correlationData, CorrelationData)
	const CorrelationResult& correlationResult() const;

	typedef XYCorrelationCurvePrivate Private;

protected:
	XYCorrelationCurve(const QString& name, XYCorrelationCurvePrivate* dd);

private:
	Q_DECLARE_PRIVATE(XYCorrelationCurve)

signals:
	void correlationDataChanged(const XYCorrelationCurve::CorrelationData&);
};

#endif
