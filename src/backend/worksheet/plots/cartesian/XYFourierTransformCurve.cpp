/***************************************************************************
    File                 : XYFourierTransformCurve.cpp
    Project              : LabPlot
    Description          : A xy-curve defined by a Fourier transform
    --------------------------------------------------------------------
    Copyright            : (C) 2016 Stefan Gerlach (stefan.gerlach@uni.kn)
    Copyright            : (C) 2017 Alexander Semke (alexander.semke@web.de)

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
#include "backend/lib/macros.h"
#include "backend/gsl/errors.h"

extern "C" {
#include "backend/nsl/nsl_sf_poly.h"
}

#include <KLocalizedString>
#include <QElapsedTimer>
#include <QIcon>
#include <QThreadPool>
#include <QDebug>	// qWarning()

XYFourierTransformCurve::XYFourierTransformCurve(const QString& name)
	: XYAnalysisCurve(name, new XYFourierTransformCurvePrivate(this), AspectType::XYFourierTransformCurve) {
}

XYFourierTransformCurve::XYFourierTransformCurve(const QString& name, XYFourierTransformCurvePrivate* dd)
	: XYAnalysisCurve(name, dd, AspectType::XYFourierTransformCurve) {
}

//no need to delete the d-pointer here - it inherits from QGraphicsItem
//and is deleted during the cleanup in QGraphicsScene
XYFourierTransformCurve::~XYFourierTransformCurve() = default;

void XYFourierTransformCurve::recalculate() {
	Q_D(XYFourierTransformCurve);
	d->recalculate();
}

/*!
	Returns an icon to be used in the project explorer.
*/
QIcon XYFourierTransformCurve::icon() const {
	return QIcon::fromTheme("labplot-xy-fourier_transform-curve");
}

//##############################################################################
//##########################  getter methods  ##################################
//##############################################################################
BASIC_SHARED_D_READER_IMPL(XYFourierTransformCurve, XYFourierTransformCurve::TransformData, transformData, transformData)

const XYFourierTransformCurve::TransformResult& XYFourierTransformCurve::transformResult() const {
	Q_D(const XYFourierTransformCurve);
	return d->transformResult;
}

//##############################################################################
//#################  setter methods and undo commands ##########################
//##############################################################################
STD_SETTER_CMD_IMPL_F_S(XYFourierTransformCurve, SetTransformData, XYFourierTransformCurve::TransformData, transformData, recalculate);
void XYFourierTransformCurve::setTransformData(const XYFourierTransformCurve::TransformData& transformData) {
	Q_D(XYFourierTransformCurve);
	exec(new XYFourierTransformCurveSetTransformDataCmd(d, transformData, ki18n("%1: set transform options and perform the Fourier transform")));
}

//##############################################################################
//######################### Private implementation #############################
//##############################################################################
XYFourierTransformCurvePrivate::XYFourierTransformCurvePrivate(XYFourierTransformCurve* owner) : XYAnalysisCurvePrivate(owner), q(owner) {
}

