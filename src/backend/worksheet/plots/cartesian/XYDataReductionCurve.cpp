/***************************************************************************
    File                 : XYDataReductionCurve.cpp
    Project              : LabPlot
    Description          : A xy-curve defined by a data reduction
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
  \class XYDataReductionCurve
  \brief A xy-curve defined by a data reduction

  \ingroup worksheet
*/

#include "XYDataReductionCurve.h"
#include "XYDataReductionCurvePrivate.h"
#include "CartesianCoordinateSystem.h"
#include "backend/core/column/Column.h"
#include "backend/lib/commandtemplates.h"
#include "backend/lib/macros.h"

#include <KLocalizedString>
#include <QIcon>
#include <QElapsedTimer>
#include <QThreadPool>

XYDataReductionCurve::XYDataReductionCurve(const QString& name)
	: XYAnalysisCurve(name, new XYDataReductionCurvePrivate(this), AspectType::XYDataReductionCurve) {
}

XYDataReductionCurve::XYDataReductionCurve(const QString& name, XYDataReductionCurvePrivate* dd)
	: XYAnalysisCurve(name, dd, AspectType::XYDataReductionCurve) {
}

//no need to delete the d-pointer here - it inherits from QGraphicsItem
//and is deleted during the cleanup in QGraphicsScene
XYDataReductionCurve::~XYDataReductionCurve() = default;

void XYDataReductionCurve::recalculate() {
	Q_D(XYDataReductionCurve);
	d->recalculate();
}

/*!
	Returns an icon to be used in the project explorer.
*/
QIcon XYDataReductionCurve::icon() const {
	return QIcon::fromTheme("labplot-xy-data-reduction-curve");
}

//##############################################################################
//##########################  getter methods  ##################################
//##############################################################################
BASIC_SHARED_D_READER_IMPL(XYDataReductionCurve, XYDataReductionCurve::DataReductionData, dataReductionData, dataReductionData)

const XYDataReductionCurve::DataReductionResult& XYDataReductionCurve::dataReductionResult() const {
	Q_D(const XYDataReductionCurve);
	return d->dataReductionResult;
}

//##############################################################################
//#################  setter methods and undo commands ##########################
//##############################################################################
STD_SETTER_CMD_IMPL_F_S(XYDataReductionCurve, SetDataReductionData, XYDataReductionCurve::DataReductionData, dataReductionData, recalculate);
void XYDataReductionCurve::setDataReductionData(const XYDataReductionCurve::DataReductionData& reductionData) {
	Q_D(XYDataReductionCurve);
	exec(new XYDataReductionCurveSetDataReductionDataCmd(d, reductionData, ki18n("%1: set options and perform the data reduction")));
}

//##############################################################################
//######################### Private implementation #############################
//##############################################################################
XYDataReductionCurvePrivate::XYDataReductionCurvePrivate(XYDataReductionCurve* owner) : XYAnalysisCurvePrivate(owner), q(owner)  {
}

