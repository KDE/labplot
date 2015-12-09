/***************************************************************************
    File                 : DatapickerCurve.cpp
    Project              : LabPlot
    Description          : container for Curve-Point and Datasheet/Spreadsheet
                           of datapicker
    --------------------------------------------------------------------
    Copyright            : (C) 2015 by Ankit Wagadre (wagadre.ankit@gmail.com)
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

#include "DatapickerCurve.h"
#include "backend/datapicker/DatapickerCurvePrivate.h"
#include "backend/datapicker/Datapicker.h"
#include "backend/datapicker/DatapickerPoint.h"
#include "backend/lib/XmlStreamReader.h"
#include "backend/lib/commandtemplates.h"
#include "backend/spreadsheet/Spreadsheet.h"

#include <QMenu>
#include <QVector3D>

#include <KLocale>
#include <KIcon>
#include <KAction>
#include <KConfigGroup>

/**
 * \class DatapickerCurve
 * \brief Top-level container for Curve-Point and Datasheet/Spreadsheet of datapicker.
 * \ingroup backend
 */

DatapickerCurve::DatapickerCurve(const QString &name) : AbstractAspect(name), d_ptr(new DatapickerCurvePrivate(this)) {

	init();
}

DatapickerCurve::DatapickerCurve(const QString &name, DatapickerCurvePrivate *dd) : AbstractAspect(name), d_ptr(dd) {

	init();
}

DatapickerCurve::~DatapickerCurve() {
}

void DatapickerCurve::init() {
	Q_D(DatapickerCurve);

	KConfig config;
	KConfigGroup group;
	group = config.group("DatapickerCurve");
	d->posXColumn = NULL;
	d->posYColumn = NULL;
	d->posZColumn = NULL;
	d->plusDeltaXColumn = NULL;
	d->minusDeltaXColumn = NULL;
	d->plusDeltaYColumn = NULL;
	d->minusDeltaYColumn = NULL;
	d->curveErrorTypes.x = (ErrorType) group.readEntry("CurveErrorType_X", (int) NoError);
	d->curveErrorTypes.y = (ErrorType) group.readEntry("CurveErrorType_X", (int) NoError);

	this->initAction();
}

void DatapickerCurve::initAction() {
	updateDatasheetAction = new KAction(KIcon("view-refresh"), i18n("Update Spreadsheet"), this);
	connect( updateDatasheetAction, SIGNAL(triggered()), this, SLOT(updateDatasheet()) );
}

/*!
    Returns an icon to be used in the project explorer.
*/
QIcon DatapickerCurve::icon() const {
	return  KIcon("labplot-xy-curve");
}

/*!
    Return a new context menu
*/
QMenu* DatapickerCurve::createContextMenu() {
	QMenu *menu = AbstractAspect::createContextMenu();
	Q_ASSERT(menu);

	QAction* firstAction = 0;
	if (menu->actions().size()>1)
		firstAction = menu->actions().at(1);

	menu->insertAction(firstAction, updateDatasheetAction);

	return menu;
}

Column* DatapickerCurve::appendColumn(const QString& name) {
	Column* col = new Column(i18n("Column"), AbstractColumn::Numeric);
	col->insertRows(0, m_datasheet->rowCount());
	col->setName(name);
	m_datasheet->addChild(col);

	return col;
}
//##############################################################################
//##########################  getter methods  ##################################
//##############################################################################
BASIC_SHARED_D_READER_IMPL(DatapickerCurve, DatapickerCurve::Errors, curveErrorTypes, curveErrorTypes)
BASIC_SHARED_D_READER_IMPL(DatapickerCurve, AbstractColumn*, posXColumn, posXColumn)
QString& DatapickerCurve::posXColumnPath() const {
	return d_ptr->posXColumnPath;
}
BASIC_SHARED_D_READER_IMPL(DatapickerCurve, AbstractColumn*, posYColumn, posYColumn)
QString& DatapickerCurve::posYColumnPath() const {
	return d_ptr->posYColumnPath;
}
BASIC_SHARED_D_READER_IMPL(DatapickerCurve, AbstractColumn*, posZColumn, posZColumn)
QString& DatapickerCurve::posZColumnPath() const {
	return d_ptr->posZColumnPath;
}
BASIC_SHARED_D_READER_IMPL(DatapickerCurve, AbstractColumn*, plusDeltaXColumn, plusDeltaXColumn)
QString& DatapickerCurve::plusDeltaXColumnPath() const {
	return d_ptr->plusDeltaXColumnPath;
}
BASIC_SHARED_D_READER_IMPL(DatapickerCurve, AbstractColumn*, minusDeltaXColumn, minusDeltaXColumn)
QString& DatapickerCurve::minusDeltaXColumnPath() const {
	return d_ptr->minusDeltaXColumnPath;
}
BASIC_SHARED_D_READER_IMPL(DatapickerCurve, AbstractColumn*, plusDeltaYColumn, plusDeltaYColumn)
QString& DatapickerCurve::plusDeltaYColumnPath() const {
	return d_ptr->plusDeltaYColumnPath;
}
BASIC_SHARED_D_READER_IMPL(DatapickerCurve, AbstractColumn*, minusDeltaYColumn, minusDeltaYColumn)
QString& DatapickerCurve::minusDeltaYColumnPath() const {
	return d_ptr->minusDeltaYColumnPath;
}

