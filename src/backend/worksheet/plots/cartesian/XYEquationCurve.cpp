/*
    File                 : XYEquationCurve.cpp
    Project              : LabPlot
    Description          : A xy-curve defined by a mathematical equation
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2014-2017 Alexander Semke <alexander.semke@web.de>
    SPDX-License-Identifier: GPL-2.0-or-later
*/


/*!
  \class XYEquationCurve
  \brief A xy-curve defined by a mathematical equation

  \ingroup worksheet
*/

#include "XYEquationCurve.h"
#include "XYEquationCurvePrivate.h"
#include "backend/core/AbstractColumn.h"
#include "backend/core/column/Column.h"
#include "backend/lib/commandtemplates.h"
#include "backend/gsl/ExpressionParser.h"
#include "backend/worksheet/plots/cartesian/Symbol.h"

#include <QIcon>
#include <KLocalizedString>

XYEquationCurve::XYEquationCurve(const QString& name)
	: XYCurve(name, new XYEquationCurvePrivate(this), AspectType::XYEquationCurve) {

	init();
}

XYEquationCurve::XYEquationCurve(const QString& name, XYEquationCurvePrivate* dd)
	: XYCurve(name, dd, AspectType::XYEquationCurve) {

	init();
}

//no need to delete the d-pointer here - it inherits from QGraphicsItem
//and is deleted during the cleanup in QGraphicsScene
XYEquationCurve::~XYEquationCurve() = default;

void XYEquationCurve::init() {
	Q_D(XYEquationCurve);

	d->xColumn->setHidden(true);
	addChildFast(d->xColumn);

	d->yColumn->setHidden(true);
	addChildFast(d->yColumn);

	//TODO: read from the saved settings for XYEquationCurve?
	d->lineType = XYCurve::LineType::Line;
	d->symbol->setStyle(Symbol::Style::NoSymbols);

	setUndoAware(false);
	suppressRetransform(true);
	setXColumn(d->xColumn);
	setYColumn(d->yColumn);
	suppressRetransform(false);
	setUndoAware(true);
}

void XYEquationCurve::recalculate() {
	Q_D(XYEquationCurve);
	d->recalculate();
}

/*!
	Returns an icon to be used in the project explorer.
*/
QIcon XYEquationCurve::icon() const {
	return QIcon::fromTheme("labplot-xy-equation-curve");
}

//##############################################################################
//##########################  getter methods  ##################################
//##############################################################################
BASIC_SHARED_D_READER_IMPL(XYEquationCurve, XYEquationCurve::EquationData, equationData, equationData)

//##############################################################################
//#################  setter methods and undo commands ##########################
//##############################################################################
STD_SETTER_CMD_IMPL_F_S(XYEquationCurve, SetEquationData, XYEquationCurve::EquationData, equationData, recalculate);
void XYEquationCurve::setEquationData(const XYEquationCurve::EquationData& equationData) {
	Q_D(XYEquationCurve);
	if ( (equationData.expression1 != d->equationData.expression1)
		|| (equationData.expression2 != d->equationData.expression2)
		|| (equationData.min != d->equationData.min)
		|| (equationData.max != d->equationData.max)
		|| (equationData.count != d->equationData.count) )
		exec(new XYEquationCurveSetEquationDataCmd(d, equationData, ki18n("%1: set equation")));
}

//##############################################################################
//######################### Private implementation #############################
//##############################################################################
XYEquationCurvePrivate::XYEquationCurvePrivate(XYEquationCurve* owner) : XYCurvePrivate(owner),
	xColumn(new Column("x", AbstractColumn::ColumnMode::Numeric)),
	yColumn(new Column("y", AbstractColumn::ColumnMode::Numeric)),
	xVector(static_cast<QVector<double>* >(xColumn->data())),
	yVector(static_cast<QVector<double>* >(yColumn->data())),
	q(owner)  {

}

//no need to delete xColumn and yColumn, they are deleted
//when the parent aspect is removed
XYEquationCurvePrivate::~XYEquationCurvePrivate() = default;

void XYEquationCurvePrivate::recalculate() {
	//resize the vector if a new number of point to calculate was provided
	if (equationData.count != xVector->size()) {
		if (equationData.count >= 1) {
			xVector->resize(equationData.count);
			yVector->resize(equationData.count);
		} else {
			//invalid number of points provided
			xVector->clear();
			yVector->clear();
			recalcLogicalPoints();
			emit q->dataChanged();
			return;
		}
		xColumn->invalidateProperties();
		yColumn->invalidateProperties();
	} else {
		if (equationData.count < 1)
			return;
	}

	ExpressionParser* parser = ExpressionParser::getInstance();
	bool rc = false;
	if (equationData.type == XYEquationCurve::EquationType::Cartesian) {
		rc = parser->evaluateCartesian( equationData.expression1, equationData.min, equationData.max,
						equationData.count, xVector, yVector );
	} else if (equationData.type == XYEquationCurve::EquationType::Polar) {
		rc = parser->evaluatePolar( equationData.expression1, equationData.min, equationData.max,
						equationData.count, xVector, yVector );
	} else if (equationData.type == XYEquationCurve::EquationType::Parametric) {
		rc = parser->evaluateParametric(equationData.expression1, equationData.expression2,
						equationData.min, equationData.max, equationData.count,
						xVector, yVector);
	}

	if (!rc) {
		xVector->clear();
		xColumn->invalidateProperties();
		yVector->clear();
		yColumn->invalidateProperties();
	}

	recalcLogicalPoints();
	emit q->dataChanged();
}

//##############################################################################
//##################  Serialization/Deserialization  ###########################
//##############################################################################
//! Save as XML
void XYEquationCurve::save(QXmlStreamWriter* writer) const{
	Q_D(const XYEquationCurve);

	writer->writeStartElement("xyEquationCurve");

	//write xy-curve information
	XYCurve::save(writer);

	//write xy-equationCurve specific information
	writer->writeStartElement("equationData");
	writer->writeAttribute( "type", QString::number(static_cast<int>(d->equationData.type)) );
	writer->writeAttribute( "expression1", d->equationData.expression1 );
	writer->writeAttribute( "expression2", d->equationData.expression2 );
	writer->writeAttribute( "min", d->equationData.min);
	writer->writeAttribute( "max", d->equationData.max );
	writer->writeAttribute( "count", QString::number(d->equationData.count) );
	writer->writeEndElement();

	writer->writeEndElement();
}

//! Load from XML
bool XYEquationCurve::load(XmlStreamReader* reader, bool preview) {
	Q_D(XYEquationCurve);

	KLocalizedString attributeWarning = ki18n( "Attribute '%1' missing or empty, default value is used" );
	QXmlStreamAttributes attribs;
	QString str;

	while (!reader->atEnd()) {
		reader->readNext();
		if (reader->isEndElement() && reader->name() == "xyEquationCurve")
			break;

		if (!reader->isStartElement())
			continue;

		if (reader->name() == "xyCurve") {
			if ( !XYCurve::load(reader, preview) )
				return false;
		} else if (!preview && reader->name() == "equationData") {
			attribs = reader->attributes();

			READ_INT_VALUE("type", equationData.type, XYEquationCurve::EquationType);
			READ_STRING_VALUE("expression1", equationData.expression1);
			READ_STRING_VALUE("expression2", equationData.expression2);
			READ_STRING_VALUE("min", equationData.min);
			READ_STRING_VALUE("max", equationData.max);
			READ_INT_VALUE("count", equationData.count, int);
		}
	}

	return true;
}
