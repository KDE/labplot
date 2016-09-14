/***************************************************************************
    File                 : XYDifferentationCurve.cpp
    Project              : LabPlot
    Description          : A xy-curve defined by an differentation
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
  \class XYDifferentationCurve
  \brief A xy-curve defined by an differentation

  \ingroup worksheet
*/

#include "XYDifferentationCurve.h"
#include "XYDifferentationCurvePrivate.h"
#include "CartesianCoordinateSystem.h"
#include "backend/core/column/Column.h"
#include "backend/lib/commandtemplates.h"

#include <cmath>	// isnan
#include <cfloat>	// DBL_MIN
extern "C" {
#include <gsl/gsl_errno.h>
}

#include <KIcon>
#include <KLocale>
#include <QElapsedTimer>
#include <QThreadPool>
#ifndef NDEBUG
#include <QDebug>
#endif

XYDifferentationCurve::XYDifferentationCurve(const QString& name)
		: XYCurve(name, new XYDifferentationCurvePrivate(this)) {
	init();
}

XYDifferentationCurve::XYDifferentationCurve(const QString& name, XYDifferentationCurvePrivate* dd)
		: XYCurve(name, dd) {
	init();
}


XYDifferentationCurve::~XYDifferentationCurve() {
	//no need to delete the d-pointer here - it inherits from QGraphicsItem
	//and is deleted during the cleanup in QGraphicsScene
}

void XYDifferentationCurve::init() {
	Q_D(XYDifferentationCurve);

	//TODO: read from the saved settings for XYDifferentationCurve?
	d->lineType = XYCurve::Line;
	d->symbolsStyle = Symbol::NoSymbols;
}

void XYDifferentationCurve::recalculate() {
	Q_D(XYDifferentationCurve);
	d->recalculate();
}

/*!
	Returns an icon to be used in the project explorer.
*/
QIcon XYDifferentationCurve::icon() const {
	return KIcon("labplot-xy-differentation-curve");
}

//##############################################################################
//##########################  getter methods  ##################################
//##############################################################################
BASIC_SHARED_D_READER_IMPL(XYDifferentationCurve, const AbstractColumn*, xDataColumn, xDataColumn)
BASIC_SHARED_D_READER_IMPL(XYDifferentationCurve, const AbstractColumn*, yDataColumn, yDataColumn)
const QString& XYDifferentationCurve::xDataColumnPath() const { Q_D(const XYDifferentationCurve); return d->xDataColumnPath; }
const QString& XYDifferentationCurve::yDataColumnPath() const { Q_D(const XYDifferentationCurve); return d->yDataColumnPath; }

BASIC_SHARED_D_READER_IMPL(XYDifferentationCurve, XYDifferentationCurve::DifferentationData, differentationData, differentationData)

const XYDifferentationCurve::DifferentationResult& XYDifferentationCurve::differentationResult() const {
	Q_D(const XYDifferentationCurve);
	return d->differentationResult;
}

bool XYDifferentationCurve::isSourceDataChangedSinceLastDifferentation() const {
	Q_D(const XYDifferentationCurve);
	return d->sourceDataChangedSinceLastDifferentation;
}

//##############################################################################
//#################  setter methods and undo commands ##########################
//##############################################################################
STD_SETTER_CMD_IMPL_S(XYDifferentationCurve, SetXDataColumn, const AbstractColumn*, xDataColumn)
void XYDifferentationCurve::setXDataColumn(const AbstractColumn* column) {
	Q_D(XYDifferentationCurve);
	if (column != d->xDataColumn) {
		exec(new XYDifferentationCurveSetXDataColumnCmd(d, column, i18n("%1: assign x-data")));
		emit sourceDataChangedSinceLastDifferentation();
		if (column) {
			connect(column, SIGNAL(dataChanged(const AbstractColumn*)), this, SLOT(handleSourceDataChanged()));
			//TODO disconnect on undo
		}
	}
}

