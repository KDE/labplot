/*
	File                 : XYFourierTransformCurve.cpp
	Project              : LabPlot
	Description          : A xy-curve defined by a Fourier transform
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2016 Stefan Gerlach <stefan.gerlach@uni.kn>
	SPDX-FileCopyrightText: 2017 Alexander Semke <alexander.semke@web.de>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "XYFourierTransformCurve.h"
#include "XYFourierTransformCurvePrivate.h"
#include "backend/core/AbstractColumn.h"
#include "backend/core/column/Column.h"
#include "backend/gsl/errors.h"
#include "backend/lib/XmlStreamReader.h"
#include "backend/lib/commandtemplates.h"
#include "backend/lib/macros.h"

extern "C" {
#include "backend/nsl/nsl_sf_poly.h"
}

#include <KLocalizedString>
#include <QDebug> // qWarning()
#include <QElapsedTimer>
#include <QIcon>
#include <QThreadPool>

/*!
 * \class XYFourierTransformCurve
 * \brief A xy-curve defined by a Fourier transform.
 * \ingroup CartesianAnalysisPlots
 */
XYFourierTransformCurve::XYFourierTransformCurve(const QString& name)
	: XYAnalysisCurve(name, new XYFourierTransformCurvePrivate(this), AspectType::XYFourierTransformCurve) {
}

XYFourierTransformCurve::XYFourierTransformCurve(const QString& name, XYFourierTransformCurvePrivate* dd)
	: XYAnalysisCurve(name, dd, AspectType::XYFourierTransformCurve) {
}

// no need to delete the d-pointer here - it inherits from QGraphicsItem
// and is deleted during the cleanup in QGraphicsScene
XYFourierTransformCurve::~XYFourierTransformCurve() = default;

void XYFourierTransformCurve::recalculate() {
	Q_D(XYFourierTransformCurve);
	d->recalculate();
}

/*!
	Returns an icon to be used in the project explorer.
*/
QIcon XYFourierTransformCurve::icon() const {
	return QIcon::fromTheme(QStringLiteral("labplot-xy-fourier-transform-curve"));
}

// ##############################################################################
// ##########################  getter methods  ##################################
// ##############################################################################
BASIC_SHARED_D_READER_IMPL(XYFourierTransformCurve, XYFourierTransformCurve::TransformData, transformData, transformData)

const XYAnalysisCurve::Result& XYFourierTransformCurve::result() const {
	Q_D(const XYFourierTransformCurve);
	return d->transformResult;
}

// ##############################################################################
// #################  setter methods and undo commands ##########################
// ##############################################################################
STD_SETTER_CMD_IMPL_F_S(XYFourierTransformCurve, SetTransformData, XYFourierTransformCurve::TransformData, transformData, recalculate)
void XYFourierTransformCurve::setTransformData(const XYFourierTransformCurve::TransformData& transformData) {
	Q_D(XYFourierTransformCurve);
	exec(new XYFourierTransformCurveSetTransformDataCmd(d, transformData, ki18n("%1: set transform options and perform the Fourier transform")));
}

// ##############################################################################
// ######################### Private implementation #############################
// ##############################################################################
XYFourierTransformCurvePrivate::XYFourierTransformCurvePrivate(XYFourierTransformCurve* owner)
	: XYAnalysisCurvePrivate(owner)
	, q(owner) {
}

// no need to delete xColumn and yColumn, they are deleted
// when the parent aspect is removed
XYFourierTransformCurvePrivate::~XYFourierTransformCurvePrivate() = default;

void XYFourierTransformCurvePrivate::resetResults() {
	transformResult = XYFourierTransformCurve::TransformResult();
}

