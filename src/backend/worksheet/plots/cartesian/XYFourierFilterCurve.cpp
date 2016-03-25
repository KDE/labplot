/***************************************************************************
    File                 : XYFourierFilterCurve.cpp
    Project              : LabPlot
    Description          : A xy-curve defined by a Fourier filter
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
  \class XYFourierFilterCurve
  \brief A xy-curve defined by a Fourier filter

  \ingroup worksheet
*/

#include "XYFourierFilterCurve.h"
#include "XYFourierFilterCurvePrivate.h"
#include "backend/core/AbstractColumn.h"
#include "backend/core/column/Column.h"
#include "backend/lib/commandtemplates.h"
/*#include "backend/gsl/ExpressionParser.h"
#include "backend/gsl/parser_extern.h"

#include <gsl/gsl_blas.h>
#include <gsl/gsl_vector.h>
#include <gsl/gsl_matrix.h>
#include <gsl/gsl_version.h>
*/
#include <KIcon>
#include <KLocale>
#include <QElapsedTimer>
#include <QDebug>

XYFourierFilterCurve::XYFourierFilterCurve(const QString& name)
		: XYCurve(name, new XYFourierFilterCurvePrivate(this)){
	init();
}

XYFourierFilterCurve::XYFourierFilterCurve(const QString& name, XYFourierFilterCurvePrivate* dd)
		: XYCurve(name, dd) {
	init();
}


XYFourierFilterCurve::~XYFourierFilterCurve() {
	//no need to delete the d-pointer here - it inherits from QGraphicsItem
	//and is deleted during the cleanup in QGraphicsScene
}

void XYFourierFilterCurve::init() {
	Q_D(XYFourierFilterCurve);

	//TODO: read from the saved settings for XYFourierFilterCurve?
	d->lineType = XYCurve::Line;
	d->symbolsStyle = Symbol::NoSymbols;
}

void XYFourierFilterCurve::recalculate() {
	Q_D(XYFourierFilterCurve);
	d->recalculate();
}

/*!
	Returns an icon to be used in the project explorer.
*/
QIcon XYFourierFilterCurve::icon() const {
	return KIcon("labplot-xy-fourier_filter-curve");
}

//##############################################################################
//##########################  getter methods  ##################################
//##############################################################################
BASIC_SHARED_D_READER_IMPL(XYFourierFilterCurve, const AbstractColumn*, xDataColumn, xDataColumn)
BASIC_SHARED_D_READER_IMPL(XYFourierFilterCurve, const AbstractColumn*, yDataColumn, yDataColumn)
const QString& XYFourierFilterCurve::xDataColumnPath() const { Q_D(const XYFourierFilterCurve); return d->xDataColumnPath; }
const QString& XYFourierFilterCurve::yDataColumnPath() const { Q_D(const XYFourierFilterCurve); return d->yDataColumnPath; }

BASIC_SHARED_D_READER_IMPL(XYFourierFilterCurve, XYFourierFilterCurve::FilterData, filterData, filterData)

bool XYFourierFilterCurve::isSourceDataChangedSinceLastFilter() const {
	Q_D(const XYFourierFilterCurve);
	return d->sourceDataChangedSinceLastFilter;
}

//##############################################################################
//#################  setter methods and undo commands ##########################
//##############################################################################
STD_SETTER_CMD_IMPL_S(XYFourierFilterCurve, SetXDataColumn, const AbstractColumn*, xDataColumn)
void XYFourierFilterCurve::setXDataColumn(const AbstractColumn* column) {
	Q_D(XYFourierFilterCurve);
	if (column != d->xDataColumn) {
		exec(new XYFourierFilterCurveSetXDataColumnCmd(d, column, i18n("%1: assign x-data")));
		emit sourceDataChangedSinceLastFilter();
		if (column) {
			connect(column, SIGNAL(dataChanged(const AbstractColumn*)), this, SLOT(handleSourceDataChanged()));
			//TODO disconnect on undo
		}
	}
}

