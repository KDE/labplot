/***************************************************************************
    File                 : XYEquationCurve.cpp
    Project              : LabPlot
    Description          : A xy-curve defined by a mathematical equation
    --------------------------------------------------------------------
    Copyright            : (C) 2014-2017 Alexander Semke (alexander.semke@web.de)

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

#include <QIcon>
#include <KLocale>

XYEquationCurve::XYEquationCurve(const QString& name)
		: XYCurve(name, new XYEquationCurvePrivate(this)) {
	init();
}

XYEquationCurve::XYEquationCurve(const QString& name, XYEquationCurvePrivate* dd)
		: XYCurve(name, dd) {
	init();
}

XYEquationCurve::~XYEquationCurve() {
	//no need to delete the d-pointer here - it inherits from QGraphicsItem
	//and is deleted during the cleanup in QGraphicsScene
}

void XYEquationCurve::init() {
	Q_D(XYEquationCurve);

	d->xColumn->setHidden(true);
	addChildFast(d->xColumn);

	d->yColumn->setHidden(true);
	addChildFast(d->yColumn);

	//TODO: read from the saved settings for XYEquationCurve?
	d->lineType = XYCurve::Line;
	d->symbolsStyle = Symbol::NoSymbols;

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
	xColumn(new Column("x", AbstractColumn::Numeric)),
	yColumn(new Column("y", AbstractColumn::Numeric)),
	xVector(static_cast<QVector<double>* >(xColumn->data())),
	yVector(static_cast<QVector<double>* >(yColumn->data())),
	q(owner)  {

}

XYEquationCurvePrivate::~XYEquationCurvePrivate() {
	//no need to delete xColumn and yColumn, they are deleted
	//when the parent aspect is removed
}

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
			emit q->dataChanged();
			return;
		}
	} else {
		if (equationData.count < 1)
			return;
	}

	ExpressionParser* parser = ExpressionParser::getInstance();
	bool rc = false;
	if (equationData.type == XYEquationCurve::Cartesian) {
		rc = parser->evaluateCartesian( equationData.expression1, equationData.min, equationData.max,
						equationData.count, xVector, yVector );
	} else if (equationData.type == XYEquationCurve::Polar) {
		rc = parser->evaluatePolar( equationData.expression1, equationData.min, equationData.max,
						equationData.count, xVector, yVector );
	} else if (equationData.type == XYEquationCurve::Parametric) {
		rc = parser->evaluateParametric(equationData.expression1, equationData.expression2,
						equationData.min, equationData.max, equationData.count,
						xVector, yVector);
	}

	if (!rc) {
		xVector->clear();
		yVector->clear();
	}
	emit q->dataChanged();
}

//##############################################################################
//##################  Serialization/Deserialization  ###########################
//##############################################################################
//! Save as XML
void XYEquationCurve::save(QXmlStreamWriter* writer) const{
	Q_D(const XYEquationCurve);

	writer->writeStartElement( "xyEquationCurve" );

	//write xy-curve information
	XYCurve::save(writer);

	//write xy-equationCurve specific information
	writer->writeStartElement( "equationData" );
	writer->writeAttribute( "type", QString::number(d->equationData.type) );
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

	if (!reader->isStartElement() || reader->name() != "xyEquationCurve") {
		reader->raiseError(i18n("no xy equation curve element found"));
		return false;
	}

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
