/***************************************************************************
    File                 : XYSmoothCurve.h
    Project              : LabPlot
    Description          : A xy-curve defined by a smooth
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

#ifndef XYSMOOTHCURVE_H
#define XYSMOOTHCURVE_H

#include "backend/worksheet/plots/cartesian/XYCurve.h"

extern "C" {
#include "backend/nsl/nsl_smooth.h"
}

class XYSmoothCurvePrivate;
class XYSmoothCurve: public XYCurve {
	Q_OBJECT

	public:
		enum SmoothType {MovingAverage,MovingAverageLagged,Percentile,SavitzkyGolay}; //TODO: LOWESS/etc., Bezier, B-Spline, (FFT Filter)

		struct SmoothData {
			SmoothData() : type(MovingAverage), points(5), weight(nsl_smooth_weight_uniform), percentile(0.5), order(2),
				mode(nsl_smooth_pad_interp), lvalue(0.0), rvalue(0.0) {};

			XYSmoothCurve::SmoothType type;		// type of smoothing
			unsigned int points;			// number of points
			nsl_smooth_weight_type weight;		// type of weight
			double percentile;			// percentile for percentile filter (0.0 .. 1.0)
			unsigned order;				// order for Savitzky-Golay filter
			nsl_smooth_pad_mode mode;		// mode of padding for edges
			double lvalue, rvalue;			// values for constant padding
		};
		struct SmoothResult {
			SmoothResult() : available(false), valid(false), elapsedTime(0) {};

			bool available;
			bool valid;
			QString status;
			qint64 elapsedTime;
		};

		explicit XYSmoothCurve(const QString& name);
		virtual ~XYSmoothCurve();

		void recalculate();
		virtual QIcon icon() const;
		virtual void save(QXmlStreamWriter*) const;
		virtual bool load(XmlStreamReader*);

		POINTER_D_ACCESSOR_DECL(const AbstractColumn, xDataColumn, XDataColumn)
		POINTER_D_ACCESSOR_DECL(const AbstractColumn, yDataColumn, YDataColumn)
		const QString& xDataColumnPath() const;
		const QString& yDataColumnPath() const;

		CLASS_D_ACCESSOR_DECL(SmoothData, smoothData, SmoothData)
		const SmoothResult& smoothResult() const;
		bool isSourceDataChangedSinceLastSmooth() const;

		typedef WorksheetElement BaseClass;
		typedef XYSmoothCurvePrivate Private;

	protected:
		XYSmoothCurve(const QString& name, XYSmoothCurvePrivate* dd);

	private:
		Q_DECLARE_PRIVATE(XYSmoothCurve)
		void init();

	private slots:
		void handleSourceDataChanged();

	signals:
		friend class XYSmoothCurveSetXDataColumnCmd;
		friend class XYSmoothCurveSetYDataColumnCmd;
		void xDataColumnChanged(const AbstractColumn*);
		void yDataColumnChanged(const AbstractColumn*);

		friend class XYSmoothCurveSetSmoothDataCmd;
		void smoothDataChanged(const XYSmoothCurve::SmoothData&);
		void sourceDataChangedSinceLastSmooth();
};

#endif
