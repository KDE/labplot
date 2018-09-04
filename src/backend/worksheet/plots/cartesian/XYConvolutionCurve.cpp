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

XYConvolutionCurve::XYConvolutionCurve(const QString& name)
		: XYAnalysisCurve(name, new XYConvolutionCurvePrivate(this)) {
}

XYConvolutionCurve::XYConvolutionCurve(const QString& name, XYConvolutionCurvePrivate* dd)
		: XYAnalysisCurve(name, dd) {
}

XYConvolutionCurve::~XYConvolutionCurve() {
	//no need to delete the d-pointer here - it inherits from QGraphicsItem
	//and is deleted during the cleanup in QGraphicsScene
}

void XYConvolutionCurve::recalculate() {
	Q_D(XYConvolutionCurve);
	d->recalculate();
}

/*!
	Returns an icon to be used in the project explorer.
*/
QIcon XYConvolutionCurve::icon() const {
	return QIcon::fromTheme("labplot-xy-convolution-curve");
}

//##############################################################################
//##########################  getter methods  ##################################
//##############################################################################
BASIC_SHARED_D_READER_IMPL(XYConvolutionCurve, XYConvolutionCurve::ConvolutionData, convolutionData, convolutionData)

const XYConvolutionCurve::ConvolutionResult& XYConvolutionCurve::convolutionResult() const {
	Q_D(const XYConvolutionCurve);
	return d->convolutionResult;
}

BASIC_SHARED_D_READER_IMPL(XYConvolutionCurve, const AbstractColumn*, y2DataColumn, y2DataColumn)
const QString& XYConvolutionCurve::y2DataColumnPath() const { Q_D(const XYConvolutionCurve); return d->y2DataColumnPath; }
//##############################################################################
//#################  setter methods and undo commands ##########################
//##############################################################################
STD_SETTER_CMD_IMPL_F_S(XYConvolutionCurve, SetConvolutionData, XYConvolutionCurve::ConvolutionData, convolutionData, recalculate);
void XYConvolutionCurve::setConvolutionData(const XYConvolutionCurve::ConvolutionData& convolutionData) {
	Q_D(XYConvolutionCurve);
	exec(new XYConvolutionCurveSetConvolutionDataCmd(d, convolutionData, ki18n("%1: set options and perform the convolution")));
}

STD_SETTER_CMD_IMPL_S(XYConvolutionCurve, SetY2DataColumn, const AbstractColumn*, y2DataColumn)
void XYConvolutionCurve::setY2DataColumn(const AbstractColumn* column) {
	Q_D(XYConvolutionCurve);
	if (column != d->y2DataColumn) {
		exec(new XYConvolutionCurveSetY2DataColumnCmd(d, column, ki18n("%1: assign response y-data")));
		handleSourceDataChanged();
		if (column) {
			connect(column, SIGNAL(dataChanged(const AbstractColumn*)), this, SLOT(handleSourceDataChanged()));
			//TODO disconnect on undo
		}
	}
}

//##############################################################################
//######################### Private implementation #############################
//##############################################################################
XYConvolutionCurvePrivate::XYConvolutionCurvePrivate(XYConvolutionCurve* owner) : XYAnalysisCurvePrivate(owner),
	y2DataColumn(nullptr), q(owner)  {

}

XYConvolutionCurvePrivate::~XYConvolutionCurvePrivate() {
	//no need to delete xColumn and yColumn, they are deleted
	//when the parent aspect is removed
}