//##############################################################################
//#########################  setter methods  ###################################
//##############################################################################
void DatapickerCurve::addDatasheet(const DatapickerImage::GraphType& type) {
	Q_D(DatapickerCurve);

	m_datasheet = new Spreadsheet(0, i18n("Data"));
	addChild(m_datasheet);
	QString xLabel = "x";
	QString yLabel = "y";

	if (type == DatapickerImage::PolarInDegree) {
		xLabel = "r";
		yLabel = "y(deg)";
	} else if (type == DatapickerImage::PolarInRadians) {
		xLabel = "r";
		yLabel = "y(rad)";
	} else if (type == DatapickerImage::LogarithmicX) {
		xLabel = "log(x)";
		yLabel = "y";
	} else if (type == DatapickerImage::LogarithmicY) {
		xLabel = "x";
		yLabel = "log(y)";
	}

	if (type == DatapickerImage::Ternary)
		d->posZColumn = appendColumn(i18n("c"));

	d->posXColumn = m_datasheet->column(0);
	d->posXColumn->setName(xLabel);

	d->posYColumn = m_datasheet->column(1);
	d->posYColumn->setName(yLabel);
}

STD_SETTER_CMD_IMPL_S(DatapickerCurve, SetCurveErrorTypes, DatapickerCurve::Errors, curveErrorTypes)
void DatapickerCurve::setCurveErrorTypes(const DatapickerCurve::Errors errors) {
	Q_D(DatapickerCurve);
	if (d->curveErrorTypes.x != errors.x || d->curveErrorTypes.y != errors.y) {
		beginMacro(i18n("%1: set xy-error type", name()));
		exec(new DatapickerCurveSetCurveErrorTypesCmd(d, errors, i18n("%1: set xy-error type")));

		if ( errors.x != NoError && !d->plusDeltaXColumn ) {
			setPlusDeltaXColumn(appendColumn(i18n("+delta_x")));
		} else if ( d->plusDeltaXColumn && errors.x == NoError ) {
			d->plusDeltaXColumn->remove();
			d->plusDeltaXColumn = 0;
		}

		if ( errors.x == AsymmetricError && !d->minusDeltaXColumn ) {
			setMinusDeltaXColumn(appendColumn(i18n("-delta_x")));
		} else if ( d->minusDeltaXColumn && errors.x != AsymmetricError ) {
			d->minusDeltaXColumn->remove();
			d->minusDeltaXColumn = 0;
		}

		if ( errors.y != NoError && !d->plusDeltaYColumn ) {
			setPlusDeltaYColumn(appendColumn(i18n("+delta_y")));
		} else if ( d->plusDeltaYColumn && errors.y == NoError ) {
			d->plusDeltaYColumn->remove();
			d->plusDeltaYColumn = 0;
		}

		if ( errors.y == AsymmetricError && !d->minusDeltaYColumn ) {
			setMinusDeltaYColumn(appendColumn(i18n("-delta_y")));
		} else if ( d->minusDeltaYColumn && errors.y != AsymmetricError ) {
			d->minusDeltaYColumn->remove();
			d->minusDeltaYColumn = 0;
		}

		endMacro();
	}
}

STD_SETTER_CMD_IMPL_S(DatapickerCurve, SetPosXColumn, AbstractColumn*, posXColumn)
void DatapickerCurve::setPosXColumn(AbstractColumn* column) {
	Q_D(DatapickerCurve);
	if (d->posXColumn != column)
		exec(new DatapickerCurveSetPosXColumnCmd(d, column, i18n("%1: set position X column")));
}

STD_SETTER_CMD_IMPL_S(DatapickerCurve, SetPosYColumn, AbstractColumn*, posYColumn)
void DatapickerCurve::setPosYColumn(AbstractColumn* column) {
	Q_D(DatapickerCurve);
	if (d->posYColumn != column)
		exec(new DatapickerCurveSetPosYColumnCmd(d, column, i18n("%1: set position Y column")));
}

STD_SETTER_CMD_IMPL_S(DatapickerCurve, SetPosZColumn, AbstractColumn*, posZColumn)
void DatapickerCurve::setPosZColumn(AbstractColumn* column) {
	Q_D(DatapickerCurve);
	if (d->posZColumn != column)
		exec(new DatapickerCurveSetPosZColumnCmd(d, column, i18n("%1: set position Z column")));
}

