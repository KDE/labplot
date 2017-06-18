/***************************************************************************
    File                 : XYIntegrationCurve.h
    Project              : LabPlot
    Description          : A xy-curve defined by an integration
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

#ifndef XYINTEGRATIONCURVE_H
#define XYINTEGRATIONCURVE_H

#include "backend/worksheet/plots/cartesian/XYCurve.h"
extern "C" {
#include "backend/nsl/nsl_int.h"
}

class XYIntegrationCurvePrivate;
class XYIntegrationCurve: public XYCurve {
	Q_OBJECT

	public:
		struct IntegrationData {
			IntegrationData() : method(nsl_int_method_trapezoid), absolute(false), autoRange(true), xRange(2) {};

			nsl_int_method_type method;	// method for integration
			bool absolute;			// absolute area?
			bool autoRange;			// use all data?
			QVector<double> xRange;		// x range for integration
		};
		struct IntegrationResult {
			IntegrationResult() : available(false), valid(false), elapsedTime(0), value(0) {};

			bool available;
			bool valid;
			QString status;
			qint64 elapsedTime;
			double value;	// final result of integration
		};

		explicit XYIntegrationCurve(const QString& name);
		virtual ~XYIntegrationCurve();

		void recalculate();
		virtual QIcon icon() const;
		virtual void save(QXmlStreamWriter*) const;
		virtual bool load(XmlStreamReader*);

		POINTER_D_ACCESSOR_DECL(const AbstractColumn, xDataColumn, XDataColumn)
		POINTER_D_ACCESSOR_DECL(const AbstractColumn, yDataColumn, YDataColumn)
		const QString& xDataColumnPath() const;
		const QString& yDataColumnPath() const;

		CLASS_D_ACCESSOR_DECL(IntegrationData, integrationData, IntegrationData)
		const IntegrationResult& integrationResult() const;

		typedef XYIntegrationCurvePrivate Private;

	protected:
		XYIntegrationCurve(const QString& name, XYIntegrationCurvePrivate* dd);

	private:
		Q_DECLARE_PRIVATE(XYIntegrationCurve)
		void init();

	signals:
		friend class XYIntegrationCurveSetXDataColumnCmd;
		friend class XYIntegrationCurveSetYDataColumnCmd;
		void xDataColumnChanged(const AbstractColumn*);
		void yDataColumnChanged(const AbstractColumn*);

		friend class XYIntegrationCurveSetIntegrationDataCmd;
		void integrationDataChanged(const XYIntegrationCurve::IntegrationData&);
};

#endif
