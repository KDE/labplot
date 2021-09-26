/*
    File                 : XYInterpolationCurve.cpp
    Project              : LabPlot
    Description          : A xy-curve defined by an interpolation
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2016-2021 Stefan Gerlach <stefan.gerlach@uni.kn>
    SPDX-FileCopyrightText: 2016-2017 Alexander Semke <alexander.semke@web.de>
    SPDX-License-Identifier: GPL-2.0-or-later
*/


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
#include "backend/lib/macros.h"
#include "backend/lib/XmlStreamReader.h"
#include "backend/gsl/errors.h"

extern "C" {
#include <gsl/gsl_interp.h>
#include <gsl/gsl_spline.h>
#include "backend/nsl/nsl_diff.h"
#include "backend/nsl/nsl_int.h"
}

#include <QElapsedTimer>
#include <QThreadPool>
#include <QIcon>

XYInterpolationCurve::XYInterpolationCurve(const QString& name)
	: XYAnalysisCurve(name, new XYInterpolationCurvePrivate(this), AspectType::XYInterpolationCurve) {
}

XYInterpolationCurve::XYInterpolationCurve(const QString& name, XYInterpolationCurvePrivate* dd)
	: XYAnalysisCurve(name, dd, AspectType::XYInterpolationCurve) {
}

//no need to delete the d-pointer here - it inherits from QGraphicsItem
//and is deleted during the cleanup in QGraphicsScene
XYInterpolationCurve::~XYInterpolationCurve() = default;

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
BASIC_SHARED_D_READER_IMPL(XYInterpolationCurve, XYInterpolationCurve::InterpolationData, interpolationData, interpolationData)

const XYInterpolationCurve::InterpolationResult& XYInterpolationCurve::interpolationResult() const {
	Q_D(const XYInterpolationCurve);
	return d->interpolationResult;
}

//##############################################################################
//#################  setter methods and undo commands ##########################
//##############################################################################
STD_SETTER_CMD_IMPL_F_S(XYInterpolationCurve, SetInterpolationData, XYInterpolationCurve::InterpolationData, interpolationData, recalculate);
void XYInterpolationCurve::setInterpolationData(const XYInterpolationCurve::InterpolationData& interpolationData) {
	Q_D(XYInterpolationCurve);
	exec(new XYInterpolationCurveSetInterpolationDataCmd(d, interpolationData, ki18n("%1: set options and perform the interpolation")));
}

//##############################################################################
//######################### Private implementation #############################
//##############################################################################
XYInterpolationCurvePrivate::XYInterpolationCurvePrivate(XYInterpolationCurve* owner) : XYAnalysisCurvePrivate(owner), q(owner) {
}

//no need to delete xColumn and yColumn, they are deleted
//when the parent aspect is removed
XYInterpolationCurvePrivate::~XYInterpolationCurvePrivate() = default;