STD_SETTER_CMD_IMPL_S(DatapickerCurve, SetPlusDeltaXColumn, AbstractColumn*, plusDeltaXColumn)
void DatapickerCurve::setPlusDeltaXColumn(AbstractColumn* column) {
	Q_D(DatapickerCurve);
	if (d->plusDeltaXColumn != column)
		exec(new DatapickerCurveSetPlusDeltaXColumnCmd(d, column, i18n("%1: set +delta_X column")));
}

STD_SETTER_CMD_IMPL_S(DatapickerCurve, SetMinusDeltaXColumn, AbstractColumn*, minusDeltaXColumn)
void DatapickerCurve::setMinusDeltaXColumn(AbstractColumn* column) {
	Q_D(DatapickerCurve);
	if (d->minusDeltaXColumn != column)
		exec(new DatapickerCurveSetMinusDeltaXColumnCmd(d, column, i18n("%1: set -delta_X column")));
}

STD_SETTER_CMD_IMPL_S(DatapickerCurve, SetPlusDeltaYColumn, AbstractColumn*, plusDeltaYColumn)
void DatapickerCurve::setPlusDeltaYColumn(AbstractColumn* column) {
	Q_D(DatapickerCurve);
	if (d->plusDeltaYColumn != column)
		exec(new DatapickerCurveSetPlusDeltaYColumnCmd(d, column, i18n("%1: set +delta_Y column")));
}

STD_SETTER_CMD_IMPL_S(DatapickerCurve, SetMinusDeltaYColumn, AbstractColumn*, minusDeltaYColumn)
void DatapickerCurve::setMinusDeltaYColumn(AbstractColumn* column) {
	Q_D(DatapickerCurve);
	if (d->minusDeltaYColumn != column)
		exec(new DatapickerCurveSetMinusDeltaYColumnCmd(d, column, i18n("%1: set -delta_Y column")));
}

void DatapickerCurve::setPrinting(bool on) {
	foreach (WorksheetElement* elem, children<WorksheetElement>(IncludeHidden))
		elem->setPrinting(on);
}

/*!
    Selects or deselects the Datapicker/Curve in the project explorer.
    This function is called in \c DatapickerImageView.
*/
void DatapickerCurve::setSelectedInView(const bool b) {
	if (b)
		emit childAspectSelectedInView(this);
	else
		emit childAspectDeselectedInView(this);
}
//##############################################################################
//######  SLOTs for changes triggered via QActions in the context menu  ########
//##############################################################################
void DatapickerCurve::updateDatasheet() {
	beginMacro(i18n("%1: update datasheet", name()));

	foreach (DatapickerPoint* point, children<DatapickerPoint>(IncludeHidden))
		updateData(point);

	endMacro();
}

/*!
    Update datasheet for corresponding curve-point,
    it is called every time whenever there is any change in position
    of curve-point or its error-bar so keep it undo unaware
    no need to create extra entry in undo stack
*/
void DatapickerCurve::updateData(const DatapickerPoint* point) {
	Q_D(DatapickerCurve);
	Datapicker* datapicker = dynamic_cast<Datapicker*>(parentAspect());
	if (!datapicker)
		return;

	int row = indexOfChild<DatapickerPoint>(point ,AbstractAspect::IncludeHidden);
	QVector3D data = datapicker->mapSceneToLogical(point->position().point);

	if(d->posXColumn) {
		d->posXColumn->setUndoAware(false);
		d->posXColumn->setValueAt(row, data.x());
		d->posXColumn->setUndoAware(true);
	}

	if(d->posYColumn) {
		d->posYColumn->setUndoAware(false);
		d->posYColumn->setValueAt(row, data.y());
		d->posYColumn->setUndoAware(true);
	}

	if(d->posZColumn) {
		d->posZColumn->setUndoAware(false);
		d->posZColumn->setValueAt(row, data.y());
		d->posZColumn->setUndoAware(true);
	}

	if (d->plusDeltaXColumn) {
		data = datapicker->mapSceneLengthToLogical(QPointF(point->plusDeltaXPos().x(), 0));
		d->plusDeltaXColumn->setUndoAware(false);
		d->plusDeltaXColumn->setValueAt(row, qAbs(data.x()));
		d->plusDeltaXColumn->setUndoAware(true);
	}

	if (d->minusDeltaXColumn) {
		data = datapicker->mapSceneLengthToLogical(QPointF(point->minusDeltaXPos().x(), 0));
		d->minusDeltaXColumn->setUndoAware(false);
		d->minusDeltaXColumn->setValueAt(row, qAbs(data.x()));
		d->minusDeltaXColumn->setUndoAware(true);
	}

	if (d->plusDeltaYColumn) {
		data = datapicker->mapSceneLengthToLogical(QPointF(0, point->plusDeltaYPos().y()));
		d->plusDeltaYColumn->setUndoAware(false);
		d->plusDeltaYColumn->setValueAt(row, qAbs(data.y()));
		d->plusDeltaYColumn->setUndoAware(true);
	}

	if (d->minusDeltaYColumn) {
		data = datapicker->mapSceneLengthToLogical(QPointF(0, point->minusDeltaYPos().y()));
		d->minusDeltaYColumn->setUndoAware(false);
		d->minusDeltaYColumn->setValueAt(row, qAbs(data.y()));
		d->minusDeltaYColumn->setUndoAware(true);
	}
}

