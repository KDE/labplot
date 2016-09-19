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
#include "CartesianCoordinateSystem.h"
#include "backend/core/column/Column.h"
#include "backend/lib/commandtemplates.h"

#include <cmath>	// isnan
#include <cfloat>	// DBL_MIN
extern "C" {
#include <gsl/gsl_errno.h>
#include <gsl/gsl_interp.h>
#include <gsl/gsl_spline.h>
#include "backend/nsl/nsl_diff.h"
}

#include <KLocale>
#include <QElapsedTimer>
#include <QThreadPool>
#include <QIcon>
#ifndef NDEBUG
#include <QDebug>
#endif

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
	return QIcon::fromTheme("labplot-xy-interpolation-curve");
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

	//copy all valid data point for the interpolation to temporary vectors
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
		interpolationResult.available = true;
		interpolationResult.valid = false;
		interpolationResult.status = i18n("Not enough data points available.");
		emit (q->dataChanged());
		sourceDataChangedSinceLastInterpolation = false;
		return;
	}

	double* xdata = xdataVector.data();
	double* ydata = ydataVector.data();

	const double min = xDataColumn->minimum();
	const double max = xDataColumn->maximum();

	// interpolation settings
	const nsl_interp_type type = interpolationData.type;
	const nsl_interp_pch_variant variant = interpolationData.variant;
	const double tension = interpolationData.tension;
	const double continuity = interpolationData.continuity;
	const double bias = interpolationData.bias;
	const nsl_interp_evaluate evaluate = interpolationData.evaluate;
	const unsigned int npoints = interpolationData.npoints;
#ifndef NDEBUG
	qDebug()<<"type:"<<nsl_interp_type_name[type];
	qDebug()<<"cubic Hermite variant:"<<nsl_interp_pch_variant_name[variant]<<tension<<continuity<<bias;
	qDebug()<<"evaluate:"<<nsl_interp_evaluate_name[evaluate];
	qDebug()<<"npoints ="<<npoints;
