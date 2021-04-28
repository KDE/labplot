/***************************************************************************
    File                 : XYHilbertTransformCurve.h
    Project              : LabPlot
    Description          : A xy-curve defined by a Hilbert transform
    --------------------------------------------------------------------
    Copyright            : (C) 2021 Stefan Gerlach (stefan.gerlach@uni.kn)

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

signals:
	void transformDataChanged(const XYHilbertTransformCurve::TransformData&);
};

#endif