STD_SETTER_CMD_IMPL_S(XYFourierFilterCurve, SetYDataColumn, const AbstractColumn*, yDataColumn)
void XYFourierFilterCurve::setYDataColumn(const AbstractColumn* column) {
	Q_D(XYFourierFilterCurve);
	if (column != d->yDataColumn) {
		exec(new XYFourierFilterCurveSetYDataColumnCmd(d, column, i18n("%1: assign y-data")));
		emit sourceDataChangedSinceLastFilter();
		if (column) {
			connect(column, SIGNAL(dataChanged(const AbstractColumn*)), this, SLOT(handleSourceDataChanged()));
			//TODO disconnect on undo
		}
	}
}

STD_SETTER_CMD_IMPL_F_S(XYFourierFilterCurve, SetFilterData, XYFourierFilterCurve::FilterData, filterData, recalculate);
void XYFourierFilterCurve::setFilterData(const XYFourierFilterCurve::FilterData& filterData) {
	Q_D(XYFourierFilterCurve);
	exec(new XYFourierFilterCurveSetFilterDataCmd(d, filterData, i18n("%1: set filter options and perform the Fourier filter")));
}

//##############################################################################
//################################## SLOTS ####################################
//##############################################################################
void XYFourierFilterCurve::handleSourceDataChanged() {
	Q_D(XYFourierFilterCurve);
	d->sourceDataChangedSinceLastFilter = true;
	emit sourceDataChangedSinceLastFilter();
}
//##############################################################################
//######################### Private implementation #############################
//##############################################################################
XYFourierFilterCurvePrivate::XYFourierFilterCurvePrivate(XYFourierFilterCurve* owner) : XYCurvePrivate(owner),
	xDataColumn(0), yDataColumn(0), weightsColumn(0),
	xColumn(0), yColumn(0), residualsColumn(0),
	xVector(0), yVector(0), residualsVector(0),
	sourceDataChangedSinceLastFilter(false),
	q(owner)  {

}

XYFourierFilterCurvePrivate::~XYFourierFilterCurvePrivate() {
	//no need to delete xColumn and yColumn, they are deleted
	//when the parent aspect is removed
}

// ...
// see XYFitCurvePrivate