#endif
///////////////////////////////////////////////////////////
	int status=0;

	gsl_interp_accel *acc = gsl_interp_accel_alloc();
	gsl_spline *spline=0;
	switch (type) {
	case nsl_interp_type_linear:
		spline = gsl_spline_alloc(gsl_interp_linear, n);
		status = gsl_spline_init(spline, xdata, ydata, n);
		break;
	case nsl_interp_type_polynomial:
		spline = gsl_spline_alloc(gsl_interp_polynomial, n);
		status = gsl_spline_init(spline, xdata, ydata, n);
		break;
	case nsl_interp_type_cspline:
		spline = gsl_spline_alloc(gsl_interp_cspline, n);
		status = gsl_spline_init(spline, xdata, ydata, n);
		break;
	case nsl_interp_type_cspline_periodic:
		spline = gsl_spline_alloc(gsl_interp_cspline_periodic, n);
		status = gsl_spline_init(spline, xdata, ydata, n);
		break;
	case nsl_interp_type_akima:
		spline = gsl_spline_alloc(gsl_interp_akima, n);
		status = gsl_spline_init(spline, xdata, ydata, n);
		break;
	case nsl_interp_type_akima_periodic:
		spline = gsl_spline_alloc(gsl_interp_akima_periodic, n);
		status = gsl_spline_init(spline, xdata, ydata, n);
		break;
	case nsl_interp_type_steffen:
#if GSL_MAJOR_VERSION >= 2
		spline = gsl_spline_alloc(gsl_interp_steffen, n);
		status = gsl_spline_init(spline, xdata, ydata, n);
#endif
		break;
	default:
		break;
	}

	xVector->resize(npoints);
	yVector->resize(npoints);
	for (unsigned int i = 0; i < npoints; i++) {
		unsigned int a=0,b=n-1;

		double x = min + i*(max-min)/(npoints-1);
		(*xVector)[i] = x;

		// find index a,b for interval [x[a],x[b]] around x[i] using bisection
		int j=0;
		switch (type) {
		case nsl_interp_type_cosine:
		case nsl_interp_type_exponential:
		case nsl_interp_type_pch:
			while (b-a > 1) {
				j=floor((a+b)/2.);
				if (xdata[j] > x)
					b=j;
				else
					a=j;
			}
			break;
		default:
			break;
		}

		// evaluate interpolation
		double t;
		switch (type) {
		case nsl_interp_type_linear:
		case nsl_interp_type_polynomial:
		case nsl_interp_type_cspline:
		case nsl_interp_type_cspline_periodic:
		case nsl_interp_type_akima:
		case nsl_interp_type_akima_periodic:
		case nsl_interp_type_steffen:
			switch (evaluate) {
			case nsl_interp_evaluate_function:
				(*yVector)[i] = gsl_spline_eval(spline, x, acc);
				break;
			case nsl_interp_evaluate_derivative:
				(*yVector)[i] = gsl_spline_eval_deriv(spline, x, acc);
				break;
			case nsl_interp_evaluate_second_derivative:
				(*yVector)[i] = gsl_spline_eval_deriv2(spline, x, acc);
				break;
			case nsl_interp_evaluate_integral:
				(*yVector)[i] = gsl_spline_eval_integ(spline, min, x, acc);
				break;
			}
			break;
		case nsl_interp_type_cosine:
			t = (x-xdata[a])/(xdata[b]-xdata[a]);
			t = (1.-cos(M_PI*t))/2.;
			(*yVector)[i] =  ydata[a] + t*(ydata[b]-ydata[a]);
			break;
		case nsl_interp_type_exponential:
			t = (x-xdata[a])/(xdata[b]-xdata[a]);
			(*yVector)[i] = ydata[a]*pow(ydata[b]/ydata[a],t);
			break;
		case nsl_interp_type_pch: {
			t = (x-xdata[a])/(xdata[b]-xdata[a]);
			double t2=t*t, t3=t2*t;
			double h1=2.*t3-3.*t2+1, h2=-2.*t3+3.*t2, h3=t3-2*t2+t, h4=t3-t2;
			double m1=0.,m2=0.;
			switch (variant) {
			case nsl_interp_pch_variant_finite_difference:
				if (a==0)
					m1=(ydata[b]-ydata[a])/(xdata[b]-xdata[a]);
				else
					m1=( (ydata[b]-ydata[a])/(xdata[b]-xdata[a]) + (ydata[a]-ydata[a-1])/(xdata[a]-xdata[a-1]) )/2.;
				if (b==n-1)
					m2=(ydata[b]-ydata[a])/(xdata[b]-xdata[a]);
				else
					m2=( (ydata[b+1]-ydata[b])/(xdata[b+1]-xdata[b]) + (ydata[b]-ydata[a])/(xdata[b]-xdata[a]) )/2.;

				break;
			case nsl_interp_pch_variant_catmull_rom:
				if (a==0)
					m1=(ydata[b]-ydata[a])/(xdata[b]-xdata[a]);
				else
					m1=(ydata[b]-ydata[a-1])/(xdata[b]-xdata[a-1]);
				if (b==n-1)
					m2=(ydata[b]-ydata[a])/(xdata[b]-xdata[a]);
				else
					m2=(ydata[b+1]-ydata[a])/(xdata[b+1]-xdata[a]);

				break;
			case nsl_interp_pch_variant_cardinal:
				if (a==0)
					m1=(ydata[b]-ydata[a])/(xdata[b]-xdata[a]);
				else
					m1=(ydata[b]-ydata[a-1])/(xdata[b]-xdata[a-1]);
				m1 *= (1.-tension);
				if (b==n-1)
					m2=(ydata[b]-ydata[a])/(xdata[b]-xdata[a]);
				else
					m2=(ydata[b+1]-ydata[a])/(xdata[b+1]-xdata[a]);
				m2 *= (1.-tension);

				break;
			case nsl_interp_pch_variant_kochanek_bartels:
				if (a==0)
					m1=(1.+continuity)*(1.-bias)*(ydata[b]-ydata[a])/(xdata[b]-xdata[a]);
				else
					m1=( (1.-continuity)*(1.+bias)*(ydata[a]-ydata[a-1])/(xdata[a]-xdata[a-1]) 
						+ (1.+continuity)*(1.-bias)*(ydata[b]-ydata[a])/(xdata[b]-xdata[a]) )/2.;
				m1 *= (1.-tension);
				if (b==n-1)
					m2=(1.+continuity)*(1.+bias)*(ydata[b]-ydata[a])/(xdata[b]-xdata[a]);
				else
					m2=( (1.+continuity)*(1.+bias)*(ydata[b]-ydata[a])/(xdata[b]-xdata[a]) 
						+ (1.-continuity)*(1.-bias)*(ydata[b+1]-ydata[b])/(xdata[b+1]-xdata[b]) )/2.;
				m2 *= (1.-tension);
				
				break;
			}	

			// Hermite polynomial
			(*yVector)[i] = ydata[a]*h1+ydata[b]*h2+(xdata[b]-xdata[a])*(m1*h3+m2*h4);
		}
			break;
		case nsl_interp_type_rational: {
			double v,dv;
			nsl_interp_ratint(xdata, ydata, n, x, &v, &dv);
			(*yVector)[i] = v;
			//TODO: use error dv
			break;
		}
		}
	}

	// calculate "evaluate" option for own types
	switch (type) {
	case nsl_interp_type_cosine:
	case nsl_interp_type_exponential:
	case nsl_interp_type_pch:
	case nsl_interp_type_rational:
		switch (evaluate) {
		case nsl_interp_evaluate_function:
			break;
		case nsl_interp_evaluate_derivative:
			nsl_diff_deriv_first(xVector->data(), yVector->data(), npoints);
			break;
		case nsl_interp_evaluate_second_derivative:
			nsl_diff_deriv_second(xVector->data(), yVector->data(), npoints);
			break;
		case nsl_interp_evaluate_integral:
			nsl_interp_integral(xVector->data(), yVector->data(), npoints);
			break;
		}
		break;
	default:
		break;
	}

	// check values
	for (unsigned int i = 0; i < npoints; i++) {
		if ((*yVector)[i] > CartesianScale::LIMIT_MAX)
			(*yVector)[i] = CartesianScale::LIMIT_MAX;
		else if ((*yVector)[i] < CartesianScale::LIMIT_MIN)
			(*yVector)[i] = CartesianScale::LIMIT_MIN;
	}

	gsl_spline_free(spline);
	gsl_interp_accel_free(acc);

