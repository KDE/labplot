/***************************************************************************
    File                 : XYInterpolationCurve.cpp
    Project              : LabPlot
    Description          : A xy-curve defined by an interpolation
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
  \class XYInterpolationCurve
  \brief A xy-curve defined by an interpolation

  \ingroup worksheet
*/

#include "XYInterpolationCurve.h"
#include "XYInterpolationCurvePrivate.h"
#include "backend/core/AbstractColumn.h"
#include "backend/core/column/Column.h"
#include "backend/lib/commandtemplates.h"
#include <cmath>	// isnan
/*#include "backend/gsl/ExpressionParser.h"
#include "backend/gsl/parser_extern.h"
*/
//#include <gsl/gsl_version.h>
#include <gsl/gsl_fft_real.h>
#include <gsl/gsl_fft_halfcomplex.h>
#include <gsl/gsl_sf_pow_int.h>

#include <KIcon>
#include <KLocale>
#include <QElapsedTimer>
#include <QDebug>

XYInterpolationCurve::XYInterpolationCurve(const QString& name)
		: XYCurve(name, new XYInterpolationCurvePrivate(this)) {
	init();
}

XYInterpolationCurve::XYInterpolationCurve(const QString& name, XYInterpolationCurvePrivate* dd)
		: XYCurve(name, dd) {
	init();
}


XYInterpolationCurve::~XYInterpolationCurve() {
	//no need to delete the d-pointer here - it inherits from QGraphicsItem
	//and is deleted during the cleanup in QGraphicsScene
}

void XYInterpolationCurve::init() {
	Q_D(XYInterpolationCurve);

	//TODO: read from the saved settings for XYInterpolationCurve?
	d->lineType = XYCurve::Line;
	d->symbolsStyle = Symbol::NoSymbols;
}

void XYInterpolationCurve::recalculate() {
	Q_D(XYInterpolationCurve);
	d->recalculate();
}

/*!
	Returns an icon to be used in the project explorer.
*/
QIcon XYInterpolationCurve::icon() const {
	return KIcon("labplot-xy-interpolation-curve");
}

//##############################################################################
//##########################  getter methods  ##################################
//##############################################################################
BASIC_SHARED_D_READER_IMPL(XYInterpolationCurve, const AbstractColumn*, xDataColumn, xDataColumn)
BASIC_SHARED_D_READER_IMPL(XYInterpolationCurve, const AbstractColumn*, yDataColumn, yDataColumn)
const QString& XYInterpolationCurve::xDataColumnPath() const { Q_D(const XYInterpolationCurve); return d->xDataColumnPath; }
const QString& XYInterpolationCurve::yDataColumnPath() const { Q_D(const XYInterpolationCurve); return d->yDataColumnPath; }

BASIC_SHARED_D_READER_IMPL(XYInterpolationCurve, XYInterpolationCurve::InterpolationData, interpolationData, interpolationData)

const XYInterpolationCurve::InterpolationResult& XYInterpolationCurve::interpolationResult() const {
	Q_D(const XYInterpolationCurve);
	return d->interpolationResult;
}

bool XYInterpolationCurve::isSourceDataChangedSinceLastInterpolation() const {
	Q_D(const XYInterpolationCurve);
	return d->sourceDataChangedSinceLastInterpolation;
}

//##############################################################################
//#################  setter methods and undo commands ##########################
//##############################################################################
STD_SETTER_CMD_IMPL_S(XYInterpolationCurve, SetXDataColumn, const AbstractColumn*, xDataColumn)
void XYInterpolationCurve::setXDataColumn(const AbstractColumn* column) {
	Q_D(XYInterpolationCurve);
	if (column != d->xDataColumn) {
		exec(new XYInterpolationCurveSetXDataColumnCmd(d, column, i18n("%1: assign x-data")));
		emit sourceDataChangedSinceLastInterpolation();
		if (column) {
			connect(column, SIGNAL(dataChanged(const AbstractColumn*)), this, SLOT(handleSourceDataChanged()));
			//TODO disconnect on undo
		}
	}
}