STD_SETTER_CMD_IMPL_S(XYDifferentationCurve, SetYDataColumn, const AbstractColumn*, yDataColumn)
void XYDifferentationCurve::setYDataColumn(const AbstractColumn* column) {
	Q_D(XYDifferentationCurve);
	if (column != d->yDataColumn) {
		exec(new XYDifferentationCurveSetYDataColumnCmd(d, column, i18n("%1: assign y-data")));
		emit sourceDataChangedSinceLastDifferentation();
		if (column) {
			connect(column, SIGNAL(dataChanged(const AbstractColumn*)), this, SLOT(handleSourceDataChanged()));
			//TODO disconnect on undo
		}
	}
}

STD_SETTER_CMD_IMPL_F_S(XYDifferentationCurve, SetDifferentationData, XYDifferentationCurve::DifferentationData, differentationData, recalculate);
void XYDifferentationCurve::setDifferentationData(const XYDifferentationCurve::DifferentationData& differentationData) {
	Q_D(XYDifferentationCurve);
	exec(new XYDifferentationCurveSetDifferentationDataCmd(d, differentationData, i18n("%1: set options and perform the differentation")));
}

//##############################################################################
//################################## SLOTS ####################################
//##############################################################################
void XYDifferentationCurve::handleSourceDataChanged() {
	Q_D(XYDifferentationCurve);
	d->sourceDataChangedSinceLastDifferentation = true;
	emit sourceDataChangedSinceLastDifferentation();
}
//##############################################################################
//######################### Private implementation #############################
//##############################################################################
XYDifferentationCurvePrivate::XYDifferentationCurvePrivate(XYDifferentationCurve* owner) : XYCurvePrivate(owner),
	xDataColumn(0), yDataColumn(0), 
	xColumn(0), yColumn(0), 
	xVector(0), yVector(0), 
	sourceDataChangedSinceLastDifferentation(false),
	q(owner)  {

}

XYDifferentationCurvePrivate::~XYDifferentationCurvePrivate() {
	//no need to delete xColumn and yColumn, they are deleted
	//when the parent aspect is removed
}

// ...
// see XYFitCurvePrivate

void XYDifferentationCurvePrivate::recalculate() {
	QElapsedTimer timer;
	timer.start();

	//create differentation result columns if not available yet, clear them otherwise
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
	differentationResult = XYDifferentationCurve::DifferentationResult();

	if (!xDataColumn || !yDataColumn) {
		emit (q->dataChanged());
		sourceDataChangedSinceLastDifferentation = false;
		return;
	}

	//check column sizes
	if (xDataColumn->rowCount()!=yDataColumn->rowCount()) {
		differentationResult.available = true;
		differentationResult.valid = false;
		differentationResult.status = i18n("Number of x and y data points must be equal.");
		emit (q->dataChanged());
		sourceDataChangedSinceLastDifferentation = false;
		return;
	}

	//copy all valid data point for the differentation to temporary vectors
	QVector<double> xdataVector;
	QVector<double> ydataVector;
	for (int row=0; row<xDataColumn->rowCount(); ++row) {
		//only copy those data where _all_ values (for x and y, if given) are valid
		if (!std::isnan(xDataColumn->valueAt(row)) && !std::isnan(yDataColumn->valueAt(row))
			&& !xDataColumn->isMasked(row) && !yDataColumn->isMasked(row)) {

			xdataVector.append(xDataColumn->valueAt(row));
			ydataVector.append(yDataColumn->valueAt(row));
		}
	}

	//number of data points to differentate
	const unsigned int n = ydataVector.size();
	if (n < 2) {
		differentationResult.available = true;
		differentationResult.valid = false;
		differentationResult.status = i18n("Not enough data points available.");
		emit (q->dataChanged());
		sourceDataChangedSinceLastDifferentation = false;
		return;
	}

	double* xdata = xdataVector.data();
	double* ydata = ydataVector.data();

	const double min = xDataColumn->minimum();
	const double max = xDataColumn->maximum();

	// differentation settings
	const int type = differentationData.type;
#ifndef NDEBUG
	qDebug()<<"type:"<<type;
#endif
///////////////////////////////////////////////////////////
	int status=0;

///////////////////////////////////////////////////////////

	//write the result
	differentationResult.available = true;
	differentationResult.valid = true;
	differentationResult.status = QString(gsl_strerror(status));;
	differentationResult.elapsedTime = timer.elapsed();

	//redraw the curve
	emit (q->dataChanged());
	sourceDataChangedSinceLastDifferentation = false;
}

