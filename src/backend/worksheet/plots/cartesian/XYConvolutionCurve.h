/*
    File                 : XYConvolutionCurve.h
    Project              : LabPlot
    Description          : A xy-curve defined by a convolution
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2018 Stefan Gerlach (stefan.gerlach@uni.kn)
    SPDX-License-Identifier: GPL-2.0-or-later
*/


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
		ConvolutionData() {};

		double samplingInterval{1.};				// sampling interval used when no x-axis is present
		nsl_conv_kernel_type kernel{nsl_conv_kernel_avg};	// kernel to use when no response selected
		size_t kernelSize{2};					// size of kernel
		nsl_conv_direction_type direction{nsl_conv_direction_forward};	// forward (convolution) or backward (deconvolution)
		nsl_conv_type_type type{nsl_conv_type_linear};		// linear or circular
		nsl_conv_method_type method{nsl_conv_method_auto};	// how to calculate convolution (auto, direct or FFT method)
		nsl_conv_norm_type normalize{nsl_conv_norm_none};	// normalization of response
		nsl_conv_wrap_type wrap{nsl_conv_wrap_none};		// wrap response
		bool autoRange{true};					// use all data?
		//TODO: use Range
		QVector<double> xRange{0., 0.};				// x range for convolution
	};
	struct ConvolutionResult {
		ConvolutionResult() {};

		bool available{false};
		bool valid{false};
		QString status;
		qint64 elapsedTime{0};
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
