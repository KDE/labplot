/***************************************************************************
    File                 : XYEquationCurve.h
    Project              : LabPlot
    Description          : A xy-curve defined by a mathematical equation
    --------------------------------------------------------------------
    Copyright            : (C) 2014-2017 Alexander Semke (alexander.semke@web.de)

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

#ifndef XYEQUATIONCURVE_H
#define XYEQUATIONCURVE_H

#include "backend/worksheet/plots/cartesian/XYCurve.h"

class XYEquationCurvePrivate;
class XYEquationCurve : public XYCurve {
Q_OBJECT

public:
	enum EquationType {Cartesian, Polar, Parametric, Implicit, Neutral};

	struct EquationData {
		EquationData() : type(Cartesian), min("0"), max("1"), count(1000) {};

		EquationType type;
		QString expression1;
		QString expression2;
		QString min;
		QString max;
		int count;
	};

	explicit XYEquationCurve(const QString& name);
	~XYEquationCurve() override;

	void recalculate();
	QIcon icon() const override;
	void save(QXmlStreamWriter*) const override;
	bool load(XmlStreamReader*, bool preview) override;

	CLASS_D_ACCESSOR_DECL(EquationData, equationData, EquationData)

	typedef XYEquationCurvePrivate Private;

protected:
	XYEquationCurve(const QString& name, XYEquationCurvePrivate* dd);

private:
	Q_DECLARE_PRIVATE(XYEquationCurve)
	void init();

signals:
	void equationDataChanged(const XYEquationCurve::EquationData&);
};

#endif