//no need to delete xColumn and yColumn, they are deleted
//when the parent aspect is removed
XYDataReductionCurvePrivate::~XYDataReductionCurvePrivate() = default;

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

	//determine the data source columns
	const AbstractColumn* tmpXDataColumn = nullptr;
	const AbstractColumn* tmpYDataColumn = nullptr;
	if (dataSourceType == XYAnalysisCurve::DataSourceSpreadsheet) {
		//spreadsheet columns as data source
		tmpXDataColumn = xDataColumn;
		tmpYDataColumn = yDataColumn;
	} else {
		//curve columns as data source
		tmpXDataColumn = dataSourceCurve->xColumn();
		tmpYDataColumn = dataSourceCurve->yColumn();
	}

	if (!tmpXDataColumn || !tmpYDataColumn) {
		recalcLogicalPoints();
		emit q->dataChanged();
		sourceDataChangedSinceLastRecalc = false;
		return;
	}

	//check column sizes
	if (tmpXDataColumn->rowCount() != tmpYDataColumn->rowCount()) {
		dataReductionResult.available = true;
		dataReductionResult.valid = false;
		dataReductionResult.status = i18n("Number of x and y data points must be equal.");
		emit q->dataChanged();
		sourceDataChangedSinceLastRecalc = false;
		return;
	}

	//copy all valid data point for the data reduction to temporary vectors
	QVector<double> xdataVector;
	QVector<double> ydataVector;

	double xmin;
	double xmax;
	if (dataReductionData.autoRange) {
		xmin = tmpXDataColumn->minimum();
		xmax = tmpXDataColumn->maximum();
	} else {
		xmin = dataReductionData.xRange.first();
		xmax = dataReductionData.xRange.last();
	}

	for (int row = 0; row<tmpXDataColumn->rowCount(); ++row) {
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

	//number of data points to use
	const size_t n = (size_t)xdataVector.size();
	if (n < 2) {
		dataReductionResult.available = true;
		dataReductionResult.valid = false;
		dataReductionResult.status = i18n("Not enough data points available.");
		recalcLogicalPoints();
		emit q->dataChanged();
		sourceDataChangedSinceLastRecalc = false;
		return;
	}

	double* xdata = xdataVector.data();
	double* ydata = ydataVector.data();

	// dataReduction settings
	const nsl_geom_linesim_type type = dataReductionData.type;
	const double tol = dataReductionData.tolerance;
	const double tol2 = dataReductionData.tolerance2;

	DEBUG("n =" << n);
	DEBUG("type:" << nsl_geom_linesim_type_name[type]);
	DEBUG("tolerance/step:" << tol);
	DEBUG("tolerance2/repeat/maxtol/region:" << tol2);

///////////////////////////////////////////////////////////
	emit q->completed(10);

	size_t npoints = 0;
	double calcTolerance = 0;	// calculated tolerance from Douglas-Peucker variant
	size_t *index = (size_t *) malloc(n*sizeof(size_t));
	switch (type) {
	case nsl_geom_linesim_type_douglas_peucker_variant:	// tol used as number of points
		npoints = tol;
		calcTolerance = nsl_geom_linesim_douglas_peucker_variant(xdata, ydata, n, npoints, index);
		break;
	case nsl_geom_linesim_type_douglas_peucker:
		npoints = nsl_geom_linesim_douglas_peucker(xdata, ydata, n, tol, index);
		break;
	case nsl_geom_linesim_type_nthpoint:	// tol used as step
		npoints = nsl_geom_linesim_nthpoint(n, (int)tol, index);
		break;
	case nsl_geom_linesim_type_raddist:
		npoints = nsl_geom_linesim_raddist(xdata, ydata, n, tol, index);
		break;
	case nsl_geom_linesim_type_perpdist:	// tol2 used as repeat
		npoints = nsl_geom_linesim_perpdist_repeat(xdata, ydata, n, tol, tol2, index);
		break;
	case nsl_geom_linesim_type_interp:
		npoints = nsl_geom_linesim_interp(xdata, ydata, n, tol, index);
		break;
	case nsl_geom_linesim_type_visvalingam_whyatt:
		npoints = nsl_geom_linesim_visvalingam_whyatt(xdata, ydata, n, tol, index);
		break;
	case nsl_geom_linesim_type_reumann_witkam:
		npoints = nsl_geom_linesim_reumann_witkam(xdata, ydata, n, tol, index);
		break;
	case nsl_geom_linesim_type_opheim:
		npoints = nsl_geom_linesim_opheim(xdata, ydata, n, tol, tol2, index);
		break;
	case nsl_geom_linesim_type_lang:	// tol2 used as region
		npoints = nsl_geom_linesim_opheim(xdata, ydata, n, tol, tol2, index);
		break;
	}

	DEBUG("npoints =" << npoints);
	if (type == nsl_geom_linesim_type_douglas_peucker_variant)
		DEBUG("calculated tolerance =" << calcTolerance)
	else
		Q_UNUSED(calcTolerance);

	emit q->completed(80);

	xVector->resize((int)npoints);
	yVector->resize((int)npoints);
	for (int i = 0; i < (int)npoints; i++) {
		(*xVector)[i] = xdata[index[i]];
		(*yVector)[i] = ydata[index[i]];
	}

	emit q->completed(90);

	const double posError = nsl_geom_linesim_positional_squared_error(xdata, ydata, n, index);
	const double areaError = nsl_geom_linesim_area_error(xdata, ydata, n, index);

	free(index);

///////////////////////////////////////////////////////////

	//write the result
	dataReductionResult.available = true;
	dataReductionResult.valid = true;
	if (npoints > 0)
		dataReductionResult.status = QString("OK");
	else
		dataReductionResult.status = QString("FAILURE");
	dataReductionResult.elapsedTime = timer.elapsed();
	dataReductionResult.npoints = npoints;
	dataReductionResult.posError = posError;
	dataReductionResult.areaError = areaError;

	//redraw the curve
	recalcLogicalPoints();
	emit q->dataChanged();
	sourceDataChangedSinceLastRecalc = false;

	emit q->completed(100);
}