STD_SETTER_CMD_IMPL_S(XYInterpolationCurve, SetYDataColumn, const AbstractColumn*, yDataColumn)
void XYInterpolationCurve::setYDataColumn(const AbstractColumn* column) {
	Q_D(XYInterpolationCurve);
	if (column != d->yDataColumn) {
		exec(new XYInterpolationCurveSetYDataColumnCmd(d, column, i18n("%1: assign y-data")));
		emit sourceDataChangedSinceLastInterpolation();
		if (column) {
			connect(column, SIGNAL(dataChanged(const AbstractColumn*)), this, SLOT(handleSourceDataChanged()));
			//TODO disconnect on undo
		}
	}
}

STD_SETTER_CMD_IMPL_F_S(XYInterpolationCurve, SetInterpolationData, XYInterpolationCurve::InterpolationData, interpolationData, recalculate);
void XYInterpolationCurve::setInterpolationData(const XYInterpolationCurve::InterpolationData& interpolationData) {
	Q_D(XYInterpolationCurve);
	exec(new XYInterpolationCurveSetInterpolationDataCmd(d, interpolationData, i18n("%1: set options and perform the interpolation")));
}

//##############################################################################
//################################## SLOTS ####################################
//##############################################################################
void XYInterpolationCurve::handleSourceDataChanged() {
	Q_D(XYInterpolationCurve);
	d->sourceDataChangedSinceLastInterpolation = true;
	emit sourceDataChangedSinceLastInterpolation();
}
//##############################################################################
//######################### Private implementation #############################
//##############################################################################
XYInterpolationCurvePrivate::XYInterpolationCurvePrivate(XYInterpolationCurve* owner) : XYCurvePrivate(owner),
	xDataColumn(0), yDataColumn(0), 
	xColumn(0), yColumn(0), 
	xVector(0), yVector(0), 
	sourceDataChangedSinceLastInterpolation(false),
	q(owner)  {

}

XYInterpolationCurvePrivate::~XYInterpolationCurvePrivate() {
	//no need to delete xColumn and yColumn, they are deleted
	//when the parent aspect is removed
}

// ...
// see XYFitCurvePrivate

void XYInterpolationCurvePrivate::recalculate() {
	QElapsedTimer timer;
	timer.start();

	//create interpolation result columns if not available yet, clear them otherwise
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
	interpolationResult = XYInterpolationCurve::InterpolationResult();

	if (!xDataColumn || !yDataColumn) {
		emit (q->dataChanged());
		sourceDataChangedSinceLastInterpolation = false;
		return;
	}

	//check column sizes
	if (xDataColumn->rowCount()!=yDataColumn->rowCount()) {
		interpolationResult.available = true;
		interpolationResult.valid = false;
		interpolationResult.status = i18n("Number of x and y data points must be equal.");
		emit (q->dataChanged());
		sourceDataChangedSinceLastInterpolation = false;
		return;
	}

	//copy all valid data point for the fit to temporary vectors
	QVector<double> xdataVector;
	QVector<double> ydataVector;
	for (int row=0; row<xDataColumn->rowCount(); ++row) {
		//only copy those data where _all_ values (for x and y, if given) are valid
		if (!isnan(xDataColumn->valueAt(row)) && !isnan(yDataColumn->valueAt(row))
			&& !xDataColumn->isMasked(row) && !yDataColumn->isMasked(row)) {

			xdataVector.append(xDataColumn->valueAt(row));
			ydataVector.append(yDataColumn->valueAt(row));
		}
	}

	//number of data points to fit
	unsigned int n = ydataVector.size();
	if (n == 0) {
		interpolationResult.available = true;
		interpolationResult.valid = false;
		interpolationResult.status = i18n("No data points available.");
		emit (q->dataChanged());
		sourceDataChangedSinceLastInterpolation = false;
		return;
	}

	//double* xdata = xdataVector.data();
	double* ydata = ydataVector.data();

        double min = xDataColumn->minimum();
        double max = xDataColumn->maximum();

	// interpolation settings
//	unsigned int order = interpolationData.order;
#ifdef QT_DEBUG
	//TODO
#endif
///////////////////////////////////////////////////////////
//TODO
///////////////////////////////////////////////////////////

	//write the result
	interpolationResult.available = true;
	interpolationResult.valid = true;
	interpolationResult.status = i18n("OK");
	interpolationResult.elapsedTime = timer.elapsed();

	//redraw the curve
	emit (q->dataChanged());
	sourceDataChangedSinceLastInterpolation = false;
}

