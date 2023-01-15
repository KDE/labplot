/*
	File                 : XYCorrelationCurve.cpp
	Project              : LabPlot
	Description          : A xy-curve defined by a correlation
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2018 Stefan Gerlach <stefan.gerlach@uni.kn>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

/*!
  \class XYCorrelationCurve
  \brief A xy-curve defined by a correlation

  \ingroup worksheet
*/

#include "XYCorrelationCurve.h"
#include "XYCorrelationCurvePrivate.h"
#include "backend/core/column/Column.h"
#include "backend/lib/XmlStreamReader.h"
#include "backend/lib/commandtemplates.h"
#include "backend/lib/macros.h"

#include <KLocalizedString>
#include <QElapsedTimer>
#include <QIcon>
#include <QThreadPool>

extern "C" {
#include <gsl/gsl_math.h>
}

XYCorrelationCurve::XYCorrelationCurve(const QString& name)
	: XYAnalysisCurve(name, new XYCorrelationCurvePrivate(this), AspectType::XYCorrelationCurve) {
}

XYCorrelationCurve::XYCorrelationCurve(const QString& name, XYCorrelationCurvePrivate* dd)
	: XYAnalysisCurve(name, dd, AspectType::XYCorrelationCurve) {
}

// no need to delete the d-pointer here - it inherits from QGraphicsItem
// and is deleted during the cleanup in QGraphicsScene
XYCorrelationCurve::~XYCorrelationCurve() = default;

void XYCorrelationCurve::recalculate() {
	Q_D(XYCorrelationCurve);
	d->recalculate();
}

const XYAnalysisCurve::Result& XYCorrelationCurve::result() const {
	Q_D(const XYCorrelationCurve);
	return d->correlationResult;
}

/*!
	Returns an icon to be used in the project explorer.
*/
QIcon XYCorrelationCurve::icon() const {
	// 	return QIcon::fromTheme("labplot-xy-correlation-curve"); //not available yet
	return QIcon::fromTheme(QStringLiteral("labplot-xy-curve"));
}

//##############################################################################
//##########################  getter methods  ##################################
//##############################################################################
BASIC_SHARED_D_READER_IMPL(XYCorrelationCurve, XYCorrelationCurve::CorrelationData, correlationData, correlationData)

const XYCorrelationCurve::CorrelationResult& XYCorrelationCurve::correlationResult() const {
	Q_D(const XYCorrelationCurve);
	return d->correlationResult;
}

//##############################################################################
//#################  setter methods and undo commands ##########################
//##############################################################################
STD_SETTER_CMD_IMPL_F_S(XYCorrelationCurve, SetCorrelationData, XYCorrelationCurve::CorrelationData, correlationData, recalculate)
void XYCorrelationCurve::setCorrelationData(const XYCorrelationCurve::CorrelationData& correlationData) {
	Q_D(XYCorrelationCurve);
	exec(new XYCorrelationCurveSetCorrelationDataCmd(d, correlationData, ki18n("%1: set options and perform the correlation")));
}

//##############################################################################
//######################### Private implementation #############################
//##############################################################################
XYCorrelationCurvePrivate::XYCorrelationCurvePrivate(XYCorrelationCurve* owner)
	: XYAnalysisCurvePrivate(owner)
	, q(owner) {
}

// no need to delete xColumn and yColumn, they are deleted
// when the parent aspect is removed
XYCorrelationCurvePrivate::~XYCorrelationCurvePrivate() = default;

void XYCorrelationCurvePrivate::resetResults() {
	correlationResult = XYCorrelationCurve::CorrelationResult();
}

