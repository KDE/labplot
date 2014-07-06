/***************************************************************************
    File                 : XYFitCurve.cpp
    Project              : LabPlot
    Description          : A xy-curve defined by a fit model
    --------------------------------------------------------------------
    Copyright            : (C) 2014 Alexander Semke (alexander.semke*web.de)

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
  \class XYFitCurve
  \brief A xy-curve defined by a fit model

  \ingroup worksheet
*/

#include "XYFitCurve.h"
#include "XYFitCurvePrivate.h"
#include "backend/core/AbstractColumn.h"
#include "backend/core/column/Column.h"
#include "backend/lib/commandtemplates.h"
#include "backend/gsl/ExpressionParser.h"
#include "backend/gsl/parser_extern.h"
#include "backend/gsl/parser_struct.h"

#include <math.h>
#include <gsl/gsl_blas.h>
#include <gsl/gsl_vector.h>
#include <gsl/gsl_matrix.h>


#include <KIcon>
#include <QDebug>

XYFitCurve::XYFitCurve(const QString& name)
		: XYCurve(name, new XYFitCurvePrivate(this)){
	init();
}

XYFitCurve::XYFitCurve(const QString& name, XYFitCurvePrivate* dd)
		: XYCurve(name, dd){
	init();
}


XYFitCurve::~XYFitCurve() {
	//no need to delete the d-pointer here - it inherits from QGraphicsItem
	//and is deleted during the cleanup in QGraphicsScene
}

void XYFitCurve::init() {
	Q_D(XYFitCurve);

	d->xColumn->setHidden(true);
	addChild(d->xColumn);

	d->yColumn->setHidden(true);
	addChild(d->yColumn);

	//TODO: read from the saved settings for XYFitCurve?
	d->lineType = XYCurve::Line;
	d->swapSymbolsTypeId("none");

	setUndoAware(false);
	setXColumn(d->xColumn);
	setYColumn(d->yColumn);
	setUndoAware(true);
}

void XYFitCurve::recalculate() {
	Q_D(XYFitCurve);
	d->recalculate();
}

/*!
	Returns an icon to be used in the project explorer.
*/
QIcon XYFitCurve::icon() const {
	return KIcon("xy-fit-curve");
}

//##############################################################################
//##########################  getter methods  ##################################
//##############################################################################
BASIC_SHARED_D_READER_IMPL(XYFitCurve, const AbstractColumn*, xDataColumn, xDataColumn)
BASIC_SHARED_D_READER_IMPL(XYFitCurve, const AbstractColumn*, yDataColumn, yDataColumn)
BASIC_SHARED_D_READER_IMPL(XYFitCurve, const AbstractColumn*, weightsColumn, weightsColumn)
const QString& XYFitCurve::xDataColumnPath() const { Q_D(const XYFitCurve); return d->xDataColumnPath; }
const QString& XYFitCurve::yDataColumnPath() const {	Q_D(const XYFitCurve); return d->yDataColumnPath; }
const QString& XYFitCurve::weightsColumnPath() const { Q_D(const XYFitCurve);return d->weightsColumnPath; }

BASIC_SHARED_D_READER_IMPL(XYFitCurve, XYFitCurve::FitData, fitData, fitData)

const XYFitCurve::FitResult& XYFitCurve::fitResult() const {
	Q_D(const XYFitCurve);
	return d->fitResult;
}
//##############################################################################
//#################  setter methods and undo commands ##########################
//##############################################################################
STD_SETTER_CMD_IMPL_S(XYFitCurve, SetXDataColumn, const AbstractColumn*, xDataColumn)
void XYFitCurve::setXDataColumn(const AbstractColumn* column) {
	Q_D(XYFitCurve);
	if (column != d->xDataColumn)
		exec(new XYFitCurveSetXDataColumnCmd(d, column, i18n("%1: assign x-data")));
}

STD_SETTER_CMD_IMPL_S(XYFitCurve, SetYDataColumn, const AbstractColumn*, yDataColumn)
void XYFitCurve::setYDataColumn(const AbstractColumn* column) {
	Q_D(XYFitCurve);
	if (column != d->yDataColumn)
		exec(new XYFitCurveSetYDataColumnCmd(d, column, i18n("%1: assign y-data")));
}

STD_SETTER_CMD_IMPL_S(XYFitCurve, SetWeightsColumn, const AbstractColumn*, weightsColumn)
void XYFitCurve::setWeightsColumn(const AbstractColumn* column) {
	Q_D(XYFitCurve);
	if (column != d->weightsColumn)
		exec(new XYFitCurveSetWeightsColumnCmd(d, column, i18n("%1: assign weights")));
}

STD_SETTER_CMD_IMPL_F_S(XYFitCurve, SetFitData, XYFitCurve::FitData, fitData, recalculate);
void XYFitCurve::setFitData(const XYFitCurve::FitData& fitData) {
	Q_D(XYFitCurve);
	exec(new XYFitCurveSetFitDataCmd(d, fitData, i18n("%1: set fit options and perform the fit")));
}