//##############################################################################
//##################  Serialization/Deserialization  ###########################
//##############################################################################
//! Save as XML
void DatapickerCurve::save(QXmlStreamWriter* writer) const {
	Q_D(const DatapickerCurve);

	writer->writeStartElement( "datapickerCurve" );
	writeBasicAttributes(writer);
	writeCommentElement(writer);

	//general
	writer->writeStartElement( "general" );
	WRITE_COLUMN(d->posXColumn, posXColumn);
	WRITE_COLUMN(d->posYColumn, posYColumn);
	WRITE_COLUMN(d->posZColumn, posZColumn);
	WRITE_COLUMN(d->plusDeltaXColumn, plusDeltaXColumn);
	WRITE_COLUMN(d->minusDeltaXColumn, minusDeltaXColumn);
	WRITE_COLUMN(d->plusDeltaYColumn, plusDeltaYColumn);
	WRITE_COLUMN(d->minusDeltaYColumn, minusDeltaYColumn);
	writer->writeAttribute( "curveErrorType_X", QString::number(d->curveErrorTypes.x) );
	writer->writeAttribute( "curveErrorType_Y", QString::number(d->curveErrorTypes.y) );
	writer->writeEndElement();

	//serialize all children
	QList<AbstractAspect *> childrenAspect = children<AbstractAspect>(IncludeHidden);
	foreach(AbstractAspect *child, childrenAspect)
		child->save(writer);

	writer->writeEndElement(); // close section
}

//! Load from XML
bool DatapickerCurve::load(XmlStreamReader* reader) {
	Q_D(DatapickerCurve);

	if(!reader->isStartElement() || reader->name() != "datapickerCurve") {
		reader->raiseError(i18n("no dataPicker curve element found"));
		return false;
	}

	if (!readBasicAttributes(reader))
		return false;

	QString attributeWarning = i18n("Attribute '%1' missing or empty, default value is used");
	QXmlStreamAttributes attribs;
	QString str;

	while (!reader->atEnd()) {
		reader->readNext();
		if (reader->isEndElement() && reader->name() == "datapickerCurve")
			break;

		if (!reader->isStartElement())
			continue;

		if (reader->name() == "comment") {
			if (!readCommentElement(reader)) return false;
		} else if (reader->name() == "general") {
			attribs = reader->attributes();

			str = attribs.value("curveErrorType_X").toString();
			if(str.isEmpty())
				reader->raiseWarning(attributeWarning.arg("curveErrorType_X"));
			else
				d->curveErrorTypes.x = ErrorType(str.toInt());

			str = attribs.value("curveErrorType_Y").toString();
			if(str.isEmpty())
				reader->raiseWarning(attributeWarning.arg("curveErrorType_Y"));
			else
				d->curveErrorTypes.y = ErrorType(str.toInt());

			READ_COLUMN(posXColumn);
			READ_COLUMN(posYColumn);
			READ_COLUMN(posZColumn);
			READ_COLUMN(plusDeltaXColumn);
			READ_COLUMN(minusDeltaXColumn);
			READ_COLUMN(plusDeltaYColumn);
			READ_COLUMN(minusDeltaYColumn);

		} else if (reader->name() == "datapickerPoint") {
			DatapickerPoint* curvePoint = new DatapickerPoint("");
			curvePoint->setHidden(true);
			if (!curvePoint->load(reader)) {
				delete curvePoint;
				return false;
			} else {
				addChild(curvePoint);
				curvePoint->initErrorBar(curveErrorTypes());
			}
		} else if (reader->name() == "spreadsheet") {
			Spreadsheet* datasheet = new Spreadsheet(0, "spreadsheet", true);
			if (!datasheet->load(reader)) {
				delete datasheet;
				return false;
			} else {
				addChild(datasheet);
				m_datasheet = datasheet;
			}
		} else { // unknown element
			reader->raiseWarning(i18n("unknown element '%1'", reader->name().toString()));
			if (!reader->skipToEndElement()) return false;
		}
	}

	return true;
}
