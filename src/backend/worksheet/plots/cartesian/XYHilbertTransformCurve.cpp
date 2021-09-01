/*
    File                 : XYHilbertTransformCurve.cpp
    Project              : LabPlot
    Description          : A xy-curve defined by a Hilbert transform
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2021 Stefan Gerlach (stefan.gerlach@uni.kn)

*/

/***************************************************************************
 *                                                                         *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *                                                                         *
 ***************************************************************************/

/*!
  \class XYHilbertTransformCurve
  \brief A xy-curve defined by a Hilbert transform

  \ingroup worksheet
*/

#include "XYHilbertTransformCurve.h"
#include "XYHilbertTransformCurvePrivate.h"
#include "backend/core/AbstractColumn.h"
#include "backend/core/column/Column.h"
#include "backend/lib/commandtemplates.h"
#include "backend/lib/macros.h"
#include "backend/gsl/errors.h"

extern "C" {
//#include "backend/nsl/nsl_sf_poly.h"
}

#include <KLocalizedString>
#include <QElapsedTimer>
#include <QIcon>
#include <QThreadPool>
#include <QDebug>	// qWarning()

XYHilbertTransformCurve::XYHilbertTransformCurve(const QString& name)
	: XYAnalysisCurve(name, new XYHilbertTransformCurvePrivate(this), AspectType::XYHilbertTransformCurve) {
}

XYHilbertTransformCurve::XYHilbertTransformCurve(const QString& name, XYHilbertTransformCurvePrivate* dd)
	: XYAnalysisCurve(name, dd, AspectType::XYHilbertTransformCurve) {
}

//no need to delete the d-pointer here - it inherits from QGraphicsItem
//and is deleted during the cleanup in QGraphicsScene
XYHilbertTransformCurve::~XYHilbertTransformCurve() = default;

void XYHilbertTransformCurve::recalculate() {
	Q_D(XYHilbertTransformCurve);
	d->recalculate();
}

/*!
	Returns an icon to be used in the project explorer.
*/
QIcon XYHilbertTransformCurve::icon() const {
	return QIcon::fromTheme("labplot-xy-fourier-transform-curve");
}

//##############################################################################
//##########################  getter methods  ##################################
//##############################################################################
BASIC_SHARED_D_READER_IMPL(XYHilbertTransformCurve, XYHilbertTransformCurve::TransformData, transformData, transformData)

const XYHilbertTransformCurve::TransformResult& XYHilbertTransformCurve::transformResult() const {
	Q_D(const XYHilbertTransformCurve);
	return d->transformResult;
}

//##############################################################################
//#################  setter methods and undo commands ##########################
//##############################################################################
STD_SETTER_CMD_IMPL_F_S(XYHilbertTransformCurve, SetTransformData, XYHilbertTransformCurve::TransformData, transformData, recalculate);
void XYHilbertTransformCurve::setTransformData(const XYHilbertTransformCurve::TransformData& transformData) {
	Q_D(XYHilbertTransformCurve);
	exec(new XYHilbertTransformCurveSetTransformDataCmd(d, transformData, ki18n("%1: set transform options and perform the Hilbert transform")));
}

//##############################################################################
//######################### Private implementation #############################
//##############################################################################
XYHilbertTransformCurvePrivate::XYHilbertTransformCurvePrivate(XYHilbertTransformCurve* owner) : XYAnalysisCurvePrivate(owner), q(owner) {
}

//no need to delete xColumn and yColumn, they are deleted
//when the parent aspect is removed
XYHilbertTransformCurvePrivate::~XYHilbertTransformCurvePrivate() = default;

