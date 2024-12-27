/*
	File                 : XYDifferentiationCurve.cpp
	Project              : LabPlot
	Description          : A xy-curve defined by an differentiation
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2016-2022 Stefan Gerlach <stefan.gerlach@uni.kn>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "XYDifferentiationCurve.h"
#include "XYDifferentiationCurvePrivate.h"
#include "backend/core/column/Column.h"
#include "backend/lib/XmlStreamReader.h"
#include "backend/lib/commandtemplates.h"

#include <KLocalizedString>
#include <QElapsedTimer>
#include <QIcon>
#include <QThreadPool>

/*!
 * \class XYDifferentiationCurve
 * \brief A xy-curve defined by a differentiation.
 * \ingroup CartesianAnalysisPlots
*/
XYDifferentiationCurve::XYDifferentiationCurve(const QString& name)
	: XYAnalysisCurve(name, new XYDifferentiationCurvePrivate(this), AspectType::XYDifferentiationCurve) {
}

XYDifferentiationCurve::XYDifferentiationCurve(const QString& name, XYDifferentiationCurvePrivate* dd)
	: XYAnalysisCurve(name, dd, AspectType::XYDifferentiationCurve) {
}

// no need to delete the d-pointer here - it inherits from QGraphicsItem
// and is deleted during the cleanup in QGraphicsScene
XYDifferentiationCurve::~XYDifferentiationCurve() = default;

void XYDifferentiationCurve::recalculate() {
	Q_D(XYDifferentiationCurve);
	d->recalculate();
}

const XYAnalysisCurve::Result& XYDifferentiationCurve::result() const {
	return differentiationResult();
}

/*!
	Returns an icon to be used in the project explorer.
*/
QIcon XYDifferentiationCurve::icon() const {
	return QIcon::fromTheme(QStringLiteral("labplot-xy-curve"));
}

// ##############################################################################
// ##########################  getter methods  ##################################
// ##############################################################################
BASIC_SHARED_D_READER_IMPL(XYDifferentiationCurve, XYDifferentiationCurve::DifferentiationData, differentiationData, differentiationData)

const XYDifferentiationCurve::DifferentiationResult& XYDifferentiationCurve::differentiationResult() const {
	Q_D(const XYDifferentiationCurve);
	return d->differentiationResult;
}

// ##############################################################################
// #################  setter methods and undo commands ##########################
// ##############################################################################
STD_SETTER_CMD_IMPL_F_S(XYDifferentiationCurve, SetDifferentiationData, XYDifferentiationCurve::DifferentiationData, differentiationData, recalculate)
void XYDifferentiationCurve::setDifferentiationData(const XYDifferentiationCurve::DifferentiationData& differentiationData) {
	Q_D(XYDifferentiationCurve);
	exec(new XYDifferentiationCurveSetDifferentiationDataCmd(d, differentiationData, ki18n("%1: set options and perform the differentiation")));
}

// ##############################################################################
// ######################### Private implementation #############################
// ##############################################################################
XYDifferentiationCurvePrivate::XYDifferentiationCurvePrivate(XYDifferentiationCurve* owner)
	: XYAnalysisCurvePrivate(owner)
	, q(owner) {
}

// no need to delete xColumn and yColumn, they are deleted
// when the parent aspect is removed
XYDifferentiationCurvePrivate::~XYDifferentiationCurvePrivate() = default;

void XYDifferentiationCurvePrivate::resetResults() {
	differentiationResult = XYDifferentiationCurve::DifferentiationResult();
}

