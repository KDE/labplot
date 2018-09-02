/***************************************************************************
    File                 : XYConvolutionCurve.h
    Project              : LabPlot
    Description          : A xy-curve defined by a convolution
    --------------------------------------------------------------------
    Copyright            : (C) 2018 Stefan Gerlach (stefan.gerlach@uni.kn)

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

#ifndef XYCONVOLUTIONCURVE_H
#define XYCONVOLUTIONCURVE_H

#include "backend/worksheet/plots/cartesian/XYAnalysisCurve.h"

class XYConvolutionCurvePrivate;
class XYConvolutionCurve : public XYAnalysisCurve {
Q_OBJECT

public:
	struct ConvolutionData {
		ConvolutionData() : absolute(false), autoRange(true), xRange(2) {};

		//TODO: check options
		bool absolute;			// absolute area?
		bool autoRange;			// use all data?
		QVector<double> xRange;		// x range for convolution
	};
	struct ConvolutionResult {
		ConvolutionResult() : available(false), valid(false), elapsedTime(0), value(0) {};

		bool available;
		bool valid;
		QString status;
		qint64 elapsedTime;
		double value;	// final result of convolution
	};

	explicit XYConvolutionCurve(const QString& name);
	~XYConvolutionCurve() override;

	void recalculate() override;
	QIcon icon() const override;
	void save(QXmlStreamWriter*) const override;
	bool load(XmlStreamReader*, bool preview) override;

	POINTER_D_ACCESSOR_DECL(const AbstractColumn, y2DataColumn, Y2DataColumn)
	const QString& y2DataColumnPath() const;
	CLASS_D_ACCESSOR_DECL(ConvolutionData, convolutionData, ConvolutionData)
	const ConvolutionResult& convolutionResult() const;

	typedef XYConvolutionCurvePrivate Private;

protected:
	XYConvolutionCurve(const QString& name, XYConvolutionCurvePrivate* dd);

private:
	Q_DECLARE_PRIVATE(XYConvolutionCurve)

signals:
	void y2DataColumnChanged(const AbstractColumn*);
	void convolutionDataChanged(const XYConvolutionCurve::ConvolutionData&);
};

#endif
