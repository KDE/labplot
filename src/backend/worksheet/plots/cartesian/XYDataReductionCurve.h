/***************************************************************************
    File                 : XYDataReductionCurve.h
    Project              : LabPlot
    Description          : A xy-curve defined by a data reduction
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

#ifndef XYDATAREDUCTIONCURVE_H
#define XYDATAREDUCTIONCURVE_H

#include "backend/worksheet/plots/cartesian/XYAnalysisCurve.h"

extern "C" {
#include "backend/nsl/nsl_geom_linesim.h"
}

class XYDataReductionCurvePrivate;

class XYDataReductionCurve : public XYAnalysisCurve {
	Q_OBJECT

public:
	struct DataReductionData {
		DataReductionData() {};

		nsl_geom_linesim_type type{nsl_geom_linesim_type_douglas_peucker_variant};	// type of simplification
		bool autoTolerance{true};	// automatic tolerance
		double tolerance{0.0};		// tolerance
		bool autoTolerance2{true};	// automatic tolerance2
		double tolerance2{0.0};		// tolerance2
		bool autoRange{true};		// use all data?
		//TODO: use Range
		QVector<double> xRange{0., 0.};		// x range for integration
	};
	struct DataReductionResult {
		DataReductionResult() {};

		bool available{false};
		bool valid{false};
		QString status;
		qint64 elapsedTime{0};
		size_t npoints{0};
		double posError{0};
		double areaError{0};
	};

	explicit XYDataReductionCurve(const QString& name);
	~XYDataReductionCurve() override;

	void recalculate() override;
	QIcon icon() const override;
	void save(QXmlStreamWriter*) const override;
	bool load(XmlStreamReader*, bool preview) override;

	CLASS_D_ACCESSOR_DECL(DataReductionData, dataReductionData, DataReductionData)
	const DataReductionResult& dataReductionResult() const;

	typedef XYDataReductionCurvePrivate Private;

protected:
	XYDataReductionCurve(const QString& name, XYDataReductionCurvePrivate* dd);

private:
	Q_DECLARE_PRIVATE(XYDataReductionCurve)

signals:
	void dataReductionDataChanged(const XYDataReductionCurve::DataReductionData&);
	void completed(int); //!< int ranging from 0 to 100 notifies about the status of the analysis process
};

#endif
