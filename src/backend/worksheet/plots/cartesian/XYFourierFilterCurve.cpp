/*
    File                 : XYFourierFilterCurve.cpp
    Project              : LabPlot
    Description          : A xy-curve defined by a Fourier filter
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2016 Stefan Gerlach <stefan.gerlach@uni.kn>
    SPDX-FileCopyrightText: 2017 Alexander Semke <alexander.semke@web.de>
    SPDX-License-Identifier: GPL-2.0-or-later
*/


/*!
  \class XYFourierFilterCurve
  \brief A xy-curve defined by a Fourier filter

  \ingroup worksheet
*/

#include "XYFourierFilterCurve.h"
#include "XYFourierFilterCurvePrivate.h"
#include "backend/core/AbstractColumn.h"
#include "backend/core/column/Column.h"
#include "backend/lib/commandtemplates.h"
#include "backend/lib/macros.h"
#include "backend/lib/XmlStreamReader.h"
#include "backend/gsl/errors.h"

extern "C" {
#include <gsl/gsl_sf_pow_int.h>
#ifdef HAVE_FFTW3
#include <fftw3.h>
#endif

#include "backend/nsl/nsl_sf_poly.h"
}

#include <QElapsedTimer>
#include <QIcon>
#include <QThreadPool>
#include <QDebug>	// qWarning()

XYFourierFilterCurve::XYFourierFilterCurve(const QString& name)
	: XYAnalysisCurve(name, new XYFourierFilterCurvePrivate(this), AspectType::XYFourierFilterCurve) {
}

XYFourierFilterCurve::XYFourierFilterCurve(const QString& name, XYFourierFilterCurvePrivate* dd)
	: XYAnalysisCurve(name, dd, AspectType::XYFourierFilterCurve) {
}

//no need to delete the d-pointer here - it inherits from QGraphicsItem
//and is deleted during the cleanup in QGraphicsScene
XYFourierFilterCurve::~XYFourierFilterCurve() = default;

void XYFourierFilterCurve::recalculate() {
	Q_D(XYFourierFilterCurve);
	d->recalculate();
}

/*!
	Returns an icon to be used in the project explorer.
*/
QIcon XYFourierFilterCurve::icon() const {
	return QIcon::fromTheme("labplot-xy-fourier-filter-curve");
}

//##############################################################################
//##########################  getter methods  ##################################
//##############################################################################
BASIC_SHARED_D_READER_IMPL(XYFourierFilterCurve, XYFourierFilterCurve::FilterData, filterData, filterData)

const XYFourierFilterCurve::FilterResult& XYFourierFilterCurve::filterResult() const {
	Q_D(const XYFourierFilterCurve);
	return d->filterResult;
}

//##############################################################################
//#################  setter methods and undo commands ##########################
//##############################################################################
STD_SETTER_CMD_IMPL_F_S(XYFourierFilterCurve, SetFilterData, XYFourierFilterCurve::FilterData, filterData, recalculate);
void XYFourierFilterCurve::setFilterData(const XYFourierFilterCurve::FilterData& filterData) {
	Q_D(XYFourierFilterCurve);
	exec(new XYFourierFilterCurveSetFilterDataCmd(d, filterData, ki18n("%1: set filter options and perform the Fourier filter")));
}

//##############################################################################
//######################### Private implementation #############################
//##############################################################################
XYFourierFilterCurvePrivate::XYFourierFilterCurvePrivate(XYFourierFilterCurve* owner) : XYAnalysisCurvePrivate(owner), q(owner) {
}

//no need to delete xColumn and yColumn, they are deleted
//when the parent aspect is removed
XYFourierFilterCurvePrivate::~XYFourierFilterCurvePrivate() = default;

