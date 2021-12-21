/*
    File                 : XYIntegrationCurve.cpp
    Project              : LabPlot
    Description          : A xy-curve defined by an integration
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2016 Stefan Gerlach <stefan.gerlach@uni.kn>
    SPDX-FileCopyrightText: 2017 Alexander Semke <alexander.semke@web.de>
    SPDX-License-Identifier: GPL-2.0-or-later
*/


/*!
  \class XYIntegrationCurve
  \brief A xy-curve defined by an integration

  \ingroup worksheet
*/

#include "XYIntegrationCurve.h"
#include "XYIntegrationCurvePrivate.h"
#include "CartesianCoordinateSystem.h"
#include "backend/core/column/Column.h"
#include "backend/lib/commandtemplates.h"
#include "backend/lib/macros.h"
#include "backend/lib/XmlStreamReader.h"

extern "C" {
#include <gsl/gsl_errno.h>
}

#include <KLocalizedString>
#include <QIcon>
#include <QElapsedTimer>
#include <QThreadPool>

XYIntegrationCurve::XYIntegrationCurve(const QString& name)
	: XYAnalysisCurve(name, new XYIntegrationCurvePrivate(this), AspectType::XYIntegrationCurve) {
}

XYIntegrationCurve::XYIntegrationCurve(const QString& name, XYIntegrationCurvePrivate* dd)
	: XYAnalysisCurve(name, dd, AspectType::XYIntegrationCurve) {
}

//no need to delete the d-pointer here - it inherits from QGraphicsItem
//and is deleted during the cleanup in QGraphicsScene
XYIntegrationCurve::~XYIntegrationCurve() = default;

void XYIntegrationCurve::recalculate() {
	Q_D(XYIntegrationCurve);
	d->recalculate();
}

/*!
	Returns an icon to be used in the project explorer.
*/
QIcon XYIntegrationCurve::icon() const {
	return QIcon::fromTheme("labplot-xy-curve");
}

//##############################################################################
//##########################  getter methods  ##################################
//##############################################################################
BASIC_SHARED_D_READER_IMPL(XYIntegrationCurve, XYIntegrationCurve::IntegrationData, integrationData, integrationData)

const XYIntegrationCurve::IntegrationResult& XYIntegrationCurve::integrationResult() const {
	Q_D(const XYIntegrationCurve);
	return d->integrationResult;
}

//##############################################################################
//#################  setter methods and undo commands ##########################
//##############################################################################
STD_SETTER_CMD_IMPL_F_S(XYIntegrationCurve, SetIntegrationData, XYIntegrationCurve::IntegrationData, integrationData, recalculate)
void XYIntegrationCurve::setIntegrationData(const XYIntegrationCurve::IntegrationData& integrationData) {
	Q_D(XYIntegrationCurve);
	exec(new XYIntegrationCurveSetIntegrationDataCmd(d, integrationData, ki18n("%1: set options and perform the integration")));
}

//##############################################################################
//######################### Private implementation #############################
//##############################################################################
XYIntegrationCurvePrivate::XYIntegrationCurvePrivate(XYIntegrationCurve* owner) : XYAnalysisCurvePrivate(owner), q(owner) {
}

//no need to delete xColumn and yColumn, they are deleted
//when the parent aspect is removed
XYIntegrationCurvePrivate::~XYIntegrationCurvePrivate() = default;

