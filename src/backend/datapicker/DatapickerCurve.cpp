/*
    File                 : DatapickerCurve.cpp
    Project              : LabPlot
    Description          : container for Curve-Point and Datasheet/Spreadsheet
    of datapicker
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2015 Ankit Wagadre <wagadre.ankit@gmail.com>
    SPDX-FileCopyrightText: 2015-2021 Alexander Semke <alexander.semke@web.de>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "DatapickerCurve.h"
#include "backend/datapicker/DatapickerCurvePrivate.h"
#include "backend/datapicker/Datapicker.h"
#include "backend/datapicker/DatapickerPoint.h"
#include "backend/lib/commandtemplates.h"
#include "backend/lib/XmlStreamReader.h"
#include "backend/spreadsheet/Spreadsheet.h"
#include "backend/worksheet/Worksheet.h"
#include "backend/worksheet/plots/cartesian/Symbol.h"

#include <QIcon>
#include <QVector3D>

#include <KConfig>
#include <KLocalizedString>
#include <KConfigGroup>

/**
 * \class DatapickerCurve
 * \brief Top-level container for Curve-Point and Datasheet/Spreadsheet of datapicker.
 * \ingroup backend
 */

DatapickerCurve::DatapickerCurve(const QString &name)
	: AbstractAspect(name, AspectType::DatapickerCurve), d_ptr(new DatapickerCurvePrivate(this)) {

	init();
}

DatapickerCurve::DatapickerCurve(const QString &name, DatapickerCurvePrivate *dd)
	: AbstractAspect(name, AspectType::DatapickerCurve), d_ptr(dd) {

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
	d->pointVisibility = group.readEntry("PointVisibility", true);

	//error bars
	d->curveErrorTypes.x = (ErrorType) group.readEntry("CurveErrorType_X", static_cast<int>(ErrorType::NoError));
	d->curveErrorTypes.y = (ErrorType) group.readEntry("CurveErrorType_Y", static_cast<int>(ErrorType::NoError));
	d->pointErrorBarSize = group.readEntry("ErrorBarSize", Worksheet::convertToSceneUnits(8, Worksheet::Unit::Point));
	d->pointErrorBarBrush.setStyle( (Qt::BrushStyle)group.readEntry("ErrorBarFillingStyle", (int)Qt::NoBrush) );
	d->pointErrorBarBrush.setColor( group.readEntry("ErrorBarFillingColor", QColor(Qt::black)) );
	d->pointErrorBarPen.setStyle( (Qt::PenStyle)group.readEntry("ErrorBarBorderStyle", (int)Qt::SolidLine) );
	d->pointErrorBarPen.setColor( group.readEntry("ErrorBarBorderColor", QColor(Qt::black)) );
	d->pointErrorBarPen.setWidthF( group.readEntry("ErrorBarBorderWidth", Worksheet::convertToSceneUnits(1, Worksheet::Unit::Point)) );

	//initialize the symbol
	d->symbol = new Symbol(QString());
	addChild(d->symbol);
	d->symbol->setHidden(true);
	connect(d->symbol, &Symbol::updateRequested, [=]{d->retransform();});
	connect(d->symbol, &Symbol::updatePixmapRequested, [=]{d->retransform();});
	d->symbol->init(group);
}

/*!
    Returns an icon to be used in the project explorer.
*/
QIcon DatapickerCurve::icon() const {
	return QIcon::fromTheme("labplot-xy-curve");
}

Column* DatapickerCurve::appendColumn(const QString& name) {
	Column* col = new Column(i18n("Column"), AbstractColumn::ColumnMode::Double);
	col->insertRows(0, m_datasheet->rowCount());
	col->setName(name);
	col->setFixed(true);
	m_datasheet->addChild(col);

	return col;
}

//##############################################################################
//##########################  getter methods  ##################################
//##############################################################################
Symbol* DatapickerCurve::symbol() const {
	Q_D(const DatapickerCurve);
	return d->symbol;
}