void XYFourierFilterCurvePrivate::recalculate() {
	QElapsedTimer timer;
	timer.start();

	//create filter result columns if not available yet, clear them otherwise
	if (!xColumn) {
		xColumn = new Column("x", AbstractColumn::ColumnMode::Double);
		yColumn = new Column("y", AbstractColumn::ColumnMode::Double);
		xVector = static_cast<QVector<double>* >(xColumn->data());
		yVector = static_cast<QVector<double>* >(yColumn->data());

		xColumn->setHidden(true);
		q->addChild(xColumn);
		yColumn->setHidden(true);
		q->addChild(yColumn);

		q->setUndoAware(false);
		q->setXColumn(xColumn);
		q->setYColumn(yColumn);
		q->setUndoAware(true);
	} else {
		xVector->clear();
		yVector->clear();
	}

	// clear the previous result
	filterResult = XYFourierFilterCurve::FilterResult();

	//determine the data source columns
	const AbstractColumn* tmpXDataColumn = nullptr;
	const AbstractColumn* tmpYDataColumn = nullptr;
	if (dataSourceType == XYAnalysisCurve::DataSourceType::Spreadsheet) {
		//spreadsheet columns as data source
		tmpXDataColumn = xDataColumn;
		tmpYDataColumn = yDataColumn;
	} else {
		//curve columns as data source
		tmpXDataColumn = dataSourceCurve->xColumn();
		tmpYDataColumn = dataSourceCurve->yColumn();
	}

	if (!tmpXDataColumn || !tmpYDataColumn) {
		recalcLogicalPoints();
		emit q->dataChanged();
		sourceDataChangedSinceLastRecalc = false;
		return;
	}

	//copy all valid data point for the differentiation to temporary vectors
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


	int rowCount = qMin(tmpXDataColumn->rowCount(), tmpYDataColumn->rowCount());
	for (int row = 0; row < rowCount; ++row) {
		//only copy those data where _all_ values (for x and y, if given) are valid
		if (std::isnan(tmpXDataColumn->valueAt(row)) || std::isnan(tmpYDataColumn->valueAt(row))
			|| tmpXDataColumn->isMasked(row) || tmpYDataColumn->isMasked(row))
			continue;

		// only when inside given range
		if (tmpXDataColumn->valueAt(row) >= xmin && tmpXDataColumn->valueAt(row) <= xmax) {
			xdataVector.append(tmpXDataColumn->valueAt(row));
			ydataVector.append(tmpYDataColumn->valueAt(row));
		}

	}

	//number of data points to filter
	const size_t n = (size_t)xdataVector.size();
	if (n == 0) {
		filterResult.available = true;
		filterResult.valid = false;
		filterResult.status = i18n("No data points available.");
		recalcLogicalPoints();
		emit q->dataChanged();
		sourceDataChangedSinceLastRecalc = false;
		return;
	}

	//double* xdata = xdataVector.data();
	double* ydata = ydataVector.data();


	// filter settings
	const nsl_filter_type type = filterData.type;
	const nsl_filter_form form = filterData.form;
	const int order = filterData.order;
	const double cutoff = filterData.cutoff, cutoff2 = filterData.cutoff2;
	const nsl_filter_cutoff_unit unit = filterData.unit, unit2 = filterData.unit2;

	DEBUG("n ="<<n);
	DEBUG("type:"<<nsl_filter_type_name[type]);
	DEBUG("form (order "<<order<<") :"<<nsl_filter_form_name[form]);
	DEBUG("cutoffs ="<<cutoff<<cutoff2);
	DEBUG("unit :"<<nsl_filter_cutoff_unit_name[unit]<<nsl_filter_cutoff_unit_name[unit2]);

///////////////////////////////////////////////////////////
	// calculate index
	double cutindex = 0, cutindex2 = 0;
	switch (unit) {
	case nsl_filter_cutoff_unit_frequency:
		cutindex = cutoff*(xmax-xmin);
		break;
	case nsl_filter_cutoff_unit_fraction:
		cutindex = cutoff*(int)n;
		break;
	case nsl_filter_cutoff_unit_index:
		cutindex = cutoff;
	}
	switch (unit2) {
	case nsl_filter_cutoff_unit_frequency:
		cutindex2 = cutoff2*(xmax-xmin);
		break;
	case nsl_filter_cutoff_unit_fraction:
		cutindex2 = cutoff2*n;
		break;
	case nsl_filter_cutoff_unit_index:
		cutindex2 = cutoff2;
	}
	const double bandwidth = (cutindex2 - cutindex);
	if ((type == nsl_filter_type_band_pass || type == nsl_filter_type_band_reject) && bandwidth <= 0) {
		qWarning()<<"band width must be > 0. Giving up.";
		return;
	}

	DEBUG("cut off @" << cutindex << cutindex2);
	DEBUG("bandwidth =" << bandwidth);

	// run filter
	int status = nsl_filter_fourier(ydata, n, type, form, order, cutindex, bandwidth);

	xVector->resize((int)n);
	yVector->resize((int)n);
	memcpy(xVector->data(), xdataVector.data(), n*sizeof(double));
	memcpy(yVector->data(), ydata, n*sizeof(double));
///////////////////////////////////////////////////////////

	//write the result
	filterResult.available = true;
	filterResult.valid = true;
	filterResult.status = gslErrorToString(status);
	filterResult.elapsedTime = timer.elapsed();

	//redraw the curve
	recalcLogicalPoints();
	emit q->dataChanged();
	sourceDataChangedSinceLastRecalc = false;
}

