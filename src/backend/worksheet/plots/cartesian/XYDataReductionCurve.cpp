/*
	File                 : XYDataReductionCurve.cpp
	Project              : LabPlot
	Description          : A xy-curve defined by a data reduction
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2016 Stefan Gerlach <stefan.gerlach@uni.kn>
	SPDX-FileCopyrightText: 2017 Alexander Semke <alexander.semke@web.de>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

/*!
  \class XYDataReductionCurve
  \brief A xy-curve defined by a data reduction

  \ingroup worksheet
*/

#include "XYDataReductionCurve.h"
#include "CartesianCoordinateSystem.h"
#include "XYDataReductionCurvePrivate.h"
#include "backend/core/column/Column.h"
#include "backend/lib/XmlStreamReader.h"
#include "backend/lib/commandtemplates.h"
#include "backend/lib/macros.h"

#include <KLocalizedString>
#include <QElapsedTimer>
#include <QIcon>
#include <QThreadPool>

XYDataReductionCurve::XYDataReductionCurve(const QString& name)
	: XYAnalysisCurve(name, new XYDataReductionCurvePrivate(this), AspectType::XYDataReductionCurve) {
}

XYDataReductionCurve::XYDataReductionCurve(const QString& name, XYDataReductionCurvePrivate* dd)
	: XYAnalysisCurve(name, dd, AspectType::XYDataReductionCurve) {
}

// no need to delete the d-pointer here - it inherits from QGraphicsItem
// and is deleted during the cleanup in QGraphicsScene
XYDataReductionCurve::~XYDataReductionCurve() = default;

void XYDataReductionCurve::recalculate() {
	Q_D(XYDataReductionCurve);
	d->recalculate();
}

const XYAnalysisCurve::Result& XYDataReductionCurve::result() const {
	Q_D(const XYDataReductionCurve);
	return d->dataReductionResult;
}
/*!
	Returns an icon to be used in the project explorer.
*/
QIcon XYDataReductionCurve::icon() const {
	return QIcon::fromTheme(QStringLiteral("labplot-xy-data-reduction-curve"));
}

// ##############################################################################
// ##########################  getter methods  ##################################
// ##############################################################################
BASIC_SHARED_D_READER_IMPL(XYDataReductionCurve, XYDataReductionCurve::DataReductionData, dataReductionData, dataReductionData)

const XYDataReductionCurve::DataReductionResult& XYDataReductionCurve::dataReductionResult() const {
	Q_D(const XYDataReductionCurve);
	return d->dataReductionResult;
}

// ##############################################################################
// #################  setter methods and undo commands ##########################
// ##############################################################################
STD_SETTER_CMD_IMPL_F_S(XYDataReductionCurve, SetDataReductionData, XYDataReductionCurve::DataReductionData, dataReductionData, recalculate)
void XYDataReductionCurve::setDataReductionData(const XYDataReductionCurve::DataReductionData& reductionData) {
	Q_D(XYDataReductionCurve);
	exec(new XYDataReductionCurveSetDataReductionDataCmd(d, reductionData, ki18n("%1: set options and perform the data reduction")));
}

// ##############################################################################
// ######################### Private implementation #############################
// ##############################################################################
XYDataReductionCurvePrivate::XYDataReductionCurvePrivate(XYDataReductionCurve* owner)
	: XYAnalysisCurvePrivate(owner)
	, q(owner) {
}

// no need to delete xColumn and yColumn, they are deleted
// when the parent aspect is removed
XYDataReductionCurvePrivate::~XYDataReductionCurvePrivate() = default;

void XYDataReductionCurvePrivate::resetResults() {
	dataReductionResult = XYDataReductionCurve::DataReductionResult();
}

const XYAnalysisCurve::Result& XYDataReductionCurvePrivate::result() const {
	return dataReductionResult;
}

