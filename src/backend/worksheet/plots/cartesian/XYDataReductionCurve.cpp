/***************************************************************************
    File                 : XYDataReductionCurve.cpp
    Project              : LabPlot
    Description          : A xy-curve defined by a data reduction
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
  \class XYDataReductionCurve
  \brief A xy-curve defined by a data reduction

  \ingroup worksheet
*/

#include "XYDataReductionCurve.h"
#include "XYDataReductionCurvePrivate.h"
#include "CartesianCoordinateSystem.h"
#include "backend/core/column/Column.h"
#include "backend/lib/commandtemplates.h"

#include <cmath>	// isnan

#include <KIcon>
#include <KLocale>
#include <QElapsedTimer>
#include <QThreadPool>
#ifndef NDEBUG
#include <QDebug>
#endif

XYDataReductionCurve::XYDataReductionCurve(const QString& name)
		: XYCurve(name, new XYDataReductionCurvePrivate(this)) {
	init();
}

XYDataReductionCurve::XYDataReductionCurve(const QString& name, XYDataReductionCurvePrivate* dd)
		: XYCurve(name, dd) {
	init();
}


XYDataReductionCurve::~XYDataReductionCurve() {
	//no need to delete the d-pointer here - it inherits from QGraphicsItem
	//and is deleted during the cleanup in QGraphicsScene
}

void XYDataReductionCurve::init() {
	Q_D(XYDataReductionCurve);

	//TODO: read from the saved settings for XYDataReductionCurve?
	d->lineType = XYCurve::Line;
	d->symbolsStyle = Symbol::NoSymbols;
}

void XYDataReductionCurve::recalculate() {
	Q_D(XYDataReductionCurve);
	d->recalculate();
}

/*!
	Returns an icon to be used in the project explorer.
*/
QIcon XYDataReductionCurve::icon() const {
	return KIcon("labplot-xy-data-reduction-curve");
}

//##############################################################################
//##########################  getter methods  ##################################
//##############################################################################
BASIC_SHARED_D_READER_IMPL(XYDataReductionCurve, const AbstractColumn*, xDataColumn, xDataColumn)
BASIC_SHARED_D_READER_IMPL(XYDataReductionCurve, const AbstractColumn*, yDataColumn, yDataColumn)
const QString& XYDataReductionCurve::xDataColumnPath() const { Q_D(const XYDataReductionCurve); return d->xDataColumnPath; }
const QString& XYDataReductionCurve::yDataColumnPath() const { Q_D(const XYDataReductionCurve); return d->yDataColumnPath; }

BASIC_SHARED_D_READER_IMPL(XYDataReductionCurve, XYDataReductionCurve::DataReductionData, dataReductionData, dataReductionData)

const XYDataReductionCurve::DataReductionResult& XYDataReductionCurve::dataReductionResult() const {
	Q_D(const XYDataReductionCurve);
	return d->dataReductionResult;
}

bool XYDataReductionCurve::isSourceDataChangedSinceLastDataReduction() const {
	Q_D(const XYDataReductionCurve);
	return d->sourceDataChangedSinceLastDataReduction;
}

//##############################################################################
//#################  setter methods and undo commands ##########################
//##############################################################################
STD_SETTER_CMD_IMPL_S(XYDataReductionCurve, SetXDataColumn, const AbstractColumn*, xDataColumn)
void XYDataReductionCurve::setXDataColumn(const AbstractColumn* column) {
	Q_D(XYDataReductionCurve);
	if (column != d->xDataColumn) {
		exec(new XYDataReductionCurveSetXDataColumnCmd(d, column, i18n("%1: assign x-data")));
		emit sourceDataChangedSinceLastDataReduction();
		if (column) {
			connect(column, SIGNAL(dataChanged(const AbstractColumn*)), this, SLOT(handleSourceDataChanged()));
			//TODO disconnect on undo
		}
	}
}

STD_SETTER_CMD_IMPL_S(XYDataReductionCurve, SetYDataColumn, const AbstractColumn*, yDataColumn)
void XYDataReductionCurve::setYDataColumn(const AbstractColumn* column) {
	Q_D(XYDataReductionCurve);
	if (column != d->yDataColumn) {
		exec(new XYDataReductionCurveSetYDataColumnCmd(d, column, i18n("%1: assign y-data")));
		emit sourceDataChangedSinceLastDataReduction();
		if (column) {
			connect(column, SIGNAL(dataChanged(const AbstractColumn*)), this, SLOT(handleSourceDataChanged()));
			//TODO disconnect on undo
		}
	}
}

STD_SETTER_CMD_IMPL_F_S(XYDataReductionCurve, SetDataReductionData, XYDataReductionCurve::DataReductionData, dataReductionData, recalculate);
void XYDataReductionCurve::setDataReductionData(const XYDataReductionCurve::DataReductionData& reductionData) {
	Q_D(XYDataReductionCurve);
	exec(new XYDataReductionCurveSetDataReductionDataCmd(d, reductionData, i18n("%1: set options and perform the data reduction")));
}

//##############################################################################
//################################## SLOTS ####################################
//##############################################################################
void XYDataReductionCurve::handleSourceDataChanged() {
	Q_D(XYDataReductionCurve);
	d->sourceDataChangedSinceLastDataReduction = true;
	emit sourceDataChangedSinceLastDataReduction();
}
//##############################################################################
//######################### Private implementation #############################
//##############################################################################
XYDataReductionCurvePrivate::XYDataReductionCurvePrivate(XYDataReductionCurve* owner) : XYCurvePrivate(owner),
	xDataColumn(0), yDataColumn(0), 
	xColumn(0), yColumn(0), 
	xVector(0), yVector(0), 
	sourceDataChangedSinceLastDataReduction(false),
	q(owner)  {

}

