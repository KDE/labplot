/*
	File                 : XYLineSimplificationCurve.cpp
	Project              : LabPlot
	Description          : A xy-curve defined by a line simplification
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2016 Stefan Gerlach <stefan.gerlach@uni.kn>
	SPDX-FileCopyrightText: 2017 Alexander Semke <alexander.semke@web.de>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "XYLineSimplificationCurve.h"
#include "XYLineSimplificationCurvePrivate.h"
#include "backend/core/column/Column.h"
#include "backend/lib/XmlStreamReader.h"
#include "backend/lib/commandtemplates.h"

#include <KLocalizedString>
#include <QElapsedTimer>
#include <QIcon>
#include <QThreadPool>

/*!
 * \class XYLineSimplificationCurve
 * \brief A xy-curve defined by a line simplification.
 * \ingroup CartesianAnalysisPlots
 */
XYLineSimplificationCurve::XYLineSimplificationCurve(const QString& name)
	: XYAnalysisCurve(name, new XYLineSimplificationCurvePrivate(this), AspectType::XYLineSimplificationCurve) {
}

XYLineSimplificationCurve::XYLineSimplificationCurve(const QString& name, XYLineSimplificationCurvePrivate* dd)
	: XYAnalysisCurve(name, dd, AspectType::XYLineSimplificationCurve) {
}

// no need to delete the d-pointer here - it inherits from QGraphicsItem
// and is deleted during the cleanup in QGraphicsScene
XYLineSimplificationCurve::~XYLineSimplificationCurve() = default;

const XYAnalysisCurve::Result& XYLineSimplificationCurve::result() const {
	Q_D(const XYLineSimplificationCurve);
	return d->lineSimplificationResult;
}
/*!
	Returns an icon to be used in the project explorer.
*/
QIcon XYLineSimplificationCurve::icon() const {
	return QIcon::fromTheme(QStringLiteral("labplot-xy-curve"));
}

// ##############################################################################
// ##########################  getter methods  ##################################
// ##############################################################################
BASIC_SHARED_D_READER_IMPL(XYLineSimplificationCurve, XYLineSimplificationCurve::LineSimplificationData, lineSimplificationData, lineSimplificationData)

const XYLineSimplificationCurve::LineSimplificationResult& XYLineSimplificationCurve::lineSimplificationResult() const {
	Q_D(const XYLineSimplificationCurve);
	return d->lineSimplificationResult;
}

// ##############################################################################
// #################  setter methods and undo commands ##########################
// ##############################################################################
STD_SETTER_CMD_IMPL_F_S(XYLineSimplificationCurve,
						SetLineSimplificationData,
						XYLineSimplificationCurve::LineSimplificationData,
						lineSimplificationData,
						recalculate)
void XYLineSimplificationCurve::setLineSimplificationData(const XYLineSimplificationCurve::LineSimplificationData& data) {
	Q_D(XYLineSimplificationCurve);
	exec(new XYLineSimplificationCurveSetLineSimplificationDataCmd(d, data, ki18n("%1: set options and perform the line simplification")));
}

// ##############################################################################
// ######################### Private implementation #############################
// ##############################################################################
XYLineSimplificationCurvePrivate::XYLineSimplificationCurvePrivate(XYLineSimplificationCurve* owner)
	: XYAnalysisCurvePrivate(owner)
	, q(owner) {
}

// no need to delete xColumn and yColumn, they are deleted
// when the parent aspect is removed
XYLineSimplificationCurvePrivate::~XYLineSimplificationCurvePrivate() = default;

void XYLineSimplificationCurvePrivate::resetResults() {
	lineSimplificationResult = XYLineSimplificationCurve::LineSimplificationResult();
}

const XYAnalysisCurve::Result& XYLineSimplificationCurvePrivate::result() const {
	return lineSimplificationResult;
}