bool XYCorrelationCurvePrivate::recalculateSpecific(const AbstractColumn* tmpXDataColumn, const AbstractColumn* tmpYDataColumn) {
	DEBUG(Q_FUNC_INFO);
	QElapsedTimer timer;
	timer.start();

	// determine the data source columns
	const AbstractColumn* tmpY2DataColumn = nullptr;
	if (dataSourceType == XYAnalysisCurve::DataSourceType::Spreadsheet) {
		// spreadsheet columns as data source
		tmpY2DataColumn = y2DataColumn;
	} else {
		// curve columns as data source (autocorrelation)
		tmpY2DataColumn = dataSourceCurve->yColumn();
	}

	if (tmpY2DataColumn == nullptr) {
		return true;
	}

	// copy all valid data point for the correlation to temporary vectors
	QVector<double> xdataVector;
	QVector<double> ydataVector;
	QVector<double> y2dataVector;

	double xmin, xmax;
	if (tmpXDataColumn != nullptr && correlationData.autoRange) {
		xmin = tmpXDataColumn->minimum();
		xmax = tmpXDataColumn->maximum();
	} else {
		xmin = correlationData.xRange.first();
		xmax = correlationData.xRange.last();
	}

	// only copy those data where values are valid and in range
	if (tmpXDataColumn != nullptr) { // x-axis present (with possible range)
		for (int row = 0; row < tmpXDataColumn->rowCount(); ++row) {
			if (tmpXDataColumn->isValid(row) && !tmpXDataColumn->isMasked(row) && tmpYDataColumn->isValid(row) && !tmpYDataColumn->isMasked(row)) {
				if (tmpXDataColumn->valueAt(row) >= xmin && tmpXDataColumn->valueAt(row) <= xmax) {
					xdataVector.append(tmpXDataColumn->valueAt(row));
					ydataVector.append(tmpYDataColumn->valueAt(row));
				}
			}
		}
	} else { // no x-axis: take all valid values
		for (int row = 0; row < tmpYDataColumn->rowCount(); ++row)
			if (tmpYDataColumn->isValid(row) && !tmpYDataColumn->isMasked(row))
				ydataVector.append(tmpYDataColumn->valueAt(row));
	}

	if (tmpY2DataColumn != nullptr) {
		for (int row = 0; row < tmpY2DataColumn->rowCount(); ++row)
			if (tmpY2DataColumn->isValid(row) && !tmpY2DataColumn->isMasked(row))
				y2dataVector.append(tmpY2DataColumn->valueAt(row));
	}

	const size_t n = (size_t)ydataVector.size(); // number of points for signal
	const size_t m = (size_t)y2dataVector.size(); // number of points for response
	if (n < 1 || m < 1) {
		correlationResult.available = true;
		correlationResult.valid = false;
		correlationResult.status = i18n("Not enough data points available.");
		return true;
	}

	double* xdata = xdataVector.data();
	double* ydata = ydataVector.data();
	double* y2data = y2dataVector.data();

	// correlation settings
	const double samplingInterval = correlationData.samplingInterval;
	const nsl_corr_type_type type = correlationData.type;
	const nsl_corr_norm_type norm = correlationData.normalize;

	DEBUG("signal_1 n = " << n << ", signal_2 n = " << m);
	DEBUG("sampling interval = " << samplingInterval);
	DEBUG("type = " << nsl_corr_type_name[type]);
	DEBUG("norm = " << nsl_corr_norm_name[norm]);

	///////////////////////////////////////////////////////////
	size_t np = GSL_MAX(n, m);
	if (type == nsl_corr_type_linear)
		np = 2 * np - 1;

	double* out = (double*)malloc(np * sizeof(double));
	int status = nsl_corr_correlation(ydata, n, y2data, m, type, norm, out);

	xVector->resize((int)np);
	yVector->resize((int)np);
	// take given x-axis values or use index
	if (tmpXDataColumn != nullptr) {
		int size = GSL_MIN(xdataVector.size(), (int)np);
		memcpy(xVector->data(), xdata, size * sizeof(double));
		double sampleInterval = (xVector->data()[size - 1] - xVector->data()[0]) / (xdataVector.size() - 1);
		DEBUG("xdata size = " << xdataVector.size() << ", np = " << np << ", sample interval = " << sampleInterval);
		for (int i = size; i < (int)np; i++) // fill missing values
			xVector->data()[i] = xVector->data()[size - 1] + (i - size + 1) * sampleInterval;
	} else { // fill with index (starting with 0)
		if (type == nsl_corr_type_linear)
			for (size_t i = 0; i < np; i++)
				xVector->data()[i] = (int)(i - np / 2) * samplingInterval;
		else
			for (size_t i = 0; i < np; i++)
				xVector->data()[i] = (int)i * samplingInterval;
	}

	memcpy(yVector->data(), out, np * sizeof(double));
	free(out);
	///////////////////////////////////////////////////////////

	// write the result
	correlationResult.available = true;
	correlationResult.valid = (status == 0);
	correlationResult.status = QString::number(status);
	correlationResult.elapsedTime = timer.elapsed();

	return true;
}