bool XYFourierTransformCurvePrivate::recalculateSpecific(const AbstractColumn* tmpXDataColumn, const AbstractColumn* tmpYDataColumn) {
	QElapsedTimer timer;
	timer.start();

	// copy all valid data point for the transform to temporary vectors
	QVector<double> xdataVector;
	QVector<double> ydataVector;
	double xmin = transformData.xRange.first();
	double xmax = transformData.xRange.last();
	if (transformData.autoRange) {
		xmin = q->xDataColumn()->minimum();
		xmax = q->xDataColumn()->maximum();
	}

	int rowCount = std::min(tmpXDataColumn->rowCount(), tmpYDataColumn->rowCount());
	for (int row = 0; row < rowCount; ++row) {
		// only copy those data where _all_ values (for x and y, if given) are valid
		if (std::isnan(tmpXDataColumn->valueAt(row)) || std::isnan(tmpYDataColumn->valueAt(row)) || tmpXDataColumn->isMasked(row)
			|| tmpYDataColumn->isMasked(row))
			continue;

		// only when inside given range
		if (tmpXDataColumn->valueAt(row) >= xmin && tmpXDataColumn->valueAt(row) <= xmax) {
			xdataVector.append(tmpXDataColumn->valueAt(row));
			ydataVector.append(tmpYDataColumn->valueAt(row));
		}
	}

	// number of data points to transform
	auto n = (unsigned int)ydataVector.size();
	if (n == 0) {
		transformResult.available = true;
		transformResult.valid = false;
		transformResult.status = i18n("No data points available.");
		return true;
	}

	double* xdata = xdataVector.data();
	double* ydata = ydataVector.data();

	// transform settings
	const nsl_sf_window_type windowType = transformData.windowType;
	const nsl_dft_result_type type = transformData.type;
	const bool twoSided = transformData.twoSided;
	const bool shifted = transformData.shifted;
	const nsl_dft_xscale xScale = transformData.xScale;

	DEBUG("n =" << n);
	DEBUG("window type:" << nsl_sf_window_type_name[windowType]);
	DEBUG("type:" << nsl_dft_result_type_name[type]);
	DEBUG("scale:" << nsl_dft_xscale_name[xScale]);
	DEBUG("two sided:" << twoSided);
	DEBUG("shifted:" << shifted);
#ifndef NDEBUG
	QDebug out = qDebug();
	for (unsigned int i = 0; i < n; i++)
		out << ydata[i];
#endif

	///////////////////////////////////////////////////////////
	// transform with window
	gsl_set_error_handler_off();
	int status = nsl_dft_transform_window(ydata, 1, n, twoSided, type, windowType);

	unsigned int N = n;
	if (twoSided == false)
		N = n / 2;

	switch (xScale) {
	case nsl_dft_xscale_frequency:
		for (unsigned int i = 0; i < N; i++) {
			if (i >= n / 2 && shifted)
				xdata[i] = (n - 1) / (xmax - xmin) * (i / (double)n - 1.);
			else
				xdata[i] = (n - 1) * i / (xmax - xmin) / n;
		}
		break;
	case nsl_dft_xscale_index:
		for (unsigned int i = 0; i < N; i++) {
			if (i >= n / 2 && shifted)
				xdata[i] = (int)i - (int)N;
			else
				xdata[i] = i;
		}
		break;
	case nsl_dft_xscale_period: {
		double f0 = (n - 1) / (xmax - xmin) / n;
		for (unsigned int i = 0; i < N; i++) {
			double f = (n - 1) * i / (xmax - xmin) / n;
			xdata[i] = 1 / (f + f0);
		}
		break;
	}
	}
#ifndef NDEBUG
	out = qDebug();
	for (unsigned int i = 0; i < N; i++)
		out << ydata[i] << '(' << xdata[i] << ')';
#endif

	xVector->resize((int)N);
	yVector->resize((int)N);
	if (shifted) {
		memcpy(xVector->data(), &xdata[n / 2], n / 2 * sizeof(double));
		memcpy(&xVector->data()[n / 2], xdata, n / 2 * sizeof(double));
		memcpy(yVector->data(), &ydata[n / 2], n / 2 * sizeof(double));
		memcpy(&yVector->data()[n / 2], ydata, n / 2 * sizeof(double));
	} else {
		memcpy(xVector->data(), xdata, N * sizeof(double));
		memcpy(yVector->data(), ydata, N * sizeof(double));
	}
	///////////////////////////////////////////////////////////

	// write the result
	transformResult.available = true;
	transformResult.valid = (status == GSL_SUCCESS);
	transformResult.status = gslErrorToString(status);
	transformResult.elapsedTime = timer.elapsed();

	return true;
}