//##############################################################################
//##################  Serialization/Deserialization  ###########################
//##############################################################################
//! Save as XML
void XYDifferentationCurve::save(QXmlStreamWriter* writer) const{
	Q_D(const XYDifferentationCurve);

	writer->writeStartElement("xyDifferentationCurve");

	//write xy-curve information
	XYCurve::save(writer);

	//write xy-differentation-curve specific information
	// differentation data
	writer->writeStartElement("differentationData");
	WRITE_COLUMN(d->xDataColumn, xDataColumn);
	WRITE_COLUMN(d->yDataColumn, yDataColumn);
	writer->writeAttribute( "type", QString::number(d->differentationData.type) );
	writer->writeEndElement();// differentationData

	// differentation results (generated columns)
	writer->writeStartElement("differentationResult");
	writer->writeAttribute( "available", QString::number(d->differentationResult.available) );
	writer->writeAttribute( "valid", QString::number(d->differentationResult.valid) );
	writer->writeAttribute( "status", d->differentationResult.status );
	writer->writeAttribute( "time", QString::number(d->differentationResult.elapsedTime) );

	//save calculated columns if available
	if (d->xColumn) {
		d->xColumn->save(writer);
		d->yColumn->save(writer);
	}
	writer->writeEndElement(); //"differentationResult"

	writer->writeEndElement(); //"xyDifferentationCurve"
}

//! Load from XML
bool XYDifferentationCurve::load(XmlStreamReader* reader) {
	Q_D(XYDifferentationCurve);

	if (!reader->isStartElement() || reader->name() != "xyDifferentationCurve") {
		reader->raiseError(i18n("no xy differentation curve element found"));
		return false;
	}

	QString attributeWarning = i18n("Attribute '%1' missing or empty, default value is used");
	QXmlStreamAttributes attribs;
	QString str;

	while (!reader->atEnd()) {
		reader->readNext();
		if (reader->isEndElement() && reader->name() == "xyDifferentationCurve")
			break;

		if (!reader->isStartElement())
			continue;

		if (reader->name() == "xyCurve") {
			if ( !XYCurve::load(reader) )
				return false;
		} else if (reader->name() == "differentationData") {
			attribs = reader->attributes();

			READ_COLUMN(xDataColumn);
			READ_COLUMN(yDataColumn);

			str = attribs.value("type").toString();
			if (str.isEmpty())
				reader->raiseWarning(attributeWarning.arg("'type'"));
			else
				d->differentationData.type = str.toInt();

		} else if (reader->name() == "differentationResult") {

			attribs = reader->attributes();

			str = attribs.value("available").toString();
			if (str.isEmpty())
				reader->raiseWarning(attributeWarning.arg("'available'"));
			else
				d->differentationResult.available = str.toInt();

			str = attribs.value("valid").toString();
			if (str.isEmpty())
				reader->raiseWarning(attributeWarning.arg("'valid'"));
			else
				d->differentationResult.valid = str.toInt();
			
			str = attribs.value("status").toString();
			if (str.isEmpty())
				reader->raiseWarning(attributeWarning.arg("'status'"));
			else
				d->differentationResult.status = str;

			str = attribs.value("time").toString();
			if (str.isEmpty())
				reader->raiseWarning(attributeWarning.arg("'time'"));
			else
				d->differentationResult.elapsedTime = str.toInt();
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
