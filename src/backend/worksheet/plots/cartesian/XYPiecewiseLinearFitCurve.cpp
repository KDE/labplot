/*
	File                 : XYPiecewiseLinearFitCurve.cpp
	Project              : LabPlot
	Description          : A xy-curve defined by piecewise linear regression
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2026 Alexander Semke <alexander.semke@web.de>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "XYPiecewiseLinearFitCurve.h"
#include "XYPiecewiseLinearFitCurvePrivate.h"
#include "backend/core/column/Column.h"
#include "backend/note/Note.h"
#include "backend/worksheet/plots/cartesian/ReferenceLine.h"
#include "backend/nsl/nsl_stats.h"

#include <KLocalizedString>
#include <QElapsedTimer>
#include <QFontDatabase>
#include <QIcon>
#include <QThreadPool>

#include <gsl/gsl_cdf.h>

namespace {
void calculateFitMetrics(XYFitCurve::FitResult& result, size_t n, unsigned int np) {
	if (n == 0)
		return;

	result.mse = result.sse / n;
	result.rmse = std::sqrt(result.mse);
	result.rsquare = (result.sst > 0.) ? 1. - result.sse / result.sst : 0.;

	if (result.dof > 0) {
		result.rms = result.sse / result.dof;
		result.rsd = std::sqrt(result.rms);
		result.chisq_p = nsl_stats_chisq_p(result.sse, result.dof);
	} else {
		result.rms = result.rsd = 0.;
		result.chisq_p = 1.;
	}

	if (result.dof > 1 && result.sst > 0.)
		result.rsquareAdj = 1. - (1. - result.rsquare) * (n - 1.) / (result.dof - 1.);
	else
		result.rsquareAdj = 0.;

	if (result.dof > 0 && result.sst > 0.) {
		if (result.sse > 0.) {
			result.fdist_F = (result.sst - result.sse) / (result.sse / result.dof);
			result.fdist_p = gsl_cdf_fdist_Q(result.fdist_F, np - 1, result.dof);
		} else {
			result.fdist_F = std::numeric_limits<double>::infinity();
			result.fdist_p = 0.;
		}
	} else {
		result.fdist_F = 0.;
		result.fdist_p = 1.;
	}

	result.logLik = nsl_stats_logLik(result.sse, n);
	if (result.sse > 0.) {
		result.aic = nsl_stats_aic(result.sse, n, np, 1);
		result.bic = nsl_stats_bic(result.sse, n, np, 1);
	} else {
		result.aic = result.bic = -std::numeric_limits<double>::infinity();
	}
}
}

XYPiecewiseLinearFitCurve::XYPiecewiseLinearFitCurve(const QString& name)
	: XYAnalysisCurve(name, new XYPiecewiseLinearFitCurvePrivate(this), AspectType::XYPiecewiseLinearFitCurve) {
}

XYPiecewiseLinearFitCurve::XYPiecewiseLinearFitCurve(const QString& name, XYPiecewiseLinearFitCurvePrivate* dd)
	: XYAnalysisCurve(name, dd, AspectType::XYPiecewiseLinearFitCurve) {
}

XYPiecewiseLinearFitCurve::~XYPiecewiseLinearFitCurve() = default;

const XYAnalysisCurve::Result& XYPiecewiseLinearFitCurve::result() const {
	Q_D(const XYPiecewiseLinearFitCurve);
	return d->fitResult;
}

const XYPiecewiseLinearFitCurve::FitResult& XYPiecewiseLinearFitCurve::fitResult() const {
	Q_D(const XYPiecewiseLinearFitCurve);
	return d->fitResult;
}

QIcon XYPiecewiseLinearFitCurve::icon() const {
	return QIcon::fromTheme(QStringLiteral("labplot-xy-fit-curve"));
}

// ##############################################################################
// ##########################  getter methods  ##################################
// ##############################################################################
BASIC_D_READER_IMPL(XYPiecewiseLinearFitCurve, XYPiecewiseLinearFitCurve::FitData, fitData, fitData)
BASIC_D_READER_IMPL(XYPiecewiseLinearFitCurve, bool, changepointLinesEnabled, changepointLinesEnabled)

QVector<ReferenceLine*> XYPiecewiseLinearFitCurve::changepointLines() const {
	return children<ReferenceLine>(ChildIndexFlag::IncludeHidden);
}

// ##############################################################################
// #################  setter methods and undo commands ##########################
// ##############################################################################
STD_SETTER_CMD_IMPL_F_S(XYPiecewiseLinearFitCurve, SetFitData, XYPiecewiseLinearFitCurve::FitData, fitData, recalculate)
void XYPiecewiseLinearFitCurve::setFitData(const XYPiecewiseLinearFitCurve::FitData& fitData) {
	Q_D(XYPiecewiseLinearFitCurve);
	exec(new XYPiecewiseLinearFitCurveSetFitDataCmd(d, fitData, ki18n("%1: set fit options and perform the fit")));
}

STD_SETTER_CMD_IMPL_F_S(XYPiecewiseLinearFitCurve, SetChangepointLinesEnabled, bool, changepointLinesEnabled, updateChangepointLines)
void XYPiecewiseLinearFitCurve::setChangepointLinesEnabled(bool enabled) {
	Q_D(XYPiecewiseLinearFitCurve);
	if (d->changepointLinesEnabled != enabled)
		exec(new XYPiecewiseLinearFitCurveSetChangepointLinesEnabledCmd(d, enabled, ki18n("%1: toggle changepoint lines")));
}

// ##############################################################################
// ######################### Private implementation #############################
// ##############################################################################
XYPiecewiseLinearFitCurvePrivate::XYPiecewiseLinearFitCurvePrivate(XYPiecewiseLinearFitCurve* owner)
	: XYAnalysisCurvePrivate(owner)
	, q(owner) {
}

XYPiecewiseLinearFitCurvePrivate::~XYPiecewiseLinearFitCurvePrivate() = default;

void XYPiecewiseLinearFitCurvePrivate::retransform() {
	XYAnalysisCurvePrivate::retransform();

	const auto& lines = q->changepointLines();
	for (auto* line : lines) {
		line->retransform();
	}
}

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

	Range<double> xRange = Range<double>{tmpXDataColumn->minimum(), tmpXDataColumn->maximum()};

	size_t validPoints = 0;
	for (int i = 0; i < tmpXDataColumn->rowCount(); ++i) {
		double x = tmpXDataColumn->valueAt(i);
		double y = tmpYDataColumn->valueAt(i);

		if (std::isnan(x) || std::isnan(y))
			continue;
		if (x < xRange.start() || x > xRange.end())
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
	for (size_t i = 0; i < numChangepoints; ++i) {
		qDebug() << "Changepoint" << i << ":" << changepoints[i] << "x =" << xData[changepoints[i]];
		fitResult.changepoints[i] = xData[changepoints[i]]; // Store X-value, not index
	}

	fitResult.segmentResults.resize(fitResult.numSegments);

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

		if (fitData.connectionType == XYPiecewiseLinearFitCurve::ConnectionType::Continuous && seg > 0) {
			// Continuous: constrain fit so segment starts at previous endpoint
			double xChangepoint = xData[segStart];
			double yPrev = fitResult.segmentResults[seg - 1].paramValues.value(1) * xChangepoint + fitResult.segmentResults[seg - 1].paramValues.value(0);
			fitSegmentConstrained(segX, segY, xChangepoint, yPrev, seg);
		} else {
			// Discontinuous: independent fit
			fitSegment(segX, segY, seg);
		}

		totalSSE += fitResult.segmentResults[seg].sse;

		segStart = segEnd;
	}

	fitResult.sse = totalSSE;
	fitResult.rsquare = (totalSST > 0.) ? (1. - totalSSE / totalSST) : 0.;

	const int pointsPerSegment = 2; // for lines 2 points are sufficient

	// Total points = pointsPerSegment * numSegments + (numSegments - 1) NaN separators
	size_t totalPoints = pointsPerSegment * fitResult.numSegments + (fitResult.numSegments - 1);
	QVector<double>* xVector = new QVector<double>(totalPoints);
	QVector<double>* yVector = new QVector<double>(totalPoints);

	size_t outputIdx = 0;
	for (size_t seg = 0; seg < fitResult.numSegments; ++seg) {
		// Determine X range for this segment
		double segXMin, segXMax;
		if (seg == 0) {
			segXMin = xRange.start();
			segXMax = (numChangepoints > 0) ? xData[changepoints[0]] : xRange.end();
		} else if (seg == fitResult.numSegments - 1) {
			segXMin = xData[changepoints[seg - 1]];
			segXMax = xRange.end();
		} else {
			segXMin = xData[changepoints[seg - 1]];
			segXMax = xData[changepoints[seg]];
		}

		double step = (segXMax - segXMin) / (pointsPerSegment - 1);
		for (size_t i = 0; i < pointsPerSegment; ++i) {
			double x = segXMin + i * step;
			(*xVector)[outputIdx] = x;
			(*yVector)[outputIdx] = fitResult.segmentResults[seg].paramValues.value(1) * x + fitResult.segmentResults[seg].paramValues.value(0);
			outputIdx++;
		}

		// Add NaN separator between segments (except after last segment)
		if (seg < fitResult.numSegments - 1) {
			(*xVector)[outputIdx] = std::nan("");
			(*yVector)[outputIdx] = std::nan("");
			outputIdx++;
		}
	}

	// Update changepoint reference lines
	updateChangepointLines();

	// Create and update results note
	if (!resultsNote) {
		resultsNote = new Note(i18n("Fit Results"));
		resultsNote->setFixed(true);
		resultsNote->setBackgroundColor(QColor(Qt::white));
		resultsNote->setTextFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
		q->addChildFast(resultsNote);
	}

	delete[] changepoints;

	xColumn->setValues(*xVector);
	yColumn->setValues(*yVector);

	delete xVector;
	delete yVector;

	fitResult.available = true;
	fitResult.valid = true;
	fitResult.status = QStringLiteral("OK");

	updateResultsNote();

	return true;
}

void XYPiecewiseLinearFitCurvePrivate::fitSegment(const QVector<double>& x, const QVector<double>& y, size_t seg) {
	auto& result = fitResult.segmentResults[seg];
	result = XYFitCurve::FitResult();
	result.paramValues.resize(2);
	result.errorValues.resize(2);
	result.tdist_tValues.resize(2);
	result.tdist_pValues.resize(2);
	result.tdist_pValues.fill(1.);

	size_t n = x.size();
	if (n < 2) {
		// Reset to defaults
		result.paramValues[0] = (n == 1) ? y[0] : 0.; // intercept
		result.paramValues[1] = 0.; // slope
		calculateFitMetrics(result, n, 1);
		return;
	}

	// Compute slope and intercept using centered sums for numerical stability.
	double sum_x = 0., sum_y = 0.;
	for (size_t i = 0; i < n; ++i) {
		sum_x += x[i];
		sum_y += y[i];
	}

	double mean_x = sum_x / n;
	double mean_y = sum_y / n;
	double Sxx = 0., Sxy = 0.;
	for (size_t i = 0; i < n; ++i) {
		const double dx = x[i] - mean_x;
		const double dy = y[i] - mean_y;
		Sxx += dx * dx;
		Sxy += dx * dy;
	}

	if (Sxx > 1e-15) {
		result.paramValues[1] = Sxy / Sxx; // slope
		result.paramValues[0] = mean_y - result.paramValues[1] * mean_x; // intercept
	} else {
		result.paramValues[1] = 0.; // slope
		result.paramValues[0] = mean_y; // intercept
	}

	// Compute residuals and goodness metrics
	double sse = 0., sst = 0., mae = 0.;
	for (size_t i = 0; i < n; ++i) {
		double pred = result.paramValues[1] * x[i] + result.paramValues[0];
		double residual = y[i] - pred;
		sse += residual * residual;
		sst += (y[i] - mean_y) * (y[i] - mean_y);
		mae += std::abs(residual);
	}

	result.sse = sse;
	result.sst = sst;
	result.dof = (Sxx > 1e-15) ? n - 2 : n - 1;

	if (result.dof > 0) {
		double mse_val = sse / result.dof;
		if (Sxx > 1e-15) {
			result.errorValues[1] = std::sqrt(mse_val / Sxx); // slopeError
			result.errorValues[0] = std::sqrt(mse_val * (1.0 / n + mean_x * mean_x / Sxx)); // interceptError
			if (result.errorValues[1] > 0.) {
				result.tdist_tValues[1] = result.paramValues[1] / result.errorValues[1];
				result.tdist_pValues[1] = nsl_stats_tdist_p(result.tdist_tValues[1], result.dof);
			}
			if (result.errorValues[0] > 0.) {
				result.tdist_tValues[0] = result.paramValues[0] / result.errorValues[0];
				result.tdist_pValues[0] = nsl_stats_tdist_p(result.tdist_tValues[0], result.dof);
			}
		}
	} else {
		result.tdist_pValues[0] = result.tdist_pValues[1] = 1.;
	}

	result.mae = mae / n;
	calculateFitMetrics(result, n, Sxx > 1e-15 ? 2 : 1);
}

// Constrained linear regression: fit line through (x0, y0) with minimum squared error
void XYPiecewiseLinearFitCurvePrivate::fitSegmentConstrained(const QVector<double>& x, const QVector<double>& y, double x0, double y0, size_t seg) {
	auto& result = fitResult.segmentResults[seg];
	result = XYFitCurve::FitResult();
	result.paramValues.resize(2);
	result.errorValues.resize(2);
	result.tdist_tValues.resize(2);
	result.tdist_pValues.resize(2);
	result.tdist_pValues.fill(1.);

	size_t n = x.size();
	if (n == 0) {
		// Reset to defaults
		result.paramValues[0] = y0; // intercept
		result.paramValues[1] = 0.; // slope
		return;
	}

	// m = sum((x[i] - x0) * (y[i] - y0)) / sum((x[i] - x0)^2)
	double num = 0., denom = 0.;
	for (size_t i = 0; i < n; ++i) {
		double dx = x[i] - x0;
		double dy = y[i] - y0;
		num += dx * dy;
		denom += dx * dx;
	}

	if (denom > 1e-15) {
		result.paramValues[1] = num / denom; // slope
		result.paramValues[0] = y0 - result.paramValues[1] * x0; // intercept
	} else {
		result.paramValues[1] = 0.; // slope
		result.paramValues[0] = y0; // intercept
	}

	// Compute residuals
	double mean_y = 0.;
	for (size_t i = 0; i < n; ++i)
		mean_y += y[i];
	mean_y /= n;

	double sse = 0., sst = 0., mae = 0.;
	for (size_t i = 0; i < n; ++i) {
		double pred = result.paramValues[1] * x[i] + result.paramValues[0];
		double residual = y[i] - pred;
		sse += residual * residual;
		sst += (y[i] - mean_y) * (y[i] - mean_y);
		mae += std::abs(residual);
	}

	result.sse = sse;
	result.sst = sst;
	result.dof = n - 1; // Constrained fit: only 1 free parameter

	if (result.dof > 0) {
		double mse_val = sse / result.dof;
		if (denom > 1e-15)
			result.errorValues[1] = std::sqrt(mse_val / denom); // slopeError
		result.errorValues[0] = 0.; // interceptError - constrained

		if (result.errorValues[1] > 0.) {
			result.tdist_tValues[1] = result.paramValues[1] / result.errorValues[1];
			result.tdist_pValues[1] = nsl_stats_tdist_p(result.tdist_tValues[1], result.dof);
		}
		result.tdist_tValues[0] = 0.; // intercept_t - constrained
		result.tdist_pValues[0] = 1.; // intercept_p - constrained
	} else {
		result.errorValues[0] = result.errorValues[1] = 0.;
		result.tdist_tValues[0] = result.tdist_tValues[1] = 0.;
		result.tdist_pValues[0] = result.tdist_pValues[1] = 1.;
	}

	result.mae = mae / n;
	calculateFitMetrics(result, n, 1);
}

void XYPiecewiseLinearFitCurvePrivate::updateChangepointLines() {
	// Remove existing changepoint lines
	auto existingLines = q->changepointLines();
	for (auto* line : existingLines)
		q->removeChild(line);

	// Create new lines if enabled
	if (!changepointLinesEnabled || fitResult.changepoints.isEmpty())
		return;

	auto* plot = static_cast<CartesianPlot*>(q->parentAspect());
	if (!plot)
		return;

	for (int i = 0; i < fitResult.changepoints.size(); ++i) {
		double xPos = fitResult.changepoints[i]; // Already X-value, not index
		auto* line = new ReferenceLine(plot, QStringLiteral("Changepoint %1").arg(i + 1), false);
		line->setHidden(true);
		line->setCoordinateSystemIndex(q->coordinateSystemIndex());
		line->setOrientation(ReferenceLine::Orientation::Vertical);
		line->setPositionLogical(QPointF(xPos, 0));
		line->setParentGraphicsItem(q->graphicsItem());
		q->addChildFast(line);
		line->retransform();
	}
}

void XYPiecewiseLinearFitCurvePrivate::updateResultsNote() {
	if (!resultsNote)
		return;

	QString text;
	text += i18n("Piecewise Linear Fit Results") + QStringLiteral("\n");
	text += QStringLiteral("========================================\n\n");

	if (!fitResult.available) {
		text += i18n("No results available yet");
		resultsNote->setText(text);
		return;
	}

	if (!fitResult.valid) {
		text += i18n("Status: %1", fitResult.status);
		resultsNote->setText(text);
		return;
	}

	// Overall summary
	text += i18n("Status: %1", fitResult.status) + QStringLiteral("\n");
	text += i18n("Number of Segments: %1", fitResult.numSegments) + QStringLiteral("\n");
	text += i18n("Number of Changepoints: %1", fitResult.changepoints.size()) + QStringLiteral("\n");
	text += i18n("Overall R²: %1", QString::number(fitResult.rsquare, 'f', 6)) + QStringLiteral("\n");
	text += i18n("Overall SSE: %1", QString::number(fitResult.sse, 'g', 6)) + QStringLiteral("\n\n");

	// Changepoints
	if (!fitResult.changepoints.isEmpty()) {
		text += i18n("Changepoint Positions (X-values)") + QStringLiteral(":\n");
		for (int i = 0; i < fitResult.changepoints.size(); ++i)
			text += QStringLiteral("  %1: %2\n").arg(i + 1).arg(fitResult.changepoints[i], 0, 'g', 6);
		text += QStringLiteral("\n");
	}

	// Per-segment details
	for (size_t seg = 0; seg < fitResult.numSegments; ++seg) {
		const auto& segResult = fitResult.segmentResults[seg];
		text += QStringLiteral("----------------------------------------\n");
		text += i18n("Segment %1", seg + 1) + QStringLiteral("\n");
		text += QStringLiteral("----------------------------------------\n");

		// Parameters
		text += i18n("Parameters:") + QStringLiteral("\n");
		text += QStringLiteral("  Slope     = %1 ± %2\n").arg(segResult.paramValues.value(1), 0, 'g', 6).arg(segResult.errorValues.value(1), 0, 'g', 3);
		text += QStringLiteral("  Intercept = %1 ± %2\n").arg(segResult.paramValues.value(0), 0, 'g', 6).arg(segResult.errorValues.value(0), 0, 'g', 3);

		// Parameter statistics
		text += QStringLiteral("\n");
		text += i18n("Parameter Statistics (Slope):") + QStringLiteral("\n");
		text += QStringLiteral("  t-value = %1\n").arg(segResult.tdist_tValues.value(1), 0, 'g', 4);
		text += QStringLiteral("  p-value = %1\n").arg(segResult.tdist_pValues.value(1), 0, 'g', 4);

		// Goodness of fit
		text += QStringLiteral("\n");
		text += i18n("Goodness of Fit:") + QStringLiteral("\n");
		text += QStringLiteral("  R²           = %1\n").arg(segResult.rsquare, 0, 'f', 6);
		text += QStringLiteral("  Adjusted R²  = %1\n").arg(segResult.rsquareAdj, 0, 'f', 6);
		text += QStringLiteral("  SSE          = %1\n").arg(segResult.sse, 0, 'g', 6);
		text += QStringLiteral("  RMS          = %1\n").arg(segResult.rms, 0, 'g', 6);
		text += QStringLiteral("  MAE          = %1\n").arg(segResult.mae, 0, 'g', 6);
		text += QStringLiteral("  DoF          = %1\n").arg(segResult.dof);

		// Statistical tests
		text += QStringLiteral("\n");
		text += i18n("Statistical Tests:") + QStringLiteral("\n");
		text += QStringLiteral("  χ² p-value   = %1\n").arg(segResult.chisq_p, 0, 'g', 4);
		text += QStringLiteral("  F-statistic  = %1\n").arg(segResult.fdist_F, 0, 'g', 4);
		text += QStringLiteral("  F p-value    = %1\n").arg(segResult.fdist_p, 0, 'g', 4);

		// Information criteria
		text += QStringLiteral("\n");
		text += i18n("Information Criteria:") + QStringLiteral("\n");
		text += QStringLiteral("  AIC          = %1\n").arg(segResult.aic, 0, 'g', 6);
		text += QStringLiteral("  BIC          = %1\n").arg(segResult.bic, 0, 'g', 6);

		text += QStringLiteral("\n");
	}

	resultsNote->setUndoAware(false);
	resultsNote->setText(text);
	resultsNote->setUndoAware(true);
}

// ##############################################################################
// ##################  Serialization/Deserialization  ###########################
// ##############################################################################
//! Save as XML
void XYPiecewiseLinearFitCurve::save(QXmlStreamWriter* writer) const {
	Q_D(const XYPiecewiseLinearFitCurve);

	writer->writeStartElement(QStringLiteral("xyPiecewiseLinearFitCurve"));

	// write the base class
	XYAnalysisCurve::save(writer);

	writer->writeStartElement(QStringLiteral("fitData"));
	writer->writeAttribute(QStringLiteral("changepointMethod"), QString::number(d->fitData.changepointMethod));
	writer->writeAttribute(QStringLiteral("connectionType"), QString::number(static_cast<int>(d->fitData.connectionType)));
	writer->writeAttribute(QStringLiteral("penalty"), QString::number(d->fitData.penalty));
	writer->writeAttribute(QStringLiteral("minSegmentSize"), QString::number(d->fitData.minSegmentSize));
	writer->writeAttribute(QStringLiteral("maxChangepoints"), QString::number(d->fitData.maxChangepoints));
	writer->writeAttribute(QStringLiteral("changepointLinesEnabled"), QString::number(d->changepointLinesEnabled));
	writer->writeEndElement();

	writer->writeStartElement(QStringLiteral("fitResult"));
	writer->writeAttribute(QStringLiteral("available"), QString::number(d->fitResult.available));
	writer->writeAttribute(QStringLiteral("valid"), QString::number(d->fitResult.valid));
	writer->writeAttribute(QStringLiteral("status"), d->fitResult.status);
	writer->writeAttribute(QStringLiteral("numSegments"), QString::number(d->fitResult.numSegments));
	writer->writeAttribute(QStringLiteral("rsquare"), QString::number(d->fitResult.rsquare));
	writer->writeAttribute(QStringLiteral("sse"), QString::number(d->fitResult.sse));

	// one <segment> element per fit segment, holding all per-segment values as attributes;
	// changepoints has one entry less than the number of segments
	for (size_t i = 0; i < d->fitResult.numSegments; ++i) {
		const auto& segResult = d->fitResult.segmentResults[i];
		writer->writeStartElement(QStringLiteral("segment"));
		writer->writeAttribute(QStringLiteral("slope"), QString::number(segResult.paramValues.value(1), 'g', 15));
		writer->writeAttribute(QStringLiteral("intercept"), QString::number(segResult.paramValues.value(0), 'g', 15));
		writer->writeAttribute(QStringLiteral("slopeError"), QString::number(segResult.errorValues.value(1), 'g', 15));
		writer->writeAttribute(QStringLiteral("interceptError"), QString::number(segResult.errorValues.value(0), 'g', 15));
		writer->writeAttribute(QStringLiteral("rsquare"), QString::number(segResult.rsquare, 'g', 15));
		writer->writeAttribute(QStringLiteral("rsquareAdj"), QString::number(segResult.rsquareAdj, 'g', 15));
		writer->writeAttribute(QStringLiteral("sseValue"), QString::number(segResult.sse, 'g', 15));
		writer->writeAttribute(QStringLiteral("sstValue"), QString::number(segResult.sst, 'g', 15));
		writer->writeAttribute(QStringLiteral("rmsValue"), QString::number(segResult.rms, 'g', 15));
		writer->writeAttribute(QStringLiteral("rsdValue"), QString::number(segResult.rsd, 'g', 15));
		writer->writeAttribute(QStringLiteral("mseValue"), QString::number(segResult.mse, 'g', 15));
		writer->writeAttribute(QStringLiteral("rmseValue"), QString::number(segResult.rmse, 'g', 15));
		writer->writeAttribute(QStringLiteral("maeValue"), QString::number(segResult.mae, 'g', 15));
		writer->writeAttribute(QStringLiteral("dofValue"), QString::number(segResult.dof));
		writer->writeAttribute(QStringLiteral("chisqP"), QString::number(segResult.chisq_p, 'g', 15));
		writer->writeAttribute(QStringLiteral("fdistF"), QString::number(segResult.fdist_F, 'g', 15));
		writer->writeAttribute(QStringLiteral("fdistP"), QString::number(segResult.fdist_p, 'g', 15));
		writer->writeAttribute(QStringLiteral("aicValue"), QString::number(segResult.aic, 'g', 15));
		writer->writeAttribute(QStringLiteral("bicValue"), QString::number(segResult.bic, 'g', 15));
		writer->writeAttribute(QStringLiteral("slopeT"), QString::number(segResult.tdist_tValues.value(1), 'g', 15));
		writer->writeAttribute(QStringLiteral("slopeP"), QString::number(segResult.tdist_pValues.value(1), 'g', 15));
		if (i < static_cast<size_t>(d->fitResult.changepoints.size()))
			writer->writeAttribute(QStringLiteral("changepoint"), QString::number(d->fitResult.changepoints.at(i), 'g', 15));
		writer->writeEndElement(); // segment
	}
	writer->writeEndElement();

	// save calculated columns if available
	if (saveCalculations() && d->xColumn) {
		d->xColumn->save(writer);
		d->yColumn->save(writer);
	}

	writer->writeEndElement();
}

//! Load from XML
bool XYPiecewiseLinearFitCurve::load(XmlStreamReader* reader, bool preview) {
	setIsLoading(true);
	Q_D(XYPiecewiseLinearFitCurve);

	if (!readBasicAttributes(reader))
		return false;

	QXmlStreamAttributes attribs;
	while (!reader->atEnd()) {
		reader->readNext();
		if (reader->isEndElement() && reader->name() == QLatin1String("xyPiecewiseLinearFitCurve"))
			break;

		if (!reader->isStartElement())
			continue;

		if (reader->name() == QLatin1String("xyAnalysisCurve")) {
			if (!XYAnalysisCurve::load(reader, preview))
				return false;
		} else if (reader->name() == QLatin1String("fitData")) {
			attribs = reader->attributes();
			d->fitData.changepointMethod = static_cast<nsl_changepoint_method>(attribs.value(QStringLiteral("changepointMethod")).toInt());
			d->fitData.connectionType = static_cast<ConnectionType>(attribs.value(QStringLiteral("connectionType")).toInt());
			d->fitData.penalty = attribs.value(QStringLiteral("penalty")).toDouble();
			d->fitData.minSegmentSize = attribs.value(QStringLiteral("minSegmentSize")).toULongLong();
			d->fitData.maxChangepoints = attribs.value(QStringLiteral("maxChangepoints")).toULongLong();
			d->changepointLinesEnabled = attribs.value(QStringLiteral("changepointLinesEnabled")).toInt();
		} else if (reader->name() == QLatin1String("fitResult")) {
			attribs = reader->attributes();
			d->fitResult.available = attribs.value(QStringLiteral("available")).toInt();
			d->fitResult.valid = attribs.value(QStringLiteral("valid")).toInt();
			d->fitResult.status = attribs.value(QStringLiteral("status")).toString();
			d->fitResult.numSegments = attribs.value(QStringLiteral("numSegments")).toULongLong();
			d->fitResult.rsquare = attribs.value(QStringLiteral("rsquare")).toDouble();
			d->fitResult.sse = attribs.value(QStringLiteral("sse")).toDouble();
		} else if (reader->name() == QLatin1String("segment")) {
			attribs = reader->attributes();
			XYFitCurve::FitResult segResult;
			segResult.paramValues.resize(2);
			segResult.errorValues.resize(2);
			segResult.tdist_tValues.resize(2);
			segResult.tdist_pValues.resize(2);

			segResult.paramValues[1] = attribs.value(QStringLiteral("slope")).toDouble(); // slope
			segResult.paramValues[0] = attribs.value(QStringLiteral("intercept")).toDouble(); // intercept
			segResult.errorValues[1] = attribs.value(QStringLiteral("slopeError")).toDouble();
			segResult.errorValues[0] = attribs.value(QStringLiteral("interceptError")).toDouble();
			segResult.rsquare = attribs.value(QStringLiteral("rsquare")).toDouble();
			segResult.rsquareAdj = attribs.value(QStringLiteral("rsquareAdj")).toDouble();
			segResult.sse = attribs.value(QStringLiteral("sseValue")).toDouble();
			segResult.sst = attribs.value(QStringLiteral("sstValue")).toDouble();
			segResult.rms = attribs.value(QStringLiteral("rmsValue")).toDouble();
			segResult.rsd = attribs.value(QStringLiteral("rsdValue")).toDouble();
			segResult.mse = attribs.value(QStringLiteral("mseValue")).toDouble();
			segResult.rmse = attribs.value(QStringLiteral("rmseValue")).toDouble();
			segResult.mae = attribs.value(QStringLiteral("maeValue")).toDouble();
			segResult.dof = attribs.value(QStringLiteral("dofValue")).toInt();
			segResult.chisq_p = attribs.value(QStringLiteral("chisqP")).toDouble();
			segResult.fdist_F = attribs.value(QStringLiteral("fdistF")).toDouble();
			segResult.fdist_p = attribs.value(QStringLiteral("fdistP")).toDouble();
			segResult.aic = attribs.value(QStringLiteral("aicValue")).toDouble();
			segResult.bic = attribs.value(QStringLiteral("bicValue")).toDouble();
			segResult.tdist_tValues[1] = attribs.value(QStringLiteral("slopeT")).toDouble();
			segResult.tdist_pValues[1] = attribs.value(QStringLiteral("slopeP")).toDouble();
			d->fitResult.segmentResults << segResult;
			if (attribs.hasAttribute(QStringLiteral("changepoint")))
				d->fitResult.changepoints << attribs.value(QStringLiteral("changepoint")).toDouble();
		} else if (reader->name() == QLatin1String("column")) {
			auto* column = new Column(QString(), AbstractColumn::ColumnMode::Double);
			if (!column->load(reader, preview)) {
				delete column;
				return false;
			}
			if (column->name() == QLatin1String("x"))
				d->xColumn = column;
			else if (column->name() == QLatin1String("y"))
				d->yColumn = column;
			else
				delete column;
		} else { // unknown element
			reader->raiseUnknownElementWarning();
			if (!reader->skipToEndElement())
				return false;
		}
	}

	if (preview)
		return true;

	// wait for data to be read before using the pointers
	QThreadPool::globalInstance()->waitForDone();

	// Add result note (not saved in projects, recreated on load)
	d->resultsNote = new Note(i18n("Fit Results"));
	d->resultsNote->setFixed(true);
	d->resultsNote->setBackgroundColor(QColor(Qt::white));
	d->resultsNote->setTextFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
	addChildFast(d->resultsNote);

	if (d->xColumn && d->yColumn) {
		d->xColumn->setHidden(true);
		addChild(d->xColumn);

		d->yColumn->setHidden(true);
		addChild(d->yColumn);

		d->xVector = static_cast<QVector<double>*>(d->xColumn->data());
		d->yVector = static_cast<QVector<double>*>(d->yColumn->data());

		static_cast<XYCurvePrivate*>(d_ptr)->xColumn = d->xColumn;
		static_cast<XYCurvePrivate*>(d_ptr)->yColumn = d->yColumn;

		recalc();
	}

	return true;
}
