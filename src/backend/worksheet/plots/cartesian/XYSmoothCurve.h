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

class XYSmoothCurvePrivate;
class XYSmoothCurve: public XYCurve {
	Q_OBJECT

	public:
		enum SmoothType {MovingAverage};
		enum WeightType {Uniform, Triangular, Binomial, Parabolic, Quartic, Triweight, Tricube, Cosine};

		struct SmoothData {
			SmoothData() : type(MovingAverage), points(3), weight(Uniform) {};

			XYSmoothCurve::SmoothType type;		// type of smooth
			unsigned int points;			// number of points
			XYSmoothCurve::WeightType weight;	// type of weight
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