void XYFourierFilterCurvePrivate::recalculate() {
/*
	QElapsedTimer timer;
	timer.start();

	//create fit result columns if not available yet, clear them otherwise
	if (!xColumn) {
		xColumn = new Column("x", AbstractColumn::Numeric);
		yColumn = new Column("y", AbstractColumn::Numeric);
		residualsColumn = new Column("residuals", AbstractColumn::Numeric);
		xVector = static_cast<QVector<double>* >(xColumn->data());
		yVector = static_cast<QVector<double>* >(yColumn->data());
		residualsVector = static_cast<QVector<double>* >(residualsColumn->data());

		xColumn->setHidden(true);
		q->addChild(xColumn);

		yColumn->setHidden(true);
		q->addChild(yColumn);

		q->addChild(residualsColumn);

		q->setUndoAware(false);
		q->setXColumn(xColumn);
		q->setYColumn(yColumn);
		q->setUndoAware(true);
	} else {
		xVector->clear();
		yVector->clear();
		residualsVector->clear();
	}

	// clear the previous result
	fitResult = XYFitCurve::FitResult();

	if (!xDataColumn || !yDataColumn) {
		emit (q->dataChanged());
		sourceDataChangedSinceLastFilter = false;
		return;
	}

	//fit settings
	int maxIters = fitData.maxIterations; //maximal number of iterations
	float delta = fitData.eps; //fit tolerance
	const unsigned int np = fitData.paramNames.size(); //number of fit parameters
	if (np == 0) {
		fitResult.available = true;
		fitResult.valid = false;
		fitResult.status = i18n("Model has no parameters.");
		emit (q->dataChanged());
		sourceDataChangedSinceLastFilter = false;
		return;
	}

	//check column sizes
	if (xDataColumn->rowCount()!=yDataColumn->rowCount()) {
		fitResult.available = true;
		fitResult.valid = false;
		fitResult.status = i18n("Number of x and y data points must be equal.");
		emit (q->dataChanged());
		sourceDataChangedSinceLastFilter = false;
		return;
	}
	if (weightsColumn) {
		if (weightsColumn->rowCount()<xDataColumn->rowCount()) {
			fitResult.available = true;
			fitResult.valid = false;
			fitResult.status = i18n("Not sufficient weight data points provided.");
			emit (q->dataChanged());
			sourceDataChangedSinceLastFilter = false;
			return;
		}
	}

	//copy all valid data point for the fit to temporary vectors
	QVector<double> xdataVector;
	QVector<double> ydataVector;
	QVector<double> sigmaVector;
	for (int row=0; row<xDataColumn->rowCount(); ++row) {
		//only copy those data where _all_ values (for x, y and sigma, if given) are valid
		if (!isnan(xDataColumn->valueAt(row)) && !isnan(yDataColumn->valueAt(row))
			&& !xDataColumn->isMasked(row) && !yDataColumn->isMasked(row)) {

			if (!weightsColumn) {
				xdataVector.append(xDataColumn->valueAt(row));
				ydataVector.append(yDataColumn->valueAt(row));
			} else {
				if (!isnan(weightsColumn->valueAt(row))) {
					xdataVector.append(xDataColumn->valueAt(row));
					ydataVector.append(yDataColumn->valueAt(row));

					if (fitData.weightsType == XYFitCurve::WeightsFromColumn) {
						//weights from a given column -> calculate the square root of the inverse (sigma = sqrt(1/weight))
						sigmaVector.append( sqrt(1/weightsColumn->valueAt(row)) );
					} else if (fitData.weightsType == XYFitCurve::WeightsFromErrorColumn) {
						//weights from a given column with error bars (sigma = error)
						sigmaVector.append( weightsColumn->valueAt(row) );
					}
				}
			}
		}
	}

	//number of data points to fit
	unsigned int n = xdataVector.size();
	if (n == 0) {
		fitResult.available = true;
		fitResult.valid = false;
		fitResult.status = i18n("No data points available.");
		emit (q->dataChanged());
		sourceDataChangedSinceLastFilter = false;
		return;
	}

	if (n<np) {
		fitResult.available = true;
		fitResult.valid = false;
		fitResult.status = i18n("The number of data points (%1) must be greater than or equal to the number of parameters (%2).").arg(n).arg(np);
		emit (q->dataChanged());
		sourceDataChangedSinceLastFilter = false;
		return;
	}

	double* xdata = xdataVector.data();
	double* ydata = ydataVector.data();
	double* sigma = 0;
	if (sigmaVector.size())
		sigma = sigmaVector.data();

	//function to fit
	gsl_multifit_function_fdf f;
	struct data params = {n, xdata, ydata, sigma, fitData.modelType, fitData.degree, &fitData.model, &fitData.paramNames};
	f.f = &func_f;
	f.df = &func_df;
	f.fdf = &func_fdf;
	f.n = n;
	f.p = np;
	f.params = &params;

	//initialize the solver
	const gsl_multifit_fdfsolver_type* T = gsl_multifit_fdfsolver_lmsder;
	gsl_multifit_fdfsolver* s = gsl_multifit_fdfsolver_alloc (T, n, np);
	double* x_init = fitData.paramStartValues.data();
	gsl_vector_view x = gsl_vector_view_array (x_init, np);
	gsl_multifit_fdfsolver_set (s, &f, &x.vector);

	//iterate
	int status;
	int iter = 0;
	fitResult.solverOutput.clear();
	writeSolverState(s);
	do {
		iter++;
		status = gsl_multifit_fdfsolver_iterate (s);
		writeSolverState(s);
		if (status) break;
		status = gsl_multifit_test_delta (s->dx, s->x, delta, delta);
	} while (status == GSL_CONTINUE && iter < maxIters);

	//get the covariance matrix
	gsl_matrix* covar = gsl_matrix_alloc (np, np);
#if GSL_MAJOR_VERSION >=2
	gsl_matrix *J=0;
	gsl_multifit_fdfsolver_jac (s, J);
	gsl_multifit_covar (J, 0.0, covar);
#else
	gsl_multifit_covar (s->J, 0.0, covar);
#endif

	//write the result
	fitResult.available = true;
	fitResult.valid = true;
	fitResult.status = QString(gsl_strerror(status)); //TODO: add i18n
	fitResult.iterations = iter;
	fitResult.dof = n-np;

	//calculate:
	//residuals (Y_i-y_i)
	//sse = sum of squared errors (SSE) = residual sum of errors (RSS) = sum of sq. residuals (SSR) = \sum_i^n (Y_i-y_i)^2
	//mse = mean squared error = 1/n \sum_i^n  (Y_i-y_i)^2
	//rmse = root-mean squared error = \sqrt(mse)
	//mae = mean absolute error = \sum_i^n |Y_i-y_i|
	//rms = residual mean square = sse/d.o.f.
	//rsd = residual standard deviation = sqrt(rms)
	//Coefficient of determination, R-squared = 1 - SSE/SSTOT with the total sum of squares SSTOT = \sum_i (y_i - ybar)^2 and ybar = 1/n \sum_i y_i
	//Adjusted Coefficient of determination  adj. R-squared = 1 - (1-R-squared^2)*(n-1)/(n-np-1);

	residualsVector->resize(n);
	for (unsigned int i=0; i<n; ++i) {
		residualsVector->data()[i] = gsl_vector_get(s->f, i);
	}
	residualsColumn->setChanged();

	//gsl_blas_dnrm2() - computes the Euclidian norm (||x||_2 = \sqrt {\sum x_i^2}) of the vector with the elements (Yi - y[i])/sigma[i]
	//gsl_blas_dasum() - computes the absolute sum \sum |x_i| of the elements of the vector with the elements (Yi - y[i])/sigma[i]
	fitResult.sse = pow(gsl_blas_dnrm2(s->f), 2);
	fitResult.mse = fitResult.sse/n;
	fitResult.rmse = sqrt(fitResult.mse);
	fitResult.mae = gsl_blas_dasum(s->f);
	if (fitResult.dof!=0) {
		fitResult.rms = fitResult.sse/fitResult.dof;
		fitResult.rsd = sqrt(fitResult.rms);
	}

	//Coefficient of determination, R-squared
	double ybar = 0; //mean value of the y-data
	for (unsigned int i=0; i<n; ++i)
		ybar += ydata[i];
	ybar = ybar/n;
	double sstot = 0;
	for (unsigned int i=0; i<n; ++i)
		sstot += pow(ydata[i]-ybar, 2);
	fitResult.rsquared = 1 - fitResult.sse/sstot;
	fitResult.rsquaredAdj = 1-(1-fitResult.rsquared*fitResult.rsquared)*(n-1)/(n-np-1);

	//parameter values
	double c = GSL_MIN_DBL(1, sqrt(fitResult.sse)); //limit error for poor fit
	fitResult.paramValues.resize(np);
	fitResult.errorValues.resize(np);
	for (unsigned int i=0; i<np; i++) {
		fitResult.paramValues[i] = gsl_vector_get(s->x, i);
		fitResult.errorValues[i] = c*sqrt(gsl_matrix_get(covar,i,i));
	}

	//free resources
	gsl_multifit_fdfsolver_free(s);
	gsl_matrix_free(covar);

	//calculate the fit function (vectors)
	ExpressionParser* parser = ExpressionParser::getInstance();
	double min = xDataColumn->minimum();
	double max = xDataColumn->maximum();
	xVector->resize(fitData.fittedPoints);
	yVector->resize(fitData.fittedPoints);
	bool rc = parser->evaluateCartesian(fitData.model, QString::number(min), QString::number(max), fitData.fittedPoints, xVector, yVector, fitData.paramNames, fitResult.paramValues);
	if (!rc) {
		xVector->clear();
		yVector->clear();
	}

	fitResult.elapsedTime = timer.elapsed();
*/
	//redraw the curve
	emit (q->dataChanged());
	sourceDataChangedSinceLastFilter = false;
}

