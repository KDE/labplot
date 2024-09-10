/*
	File                 : DatapickerCurve.cpp
	Project              : LabPlot
	Description          : container for Curve-Point and Datasheet/Spreadsheet
	of datapicker
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2015 Ankit Wagadre <wagadre.ankit@gmail.com>
	SPDX-FileCopyrightText: 2015-2022 Alexander Semke <alexander.semke@web.de>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "DatapickerCurve.h"
#include "backend/datapicker/Datapicker.h"
#include "backend/datapicker/DatapickerCurvePrivate.h"
#include "backend/datapicker/DatapickerPoint.h"
#include "backend/lib/XmlStreamReader.h"
#include "backend/lib/commandtemplates.h"
#include "backend/spreadsheet/Spreadsheet.h"
#include "backend/worksheet/Worksheet.h"
#include "backend/worksheet/plots/cartesian/Symbol.h"

#include <QIcon>
#include <QVector3D>

#include <KConfig>
#include <KConfigGroup>
#include <KLocalizedString>

/**
 * \class DatapickerCurve
 * \brief Top-level container for Curve-Point and Datasheet/Spreadsheet of datapicker.
 * \ingroup backend
 */

DatapickerCurve::DatapickerCurve(const QString& name)
	: AbstractAspect(name, AspectType::DatapickerCurve)
	, d_ptr(new DatapickerCurvePrivate(this)) {
	init();
}

DatapickerCurve::DatapickerCurve(const QString& name, DatapickerCurvePrivate* dd)
	: AbstractAspect(name, AspectType::DatapickerCurve)
	, d_ptr(dd) {
	init();
}

DatapickerCurve::~DatapickerCurve() {
	delete d_ptr;
}

void DatapickerCurve::init() {
	Q_D(DatapickerCurve);

	KConfig config;
	KConfigGroup group;
	group = config.group(QStringLiteral("DatapickerCurve"));
	d->pointVisibility = group.readEntry(QStringLiteral("PointVisibility"), true);

	// error bars
	d->curveErrorTypes.x = (ErrorType)group.readEntry(QStringLiteral("CurveErrorType_X"), static_cast<int>(ErrorType::NoError));
	d->curveErrorTypes.y = (ErrorType)group.readEntry(QStringLiteral("CurveErrorType_Y"), static_cast<int>(ErrorType::NoError));
	d->pointErrorBarSize = group.readEntry(QStringLiteral("ErrorBarSize"), Worksheet::convertToSceneUnits(8, Worksheet::Unit::Point));
	d->pointErrorBarBrush.setStyle((Qt::BrushStyle)group.readEntry(QStringLiteral("ErrorBarFillingStyle"), (int)Qt::NoBrush));
	d->pointErrorBarBrush.setColor(group.readEntry(QStringLiteral("ErrorBarFillingColor"), QColor(Qt::black)));
	d->pointErrorBarPen.setStyle((Qt::PenStyle)group.readEntry(QStringLiteral("ErrorBarBorderStyle"), (int)Qt::SolidLine));
	d->pointErrorBarPen.setColor(group.readEntry(QStringLiteral("ErrorBarBorderColor"), QColor(Qt::black)));
	d->pointErrorBarPen.setWidthF(group.readEntry(QStringLiteral("ErrorBarBorderWidth"), Worksheet::convertToSceneUnits(1, Worksheet::Unit::Point)));

	// initialize the symbol
	d->symbol = new Symbol(QString());
	addChild(d->symbol);
	d->symbol->setHidden(true);
	connect(d->symbol, &Symbol::updateRequested, [=] {
		d->retransform();
	});
	connect(d->symbol, &Symbol::updatePixmapRequested, [=] {
		d->retransform();
	});
	d->symbol->init(group);

	connect(this, &AbstractAspect::childAspectAdded, this, &DatapickerCurve::childAdded);
	connect(this, &AbstractAspect::childAspectAboutToBeRemoved, this, &DatapickerCurve::childRemoved);
}

void DatapickerCurve::childAdded(const AbstractAspect* child) {
	if (m_supressResizeDatasheet)
		return;
	const auto* p = dynamic_cast<const DatapickerPoint*>(child);
	if (!p)
		return;
	m_datasheet->setRowCount(m_datasheet->rowCount() + 1);
}

void DatapickerCurve::childRemoved(const AbstractAspect* child) {
	Q_UNUSED(child);
	const auto* point = dynamic_cast<const DatapickerPoint*>(child);
	if (!point)
		return;

	int row = indexOfChild<DatapickerPoint>(point, ChildIndexFlag::IncludeHidden);
	m_datasheet->removeRows(row, 1);
}

