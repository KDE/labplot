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
#include "backend/worksheet/plots/cartesian/ReferenceLine.h"

#include <KLocalizedString>
#include <QElapsedTimer>
#include <QIcon>
#include <QThreadPool>

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

BASIC_D_READER_IMPL(XYPiecewiseLinearFitCurve, XYPiecewiseLinearFitCurve::FitData, fitData, fitData)

STD_SETTER_CMD_IMPL_F_S(XYPiecewiseLinearFitCurve, SetFitData, XYPiecewiseLinearFitCurve::FitData, fitData, recalculate)
void XYPiecewiseLinearFitCurve::setFitData(const XYPiecewiseLinearFitCurve::FitData& fitData) {
	Q_D(XYPiecewiseLinearFitCurve);
	exec(new XYPiecewiseLinearFitCurveSetFitDataCmd(d, fitData, ki18n("%1: set fit options and perform the fit")));
}

QVector<ReferenceLine*> XYPiecewiseLinearFitCurve::changepointLines() const {
	return children<ReferenceLine>(ChildIndexFlag::IncludeHidden);
}

// ##############################################################################
// ######################### Private implementation #############################
// ##############################################################################
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
			(*yVector)[outputIdx] = fitResult.slopes[seg] * x + fitResult.intercepts[seg];
			outputIdx++;
		}

		// Add NaN separator between segments (except after last segment)
		if (seg < fitResult.numSegments - 1) {
			(*xVector)[outputIdx] = std::nan("");
			(*yVector)[outputIdx] = std::nan("");
			outputIdx++;
		}
	}

	// Update changepoint reference lines before deleting changepoints array
	updateChangepointLines(xData, numChangepoints, changepoints);

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

void XYPiecewiseLinearFitCurvePrivate::updateChangepointLines(const QVector<double>& xData, size_t numChangepoints, const size_t* changepoints) {
	// Remove existing changepoint lines
	auto existingLines = q->changepointLines();
	for (auto* line : existingLines)
		q->removeChild(line);

	// Create new lines if enabled
	if (!fitData.changepointLinesEnabled || numChangepoints == 0)
		return;

	auto* plot = static_cast<CartesianPlot*>(q->parentAspect());
	if (!plot)
		return;

	for (size_t i = 0; i < numChangepoints; ++i) {
		double xPos = xData[changepoints[i]];
		auto* line = new ReferenceLine(plot, QStringLiteral("Changepoint %1").arg(i + 1), false);
		line->setHidden(true);
		line->setCoordinateSystemIndex(q->coordinateSystemIndex());
		line->setOrientation(ReferenceLine::Orientation::Vertical);
		line->setPositionLogical(QPointF(xPos, 0));
		q->addChildFast(line);
		line->retransform();
	}
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
	writer->writeAttribute(QStringLiteral("penalty"), QString::number(d->fitData.penalty));
	writer->writeAttribute(QStringLiteral("minSegmentSize"), QString::number(d->fitData.minSegmentSize));
	writer->writeAttribute(QStringLiteral("maxChangepoints"), QString::number(d->fitData.maxChangepoints));
	writer->writeAttribute(QStringLiteral("changepointLinesEnabled"), QString::number(d->fitData.changepointLinesEnabled));
	writer->writeEndElement();

	writer->writeStartElement(QStringLiteral("fitResult"));
	writer->writeAttribute(QStringLiteral("available"), QString::number(d->fitResult.available));
	writer->writeAttribute(QStringLiteral("valid"), QString::number(d->fitResult.valid));
	writer->writeAttribute(QStringLiteral("status"), d->fitResult.status);
	writer->writeAttribute(QStringLiteral("numSegments"), QString::number(d->fitResult.numSegments));
	writer->writeAttribute(QStringLiteral("overallRsquare"), QString::number(d->fitResult.overallRsquare));
	writer->writeAttribute(QStringLiteral("sse"), QString::number(d->fitResult.sse));
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
			d->fitData.penalty = attribs.value(QStringLiteral("penalty")).toDouble();
			d->fitData.minSegmentSize = attribs.value(QStringLiteral("minSegmentSize")).toULongLong();
			d->fitData.maxChangepoints = attribs.value(QStringLiteral("maxChangepoints")).toULongLong();
			d->fitData.changepointLinesEnabled = attribs.value(QStringLiteral("changepointLinesEnabled")).toInt();
		} else if (reader->name() == QLatin1String("fitResult")) {
			attribs = reader->attributes();
			d->fitResult.available = attribs.value(QStringLiteral("available")).toInt();
			d->fitResult.valid = attribs.value(QStringLiteral("valid")).toInt();
			d->fitResult.status = attribs.value(QStringLiteral("status")).toString();
			d->fitResult.numSegments = attribs.value(QStringLiteral("numSegments")).toULongLong();
			d->fitResult.overallRsquare = attribs.value(QStringLiteral("overallRsquare")).toDouble();
			d->fitResult.sse = attribs.value(QStringLiteral("sse")).toDouble();
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
