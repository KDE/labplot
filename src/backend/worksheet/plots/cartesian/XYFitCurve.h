/***************************************************************************
    File                 : XYFitCurve.h
    Project              : LabPlot
    Description          : A xy-curve defined by a fit model
    --------------------------------------------------------------------
    Copyright            : (C) 2014 Alexander Semke (alexander.semke@web.de)

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

#ifndef XYFITCURVE_H
#define XYFITCURVE_H

#include "backend/worksheet/plots/cartesian/XYCurve.h"

class XYFitCurvePrivate;
class XYFitCurve: public XYCurve {
	Q_OBJECT

	public:
		enum FitType {Cartesian, Polar, Parametric, Implicit};

		struct FitData {
			FitData() : type(Cartesian) {};

			FitType type;

		};

		explicit XYFitCurve(const QString& name);
		virtual ~XYFitCurve();

		void recalculate();
		virtual QIcon icon() const;
		virtual void save(QXmlStreamWriter*) const;
		virtual bool load(XmlStreamReader*);

		CLASS_D_ACCESSOR_DECL(FitData, fitData, FitData)

		typedef AbstractWorksheetElement BaseClass;
		typedef XYFitCurvePrivate Private;

	protected:
		XYFitCurve(const QString& name, XYFitCurvePrivate* dd);

	private:
		Q_DECLARE_PRIVATE(XYFitCurve)
		void init();

	signals:
		friend class XYFitCurveSetFitDataCmd;
		void fitDataChanged(const XYFitCurve::FitData&);
};

#endif