//##############################################################################
//##################  Serialization/Deserialization  ###########################
//##############################################################################
//! Save as XML
void XYCorrelationCurve::save(QXmlStreamWriter* writer) const {
	Q_D(const XYCorrelationCurve);

	writer->writeStartElement(QStringLiteral("xyCorrelationCurve"));

	// write the base class
	XYAnalysisCurve::save(writer);

	// write xy-correlation-curve specific information
	//  correlation data
	writer->writeStartElement(QStringLiteral("correlationData"));
	writer->writeAttribute(QStringLiteral("samplingInterval"), QString::number(d->correlationData.samplingInterval));
	writer->writeAttribute(QStringLiteral("autoRange"), QString::number(d->correlationData.autoRange));
	writer->writeAttribute(QStringLiteral("xRangeMin"), QString::number(d->correlationData.xRange.first()));
	writer->writeAttribute(QStringLiteral("xRangeMax"), QString::number(d->correlationData.xRange.last()));
	writer->writeAttribute(QStringLiteral("type"), QString::number(d->correlationData.type));
	writer->writeAttribute(QStringLiteral("normalize"), QString::number(d->correlationData.normalize));
	writer->writeEndElement(); // correlationData

	// correlation results (generated columns)
	writer->writeStartElement(QStringLiteral("correlationResult"));
	writer->writeAttribute(QStringLiteral("available"), QString::number(d->correlationResult.available));
	writer->writeAttribute(QStringLiteral("valid"), QString::number(d->correlationResult.valid));
	writer->writeAttribute(QStringLiteral("status"), d->correlationResult.status);
	writer->writeAttribute(QStringLiteral("time"), QString::number(d->correlationResult.elapsedTime));

	// save calculated columns if available
	if (saveCalculations() && d->xColumn) {
		d->xColumn->save(writer);
		d->yColumn->save(writer);
	}
	writer->writeEndElement(); //"correlationResult"

	writer->writeEndElement(); //"xyCorrelationCurve"
}

//! Load from XML
bool XYCorrelationCurve::load(XmlStreamReader* reader, bool preview) {
	DEBUG("XYCorrelationCurve::load()");
	Q_D(XYCorrelationCurve);

	KLocalizedString attributeWarning = ki18n("Attribute '%1' missing or empty, default value is used");
	QXmlStreamAttributes attribs;
	QString str;

	while (!reader->atEnd()) {
		reader->readNext();
		if (reader->isEndElement() && reader->name() == QLatin1String("xyCorrelationCurve"))
			break;

		if (!reader->isStartElement())
			continue;

		if (reader->name() == QLatin1String("xyAnalysisCurve")) {
			if (!XYAnalysisCurve::load(reader, preview))
				return false;
		} else if (!preview && reader->name() == QLatin1String("correlationData")) {
			attribs = reader->attributes();
			READ_DOUBLE_VALUE("samplingInterval", correlationData.samplingInterval);
			READ_INT_VALUE("autoRange", correlationData.autoRange, bool);
			READ_DOUBLE_VALUE("xRangeMin", correlationData.xRange.first());
			READ_DOUBLE_VALUE("xRangeMax", correlationData.xRange.last());
			READ_INT_VALUE("type", correlationData.type, nsl_corr_type_type);
			READ_INT_VALUE("normalize", correlationData.normalize, nsl_corr_norm_type);
		} else if (!preview && reader->name() == QLatin1String("correlationResult")) {
			attribs = reader->attributes();
			READ_INT_VALUE("available", correlationResult.available, int);
			READ_INT_VALUE("valid", correlationResult.valid, int);
			READ_STRING_VALUE("status", correlationResult.status);
			READ_INT_VALUE("time", correlationResult.elapsedTime, int);
		} else if (!preview && reader->name() == QLatin1String("column")) {
			Column* column = new Column(QString(), AbstractColumn::ColumnMode::Double);
			if (!column->load(reader, preview)) {
				delete column;
				return false;
			}
			if (column->name() == QLatin1String("x"))
				d->xColumn = column;
			else if (column->name() == QLatin1String("y"))
				d->yColumn = column;
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

		recalcLogicalPoints();
	}

	return true;
}