//##############################################################################
//##################  Serialization/Deserialization  ###########################
//##############################################################################
//! Save as XML
void XYFourierFilterCurve::save(QXmlStreamWriter* writer) const {
	Q_D(const XYFourierFilterCurve);

	writer->writeStartElement("xyFourierFilterCurve");

	//write the base class
	XYAnalysisCurve::save(writer);

	//write xy-fourier_filter-curve specific information
	//filter data
	writer->writeStartElement("filterData");
	writer->writeAttribute( "autoRange", QString::number(d->filterData.autoRange) );
	writer->writeAttribute( "xRangeMin", QString::number(d->filterData.xRange.first()) );
	writer->writeAttribute( "xRangeMax", QString::number(d->filterData.xRange.last()) );
	writer->writeAttribute( "type", QString::number(d->filterData.type) );
	writer->writeAttribute( "form", QString::number(d->filterData.form) );
	writer->writeAttribute( "order", QString::number(d->filterData.order) );
	writer->writeAttribute( "cutoff", QString::number(d->filterData.cutoff) );
	writer->writeAttribute( "unit", QString::number(d->filterData.unit) );
	writer->writeAttribute( "cutoff2", QString::number(d->filterData.cutoff2) );
	writer->writeAttribute( "unit2", QString::number(d->filterData.unit2) );
	writer->writeEndElement();// filterData

	//filter results (generated columns)
	writer->writeStartElement("filterResult");
	writer->writeAttribute( "available", QString::number(d->filterResult.available) );
	writer->writeAttribute( "valid", QString::number(d->filterResult.valid) );
	writer->writeAttribute( "status", d->filterResult.status );
	writer->writeAttribute( "time", QString::number(d->filterResult.elapsedTime) );

	//save calculated columns if available
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

	KLocalizedString attributeWarning = ki18n("Attribute '%1' missing or empty, default value is used");
	QXmlStreamAttributes attribs;
	QString str;

	while (!reader->atEnd()) {
		reader->readNext();
		if (reader->isEndElement() && reader->name() == "xyFourierFilterCurve")
			break;

		if (!reader->isStartElement())
			continue;

		if (reader->name() == "xyAnalysisCurve") {
			if ( !XYAnalysisCurve::load(reader, preview) )
				return false;
		} else if (!preview && reader->name() == "filterData") {
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
		} else if (!preview && reader->name() == "filterResult") {
			attribs = reader->attributes();
			READ_INT_VALUE("available", filterResult.available, int);
			READ_INT_VALUE("valid", filterResult.valid, int);
			READ_STRING_VALUE("status", filterResult.status);
			READ_INT_VALUE("time", filterResult.elapsedTime, int);
		} else if (reader->name() == "column") {
			Column* column = new Column(QString(), AbstractColumn::ColumnMode::Double);
			if (!column->load(reader, preview)) {
				delete column;
				return false;
			}

			if (column->name() == "x")
				d->xColumn = column;
			else if (column->name() == "y")
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

		d->xVector = static_cast<QVector<double>* >(d->xColumn->data());
		d->yVector = static_cast<QVector<double>* >(d->yColumn->data());

		static_cast<XYCurvePrivate*>(d_ptr)->xColumn = d->xColumn;
		static_cast<XYCurvePrivate*>(d_ptr)->yColumn = d->yColumn;

		recalcLogicalPoints();
	}

	return true;
}