//##############################################################################
//##################  Serialization/Deserialization  ###########################
//##############################################################################
//! Save as XML
void XYInterpolationCurve::save(QXmlStreamWriter* writer) const{
	Q_D(const XYInterpolationCurve);

	writer->writeStartElement("xyInterpolationCurve");

	//write xy-curve information
	XYCurve::save(writer);

	//write xy-interpolation-curve specific information
	// interpolation data
	writer->writeStartElement("interpolationData");
	WRITE_COLUMN(d->xDataColumn, xDataColumn);
	WRITE_COLUMN(d->yDataColumn, yDataColumn);
	//writer->writeAttribute( "order", QString::number(d->interpolationData.order) );
	//writer->writeAttribute( "cutoff", QString::number(d->interpolationData.cutoff) );
	writer->writeEndElement();// interpolationData

	// interpolation results (generated columns)
	writer->writeStartElement("interpolationResult");
	writer->writeAttribute( "available", QString::number(d->interpolationResult.available) );
	writer->writeAttribute( "valid", QString::number(d->interpolationResult.valid) );
	writer->writeAttribute( "status", d->interpolationResult.status );
	writer->writeAttribute( "time", QString::number(d->interpolationResult.elapsedTime) );

	//save calculated columns if available
	if (d->xColumn) {
		d->xColumn->save(writer);
		d->yColumn->save(writer);
	}
	writer->writeEndElement(); //"interpolationResult"

	writer->writeEndElement(); //"xyInterpolationCurve"
}

//! Load from XML
bool XYInterpolationCurve::load(XmlStreamReader* reader){
	Q_D(XYInterpolationCurve);

	if(!reader->isStartElement() || reader->name() != "xyInterpolationCurve"){
		reader->raiseError(i18n("no xy Fourier interpolation curve element found"));
		return false;
	}

	QString attributeWarning = i18n("Attribute '%1' missing or empty, default value is used");
	QXmlStreamAttributes attribs;
	QString str;

	while (!reader->atEnd()) {
		reader->readNext();
		if (reader->isEndElement() && reader->name() == "xyInterpolationCurve")
			break;

		if (!reader->isStartElement())
			continue;

		if (reader->name() == "xyCurve") {
			if ( !XYCurve::load(reader) )
				return false;
		} else if (reader->name() == "interpolationData") {
			attribs = reader->attributes();

			READ_COLUMN(xDataColumn);
			READ_COLUMN(yDataColumn);

/*			str = attribs.value("order").toString();
			if(str.isEmpty())
				reader->raiseWarning(attributeWarning.arg("'order'"));
			else
				d->interpolationData.order = str.toInt();
*/
		} else if (reader->name() == "interpolationResult") {

			attribs = reader->attributes();

			str = attribs.value("available").toString();
			if(str.isEmpty())
				reader->raiseWarning(attributeWarning.arg("'available'"));
			else
				d->interpolationResult.available = str.toInt();

			str = attribs.value("valid").toString();
			if(str.isEmpty())
				reader->raiseWarning(attributeWarning.arg("'valid'"));
			else
				d->interpolationResult.valid = str.toInt();
			
			str = attribs.value("status").toString();
			if(str.isEmpty())
				reader->raiseWarning(attributeWarning.arg("'status'"));
			else
				d->interpolationResult.status = str;

			str = attribs.value("time").toString();
			if(str.isEmpty())
				reader->raiseWarning(attributeWarning.arg("'time'"));
			else
				d->interpolationResult.elapsedTime = str.toInt();
		} else if(reader->name() == "column") {
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

	if (d->xColumn) {
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