/*!
 * writes out the current state of the solver \c s
 */
/*void XYFitCurvePrivate::writeSolverState(gsl_multifit_fdfsolver* s) {
	QString state;

	//current parameter values, semicolon separated
	for (int i=0; i<fitData.paramNames.size(); ++i)
		state += QString::number(gsl_vector_get(s->x, i)) + '\t';

	//current value of the chi2-function
	state += QString::number(pow(gsl_blas_dnrm2 (s->f),2));
	state += ';';

	fitResult.solverOutput += state;
}*/


//##############################################################################
//##################  Serialization/Deserialization  ###########################
//##############################################################################
//! Save as XML
void XYFourierFilterCurve::save(QXmlStreamWriter* writer) const{
	Q_D(const XYFourierFilterCurve);

	writer->writeStartElement("xyFourierFilterCurve");

	//write xy-curve information
	XYCurve::save(writer);

	//write xy-fourier_filter-curve specific information
/*
	//fit data
	writer->writeStartElement("fitData");
	WRITE_COLUMN(d->xDataColumn, xDataColumn);
	WRITE_COLUMN(d->yDataColumn, yDataColumn);
	WRITE_COLUMN(d->weightsColumn, weightsColumn);
	writer->writeAttribute( "modelType", QString::number(d->fitData.modelType) );
	writer->writeAttribute( "weightsType", QString::number(d->fitData.weightsType) );
	writer->writeAttribute( "degree", QString::number(d->fitData.degree) );
	writer->writeAttribute( "model", d->fitData.model );
	writer->writeAttribute( "maxIterations", QString::number(d->fitData.maxIterations) );
	writer->writeAttribute( "eps", QString::number(d->fitData.eps) );
	writer->writeAttribute( "fittedPoints", QString::number(d->fitData.fittedPoints) );

	writer->writeStartElement("paramNames");
	for (int i=0; i<d->fitData.paramNames.size(); ++i)
		writer->writeTextElement("name", d->fitData.paramNames.at(i));
	writer->writeEndElement();

	writer->writeStartElement("paramStartValues");
	for (int i=0; i<d->fitData.paramStartValues.size(); ++i)
		writer->writeTextElement("startValue", QString::number(d->fitData.paramStartValues.at(i)));
	writer->writeEndElement();

	writer->writeEndElement();// fourierFilterData

	//fit results (generated columns and goodness of the fit)
	//sse = sum of squared errors (SSE) = residual sum of errors (RSS) = sum of sq. residuals (SSR) = \sum_i^n (Y_i-y_i)^2
	//mse = mean squared error = 1/n \sum_i^n  (Y_i-y_i)^2
	//rmse = root-mean squared error = \sqrt(mse)
	//mae = mean absolute error = \sum_i^n |Y_i-y_i|
	//rms = residual mean square = sse/d.o.f.
	//rsd = residual standard deviation = sqrt(rms)
	//R-squared
	//adjusted R-squared
	writer->writeStartElement("fitResult");
	writer->writeAttribute( "available", QString::number(d->fitResult.available) );
	writer->writeAttribute( "valid", QString::number(d->fitResult.valid) );
	writer->writeAttribute( "status", d->fitResult.status );
	writer->writeAttribute( "iterations", QString::number(d->fitResult.iterations) );
	writer->writeAttribute( "time", QString::number(d->fitResult.elapsedTime) );
	writer->writeAttribute( "dof", QString::number(d->fitResult.dof) );
	writer->writeAttribute( "sse", QString::number(d->fitResult.sse) );
	writer->writeAttribute( "mse", QString::number(d->fitResult.mse) );
	writer->writeAttribute( "rmse", QString::number(d->fitResult.rmse) );
	writer->writeAttribute( "mae", QString::number(d->fitResult.mae) );
	writer->writeAttribute( "rms", QString::number(d->fitResult.rms) );
	writer->writeAttribute( "rsd", QString::number(d->fitResult.rsd) );
	writer->writeAttribute( "rsquared", QString::number(d->fitResult.rsquared) );
	writer->writeAttribute( "rsquaredAdj", QString::number(d->fitResult.rsquaredAdj) );
	writer->writeAttribute( "solverOutput", d->fitResult.solverOutput );

	writer->writeStartElement("paramValues");
	for (int i=0; i<d->fitResult.paramValues.size(); ++i)
		writer->writeTextElement("value", QString::number(d->fitResult.paramValues.at(i)));
	writer->writeEndElement();

	writer->writeStartElement("errorValues");
	for (int i=0; i<d->fitResult.errorValues.size(); ++i)
		writer->writeTextElement("error", QString::number(d->fitResult.errorValues.at(i)));
	writer->writeEndElement();

	//save calculated columns if available
	if (d->xColumn) {
		d->xColumn->save(writer);
		d->yColumn->save(writer);
		d->residualsColumn->save(writer);
	}
*/

	//writer->writeEndElement(); //"fitResult"
	writer->writeEndElement(); //"xyFourierFilterCurve"
}

