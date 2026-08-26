/*
	File                 : XYPiecewiseLinearFitCurvePrivate.cpp
	Project              : LabPlot
	Description          : Private members of XYPiecewiseLinearFitCurve
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2026 Alexander Semke <alexander.semke@web.de>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "XYPiecewiseLinearFitCurvePrivate.h"
#include "backend/core/column/Column.h"

#include <cmath>

XYPiecewiseLinearFitCurvePrivate::XYPiecewiseLinearFitCurvePrivate(XYPiecewiseLinearFitCurve* owner)
	: XYAnalysisCurvePrivate(owner)
	, q(owner) {
}

XYPiecewiseLinearFitCurvePrivate::~XYPiecewiseLinearFitCurvePrivate() = default;

void XYPiecewiseLinearFitCurvePrivate::resetResults() {
	fitResult = XYPiecewiseLinearFitCurve::FitResult();
}

bool XYPiecewiseLinearFitCurvePrivate::recalculateSpecific(const AbstractColumn* tmpXDataColumn, const AbstractColumn* tmpYDataColumn) {
	fitResult.available = false;
	fitResult.valid = false;
	fitResult.status.clear();

	if (!tmpXDataColumn || !tmpYDataColumn) {
		fitResult.status = QStringLiteral("No data columns provided");
		return false;
	}

	const size_t n = static_cast<size_t>(tmpXDataColumn->rowCount());
	if (n < 2 * fitData.minSegmentSize) {
		fitResult.status = QStringLiteral("Not enough data points");
		return false;
	}

	QVector<double> xData(n);
	QVector<double> yData(n);

	Range<double> xRange = fitData.autoRange ? Range<double>{tmpXDataColumn->minimum(), tmpXDataColumn->maximum()} : fitData.fitRange;

	size_t validPoints = 0;
	for (int i = 0; i < tmpXDataColumn->rowCount(); ++i) {
		double x = tmpXDataColumn->valueAt(i);
		double y = tmpYDataColumn->valueAt(i);

		if (std::isnan(x) || std::isnan(y))
			continue;
		if (!fitData.autoRange && (x < xRange.start() || x > xRange.end()))
			continue;

		xData[validPoints] = x;
		yData[validPoints] = y;
		validPoints++;
	}

	if (validPoints < 2 * fitData.minSegmentSize) {
		fitResult.status = QStringLiteral("Not enough valid data points in range");
		return false;
	}

	xData.resize(validPoints);
	yData.resize(validPoints);

	size_t* changepoints = new size_t[fitData.maxChangepoints];
	size_t numChangepoints = 0;

	if (fitData.changepointMethod == nsl_changepoint_method_binary_segmentation) {
		numChangepoints = nsl_changepoint_binary_segmentation(xData.data(),
															  yData.data(),
															  validPoints,
															  fitData.penalty,
															  fitData.minSegmentSize,
															  changepoints,
															  fitData.maxChangepoints);
	} else
		numChangepoints =
			nsl_changepoint_pelt(xData.data(), yData.data(), validPoints, fitData.penalty, fitData.minSegmentSize, changepoints, fitData.maxChangepoints);

	fitResult.numSegments = numChangepoints + 1;
	fitResult.changepoints.resize(numChangepoints);
	for (size_t i = 0; i < numChangepoints; ++i)
		fitResult.changepoints[i] = changepoints[i];

	fitResult.slopes.resize(fitResult.numSegments);
	fitResult.intercepts.resize(fitResult.numSegments);
	fitResult.rsquares.resize(fitResult.numSegments);

	size_t segStart = 0;
	double totalSSE = 0., totalSST = 0.;
	double meanY = 0.;
	for (size_t i = 0; i < validPoints; ++i)
		meanY += yData[i];
	meanY /= validPoints;

	for (size_t i = 0; i < validPoints; ++i)
		totalSST += (yData[i] - meanY) * (yData[i] - meanY);

	for (size_t seg = 0; seg < fitResult.numSegments; ++seg) {
		size_t segEnd = (seg < numChangepoints) ? changepoints[seg] : validPoints;

		QVector<double> segX = xData.mid(segStart, segEnd - segStart);
		QVector<double> segY = yData.mid(segStart, segEnd - segStart);

		double slope, intercept, rsq;
		fitSegment(segX, segY, slope, intercept, rsq);

		fitResult.slopes[seg] = slope;
		fitResult.intercepts[seg] = intercept;
		fitResult.rsquares[seg] = rsq;

		for (size_t i = segStart; i < segEnd; ++i) {
			double pred = slope * xData[i] + intercept;
			totalSSE += (yData[i] - pred) * (yData[i] - pred);
		}

		segStart = segEnd;
	}

	fitResult.sse = totalSSE;
	fitResult.overallRsquare = (totalSST > 0.) ? (1. - totalSSE / totalSST) : 0.;

	Range<double> evalRange = fitData.autoEvalRange ? xRange : fitData.evalRange;
	size_t evalPoints = fitData.evaluatedPoints;
	QVector<double>* xVector = new QVector<double>(evalPoints);
	QVector<double>* yVector = new QVector<double>(evalPoints);

	double step = (evalRange.end() - evalRange.start()) / (evalPoints - 1);
	for (size_t i = 0; i < evalPoints; ++i) {
		double x = evalRange.start() + i * step;
		(*xVector)[i] = x;

		size_t seg = 0;
		for (size_t j = 0; j < numChangepoints; ++j) {
			if (i < changepoints[j] * evalPoints / validPoints)
				break;
			seg++;
		}
		seg = qMin(seg, fitResult.numSegments - 1);

		(*yVector)[i] = fitResult.slopes[seg] * x + fitResult.intercepts[seg];
	}

	delete[] changepoints;

	xColumn->setValues(*xVector);
	yColumn->setValues(*yVector);

	delete xVector;
	delete yVector;

	fitResult.available = true;
	fitResult.valid = true;
	fitResult.status = QStringLiteral("OK");

	return true;
}

void XYPiecewiseLinearFitCurvePrivate::fitSegment(const QVector<double>& x, const QVector<double>& y, double& slope, double& intercept, double& rsquare) {
	size_t n = x.size();
	if (n < 2) {
		slope = 0.;
		intercept = (n == 1) ? y[0] : 0.;
		rsquare = 0.;
		return;
	}

	double sum_x = 0., sum_y = 0., sum_xx = 0., sum_xy = 0.;
	for (size_t i = 0; i < n; ++i) {
		sum_x += x[i];
		sum_y += y[i];
		sum_xx += x[i] * x[i];
		sum_xy += x[i] * y[i];
	}

	double mean_x = sum_x / n;
	double mean_y = sum_y / n;
	double var_x = sum_xx / n - mean_x * mean_x;

	if (std::fabs(var_x) < 1e-15) {
		slope = 0.;
		intercept = mean_y;
		rsquare = 0.;
		return;
	}

	slope = (sum_xy / n - mean_x * mean_y) / var_x;
	intercept = mean_y - slope * mean_x;

	double sse = 0., sst = 0.;
	for (size_t i = 0; i < n; ++i) {
		double pred = slope * x[i] + intercept;
		sse += (y[i] - pred) * (y[i] - pred);
		sst += (y[i] - mean_y) * (y[i] - mean_y);
	}

	rsquare = (sst > 0.) ? (1. - sse / sst) : 0.;
}