XYDataReductionCurvePrivate::~XYDataReductionCurvePrivate() {
	//no need to delete xColumn and yColumn, they are deleted
	//when the parent aspect is removed
}

// ...
// see XYFitCurvePrivate

void XYDataReductionCurvePrivate::recalculate() {
	QElapsedTimer timer;
	timer.start();

	//create dataReduction result columns if not available yet, clear them otherwise
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
	dataReductionResult = XYDataReductionCurve::DataReductionResult();

	if (!xDataColumn || !yDataColumn) {
		emit (q->dataChanged());
		sourceDataChangedSinceLastDataReduction = false;
		return;
	}

	//check column sizes
	if (xDataColumn->rowCount()!=yDataColumn->rowCount()) {
		dataReductionResult.available = true;
		dataReductionResult.valid = false;
		dataReductionResult.status = i18n("Number of x and y data points must be equal.");
		emit (q->dataChanged());
		sourceDataChangedSinceLastDataReduction = false;
		return;
	}

	//copy all valid data point for the dataReduction to temporary vectors
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

	//number of data points to interpolate
	const unsigned int n = ydataVector.size();
	if (n < 2) {
		dataReductionResult.available = true;
		dataReductionResult.valid = false;
		dataReductionResult.status = i18n("Not enough data points available.");
		emit (q->dataChanged());
		sourceDataChangedSinceLastDataReduction = false;
		return;
	}

	//double* xdata = xdataVector.data();
	//double* ydata = ydataVector.data();

	//const double min = xDataColumn->minimum();
	//const double max = xDataColumn->maximum();

	// dataReduction settings
	//const nsl_interp_type type = dataReductionData.type;
#ifndef NDEBUG
	//qDebug()<<"type:"<<nsl_interp_type_name[type];
#endif
///////////////////////////////////////////////////////////
	int status=0;

///////////////////////////////////////////////////////////

	//write the result
	dataReductionResult.available = true;
	dataReductionResult.valid = true;
	//dataReductionResult.status = QString(gsl_strerror(status));;
	dataReductionResult.elapsedTime = timer.elapsed();

	//redraw the curve
	emit (q->dataChanged());
	sourceDataChangedSinceLastDataReduction = false;
}

//##############################################################################
//##################  Serialization/Deserialization  ###########################
//##############################################################################
//! Save as XML
void XYDataReductionCurve::save(QXmlStreamWriter* writer) const{
	Q_D(const XYDataReductionCurve);

	writer->writeStartElement("xyDataReductionCurve");

	//write xy-curve information
	XYCurve::save(writer);

	//write xy-dataReduction-curve specific information
	// dataReduction data
	writer->writeStartElement("dataReductionData");
	WRITE_COLUMN(d->xDataColumn, xDataColumn);
	WRITE_COLUMN(d->yDataColumn, yDataColumn);
	//writer->writeAttribute( "type", QString::number(d->dataReductionData.type) );
	writer->writeEndElement();// dataReductionData

	// dataReduction results (generated columns)
	writer->writeStartElement("dataReductionResult");
	writer->writeAttribute( "available", QString::number(d->dataReductionResult.available) );
	writer->writeAttribute( "valid", QString::number(d->dataReductionResult.valid) );
	writer->writeAttribute( "status", d->dataReductionResult.status );
	writer->writeAttribute( "time", QString::number(d->dataReductionResult.elapsedTime) );

	//save calculated columns if available
	if (d->xColumn) {
		d->xColumn->save(writer);
		d->yColumn->save(writer);
	}
	writer->writeEndElement(); //"dataReductionResult"

	writer->writeEndElement(); //"xyDataReductionCurve"
}

//! Load from XML
bool XYDataReductionCurve::load(XmlStreamReader* reader) {
	Q_D(XYDataReductionCurve);

	if (!reader->isStartElement() || reader->name() != "xyDataReductionCurve") {
		reader->raiseError(i18n("no xy dataReduction curve element found"));
		return false;
	}

	QString attributeWarning = i18n("Attribute '%1' missing or empty, default value is used");
	QXmlStreamAttributes attribs;
	QString str;

	while (!reader->atEnd()) {
		reader->readNext();
		if (reader->isEndElement() && reader->name() == "xyDataReductionCurve")
			break;

		if (!reader->isStartElement())
			continue;

		if (reader->name() == "xyCurve") {
			if ( !XYCurve::load(reader) )
				return false;
		} else if (reader->name() == "dataReductionData") {
			attribs = reader->attributes();

			READ_COLUMN(xDataColumn);
			READ_COLUMN(yDataColumn);

		} else if (reader->name() == "dataReductionResult") {

			attribs = reader->attributes();

			str = attribs.value("available").toString();
			if (str.isEmpty())
				reader->raiseWarning(attributeWarning.arg("'available'"));
			else
				d->dataReductionResult.available = str.toInt();

			str = attribs.value("valid").toString();
			if (str.isEmpty())
				reader->raiseWarning(attributeWarning.arg("'valid'"));
			else
				d->dataReductionResult.valid = str.toInt();
			
			str = attribs.value("status").toString();
			if (str.isEmpty())
				reader->raiseWarning(attributeWarning.arg("'status'"));
			else
				d->dataReductionResult.status = str;

			str = attribs.value("time").toString();
			if (str.isEmpty())
				reader->raiseWarning(attributeWarning.arg("'time'"));
			else
				d->dataReductionResult.elapsedTime = str.toInt();
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