//##############################################################################
//##################  Serialization/Deserialization  ###########################
//##############################################################################
//! Save as XML
void XYDataReductionCurve::save(QXmlStreamWriter* writer) const{
	Q_D(const XYDataReductionCurve);

	writer->writeStartElement("xyDataReductionCurve");

	//write the base class
	XYAnalysisCurve::save(writer);

	//write xy-dataReduction-curve specific information
	// dataReduction data
	writer->writeStartElement("dataReductionData");
	writer->writeAttribute( "autoRange", QString::number(d->dataReductionData.autoRange) );
	writer->writeAttribute( "xRangeMin", QString::number(d->dataReductionData.xRange.first()) );
	writer->writeAttribute( "xRangeMax", QString::number(d->dataReductionData.xRange.last()) );
	writer->writeAttribute( "type", QString::number(d->dataReductionData.type) );
	writer->writeAttribute( "autoTolerance", QString::number(d->dataReductionData.autoTolerance) );
	writer->writeAttribute( "tolerance", QString::number(d->dataReductionData.tolerance) );
	writer->writeAttribute( "autoTolerance2", QString::number(d->dataReductionData.autoTolerance2) );
	writer->writeAttribute( "tolerance2", QString::number(d->dataReductionData.tolerance2) );
	writer->writeEndElement();// dataReductionData

	// dataReduction results (generated columns)
	writer->writeStartElement("dataReductionResult");
	writer->writeAttribute( "available", QString::number(d->dataReductionResult.available) );
	writer->writeAttribute( "valid", QString::number(d->dataReductionResult.valid) );
	writer->writeAttribute( "status", d->dataReductionResult.status );
	writer->writeAttribute( "time", QString::number(d->dataReductionResult.elapsedTime) );
	writer->writeAttribute( "npoints", QString::number(d->dataReductionResult.npoints) );
	writer->writeAttribute( "posError", QString::number(d->dataReductionResult.posError) );
	writer->writeAttribute( "areaError", QString::number(d->dataReductionResult.areaError) );

	//save calculated columns if available
	if (d->xColumn) {
		d->xColumn->save(writer);
		d->yColumn->save(writer);
	}
	writer->writeEndElement(); //"dataReductionResult"

	writer->writeEndElement(); //"xyDataReductionCurve"
}

//! Load from XML
bool XYDataReductionCurve::load(XmlStreamReader* reader, bool preview) {
	Q_D(XYDataReductionCurve);

	KLocalizedString attributeWarning = ki18n("Attribute '%1' missing or empty, default value is used");
	QXmlStreamAttributes attribs;
	QString str;

	while (!reader->atEnd()) {
		reader->readNext();
		if (reader->isEndElement() && reader->name() == "xyDataReductionCurve")
			break;

		if (!reader->isStartElement())
			continue;

		if (reader->name() == "xyAnalysisCurve") {
			if ( !XYAnalysisCurve::load(reader, preview) )
				return false;
		} else if (!preview && reader->name() == "dataReductionData") {
			attribs = reader->attributes();
			READ_INT_VALUE("autoRange", dataReductionData.autoRange, bool);
			READ_DOUBLE_VALUE("xRangeMin", dataReductionData.xRange.first());
			READ_DOUBLE_VALUE("xRangeMax", dataReductionData.xRange.last());
			READ_INT_VALUE("type", dataReductionData.type, nsl_geom_linesim_type);
			READ_INT_VALUE("autoTolerance", dataReductionData.autoTolerance, int);
			READ_DOUBLE_VALUE("tolerance", dataReductionData.tolerance);
			READ_INT_VALUE("autoTolerance2", dataReductionData.autoTolerance2, int);
			READ_DOUBLE_VALUE("tolerance2", dataReductionData.tolerance2);
		} else if (!preview && reader->name() == "dataReductionResult") {
			attribs = reader->attributes();
			READ_INT_VALUE("available", dataReductionResult.available, int);
			READ_INT_VALUE("valid", dataReductionResult.valid, int);
			READ_STRING_VALUE("status", dataReductionResult.status);
			READ_INT_VALUE("time", dataReductionResult.elapsedTime, int);
			READ_INT_VALUE("npoints", dataReductionResult.npoints, size_t);
			READ_DOUBLE_VALUE("posError", dataReductionResult.posError);
			READ_DOUBLE_VALUE("areaError", dataReductionResult.areaError);
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