// ...
// see XYFitCurvePrivate
bool XYDifferentiationCurvePrivate::recalculateSpecific(const AbstractColumn* tmpXDataColumn, const AbstractColumn* tmpYDataColumn) {
	QElapsedTimer timer;
	timer.start();

	// copy all valid data point for the differentiation to temporary vectors
	QVector<double> xdataVector;
	QVector<double> ydataVector;

	double xmin;
	double xmax;
	if (differentiationData.autoRange) {
		xmin = tmpXDataColumn->minimum();
		xmax = tmpXDataColumn->maximum();
	} else {
		xmin = differentiationData.xRange.first();
		xmax = differentiationData.xRange.last();
	}

	XYAnalysisCurve::copyData(xdataVector, ydataVector, tmpXDataColumn, tmpYDataColumn, xmin, xmax, true);

	// number of data points to differentiate
	const size_t n = (size_t)xdataVector.size();
	if (n < 3) {
		differentiationResult.available = true;
		differentiationResult.valid = false;
		differentiationResult.status = i18n("Not enough data points available.");
		return true;
	}

	double* xdata = xdataVector.data();
	double* ydata = ydataVector.data();

	// differentiation settings
	const nsl_diff_deriv_order_type derivOrder = differentiationData.derivOrder;
	const int accOrder = differentiationData.accOrder;

	DEBUG(nsl_diff_deriv_order_name[derivOrder] << " derivative");
	DEBUG("accuracy order: " << accOrder);
	// WARN("DATA:")
	// for (int i = 0; i < n; i++)
	//	WARN(xdata[i] << "," << ydata[i])

	///////////////////////////////////////////////////////////
	int status = 0;

	switch (derivOrder) {
	case nsl_diff_deriv_order_first:
		status = nsl_diff_first_deriv(xdata, ydata, n, accOrder);
		break;
	case nsl_diff_deriv_order_second:
		status = nsl_diff_second_deriv(xdata, ydata, n, accOrder);
		break;
	case nsl_diff_deriv_order_third:
		status = nsl_diff_third_deriv(xdata, ydata, n, accOrder);
		break;
	case nsl_diff_deriv_order_fourth:
		status = nsl_diff_fourth_deriv(xdata, ydata, n, accOrder);
		break;
	case nsl_diff_deriv_order_fifth:
		status = nsl_diff_fifth_deriv(xdata, ydata, n, accOrder);
		break;
	case nsl_diff_deriv_order_sixth:
		status = nsl_diff_sixth_deriv(xdata, ydata, n, accOrder);
		break;
	}

	xVector->resize((int)n);
	yVector->resize((int)n);
	memcpy(xVector->data(), xdata, n * sizeof(double));
	memcpy(yVector->data(), ydata, n * sizeof(double));
	///////////////////////////////////////////////////////////
	// WARN("RESULT:")
	// for (int i = 0; i < n; i++)
	//	WARN(xdata[i] << "," << ydata[i])

	// write the result
	differentiationResult.available = true;
	differentiationResult.valid = (status == 0);
	differentiationResult.status = QString::number(status);
	differentiationResult.elapsedTime = timer.elapsed();

	return true;
}

// ##############################################################################
// ##################  Serialization/Deserialization  ###########################
// ##############################################################################
//! Save as XML
void XYDifferentiationCurve::save(QXmlStreamWriter* writer) const {
	Q_D(const XYDifferentiationCurve);

	writer->writeStartElement(QStringLiteral("xyDifferentiationCurve"));

	// write the base class
	XYAnalysisCurve::save(writer);

	// write xy-differentiation-curve specific information
	//  differentiation data
	writer->writeStartElement(QStringLiteral("differentiationData"));
	writer->writeAttribute(QStringLiteral("derivOrder"), QString::number(d->differentiationData.derivOrder));
	writer->writeAttribute(QStringLiteral("accOrder"), QString::number(d->differentiationData.accOrder));
	writer->writeAttribute(QStringLiteral("autoRange"), QString::number(d->differentiationData.autoRange));
	writer->writeAttribute(QStringLiteral("xRangeMin"), QString::number(d->differentiationData.xRange.first()));
	writer->writeAttribute(QStringLiteral("xRangeMax"), QString::number(d->differentiationData.xRange.last()));
	writer->writeEndElement(); // differentiationData

	// differentiation results (generated columns)
	writer->writeStartElement(QStringLiteral("differentiationResult"));
	writer->writeAttribute(QStringLiteral("available"), QString::number(d->differentiationResult.available));
	writer->writeAttribute(QStringLiteral("valid"), QString::number(d->differentiationResult.valid));
	writer->writeAttribute(QStringLiteral("status"), d->differentiationResult.status);
	writer->writeAttribute(QStringLiteral("time"), QString::number(d->differentiationResult.elapsedTime));

	// save calculated columns if available
	if (saveCalculations() && d->xColumn) {
		d->xColumn->save(writer);
		d->yColumn->save(writer);
	}
	writer->writeEndElement(); //"differentiationResult"

	writer->writeEndElement(); //"xyDifferentiationCurve"
}

//! Load from XML
bool XYDifferentiationCurve::load(XmlStreamReader* reader, bool preview) {
	Q_D(XYDifferentiationCurve);

	QXmlStreamAttributes attribs;
	QString str;

	while (!reader->atEnd()) {
		reader->readNext();
		if (reader->isEndElement() && reader->name() == QLatin1String("xyDifferentiationCurve"))
			break;

		if (!reader->isStartElement())
			continue;

		if (reader->name() == QLatin1String("xyAnalysisCurve")) {
			if (!XYAnalysisCurve::load(reader, preview))
				return false;
		} else if (!preview && reader->name() == QLatin1String("differentiationData")) {
			attribs = reader->attributes();
			READ_INT_VALUE("autoRange", differentiationData.autoRange, bool);
			READ_DOUBLE_VALUE("xRangeMin", differentiationData.xRange.first());
			READ_DOUBLE_VALUE("xRangeMax", differentiationData.xRange.last());
			READ_INT_VALUE("derivOrder", differentiationData.derivOrder, nsl_diff_deriv_order_type);
			READ_INT_VALUE("accOrder", differentiationData.accOrder, int);
		} else if (!preview && reader->name() == QLatin1String("differentiationResult")) {
			attribs = reader->attributes();
			READ_INT_VALUE("available", differentiationResult.available, int);
			READ_INT_VALUE("valid", differentiationResult.valid, int);
			READ_STRING_VALUE("status", differentiationResult.status);
			READ_INT_VALUE("time", differentiationResult.elapsedTime, int);
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
