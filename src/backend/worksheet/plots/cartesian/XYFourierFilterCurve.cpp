/*
	File                 : XYFourierFilterCurve.cpp
	Project              : LabPlot
	Description          : A xy-curve defined by a Fourier filter
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2016 Stefan Gerlach <stefan.gerlach@uni.kn>
	SPDX-FileCopyrightText: 2017 Alexander Semke <alexander.semke@web.de>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "XYFourierFilterCurve.h"
#include "XYFourierFilterCurvePrivate.h"
#include "backend/core/column/Column.h"
#include "backend/gsl/errors.h"
#include "backend/lib/XmlStreamReader.h"
#include "backend/lib/commandtemplates.h"

#include <gsl/gsl_sf_pow_int.h>
extern "C" {
#ifdef HAVE_FFTW3
#include <fftw3.h>
#endif

#include "backend/nsl/nsl_sf_poly.h"
}

#include <QElapsedTimer>
#include <QDebug> // qWarning()
#include <QIcon>
#include <QThreadPool>

/*!
 * \class XYFourierFilterCurve
 * \brief A xy-curve defined by a Fourier filter.
 * \ingroup CartesianAnalysisPlots
 */
XYFourierFilterCurve::XYFourierFilterCurve(const QString& name)
	: XYAnalysisCurve(name, new XYFourierFilterCurvePrivate(this), AspectType::XYFourierFilterCurve) {
}

XYFourierFilterCurve::XYFourierFilterCurve(const QString& name, XYFourierFilterCurvePrivate* dd)
	: XYAnalysisCurve(name, dd, AspectType::XYFourierFilterCurve) {
}

// no need to delete the d-pointer here - it inherits from QGraphicsItem
// and is deleted during the cleanup in QGraphicsScene
XYFourierFilterCurve::~XYFourierFilterCurve() = default;

const XYAnalysisCurve::Result& XYFourierFilterCurve::result() const {
	Q_D(const XYFourierFilterCurve);
	return d->filterResult;
}

/*!
	Returns an icon to be used in the project explorer.
*/
QIcon XYFourierFilterCurve::icon() const {
	return QIcon::fromTheme(QStringLiteral("labplot-xy-fourier-filter-curve"));
}

// ##############################################################################
// ##########################  getter methods  ##################################
// ##############################################################################
BASIC_SHARED_D_READER_IMPL(XYFourierFilterCurve, XYFourierFilterCurve::FilterData, filterData, filterData)

const XYFourierFilterCurve::FilterResult& XYFourierFilterCurve::filterResult() const {
	Q_D(const XYFourierFilterCurve);
	return d->filterResult;
}

// ##############################################################################
// #################  setter methods and undo commands ##########################
// ##############################################################################
STD_SETTER_CMD_IMPL_F_S(XYFourierFilterCurve, SetFilterData, XYFourierFilterCurve::FilterData, filterData, recalculate)
void XYFourierFilterCurve::setFilterData(const XYFourierFilterCurve::FilterData& filterData) {
	Q_D(XYFourierFilterCurve);
	exec(new XYFourierFilterCurveSetFilterDataCmd(d, filterData, ki18n("%1: set filter options and perform the Fourier filter")));
}

// ##############################################################################
// ######################### Private implementation #############################
// ##############################################################################
XYFourierFilterCurvePrivate::XYFourierFilterCurvePrivate(XYFourierFilterCurve* owner)
	: XYAnalysisCurvePrivate(owner)
	, q(owner) {
}

// no need to delete xColumn and yColumn, they are deleted
// when the parent aspect is removed
XYFourierFilterCurvePrivate::~XYFourierFilterCurvePrivate() = default;

void XYFourierFilterCurvePrivate::resetResults() {
	filterResult = XYFourierFilterCurve::FilterResult();
}