/*!
	Returns an icon to be used in the project explorer.
*/
QIcon DatapickerCurve::icon() const {
	return QIcon::fromTheme(QStringLiteral("labplot-xy-curve"));
}

Column* DatapickerCurve::appendColumn(const QString& name) {
	auto* col = new Column(name);
	col->insertRows(0, m_datasheet->rowCount());
	col->setFixed(true);
	col->setUndoAware(false);
	m_datasheet->addChild(col);

	return col;
}

// ##############################################################################
// ##########################  getter methods  ##################################
// ##############################################################################
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

// ##############################################################################
// #########################  setter methods  ###################################
// ##############################################################################
void DatapickerCurve::addDatasheet(DatapickerImage::GraphType type) {
	Q_D(DatapickerCurve);

	m_datasheet = new Spreadsheet(i18n("Data"));
	m_datasheet->setFixed(true);
	m_datasheet->setRowCount(0);
	addChild(m_datasheet);

	QString xLabel;
	QString yLabel;

	switch (type) {
	case DatapickerImage::GraphType::Linear: {
		xLabel = QLatin1Char('x');
		yLabel = QLatin1Char('y');
		break;
	}
	case DatapickerImage::GraphType::PolarInDegree: {
		xLabel = QLatin1String("r");
		yLabel = QLatin1String("y(deg)");
		break;
	}
	case DatapickerImage::GraphType::PolarInRadians: {
		xLabel = QLatin1String("r");
		yLabel = QLatin1String("y(rad)");
		break;
	}
	case DatapickerImage::GraphType::LnXY: {
		xLabel = QLatin1String("ln(x)");
		yLabel = QLatin1String("ln(y)");
		break;
	}
	case DatapickerImage::GraphType::LnX: {
		xLabel = QLatin1String("ln(x)");
		yLabel = QLatin1String("y");
		break;
	}
	case DatapickerImage::GraphType::LnY: {
		xLabel = QLatin1String("x");
		yLabel = QLatin1String("ln(y)");
		break;
	}
	case DatapickerImage::GraphType::Log10XY: {
		xLabel = QLatin1String("log(x)");
		yLabel = QLatin1String("log(y)");
		break;
	}
	case DatapickerImage::GraphType::Log10X: {
		xLabel = QLatin1String("log(x)");
		yLabel = QLatin1String("y");
		break;
	}
	case DatapickerImage::GraphType::Log10Y: {
		xLabel = QLatin1String("x");
		yLabel = QLatin1String("log(y)");
		break;
	}
	case DatapickerImage::GraphType::Ternary: {
		xLabel = QLatin1Char('a');
		yLabel = QLatin1Char('b');
		break;
	}
	}

	// the default spreadsheet can have arbitrary number of colums as per user's default template.
	// make sure we have the columns for x and y only
	if (m_datasheet->columnCount() < 1)
		appendColumn(xLabel);
	if (m_datasheet->columnCount() < 2)
		appendColumn(yLabel);
	if (m_datasheet->columnCount() > 2)
		m_datasheet->setColumnCount(2);

	// add the third column for Ternary
	if (type == DatapickerImage::GraphType::Ternary)
		d->posZColumn = appendColumn(QLatin1String("c"));

	d->posXColumn = m_datasheet->column(0);
	d->posXColumn->setName(xLabel);
	d->posXColumn->setPlotDesignation(AbstractColumn::PlotDesignation::X);
	d->posXColumn->setFixed(true);
	d->posXColumn->setUndoAware(false);

	d->posYColumn = m_datasheet->column(1);
	d->posYColumn->setName(yLabel);
	d->posYColumn->setPlotDesignation(AbstractColumn::PlotDesignation::Y);
	d->posYColumn->setFixed(true);
	d->posYColumn->setUndoAware(false);
}

