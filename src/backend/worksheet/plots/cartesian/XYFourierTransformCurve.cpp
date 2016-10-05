/***************************************************************************
    File                 : XYFourierTransformCurve.cpp
    Project              : LabPlot
    Description          : A xy-curve defined by a Fourier transform
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
  \class XYFourierTransformCurve
  \brief A xy-curve defined by a Fourier transform

  \ingroup worksheet
*/

#include "XYFourierTransformCurve.h"
#include "XYFourierTransformCurvePrivate.h"
#include "backend/core/AbstractColumn.h"
#include "backend/core/column/Column.h"
#include "backend/lib/commandtemplates.h"

#include <cmath>	// isnan
extern "C" {
#include <gsl/gsl_errno.h>
#include "backend/nsl/nsl_sf_poly.h"
}

#include <KIcon>
#include <KLocale>
#include <QElapsedTimer>
#include <QThreadPool>
#ifndef NDEBUG
#include <QDebug>
#endif

XYFourierTransformCurve::XYFourierTransformCurve(const QString& name)
		: XYCurve(name, new XYFourierTransformCurvePrivate(this)) {
	init();
}

XYFourierTransformCurve::XYFourierTransformCurve(const QString& name, XYFourierTransformCurvePrivate* dd)
		: XYCurve(name, dd) {
	init();
}


XYFourierTransformCurve::~XYFourierTransformCurve() {
	//no need to delete the d-pointer here - it inherits from QGraphicsItem
	//and is deleted during the cleanup in QGraphicsScene
}

void XYFourierTransformCurve::init() {
	Q_D(XYFourierTransformCurve);

	//TODO: read from the saved settings for XYFourierTransformCurve?
	d->lineType = XYCurve::Line;
	d->symbolsStyle = Symbol::NoSymbols;
}

void XYFourierTransformCurve::recalculate() {
	Q_D(XYFourierTransformCurve);
	d->recalculate();
}

/*!
	Returns an icon to be used in the project explorer.
*/
QIcon XYFourierTransformCurve::icon() const {
	return KIcon("labplot-xy-fourier_transform-curve");
}

//##############################################################################
//##########################  getter methods  ##################################
//##############################################################################
BASIC_SHARED_D_READER_IMPL(XYFourierTransformCurve, const AbstractColumn*, xDataColumn, xDataColumn)
BASIC_SHARED_D_READER_IMPL(XYFourierTransformCurve, const AbstractColumn*, yDataColumn, yDataColumn)
const QString& XYFourierTransformCurve::xDataColumnPath() const { Q_D(const XYFourierTransformCurve); return d->xDataColumnPath; }
const QString& XYFourierTransformCurve::yDataColumnPath() const { Q_D(const XYFourierTransformCurve); return d->yDataColumnPath; }

BASIC_SHARED_D_READER_IMPL(XYFourierTransformCurve, XYFourierTransformCurve::TransformData, transformData, transformData)

const XYFourierTransformCurve::TransformResult& XYFourierTransformCurve::transformResult() const {
	Q_D(const XYFourierTransformCurve);
	return d->transformResult;
}

bool XYFourierTransformCurve::isSourceDataChangedSinceLastTransform() const {
	Q_D(const XYFourierTransformCurve);
	return d->sourceDataChangedSinceLastTransform;
}

//##############################################################################
//#################  setter methods and undo commands ##########################
//##############################################################################
STD_SETTER_CMD_IMPL_S(XYFourierTransformCurve, SetXDataColumn, const AbstractColumn*, xDataColumn)
void XYFourierTransformCurve::setXDataColumn(const AbstractColumn* column) {
	Q_D(XYFourierTransformCurve);
	if (column != d->xDataColumn) {
		exec(new XYFourierTransformCurveSetXDataColumnCmd(d, column, i18n("%1: assign x-data")));
		emit sourceDataChangedSinceLastTransform();
		if (column) {
			connect(column, SIGNAL(dataChanged(const AbstractColumn*)), this, SLOT(handleSourceDataChanged()));
			//TODO disconnect on undo
		}
	}
}