bool XYDataReductionCurvePrivate::recalculateSpecific(const AbstractColumn* tmpXDataColumn, const AbstractColumn* tmpYDataColumn) {
	QElapsedTimer timer;
	timer.start();

	// copy all valid data point for the data reduction to temporary vectors
	QVector<double> xdataVector;
	QVector<double> ydataVector;

	double xmin;
	double xmax;
	if (dataReductionData.autoRange) {
		xmin = tmpXDataColumn->minimum();
		xmax = tmpXDataColumn->maximum();
	} else {
		xmin = dataReductionData.xRange.first();
		xmax = dataReductionData.xRange.last();
	}

	XYAnalysisCurve::copyData(xdataVector, ydataVector, tmpXDataColumn, tmpYDataColumn, xmin, xmax);

	// number of data points to use
	const size_t n = (size_t)xdataVector.size();
	if (n < 2) {
		dataReductionResult.available = true;
		dataReductionResult.valid = false;
		dataReductionResult.status = i18n("Not enough data points available.");
		return true;
	}

	double* xdata = xdataVector.data();
	double* ydata = ydataVector.data();

	// dataReduction settings
	const nsl_geom_linesim_type type = dataReductionData.type;
	const double tol = dataReductionData.tolerance;
	const double tol2 = dataReductionData.tolerance2;

	DEBUG("n =" << n);
	DEBUG("type:" << nsl_geom_linesim_type_name[type]);
	DEBUG("tolerance/step:" << tol);
	DEBUG("tolerance2/repeat/maxtol/region:" << tol2);

	///////////////////////////////////////////////////////////
	Q_EMIT q->completed(10);

	size_t npoints = 0;
	double calcTolerance = 0; // calculated tolerance from Douglas-Peucker variant
	size_t* index = (size_t*)malloc(n * sizeof(size_t));
	switch (type) {
	case nsl_geom_linesim_type_douglas_peucker_variant: // tol used as number of points
		npoints = tol;
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
		npoints = nsl_geom_linesim_opheim(xdata, ydata, n, tol, tol2, index);
		break;
	}

	DEBUG("npoints =" << npoints);
	if (type == nsl_geom_linesim_type_douglas_peucker_variant) {
		DEBUG("calculated tolerance =" << calcTolerance)
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
	dataReductionResult.available = true;
	dataReductionResult.valid = npoints > 0;
	if (npoints > 0)
		dataReductionResult.status = QStringLiteral("OK");
	else
		dataReductionResult.status = QStringLiteral("FAILURE");
	dataReductionResult.elapsedTime = timer.elapsed();
	dataReductionResult.npoints = npoints;
	dataReductionResult.posError = posError;
	dataReductionResult.areaError = areaError;

	Q_EMIT q->completed(100);
	return true;
}

// ##############################################################################
// ##################  Serialization/Deserialization  ###########################
// ##############################################################################
//! Save as XML
void XYDataReductionCurve::save(QXmlStreamWriter* writer) const {
	Q_D(const XYDataReductionCurve);

	writer->writeStartElement(QStringLiteral("xyDataReductionCurve"));

	// write the base class
	XYAnalysisCurve::save(writer);

	// write xy-dataReduction-curve specific information
	//  dataReduction data
	writer->writeStartElement(QStringLiteral("dataReductionData"));
	writer->writeAttribute(QStringLiteral("autoRange"), QString::number(d->dataReductionData.autoRange));
	writer->writeAttribute(QStringLiteral("xRangeMin"), QString::number(d->dataReductionData.xRange.first()));
	writer->writeAttribute(QStringLiteral("xRangeMax"), QString::number(d->dataReductionData.xRange.last()));
	writer->writeAttribute(QStringLiteral("type"), QString::number(d->dataReductionData.type));
	writer->writeAttribute(QStringLiteral("autoTolerance"), QString::number(d->dataReductionData.autoTolerance));
	writer->writeAttribute(QStringLiteral("tolerance"), QString::number(d->dataReductionData.tolerance));
	writer->writeAttribute(QStringLiteral("autoTolerance2"), QString::number(d->dataReductionData.autoTolerance2));
	writer->writeAttribute(QStringLiteral("tolerance2"), QString::number(d->dataReductionData.tolerance2));
	writer->writeEndElement(); // dataReductionData

	// dataReduction results (generated columns)
	writer->writeStartElement(QStringLiteral("dataReductionResult"));
	writer->writeAttribute(QStringLiteral("available"), QString::number(d->dataReductionResult.available));
	writer->writeAttribute(QStringLiteral("valid"), QString::number(d->dataReductionResult.valid));
	writer->writeAttribute(QStringLiteral("status"), d->dataReductionResult.status);
	writer->writeAttribute(QStringLiteral("time"), QString::number(d->dataReductionResult.elapsedTime));
	writer->writeAttribute(QStringLiteral("npoints"), QString::number(d->dataReductionResult.npoints));
	writer->writeAttribute(QStringLiteral("posError"), QString::number(d->dataReductionResult.posError));
	writer->writeAttribute(QStringLiteral("areaError"), QString::number(d->dataReductionResult.areaError));

	// save calculated columns if available
	if (saveCalculations() && d->xColumn) {
		d->xColumn->save(writer);
		d->yColumn->save(writer);
	}
	writer->writeEndElement(); //"dataReductionResult"

	writer->writeEndElement(); //"xyDataReductionCurve"
}

//! Load from XML
bool XYDataReductionCurve::load(XmlStreamReader* reader, bool preview) {
	Q_D(XYDataReductionCurve);

	QXmlStreamAttributes attribs;
	QString str;

	while (!reader->atEnd()) {
		reader->readNext();
		if (reader->isEndElement() && reader->name() == QLatin1String("xyDataReductionCurve"))
			break;

		if (!reader->isStartElement())
			continue;

		if (reader->name() == QLatin1String("xyAnalysisCurve")) {
			if (!XYAnalysisCurve::load(reader, preview))
				return false;
		} else if (!preview && reader->name() == QLatin1String("dataReductionData")) {
			attribs = reader->attributes();
			READ_INT_VALUE("autoRange", dataReductionData.autoRange, bool);
			READ_DOUBLE_VALUE("xRangeMin", dataReductionData.xRange.first());
			READ_DOUBLE_VALUE("xRangeMax", dataReductionData.xRange.last());
			READ_INT_VALUE("type", dataReductionData.type, nsl_geom_linesim_type);
			READ_INT_VALUE("autoTolerance", dataReductionData.autoTolerance, int);
			READ_DOUBLE_VALUE("tolerance", dataReductionData.tolerance);
			READ_INT_VALUE("autoTolerance2", dataReductionData.autoTolerance2, int);
			READ_DOUBLE_VALUE("tolerance2", dataReductionData.tolerance2);
		} else if (!preview && reader->name() == QLatin1String("dataReductionResult")) {
			attribs = reader->attributes();
			READ_INT_VALUE("available", dataReductionResult.available, int);
			READ_INT_VALUE("valid", dataReductionResult.valid, int);
			READ_STRING_VALUE("status", dataReductionResult.status);
			READ_INT_VALUE("time", dataReductionResult.elapsedTime, int);
			READ_INT_VALUE("npoints", dataReductionResult.npoints, size_t);
			READ_DOUBLE_VALUE("posError", dataReductionResult.posError);
			READ_DOUBLE_VALUE("areaError", dataReductionResult.areaError);
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
