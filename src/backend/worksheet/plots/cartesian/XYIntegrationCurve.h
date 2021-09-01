/*
    File                 : XYIntegrationCurve.h
    Project              : LabPlot
    Description          : A xy-curve defined by an integration
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2016-2021 Stefan Gerlach (stefan.gerlach@uni.kn)
    SPDX-FileCopyrightText: 2017 Alexander Semke (alexander.semke@web.de)
    SPDX-License-Identifier: GPL-2.0-or-later
*/


#ifndef XYINTEGRATIONCURVE_H
#define XYINTEGRATIONCURVE_H

#include "backend/worksheet/plots/cartesian/XYAnalysisCurve.h"
extern "C" {
#include "backend/nsl/nsl_int.h"
}

class XYIntegrationCurvePrivate;

class XYIntegrationCurve : public XYAnalysisCurve {
	Q_OBJECT

public:
	struct IntegrationData {
		IntegrationData() {};

		nsl_int_method_type method{nsl_int_method_trapezoid};	// method for integration
		bool absolute{false};		// absolute area?
		bool autoRange{true};		// use all data?
		//TODO: use Range
		QVector<double> xRange{0, 0};	// x range for integration
	};
	struct IntegrationResult {
		IntegrationResult() {};

		bool available{false};
		bool valid{false};
		QString status;
		qint64 elapsedTime{0};
		double value{0.0};	// final result of integration
	};

	explicit XYIntegrationCurve(const QString& name);
	~XYIntegrationCurve() override;

	void recalculate() override;
	QIcon icon() const override;
	void save(QXmlStreamWriter*) const override;
	bool load(XmlStreamReader*, bool preview) override;

	CLASS_D_ACCESSOR_DECL(IntegrationData, integrationData, IntegrationData)
	const IntegrationResult& integrationResult() const;

	typedef XYIntegrationCurvePrivate Private;

protected:
	XYIntegrationCurve(const QString& name, XYIntegrationCurvePrivate* dd);

private:
	Q_DECLARE_PRIVATE(XYIntegrationCurve)

signals:
	void integrationDataChanged(const XYIntegrationCurve::IntegrationData&);
};

#endif