STD_SETTER_CMD_IMPL_S(XYFourierTransformCurve, SetYDataColumn, const AbstractColumn*, yDataColumn)
void XYFourierTransformCurve::setYDataColumn(const AbstractColumn* column) {
	Q_D(XYFourierTransformCurve);
	if (column != d->yDataColumn) {
		exec(new XYFourierTransformCurveSetYDataColumnCmd(d, column, i18n("%1: assign y-data")));
		emit sourceDataChangedSinceLastTransform();
		if (column) {
			connect(column, SIGNAL(dataChanged(const AbstractColumn*)), this, SLOT(handleSourceDataChanged()));
			//TODO disconnect on undo
		}
	}
}

STD_SETTER_CMD_IMPL_F_S(XYFourierTransformCurve, SetTransformData, XYFourierTransformCurve::TransformData, transformData, recalculate);
void XYFourierTransformCurve::setTransformData(const XYFourierTransformCurve::TransformData& transformData) {
	Q_D(XYFourierTransformCurve);
	exec(new XYFourierTransformCurveSetTransformDataCmd(d, transformData, i18n("%1: set transform options and perform the Fourier transform")));
}

//##############################################################################
//################################## SLOTS ####################################
//##############################################################################
void XYFourierTransformCurve::handleSourceDataChanged() {
	Q_D(XYFourierTransformCurve);
	d->sourceDataChangedSinceLastTransform = true;
	emit sourceDataChangedSinceLastTransform();
}
//##############################################################################
//######################### Private implementation #############################
//##############################################################################
XYFourierTransformCurvePrivate::XYFourierTransformCurvePrivate(XYFourierTransformCurve* owner) : XYCurvePrivate(owner),
	xDataColumn(0), yDataColumn(0), 
	xColumn(0), yColumn(0), 
	xVector(0), yVector(0), 
	sourceDataChangedSinceLastTransform(false),
	q(owner) {

}

XYFourierTransformCurvePrivate::~XYFourierTransformCurvePrivate() {
	//no need to delete xColumn and yColumn, they are deleted
	//when the parent aspect is removed
}

// ...
// see XYFitCurvePrivate

