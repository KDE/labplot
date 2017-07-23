/***************************************************************************
    File                 : XYIntegrationCurve.cpp
    Project              : LabPlot
    Description          : A xy-curve defined by an integration
    --------------------------------------------------------------------
    Copyright            : (C) 2016 Stefan Gerlach (stefan.gerlach@uni.kn)

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *  This program is free software; you can redistribute it and/or modify   *
 *  it under the terms of the GNU General Public License as published by   *
 *  the Free Software Foundation; either version 2 of the License, or      *
 *  (at your option) any later version.                                    *
 *                                                                         *
 *  This program is distributed in the hope that it will be useful,        *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 *  GNU General Public License for more details.                           *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the Free Software           *
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor,                    *
 *   Boston, MA  02110-1301  USA                                           *
 *                                                                         *
 ***************************************************************************/

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

#include <cmath>	// isnan
#include <cfloat>	// DBL_MIN
extern "C" {
#include <gsl/gsl_errno.h>
}

#include <KLocale>
#include <QIcon>
#include <QElapsedTimer>
#include <QThreadPool>

XYIntegrationCurve::XYIntegrationCurve(const QString& name)
		: XYCurve(name, new XYIntegrationCurvePrivate(this)) {
	init();
}

XYIntegrationCurve::XYIntegrationCurve(const QString& name, XYIntegrationCurvePrivate* dd)
		: XYCurve(name, dd) {
	init();
}


XYIntegrationCurve::~XYIntegrationCurve() {
	//no need to delete the d-pointer here - it inherits from QGraphicsItem
	//and is deleted during the cleanup in QGraphicsScene
}

void XYIntegrationCurve::init() {
	Q_D(XYIntegrationCurve);

	//TODO: read from the saved settings for XYIntegrationCurve?
	d->lineType = XYCurve::Line;
	d->symbolsStyle = Symbol::NoSymbols;
}

void XYIntegrationCurve::recalculate() {
	Q_D(XYIntegrationCurve);
	d->recalculate();
}

/*!
	Returns an icon to be used in the project explorer.
*/
QIcon XYIntegrationCurve::icon() const {
	return QIcon::fromTheme("labplot-xy-integration-curve");
}

//##############################################################################
//##########################  getter methods  ##################################
//##############################################################################
BASIC_SHARED_D_READER_IMPL(XYIntegrationCurve, const AbstractColumn*, xDataColumn, xDataColumn)
BASIC_SHARED_D_READER_IMPL(XYIntegrationCurve, const AbstractColumn*, yDataColumn, yDataColumn)
const QString& XYIntegrationCurve::xDataColumnPath() const { Q_D(const XYIntegrationCurve); return d->xDataColumnPath; }
const QString& XYIntegrationCurve::yDataColumnPath() const { Q_D(const XYIntegrationCurve); return d->yDataColumnPath; }

BASIC_SHARED_D_READER_IMPL(XYIntegrationCurve, XYIntegrationCurve::IntegrationData, integrationData, integrationData)

const XYIntegrationCurve::IntegrationResult& XYIntegrationCurve::integrationResult() const {
	Q_D(const XYIntegrationCurve);
	return d->integrationResult;
}

//##############################################################################
//#################  setter methods and undo commands ##########################
//##############################################################################
STD_SETTER_CMD_IMPL_S(XYIntegrationCurve, SetXDataColumn, const AbstractColumn*, xDataColumn)
void XYIntegrationCurve::setXDataColumn(const AbstractColumn* column) {
	Q_D(XYIntegrationCurve);
	if (column != d->xDataColumn) {
		exec(new XYIntegrationCurveSetXDataColumnCmd(d, column, i18n("%1: assign x-data")));
		handleSourceDataChanged();
		if (column) {
			connect(column, SIGNAL(dataChanged(const AbstractColumn*)), this, SLOT(handleSourceDataChanged()));
			//TODO disconnect on undo
		}
	}
}