bool XYLineSimplificationCurvePrivate::recalculateSpecific(const AbstractColumn* tmpXDataColumn, const AbstractColumn* tmpYDataColumn) {
	QElapsedTimer timer;
	timer.start();

	// copy all valid data point for the line simplification to temporary vectors
	QVector<double> xdataVector;
	QVector<double> ydataVector;

	double xmin;
	double xmax;
	if (lineSimplificationData.autoRange) {
		xmin = tmpXDataColumn->minimum();
		xmax = tmpXDataColumn->maximum();
	} else {
		xmin = lineSimplificationData.xRange.first();
		xmax = lineSimplificationData.xRange.last();
	}

	XYAnalysisCurve::copyData(xdataVector, ydataVector, tmpXDataColumn, tmpYDataColumn, xmin, xmax);

	// number of data points to use
	const size_t n = (size_t)xdataVector.size();
	if (n < 2) {
		lineSimplificationResult.available = true;
		lineSimplificationResult.valid = false;
		lineSimplificationResult.status = i18n("Not enough data points available.");
		return true;
	}

	double* xdata = xdataVector.data();
	double* ydata = ydataVector.data();

	// lineSimplification settings
	const nsl_geom_linesim_type type = lineSimplificationData.type;
	const double tol = lineSimplificationData.tolerance;
	const double tol2 = lineSimplificationData.tolerance2;

	DEBUG(Q_FUNC_INFO << ", n = " << n);
	DEBUG(Q_FUNC_INFO << ", type: " << nsl_geom_linesim_type_name[type]);
	DEBUG(Q_FUNC_INFO << ", tolerance/step: " << tol);
	DEBUG(Q_FUNC_INFO << ", tolerance2/repeat/maxtol/region: " << tol2);

	///////////////////////////////////////////////////////////
	Q_EMIT q->completed(10);

	size_t npoints = 0;
	double calcTolerance = 0; // calculated tolerance from Douglas-Peucker variant
	size_t* index = (size_t*)malloc(n * sizeof(size_t));
	switch (type) {
	case nsl_geom_linesim_type_douglas_peucker_variant: // tol used as number of points
		npoints = size_t(tol);
		calcTolerance = nsl_geom_linesim_douglas_peucker_variant(xdata, ydata, n, npoints, index);
		break;
	case nsl_geom_linesim_type_douglas_peucker:
		npoints = nsl_geom_linesim_douglas_peucker(xdata, ydata, n, tol, index);
		break;
	case nsl_geom_linesim_type_nthpoint: // tol used as step
		npoints = nsl_geom_linesim_nthpoint(n, (int)tol, index);
		break;
	case nsl_geom_linesim_type_raddist:
		npoints = nsl_geom_linesim_raddist(xdata, ydata, n, tol, index);
		break;
	case nsl_geom_linesim_type_perpdist: // tol2 used as repeat
		npoints = nsl_geom_linesim_perpdist_repeat(xdata, ydata, n, tol, tol2, index);
		break;
	case nsl_geom_linesim_type_interp:
		npoints = nsl_geom_linesim_interp(xdata, ydata, n, tol, index);
		break;
	case nsl_geom_linesim_type_visvalingam_whyatt:
		npoints = nsl_geom_linesim_visvalingam_whyatt(xdata, ydata, n, tol, index);
		break;
	case nsl_geom_linesim_type_reumann_witkam:
		npoints = nsl_geom_linesim_reumann_witkam(xdata, ydata, n, tol, index);
		break;
	case nsl_geom_linesim_type_opheim:
		npoints = nsl_geom_linesim_opheim(xdata, ydata, n, tol, tol2, index);
		break;
	case nsl_geom_linesim_type_lang: // tol2 used as region
		npoints = nsl_geom_linesim_lang(xdata, ydata, n, tol, tol2, index);
		break;
	}

	DEBUG(Q_FUNC_INFO << ", npoints = " << npoints);
	if (type == nsl_geom_linesim_type_douglas_peucker_variant) {
		DEBUG(Q_FUNC_INFO << ", calculated tolerance = " << calcTolerance)
	} else
		Q_UNUSED(calcTolerance);

	Q_EMIT q->completed(80);

	xVector->resize((int)npoints);
	yVector->resize((int)npoints);
	for (int i = 0; i < (int)npoints; i++) {
		(*xVector)[i] = xdata[index[i]];
		(*yVector)[i] = ydata[index[i]];
	}

	Q_EMIT q->completed(90);

	const double posError = nsl_geom_linesim_positional_squared_error(xdata, ydata, n, index);
	const double areaError = nsl_geom_linesim_area_error(xdata, ydata, n, index);

	free(index);

	///////////////////////////////////////////////////////////

	// write the result
	lineSimplificationResult.available = true;
	lineSimplificationResult.valid = npoints > 0;
	if (npoints > 0)
		lineSimplificationResult.status = QStringLiteral("OK");
	else
		lineSimplificationResult.status = QStringLiteral("FAILURE");
	lineSimplificationResult.elapsedTime = timer.elapsed();
	lineSimplificationResult.npoints = npoints;
	lineSimplificationResult.posError = posError;
	lineSimplificationResult.areaError = areaError;

	Q_EMIT q->completed(100);
	return true;
}