void XYFourierTransformCurvePrivate::recalculate() {
	QElapsedTimer timer;
	timer.start();

	//create transform result columns if not available yet, clear them otherwise
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
	transformResult = XYFourierTransformCurve::TransformResult();

	if (!xDataColumn || !yDataColumn) {
		emit (q->dataChanged());
		sourceDataChangedSinceLastTransform = false;
		return;
	}

	//check column sizes
	if (xDataColumn->rowCount()!=yDataColumn->rowCount()) {
		transformResult.available = true;
		transformResult.valid = false;
		transformResult.status = i18n("Number of x and y data points must be equal.");
		emit (q->dataChanged());
		sourceDataChangedSinceLastTransform = false;
		return;
	}

	//copy all valid data point for the transform to temporary vectors
	QVector<double> xdataVector;
	QVector<double> ydataVector;
	const double xmin = transformData.xRange.front();
	const double xmax = transformData.xRange.back();
	for (int row=0; row<xDataColumn->rowCount(); ++row) {
		//only copy those data where _all_ values (for x and y, if given) are valid
		if (!std::isnan(xDataColumn->valueAt(row)) && !std::isnan(yDataColumn->valueAt(row))
				&& !xDataColumn->isMasked(row) && !yDataColumn->isMasked(row)) {
			// only when inside given range
			if (xDataColumn->valueAt(row) >= xmin && xDataColumn->valueAt(row) <= xmax) {
				xdataVector.append(xDataColumn->valueAt(row));
				ydataVector.append(yDataColumn->valueAt(row));
			}
		}
	}

	//number of data points to transform
	unsigned int n = ydataVector.size();
	if (n == 0) {
		transformResult.available = true;
		transformResult.valid = false;
		transformResult.status = i18n("No data points available.");
		emit (q->dataChanged());
		sourceDataChangedSinceLastTransform = false;
		return;
	}

	double* xdata = xdataVector.data();
	double* ydata = ydataVector.data();

	// transform settings
	const nsl_sf_window_type windowType = transformData.windowType;
	const nsl_dft_result_type type = transformData.type;
	const bool twoSided = transformData.twoSided;
	const bool shifted = transformData.shifted;
	const nsl_dft_xscale xScale = transformData.xScale;
#ifndef NDEBUG
	qDebug()<<"n ="<<n;
	qDebug()<<"window type:"<<nsl_sf_window_type_name[windowType];
	qDebug()<<"type:"<<nsl_dft_result_type_name[type];
	qDebug()<<"scale:"<<nsl_dft_xscale_name[xScale];
	qDebug()<<"two sided:"<<twoSided;
	qDebug()<<"shifted:"<<shifted;
	QDebug out = qDebug();
	for (unsigned int i=0; i < n; i++)
		out<<ydata[i];
#endif
///////////////////////////////////////////////////////////
	// transform with window
	int status = nsl_dft_transform_window(ydata, 1, n, twoSided, type, windowType);

	unsigned int N=n;
	if(twoSided == false)
		N=n/2;

	switch (xScale) {
	case nsl_dft_xscale_frequency:
		for (unsigned int i=0; i < N; i++) {
			if(i >= n/2 && shifted)
				xdata[i] = (n-1)/(xmax-xmin)*(i/(double)n-1.);
			else
				xdata[i] = (n-1)*i/(xmax-xmin)/n;
		}
		break;
	case nsl_dft_xscale_index:
		for (unsigned int i=0; i < N; i++) {
			if (i >= n/2 && shifted)
				xdata[i] = (int)i-(int) N;
			else
				xdata[i] = i;
		}
		break;
	case nsl_dft_xscale_period: {
		double f0 = (n-1)/(xmax-xmin)/n;
		for (unsigned int i=0; i < N; i++) {
			double f = (n-1)*i/(xmax-xmin)/n;
			xdata[i] = 1/(f+f0);
		}
		break;
	}
	}
#ifndef NDEBUG
	out = qDebug();
	for (unsigned int i=0; i < N; i++)
		out<<ydata[i]<<'('<<xdata[i]<<')';
#endif

	xVector->resize(N);
	yVector->resize(N);
	if(shifted) {
		memcpy(xVector->data(), &xdata[n/2], n/2*sizeof(double));
		memcpy(&xVector->data()[n/2], xdata, n/2*sizeof(double));
		memcpy(yVector->data(), &ydata[n/2], n/2*sizeof(double));
		memcpy(&yVector->data()[n/2], ydata, n/2*sizeof(double));
	} else {
		memcpy(xVector->data(), xdata, N*sizeof(double));
		memcpy(yVector->data(), ydata, N*sizeof(double));
	}
///////////////////////////////////////////////////////////

	//write the result
	transformResult.available = true;
	transformResult.valid = true;
	transformResult.status = QString(gsl_strerror(status));;
	transformResult.elapsedTime = timer.elapsed();

	//redraw the curve
	emit (q->dataChanged());
	sourceDataChangedSinceLastTransform = false;
}

//##############################################################################
//##################  Serialization/Deserialization  ###########################
//##############################################################################
//! Save as XML
void XYFourierTransformCurve::save(QXmlStreamWriter* writer) const{
	Q_D(const XYFourierTransformCurve);

	writer->writeStartElement("xyFourierTransformCurve");

	//write xy-curve information
	XYCurve::save(writer);

	//write xy-fourier_transform-curve specific information
	//transform data
	writer->writeStartElement("transformData");
	WRITE_COLUMN(d->xDataColumn, xDataColumn);
	WRITE_COLUMN(d->yDataColumn, yDataColumn);
	writer->writeAttribute( "autoRange", QString::number(d->transformData.autoRange) );
	writer->writeAttribute( "xRangeMin", QString::number(d->transformData.xRange.front()) );
	writer->writeAttribute( "xRangeMax", QString::number(d->transformData.xRange.back()) );
	writer->writeAttribute( "type", QString::number(d->transformData.type) );
	writer->writeAttribute( "twoSided", QString::number(d->transformData.twoSided) );
	writer->writeAttribute( "shifted", QString::number(d->transformData.shifted) );
	writer->writeAttribute( "xScale", QString::number(d->transformData.xScale) );
	writer->writeAttribute( "windowType", QString::number(d->transformData.windowType) );
	writer->writeEndElement();// transformData

	//transform results (generated columns)
	writer->writeStartElement("transformResult");
	writer->writeAttribute( "available", QString::number(d->transformResult.available) );
	writer->writeAttribute( "valid", QString::number(d->transformResult.valid) );
	writer->writeAttribute( "status", d->transformResult.status );
	writer->writeAttribute( "time", QString::number(d->transformResult.elapsedTime) );

	//save calculated columns if available
	if (d->xColumn && d->yColumn) {
		d->xColumn->save(writer);
		d->yColumn->save(writer);
	}
	writer->writeEndElement(); //"transformResult"
	writer->writeEndElement(); //"xyFourierTransformCurve"
}

