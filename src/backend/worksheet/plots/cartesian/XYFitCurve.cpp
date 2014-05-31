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
BASIC_SHARED_D_READER_IMPL(XYFitCurve, XYFitCurve::FitData, fitData, fitData)

//##############################################################################
//#################  setter methods and undo commands ##########################
//##############################################################################
STD_SETTER_CMD_IMPL_F_S(XYFitCurve, SetFitData, XYFitCurve::FitData, fitData, recalculate);
void XYFitCurve::setFitData(const XYFitCurve::FitData& fitData) {
	Q_D(XYFitCurve);
}

//##############################################################################
//######################### Private implementation #############################
//##############################################################################
XYFitCurvePrivate::XYFitCurvePrivate(XYFitCurve* owner) : XYCurvePrivate(owner),
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

void XYFitCurvePrivate::recalculate() {
	//TODO
// 	emit (q->xDataChanged());
// 	emit (q->yDataChanged());
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

			str = attribs.value("type").toString();
            if(str.isEmpty())
                reader->raiseWarning(attributeWarning.arg("'type'"));
            else
                d->fitData.type = (XYFitCurve::FitType)str.toInt();
		}
	}

	return true;
}
