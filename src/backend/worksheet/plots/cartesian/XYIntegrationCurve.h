/***************************************************************************
    File                 : XYIntegrationCurve.h
    Project              : LabPlot
    Description          : A xy-curve defined by an integration
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
		IntegrationData() : xRange(2) {};

		nsl_int_method_type method{nsl_int_method_trapezoid};	// method for integration
		bool absolute{false};		// absolute area?
		bool autoRange{true};		// use all data?
		QVector<double> xRange;		// x range for integration
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