void XYInterpolationCurvePrivate::recalculate() {
	QElapsedTimer timer;
	timer.start();

	//create interpolation result columns if not available yet, clear them otherwise
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

	// clear the previous result
	interpolationResult = XYInterpolationCurve::InterpolationResult();

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
		recalcLogicalPoints();
		emit q->dataChanged();
		sourceDataChangedSinceLastRecalc = false;
		return;
	}

	//check column sizes
	if (tmpXDataColumn->rowCount() != tmpYDataColumn->rowCount()) {
		interpolationResult.available = true;
		interpolationResult.valid = false;
		interpolationResult.status = i18n("Number of x and y data points must be equal.");
		recalcLogicalPoints();
		emit q->dataChanged();
		sourceDataChangedSinceLastRecalc = false;
		return;
	}

	//copy all valid data point for the interpolation to temporary vectors
	QVector<double> xdataVector;
	QVector<double> ydataVector;

	double xmin, xmax;
	if (interpolationData.autoRange) {	// all points
		xmin = tmpXDataColumn->minimum();
		xmax = tmpXDataColumn->maximum();
	} else {
		xmin = interpolationData.xRange.first();
		xmax = interpolationData.xRange.last();
	}

	XYAnalysisCurve::copyData(xdataVector, ydataVector, tmpXDataColumn, tmpYDataColumn, xmin, xmax);

	// only use range of valid data points
	const double validXMin = *std::min_element(xdataVector.constBegin(), xdataVector.constEnd());
	const double validXMax = *std::max_element(xdataVector.constBegin(), xdataVector.constEnd());
	if (interpolationData.autoRange) {
		xmin = validXMin;
		xmax = validXMax;
	} else {
		xmin = qMax(xmin, validXMin);
		xmax = qMin(xmax, validXMax);
	}
	DEBUG(Q_FUNC_INFO << ", x range = " << xmin << " .. " << xmax)

	//number of data points to interpolate
	const size_t n = (size_t)xdataVector.size();
	if (n < 2) {
		interpolationResult.available = true;
		interpolationResult.valid = false;
		interpolationResult.status = i18n("Not enough data points available.");
		recalcLogicalPoints();
		emit q->dataChanged();
		sourceDataChangedSinceLastRecalc = false;
		return;
	}

	double* xdata = xdataVector.data();
	double* ydata = ydataVector.data();

	// interpolation settings
	const nsl_interp_type type = interpolationData.type;
	const nsl_interp_pch_variant variant = interpolationData.variant;
	const double tension = interpolationData.tension;
	const double continuity = interpolationData.continuity;
	const double bias = interpolationData.bias;
	const nsl_interp_evaluate evaluate = interpolationData.evaluate;
	const size_t npoints = interpolationData.npoints;

	DEBUG(Q_FUNC_INFO << ", type = " << nsl_interp_type_name[type]);
	DEBUG(Q_FUNC_INFO << ", cubic Hermite variant: " << nsl_interp_pch_variant_name[variant] << " (" << tension << continuity << bias << ")");
	DEBUG(Q_FUNC_INFO << ", evaluate: " << nsl_interp_evaluate_name[evaluate]);
	DEBUG(Q_FUNC_INFO << ", npoints = " << npoints);
	DEBUG(Q_FUNC_INFO << ", data points = " << n);

