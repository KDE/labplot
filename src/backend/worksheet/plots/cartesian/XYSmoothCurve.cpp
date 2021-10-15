/*
    File                 : XYSmoothCurve.cpp
    Project              : LabPlot
    Description          : A xy-curve defined by a smooth
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2016 Stefan Gerlach <stefan.gerlach@uni.kn>
    SPDX-FileCopyrightText: 2017 Alexander Semke <alexander.semke@web.de>
    SPDX-License-Identifier: GPL-2.0-or-later
*/


/*!
  \class XYSmoothCurve
  \brief A xy-curve defined by a smooth

  \ingroup worksheet
*/

#include "XYSmoothCurve.h"
#include "XYSmoothCurvePrivate.h"
#include "backend/core/column/Column.h"
#include "backend/lib/commandtemplates.h"
#include "backend/lib/macros.h"
#include "backend/lib/XmlStreamReader.h"

#include <KLocalizedString>

#include <QIcon>
#include <QElapsedTimer>
#include <QThreadPool>

extern "C" {
#include <gsl/gsl_math.h>	// gsl_pow_*
#include "backend/nsl/nsl_stats.h"
#include "backend/nsl/nsl_sf_kernel.h"
}

XYSmoothCurve::XYSmoothCurve(const QString& name)
	: XYAnalysisCurve(name, new XYSmoothCurvePrivate(this), AspectType::XYSmoothCurve) {
}

XYSmoothCurve::XYSmoothCurve(const QString& name, XYSmoothCurvePrivate* dd)
	: XYAnalysisCurve(name, dd, AspectType::XYSmoothCurve) {
}

//no need to delete the d-pointer here - it inherits from QGraphicsItem
//and is deleted during the cleanup in QGraphicsScene
XYSmoothCurve::~XYSmoothCurve() = default;

void XYSmoothCurve::recalculate() {
	Q_D(XYSmoothCurve);
	d->recalculate();
}

/*!
	Returns an icon to be used in the project explorer.
*/
QIcon XYSmoothCurve::icon() const {
	return QIcon::fromTheme("labplot-xy-smoothing-curve");
}

//##############################################################################
//##########################  getter methods  ##################################
//##############################################################################
BASIC_SHARED_D_READER_IMPL(XYSmoothCurve, XYSmoothCurve::SmoothData, smoothData, smoothData)

const XYSmoothCurve::SmoothResult& XYSmoothCurve::smoothResult() const {
	Q_D(const XYSmoothCurve);
	return d->smoothResult;
}

//##############################################################################
//#################  setter methods and undo commands ##########################
//##############################################################################
STD_SETTER_CMD_IMPL_F_S(XYSmoothCurve, SetSmoothData, XYSmoothCurve::SmoothData, smoothData, recalculate);
void XYSmoothCurve::setSmoothData(const XYSmoothCurve::SmoothData& smoothData) {
	Q_D(XYSmoothCurve);
	exec(new XYSmoothCurveSetSmoothDataCmd(d, smoothData, ki18n("%1: set options and perform the smooth")));
}

//##############################################################################
//######################### Private implementation #############################
//##############################################################################
XYSmoothCurvePrivate::XYSmoothCurvePrivate(XYSmoothCurve* owner) : XYAnalysisCurvePrivate(owner), q(owner)  {
}

//no need to delete xColumn and yColumn, they are deleted
//when the parent aspect is removed
XYSmoothCurvePrivate::~XYSmoothCurvePrivate() = default;