BASIC_SHARED_D_READER_IMPL(DatapickerCurve, DatapickerCurve::Errors, curveErrorTypes, curveErrorTypes)
BASIC_SHARED_D_READER_IMPL(DatapickerCurve, qreal, pointErrorBarSize, pointErrorBarSize)
BASIC_SHARED_D_READER_IMPL(DatapickerCurve, QBrush, pointErrorBarBrush, pointErrorBarBrush)
BASIC_SHARED_D_READER_IMPL(DatapickerCurve, QPen, pointErrorBarPen, pointErrorBarPen)
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

	m_datasheet = new Spreadsheet(i18n("Data"));
	m_datasheet->setFixed(true);
	addChild(m_datasheet);
	QString xLabel('x');
	QString yLabel('y');

	if (type == DatapickerImage::GraphType::PolarInDegree) {
		xLabel = QLatin1String("r");
		yLabel = QLatin1String("y(deg)");
	} else if (type == DatapickerImage::GraphType::PolarInRadians) {
		xLabel = QLatin1String("r");
		yLabel = QLatin1String("y(rad)");
	} else if (type == DatapickerImage::GraphType::LogarithmicX) {
		xLabel = QLatin1String("log(x)");
		yLabel = QLatin1String("y");
	} else if (type == DatapickerImage::GraphType::LogarithmicY) {
		xLabel = QLatin1String("x");
		yLabel = QLatin1String("log(y)");
	}

	if (type == DatapickerImage::GraphType::Ternary)
		d->posZColumn = appendColumn(i18n("c"));

	d->posXColumn = m_datasheet->column(0);
	d->posXColumn->setName(xLabel);
	d->posXColumn->setFixed(true);

	d->posYColumn = m_datasheet->column(1);
	d->posYColumn->setName(yLabel);
	d->posYColumn->setFixed(true);
}