//##############################################################################
//######################### Private implementation #############################
//##############################################################################
XYFitCurvePrivate::XYFitCurvePrivate(XYFitCurve* owner) : XYCurvePrivate(owner),
	xDataColumn(0),
	yDataColumn(0),
	weightsColumn(0),
	xColumn(new Column("x", AbstractColumn::Numeric)),
	yColumn(new Column("y", AbstractColumn::Numeric)),
	xVector(static_cast<QVector<double>* >(xColumn->data())),
	yVector(static_cast<QVector<double>* >(yColumn->data())),
	q(owner)  {

}

XYFitCurvePrivate::~XYFitCurvePrivate() {
	//no need to delete xColumn and yColumn, they are deleted
	//when the parent aspect is removed
}

struct data {
	size_t n; //number of data points
	double* x; //pointer to the vector with x-data values
	double* y; //pointer to the vector with y-data vector
	double* sigma; //pointer to the vector with sigma values
	XYFitCurve::ModelType modelType;
	int degree;
	QString* func; // string containing the definition of the model/function
	QStringList* paramNames;
};

/*!
 * \param v vector containing current values of the fit parameters
 * \param params
 * \param f vector with the elements (Yi - y[i])/sigma[i]
 */
int func_f(const gsl_vector* paramValues, void* params, gsl_vector* f) {
	int n = ((struct data*)params)->n;
	double* x = ((struct data*)params)->x;
	double* y = ((struct data*)params)->y;
	double* sigma = ((struct data*)params)->sigma;
	char *func = ((struct data*)params)->func->toLocal8Bit().data();
	QStringList* paramNames = ((struct data*)params)->paramNames;

	//set current values of the parameters
	for (int j=0; j<paramNames->size(); j++)
		assign_variable(paramNames->at(j).toLatin1().data(), gsl_vector_get(paramValues,j));

	char var[]="x";
	for (int i = 0; i < n; i++) {
		if (isnan(x[i]) || isnan(y[i]))
			continue;

		double Yi=0;
		//TODO: add checks for allowed valus of x for different models if required (x>0 for ln(x) etc.)

		assign_variable(var, x[i]);
		Yi = parse(func);
		if(parse_errors()>0) {
			qDebug()<<"func_f: parse errors in parsing "<<func;
			return GSL_EINVAL;
		}

// 		Yi += base; //TODO
		if (sigma) {
				gsl_vector_set (f, i, (Yi - y[i])/sigma[i]);
		} else {
				gsl_vector_set (f, i, (Yi - y[i]));
		}
	}

	return GSL_SUCCESS;
}

/*!
 * calculates the matrix elements of Jacobian matrix
 * \param paramValues current parameter values
 * \param params
 * \param J Jacobian matrix
 * */
int func_df(const gsl_vector* paramValues, void* params, gsl_matrix* J) {
	int n = ((struct data*)params)->n;
	double* x = ((struct data*)params)->x;
	double* y = ((struct data*)params)->y;
	double* sigma = ((struct data*)params)->sigma;
	char* func = ((struct data*)params)->func->toLatin1().data();
	QStringList* paramNames = ((struct data*)params)->paramNames;
	XYFitCurve::ModelType modelType = ((struct data*)params)->modelType;

	//copy current parameter values from gsl-vector to a local double array for the sake of easier usage below
	double* p = new double[paramNames->size()];
	for (int i=0; i<paramNames->size(); i++)
		p[i]=gsl_vector_get(paramValues,i);

	//calculate the Jacobian matrix
	for (int i=0; i<n; i++) {
		/* Jacobian matrix J(i,j) = dfi / dxj, */
		/* where fi = (Yi - yi)/sigma[i],      */
		/*	Yi = model  */
		/* and the xj are the parameters */

		double t = x[i];
		double s = 1.0;
		if (sigma)
			s = sigma[i];

		switch(modelType) {
			case XYFitCurve::Polynomial: {
				for (int j=0; j<paramNames->size(); ++j) {
					gsl_matrix_set(J, i, j, pow(t,j)/s);
				}
				break;
			}
		}
	}

	return GSL_SUCCESS;
}

int func_fdf(const gsl_vector* x, void* params, gsl_vector* f,gsl_matrix* J) {
	func_f (x, params, f);
	func_df (x, params, J);
	return GSL_SUCCESS;
}

