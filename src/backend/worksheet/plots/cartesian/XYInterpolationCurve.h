/***************************************************************************
    File                 : XYInterpolationCurve.h
    Project              : LabPlot
    Description          : A xy-curve defined by an interpolation
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

#ifndef XYINTERPOLATIONCURVE_H
#define XYINTERPOLATIONCURVE_H

#include "backend/worksheet/plots/cartesian/XYCurve.h"
#include <gsl/gsl_version.h>

class XYInterpolationCurvePrivate;
class XYInterpolationCurve: public XYCurve {
	Q_OBJECT

	public:
		enum InterpolationType {Linear,Polynomial,CSpline,CSplinePeriodic,Akima,AkimaPeriodic,Steffen};	// TODO:more
		enum InterpolationEval {Function,Derivative,Derivative2,Integral};
		static constexpr double LIMIT_MAX = 1e15;
		static constexpr double LIMIT_MIN = -1e15;

		struct InterpolationData {
			InterpolationData() : type(Linear), evaluate(Function), npoints(100) {};

			XYInterpolationCurve::InterpolationType type;		// type of interpolation
			XYInterpolationCurve::InterpolationEval evaluate;	// what to evaluate
			unsigned int npoints;					// nr. of points
		};
		struct InterpolationResult {
			InterpolationResult() : available(false), valid(false), elapsedTime(0) {};

			bool available;
			bool valid;
			QString status;
			qint64 elapsedTime;
		};

		explicit XYInterpolationCurve(const QString& name);
		virtual ~XYInterpolationCurve();

		void recalculate();
		virtual QIcon icon() const;
		virtual void save(QXmlStreamWriter*) const;
		virtual bool load(XmlStreamReader*);

		POINTER_D_ACCESSOR_DECL(const AbstractColumn, xDataColumn, XDataColumn)
		POINTER_D_ACCESSOR_DECL(const AbstractColumn, yDataColumn, YDataColumn)
		const QString& xDataColumnPath() const;
		const QString& yDataColumnPath() const;

		CLASS_D_ACCESSOR_DECL(InterpolationData, interpolationData, InterpolationData)
		const InterpolationResult& interpolationResult() const;
		bool isSourceDataChangedSinceLastInterpolation() const;

		typedef WorksheetElement BaseClass;
		typedef XYInterpolationCurvePrivate Private;

	protected:
		XYInterpolationCurve(const QString& name, XYInterpolationCurvePrivate* dd);

	private:
		Q_DECLARE_PRIVATE(XYInterpolationCurve)
		void init();

	private slots:
		void handleSourceDataChanged();

	signals:
		friend class XYInterpolationCurveSetXDataColumnCmd;
		friend class XYInterpolationCurveSetYDataColumnCmd;
		void xDataColumnChanged(const AbstractColumn*);
		void yDataColumnChanged(const AbstractColumn*);

		friend class XYInterpolationCurveSetInterpolationDataCmd;
		void interpolationDataChanged(const XYInterpolationCurve::InterpolationData&);
		void sourceDataChangedSinceLastInterpolation();
};

#endif