STD_SETTER_CMD_IMPL_S(XYIntegrationCurve, SetYDataColumn, const AbstractColumn*, yDataColumn)
void XYIntegrationCurve::setYDataColumn(const AbstractColumn* column) {
	Q_D(XYIntegrationCurve);
	if (column != d->yDataColumn) {
		exec(new XYIntegrationCurveSetYDataColumnCmd(d, column, i18n("%1: assign y-data")));
		handleSourceDataChanged();
		if (column) {
			connect(column, SIGNAL(dataChanged(const AbstractColumn*)), this, SLOT(handleSourceDataChanged()));
			//TODO disconnect on undo
		}
	}
}

STD_SETTER_CMD_IMPL_F_S(XYIntegrationCurve, SetIntegrationData, XYIntegrationCurve::IntegrationData, integrationData, recalculate);
void XYIntegrationCurve::setIntegrationData(const XYIntegrationCurve::IntegrationData& integrationData) {
	Q_D(XYIntegrationCurve);
	exec(new XYIntegrationCurveSetIntegrationDataCmd(d, integrationData, i18n("%1: set options and perform the integration")));
}

//##############################################################################
//######################### Private implementation #############################
//##############################################################################
XYIntegrationCurvePrivate::XYIntegrationCurvePrivate(XYIntegrationCurve* owner) : XYCurvePrivate(owner),
	xDataColumn(0), yDataColumn(0), 
	xColumn(0), yColumn(0), 
	xVector(0), yVector(0), 
	q(owner)  {

}

XYIntegrationCurvePrivate::~XYIntegrationCurvePrivate() {
	//no need to delete xColumn and yColumn, they are deleted
	//when the parent aspect is removed
}

// ...
// see XYFitCurvePrivate

