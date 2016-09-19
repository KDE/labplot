/***************************************************************************
    File                 : XYDifferentiationCurve.cpp
    Project              : LabPlot
    Description          : A xy-curve defined by an differentiation
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
  \class XYDifferentiationCurve
  \brief A xy-curve defined by an differentiation

  \ingroup worksheet
*/

#include "XYDifferentiationCurve.h"
#include "XYDifferentiationCurvePrivate.h"
#include "CartesianCoordinateSystem.h"
#include "backend/core/column/Column.h"
#include "backend/lib/commandtemplates.h"

#include <cmath>	// isnan
#include <cfloat>	// DBL_MIN
extern "C" {
#include <gsl/gsl_errno.h>
#include "backend/nsl/nsl_diff.h"
}

#include <KIcon>
#include <KLocale>
#include <QElapsedTimer>
#include <QThreadPool>
#ifndef NDEBUG
#include <QDebug>
#endif

XYDifferentiationCurve::XYDifferentiationCurve(const QString& name)
		: XYCurve(name, new XYDifferentiationCurvePrivate(this)) {
	init();
}

XYDifferentiationCurve::XYDifferentiationCurve(const QString& name, XYDifferentiationCurvePrivate* dd)
		: XYCurve(name, dd) {
	init();
}


XYDifferentiationCurve::~XYDifferentiationCurve() {
	//no need to delete the d-pointer here - it inherits from QGraphicsItem
	//and is deleted during the cleanup in QGraphicsScene
}

void XYDifferentiationCurve::init() {
	Q_D(XYDifferentiationCurve);

	//TODO: read from the saved settings for XYDifferentiationCurve?
	d->lineType = XYCurve::Line;
	d->symbolsStyle = Symbol::NoSymbols;
}

void XYDifferentiationCurve::recalculate() {
	Q_D(XYDifferentiationCurve);
	d->recalculate();
}

/*!
	Returns an icon to be used in the project explorer.
*/
QIcon XYDifferentiationCurve::icon() const {
	return KIcon("labplot-xy-differentiation-curve");
}

//##############################################################################
//##########################  getter methods  ##################################
//##############################################################################
BASIC_SHARED_D_READER_IMPL(XYDifferentiationCurve, const AbstractColumn*, xDataColumn, xDataColumn)
BASIC_SHARED_D_READER_IMPL(XYDifferentiationCurve, const AbstractColumn*, yDataColumn, yDataColumn)
const QString& XYDifferentiationCurve::xDataColumnPath() const { Q_D(const XYDifferentiationCurve); return d->xDataColumnPath; }
const QString& XYDifferentiationCurve::yDataColumnPath() const { Q_D(const XYDifferentiationCurve); return d->yDataColumnPath; }

BASIC_SHARED_D_READER_IMPL(XYDifferentiationCurve, XYDifferentiationCurve::DifferentiationData, differentiationData, differentiationData)

const XYDifferentiationCurve::DifferentiationResult& XYDifferentiationCurve::differentiationResult() const {
	Q_D(const XYDifferentiationCurve);
	return d->differentiationResult;
}

bool XYDifferentiationCurve::isSourceDataChangedSinceLastDifferentiation() const {
	Q_D(const XYDifferentiationCurve);
	return d->sourceDataChangedSinceLastDifferentiation;
}

//##############################################################################
//#################  setter methods and undo commands ##########################
//##############################################################################
STD_SETTER_CMD_IMPL_S(XYDifferentiationCurve, SetXDataColumn, const AbstractColumn*, xDataColumn)
void XYDifferentiationCurve::setXDataColumn(const AbstractColumn* column) {
	Q_D(XYDifferentiationCurve);
	if (column != d->xDataColumn) {
		exec(new XYDifferentiationCurveSetXDataColumnCmd(d, column, i18n("%1: assign x-data")));
		emit sourceDataChangedSinceLastDifferentiation();
		if (column) {
			connect(column, SIGNAL(dataChanged(const AbstractColumn*)), this, SLOT(handleSourceDataChanged()));
			//TODO disconnect on undo
		}
	}
}