void XYFitCurvePrivate::recalculate() {
	qDebug()<<"XYFitCurvePrivate::recalculate";

	// clear the previous result
	xVector->clear();
	yVector->clear();

	if (!xDataColumn || !yDataColumn) {
		emit (q->xDataChanged());
		emit (q->yDataChanged());
		return;
	}

	//data to fit
	double* xdata = static_cast<QVector<double>* >(dynamic_cast<Column*>( const_cast<AbstractColumn*>(xDataColumn) )->data())->data();
	double* ydata = static_cast<QVector<double>* >(dynamic_cast<Column*>( const_cast<AbstractColumn*>(yDataColumn) )->data())->data();

	//fit settings
	int maxIters = fitData.maxIterations; //maximal number of iteratoins
	float delta = fitData.eps; //fit tolerance
	const int np = fitData.paramNames.size(); //number of fit parameters
	size_t n = 0; //determine the number of data points in the column, stop iterating after the first nan was encountered
	for (int i=0; i<xDataColumn->rowCount(); ++i) {
		if (isnan(xdata[i])) {
			n = i;
			break;
		}
	}

	//calculate sigma for the given weights type
	double* sigma = 0;
	if (fitData.weightsType == XYFitCurve::WeightsFromColumn) {
		//weights from a given column -> calculate the inverse (sigma = 1/weight)
		//TODO
	} else if (fitData.weightsType == XYFitCurve::WeightsFromErrorColumn) {
		//weights from a given column with error bars (sigma = error)
		if (!weightsColumn)
			sigma = static_cast<QVector<double>* >(dynamic_cast<Column*>( const_cast<AbstractColumn*>(weightsColumn) )->data())->data();
	}

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

	//get the fit result and create the info-string
	gsl_matrix* covar = gsl_matrix_alloc (np, np);
	gsl_multifit_covar (s->J, 0.0, covar);

	const size_t dof = n-np;
	double chi = gsl_blas_dnrm2(s->f); //compute the Euclidian norm (||x||_2 = \sqrt {\sum x_i^2}) of the vector with the elements (Yi - y[i])/sigma[i]
	double c = GSL_MIN_DBL(1, chi);	// limit error for poor fit

	fitResult.paramValues.resize(np);
	fitResult.errorValues.resize(np);
	for (int i=0; i<np; i++) {
		fitResult.paramValues[i] = gsl_vector_get(s->x, i);
		fitResult.errorValues[i] = c*sqrt(gsl_matrix_get(covar,i,i));
	}

	fitResult.status = QString(gsl_strerror(status)); //TODO: add i18n
	fitResult.iterations = iter;
	fitResult.chi2 =  pow(chi, 2);
	fitResult.chi2_over_dof = fitResult.chi2/dof;

	gsl_multifit_fdfsolver_free(s);
	gsl_matrix_free(covar);

	//calculate the fit function (vectors)
	ExpressionParser* parser = ExpressionParser::getInstance();
	double min = xDataColumn->minimum();
	double max = xDataColumn->maximum();
	int count = 100;
	xVector->resize(count);
	yVector->resize(count);
	bool rc = parser->evaluateCartesian(fitData.model, QString::number(min), QString::number(max), count, xVector, yVector, fitData.paramNames, fitResult.paramValues);
	if (!rc) {
		xVector->clear();
		yVector->clear();
	}

	//redraw the curve
	emit (q->xDataChanged());
	emit (q->yDataChanged());
	retransform();
}

/*!
 * writes out the current state of the solver \c s
 */
void XYFitCurvePrivate::writeSolverState(gsl_multifit_fdfsolver* s) {
	QString state;

	//current parameter values, semicolon separated
	for (int i=0; i<fitData.paramNames.size(); ++i)
		state += QString::number(gsl_vector_get(s->x, i)) + ";";

	//current value of the function
	state += QString::number(gsl_blas_dnrm2 (s->f));

	fitResult.solverOutput += state;
}


//##############################################################################
//##################  Serialization/Deserialization  ###########################
//##############################################################################
//! Save as XML
void XYFitCurve::save(QXmlStreamWriter* writer) const{
	Q_D(const XYFitCurve);

    writer->writeStartElement( "xyFitCurve" );

	//write xy-curve information
	XYCurve::save(writer);

	//write xy-fit-curve specific information
	writer->writeStartElement( "fitData" );
	WRITE_COLUMN(d->xDataColumn, xDataColumn);
	WRITE_COLUMN(d->yDataColumn, yDataColumn);
	WRITE_COLUMN(d->weightsColumn, weightsColumn);
	//TODO
	writer->writeEndElement();

	writer->writeEndElement();
}

//! Load from XML
bool XYFitCurve::load(XmlStreamReader* reader){
	Q_D(XYFitCurve);

    if(!reader->isStartElement() || reader->name() != "xyFitCurve"){
        reader->raiseError(i18n("no xy fit curve element found"));
        return false;
    }

    QString attributeWarning = i18n("Attribute '%1' missing or empty, default value is used");
    QXmlStreamAttributes attribs;
    QString str;

    while (!reader->atEnd()){
        reader->readNext();
        if (reader->isEndElement() && reader->name() == "xyFitCurve")
            break;

        if (!reader->isStartElement())
            continue;

		if (reader->name() == "xyCurve") {
            if ( !XYCurve::load(reader) )
				return false;
		}else if (reader->name() == "fitData"){
			attribs = reader->attributes();

			READ_COLUMN(xDataColumn);
			READ_COLUMN(yDataColumn);
			READ_COLUMN(weightsColumn);

			str = attribs.value("type").toString();
            if(str.isEmpty())
                reader->raiseWarning(attributeWarning.arg("'type'"));
            else
                d->fitData.modelType = (XYFitCurve::ModelType)str.toInt();
		}
	}

	return true;
}
