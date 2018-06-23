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
#include "backend/lib/XmlStreamReader.h"
#include "backend/lib/commandtemplates.h"
#include "backend/spreadsheet/Spreadsheet.h"
#include "backend/worksheet/Worksheet.h"
#include "backend/datapicker/DatapickerPoint.h"

#include <QMenu>
#include <QVector3D>

#include <KLocale>
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
	delete d_ptr;
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

	// point properties
	d->pointStyle = (Symbol::Style)group.readEntry("PointStyle", (int)Symbol::Cross);
	d->pointSize = group.readEntry("Size", Worksheet::convertToSceneUnits(7, Worksheet::Point));
	d->pointRotationAngle = group.readEntry("Rotation", 0.0);
	d->pointOpacity = group.readEntry("Opacity", 1.0);
	d->pointBrush.setStyle( (Qt::BrushStyle)group.readEntry("FillingStyle", (int)Qt::NoBrush) );
	d->pointBrush.setColor( group.readEntry("FillingColor", QColor(Qt::black)) );
	d->pointPen.setStyle( (Qt::PenStyle)group.readEntry("BorderStyle", (int)Qt::SolidLine) );
	d->pointPen.setColor( group.readEntry("BorderColor", QColor(Qt::red)) );
	d->pointPen.setWidthF( group.readEntry("BorderWidth", Worksheet::convertToSceneUnits(1, Worksheet::Point)) );
	d->pointErrorBarSize = group.readEntry("ErrorBarSize", Worksheet::convertToSceneUnits(8, Worksheet::Point));
	d->pointErrorBarBrush.setStyle( (Qt::BrushStyle)group.readEntry("ErrorBarFillingStyle", (int)Qt::NoBrush) );
	d->pointErrorBarBrush.setColor( group.readEntry("ErrorBarFillingColor", QColor(Qt::black)) );
	d->pointErrorBarPen.setStyle( (Qt::PenStyle)group.readEntry("ErrorBarBorderStyle", (int)Qt::SolidLine) );
	d->pointErrorBarPen.setColor( group.readEntry("ErrorBarBorderColor", QColor(Qt::black)) );
	d->pointErrorBarPen.setWidthF( group.readEntry("ErrorBarBorderWidth", Worksheet::convertToSceneUnits(1, Worksheet::Point)) );
	d->pointVisibility = group.readEntry("PointVisibility", true);

	this->initAction();
}

void DatapickerCurve::initAction() {
	updateDatasheetAction = new QAction(QIcon::fromTheme("view-refresh"), i18n("Update Spreadsheet"), this);
	connect(updateDatasheetAction, &QAction::triggered, this, &DatapickerCurve::updateDatasheet);
}