void XYSmoothCurvePrivate::recalculate() {
	DEBUG(Q_FUNC_INFO)
	QElapsedTimer timer;
	timer.start();

	//create smooth result columns if not available yet, clear them otherwise
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
		if (roughVector)
			roughVector->clear();
	}

	if (!roughColumn) {
		roughColumn = new Column("rough", AbstractColumn::ColumnMode::Numeric);
		roughVector = static_cast<QVector<double>* >(roughColumn->data());
		q->addChild(roughColumn);
	}

	// clear the previous result
	smoothResult = XYSmoothCurve::SmoothResult();

	//determine the data source columns
	const AbstractColumn* tmpXDataColumn = nullptr;
	const AbstractColumn* tmpYDataColumn = nullptr;
	if (dataSourceType == XYAnalysisCurve::DataSourceType::Spreadsheet) {
		//spreadsheet columns as data source
		tmpXDataColumn = xDataColumn;
		tmpYDataColumn = yDataColumn;
	} else {
		//curve columns as data source
		tmpXDataColumn = dataSourceCurve->xColumn();
		tmpYDataColumn = dataSourceCurve->yColumn();
	}

	if (!tmpXDataColumn || !tmpYDataColumn) {
		emit q->dataChanged();
		sourceDataChangedSinceLastRecalc = false;
		return;
	}

	//check column sizes
	if (tmpXDataColumn->rowCount() != tmpYDataColumn->rowCount()) {
		smoothResult.available = true;
		smoothResult.valid = false;
		smoothResult.status = i18n("Number of x and y data points must be equal.");
		recalcLogicalPoints();
		emit q->dataChanged();
		sourceDataChangedSinceLastRecalc = false;
		return;
	}

	//copy all valid data point for the smooth to temporary vectors
	QVector<double> xdataVector;
	QVector<double> ydataVector;

	double xmin;
	double xmax;
	if (smoothData.autoRange) {
		xmin = tmpXDataColumn->minimum();
		xmax = tmpXDataColumn->maximum();
	} else {
		xmin = smoothData.xRange.first();
		xmax = smoothData.xRange.last();
	}

	XYAnalysisCurve::copyData(xdataVector, ydataVector, tmpXDataColumn, tmpYDataColumn, xmin, xmax);

	//number of data points to smooth
	const size_t n = (size_t)xdataVector.size();
	if (n < 2) {
		smoothResult.available = true;
		smoothResult.valid = false;
		smoothResult.status = i18n("Not enough data points available.");
		recalcLogicalPoints();
		emit q->dataChanged();
		sourceDataChangedSinceLastRecalc = false;
		return;
	}

	double* xdata = xdataVector.data();
	double* ydata = ydataVector.data();

	double* ydataOriginal = new double[n];
	memcpy(ydataOriginal, ydata, n*sizeof(double));

	// smooth settings
	const nsl_smooth_type type = smoothData.type;
	const size_t points = smoothData.points;
	const nsl_smooth_weight_type weight = smoothData.weight;
	const double percentile = smoothData.percentile;
	const int order = smoothData.order;
	const nsl_smooth_pad_mode padMode = smoothData.mode;
	const double lvalue = smoothData.lvalue;
	const double rvalue = smoothData.rvalue;

	DEBUG("	smooth type:" << nsl_smooth_type_name[type]);
	DEBUG("	points = " << points);
	DEBUG("	weight: " << nsl_smooth_weight_type_name[weight]);
	DEBUG("	percentile = " << percentile);
	DEBUG("	order = " << order);
	DEBUG("	pad mode =	" << nsl_smooth_pad_mode_name[padMode]);
	DEBUG("	const. values = " << lvalue << ' ' << rvalue);

///////////////////////////////////////////////////////////
	int status = 0;

	switch (type) {
	case nsl_smooth_type_moving_average:
		status = nsl_smooth_moving_average(ydata, n, points, weight, padMode);
		break;
	case nsl_smooth_type_moving_average_lagged:
		status = nsl_smooth_moving_average_lagged(ydata, n, points, weight, padMode);
		break;
	case nsl_smooth_type_percentile:
		status = nsl_smooth_percentile(ydata, n, points, percentile, padMode);
		break;
	case nsl_smooth_type_savitzky_golay:
		if (padMode == nsl_smooth_pad_constant)
			nsl_smooth_pad_constant_set(lvalue, rvalue);
		status = nsl_smooth_savgol(ydata, n, points, order, padMode);
		break;
	}

	xVector->resize((int)n);
	yVector->resize((int)n);
	memcpy(xVector->data(), xdata, n*sizeof(double));
	memcpy(yVector->data(), ydata, n*sizeof(double));
///////////////////////////////////////////////////////////

	//write the result
	smoothResult.available = true;
	smoothResult.valid = true;
	smoothResult.status = QString::number(status);
	smoothResult.elapsedTime = timer.elapsed();

	//fill rough vector
	roughVector->resize((int)n);
	for (int i = 0; i < (int)n; ++i)
		roughVector->data()[i] = ydataOriginal[i] - ydata[i];
	roughColumn->setChanged();

	delete [] ydataOriginal;

	//redraw the curve
	recalcLogicalPoints();
	emit q->dataChanged();
	sourceDataChangedSinceLastRecalc = false;
}