STD_SETTER_CMD_IMPL_S(DatapickerCurve, SetCurveErrorTypes, DatapickerCurve::Errors, curveErrorTypes)
void DatapickerCurve::setCurveErrorTypes(const DatapickerCurve::Errors errors) {
	Q_D(DatapickerCurve);
	if (d->curveErrorTypes.x != errors.x || d->curveErrorTypes.y != errors.y) {
		beginMacro(i18n("%1: set xy-error type", name()));
		exec(new DatapickerCurveSetCurveErrorTypesCmd(d, errors, ki18n("%1: set xy-error type")));

		if (errors.x == ErrorType::AsymmetricError) {
			if (!d->plusDeltaXColumn)
				setPlusDeltaXColumn(appendColumn(QLatin1String("+delta_x")));
			else
				d->plusDeltaXColumn->setName(QLatin1String("+delta_x")); // make sure we have the proper name when switching from Symmetric to Asymmetric
		} else if (errors.x == ErrorType::SymmetricError){
			if (!d->plusDeltaXColumn)
				setPlusDeltaXColumn(appendColumn(QLatin1String("+-delta_x")));
			else
				d->plusDeltaXColumn->setName(QLatin1String("+-delta_x")); // make sure we have the proper name when switching from Asymmetric to Symmetric
		} else if (d->plusDeltaXColumn && errors.x == ErrorType::NoError) {
			d->plusDeltaXColumn->remove();
			d->plusDeltaXColumn = nullptr;
		}

		if (errors.x == ErrorType::AsymmetricError && !d->minusDeltaXColumn)
			setMinusDeltaXColumn(appendColumn(QLatin1String("-delta_x")));
		else if (d->minusDeltaXColumn && errors.x != ErrorType::AsymmetricError) {
			d->minusDeltaXColumn->remove();
			d->minusDeltaXColumn = nullptr;
		}

		if (errors.y == ErrorType::AsymmetricError) {
			if (!d->plusDeltaYColumn)
				setPlusDeltaYColumn(appendColumn(QLatin1String("+delta_y")));
			else
				d->plusDeltaYColumn->setName(QLatin1String("+delta_y")); // make sure we have the proper name when switching from Symmetric to Asymmetric
		} else if (errors.y == ErrorType::SymmetricError) {
			if (!d->plusDeltaYColumn)
				setPlusDeltaYColumn(appendColumn(QLatin1String("+-delta_y")));
			else
				d->plusDeltaYColumn->setName(QLatin1String("+-delta_y")); // make sure we have the proper name when switching from Asymmetric to Symmetric
		} else if (d->plusDeltaYColumn && errors.y == ErrorType::NoError) {
			d->plusDeltaYColumn->remove();
			d->plusDeltaYColumn = nullptr;
		}

		if (errors.y == ErrorType::AsymmetricError && !d->minusDeltaYColumn)
			setMinusDeltaYColumn(appendColumn(QLatin1String("-delta_y")));
		else if (d->minusDeltaYColumn && errors.y != ErrorType::AsymmetricError) {
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
void DatapickerCurve::setPointErrorBarBrush(const QBrush& brush) {
	Q_D(DatapickerCurve);
	if (brush != d->pointErrorBarBrush)
		exec(new DatapickerCurveSetPointErrorBarBrushCmd(d, brush, ki18n("%1: set error bar filling")));
}

STD_SETTER_CMD_IMPL_F_S(DatapickerCurve, SetPointErrorBarPen, QPen, pointErrorBarPen, retransform)
void DatapickerCurve::setPointErrorBarPen(const QPen& pen) {
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

// ##############################################################################
// ######  SLOTs for changes triggered via QActions in the context menu  ########
// ##############################################################################
void DatapickerCurve::suppressUpdatePoint(bool suppress) {
	m_supressResizeDatasheet = suppress;

	if (!suppress) {
		// update points
		auto points = children<DatapickerPoint>(ChildIndexFlag::IncludeHidden);
		m_datasheet->setRowCount(points.count());
		updatePoints();
	}
}

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

	if (m_supressResizeDatasheet)
		return;

	// TODO: this check shouldn't be required.
	// redesign the retransform()-call in load() to avoid it.
	if (!parentAspect())
		return;

	auto* datapicker = static_cast<Datapicker*>(parentAspect());
	int row = indexOfChild<DatapickerPoint>(point, ChildIndexFlag::IncludeHidden);

	const auto xDateTime = datapicker->xDateTime();
	if ((m_datetime && !xDateTime) || (!m_datetime && xDateTime))
		updateColumns(xDateTime);

	Vector3D data = datapicker->mapSceneToLogical(point->position());

	if (d->posXColumn) {
		if (xDateTime) {
			auto dt = QDateTime::fromMSecsSinceEpoch(data.x());
			dt.setTimeSpec(Qt::TimeSpec::UTC);
			d->posXColumn->setDateTimeAt(row, dt);
		} else
			d->posXColumn->setValueAt(row, data.x());
	}

	if (d->posYColumn)
		d->posYColumn->setValueAt(row, data.y());

	if (d->posZColumn)
		d->posZColumn->setValueAt(row, data.y());

	if (d->plusDeltaXColumn) {
		data = datapicker->mapSceneLengthToLogical(QPointF(point->plusDeltaXPos().x(), 0));
		d->plusDeltaXColumn->setValueAt(row, std::abs(data.x()));
	}

	if (d->minusDeltaXColumn) {
		data = datapicker->mapSceneLengthToLogical(QPointF(point->minusDeltaXPos().x(), 0));
		d->minusDeltaXColumn->setValueAt(row, std::abs(data.x()));
	}

	if (d->plusDeltaYColumn) {
		data = datapicker->mapSceneLengthToLogical(QPointF(0, point->plusDeltaYPos().y()));
		d->plusDeltaYColumn->setValueAt(row, std::abs(data.y()));
	}

	if (d->minusDeltaYColumn) {
		data = datapicker->mapSceneLengthToLogical(QPointF(0, point->minusDeltaYPos().y()));
		d->minusDeltaYColumn->setValueAt(row, std::abs(data.y()));
	}
}

void DatapickerCurve::updateColumns(bool datetime) {
	m_datetime = datetime;
	Q_D(DatapickerCurve);
	if (datetime)
		d->posXColumn->setColumnMode(AbstractColumn::ColumnMode::DateTime);
	else
		d->posXColumn->setColumnMode(AbstractColumn::ColumnMode::Double);
}

// ##############################################################################
// ####################### Private implementation ###############################
// ##############################################################################
DatapickerCurvePrivate::DatapickerCurvePrivate(DatapickerCurve* curve)
	: q(curve) {
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

// ##############################################################################
// ##################  Serialization/Deserialization  ###########################
// ##############################################################################
//! Save as XML
void DatapickerCurve::save(QXmlStreamWriter* writer) const {
	Q_D(const DatapickerCurve);

	writer->writeStartElement(QStringLiteral("datapickerCurve"));
	writeBasicAttributes(writer);
	writeCommentElement(writer);

	// general
	writer->writeStartElement(QStringLiteral("general"));
	WRITE_COLUMN(d->posXColumn, posXColumn);
	WRITE_COLUMN(d->posYColumn, posYColumn);
	WRITE_COLUMN(d->posZColumn, posZColumn);
	WRITE_COLUMN(d->plusDeltaXColumn, plusDeltaXColumn);
	WRITE_COLUMN(d->minusDeltaXColumn, minusDeltaXColumn);
	WRITE_COLUMN(d->plusDeltaYColumn, plusDeltaYColumn);
	WRITE_COLUMN(d->minusDeltaYColumn, minusDeltaYColumn);
	writer->writeAttribute(QStringLiteral("curveErrorType_X"), QString::number(static_cast<int>(d->curveErrorTypes.x)));
	writer->writeAttribute(QStringLiteral("curveErrorType_Y"), QString::number(static_cast<int>(d->curveErrorTypes.y)));
	writer->writeAttribute(QStringLiteral("vibible"), QString::number(d->pointVisibility));
	writer->writeEndElement();

	// Symbols
	d->symbol->save(writer);

	// error bar properties
	writer->writeStartElement(QStringLiteral("errorBarProperties"));
	writer->writeAttribute(QStringLiteral("pointErrorBarSize"), QString::number(d->pointErrorBarSize));
	WRITE_QBRUSH(d->pointErrorBarBrush);
	WRITE_QPEN(d->pointErrorBarPen);
	writer->writeEndElement();

	// serialize all children
	for (auto* child : children<AbstractAspect>(ChildIndexFlag::IncludeHidden))
		child->save(writer);

	writer->writeEndElement(); // close section
}

//! Load from XML
bool DatapickerCurve::load(XmlStreamReader* reader, bool preview) {
	Q_D(DatapickerCurve);

	if (!readBasicAttributes(reader))
		return false;

	QXmlStreamAttributes attribs;
	QString str;

	while (!reader->atEnd()) {
		reader->readNext();
		if (reader->isEndElement() && reader->name() == QLatin1String("datapickerCurve"))
			break;

		if (!reader->isStartElement())
			continue;

		if (reader->name() == QLatin1String("comment")) {
			if (!readCommentElement(reader))
				return false;
		} else if (!preview && reader->name() == QLatin1String("general")) {
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
		} else if (!preview && reader->name() == QLatin1String("symbolProperties")) {
			// old serialization that was used before the switch to Symbol::load().
			// in the old serialization the symbol properties and "point visibility" where saved
			// under "symbolProperties".
			attribs = reader->attributes();

			str = attribs.value(QStringLiteral("pointRotationAngle")).toString();
			if (str.isEmpty())
				reader->raiseMissingAttributeWarning(QStringLiteral("pointRotationAngle"));
			else
				d->symbol->setRotationAngle(str.toDouble());

			str = attribs.value(QStringLiteral("pointOpacity")).toString();
			if (str.isEmpty())
				reader->raiseMissingAttributeWarning(QStringLiteral("pointOpacity"));
			else
				d->symbol->setOpacity(str.toDouble());

			str = attribs.value(QStringLiteral("pointSize")).toString();
			if (str.isEmpty())
				reader->raiseMissingAttributeWarning(QStringLiteral("pointSize"));
			else
				d->symbol->setSize(str.toDouble());

			str = attribs.value(QStringLiteral("pointStyle")).toString();
			if (str.isEmpty())
				reader->raiseMissingAttributeWarning(QStringLiteral("pointStyle"));
			else
				d->symbol->setStyle(static_cast<Symbol::Style>(str.toInt()));

			// brush
			QBrush brush;
			str = attribs.value(QStringLiteral("brush_style")).toString();
			if (str.isEmpty())
				reader->raiseMissingAttributeWarning(QStringLiteral("brush_style"));
			else
				brush.setStyle(static_cast<Qt::BrushStyle>(str.toInt()));

			QColor color;
			str = attribs.value(QStringLiteral("brush_color_r")).toString();
			if (str.isEmpty())
				reader->raiseMissingAttributeWarning(QStringLiteral("brush_color_r"));
			else
				color.setRed(str.toInt());

			str = attribs.value(QStringLiteral("brush_color_g")).toString();
			if (str.isEmpty())
				reader->raiseMissingAttributeWarning(QStringLiteral("brush_color_g"));
			else
				color.setGreen(str.toInt());

			str = attribs.value(QStringLiteral("brush_color_b")).toString();
			if (str.isEmpty())
				reader->raiseMissingAttributeWarning(QStringLiteral("brush_color_b"));
			else
				color.setBlue(str.toInt());

			brush.setColor(color);
			d->symbol->setBrush(brush);

			// pen
			QPen pen;
			str = attribs.value(QStringLiteral("style")).toString();
			if (str.isEmpty())
				reader->raiseMissingAttributeWarning(QStringLiteral("style"));
			else
				pen.setStyle(static_cast<Qt::PenStyle>(str.toInt()));

			str = attribs.value(QStringLiteral("color_r")).toString();
			if (str.isEmpty())
				reader->raiseMissingAttributeWarning(QStringLiteral("color_r"));
			else
				color.setRed(str.toInt());

			str = attribs.value(QStringLiteral("color_g")).toString();
			if (str.isEmpty())
				reader->raiseMissingAttributeWarning(QStringLiteral("color_g"));
			else
				color.setGreen(str.toInt());

			str = attribs.value(QStringLiteral("color_b")).toString();
			if (str.isEmpty())
				reader->raiseMissingAttributeWarning(QStringLiteral("color_b"));
			else
				color.setBlue(str.toInt());

			pen.setColor(color);

			str = attribs.value(QStringLiteral("width")).toString();
			if (str.isEmpty())
				reader->raiseMissingAttributeWarning(QStringLiteral("width"));
			else
				pen.setWidthF(str.toDouble());

			d->symbol->setPen(pen);

			READ_INT_VALUE("pointVisibility", pointVisibility, bool);
		} else if (!preview && reader->name() == QLatin1String("symbols")) {
			d->symbol->load(reader, preview);
		} else if (!preview && reader->name() == QLatin1String("errorBarProperties")) {
			attribs = reader->attributes();

			READ_DOUBLE_VALUE("pointErrorBarSize", pointErrorBarSize);
			READ_QBRUSH(d->pointErrorBarBrush);
			READ_QPEN(d->pointErrorBarPen);
		} else if (reader->name() == QLatin1String("datapickerPoint")) {
			auto* curvePoint = new DatapickerPoint(QString());
			curvePoint->setHidden(true);
			if (!curvePoint->load(reader, preview)) {
				delete curvePoint;
				return false;
			} else {
				addChild(curvePoint);
				curvePoint->initErrorBar(curveErrorTypes());
			}
		} else if (reader->name() == QLatin1String("spreadsheet")) {
			auto* datasheet = new Spreadsheet(QStringLiteral("spreadsheet"), true);
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
			reader->raiseUnknownElementWarning();
			if (!reader->skipToEndElement())
				return false;
		}
	}

	d->retransform();
	return true;
}