void XYIntegrationCurvePrivate::recalculate() {
	QElapsedTimer timer;
	timer.start();

	//create integration result columns if not available yet, clear them otherwise
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
	integrationResult = XYIntegrationCurve::IntegrationResult();

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

	//copy all valid data point for the integration to temporary vectors
	QVector<double> xdataVector;
	QVector<double> ydataVector;

	double xmin;
	double xmax;
	if (integrationData.autoRange) {
		xmin = tmpXDataColumn->minimum();
		xmax = tmpXDataColumn->maximum();
	} else {
		xmin = integrationData.xRange.first();
		xmax = integrationData.xRange.last();
	}

	XYAnalysisCurve::copyData(xdataVector, ydataVector, tmpXDataColumn, tmpYDataColumn, xmin, xmax);

	const size_t n = (size_t)xdataVector.size();	// number of data points to integrate
	if (n < 2) {
		integrationResult.available = true;
		integrationResult.valid = false;
		integrationResult.status = i18n("Not enough data points available.");
		recalcLogicalPoints();
		emit q->dataChanged();
		sourceDataChangedSinceLastRecalc = false;
		return;
	}

	double* xdata = xdataVector.data();
	double* ydata = ydataVector.data();

	// integration settings
	const nsl_int_method_type method = integrationData.method;
	const bool absolute = integrationData.absolute;

	DEBUG("method:"<<nsl_int_method_name[method]);
	DEBUG("absolute area:"<<absolute);

///////////////////////////////////////////////////////////
	int status = 0;
	size_t np = n;

	switch (method) {
	case nsl_int_method_rectangle:
		status = nsl_int_rectangle(xdata, ydata, n, absolute);
		break;
	case nsl_int_method_trapezoid:
		status = nsl_int_trapezoid(xdata, ydata, n, absolute);
		break;
	case nsl_int_method_simpson:
		np = nsl_int_simpson(xdata, ydata, n, absolute);
		break;
	case nsl_int_method_simpson_3_8:
		np = nsl_int_simpson_3_8(xdata, ydata, n, absolute);
		break;
	}

	xVector->resize((int)np);
	yVector->resize((int)np);
	memcpy(xVector->data(), xdata, np * sizeof(double));
	memcpy(yVector->data(), ydata, np * sizeof(double));
///////////////////////////////////////////////////////////

	//write the result
	integrationResult.available = true;
	integrationResult.valid = true;
	integrationResult.status = QString::number(status);
	integrationResult.elapsedTime = timer.elapsed();
	integrationResult.value = ydata[np-1];

	//redraw the curve
	recalcLogicalPoints();
	emit q->dataChanged();
	sourceDataChangedSinceLastRecalc = false;
}

//##############################################################################
//##################  Serialization/Deserialization  ###########################
//##############################################################################
//! Save as XML
void XYIntegrationCurve::save(QXmlStreamWriter* writer) const{
	Q_D(const XYIntegrationCurve);

	writer->writeStartElement("xyIntegrationCurve");

	//write the base class
	XYAnalysisCurve::save(writer);

	//write xy-integration-curve specific information
	// integration data
	writer->writeStartElement("integrationData");
	writer->writeAttribute( "autoRange", QString::number(d->integrationData.autoRange) );
	writer->writeAttribute( "xRangeMin", QString::number(d->integrationData.xRange.first()) );
	writer->writeAttribute( "xRangeMax", QString::number(d->integrationData.xRange.last()) );
	writer->writeAttribute( "method", QString::number(d->integrationData.method) );
	writer->writeAttribute( "absolute", QString::number(d->integrationData.absolute) );
	writer->writeEndElement();// integrationData

	// integration results (generated columns)
	writer->writeStartElement("integrationResult");
	writer->writeAttribute( "available", QString::number(d->integrationResult.available) );
	writer->writeAttribute( "valid", QString::number(d->integrationResult.valid) );
	writer->writeAttribute( "status", d->integrationResult.status );
	writer->writeAttribute( "time", QString::number(d->integrationResult.elapsedTime) );
	writer->writeAttribute( "value", QString::number(d->integrationResult.value) );

	//save calculated columns if available
	if (saveCalculations() && d->xColumn) {
		d->xColumn->save(writer);
		d->yColumn->save(writer);
	}
	writer->writeEndElement(); //"integrationResult"

	writer->writeEndElement(); //"xyIntegrationCurve"
}

//! Load from XML
bool XYIntegrationCurve::load(XmlStreamReader* reader, bool preview) {
	Q_D(XYIntegrationCurve);

	KLocalizedString attributeWarning = ki18n("Attribute '%1' missing or empty, default value is used");
	QXmlStreamAttributes attribs;
	QString str;

	while (!reader->atEnd()) {
		reader->readNext();
		if (reader->isEndElement() && reader->name() == "xyIntegrationCurve")
			break;

		if (!reader->isStartElement())
			continue;

		if (reader->name() == "xyAnalysisCurve") {
			if ( !XYAnalysisCurve::load(reader, preview) )
				return false;
		} else if (!preview && reader->name() == "integrationData") {
			attribs = reader->attributes();
			READ_INT_VALUE("autoRange", integrationData.autoRange, bool);
			READ_DOUBLE_VALUE("xRangeMin", integrationData.xRange.first());
			READ_DOUBLE_VALUE("xRangeMax", integrationData.xRange.last());
			READ_INT_VALUE("method", integrationData.method, nsl_int_method_type);
			READ_INT_VALUE("absolute", integrationData.absolute, bool);
		} else if (!preview && reader->name() == "integrationResult") {
			attribs = reader->attributes();
			READ_INT_VALUE("available", integrationResult.available, int);
			READ_INT_VALUE("valid", integrationResult.valid, int);
			READ_STRING_VALUE("status", integrationResult.status);
			READ_INT_VALUE("time", integrationResult.elapsedTime, int);
			READ_DOUBLE_VALUE("value", integrationResult.value);
		} else if (!preview && reader->name() == "column") {
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