///////////////////////////////////////////////////////////
	int status = 0;

	gsl_interp_accel *acc = gsl_interp_accel_alloc();
	gsl_spline *spline = nullptr;
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
	case nsl_interp_type_cosine:
	case nsl_interp_type_pch:
	case nsl_interp_type_rational:
	case nsl_interp_type_exponential:
		break;
	}

	xVector->resize((int)npoints);
	yVector->resize((int)npoints);
	for (unsigned int i = 0; i < npoints; i++) {
		size_t a = 0, b = n - 1;

		double x = xmin + i * (xmax - xmin) / (npoints - 1);
		(*xVector)[(int)i] = x;

		// find index a,b for interval [x[a],x[b]] around x[i] using bisection
		if (type == nsl_interp_type_cosine || type == nsl_interp_type_exponential || type == nsl_interp_type_pch) {
			while (b-a > 1) {
				unsigned int j = floor((a+b)/2.);
				if (xdata[j] > x)
					b = j;
				else
					a = j;
			}
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
				(*yVector)[(int)i] = gsl_spline_eval(spline, x, acc);
				break;
			case nsl_interp_evaluate_derivative:
				(*yVector)[(int)i] = gsl_spline_eval_deriv(spline, x, acc);
				break;
			case nsl_interp_evaluate_second_derivative:
				(*yVector)[(int)i] = gsl_spline_eval_deriv2(spline, x, acc);
				break;
			case nsl_interp_evaluate_integral:
				(*yVector)[(int)i] = gsl_spline_eval_integ(spline, xmin, x, acc);
				break;
			}
			break;
		case nsl_interp_type_cosine:
			t = (x-xdata[a])/(xdata[b]-xdata[a]);
			t = (1.-cos(M_PI*t))/2.;
			(*yVector)[(int)i] =  ydata[a] + t*(ydata[b]-ydata[a]);
			break;
		case nsl_interp_type_exponential:
			t = (x-xdata[a])/(xdata[b]-xdata[a]);
			(*yVector)[(int)i] = ydata[a]*pow(ydata[b]/ydata[a],t);
			break;
		case nsl_interp_type_pch: {
				t = (x-xdata[a])/(xdata[b]-xdata[a]);
				double t2 = t*t, t3 = t2*t;
				double h1 = 2.*t3-3.*t2+1, h2 = -2.*t3+3.*t2, h3 = t3-2*t2+t, h4 = t3-t2;
				double m1 = 0.,m2 = 0.;
				switch (variant) {
				case nsl_interp_pch_variant_finite_difference:
					if (a == 0)
						m1 = (ydata[b]-ydata[a])/(xdata[b]-xdata[a]);
					else
						m1 = ( (ydata[b]-ydata[a])/(xdata[b]-xdata[a]) + (ydata[a]-ydata[a-1])/(xdata[a]-xdata[a-1]) )/2.;
					if (b == n-1)
						m2 = (ydata[b]-ydata[a])/(xdata[b]-xdata[a]);
					else
						m2 = ( (ydata[b+1]-ydata[b])/(xdata[b+1]-xdata[b]) + (ydata[b]-ydata[a])/(xdata[b]-xdata[a]) )/2.;

					break;
				case nsl_interp_pch_variant_catmull_rom:
					if (a == 0)
						m1 = (ydata[b]-ydata[a])/(xdata[b]-xdata[a]);
					else
						m1 = (ydata[b]-ydata[a-1])/(xdata[b]-xdata[a-1]);
					if (b == n-1)
						m2 = (ydata[b]-ydata[a])/(xdata[b]-xdata[a]);
					else
						m2 = (ydata[b+1]-ydata[a])/(xdata[b+1]-xdata[a]);

					break;
				case nsl_interp_pch_variant_cardinal:
					if (a == 0)
						m1 = (ydata[b]-ydata[a])/(xdata[b]-xdata[a]);
					else
						m1 = (ydata[b]-ydata[a-1])/(xdata[b]-xdata[a-1]);
					m1 *= (1.-tension);
					if (b == n-1)
						m2 = (ydata[b]-ydata[a])/(xdata[b]-xdata[a]);
					else
						m2 = (ydata[b+1]-ydata[a])/(xdata[b+1]-xdata[a]);
					m2 *= (1.-tension);

					break;
				case nsl_interp_pch_variant_kochanek_bartels:
					if (a == 0)
						m1 = (1.+continuity)*(1.-bias)*(ydata[b]-ydata[a])/(xdata[b]-xdata[a]);
					else
						m1 = ( (1.-continuity)*(1.+bias)*(ydata[a]-ydata[a-1])/(xdata[a]-xdata[a-1])
						     + (1.+continuity)*(1.-bias)*(ydata[b]-ydata[a])/(xdata[b]-xdata[a]) )/2.;
					m1 *= (1.-tension);
					if (b == n-1)
						m2 = (1.+continuity)*(1.+bias)*(ydata[b]-ydata[a])/(xdata[b]-xdata[a]);
					else
						m2 = ( (1.+continuity)*(1.+bias)*(ydata[b]-ydata[a])/(xdata[b]-xdata[a])
						     + (1.-continuity)*(1.-bias)*(ydata[b+1]-ydata[b])/(xdata[b+1]-xdata[b]) )/2.;
					m2 *= (1.-tension);

					break;
				}

				// Hermite polynomial
				(*yVector)[(int)i] = ydata[a]*h1+ydata[b]*h2+(xdata[b]-xdata[a])*(m1*h3+m2*h4);
			}
			break;
		case nsl_interp_type_rational: {
				double v,dv;
				nsl_interp_ratint(xdata, ydata, (int)n, x, &v, &dv);
				(*yVector)[(int)i] = v;
				//TODO: use error dv
				break;
			}
		}
	}

	// calculate "evaluate" option for own types
	if (type == nsl_interp_type_cosine || type == nsl_interp_type_exponential || type == nsl_interp_type_pch || type == nsl_interp_type_rational) {
		switch (evaluate) {
		case nsl_interp_evaluate_function:
			break;
		case nsl_interp_evaluate_derivative:
			nsl_diff_first_deriv_second_order(xVector->data(), yVector->data(), npoints);
			break;
		case nsl_interp_evaluate_second_derivative:
			nsl_diff_second_deriv_second_order(xVector->data(), yVector->data(), npoints);
			break;
		case nsl_interp_evaluate_integral:
			nsl_int_trapezoid(xVector->data(), yVector->data(), npoints, 0);
			break;
		}
	}

	// check values
	for (int i = 0; i < (int)npoints; i++) {
		if ((*yVector)[i] > std::numeric_limits<double>::max())
			(*yVector)[i] = std::numeric_limits<double>::max();
		else if ((*yVector)[i] < std::numeric_limits<double>::lowest())
			(*yVector)[i] = std::numeric_limits<double>::lowest();
	}

	gsl_spline_free(spline);
	gsl_interp_accel_free(acc);

///////////////////////////////////////////////////////////

	//write the result
	interpolationResult.available = true;
	interpolationResult.valid = true;
	interpolationResult.status = gslErrorToString(status);
	interpolationResult.elapsedTime = timer.elapsed();

	//redraw the curve
	recalcLogicalPoints();
	emit q->dataChanged();
	sourceDataChangedSinceLastRecalc = false;
}

