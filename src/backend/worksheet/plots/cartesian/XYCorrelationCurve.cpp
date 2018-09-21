/***************************************************************************
    File                 : XYCorrelationCurve.cpp
    Project              : LabPlot
    Description          : A xy-curve defined by a correlation
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
  \class XYCorrelationCurve
  \brief A xy-curve defined by a correlation

  \ingroup worksheet
*/

#include "XYCorrelationCurve.h"
#include "XYCorrelationCurvePrivate.h"
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

XYCorrelationCurve::XYCorrelationCurve(const QString& name)
		: XYAnalysisCurve(name, new XYCorrelationCurvePrivate(this)) {
}

XYCorrelationCurve::XYCorrelationCurve(const QString& name, XYCorrelationCurvePrivate* dd)
		: XYAnalysisCurve(name, dd) {
}

XYCorrelationCurve::~XYCorrelationCurve() {
	//no need to delete the d-pointer here - it inherits from QGraphicsItem
	//and is deleted during the cleanup in QGraphicsScene
}

void XYCorrelationCurve::recalculate() {
	Q_D(XYCorrelationCurve);
	d->recalculate();
}

/*!
	Returns an icon to be used in the project explorer.
*/
QIcon XYCorrelationCurve::icon() const {
	return QIcon::fromTheme("labplot-xy-correlation-curve");
}

//##############################################################################
//##########################  getter methods  ##################################
//##############################################################################
BASIC_SHARED_D_READER_IMPL(XYCorrelationCurve, XYCorrelationCurve::CorrelationData, correlationData, correlationData)

const XYCorrelationCurve::CorrelationResult& XYCorrelationCurve::correlationResult() const {
	Q_D(const XYCorrelationCurve);
	return d->correlationResult;
}

//##############################################################################
//#################  setter methods and undo commands ##########################
//##############################################################################
STD_SETTER_CMD_IMPL_F_S(XYCorrelationCurve, SetCorrelationData, XYCorrelationCurve::CorrelationData, correlationData, recalculate);
void XYCorrelationCurve::setCorrelationData(const XYCorrelationCurve::CorrelationData& correlationData) {
	Q_D(XYCorrelationCurve);
	exec(new XYCorrelationCurveSetCorrelationDataCmd(d, correlationData, ki18n("%1: set options and perform the correlation")));
}

//##############################################################################
//######################### Private implementation #############################
//##############################################################################
XYCorrelationCurvePrivate::XYCorrelationCurvePrivate(XYCorrelationCurve* owner) : XYAnalysisCurvePrivate(owner),
	q(owner)  {
}

XYCorrelationCurvePrivate::~XYCorrelationCurvePrivate() {
	//no need to delete xColumn and yColumn, they are deleted
	//when the parent aspect is removed
}

void XYCorrelationCurvePrivate::recalculate() {
	QElapsedTimer timer;
	timer.start();

	//create correlation result columns if not available yet, clear them otherwise
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

	//clear the previous result
	correlationResult = XYCorrelationCurve::CorrelationResult();

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
		//TODO: where to get second y-column?
		//tmpY2DataColumn = dataSourceCurve->y2Column();
	}

	if (tmpYDataColumn == nullptr || tmpY2DataColumn == nullptr) {
		emit q->dataChanged();
		sourceDataChangedSinceLastRecalc = false;
		return;
	}

	//copy all valid data point for the correlation to temporary vectors
	QVector<double> xdataVector;
	QVector<double> ydataVector;
	QVector<double> y2dataVector;

	double xmin, xmax;
	if (tmpXDataColumn != nullptr && correlationData.autoRange) {
		xmin = tmpXDataColumn->minimum();
		xmax = tmpXDataColumn->maximum();
	} else {
		xmin = correlationData.xRange.first();
		xmax = correlationData.xRange.last();
	}

	//only copy those data where values are valid
	if (tmpXDataColumn != nullptr) {	// x-axis present (with possible range)
		for (int row = 0; row < tmpXDataColumn->rowCount(); ++row) {
			if (!std::isnan(tmpXDataColumn->valueAt(row)) && !tmpXDataColumn->isMasked(row)
				&& !std::isnan(tmpYDataColumn->valueAt(row)) && !tmpYDataColumn->isMasked(row)) {
				if (tmpXDataColumn->valueAt(row) >= xmin && tmpXDataColumn->valueAt(row) <= xmax) {
					xdataVector.append(tmpXDataColumn->valueAt(row));
					ydataVector.append(tmpYDataColumn->valueAt(row));
				}
			}
		}
	} else {	// no x-axis: take all valid values
		for (int row = 0; row < tmpYDataColumn->rowCount(); ++row)
			if (!std::isnan(tmpYDataColumn->valueAt(row)) && !tmpYDataColumn->isMasked(row))
				ydataVector.append(tmpYDataColumn->valueAt(row));
	}

	if (tmpY2DataColumn != nullptr) {
		for (int row = 0; row < tmpY2DataColumn->rowCount(); ++row)
			if (!std::isnan(tmpY2DataColumn->valueAt(row)) && !tmpY2DataColumn->isMasked(row))
				y2dataVector.append(tmpY2DataColumn->valueAt(row));
	}

	const size_t n = (size_t)ydataVector.size();	// number of points for signal
	const size_t m = (size_t)y2dataVector.size();	// number of points for response
	if (n < 1 || m < 1) {
		correlationResult.available = true;
		correlationResult.valid = false;
		correlationResult.status = i18n("Not enough data points available.");
		emit q->dataChanged();
		sourceDataChangedSinceLastRecalc = false;
		return;
	}

	double* xdata = xdataVector.data();
	double* ydata = ydataVector.data();
	double* y2data = y2dataVector.data();

	// correlation settings
	const double samplingInterval = correlationData.samplingInterval;
	const nsl_corr_type_type type = correlationData.type;
	const nsl_corr_norm_type norm = correlationData.normalize;

	DEBUG("signal n = " << n << ", response m = " << m);
	DEBUG("sampling interval = " << samplingInterval);
	DEBUG("type = " << nsl_corr_type_name[type]);
	DEBUG("norm = " << nsl_corr_norm_name[norm]);

