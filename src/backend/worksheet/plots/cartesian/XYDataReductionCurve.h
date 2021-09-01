/*
    File                 : XYDataReductionCurve.h
    Project              : LabPlot
    Description          : A xy-curve defined by a data reduction
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2016 Stefan Gerlach (stefan.gerlach@uni.kn)
    SPDX-FileCopyrightText: 2017 Alexander Semke (alexander.semke@web.de)
    SPDX-License-Identifier: GPL-2.0-or-later
*/


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