//! Load from XML
bool XYFourierTransformCurve::load(XmlStreamReader* reader) {
	Q_D(XYFourierTransformCurve);

	if (!reader->isStartElement() || reader->name() != "xyFourierTransformCurve") {
		reader->raiseError(i18n("no xy Fourier transform curve element found"));
		return false;
	}

	QString attributeWarning = i18n("Attribute '%1' missing or empty, default value is used");
	QXmlStreamAttributes attribs;
	QString str;

	while (!reader->atEnd()) {
		reader->readNext();
		if (reader->isEndElement() && reader->name() == "xyFourierTransformCurve")
			break;

		if (!reader->isStartElement())
			continue;

		if (reader->name() == "xyCurve") {
			if ( !XYCurve::load(reader) )
				return false;
		} else if (reader->name() == "transformData") {
			attribs = reader->attributes();

			READ_COLUMN(xDataColumn);
			READ_COLUMN(yDataColumn);

			str = attribs.value("autoRange").toString();
			if (str.isEmpty())
				reader->raiseWarning(attributeWarning.arg("'autoRange'"));
			else
				d->transformData.autoRange = (bool)str.toInt();

			str = attribs.value("xRangeMin").toString();
			if (str.isEmpty())
				reader->raiseWarning(attributeWarning.arg("'xRangeMin'"));
			else
				d->transformData.xRange.front() = str.toDouble();

			str = attribs.value("xRangeMax").toString();
			if (str.isEmpty())
				reader->raiseWarning(attributeWarning.arg("'xRangeMax'"));
			else
				d->transformData.xRange.back() = str.toDouble();

			str = attribs.value("type").toString();
			if (str.isEmpty())
				reader->raiseWarning(attributeWarning.arg("'type'"));
			else
				d->transformData.type = (nsl_dft_result_type)str.toInt();

			str = attribs.value("twoSided").toString();
			if (str.isEmpty())
				reader->raiseWarning(attributeWarning.arg("'twoSided'"));
			else
				d->transformData.twoSided = (bool)str.toInt();

			str = attribs.value("shifted").toString();
			if (str.isEmpty())
				reader->raiseWarning(attributeWarning.arg("'shifted'"));
			else
				d->transformData.shifted = (bool)str.toInt();

			str = attribs.value("xScale").toString();
			if (str.isEmpty())
				reader->raiseWarning(attributeWarning.arg("'xScale'"));
			else
				d->transformData.xScale = (nsl_dft_xscale)str.toInt();

			str = attribs.value("windowType").toString();
			if (str.isEmpty())
				reader->raiseWarning(attributeWarning.arg("'windowType'"));
			else
				d->transformData.windowType = (nsl_sf_window_type)str.toInt();
		} else if (reader->name() == "transformResult") {

			attribs = reader->attributes();

			str = attribs.value("available").toString();
			if (str.isEmpty())
				reader->raiseWarning(attributeWarning.arg("'available'"));
			else
				d->transformResult.available = str.toInt();

			str = attribs.value("valid").toString();
			if (str.isEmpty())
				reader->raiseWarning(attributeWarning.arg("'valid'"));
			else
				d->transformResult.valid = str.toInt();
			
			str = attribs.value("status").toString();
			if (str.isEmpty())
				reader->raiseWarning(attributeWarning.arg("'status'"));
			else
				d->transformResult.status = str;

			str = attribs.value("time").toString();
			if (str.isEmpty())
				reader->raiseWarning(attributeWarning.arg("'time'"));
			else
				d->transformResult.elapsedTime = str.toInt();
		} else if (reader->name() == "column") {
			Column* column = new Column("", AbstractColumn::Numeric);
			if (!column->load(reader)) {
				delete column;
				return false;
			}

			if (column->name() == "x")
				d->xColumn = column;
			else if (column->name() == "y")
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
	} else {
		qWarning()<<"	d->xColumn == NULL!";
	}

	return true;
}