/*!
    Returns an icon to be used in the project explorer.
*/
QIcon DatapickerCurve::icon() const {
	return  QIcon::fromTheme("labplot-xy-curve");
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
BASIC_SHARED_D_READER_IMPL(DatapickerCurve, Symbol::Style, pointStyle, pointStyle)
BASIC_SHARED_D_READER_IMPL(DatapickerCurve, qreal, pointOpacity, pointOpacity)
BASIC_SHARED_D_READER_IMPL(DatapickerCurve, qreal, pointRotationAngle, pointRotationAngle)
BASIC_SHARED_D_READER_IMPL(DatapickerCurve, qreal, pointSize, pointSize)
CLASS_SHARED_D_READER_IMPL(DatapickerCurve, QBrush, pointBrush, pointBrush)
CLASS_SHARED_D_READER_IMPL(DatapickerCurve, QPen, pointPen, pointPen)
BASIC_SHARED_D_READER_IMPL(DatapickerCurve, qreal, pointErrorBarSize, pointErrorBarSize)
CLASS_SHARED_D_READER_IMPL(DatapickerCurve, QBrush, pointErrorBarBrush, pointErrorBarBrush)
CLASS_SHARED_D_READER_IMPL(DatapickerCurve, QPen, pointErrorBarPen, pointErrorBarPen)
BASIC_SHARED_D_READER_IMPL(DatapickerCurve, bool, pointVisibility, pointVisibility)

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
void DatapickerCurve::addDatasheet(DatapickerImage::GraphType type) {
	Q_D(DatapickerCurve);

	m_datasheet = new Spreadsheet(0, i18n("Data"));
	addChild(m_datasheet);
	QString xLabel('x');
	QString yLabel('y');

	if (type == DatapickerImage::PolarInDegree) {
		xLabel = QLatin1String("r");
		yLabel = QLatin1String("y(deg)");
	} else if (type == DatapickerImage::PolarInRadians) {
		xLabel = QLatin1String("r");
		yLabel = QLatin1String("y(rad)");
	} else if (type == DatapickerImage::LogarithmicX) {
		xLabel = QLatin1String("log(x)");
		yLabel = QLatin1String("y");
	} else if (type == DatapickerImage::LogarithmicY) {
		xLabel = QLatin1String("x");
		yLabel = QLatin1String("log(y)");
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
		exec(new DatapickerCurveSetCurveErrorTypesCmd(d, errors, ki18n("%1: set xy-error type")));

		if ( errors.x != NoError && !d->plusDeltaXColumn )
			setPlusDeltaXColumn(appendColumn(QLatin1String("+delta_x")));
		else if ( d->plusDeltaXColumn && errors.x == NoError ) {
			d->plusDeltaXColumn->remove();
			d->plusDeltaXColumn = 0;
		}

		if ( errors.x == AsymmetricError && !d->minusDeltaXColumn )
			setMinusDeltaXColumn(appendColumn(QLatin1String("-delta_x")));
		else if ( d->minusDeltaXColumn && errors.x != AsymmetricError ) {
			d->minusDeltaXColumn->remove();
			d->minusDeltaXColumn = 0;
		}

		if ( errors.y != NoError && !d->plusDeltaYColumn )
			setPlusDeltaYColumn(appendColumn(QLatin1String("+delta_y")));
		else if ( d->plusDeltaYColumn && errors.y == NoError ) {
			d->plusDeltaYColumn->remove();
			d->plusDeltaYColumn = 0;
		}

		if ( errors.y == AsymmetricError && !d->minusDeltaYColumn )
			setMinusDeltaYColumn(appendColumn(QLatin1String("-delta_y")));
		else if ( d->minusDeltaYColumn && errors.y != AsymmetricError ) {
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
		exec(new DatapickerCurveSetPosXColumnCmd(d, column, ki18n("%1: set position X column")));
}

STD_SETTER_CMD_IMPL_S(DatapickerCurve, SetPosYColumn, AbstractColumn*, posYColumn)
void DatapickerCurve::setPosYColumn(AbstractColumn* column) {
	Q_D(DatapickerCurve);
	if (d->posYColumn != column)
		exec(new DatapickerCurveSetPosYColumnCmd(d, column, ki18n("%1: set position Y column")));
}

STD_SETTER_CMD_IMPL_S(DatapickerCurve, SetPosZColumn, AbstractColumn*, posZColumn)
void DatapickerCurve::setPosZColumn(AbstractColumn* column) {
	Q_D(DatapickerCurve);
	if (d->posZColumn != column)
		exec(new DatapickerCurveSetPosZColumnCmd(d, column, ki18n("%1: set position Z column")));
}

STD_SETTER_CMD_IMPL_S(DatapickerCurve, SetPlusDeltaXColumn, AbstractColumn*, plusDeltaXColumn)
void DatapickerCurve::setPlusDeltaXColumn(AbstractColumn* column) {
	Q_D(DatapickerCurve);
	if (d->plusDeltaXColumn != column)
		exec(new DatapickerCurveSetPlusDeltaXColumnCmd(d, column, ki18n("%1: set +delta_X column")));
}

STD_SETTER_CMD_IMPL_S(DatapickerCurve, SetMinusDeltaXColumn, AbstractColumn*, minusDeltaXColumn)
void DatapickerCurve::setMinusDeltaXColumn(AbstractColumn* column) {
	Q_D(DatapickerCurve);
	if (d->minusDeltaXColumn != column)
		exec(new DatapickerCurveSetMinusDeltaXColumnCmd(d, column, ki18n("%1: set -delta_X column")));
}

STD_SETTER_CMD_IMPL_S(DatapickerCurve, SetPlusDeltaYColumn, AbstractColumn*, plusDeltaYColumn)
void DatapickerCurve::setPlusDeltaYColumn(AbstractColumn* column) {
	Q_D(DatapickerCurve);
	if (d->plusDeltaYColumn != column)
		exec(new DatapickerCurveSetPlusDeltaYColumnCmd(d, column, ki18n("%1: set +delta_Y column")));
}

STD_SETTER_CMD_IMPL_S(DatapickerCurve, SetMinusDeltaYColumn, AbstractColumn*, minusDeltaYColumn)
void DatapickerCurve::setMinusDeltaYColumn(AbstractColumn* column) {
	Q_D(DatapickerCurve);
	if (d->minusDeltaYColumn != column)
		exec(new DatapickerCurveSetMinusDeltaYColumnCmd(d, column, ki18n("%1: set -delta_Y column")));
}

STD_SETTER_CMD_IMPL_F_S(DatapickerCurve, SetPointStyle, Symbol::Style, pointStyle, retransform)
void DatapickerCurve::setPointStyle(Symbol::Style newStyle) {
	Q_D(DatapickerCurve);
	if (newStyle != d->pointStyle)
		exec(new DatapickerCurveSetPointStyleCmd(d, newStyle, ki18n("%1: set point's style")));
}


STD_SETTER_CMD_IMPL_F_S(DatapickerCurve, SetPointSize, qreal, pointSize, retransform)
void DatapickerCurve::setPointSize(qreal value) {
	Q_D(DatapickerCurve);
	if (!qFuzzyCompare(1 + value, 1 + d->pointSize))
		exec(new DatapickerCurveSetPointSizeCmd(d, value, ki18n("%1: set point's size")));
}

STD_SETTER_CMD_IMPL_F_S(DatapickerCurve, SetPointRotationAngle, qreal, pointRotationAngle, retransform)
void DatapickerCurve::setPointRotationAngle(qreal angle) {
	Q_D(DatapickerCurve);
	if (!qFuzzyCompare(1 + angle, 1 + d->pointRotationAngle))
		exec(new DatapickerCurveSetPointRotationAngleCmd(d, angle, ki18n("%1: rotate point")));
}

STD_SETTER_CMD_IMPL_F_S(DatapickerCurve, SetPointBrush, QBrush, pointBrush, retransform)
void DatapickerCurve::setPointBrush(const QBrush& newBrush) {
	Q_D(DatapickerCurve);
	if (newBrush != d->pointBrush)
		exec(new DatapickerCurveSetPointBrushCmd(d, newBrush, ki18n("%1: set point's filling")));
}

STD_SETTER_CMD_IMPL_F_S(DatapickerCurve, SetPointPen, QPen, pointPen, retransform)
void DatapickerCurve::setPointPen(const QPen &newPen) {
	Q_D(DatapickerCurve);
	if (newPen != d->pointPen)
		exec(new DatapickerCurveSetPointPenCmd(d, newPen, ki18n("%1: set outline style")));
}

STD_SETTER_CMD_IMPL_F_S(DatapickerCurve, SetPointOpacity, qreal, pointOpacity, retransform)
void DatapickerCurve::setPointOpacity(qreal newOpacity) {
	Q_D(DatapickerCurve);
	if (newOpacity != d->pointOpacity)
		exec(new DatapickerCurveSetPointOpacityCmd(d, newOpacity, ki18n("%1: set point's opacity")));
}

STD_SETTER_CMD_IMPL_F_S(DatapickerCurve, SetPointErrorBarSize, qreal, pointErrorBarSize, retransform)
void DatapickerCurve::setPointErrorBarSize(qreal size) {
	Q_D(DatapickerCurve);
	if (size != d->pointErrorBarSize)
		exec(new DatapickerCurveSetPointErrorBarSizeCmd(d, size, ki18n("%1: set error bar size")));
}

STD_SETTER_CMD_IMPL_F_S(DatapickerCurve, SetPointErrorBarBrush, QBrush, pointErrorBarBrush, retransform)
void DatapickerCurve::setPointErrorBarBrush(const QBrush &brush) {
	Q_D(DatapickerCurve);
	if (brush != d->pointErrorBarBrush)
		exec(new DatapickerCurveSetPointErrorBarBrushCmd(d, brush, ki18n("%1: set error bar filling")));
}

STD_SETTER_CMD_IMPL_F_S(DatapickerCurve, SetPointErrorBarPen, QPen, pointErrorBarPen, retransform)
void DatapickerCurve::setPointErrorBarPen(const QPen &pen) {
	Q_D(DatapickerCurve);
	if (pen != d->pointErrorBarPen)
		exec(new DatapickerCurveSetPointErrorBarPenCmd(d, pen, ki18n("%1: set error bar outline style")));
}

STD_SETTER_CMD_IMPL_F_S(DatapickerCurve, SetPointVisibility, bool, pointVisibility, retransform)
void DatapickerCurve::setPointVisibility(bool on) {
	Q_D(DatapickerCurve);
	if (on != d->pointVisibility)
		exec(new DatapickerCurveSetPointVisibilityCmd(d, on, on ? ki18n("%1: set visible") : ki18n("%1: set invisible")));
}

void DatapickerCurve::setPrinting(bool on) {
	for (auto* point : children<DatapickerPoint>(IncludeHidden))
		point->setPrinting(on);
}

/*!
    Selects or deselects the Datapicker/Curve in the project explorer.
    This function is called in \c DatapickerImageView.
*/
void DatapickerCurve::setSelectedInView(bool b) {
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

	for (auto* point : children<DatapickerPoint>(IncludeHidden))
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

	int row = indexOfChild<DatapickerPoint>(point, AbstractAspect::IncludeHidden);
	QVector3D data = datapicker->mapSceneToLogical(point->position());

	if(d->posXColumn)
		d->posXColumn->setValueAt(row, data.x());

	if(d->posYColumn)
		d->posYColumn->setValueAt(row, data.y());

	if(d->posZColumn)
		d->posZColumn->setValueAt(row, data.y());

	if (d->plusDeltaXColumn) {
		data = datapicker->mapSceneLengthToLogical(QPointF(point->plusDeltaXPos().x(), 0));
		d->plusDeltaXColumn->setValueAt(row, qAbs(data.x()));
	}

	if (d->minusDeltaXColumn) {
		data = datapicker->mapSceneLengthToLogical(QPointF(point->minusDeltaXPos().x(), 0));
		d->minusDeltaXColumn->setValueAt(row, qAbs(data.x()));
	}

	if (d->plusDeltaYColumn) {
		data = datapicker->mapSceneLengthToLogical(QPointF(0, point->plusDeltaYPos().y()));
		d->plusDeltaYColumn->setValueAt(row, qAbs(data.y()));
	}

	if (d->minusDeltaYColumn) {
		data = datapicker->mapSceneLengthToLogical(QPointF(0, point->minusDeltaYPos().y()));
		d->minusDeltaYColumn->setValueAt(row, qAbs(data.y()));
	}
}

//##############################################################################
//####################### Private implementation ###############################
//##############################################################################
DatapickerCurvePrivate::DatapickerCurvePrivate(DatapickerCurve *curve) : q(curve) {

}

QString DatapickerCurvePrivate::name() const {
	return q->name();
}

void DatapickerCurvePrivate::retransform() {
	QVector<DatapickerPoint*> childrenPoints = q->children<DatapickerPoint>(AbstractAspect::IncludeHidden);
	for (auto* point : childrenPoints)
		point->retransform();
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

	//symbol properties
	writer->writeStartElement( "symbolProperties" );
	writer->writeAttribute( "pointRotationAngle", QString::number(d->pointRotationAngle) );
	writer->writeAttribute( "pointOpacity", QString::number(d->pointOpacity) );
	writer->writeAttribute( "pointSize", QString::number(d->pointSize) );
	writer->writeAttribute( "pointStyle", QString::number(d->pointStyle) );
	writer->writeAttribute( "pointVisibility", QString::number(d->pointVisibility) );
	WRITE_QBRUSH(d->pointBrush);
	WRITE_QPEN(d->pointPen);
	writer->writeEndElement();

	//error bar properties
	writer->writeStartElement( "errorBarProperties" );
	writer->writeAttribute( "pointErrorBarSize", QString::number(d->pointErrorBarSize) );
	WRITE_QBRUSH(d->pointErrorBarBrush);
	WRITE_QPEN(d->pointErrorBarPen);
	writer->writeEndElement();

	//serialize all children
	for (auto* child : children<AbstractAspect>(IncludeHidden))
		child->save(writer);

	writer->writeEndElement(); // close section
}

//! Load from XML
bool DatapickerCurve::load(XmlStreamReader* reader, bool preview) {
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
		} else if (!preview && reader->name() == "general") {
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
		} else if(!preview && reader->name() == "symbolProperties") {
			attribs = reader->attributes();

			str = attribs.value("pointRotationAngle").toString();
			if(str.isEmpty())
				reader->raiseWarning(attributeWarning.arg("pointRotationAngle"));
			else
				d->pointRotationAngle = str.toFloat();

			str = attribs.value("pointOpacity").toString();
			if(str.isEmpty())
				reader->raiseWarning(attributeWarning.arg("pointOpacity"));
			else
				d->pointOpacity = str.toFloat();

			str = attribs.value("pointSize").toString();
			if(str.isEmpty())
				reader->raiseWarning(attributeWarning.arg("pointSize"));
			else
				d->pointSize = str.toFloat();

			str = attribs.value("pointStyle").toString();
			if(str.isEmpty())
				reader->raiseWarning(attributeWarning.arg("pointStyle"));
			else
				d->pointStyle = (Symbol::Style)str.toInt();

			str = attribs.value("pointVisibility").toString();
			if(str.isEmpty())
				reader->raiseWarning(attributeWarning.arg("pointVisibility"));
			else
				d->pointVisibility = (bool)str.toInt();

			READ_QBRUSH(d->pointBrush);
			READ_QPEN(d->pointPen);
		} else if(!preview && reader->name() == "errorBarProperties") {
			attribs = reader->attributes();

			str = attribs.value("pointErrorBarSize").toString();
			if(str.isEmpty())
				reader->raiseWarning(attributeWarning.arg("pointErrorBarSize"));
			else
				d->pointErrorBarSize = str.toFloat();

			READ_QBRUSH(d->pointErrorBarBrush);
			READ_QPEN(d->pointErrorBarPen);
		} else if (reader->name() == "datapickerPoint") {
			DatapickerPoint* curvePoint = new DatapickerPoint("");
			curvePoint->setHidden(true);
			if (!curvePoint->load(reader, preview)) {
				delete curvePoint;
				return false;
			} else {
				addChild(curvePoint);
				curvePoint->initErrorBar(curveErrorTypes());
			}
		} else if (reader->name() == "spreadsheet") {
			Spreadsheet* datasheet = new Spreadsheet(0, "spreadsheet", true);
			if (!datasheet->load(reader, preview)) {
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

	d->retransform();
	return true;
}