STD_SETTER_CMD_IMPL_S(XYDifferentiationCurve, SetYDataColumn, const AbstractColumn*, yDataColumn)
void XYDifferentiationCurve::setYDataColumn(const AbstractColumn* column) {
	Q_D(XYDifferentiationCurve);
	if (column != d->yDataColumn) {
		exec(new XYDifferentiationCurveSetYDataColumnCmd(d, column, i18n("%1: assign y-data")));
		emit sourceDataChangedSinceLastDifferentiation();
		if (column) {
			connect(column, SIGNAL(dataChanged(const AbstractColumn*)), this, SLOT(handleSourceDataChanged()));
			//TODO disconnect on undo
		}
	}
}

STD_SETTER_CMD_IMPL_F_S(XYDifferentiationCurve, SetDifferentiationData, XYDifferentiationCurve::DifferentiationData, differentiationData, recalculate);
void XYDifferentiationCurve::setDifferentiationData(const XYDifferentiationCurve::DifferentiationData& differentiationData) {
	Q_D(XYDifferentiationCurve);
	exec(new XYDifferentiationCurveSetDifferentiationDataCmd(d, differentiationData, i18n("%1: set options and perform the differentiation")));
}

//##############################################################################
//################################## SLOTS ####################################
//##############################################################################
void XYDifferentiationCurve::handleSourceDataChanged() {
	Q_D(XYDifferentiationCurve);
	d->sourceDataChangedSinceLastDifferentiation = true;
	emit sourceDataChangedSinceLastDifferentiation();
}
//##############################################################################
//######################### Private implementation #############################
//##############################################################################
XYDifferentiationCurvePrivate::XYDifferentiationCurvePrivate(XYDifferentiationCurve* owner) : XYCurvePrivate(owner),
	xDataColumn(0), yDataColumn(0), 
	xColumn(0), yColumn(0), 
	xVector(0), yVector(0), 
	sourceDataChangedSinceLastDifferentiation(false),
	q(owner)  {

}

XYDifferentiationCurvePrivate::~XYDifferentiationCurvePrivate() {
	//no need to delete xColumn and yColumn, they are deleted
	//when the parent aspect is removed
}

// ...
// see XYFitCurvePrivate

void XYDifferentiationCurvePrivate::recalculate() {
	QElapsedTimer timer;
	timer.start();

	//create differentiation result columns if not available yet, clear them otherwise
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
	differentiationResult = XYDifferentiationCurve::DifferentiationResult();

	if (!xDataColumn || !yDataColumn) {
		emit (q->dataChanged());
		sourceDataChangedSinceLastDifferentiation = false;
		return;
	}

	//check column sizes
	if (xDataColumn->rowCount()!=yDataColumn->rowCount()) {
		differentiationResult.available = true;
		differentiationResult.valid = false;
		differentiationResult.status = i18n("Number of x and y data points must be equal.");
		emit (q->dataChanged());
		sourceDataChangedSinceLastDifferentiation = false;
		return;
	}

	//copy all valid data point for the differentiation to temporary vectors
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

	//number of data points to differentiate
	const unsigned int n = ydataVector.size();
	if (n < 3) {
		differentiationResult.available = true;
		differentiationResult.valid = false;
		differentiationResult.status = i18n("Not enough data points available.");
		emit (q->dataChanged());
		sourceDataChangedSinceLastDifferentiation = false;
		return;
	}

	double* xdata = xdataVector.data();
	double* ydata = ydataVector.data();

	// differentiation settings
	const int order = differentiationData.order;
#ifndef NDEBUG
	qDebug()<<"order:"<<order;
#endif
///////////////////////////////////////////////////////////
	int status=0;

	switch (order) {
	case 1:
		status = nsl_diff_first_deriv_second_order(xdata, ydata, n);
		break;
	case 2:
		status = nsl_diff_second_deriv_second_order(xdata, ydata, n);
		break;
	}

	xVector->resize(n);
	yVector->resize(n);
	memcpy(xVector->data(), xdata, n*sizeof(double));
	memcpy(yVector->data(), ydata, n*sizeof(double));
///////////////////////////////////////////////////////////

	//write the result
	differentiationResult.available = true;
	differentiationResult.valid = true;
	differentiationResult.status = QString::number(status);
	differentiationResult.elapsedTime = timer.elapsed();

	//redraw the curve
	emit (q->dataChanged());
	sourceDataChangedSinceLastDifferentiation = false;
}