STD_SETTER_CMD_IMPL_S(DatapickerCurve, SetCurveErrorTypes, DatapickerCurve::Errors, curveErrorTypes)
void DatapickerCurve::setCurveErrorTypes(const DatapickerCurve::Errors errors) {
	Q_D(DatapickerCurve);
	if (d->curveErrorTypes.x != errors.x || d->curveErrorTypes.y != errors.y) {
		beginMacro(i18n("%1: set xy-error type", name()));
		exec(new DatapickerCurveSetCurveErrorTypesCmd(d, errors, ki18n("%1: set xy-error type")));

		if ( errors.x != ErrorType::NoError && !d->plusDeltaXColumn )
			setPlusDeltaXColumn(appendColumn(QLatin1String("+delta_x")));
		else if ( d->plusDeltaXColumn && errors.x ==ErrorType:: NoError ) {
			d->plusDeltaXColumn->remove();
			d->plusDeltaXColumn = nullptr;
		}

		if ( errors.x == ErrorType::AsymmetricError && !d->minusDeltaXColumn )
			setMinusDeltaXColumn(appendColumn(QLatin1String("-delta_x")));
		else if ( d->minusDeltaXColumn && errors.x != ErrorType::AsymmetricError ) {
			d->minusDeltaXColumn->remove();
			d->minusDeltaXColumn = nullptr;
		}

		if ( errors.y != ErrorType::NoError && !d->plusDeltaYColumn )
			setPlusDeltaYColumn(appendColumn(QLatin1String("+delta_y")));
		else if ( d->plusDeltaYColumn && errors.y == ErrorType::NoError ) {
			d->plusDeltaYColumn->remove();
			d->plusDeltaYColumn = nullptr;
		}

		if ( errors.y == ErrorType::AsymmetricError && !d->minusDeltaYColumn )
			setMinusDeltaYColumn(appendColumn(QLatin1String("-delta_y")));
		else if ( d->minusDeltaYColumn && errors.y != ErrorType::AsymmetricError ) {
			d->minusDeltaYColumn->remove();
			d->minusDeltaYColumn = nullptr;
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
	for (auto* point : children<DatapickerPoint>(AbstractAspect::ChildIndexFlag::IncludeHidden))
		point->setPrinting(on);
}

/*!
    Selects or deselects the Datapicker/Curve in the project explorer.
    This function is called in \c DatapickerImageView.
*/
void DatapickerCurve::setSelectedInView(bool b) {
	if (b)
		Q_EMIT childAspectSelectedInView(this);
	else
		Q_EMIT childAspectDeselectedInView(this);
}

//##############################################################################
//######  SLOTs for changes triggered via QActions in the context menu  ########
//##############################################################################
void DatapickerCurve::updatePoints() {
	for (auto* point : children<DatapickerPoint>(ChildIndexFlag::IncludeHidden))
		updatePoint(point);
}

/*!
    Update datasheet for corresponding curve-point,
    it is called every time whenever there is any change in position
    of curve-point or its error-bar so keep it undo unaware
    no need to create extra entry in undo stack
*/
void DatapickerCurve::updatePoint(const DatapickerPoint* point) {
	Q_D(DatapickerCurve);

	//TODO: this check shouldn't be required.
	//redesign the retransform()-call in load() to avoid it.
	if (!parentAspect())
		return;

	auto* datapicker = static_cast<Datapicker*>(parentAspect());
	int row = indexOfChild<DatapickerPoint>(point, ChildIndexFlag::IncludeHidden);
	QVector3D data = datapicker->mapSceneToLogical(point->position());

	if (d->posXColumn)
		d->posXColumn->setValueAt(row, data.x());

	if (d->posYColumn)
		d->posYColumn->setValueAt(row, data.y());

	if (d->posZColumn)
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
	if (q->isLoading())
		return;
	const auto& points = q->children<DatapickerPoint>(AbstractAspect::ChildIndexFlag::IncludeHidden);
	for (auto* point : points)
		point->retransform();
}

//##############################################################################
//##################  Serialization/Deserialization  ###########################
//##############################################################################
//! Save as XML
void DatapickerCurve::save(QXmlStreamWriter* writer) const {
	Q_D(const DatapickerCurve);

	writer->writeStartElement("datapickerCurve");
	writeBasicAttributes(writer);
	writeCommentElement(writer);

	//general
	writer->writeStartElement("general");
	WRITE_COLUMN(d->posXColumn, posXColumn);
	WRITE_COLUMN(d->posYColumn, posYColumn);
	WRITE_COLUMN(d->posZColumn, posZColumn);
	WRITE_COLUMN(d->plusDeltaXColumn, plusDeltaXColumn);
	WRITE_COLUMN(d->minusDeltaXColumn, minusDeltaXColumn);
	WRITE_COLUMN(d->plusDeltaYColumn, plusDeltaYColumn);
	WRITE_COLUMN(d->minusDeltaYColumn, minusDeltaYColumn);
	writer->writeAttribute( "curveErrorType_X", QString::number(static_cast<int>(d->curveErrorTypes.x)) );
	writer->writeAttribute( "curveErrorType_Y", QString::number(static_cast<int>(d->curveErrorTypes.y)) );
	writer->writeAttribute( "vibible", QString::number(d->pointVisibility) );
	writer->writeEndElement();

	//Symbols
	d->symbol->save(writer);

	//error bar properties
	writer->writeStartElement("errorBarProperties");
	writer->writeAttribute( "pointErrorBarSize", QString::number(d->pointErrorBarSize) );
	WRITE_QBRUSH(d->pointErrorBarBrush);
	WRITE_QPEN(d->pointErrorBarPen);
	writer->writeEndElement();

	//serialize all children
	for (auto* child : children<AbstractAspect>(ChildIndexFlag::IncludeHidden))
		child->save(writer);

	writer->writeEndElement(); // close section
}

//! Load from XML
bool DatapickerCurve::load(XmlStreamReader* reader, bool preview) {
	Q_D(DatapickerCurve);

	if (!readBasicAttributes(reader))
		return false;

	KLocalizedString attributeWarning = ki18n("Attribute '%1' missing or empty, default value is used");
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

			READ_INT_VALUE("visible", pointVisibility, bool);
			READ_INT_VALUE("curveErrorType_X", curveErrorTypes.x, ErrorType);
			READ_INT_VALUE("curveErrorType_Y", curveErrorTypes.y, ErrorType);

			READ_COLUMN(posXColumn);
			READ_COLUMN(posYColumn);
			READ_COLUMN(posZColumn);
			READ_COLUMN(plusDeltaXColumn);
			READ_COLUMN(minusDeltaXColumn);
			READ_COLUMN(plusDeltaYColumn);
			READ_COLUMN(minusDeltaYColumn);
		} else if (!preview && reader->name() == "symbolProperties") {
			//old serialization that was used before the switch to Symbol::load().
			//in the old serialization the symbol properties and "point visibility" where saved
			//under "symbolProperties".
			attribs = reader->attributes();

			str = attribs.value("pointRotationAngle").toString();
			if (str.isEmpty())
				reader->raiseWarning(attributeWarning.subs("pointRotationAngle").toString());
			else
				d->symbol->setRotationAngle(str.toDouble());

			str = attribs.value("pointOpacity").toString();
			if (str.isEmpty())
				reader->raiseWarning(attributeWarning.subs("pointOpacity").toString());
			else
				d->symbol->setOpacity(str.toDouble());

			str = attribs.value("pointSize").toString();
			if (str.isEmpty())
				reader->raiseWarning(attributeWarning.subs("pointSize").toString());
			else
				d->symbol->setSize(str.toDouble());

			str = attribs.value("pointStyle").toString();
			if (str.isEmpty())
				reader->raiseWarning(attributeWarning.subs("pointStyle").toString());
			else
				d->symbol->setStyle(static_cast<Symbol::Style>(str.toInt()));

			//brush
			QBrush brush;
			str = attribs.value("brush_style").toString();
			if(str.isEmpty())
				reader->raiseWarning(attributeWarning.subs("brush_style").toString());
			else
				brush.setStyle( static_cast<Qt::BrushStyle>(str.toInt()) );

			QColor color;
			str = attribs.value("brush_color_r").toString();
			if(str.isEmpty())
				reader->raiseWarning(attributeWarning.subs("brush_color_r").toString());
			else
				color.setRed(str.toInt());

			str = attribs.value("brush_color_g").toString();
			if(str.isEmpty())
				reader->raiseWarning(attributeWarning.subs("brush_color_g").toString());
			else
				color.setGreen(str.toInt());

			str = attribs.value("brush_color_b").toString();
			if(str.isEmpty())
				reader->raiseWarning(attributeWarning.subs("brush_color_b").toString());
			else
				color.setBlue(str.toInt());

			brush.setColor(color);
			d->symbol->setBrush(brush);

			//pen
			QPen pen;
			str = attribs.value("style").toString();
			if (str.isEmpty())
				reader->raiseWarning(attributeWarning.subs("style").toString());
			else
				pen.setStyle( static_cast<Qt::PenStyle>(str.toInt()) );

			str = attribs.value("color_r").toString();
			if (str.isEmpty())
				reader->raiseWarning(attributeWarning.subs("color_r").toString());
			else
				color.setRed( str.toInt() );

			str = attribs.value("color_g").toString();
			if (str.isEmpty())
				reader->raiseWarning(attributeWarning.subs("color_g").toString());
			else
				color.setGreen( str.toInt() );

			str = attribs.value("color_b").toString();
			if (str.isEmpty())
				reader->raiseWarning(attributeWarning.subs("color_b").toString());
			else
				color.setBlue( str.toInt() );

			pen.setColor(color);

			str = attribs.value("width").toString();
			if (str.isEmpty())
				reader->raiseWarning(attributeWarning.subs("width").toString());
			else
				pen.setWidthF( str.toDouble() );

			d->symbol->setPen(pen);

			READ_INT_VALUE("pointVisibility", pointVisibility, bool);
		} else if (!preview && reader->name() == "symbols") {
			d->symbol->load(reader, preview);
		} else if (!preview && reader->name() == "errorBarProperties") {
			attribs = reader->attributes();

			READ_DOUBLE_VALUE("pointErrorBarSize", pointErrorBarSize);
			READ_QBRUSH(d->pointErrorBarBrush);
			READ_QPEN(d->pointErrorBarPen);
		} else if (reader->name() == "datapickerPoint") {
			auto* curvePoint = new DatapickerPoint(QString());
			curvePoint->setHidden(true);
			if (!curvePoint->load(reader, preview)) {
				delete curvePoint;
				return false;
			} else {
				addChild(curvePoint);
				curvePoint->initErrorBar(curveErrorTypes());
			}
		} else if (reader->name() == "spreadsheet") {
			Spreadsheet* datasheet = new Spreadsheet("spreadsheet", true);
			if (!datasheet->load(reader, preview)) {
				delete datasheet;
				return false;
			} else {
				addChild(datasheet);
				datasheet->setFixed(true);
				const auto& columns = datasheet->children<Column>();
				for (auto* col : columns)
					col->setFixed(true);
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