//no need to delete xColumn and yColumn, they are deleted
//when the parent aspect is removed
XYFourierTransformCurvePrivate::~XYFourierTransformCurvePrivate() = default;

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
		recalcLogicalPoints();
		emit q->dataChanged();
		sourceDataChangedSinceLastRecalc = false;
		return;
	}

	//check column sizes
	if (xDataColumn->rowCount() != yDataColumn->rowCount()) {
		transformResult.available = true;
		transformResult.valid = false;
		transformResult.status = i18n("Number of x and y data points must be equal.");
		emit q->dataChanged();
		sourceDataChangedSinceLastRecalc = false;
		return;
	}

	//copy all valid data point for the transform to temporary vectors
	QVector<double> xdataVector;
	QVector<double> ydataVector;
	const double xmin = transformData.xRange.first();
	const double xmax = transformData.xRange.last();
	for (int row = 0; row < xDataColumn->rowCount(); ++row) {
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
	unsigned int n = (unsigned int)ydataVector.size();
	if (n == 0) {
		transformResult.available = true;
		transformResult.valid = false;
		transformResult.status = i18n("No data points available.");
		recalcLogicalPoints();
		emit q->dataChanged();
		sourceDataChangedSinceLastRecalc = false;
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

	DEBUG("n =" << n);
	DEBUG("window type:" << nsl_sf_window_type_name[windowType]);
	DEBUG("type:" << nsl_dft_result_type_name[type]);
	DEBUG("scale:" << nsl_dft_xscale_name[xScale]);
	DEBUG("two sided:" << twoSided);
	DEBUG("shifted:" << shifted);
#ifndef NDEBUG
	QDebug out = qDebug();
	for (unsigned int i = 0; i < n; i++)
		out<<ydata[i];
#endif

///////////////////////////////////////////////////////////
	// transform with window
	int status = nsl_dft_transform_window(ydata, 1, n, twoSided, type, windowType);

	unsigned int N = n;
	if (twoSided == false)
		N = n/2;

	switch (xScale) {
	case nsl_dft_xscale_frequency:
		for (unsigned int i = 0; i < N; i++) {
			if (i >= n/2 && shifted)
				xdata[i] = (n-1)/(xmax-xmin)*(i/(double)n-1.);
			else
				xdata[i] = (n-1)*i/(xmax-xmin)/n;
		}
		break;
	case nsl_dft_xscale_index:
		for (unsigned int i = 0; i < N; i++) {
			if (i >= n/2 && shifted)
				xdata[i] = (int)i-(int) N;
			else
				xdata[i] = i;
		}
		break;
	case nsl_dft_xscale_period: {
			double f0 = (n-1)/(xmax-xmin)/n;
			for (unsigned int i = 0; i < N; i++) {
				double f = (n-1)*i/(xmax-xmin)/n;
				xdata[i] = 1/(f+f0);
			}
			break;
		}
	}
#ifndef NDEBUG
	out = qDebug();
	for (unsigned int i = 0; i < N; i++)
		out << ydata[i] << '(' << xdata[i] << ')';
#endif

	xVector->resize((int)N);
	yVector->resize((int)N);
	if (shifted) {
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
	transformResult.status = gslErrorToString(status);
	transformResult.elapsedTime = timer.elapsed();

	//redraw the curve
	recalcLogicalPoints();
	emit q->dataChanged();
	sourceDataChangedSinceLastRecalc = false;
}

//##############################################################################
//##################  Serialization/Deserialization  ###########################
//##############################################################################
//! Save as XML
void XYFourierTransformCurve::save(QXmlStreamWriter* writer) const {
	Q_D(const XYFourierTransformCurve);

	writer->writeStartElement("xyFourierTransformCurve");

	//write the base class
	XYAnalysisCurve::save(writer);

	//write xy-fourier_transform-curve specific information
	//transform data
	writer->writeStartElement("transformData");
	writer->writeAttribute( "autoRange", QString::number(d->transformData.autoRange) );
	writer->writeAttribute( "xRangeMin", QString::number(d->transformData.xRange.first()) );
	writer->writeAttribute( "xRangeMax", QString::number(d->transformData.xRange.last()) );
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
bool XYFourierTransformCurve::load(XmlStreamReader* reader, bool preview) {
	Q_D(XYFourierTransformCurve);

	KLocalizedString attributeWarning = ki18n("Attribute '%1' missing or empty, default value is used");
	QXmlStreamAttributes attribs;
	QString str;

	while (!reader->atEnd()) {
		reader->readNext();
		if (reader->isEndElement() && reader->name() == "xyFourierTransformCurve")
			break;

		if (!reader->isStartElement())
			continue;

		if (reader->name() == "xyAnalysisCurve") {
			if ( !XYAnalysisCurve::load(reader, preview) )
				return false;
		} else if (!preview && reader->name() == "transformData") {
			attribs = reader->attributes();
			READ_INT_VALUE("autoRange", transformData.autoRange, bool);
			READ_DOUBLE_VALUE("xRangeMin", transformData.xRange.first());
			READ_DOUBLE_VALUE("xRangeMax", transformData.xRange.last());
			READ_INT_VALUE("type", transformData.type, nsl_dft_result_type);
			READ_INT_VALUE("twoSided", transformData.twoSided, bool);
			READ_INT_VALUE("shifted", transformData.shifted, bool);
			READ_INT_VALUE("xScale", transformData.xScale, nsl_dft_xscale);
			READ_INT_VALUE("windowType", transformData.windowType, nsl_sf_window_type);
		} else if (!preview && reader->name() == "transformResult") {
			attribs = reader->attributes();
			READ_INT_VALUE("available", transformResult.available, int);
			READ_INT_VALUE("valid", transformResult.valid, int);
			READ_STRING_VALUE("status", transformResult.status);
			READ_INT_VALUE("time", transformResult.elapsedTime, int);
		} else if (reader->name() == "column") {
			Column* column = new Column(QString(), AbstractColumn::Numeric);
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

		XYCurve::d_ptr->xColumn = d->xColumn;
		XYCurve::d_ptr->yColumn = d->yColumn;

		recalcLogicalPoints();
	}

	return true;
}