//##############################################################################
//##################  Serialization/Deserialization  ###########################
//##############################################################################
//! Save as XML
void XYDifferentiationCurve::save(QXmlStreamWriter* writer) const{
	Q_D(const XYDifferentiationCurve);

	writer->writeStartElement("xyDifferentiationCurve");

	//write xy-curve information
	XYCurve::save(writer);

	//write xy-differentiation-curve specific information
	// differentiation data
	writer->writeStartElement("differentiationData");
	WRITE_COLUMN(d->xDataColumn, xDataColumn);
	WRITE_COLUMN(d->yDataColumn, yDataColumn);
	writer->writeAttribute( "order", QString::number(d->differentiationData.order) );
	writer->writeEndElement();// differentiationData

	// differentiation results (generated columns)
	writer->writeStartElement("differentiationResult");
	writer->writeAttribute( "available", QString::number(d->differentiationResult.available) );
	writer->writeAttribute( "valid", QString::number(d->differentiationResult.valid) );
	writer->writeAttribute( "status", d->differentiationResult.status );
	writer->writeAttribute( "time", QString::number(d->differentiationResult.elapsedTime) );

	//save calculated columns if available
	if (d->xColumn) {
		d->xColumn->save(writer);
		d->yColumn->save(writer);
	}
	writer->writeEndElement(); //"differentiationResult"

	writer->writeEndElement(); //"xyDifferentiationCurve"
}

//! Load from XML
bool XYDifferentiationCurve::load(XmlStreamReader* reader) {
	Q_D(XYDifferentiationCurve);

	if (!reader->isStartElement() || reader->name() != "xyDifferentiationCurve") {
		reader->raiseError(i18n("no xy differentiation curve element found"));
		return false;
	}

	QString attributeWarning = i18n("Attribute '%1' missing or empty, default value is used");
	QXmlStreamAttributes attribs;
	QString str;

	while (!reader->atEnd()) {
		reader->readNext();
		if (reader->isEndElement() && reader->name() == "xyDifferentiationCurve")
			break;

		if (!reader->isStartElement())
			continue;

		if (reader->name() == "xyCurve") {
			if ( !XYCurve::load(reader) )
				return false;
		} else if (reader->name() == "differentiationData") {
			attribs = reader->attributes();

			READ_COLUMN(xDataColumn);
			READ_COLUMN(yDataColumn);

			str = attribs.value("order").toString();
			if (str.isEmpty())
				reader->raiseWarning(attributeWarning.arg("'order'"));
			else
				d->differentiationData.order = str.toInt();

		} else if (reader->name() == "differentiationResult") {

			attribs = reader->attributes();

			str = attribs.value("available").toString();
			if (str.isEmpty())
				reader->raiseWarning(attributeWarning.arg("'available'"));
			else
				d->differentiationResult.available = str.toInt();

			str = attribs.value("valid").toString();
			if (str.isEmpty())
				reader->raiseWarning(attributeWarning.arg("'valid'"));
			else
				d->differentiationResult.valid = str.toInt();
			
			str = attribs.value("status").toString();
			if (str.isEmpty())
				reader->raiseWarning(attributeWarning.arg("'status'"));
			else
				d->differentiationResult.status = str;

			str = attribs.value("time").toString();
			if (str.isEmpty())
				reader->raiseWarning(attributeWarning.arg("'time'"));
			else
				d->differentiationResult.elapsedTime = str.toInt();
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
