/***************************************************************************
    File                 : XYConvolutionCurve.cpp
    Project              : LabPlot
    Description          : A xy-curve defined by a convolution
    --------------------------------------------------------------------
    Copyright            : (C) 2018 Stefan Gerlach (stefan.gerlach@uni.kn)

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
  \class XYConvolutionCurve
  \brief A xy-curve defined by a convolution

  \ingroup worksheet
*/

#include "XYConvolutionCurve.h"
#include "XYConvolutionCurvePrivate.h"
#include "backend/core/column/Column.h"
#include "backend/lib/commandtemplates.h"
#include "backend/lib/macros.h"

#include <KLocalizedString>
#include <QIcon>
#include <QElapsedTimer>
#include <QThreadPool>

extern "C" {
#include <gsl/gsl_math.h>
}

XYConvolutionCurve::XYConvolutionCurve(const QString& name)
	: XYAnalysisCurve(name, new XYConvolutionCurvePrivate(this), AspectType::XYConvolutionCurve) {
}

XYConvolutionCurve::XYConvolutionCurve(const QString& name, XYConvolutionCurvePrivate* dd)
	: XYAnalysisCurve(name, dd, AspectType::XYConvolutionCurve) {
}

//no need to delete the d-pointer here - it inherits from QGraphicsItem
//and is deleted during the cleanup in QGraphicsScene
XYConvolutionCurve::~XYConvolutionCurve() = default;

void XYConvolutionCurve::recalculate() {
	Q_D(XYConvolutionCurve);
	d->recalculate();
}

/*!
	Returns an icon to be used in the project explorer.
*/
QIcon XYConvolutionCurve::icon() const {
// 	return QIcon::fromTheme("labplot-xy-convolution-curve");//not available yet
	return QIcon::fromTheme("labplot-xy-curve");
}

//##############################################################################
//##########################  getter methods  ##################################
//##############################################################################
BASIC_SHARED_D_READER_IMPL(XYConvolutionCurve, XYConvolutionCurve::ConvolutionData, convolutionData, convolutionData)

const XYConvolutionCurve::ConvolutionResult& XYConvolutionCurve::convolutionResult() const {
	Q_D(const XYConvolutionCurve);
	return d->convolutionResult;
}

//##############################################################################
//#################  setter methods and undo commands ##########################
//##############################################################################
STD_SETTER_CMD_IMPL_F_S(XYConvolutionCurve, SetConvolutionData, XYConvolutionCurve::ConvolutionData, convolutionData, recalculate);
void XYConvolutionCurve::setConvolutionData(const XYConvolutionCurve::ConvolutionData& convolutionData) {
	Q_D(XYConvolutionCurve);
	exec(new XYConvolutionCurveSetConvolutionDataCmd(d, convolutionData, ki18n("%1: set options and perform the convolution")));
}

//##############################################################################
//######################### Private implementation #############################
//##############################################################################
XYConvolutionCurvePrivate::XYConvolutionCurvePrivate(XYConvolutionCurve* owner) : XYAnalysisCurvePrivate(owner), q(owner) {
}

//no need to delete xColumn and yColumn, they are deleted
//when the parent aspect is removed
XYConvolutionCurvePrivate::~XYConvolutionCurvePrivate() = default;