void XYHilbertTransformCurvePrivate::recalculate() {
	DEBUG(Q_FUNC_INFO)
	QElapsedTimer timer;
	timer.start();

	//create transform result columns if not available yet, clear them otherwise
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
	transformResult = XYHilbertTransformCurve::TransformResult();

	if (!xDataColumn || !yDataColumn) {
		recalcLogicalPoints();
		emit q->dataChanged();
		sourceDataChangedSinceLastRecalc = false;
		DEBUG(Q_FUNC_INFO << "no data columns!")
		return;
	}

	//copy all valid data point for the transform to temporary vectors
	QVector<double> xdataVector;
	QVector<double> ydataVector;
	double xmin, xmax;
	if (xDataColumn && transformData.autoRange) {
		xmin = xDataColumn->minimum();
		xmax = xDataColumn->maximum();
	} else {
		xmin = transformData.xRange.first();
		xmax = transformData.xRange.last();
	}

	int rowCount = qMin(xDataColumn->rowCount(), yDataColumn->rowCount());
	DEBUG(Q_FUNC_INFO << ", row count = " << rowCount)
	DEBUG(Q_FUNC_INFO << ", xmin/xmax = " << xmin << '/' << xmax)
	for (int row = 0; row < rowCount; ++row) {
		// only copy those data where _all_ values (for x and y, if given) are valid
		if (std::isnan(xDataColumn->valueAt(row)) || std::isnan(yDataColumn->valueAt(row))
				|| xDataColumn->isMasked(row) || yDataColumn->isMasked(row))
			continue;

		// only when inside given range
		if (xDataColumn->valueAt(row) >= xmin && xDataColumn->valueAt(row) <= xmax) {
			xdataVector.append(xDataColumn->valueAt(row));
			ydataVector.append(yDataColumn->valueAt(row));
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
		DEBUG(Q_FUNC_INFO << "no data (n = 0)!")
		return;
	}

	double* xdata = xdataVector.data();
	double* ydata = ydataVector.data();

	// transform settings
	const nsl_hilbert_result_type type = transformData.type;

	DEBUG("n = " << n);
	DEBUG("type:" << nsl_hilbert_result_type_name[type]);
#ifndef NDEBUG
//	QDebug out = qDebug();
//	for (unsigned int i = 0; i < n; i++)
//		out<<ydata[i];
#endif

///////////////////////////////////////////////////////////
	// transform with window
//	TODO: type 
	int status = nsl_hilbert_transform(ydata, 1, n, type);

	unsigned int N = n;
#ifndef NDEBUG
//	out = qDebug();
//	for (unsigned int i = 0; i < N; i++)
//		out << ydata[i] << '(' << xdata[i] << ')';
#endif

	xVector->resize((int)N);
	yVector->resize((int)N);
	memcpy(xVector->data(), xdata, N*sizeof(double));
	memcpy(yVector->data(), ydata, N*sizeof(double));
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
void XYHilbertTransformCurve::save(QXmlStreamWriter* writer) const {
	Q_D(const XYHilbertTransformCurve);

	writer->writeStartElement("xyHilbertTransformCurve");

	//write the base class
	XYAnalysisCurve::save(writer);

	//write xy-fourier_transform-curve specific information
	//transform data
	writer->writeStartElement("transformData");
	writer->writeAttribute( "autoRange", QString::number(d->transformData.autoRange) );
	writer->writeAttribute( "xRangeMin", QString::number(d->transformData.xRange.first()) );
	writer->writeAttribute( "xRangeMax", QString::number(d->transformData.xRange.last()) );
	writer->writeAttribute( "type", QString::number(d->transformData.type) );
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
	writer->writeEndElement(); //"xyHilbertTransformCurve"
}

//! Load from XML
bool XYHilbertTransformCurve::load(XmlStreamReader* reader, bool preview) {
	Q_D(XYHilbertTransformCurve);

	KLocalizedString attributeWarning = ki18n("Attribute '%1' missing or empty, default value is used");
	QXmlStreamAttributes attribs;
	QString str;

	while (!reader->atEnd()) {
		reader->readNext();
		if (reader->isEndElement() && reader->name() == "xyHilbertTransformCurve")
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
			READ_INT_VALUE("type", transformData.type, nsl_hilbert_result_type);
		} else if (!preview && reader->name() == "transformResult") {
			attribs = reader->attributes();
			READ_INT_VALUE("available", transformResult.available, int);
			READ_INT_VALUE("valid", transformResult.valid, int);
			READ_STRING_VALUE("status", transformResult.status);
			READ_INT_VALUE("time", transformResult.elapsedTime, int);
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