void XYIntegrationCurvePrivate::recalculate() {
	QElapsedTimer timer;
	timer.start();

	//create integration result columns if not available yet, clear them otherwise
	if (!xColumn) {
		xColumn = new Column("x", AbstractColumn::Numeric);
		yColumn = new Column("y", AbstractColumn::Numeric);
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
	const AbstractColumn* tmpXDataColumn = 0;
	const AbstractColumn* tmpYDataColumn = 0;
	if (dataSourceType == XYCurve::DataSourceSpreadsheet) {
		//spreadsheet columns as data source
		tmpXDataColumn = xDataColumn;
		tmpYDataColumn = yDataColumn;
	} else {
		//curve columns as data source
		tmpXDataColumn = dataSourceCurve->xColumn();
		tmpYDataColumn = dataSourceCurve->yColumn();
	}

	if (!tmpXDataColumn || !tmpYDataColumn) {
		emit (q->dataChanged());
		sourceDataChangedSinceLastRecalc = false;
		return;
	}

	//check column sizes
	if (tmpXDataColumn->rowCount() != tmpYDataColumn->rowCount()) {
		integrationResult.available = true;
		integrationResult.valid = false;
		integrationResult.status = i18n("Number of x and y data points must be equal.");
		emit (q->dataChanged());
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

	for (int row=0; row<tmpXDataColumn->rowCount(); ++row) {
		//only copy those data where _all_ values (for x and y, if given) are valid
		if (!std::isnan(tmpXDataColumn->valueAt(row)) && !std::isnan(tmpYDataColumn->valueAt(row))
			&& !tmpXDataColumn->isMasked(row) && !tmpYDataColumn->isMasked(row)) {

			// only when inside given range
			if (tmpXDataColumn->valueAt(row) >= xmin && tmpXDataColumn->valueAt(row) <= xmax) {
				xdataVector.append(tmpXDataColumn->valueAt(row));
				ydataVector.append(tmpYDataColumn->valueAt(row));
			}
		}
	}

	const size_t n = xdataVector.size();	// number of data points to integrate
	if (n < 2) {
		integrationResult.available = true;
		integrationResult.valid = false;
		integrationResult.status = i18n("Not enough data points available.");
		emit (q->dataChanged());
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
	int status=0;
	size_t np=n;

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

	xVector->resize(np);
	yVector->resize(np);
	memcpy(xVector->data(), xdata, np*sizeof(double));
	memcpy(yVector->data(), ydata, np*sizeof(double));
///////////////////////////////////////////////////////////

	//write the result
	integrationResult.available = true;
	integrationResult.valid = true;
	integrationResult.status = QString::number(status);
	integrationResult.elapsedTime = timer.elapsed();
	integrationResult.value = ydata[np-1];

	//redraw the curve
	emit (q->dataChanged());
	sourceDataChangedSinceLastRecalc = false;
}

//##############################################################################
//##################  Serialization/Deserialization  ###########################
//##############################################################################
//! Save as XML
void XYIntegrationCurve::save(QXmlStreamWriter* writer) const{
	Q_D(const XYIntegrationCurve);

	writer->writeStartElement("xyIntegrationCurve");

	//write xy-curve information
	XYCurve::save(writer);

	//write xy-integration-curve specific information
	// integration data
	writer->writeStartElement("integrationData");
	WRITE_COLUMN(d->xDataColumn, xDataColumn);
	WRITE_COLUMN(d->yDataColumn, yDataColumn);
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
	if (d->xColumn) {
		d->xColumn->save(writer);
		d->yColumn->save(writer);
	}
	writer->writeEndElement(); //"integrationResult"

	writer->writeEndElement(); //"xyIntegrationCurve"
}

//! Load from XML
bool XYIntegrationCurve::load(XmlStreamReader* reader) {
	Q_D(XYIntegrationCurve);

	if (!reader->isStartElement() || reader->name() != "xyIntegrationCurve") {
		reader->raiseError(i18n("no xy integration curve element found"));
		return false;
	}

	QString attributeWarning = i18n("Attribute '%1' missing or empty, default value is used");
	QXmlStreamAttributes attribs;
	QString str;

	while (!reader->atEnd()) {
		reader->readNext();
		if (reader->isEndElement() && reader->name() == "xyIntegrationCurve")
			break;

		if (!reader->isStartElement())
			continue;

		if (reader->name() == "xyCurve") {
			if ( !XYCurve::load(reader) )
				return false;
		} else if (reader->name() == "integrationData") {
			attribs = reader->attributes();

			READ_COLUMN(xDataColumn);
			READ_COLUMN(yDataColumn);
			READ_INT_VALUE("autoRange", integrationData.autoRange, bool);
			READ_DOUBLE_VALUE("xRangeMin", integrationData.xRange.first());
			READ_DOUBLE_VALUE("xRangeMax", integrationData.xRange.last());

			READ_INT_VALUE("method", integrationData.method, nsl_int_method_type);
			READ_INT_VALUE("absolute", integrationData.absolute, bool);
		} else if (reader->name() == "integrationResult") {

			attribs = reader->attributes();

			READ_INT_VALUE("available", integrationResult.available, int);
			READ_INT_VALUE("valid", integrationResult.valid, int);
			READ_STRING_VALUE("status", integrationResult.status);
			READ_INT_VALUE("time", integrationResult.elapsedTime, int);
			READ_DOUBLE_VALUE("value", integrationResult.value);
		} else if (reader->name() == "column") {
			Column* column = new Column("", AbstractColumn::Numeric);
			if (!column->load(reader)) {
				delete column;
				return false;
			}
			if (column->name()=="x")
				d->xColumn = column;
			else if (column->name()=="y")
				d->yColumn = column;
		}
	}

	// wait for data to be read before using the pointers
	QThreadPool::globalInstance()->waitForDone();

	if (d->xColumn && d->yColumn) {
		d->xColumn->setHidden(true);
		addChild(d->xColumn);

		d->yColumn->setHidden(true);
		addChild(d->yColumn);

		d->xVector = static_cast<QVector<double>* >(d->xColumn->data());
		d->yVector = static_cast<QVector<double>* >(d->yColumn->data());

		setUndoAware(false);
		XYCurve::d_ptr->xColumn = d->xColumn;
		XYCurve::d_ptr->yColumn = d->yColumn;
		setUndoAware(true);
	}

	return true;
}