void XYConvolutionCurvePrivate::recalculate() {
	QElapsedTimer timer;
	timer.start();

	//create convolution result columns if not available yet, clear them otherwise
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
	convolutionResult = XYConvolutionCurve::ConvolutionResult();

	//determine the data source columns
	const AbstractColumn* tmpXDataColumn = nullptr;
	const AbstractColumn* tmpYDataColumn = nullptr;
	const AbstractColumn* tmpY2DataColumn = nullptr;
	if (dataSourceType == XYAnalysisCurve::DataSourceSpreadsheet) {
		//spreadsheet columns as data source
		tmpXDataColumn = xDataColumn;
		tmpYDataColumn = yDataColumn;
		tmpY2DataColumn = y2DataColumn;
	} else {
		//curve columns as data source
		tmpXDataColumn = dataSourceCurve->xColumn();
		tmpYDataColumn = dataSourceCurve->yColumn();
		//TODO: tmpY2DataColumn = dataSourceCurve->y2Column();
	}

	if (!tmpXDataColumn || !tmpYDataColumn) {
		emit q->dataChanged();
		sourceDataChangedSinceLastRecalc = false;
		return;
	}

	//check column sizes
	if (tmpXDataColumn->rowCount() != tmpYDataColumn->rowCount()) {
		convolutionResult.available = true;
		convolutionResult.valid = false;
		convolutionResult.status = i18n("Number of x and y data points must be equal.");
		emit q->dataChanged();
		sourceDataChangedSinceLastRecalc = false;
		return;
	}

	//copy all valid data point for the convolution to temporary vectors
	QVector<double> xdataVector;
	QVector<double> ydataVector;
	QVector<double> y2dataVector;

	double xmin;
	double xmax;
	if (convolutionData.autoRange) {
		xmin = tmpXDataColumn->minimum();
		xmax = tmpXDataColumn->maximum();
	} else {
		xmin = convolutionData.xRange.first();
		xmax = convolutionData.xRange.last();
	}

	for (int row = 0; row < tmpXDataColumn->rowCount(); ++row) {
		//only copy those data where _all_ values (for x and y, if given) are valid
		if (!std::isnan(tmpXDataColumn->valueAt(row)) && !std::isnan(tmpYDataColumn->valueAt(row))
			&& !tmpXDataColumn->isMasked(row) && !tmpYDataColumn->isMasked(row)) {

			// only when inside given range
			if (tmpXDataColumn->valueAt(row) >= xmin && tmpXDataColumn->valueAt(row) <= xmax) {
				xdataVector.append(tmpXDataColumn->valueAt(row));
				ydataVector.append(tmpYDataColumn->valueAt(row));
				y2dataVector.append(tmpY2DataColumn->valueAt(row));
			}
		}
	}

	const size_t n = (size_t)xdataVector.size();	// number of data points to integrate
	if (n < 2) {
		convolutionResult.available = true;
		convolutionResult.valid = false;
		convolutionResult.status = i18n("Not enough data points available.");
		emit q->dataChanged();
		sourceDataChangedSinceLastRecalc = false;
		return;
	}

	double* xdata = xdataVector.data();
	double* ydata = ydataVector.data();
	double* y2data = y2dataVector.data();

	// convolution settings
	const nsl_conv_type_type type = nsl_conv_type_linear;//TODO: convolutionData.type;
	const nsl_conv_method_type method = nsl_conv_method_auto;//TODO: convolutionData.method;
	const nsl_conv_direction_type direction = convolutionData.direction;
	const bool normalize = false;//TODO: convolutionData.normalize;
	const bool wrap = false;//TODO: convolutionData.wrap;

///////////////////////////////////////////////////////////
	int status = 0;
	size_t np = n;

	nsl_conv_convolution_direction(ydata, n, y2data, n, type, method, direction, normalize, wrap);

	xVector->resize((int)np);
	yVector->resize((int)np);
	memcpy(xVector->data(), xdata, np * sizeof(double));
	memcpy(yVector->data(), ydata, np * sizeof(double));
///////////////////////////////////////////////////////////

	//write the result
	convolutionResult.available = true;
	convolutionResult.valid = true;
	convolutionResult.status = QString::number(status);
	convolutionResult.elapsedTime = timer.elapsed();
	convolutionResult.value = ydata[np-1];

	//redraw the curve
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
	writer->writeAttribute( "autoRange", QString::number(d->convolutionData.autoRange) );
	writer->writeAttribute( "xRangeMin", QString::number(d->convolutionData.xRange.first()) );
	writer->writeAttribute( "xRangeMax", QString::number(d->convolutionData.xRange.last()) );
	writer->writeAttribute( "direction", QString::number(d->convolutionData.direction) );
	writer->writeEndElement();// convolutionData

	// convolution results (generated columns)
	writer->writeStartElement("convolutionResult");
	writer->writeAttribute( "available", QString::number(d->convolutionResult.available) );
	writer->writeAttribute( "valid", QString::number(d->convolutionResult.valid) );
	writer->writeAttribute( "status", d->convolutionResult.status );
	writer->writeAttribute( "time", QString::number(d->convolutionResult.elapsedTime) );
	writer->writeAttribute( "value", QString::number(d->convolutionResult.value) );

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
			READ_INT_VALUE("autoRange", convolutionData.autoRange, bool);
			READ_DOUBLE_VALUE("xRangeMin", convolutionData.xRange.first());
			READ_DOUBLE_VALUE("xRangeMax", convolutionData.xRange.last());
			READ_INT_VALUE("absolute", convolutionData.absolute, bool);
		} else if (!preview && reader->name() == "convolutionResult") {
			attribs = reader->attributes();
			READ_INT_VALUE("available", convolutionResult.available, int);
			READ_INT_VALUE("valid", convolutionResult.valid, int);
			READ_STRING_VALUE("status", convolutionResult.status);
			READ_INT_VALUE("time", convolutionResult.elapsedTime, int);
			READ_DOUBLE_VALUE("value", convolutionResult.value);
		} else if (!preview && reader->name() == "column") {
			Column* column = new Column("", AbstractColumn::Numeric);
			if (!column->load(reader, preview)) {
				delete column;
				return false;
			}
			if (column->name()=="x")
				d->xColumn = column;
			else if (column->name()=="y")
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

		setUndoAware(false);
		XYCurve::d_ptr->xColumn = d->xColumn;
		XYCurve::d_ptr->yColumn = d->yColumn;
		setUndoAware(true);
	}

	return true;
}
