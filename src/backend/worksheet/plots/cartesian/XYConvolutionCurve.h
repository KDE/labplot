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

extern "C" {
#include "backend/nsl/nsl_conv.h"
}

class XYConvolutionCurvePrivate;
class XYConvolutionCurve : public XYAnalysisCurve {
Q_OBJECT

public:
	struct ConvolutionData {
		ConvolutionData() : samplingInterval(1.), kernel(nsl_conv_kernel_avg), kernelSize(2), direction(nsl_conv_direction_forward), type(nsl_conv_type_linear),
			method(nsl_conv_method_auto), normalize(nsl_conv_norm_none), wrap(nsl_conv_wrap_none), autoRange(true), xRange(2) {};

		double samplingInterval;	// sampling interval used when no x-axis is present
		nsl_conv_kernel_type kernel;	// kernel to use when no response selected
		size_t kernelSize;		// size of kernel
		nsl_conv_direction_type direction;	// forward (convolution) or backward (deconvolution)
		nsl_conv_type_type type;	// linear or circular
		nsl_conv_method_type method;	// how to calculate convolution (auto, direct or FFT method)
		nsl_conv_norm_type normalize;	// normalization of response
		nsl_conv_wrap_type wrap;	// wrap response
		bool autoRange;			// use all data?
		QVector<double> xRange;		// x range for convolution
	};
	struct ConvolutionResult {
		ConvolutionResult() : available(false), valid(false), elapsedTime(0) {};

		bool available;
		bool valid;
		QString status;
		qint64 elapsedTime;
	};

	explicit XYConvolutionCurve(const QString& name);
	~XYConvolutionCurve() override;

	void recalculate() override;
	QIcon icon() const override;
	void save(QXmlStreamWriter*) const override;
	bool load(XmlStreamReader*, bool preview) override;

	CLASS_D_ACCESSOR_DECL(ConvolutionData, convolutionData, ConvolutionData)
	const ConvolutionResult& convolutionResult() const;

	typedef XYConvolutionCurvePrivate Private;

protected:
	XYConvolutionCurve(const QString& name, XYConvolutionCurvePrivate* dd);

private:
	Q_DECLARE_PRIVATE(XYConvolutionCurve)

signals:
	void convolutionDataChanged(const XYConvolutionCurve::ConvolutionData&);
};

#endif
