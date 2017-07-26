/***************************************************************************
    File                 : XYDataReductionCurve.h
    Project              : LabPlot
    Description          : A xy-curve defined by a data reduction
    --------------------------------------------------------------------
    Copyright            : (C) 2016 Stefan Gerlach (stefan.gerlach@uni.kn)

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

#include "backend/worksheet/plots/cartesian/XYCurve.h"
extern "C" {
#include "backend/nsl/nsl_geom_linesim.h"
}

class XYDataReductionCurvePrivate;
class XYDataReductionCurve: public XYCurve {
	Q_OBJECT

public:
	struct DataReductionData {
		DataReductionData() : type(nsl_geom_linesim_type_douglas_peucker_variant), autoTolerance(true), tolerance(0.0),
			autoTolerance2(true), tolerance2(0.0), autoRange(true), xRange(2) {};

		nsl_geom_linesim_type type;	// type of simplification
		bool autoTolerance;		// automatic tolerance
		double tolerance;		// tolerance
		bool autoTolerance2;		// automatic tolerance2
		double tolerance2;		// tolerance2
		bool autoRange;			// use all data?
		QVector<double> xRange;		// x range for integration
	};
	struct DataReductionResult {
		DataReductionResult() : available(false), valid(false), elapsedTime(0), npoints(0), posError(0), areaError(0) {};

		bool available;
		bool valid;
		QString status;
		qint64 elapsedTime;
		size_t npoints;
		double posError;
		double areaError;
	};

	explicit XYDataReductionCurve(const QString& name);
	virtual ~XYDataReductionCurve();

	void recalculate();
	virtual QIcon icon() const;
	virtual void save(QXmlStreamWriter*) const;
	virtual bool load(XmlStreamReader*);

	POINTER_D_ACCESSOR_DECL(const AbstractColumn, xDataColumn, XDataColumn)
	POINTER_D_ACCESSOR_DECL(const AbstractColumn, yDataColumn, YDataColumn)
	const QString& xDataColumnPath() const;
	const QString& yDataColumnPath() const;

	CLASS_D_ACCESSOR_DECL(DataReductionData, dataReductionData, DataReductionData)
	const DataReductionResult& dataReductionResult() const;

	typedef XYDataReductionCurvePrivate Private;

protected:
	XYDataReductionCurve(const QString& name, XYDataReductionCurvePrivate* dd);

private:
	Q_DECLARE_PRIVATE(XYDataReductionCurve)
	void init();

signals:
	friend class XYDataReductionCurveSetXDataColumnCmd;
	friend class XYDataReductionCurveSetYDataColumnCmd;
	void xDataColumnChanged(const AbstractColumn*);
	void yDataColumnChanged(const AbstractColumn*);

	friend class XYDataReductionCurveSetDataReductionDataCmd;
	void dataReductionDataChanged(const XYDataReductionCurve::DataReductionData&);
	void completed(int) const; //!< int ranging from 0 to 100 notifies about the status of the analysis process
};

#endif