void XYConvolutionCurvePrivate::recalculate() {
	QElapsedTimer timer;
	timer.start();

	//create convolution result columns if not available yet, clear them otherwise
	if (!xColumn) {
		xColumn = new Column("x", AbstractColumn::ColumnMode::Numeric);
		yColumn = new Column("y", AbstractColumn::ColumnMode::Numeric);
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

	//clear the previous result
	convolutionResult = XYConvolutionCurve::ConvolutionResult();

	//determine the data source columns
	const AbstractColumn* tmpXDataColumn = nullptr;
	const AbstractColumn* tmpYDataColumn = nullptr;
	const AbstractColumn* tmpY2DataColumn = nullptr;
	if (dataSourceType == XYAnalysisCurve::DataSourceType::Spreadsheet) {
		//spreadsheet columns as data source
		tmpXDataColumn = xDataColumn;
		tmpYDataColumn = yDataColumn;
		tmpY2DataColumn = y2DataColumn;
	} else {
		//curve columns as data source
		tmpXDataColumn = dataSourceCurve->xColumn();
		tmpYDataColumn = dataSourceCurve->yColumn();
		// no y2 column: use standard kernel
	}

	if (tmpYDataColumn == nullptr) {
		recalcLogicalPoints();
		emit q->dataChanged();
		sourceDataChangedSinceLastRecalc = false;
		return;
	}

	//copy all valid data point for the convolution to temporary vectors
	QVector<double> xdataVector;
	QVector<double> ydataVector;
	QVector<double> y2dataVector;

	double xmin, xmax;
	if (tmpXDataColumn && convolutionData.autoRange) {
		xmin = tmpXDataColumn->minimum();
		xmax = tmpXDataColumn->maximum();
	} else {
		xmin = convolutionData.xRange.first();
		xmax = convolutionData.xRange.last();
	}

	//only copy those data where values are valid and in range
	if (tmpXDataColumn) {	// x-axis present (with possible range)
		for (int row = 0; row < tmpXDataColumn->rowCount(); ++row) {
			if (tmpXDataColumn->isValid(row) && !tmpXDataColumn->isMasked(row)
				&& tmpYDataColumn->isValid(row) && !tmpYDataColumn->isMasked(row)) {
				if (tmpXDataColumn->valueAt(row) >= xmin && tmpXDataColumn->valueAt(row) <= xmax) {
					xdataVector.append(tmpXDataColumn->valueAt(row));
					ydataVector.append(tmpYDataColumn->valueAt(row));
				}
			}
		}
	} else {	// no x-axis: take all valid values
		for (int row = 0; row < tmpYDataColumn->rowCount(); ++row)
			if (tmpYDataColumn->isValid(row) && !tmpYDataColumn->isMasked(row))
				ydataVector.append(tmpYDataColumn->valueAt(row));
	}

	const nsl_conv_kernel_type kernel = convolutionData.kernel;
	const size_t kernelSize = convolutionData.kernelSize;
	if (tmpY2DataColumn != nullptr) {
		for (int row = 0; row < tmpY2DataColumn->rowCount(); ++row)
			if (tmpY2DataColumn->isValid(row) && !tmpY2DataColumn->isMasked(row))
				y2dataVector.append(tmpY2DataColumn->valueAt(row));
		DEBUG("kernel = given response");
	} else {
		DEBUG("kernel = " << nsl_conv_kernel_name[kernel] << ", size = " << kernelSize);
		double* k = new double[kernelSize];
		nsl_conv_standard_kernel(k, kernelSize, kernel);
		for (size_t i = 0; i < kernelSize; i++)
			y2dataVector.append(k[i]);
		delete[] k;
	}

	const size_t n = (size_t)ydataVector.size();	// number of points for signal
	const size_t m = (size_t)y2dataVector.size();	// number of points for response
	if (n < 1 || m < 1) {
		convolutionResult.available = true;
		convolutionResult.valid = false;
		convolutionResult.status = i18n("Not enough data points available.");
		recalcLogicalPoints();
		emit q->dataChanged();
		sourceDataChangedSinceLastRecalc = false;
		return;
	}

	double* xdata = xdataVector.data();
	double* ydata = ydataVector.data();
	double* y2data = y2dataVector.data();

	// convolution settings
	const double samplingInterval = convolutionData.samplingInterval;
	const nsl_conv_direction_type direction = convolutionData.direction;
	const nsl_conv_type_type type = convolutionData.type;
	const nsl_conv_method_type method = convolutionData.method;
	const nsl_conv_norm_type norm = convolutionData.normalize;
	const nsl_conv_wrap_type wrap = convolutionData.wrap;

	DEBUG("signal n = " << n << ", response m = " << m);
	DEBUG("sampling interval = " << samplingInterval);
	DEBUG("direction = " << nsl_conv_direction_name[direction]);
	DEBUG("type = " << nsl_conv_type_name[type]);
	DEBUG("method = " << nsl_conv_method_name[method]);
	DEBUG("norm = " << nsl_conv_norm_name[norm]);
	DEBUG("wrap = " << nsl_conv_wrap_name[wrap]);

///////////////////////////////////////////////////////////
	size_t np;
	if (type == nsl_conv_type_linear)
		np = n + m - 1;
	else
		np = GSL_MAX(n, m);

	double* out = (double*)malloc(np * sizeof(double));
	int status = nsl_conv_convolution_direction(ydata, n, y2data, m, direction, type, method, norm, wrap, out);

	if (direction == nsl_conv_direction_backward)
		if (type == nsl_conv_type_linear)
			np = abs((int)(n - m)) + 1;

	xVector->resize((int)np);
	yVector->resize((int)np);
	// take given x-axis values or use index
	if (tmpXDataColumn != nullptr) {
		int size = GSL_MIN(xdataVector.size(), (int)np);
		memcpy(xVector->data(), xdata, size * sizeof(double));
		double sampleInterval = (xVector->data()[size-1] - xVector->data()[0])/(xdataVector.size()-1);
		DEBUG("xdata size = " << xdataVector.size() << ", np = " << np << ", sample interval = " << sampleInterval);
		for (int i = size; i < (int)np; i++)	// fill missing values
			xVector->data()[i] = xVector->data()[size-1] + (i-size+1) * sampleInterval;
	} else {	// fill with index (starting with 0)
		for (size_t i = 0; i < np; i++)
			xVector->data()[i] = i * samplingInterval;
	}

	memcpy(yVector->data(), out, np * sizeof(double));
	free(out);
///////////////////////////////////////////////////////////

	//write the result
	convolutionResult.available = true;
	convolutionResult.valid = true;
	convolutionResult.status = QString::number(status);
	convolutionResult.elapsedTime = timer.elapsed();

	//redraw the curve
	recalcLogicalPoints();
	emit q->dataChanged();
	sourceDataChangedSinceLastRecalc = false;
}

//##############################################################################
//##################  Serialization/Deserialization  ###########################
//##############################################################################
//! Save as XML
void XYConvolutionCurve::save(QXmlStreamWriter* writer) const{
	Q_D(const XYConvolutionCurve);

	writer->writeStartElement("xyConvolutionCurve");

	//write the base class
	XYAnalysisCurve::save(writer);

	//write xy-convolution-curve specific information
	// convolution data
	writer->writeStartElement("convolutionData");
	writer->writeAttribute( "samplingInterval", QString::number(d->convolutionData.samplingInterval) );
	writer->writeAttribute( "kernel", QString::number(d->convolutionData.kernel) );
	writer->writeAttribute( "kernelSize", QString::number(d->convolutionData.kernelSize) );
	writer->writeAttribute( "autoRange", QString::number(d->convolutionData.autoRange) );
	writer->writeAttribute( "xRangeMin", QString::number(d->convolutionData.xRange.first()) );
	writer->writeAttribute( "xRangeMax", QString::number(d->convolutionData.xRange.last()) );
	writer->writeAttribute( "direction", QString::number(d->convolutionData.direction) );
	writer->writeAttribute( "type", QString::number(d->convolutionData.type) );
	writer->writeAttribute( "method", QString::number(d->convolutionData.method) );
	writer->writeAttribute( "normalize", QString::number(d->convolutionData.normalize) );
	writer->writeAttribute( "wrap", QString::number(d->convolutionData.wrap) );
	writer->writeEndElement();// convolutionData

	// convolution results (generated columns)
	writer->writeStartElement("convolutionResult");
	writer->writeAttribute( "available", QString::number(d->convolutionResult.available) );
	writer->writeAttribute( "valid", QString::number(d->convolutionResult.valid) );
	writer->writeAttribute( "status", d->convolutionResult.status );
	writer->writeAttribute( "time", QString::number(d->convolutionResult.elapsedTime) );

	//save calculated columns if available
	if (d->xColumn) {
		d->xColumn->save(writer);
		d->yColumn->save(writer);
	}
	writer->writeEndElement(); //"convolutionResult"

	writer->writeEndElement(); //"xyConvolutionCurve"
}

//! Load from XML
bool XYConvolutionCurve::load(XmlStreamReader* reader, bool preview) {
	DEBUG("XYConvolutionCurve::load()");
	Q_D(XYConvolutionCurve);

	KLocalizedString attributeWarning = ki18n("Attribute '%1' missing or empty, default value is used");
	QXmlStreamAttributes attribs;
	QString str;

	while (!reader->atEnd()) {
		reader->readNext();
		if (reader->isEndElement() && reader->name() == "xyConvolutionCurve")
			break;

		if (!reader->isStartElement())
			continue;

		if (reader->name() == "xyAnalysisCurve") {
			if ( !XYAnalysisCurve::load(reader, preview) )
				return false;
		} else if (!preview && reader->name() == "convolutionData") {
			attribs = reader->attributes();
			READ_DOUBLE_VALUE("samplingInterval", convolutionData.samplingInterval);
			READ_INT_VALUE("kernel", convolutionData.kernel, nsl_conv_kernel_type);
			READ_INT_VALUE("kernelSize", convolutionData.kernelSize, size_t);
			READ_INT_VALUE("autoRange", convolutionData.autoRange, bool);
			READ_DOUBLE_VALUE("xRangeMin", convolutionData.xRange.first());
			READ_DOUBLE_VALUE("xRangeMax", convolutionData.xRange.last());
			READ_INT_VALUE("direction", convolutionData.direction, nsl_conv_direction_type);
			READ_INT_VALUE("type", convolutionData.type, nsl_conv_type_type);
			READ_INT_VALUE("method", convolutionData.method, nsl_conv_method_type);
			READ_INT_VALUE("normalize", convolutionData.normalize, nsl_conv_norm_type);
			READ_INT_VALUE("wrap", convolutionData.wrap, nsl_conv_wrap_type);
		} else if (!preview && reader->name() == "convolutionResult") {
			attribs = reader->attributes();
			READ_INT_VALUE("available", convolutionResult.available, int);
			READ_INT_VALUE("valid", convolutionResult.valid, int);
			READ_STRING_VALUE("status", convolutionResult.status);
			READ_INT_VALUE("time", convolutionResult.elapsedTime, int);
		} else if (!preview && reader->name() == "column") {
			Column* column = new Column(QString(), AbstractColumn::ColumnMode::Numeric);
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