//##############################################################################
//##################  Serialization/Deserialization  ###########################
//##############################################################################
//! Save as XML
void XYSmoothCurve::save(QXmlStreamWriter* writer) const{
	Q_D(const XYSmoothCurve);

	writer->writeStartElement("xySmoothCurve");

	//write the base class
	XYAnalysisCurve::save(writer);

	//write xy-smooth-curve specific information
	// smooth data
	writer->writeStartElement("smoothData");
	writer->writeAttribute( "autoRange", QString::number(d->smoothData.autoRange) );
	writer->writeAttribute( "xRangeMin", QString::number(d->smoothData.xRange.first()) );
	writer->writeAttribute( "xRangeMax", QString::number(d->smoothData.xRange.last()) );
	writer->writeAttribute( "type", QString::number(d->smoothData.type) );
	writer->writeAttribute( "points", QString::number(d->smoothData.points) );
	writer->writeAttribute( "weight", QString::number(d->smoothData.weight) );
	writer->writeAttribute( "percentile", QString::number(d->smoothData.percentile) );
	writer->writeAttribute( "order", QString::number(d->smoothData.order) );
	writer->writeAttribute( "mode", QString::number(d->smoothData.mode) );
	writer->writeAttribute( "lvalue", QString::number(d->smoothData.lvalue) );
	writer->writeAttribute( "rvalue", QString::number(d->smoothData.rvalue) );
	writer->writeEndElement();// smoothData

	// smooth results (generated columns)
	writer->writeStartElement("smoothResult");
	writer->writeAttribute( "available", QString::number(d->smoothResult.available) );
	writer->writeAttribute( "valid", QString::number(d->smoothResult.valid) );
	writer->writeAttribute( "status", d->smoothResult.status );
	writer->writeAttribute( "time", QString::number(d->smoothResult.elapsedTime) );

	//save calculated columns if available
	if (saveCalculations() && d->xColumn) {
		d->xColumn->save(writer);
		d->yColumn->save(writer);
	}

	if (d->roughColumn)
		d->roughColumn->save(writer);

	writer->writeEndElement(); //"smoothResult"

	writer->writeEndElement(); //"xySmoothCurve"
}

//! Load from XML
bool XYSmoothCurve::load(XmlStreamReader* reader, bool preview) {
	Q_D(XYSmoothCurve);

	KLocalizedString attributeWarning = ki18n("Attribute '%1' missing or empty, default value is used");
	QXmlStreamAttributes attribs;
	QString str;

	while (!reader->atEnd()) {
		reader->readNext();
		if (reader->isEndElement() && reader->name() == "xySmoothCurve")
			break;

		if (!reader->isStartElement())
			continue;

		if (reader->name() == "xyAnalysisCurve") {
			if ( !XYAnalysisCurve::load(reader, preview) )
				return false;
		} else if (!preview && reader->name() == "smoothData") {
			attribs = reader->attributes();
			READ_INT_VALUE("autoRange", smoothData.autoRange, bool);
			READ_DOUBLE_VALUE("xRangeMin", smoothData.xRange.first());
			READ_DOUBLE_VALUE("xRangeMax", smoothData.xRange.last());
			READ_INT_VALUE("type", smoothData.type, nsl_smooth_type);
			READ_INT_VALUE("points", smoothData.points, size_t);
			READ_INT_VALUE("weight", smoothData.weight, nsl_smooth_weight_type);
			READ_DOUBLE_VALUE("percentile", smoothData.percentile);
			READ_INT_VALUE("order", smoothData.order, int);
			READ_INT_VALUE("mode", smoothData.mode, nsl_smooth_pad_mode);
			READ_DOUBLE_VALUE("lvalue", smoothData.lvalue);
			READ_DOUBLE_VALUE("rvalue", smoothData.rvalue);
		} else if (!preview && reader->name() == "smoothResult") {
			attribs = reader->attributes();
			READ_INT_VALUE("available", smoothResult.available, int);
			READ_INT_VALUE("valid", smoothResult.valid, int);
			READ_STRING_VALUE("status", smoothResult.status);
			READ_INT_VALUE("time", smoothResult.elapsedTime, int);
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
			else
				d->roughColumn = column;
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

	if (d->roughColumn) {
		addChild(d->roughColumn);
		d->roughVector = static_cast<QVector<double>* >(d->roughColumn->data());
	}

	return true;
}