bool XYFourierFilterCurvePrivate::recalculateSpecific(const AbstractColumn* tmpXDataColumn, const AbstractColumn* tmpYDataColumn) {
	QElapsedTimer timer;
	timer.start();

	// copy all valid data point for the differentiation to temporary vectors
	QVector<double> xdataVector;
	QVector<double> ydataVector;

	double xmin;
	double xmax;
	if (filterData.autoRange) {
		xmin = tmpXDataColumn->minimum();
		xmax = tmpXDataColumn->maximum();
	} else {
		xmin = filterData.xRange.first();
		xmax = filterData.xRange.last();
	}

	int rowCount = std::min(tmpXDataColumn->rowCount(), tmpYDataColumn->rowCount());
	const bool xNumeric = tmpXDataColumn->columnMode() == AbstractColumn::ColumnMode::BigInt
		|| tmpXDataColumn->columnMode() == AbstractColumn::ColumnMode::Double || tmpXDataColumn->columnMode() == AbstractColumn::ColumnMode::Integer;
	const bool xDateTime = tmpXDataColumn->columnMode() == AbstractColumn::ColumnMode::DateTime;
	const bool yNumeric = tmpYDataColumn->columnMode() == AbstractColumn::ColumnMode::BigInt
		|| tmpYDataColumn->columnMode() == AbstractColumn::ColumnMode::Double || tmpYDataColumn->columnMode() == AbstractColumn::ColumnMode::Integer;
	const bool yDateTime = tmpYDataColumn->columnMode() == AbstractColumn::ColumnMode::DateTime;
	if (xNumeric && yNumeric) {
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
	} else if (xDateTime && yNumeric) {
		for (int row = 0; row < rowCount; ++row) {
			// only copy those data where _all_ values (for x and y, if given) are valid
			const double xDT = tmpXDataColumn->dateTimeAt(row).toMSecsSinceEpoch();
			if (std::isnan(xDT) || std::isnan(tmpYDataColumn->valueAt(row)) || tmpXDataColumn->isMasked(row) || tmpYDataColumn->isMasked(row))
				continue;

			// only when inside given range
			if (xDT >= xmin && xDT <= xmax) {
				xdataVector.append(xDT);
				ydataVector.append(tmpYDataColumn->valueAt(row));
			}
		}
	} else if (yDateTime && xNumeric) {
		for (int row = 0; row < rowCount; ++row) {
			// only copy those data where _all_ values (for x and y, if given) are valid
			const double yDT = tmpYDataColumn->dateTimeAt(row).toMSecsSinceEpoch();
			if (std::isnan(tmpXDataColumn->valueAt(row)) || std::isnan(yDT) || tmpXDataColumn->isMasked(row) || tmpYDataColumn->isMasked(row))
				continue;

			// only when inside given range
			if (tmpXDataColumn->valueAt(row) >= xmin && tmpXDataColumn->valueAt(row) <= xmax) {
				xdataVector.append(tmpXDataColumn->valueAt(row));
				ydataVector.append(yDT);
			}
		}
	}

	// number of data points to filter
	const size_t n = (size_t)xdataVector.size();
	if (n == 0) {
		filterResult.available = true;
		filterResult.valid = false;
		filterResult.status = i18n("No data points available.");
		return true;
	}

	// double* xdata = xdataVector.data();
	double* ydata = ydataVector.data();

	// filter settings
	const nsl_filter_type type = filterData.type;
	const nsl_filter_form form = filterData.form;
	const int order = filterData.order;
	const double cutoff = filterData.cutoff, cutoff2 = filterData.cutoff2;
	const nsl_filter_cutoff_unit unit = filterData.unit, unit2 = filterData.unit2;

	DEBUG("n =" << n);
	DEBUG("type:" << nsl_filter_type_name[type]);
	DEBUG("form (order " << order << ") :" << nsl_filter_form_name[form]);
	DEBUG("cutoffs =" << cutoff << cutoff2);
	DEBUG("unit :" << nsl_filter_cutoff_unit_name[unit] << nsl_filter_cutoff_unit_name[unit2]);

	///////////////////////////////////////////////////////////
	// calculate index
	double cutindex = 0, cutindex2 = 0;
	switch (unit) {
	case nsl_filter_cutoff_unit_frequency:
		cutindex = cutoff * (xmax - xmin);
		if (xDateTime)
			cutindex /= 1000;
		break;
	case nsl_filter_cutoff_unit_fraction:
		cutindex = cutoff * (int)n;
		break;
	case nsl_filter_cutoff_unit_index:
		cutindex = cutoff;
	}
	switch (unit2) {
	case nsl_filter_cutoff_unit_frequency:
		cutindex2 = cutoff2 * (xmax - xmin);
		if (xDateTime)
			cutindex2 /= 1000;
		break;
	case nsl_filter_cutoff_unit_fraction:
		cutindex2 = cutoff2 * n;
		break;
	case nsl_filter_cutoff_unit_index:
		cutindex2 = cutoff2;
	}
	const double bandwidth = (cutindex2 - cutindex);
	if ((type == nsl_filter_type_band_pass || type == nsl_filter_type_band_reject) && bandwidth <= 0) {
		qWarning() << "band width must be > 0. Giving up.";
		return false;
	}

	DEBUG("cut off @" << cutindex << cutindex2);
	DEBUG("bandwidth =" << bandwidth);

	// run filter
	gsl_set_error_handler_off();
	int status = nsl_filter_fourier(ydata, n, type, form, order, cutindex, bandwidth);

	xVector->resize((int)n);
	yVector->resize((int)n);
	memcpy(xVector->data(), xdataVector.data(), n * sizeof(double));
	memcpy(yVector->data(), ydata, n * sizeof(double));
	///////////////////////////////////////////////////////////

	// write the result
	filterResult.available = true;
	filterResult.valid = (status == GSL_SUCCESS);
	filterResult.status = gslErrorToString(status);
	filterResult.elapsedTime = timer.elapsed();
	return true;
}