//! Load from XML
bool XYFourierFilterCurve::load(XmlStreamReader* reader){
	Q_D(XYFourierFilterCurve);

	if(!reader->isStartElement() || reader->name() != "xyFourierFilterCurve"){
		reader->raiseError(i18n("no xy Fourier filter curve element found"));
		return false;
	}

	QString attributeWarning = i18n("Attribute '%1' missing or empty, default value is used");
	QXmlStreamAttributes attribs;
	QString str;

	while (!reader->atEnd()) {
		reader->readNext();
		if (reader->isEndElement() && reader->name() == "xyFourierFilterCurve")
			break;

		if (!reader->isStartElement())
			continue;

		if (reader->name() == "xyCurve") {
			if ( !XYCurve::load(reader) )
				return false;
		} else if (reader->name() == "fitData") {
			attribs = reader->attributes();

			READ_COLUMN(xDataColumn);
			READ_COLUMN(yDataColumn);
			//READ_COLUMN(weightsColumn);
/*
			str = attribs.value("modelType").toString();
			if(str.isEmpty())
				reader->raiseWarning(attributeWarning.arg("'modelType'"));
			else
				d->fitData.modelType = (XYFitCurve::ModelType)str.toInt();

			str = attribs.value("weightsType").toString();
			if(str.isEmpty())
				reader->raiseWarning(attributeWarning.arg("'weightsType'"));
			else
				d->fitData.weightsType = (XYFitCurve::WeightsType)str.toInt();

			str = attribs.value("degree").toString();
			if(str.isEmpty())
				reader->raiseWarning(attributeWarning.arg("'degree'"));
			else
				d->fitData.degree = str.toInt();

			str = attribs.value("model").toString();
			if(str.isEmpty())
				reader->raiseWarning(attributeWarning.arg("'model'"));
			else
				d->fitData.model = str;

			str = attribs.value("maxIterations").toString();
			if(str.isEmpty())
				reader->raiseWarning(attributeWarning.arg("'maxIterations'"));
			else
				d->fitData.maxIterations = str.toInt();

			str = attribs.value("eps").toString();
			if(str.isEmpty())
				reader->raiseWarning(attributeWarning.arg("'eps'"));
			else
				d->fitData.eps = str.toDouble();

			str = attribs.value("fittedPoints").toString();
			if(str.isEmpty())
				reader->raiseWarning(attributeWarning.arg("'fittedPoints'"));
			else
				d->fitData.fittedPoints = str.toInt();
*/
		} else if (reader->name() == "name") {
//			d->fitData.paramNames<<reader->readElementText();
		} else if (reader->name() == "startValue") {
//			d->fitData.paramStartValues<<reader->readElementText().toDouble();
		} else if (reader->name() == "value") {
//			d->fitResult.paramValues<<reader->readElementText().toDouble();
		} else if (reader->name() == "error") {
//			d->fitResult.errorValues<<reader->readElementText().toDouble();
		} else if (reader->name() == "fitResult") {
/*
			attribs = reader->attributes();

			str = attribs.value("available").toString();
			if(str.isEmpty())
				reader->raiseWarning(attributeWarning.arg("'available'"));
			else
				d->fitResult.available = str.toInt();

// TODO: formatting
			str = attribs.value("valid").toString();
            if(str.isEmpty())
                reader->raiseWarning(attributeWarning.arg("'valid'"));
            else
                d->fitResult.valid = str.toInt();

			str = attribs.value("status").toString();
            if(str.isEmpty())
                reader->raiseWarning(attributeWarning.arg("'status'"));
            else
                d->fitResult.status = str;

			str = attribs.value("iterations").toString();
            if(str.isEmpty())
                reader->raiseWarning(attributeWarning.arg("'iterations'"));
            else
                d->fitResult.iterations = str.toInt();

			str = attribs.value("time").toString();
            if(str.isEmpty())
                reader->raiseWarning(attributeWarning.arg("'time'"));
            else
                d->fitResult.elapsedTime = str.toInt();

			str = attribs.value("dof").toString();
            if(str.isEmpty())
                reader->raiseWarning(attributeWarning.arg("'dof'"));
            else
                d->fitResult.dof = str.toDouble();

			str = attribs.value("sse").toString();
            if(str.isEmpty())
                reader->raiseWarning(attributeWarning.arg("'sse'"));
            else
                d->fitResult.sse = str.toDouble();

			str = attribs.value("mse").toString();
            if(str.isEmpty())
                reader->raiseWarning(attributeWarning.arg("'mse'"));
            else
                d->fitResult.mse = str.toDouble();

			str = attribs.value("rmse").toString();
            if(str.isEmpty())
                reader->raiseWarning(attributeWarning.arg("'rmse'"));
            else
                d->fitResult.rmse = str.toDouble();

			str = attribs.value("mae").toString();
            if(str.isEmpty())
                reader->raiseWarning(attributeWarning.arg("'mae'"));
            else
                d->fitResult.mae = str.toDouble();

			str = attribs.value("rms").toString();
            if(str.isEmpty())
                reader->raiseWarning(attributeWarning.arg("'rms'"));
            else
                d->fitResult.rms = str.toDouble();

			str = attribs.value("rsd").toString();
            if(str.isEmpty())
                reader->raiseWarning(attributeWarning.arg("'rsd'"));
            else
                d->fitResult.rsd = str.toDouble();

			str = attribs.value("rsquared").toString();
            if(str.isEmpty())
                reader->raiseWarning(attributeWarning.arg("'rsquared'"));
            else
                d->fitResult.rsquared = str.toDouble();

			str = attribs.value("rsquaredAdj").toString();
            if(str.isEmpty())
                reader->raiseWarning(attributeWarning.arg("'rsquaredAdj'"));
            else
                d->fitResult.rsquaredAdj = str.toDouble();

			str = attribs.value("solverOutput").toString();
            if(str.isEmpty())
                reader->raiseWarning(attributeWarning.arg("'solverOutput'"));
            else
                d->fitResult.solverOutput = str;
*/
		} else if(reader->name() == "column") {

		Column* column = new Column("", AbstractColumn::Numeric);
			if (!column->load(reader)) {
				delete column;
				return false;
			}
			if (column->name()=="x")
				d->xColumn = column;
			else if (column->name()=="y")
				d->yColumn = column;
			// else if (column->name()=="residuals")
			//	d->residualsColumn = column;
		}
	}

/*
	if (d->xColumn) {
		d->xColumn->setHidden(true);
		addChild(d->xColumn);

		d->yColumn->setHidden(true);
		addChild(d->yColumn);

		//addChild(d->residualsColumn);

		d->xVector = static_cast<QVector<double>* >(d->xColumn->data());
		d->yVector = static_cast<QVector<double>* >(d->yColumn->data());
		//d->residualsVector = static_cast<QVector<double>* >(d->residualsColumn->data());

		setUndoAware(false);
		XYCurve::d_ptr->xColumn = d->xColumn;
		XYCurve::d_ptr->yColumn = d->yColumn;
		setUndoAware(true);
	}
*/
	return true;
}