//##############################################################################
//##################  Serialization/Deserialization  ###########################
//##############################################################################
//! Save as XML
void XYInterpolationCurve::save(QXmlStreamWriter* writer) const {
	Q_D(const XYInterpolationCurve);

	writer->writeStartElement("xyInterpolationCurve");

	//write the base class
	XYAnalysisCurve::save(writer);

	//write xy-interpolation-curve specific information
	// interpolation data
	writer->writeStartElement("interpolationData");
	writer->writeAttribute( "autoRange", QString::number(d->interpolationData.autoRange) );
	writer->writeAttribute( "xRangeMin", QString::number(d->interpolationData.xRange.first()) );
	writer->writeAttribute( "xRangeMax", QString::number(d->interpolationData.xRange.last()) );
	writer->writeAttribute( "type", QString::number(d->interpolationData.type) );
	writer->writeAttribute( "variant", QString::number(d->interpolationData.variant) );
	writer->writeAttribute( "tension", QString::number(d->interpolationData.tension) );
	writer->writeAttribute( "continuity", QString::number(d->interpolationData.continuity) );
	writer->writeAttribute( "bias", QString::number(d->interpolationData.bias) );
	writer->writeAttribute( "npoints", QString::number(d->interpolationData.npoints) );
	writer->writeAttribute( "pointsMode", QString::number(static_cast<int>(d->interpolationData.pointsMode)) );
	writer->writeAttribute( "evaluate", QString::number(d->interpolationData.evaluate) );
	writer->writeEndElement();// interpolationData

	// interpolation results (generated columns)
	writer->writeStartElement("interpolationResult");
	writer->writeAttribute( "available", QString::number(d->interpolationResult.available) );
	writer->writeAttribute( "valid", QString::number(d->interpolationResult.valid) );
	writer->writeAttribute( "status", d->interpolationResult.status );
	writer->writeAttribute( "time", QString::number(d->interpolationResult.elapsedTime) );

	//save calculated columns if available
	if (saveCalculations() && d->xColumn) {
		d->xColumn->save(writer);
		d->yColumn->save(writer);
	}
	writer->writeEndElement(); //"interpolationResult"

	writer->writeEndElement(); //"xyInterpolationCurve"
}

//! Load from XML
bool XYInterpolationCurve::load(XmlStreamReader* reader, bool preview) {
	Q_D(XYInterpolationCurve);

	KLocalizedString attributeWarning = ki18n("Attribute '%1' missing or empty, default value is used");
	QXmlStreamAttributes attribs;
	QString str;

	while (!reader->atEnd()) {
		reader->readNext();
		if (reader->isEndElement() && reader->name() == "xyInterpolationCurve")
			break;

		if (!reader->isStartElement())
			continue;

		if (reader->name() == "xyAnalysisCurve") {
			if ( !XYAnalysisCurve::load(reader, preview) )
				return false;
		} else if (!preview && reader->name() == "interpolationData") {
			attribs = reader->attributes();
			READ_INT_VALUE("autoRange", interpolationData.autoRange, bool);
			READ_DOUBLE_VALUE("xRangeMin", interpolationData.xRange.first());
			READ_DOUBLE_VALUE("xRangeMax", interpolationData.xRange.last());
			READ_INT_VALUE("type", interpolationData.type, nsl_interp_type);
			READ_INT_VALUE("variant", interpolationData.variant, nsl_interp_pch_variant);
			READ_DOUBLE_VALUE("tension", interpolationData.tension);
			READ_DOUBLE_VALUE("continuity", interpolationData.continuity);
			READ_DOUBLE_VALUE("bias", interpolationData.bias);
			READ_INT_VALUE("npoints", interpolationData.npoints, size_t);
			READ_INT_VALUE("pointsMode", interpolationData.pointsMode, XYInterpolationCurve::PointsMode);
			READ_INT_VALUE("evaluate", interpolationData.evaluate, nsl_interp_evaluate);
		} else if (!preview && reader->name() == "interpolationResult") {
			attribs = reader->attributes();
			READ_INT_VALUE("available", interpolationResult.available, int);
			READ_INT_VALUE("valid", interpolationResult.valid, int);
			READ_STRING_VALUE("status", interpolationResult.status);
			READ_INT_VALUE("time", interpolationResult.elapsedTime, int);
		} else if (reader->name() == "column") {
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