// ##############################################################################
// ##################  Serialization/Deserialization  ###########################
// ##############################################################################
//! Save as XML
void XYFourierFilterCurve::save(QXmlStreamWriter* writer) const {
	Q_D(const XYFourierFilterCurve);

	writer->writeStartElement(QStringLiteral("xyFourierFilterCurve"));

	// write the base class
	XYAnalysisCurve::save(writer);

	// write xy-fourier_filter-curve specific information
	// filter data
	writer->writeStartElement(QStringLiteral("filterData"));
	writer->writeAttribute(QStringLiteral("autoRange"), QString::number(d->filterData.autoRange));
	writer->writeAttribute(QStringLiteral("xRangeMin"), QString::number(d->filterData.xRange.first()));
	writer->writeAttribute(QStringLiteral("xRangeMax"), QString::number(d->filterData.xRange.last()));
	writer->writeAttribute(QStringLiteral("type"), QString::number(d->filterData.type));
	writer->writeAttribute(QStringLiteral("form"), QString::number(d->filterData.form));
	writer->writeAttribute(QStringLiteral("order"), QString::number(d->filterData.order));
	writer->writeAttribute(QStringLiteral("cutoff"), QString::number(d->filterData.cutoff));
	writer->writeAttribute(QStringLiteral("unit"), QString::number(d->filterData.unit));
	writer->writeAttribute(QStringLiteral("cutoff2"), QString::number(d->filterData.cutoff2));
	writer->writeAttribute(QStringLiteral("unit2"), QString::number(d->filterData.unit2));
	writer->writeEndElement(); // filterData

	// filter results (generated columns)
	writer->writeStartElement(QStringLiteral("filterResult"));
	writer->writeAttribute(QStringLiteral("available"), QString::number(d->filterResult.available));
	writer->writeAttribute(QStringLiteral("valid"), QString::number(d->filterResult.valid));
	writer->writeAttribute(QStringLiteral("status"), d->filterResult.status);
	writer->writeAttribute(QStringLiteral("time"), QString::number(d->filterResult.elapsedTime));

	// save calculated columns if available
	if (saveCalculations() && d->xColumn && d->yColumn) {
		d->xColumn->save(writer);
		d->yColumn->save(writer);
	}
	writer->writeEndElement(); //"filterResult"
	writer->writeEndElement(); //"xyFourierFilterCurve"
}

//! Load from XML
bool XYFourierFilterCurve::load(XmlStreamReader* reader, bool preview) {
	Q_D(XYFourierFilterCurve);

	QXmlStreamAttributes attribs;
	QString str;

	while (!reader->atEnd()) {
		reader->readNext();
		if (reader->isEndElement() && reader->name() == QLatin1String("xyFourierFilterCurve"))
			break;

		if (!reader->isStartElement())
			continue;

		if (reader->name() == QLatin1String("xyAnalysisCurve")) {
			if (!XYAnalysisCurve::load(reader, preview))
				return false;
		} else if (!preview && reader->name() == QLatin1String("filterData")) {
			attribs = reader->attributes();
			READ_INT_VALUE("autoRange", filterData.autoRange, bool);
			READ_DOUBLE_VALUE("xRangeMin", filterData.xRange.first());
			READ_DOUBLE_VALUE("xRangeMax", filterData.xRange.last());
			READ_INT_VALUE("type", filterData.type, nsl_filter_type);
			READ_INT_VALUE("form", filterData.form, nsl_filter_form);
			READ_INT_VALUE("order", filterData.order, int);
			READ_DOUBLE_VALUE("cutoff", filterData.cutoff);
			READ_INT_VALUE("unit", filterData.unit, nsl_filter_cutoff_unit);
			READ_DOUBLE_VALUE("cutoff2", filterData.cutoff2);
			READ_INT_VALUE("unit2", filterData.unit2, nsl_filter_cutoff_unit);
		} else if (!preview && reader->name() == QLatin1String("filterResult")) {
			attribs = reader->attributes();
			READ_INT_VALUE("available", filterResult.available, int);
			READ_INT_VALUE("valid", filterResult.valid, int);
			READ_STRING_VALUE("status", filterResult.status);
			READ_INT_VALUE("time", filterResult.elapsedTime, int);
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