///////////////////////////////////////////////////////////
	size_t np;
	if (type == nsl_corr_type_linear)
		np = n + m - 1;
	else
		np = GSL_MAX(n, m);

	double* out = (double*)malloc(np * sizeof(double));
	int status = nsl_corr_correlation(ydata, n, y2data, m, type, norm, out);

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
	correlationResult.available = true;
	correlationResult.valid = true;
	correlationResult.status = QString::number(status);
	correlationResult.elapsedTime = timer.elapsed();

	//redraw the curve
	emit q->dataChanged();
	sourceDataChangedSinceLastRecalc = false;
}

//##############################################################################
//##################  Serialization/Deserialization  ###########################
//##############################################################################
//! Save as XML
void XYCorrelationCurve::save(QXmlStreamWriter* writer) const{
	Q_D(const XYCorrelationCurve);

	writer->writeStartElement("xyCorrelationCurve");

	//write the base class
	XYAnalysisCurve::save(writer);

	//write xy-correlation-curve specific information
	// correlation data
	writer->writeStartElement("correlationData");
	writer->writeAttribute( "samplingInterval", QString::number(d->correlationData.samplingInterval) );
	writer->writeAttribute( "autoRange", QString::number(d->correlationData.autoRange) );
	writer->writeAttribute( "xRangeMin", QString::number(d->correlationData.xRange.first()) );
	writer->writeAttribute( "xRangeMax", QString::number(d->correlationData.xRange.last()) );
	writer->writeAttribute( "type", QString::number(d->correlationData.type) );
	writer->writeAttribute( "normalize", QString::number(d->correlationData.normalize) );
	writer->writeEndElement();// correlationData

	// correlation results (generated columns)
	writer->writeStartElement("correlationResult");
	writer->writeAttribute( "available", QString::number(d->correlationResult.available) );
	writer->writeAttribute( "valid", QString::number(d->correlationResult.valid) );
	writer->writeAttribute( "status", d->correlationResult.status );
	writer->writeAttribute( "time", QString::number(d->correlationResult.elapsedTime) );

	//save calculated columns if available
	if (d->xColumn) {
		d->xColumn->save(writer);
		d->yColumn->save(writer);
	}
	writer->writeEndElement(); //"correlationResult"

	writer->writeEndElement(); //"xyCorrelationCurve"
}

//! Load from XML
bool XYCorrelationCurve::load(XmlStreamReader* reader, bool preview) {
	DEBUG("XYCorrelationCurve::load()");
	Q_D(XYCorrelationCurve);

	KLocalizedString attributeWarning = ki18n("Attribute '%1' missing or empty, default value is used");
	QXmlStreamAttributes attribs;
	QString str;

	while (!reader->atEnd()) {
		reader->readNext();
		if (reader->isEndElement() && reader->name() == "xyCorrelationCurve")
			break;

		if (!reader->isStartElement())
			continue;

		if (reader->name() == "xyAnalysisCurve") {
			if ( !XYAnalysisCurve::load(reader, preview) )
				return false;
		} else if (!preview && reader->name() == "correlationData") {
			attribs = reader->attributes();
			READ_DOUBLE_VALUE("samplingInterval", correlationData.samplingInterval);
			READ_INT_VALUE("autoRange", correlationData.autoRange, bool);
			READ_DOUBLE_VALUE("xRangeMin", correlationData.xRange.first());
			READ_DOUBLE_VALUE("xRangeMax", correlationData.xRange.last());
			READ_INT_VALUE("type", correlationData.type, nsl_corr_type_type);
			READ_INT_VALUE("normalize", correlationData.normalize, nsl_corr_norm_type);
		} else if (!preview && reader->name() == "correlationResult") {
			attribs = reader->attributes();
			READ_INT_VALUE("available", correlationResult.available, int);
			READ_INT_VALUE("valid", correlationResult.valid, int);
			READ_STRING_VALUE("status", correlationResult.status);
			READ_INT_VALUE("time", correlationResult.elapsedTime, int);
		} else if (!preview && reader->name() == "column") {
			Column* column = new Column("", AbstractColumn::Numeric);
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

		setUndoAware(false);
		XYCurve::d_ptr->xColumn = d->xColumn;
		XYCurve::d_ptr->yColumn = d->yColumn;
		setUndoAware(true);
	}

	return true;
}