///////////////////////////////////////////////////////////

	//write the result
	interpolationResult.available = true;
	interpolationResult.valid = true;
	interpolationResult.status = QString(gsl_strerror(status));;
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
	writer->writeAttribute( "type", QString::number(d->interpolationData.type) );
	writer->writeAttribute( "variant", QString::number(d->interpolationData.variant) );
	writer->writeAttribute( "tension", QString::number(d->interpolationData.tension) );
	writer->writeAttribute( "continuity", QString::number(d->interpolationData.continuity) );
	writer->writeAttribute( "bias", QString::number(d->interpolationData.bias) );
	writer->writeAttribute( "npoints", QString::number(d->interpolationData.npoints) );
	writer->writeAttribute( "pointsMode", QString::number(d->interpolationData.pointsMode) );
	writer->writeAttribute( "evaluate", QString::number(d->interpolationData.evaluate) );
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
bool XYInterpolationCurve::load(XmlStreamReader* reader) {
	Q_D(XYInterpolationCurve);

	if (!reader->isStartElement() || reader->name() != "xyInterpolationCurve") {
		reader->raiseError(i18n("no xy interpolation curve element found"));
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

			str = attribs.value("type").toString();
			if (str.isEmpty())
				reader->raiseWarning(attributeWarning.arg("'type'"));
			else
				d->interpolationData.type = (nsl_interp_type) str.toInt();

			str = attribs.value("variant").toString();
			if (str.isEmpty())
				reader->raiseWarning(attributeWarning.arg("'variant'"));
			else
				d->interpolationData.variant = (nsl_interp_pch_variant) str.toInt();

			str = attribs.value("tension").toString();
			if (str.isEmpty())
				reader->raiseWarning(attributeWarning.arg("'tension'"));
			else
				d->interpolationData.tension = str.toDouble();

			str = attribs.value("continuity").toString();
			if (str.isEmpty())
				reader->raiseWarning(attributeWarning.arg("'continuity'"));
			else
				d->interpolationData.continuity = str.toDouble();

			str = attribs.value("bias").toString();
			if (str.isEmpty())
				reader->raiseWarning(attributeWarning.arg("'bias'"));
			else
				d->interpolationData.bias = str.toDouble();

			str = attribs.value("npoints").toString();
			if (str.isEmpty())
				reader->raiseWarning(attributeWarning.arg("'npoints'"));
			else
				d->interpolationData.npoints = str.toInt();

			str = attribs.value("pointsMode").toString();
			if (str.isEmpty())
				reader->raiseWarning(attributeWarning.arg("'pointsMode'"));
			else
				d->interpolationData.pointsMode = (XYInterpolationCurve::PointsMode)str.toInt();

			str = attribs.value("evaluate").toString();
			if (str.isEmpty())
				reader->raiseWarning(attributeWarning.arg("'evaluate'"));
			else
				d->interpolationData.evaluate = (nsl_interp_evaluate)str.toInt();

		} else if (reader->name() == "interpolationResult") {

			attribs = reader->attributes();

			str = attribs.value("available").toString();
			if (str.isEmpty())
				reader->raiseWarning(attributeWarning.arg("'available'"));
			else
				d->interpolationResult.available = str.toInt();

			str = attribs.value("valid").toString();
			if (str.isEmpty())
				reader->raiseWarning(attributeWarning.arg("'valid'"));
			else
				d->interpolationResult.valid = str.toInt();
			
			str = attribs.value("status").toString();
			if (str.isEmpty())
				reader->raiseWarning(attributeWarning.arg("'status'"));
			else
				d->interpolationResult.status = str;

			str = attribs.value("time").toString();
			if (str.isEmpty())
				reader->raiseWarning(attributeWarning.arg("'time'"));
			else
				d->interpolationResult.elapsedTime = str.toInt();
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
