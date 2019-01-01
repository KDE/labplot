/***************************************************************************
    File                 : XYDifferentiationCurve.h
    Project              : LabPlot
    Description          : A xy-curve defined by an differentiation
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
		DifferentiationData() : xRange(2) {};

		nsl_diff_deriv_order_type derivOrder{nsl_diff_deriv_order_first};	// order of differentiation
		int accOrder{2};							// order of accuracy
		bool autoRange{true};							// use all data?
		QVector<double> xRange;							// x range for integration
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
