/***************************************************************************
    File                 : XYDifferentiationCurve.h
    Project              : LabPlot
    Description          : A xy-curve defined by an differentiation
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

#ifndef XYDIFFERENTIATIONCURVE_H
#define XYDIFFERENTIATIONCURVE_H

#include "backend/worksheet/plots/cartesian/XYCurve.h"
extern "C" {
#include "backend/nsl/nsl_diff.h"
}

class XYDifferentiationCurvePrivate;
class XYDifferentiationCurve: public XYCurve {
	Q_OBJECT

	public:
		struct DifferentiationData {
			DifferentiationData() : derivOrder(nsl_diff_deriv_order_first), accOrder(2), autoRange(true), xRange(2) {};

			nsl_diff_deriv_order_type derivOrder;	// order of differentiation
			int accOrder;				// order ofaccuracy
			bool autoRange;				// use all data?
			QVector<double> xRange;			// x range for integration
		};
		struct DifferentiationResult {
			DifferentiationResult() : available(false), valid(false), elapsedTime(0) {};

			bool available;
			bool valid;
			QString status;
			qint64 elapsedTime;
		};

		explicit XYDifferentiationCurve(const QString& name);
		virtual ~XYDifferentiationCurve();

		void recalculate();
		virtual QIcon icon() const;
		virtual void save(QXmlStreamWriter*) const;
		virtual bool load(XmlStreamReader*);

		POINTER_D_ACCESSOR_DECL(const AbstractColumn, xDataColumn, XDataColumn)
		POINTER_D_ACCESSOR_DECL(const AbstractColumn, yDataColumn, YDataColumn)
		const QString& xDataColumnPath() const;
		const QString& yDataColumnPath() const;

		CLASS_D_ACCESSOR_DECL(DifferentiationData, differentiationData, DifferentiationData)
		const DifferentiationResult& differentiationResult() const;

		typedef WorksheetElement BaseClass;
		typedef XYDifferentiationCurvePrivate Private;

	protected:
		XYDifferentiationCurve(const QString& name, XYDifferentiationCurvePrivate* dd);

	private:
		Q_DECLARE_PRIVATE(XYDifferentiationCurve)
		void init();

	signals:
		friend class XYDifferentiationCurveSetXDataColumnCmd;
		friend class XYDifferentiationCurveSetYDataColumnCmd;
		void xDataColumnChanged(const AbstractColumn*);
		void yDataColumnChanged(const AbstractColumn*);

		friend class XYDifferentiationCurveSetDifferentiationDataCmd;
		void differentiationDataChanged(const XYDifferentiationCurve::DifferentiationData&);
		void sourceDataChangedSinceLastDifferentiation();
};

#endif