// ##############################################################################
// ##################  Serialization/Deserialization  ###########################
// ##############################################################################
//! Save as XML
void XYFourierTransformCurve::save(QXmlStreamWriter* writer) const {
	Q_D(const XYFourierTransformCurve);

	writer->writeStartElement(QStringLiteral("xyFourierTransformCurve"));

	// write the base class
	XYAnalysisCurve::save(writer);

	// write xy-fourier_transform-curve specific information
	// transform data
	writer->writeStartElement(QStringLiteral("transformData"));
	writer->writeAttribute(QStringLiteral("autoRange"), QString::number(d->transformData.autoRange));
	writer->writeAttribute(QStringLiteral("xRangeMin"), QString::number(d->transformData.xRange.first()));
	writer->writeAttribute(QStringLiteral("xRangeMax"), QString::number(d->transformData.xRange.last()));
	writer->writeAttribute(QStringLiteral("type"), QString::number(d->transformData.type));
	writer->writeAttribute(QStringLiteral("twoSided"), QString::number(d->transformData.twoSided));
	writer->writeAttribute(QStringLiteral("shifted"), QString::number(d->transformData.shifted));
	writer->writeAttribute(QStringLiteral("xScale"), QString::number(d->transformData.xScale));
	writer->writeAttribute(QStringLiteral("windowType"), QString::number(d->transformData.windowType));
	writer->writeEndElement(); // transformData

	// transform results (generated columns)
	writer->writeStartElement(QStringLiteral("transformResult"));
	writer->writeAttribute(QStringLiteral("available"), QString::number(d->transformResult.available));
	writer->writeAttribute(QStringLiteral("valid"), QString::number(d->transformResult.valid));
	writer->writeAttribute(QStringLiteral("status"), d->transformResult.status);
	writer->writeAttribute(QStringLiteral("time"), QString::number(d->transformResult.elapsedTime));

	// save calculated columns if available
	if (saveCalculations() && d->xColumn && d->yColumn) {
		d->xColumn->save(writer);
		d->yColumn->save(writer);
	}
	writer->writeEndElement(); //"transformResult"
	writer->writeEndElement(); //"xyFourierTransformCurve"
}

//! Load from XML
bool XYFourierTransformCurve::load(XmlStreamReader* reader, bool preview) {
	Q_D(XYFourierTransformCurve);

	QXmlStreamAttributes attribs;
	QString str;

	while (!reader->atEnd()) {
		reader->readNext();
		if (reader->isEndElement() && reader->name() == QLatin1String("xyFourierTransformCurve"))
			break;

		if (!reader->isStartElement())
			continue;

		if (reader->name() == QLatin1String("xyAnalysisCurve")) {
			if (!XYAnalysisCurve::load(reader, preview))
				return false;
		} else if (!preview && reader->name() == QLatin1String("transformData")) {
			attribs = reader->attributes();
			READ_INT_VALUE("autoRange", transformData.autoRange, bool);
			READ_DOUBLE_VALUE("xRangeMin", transformData.xRange.first());
			READ_DOUBLE_VALUE("xRangeMax", transformData.xRange.last());
			READ_INT_VALUE("type", transformData.type, nsl_dft_result_type);
			READ_INT_VALUE("twoSided", transformData.twoSided, bool);
			READ_INT_VALUE("shifted", transformData.shifted, bool);
			READ_INT_VALUE("xScale", transformData.xScale, nsl_dft_xscale);
			READ_INT_VALUE("windowType", transformData.windowType, nsl_sf_window_type);
		} else if (!preview && reader->name() == QLatin1String("transformResult")) {
			attribs = reader->attributes();
			READ_INT_VALUE("available", transformResult.available, int);
			READ_INT_VALUE("valid", transformResult.valid, int);
			READ_STRING_VALUE("status", transformResult.status);
			READ_INT_VALUE("time", transformResult.elapsedTime, int);
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