// ##############################################################################
// ##################  Serialization/Deserialization  ###########################
// ##############################################################################
//! Save as XML
void XYLineSimplificationCurve::save(QXmlStreamWriter* writer) const {
	Q_D(const XYLineSimplificationCurve);

	writer->writeStartElement(QStringLiteral("xyLineSimplificationCurve"));

	// write the base class
	XYAnalysisCurve::save(writer);

	// write xy-lineSimplification-curve specific information
	//  lineSimplification data
	writer->writeStartElement(QStringLiteral("lineSimplificationData"));
	writer->writeAttribute(QStringLiteral("autoRange"), QString::number(d->lineSimplificationData.autoRange));
	writer->writeAttribute(QStringLiteral("xRangeMin"), QString::number(d->lineSimplificationData.xRange.first()));
	writer->writeAttribute(QStringLiteral("xRangeMax"), QString::number(d->lineSimplificationData.xRange.last()));
	writer->writeAttribute(QStringLiteral("type"), QString::number(d->lineSimplificationData.type));
	writer->writeAttribute(QStringLiteral("autoTolerance"), QString::number(d->lineSimplificationData.autoTolerance));
	writer->writeAttribute(QStringLiteral("tolerance"), QString::number(d->lineSimplificationData.tolerance));
	writer->writeAttribute(QStringLiteral("autoTolerance2"), QString::number(d->lineSimplificationData.autoTolerance2));
	writer->writeAttribute(QStringLiteral("tolerance2"), QString::number(d->lineSimplificationData.tolerance2));
	writer->writeEndElement(); // lineSimplificationData

	// lineSimplification results (generated columns)
	writer->writeStartElement(QStringLiteral("lineSimplificationResult"));
	writer->writeAttribute(QStringLiteral("available"), QString::number(d->lineSimplificationResult.available));
	writer->writeAttribute(QStringLiteral("valid"), QString::number(d->lineSimplificationResult.valid));
	writer->writeAttribute(QStringLiteral("status"), d->lineSimplificationResult.status);
	writer->writeAttribute(QStringLiteral("time"), QString::number(d->lineSimplificationResult.elapsedTime));
	writer->writeAttribute(QStringLiteral("npoints"), QString::number(d->lineSimplificationResult.npoints));
	writer->writeAttribute(QStringLiteral("posError"), QString::number(d->lineSimplificationResult.posError));
	writer->writeAttribute(QStringLiteral("areaError"), QString::number(d->lineSimplificationResult.areaError));

	// save calculated columns if available
	if (saveCalculations() && d->xColumn) {
		d->xColumn->save(writer);
		d->yColumn->save(writer);
	}
	writer->writeEndElement(); //"lineSimplificationResult"

	writer->writeEndElement(); //"xyLineSimplificationCurve"
}

//! Load from XML
bool XYLineSimplificationCurve::load(XmlStreamReader* reader, bool preview) {
	Q_D(XYLineSimplificationCurve);

	QXmlStreamAttributes attribs;
	QString str;

	while (!reader->atEnd()) {
		reader->readNext();
		if (reader->isEndElement() && (reader->name() == QLatin1String("xyLineSimplificationCurve") || reader->name() == QLatin1String("xyDataReductionCurve")))
			break;

		if (!reader->isStartElement())
			continue;

		if (reader->name() == QLatin1String("xyAnalysisCurve")) {
			if (!XYAnalysisCurve::load(reader, preview))
				return false;
		} else if (!preview && (reader->name() == QLatin1String("lineSimplificationData") || reader->name() == QLatin1String("dataReductionData"))) {
			attribs = reader->attributes();
			READ_INT_VALUE("autoRange", lineSimplificationData.autoRange, bool);
			READ_DOUBLE_VALUE("xRangeMin", lineSimplificationData.xRange.first());
			READ_DOUBLE_VALUE("xRangeMax", lineSimplificationData.xRange.last());
			READ_INT_VALUE("type", lineSimplificationData.type, nsl_geom_linesim_type);
			READ_INT_VALUE("autoTolerance", lineSimplificationData.autoTolerance, int);
			READ_DOUBLE_VALUE("tolerance", lineSimplificationData.tolerance);
			READ_INT_VALUE("autoTolerance2", lineSimplificationData.autoTolerance2, int);
			READ_DOUBLE_VALUE("tolerance2", lineSimplificationData.tolerance2);
		} else if (!preview && (reader->name() == QLatin1String("lineSimplificationResult") || reader->name() == QLatin1String("dataReductionResult"))) {
			attribs = reader->attributes();
			READ_INT_VALUE("available", lineSimplificationResult.available, int);
			READ_INT_VALUE("valid", lineSimplificationResult.valid, int);
			READ_STRING_VALUE("status", lineSimplificationResult.status);
			READ_INT_VALUE("time", lineSimplificationResult.elapsedTime, int);
			READ_INT_VALUE("npoints", lineSimplificationResult.npoints, size_t);
			READ_DOUBLE_VALUE("posError", lineSimplificationResult.posError);
			READ_DOUBLE_VALUE("areaError", lineSimplificationResult.areaError);
		} else if (reader->name() == QLatin1String("column")) {
			Column* column = new Column(QString(), AbstractColumn::ColumnMode::Double);
			if (!column->load(reader, preview)) {
				delete column;
				return false;
			}
			if (column->name() == QLatin1String("x"))
				d->xColumn = column;
			else if (column->name() == QLatin1String("y"))
				d->yColumn = column;
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
