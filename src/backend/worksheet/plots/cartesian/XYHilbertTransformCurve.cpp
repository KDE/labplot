/*
	File                 : XYHilbertTransformCurve.cpp
	Project              : LabPlot
	Description          : A xy-curve defined by a Hilbert transform
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2021 Stefan Gerlach <stefan.gerlach@uni.kn>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

/*!
  \class XYHilbertTransformCurve
  \brief A xy-curve defined by a Hilbert transform

  \ingroup worksheet
*/

#include "XYHilbertTransformCurve.h"
#include "XYHilbertTransformCurvePrivate.h"
#include "backend/core/AbstractColumn.h"
#include "backend/core/column/Column.h"
#include "backend/gsl/errors.h"
#include "backend/lib/XmlStreamReader.h"
#include "backend/lib/commandtemplates.h"
#include "backend/lib/macros.h"

#include <KLocalizedString>
#include <QDebug> // qWarning()
#include <QElapsedTimer>
#include <QIcon>
#include <QThreadPool>

XYHilbertTransformCurve::XYHilbertTransformCurve(const QString& name)
	: XYAnalysisCurve(name, new XYHilbertTransformCurvePrivate(this), AspectType::XYHilbertTransformCurve) {
}

XYHilbertTransformCurve::XYHilbertTransformCurve(const QString& name, XYHilbertTransformCurvePrivate* dd)
	: XYAnalysisCurve(name, dd, AspectType::XYHilbertTransformCurve) {
}

// no need to delete the d-pointer here - it inherits from QGraphicsItem
// and is deleted during the cleanup in QGraphicsScene
XYHilbertTransformCurve::~XYHilbertTransformCurve() = default;

void XYHilbertTransformCurve::recalculate() {
	Q_D(XYHilbertTransformCurve);
	d->recalculate();
}

const XYAnalysisCurve::Result& XYHilbertTransformCurve::result() const {
	Q_D(const XYHilbertTransformCurve);
	return d->transformResult;
}

/*!
	Returns an icon to be used in the project explorer.
*/
QIcon XYHilbertTransformCurve::icon() const {
	return QIcon::fromTheme(QStringLiteral("labplot-xy-fourier-transform-curve"));
}

// ##############################################################################
// ##########################  getter methods  ##################################
// ##############################################################################
BASIC_SHARED_D_READER_IMPL(XYHilbertTransformCurve, XYHilbertTransformCurve::TransformData, transformData, transformData)

// ##############################################################################
// #################  setter methods and undo commands ##########################
// ##############################################################################
STD_SETTER_CMD_IMPL_F_S(XYHilbertTransformCurve, SetTransformData, XYHilbertTransformCurve::TransformData, transformData, recalculate)
void XYHilbertTransformCurve::setTransformData(const XYHilbertTransformCurve::TransformData& transformData) {
	Q_D(XYHilbertTransformCurve);
	exec(new XYHilbertTransformCurveSetTransformDataCmd(d, transformData, ki18n("%1: set transform options and perform the Hilbert transform")));
}

// ##############################################################################
// ######################### Private implementation #############################
// ##############################################################################
XYHilbertTransformCurvePrivate::XYHilbertTransformCurvePrivate(XYHilbertTransformCurve* owner)
	: XYAnalysisCurvePrivate(owner)
	, q(owner) {
}

// no need to delete xColumn and yColumn, they are deleted
// when the parent aspect is removed
XYHilbertTransformCurvePrivate::~XYHilbertTransformCurvePrivate() = default;

void XYHilbertTransformCurvePrivate::resetResults() {
	transformResult = XYHilbertTransformCurve::TransformResult();
}

bool XYHilbertTransformCurvePrivate::recalculateSpecific(const AbstractColumn* tmpXDataColumn, const AbstractColumn* tmpYDataColumn) {
	DEBUG(Q_FUNC_INFO)
	if (!tmpXDataColumn || !tmpYDataColumn)
		return false;

	QElapsedTimer timer;
	timer.start();

	// copy all valid data point for the transform to temporary vectors
	QVector<double> xdataVector;
	QVector<double> ydataVector;
	double xmin, xmax;
	if (transformData.autoRange) {
		xmin = tmpXDataColumn->minimum();
		xmax = tmpXDataColumn->maximum();
	} else {
		xmin = transformData.xRange.first();
		xmax = transformData.xRange.last();
	}

	int rowCount = std::min(tmpXDataColumn->rowCount(), tmpYDataColumn->rowCount());
	DEBUG(Q_FUNC_INFO << ", row count = " << rowCount)
	DEBUG(Q_FUNC_INFO << ", xmin/xmax = " << xmin << '/' << xmax)
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
	unsigned int n = (unsigned int)ydataVector.size();
	if (n == 0) {
		transformResult.available = true;
		transformResult.valid = false;
		transformResult.status = i18n("No data points available.");
		DEBUG(Q_FUNC_INFO << "no data (n = 0)!")
		return true;
	}

	double* xdata = xdataVector.data();
	double* ydata = ydataVector.data();

	// transform settings
	const nsl_hilbert_result_type type = transformData.type;

	DEBUG("n = " << n);
	DEBUG("type:" << nsl_hilbert_result_type_name[type]);
#ifndef NDEBUG
//	QDebug out = qDebug();
//	for (unsigned int i = 0; i < n; i++)
//		out<<ydata[i];
#endif

	///////////////////////////////////////////////////////////
	// transform with window
	//	TODO: type
	gsl_set_error_handler_off();
	int status = nsl_hilbert_transform(ydata, 1, n, type);

	unsigned int N = n;
#ifndef NDEBUG
//	out = qDebug();
//	for (unsigned int i = 0; i < N; i++)
//		out << ydata[i] << '(' << xdata[i] << ')';
#endif

	xVector->resize((int)N);
	yVector->resize((int)N);
	memcpy(xVector->data(), xdata, N * sizeof(double));
	memcpy(yVector->data(), ydata, N * sizeof(double));
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
void XYHilbertTransformCurve::save(QXmlStreamWriter* writer) const {
	Q_D(const XYHilbertTransformCurve);

	writer->writeStartElement(QStringLiteral("xyHilbertTransformCurve"));

	// write the base class
	XYAnalysisCurve::save(writer);

	// write xy-fourier_transform-curve specific information
	// transform data
	writer->writeStartElement(QStringLiteral("transformData"));
	writer->writeAttribute(QStringLiteral("autoRange"), QString::number(d->transformData.autoRange));
	writer->writeAttribute(QStringLiteral("xRangeMin"), QString::number(d->transformData.xRange.first()));
	writer->writeAttribute(QStringLiteral("xRangeMax"), QString::number(d->transformData.xRange.last()));
	writer->writeAttribute(QStringLiteral("type"), QString::number(d->transformData.type));
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
	writer->writeEndElement(); //"xyHilbertTransformCurve"
}

//! Load from XML
bool XYHilbertTransformCurve::load(XmlStreamReader* reader, bool preview) {
	Q_D(XYHilbertTransformCurve);

	QXmlStreamAttributes attribs;
	QString str;

	while (!reader->atEnd()) {
		reader->readNext();
		if (reader->isEndElement() && reader->name() == QLatin1String("xyHilbertTransformCurve"))
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
			READ_INT_VALUE("type", transformData.type, nsl_hilbert_result_type);
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
