/*
    File                 : XYAnalysisCurve.h
    Project              : LabPlot
    Description          : Base class for all analysis curves
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2017-2018 Alexander Semke <alexander.semke@web.de>
    SPDX-FileCopyrightText: 2018 Stefan Gerlach <stefan.gerlach@uni.kn>
    SPDX-License-Identifier: GPL-2.0-or-later
*/


/*!
  \class XYAnalysisCurve
  \brief Base class for all analysis curves

  \ingroup worksheet
*/

#include "XYAnalysisCurve.h"
#include "XYAnalysisCurvePrivate.h"
#include "backend/core/column/Column.h"
#include "backend/core/Project.h"
#include "backend/lib/commandtemplates.h"
#include "backend/lib/macros.h"
#include "backend/lib/XmlStreamReader.h"
#include "backend/worksheet/plots/cartesian/Symbol.h"

#include <KLocalizedString>
#include <QDateTime>

XYAnalysisCurve::XYAnalysisCurve(const QString& name, AspectType type)
	: XYCurve(name, new XYAnalysisCurvePrivate(this), type) {

	init();
}

XYAnalysisCurve::XYAnalysisCurve(const QString& name, XYAnalysisCurvePrivate* dd, AspectType type)
	: XYCurve(name, dd, type) {

	init();
}

//no need to delete the d-pointer here - it inherits from QGraphicsItem
//and is deleted during the cleanup in QGraphicsScene
XYAnalysisCurve::~XYAnalysisCurve() = default;

void XYAnalysisCurve::init() {
	Q_D(XYAnalysisCurve);
	d->lineType = XYCurve::LineType::Line;
	d->symbol->setStyle(Symbol::Style::NoSymbols);
}

void XYAnalysisCurve::copyData(QVector<double>& xData, QVector<double>& yData,
		const AbstractColumn* xDataColumn, const AbstractColumn* yDataColumn, double xMin, double xMax) {

	const int rowCount = qMin(xDataColumn->rowCount(), yDataColumn->rowCount());
	for (int row = 0; row < rowCount; ++row) {
		if (!xDataColumn->isValid(row) || xDataColumn->isMasked(row) ||
				!yDataColumn->isValid(row) || yDataColumn->isMasked(row))
			continue;

		double x = qQNaN();
		switch (xDataColumn->columnMode()) {
		case AbstractColumn::ColumnMode::Double:
			x = xDataColumn->valueAt(row);
			break;
		case AbstractColumn::ColumnMode::Integer:
			x = xDataColumn->integerAt(row);
			break;
		case AbstractColumn::ColumnMode::BigInt:
			x = xDataColumn->bigIntAt(row);
			break;
		case AbstractColumn::ColumnMode::Text:	// invalid
			break;
		case AbstractColumn::ColumnMode::DateTime:
		case AbstractColumn::ColumnMode::Day:
		case AbstractColumn::ColumnMode::Month:
			x = xDataColumn->dateTimeAt(row).toMSecsSinceEpoch();
		}

		double y = qQNaN();
		switch (yDataColumn->columnMode()) {
		case AbstractColumn::ColumnMode::Double:
			y = yDataColumn->valueAt(row);
			break;
		case AbstractColumn::ColumnMode::Integer:
			y = yDataColumn->integerAt(row);
			break;
		case AbstractColumn::ColumnMode::BigInt:
			y = yDataColumn->bigIntAt(row);
			break;
		case AbstractColumn::ColumnMode::Text:	// invalid
			break;
		case AbstractColumn::ColumnMode::DateTime:
		case AbstractColumn::ColumnMode::Day:
		case AbstractColumn::ColumnMode::Month:
			y = yDataColumn->dateTimeAt(row).toMSecsSinceEpoch();
		}

		// only when inside given range
		if (x >= xMin && x <= xMax) {
			xData.append(x);
			yData.append(y);
		}
	}
}

//##############################################################################
//##########################  getter methods  ##################################
//##############################################################################
BASIC_SHARED_D_READER_IMPL(XYAnalysisCurve, XYAnalysisCurve::DataSourceType, dataSourceType, dataSourceType)
BASIC_SHARED_D_READER_IMPL(XYAnalysisCurve, const XYCurve*, dataSourceCurve, dataSourceCurve)
const QString& XYAnalysisCurve::dataSourceCurvePath() const {
	D(XYAnalysisCurve);
	return d->dataSourceCurvePath;
}

BASIC_SHARED_D_READER_IMPL(XYAnalysisCurve, const AbstractColumn*, xDataColumn, xDataColumn)
BASIC_SHARED_D_READER_IMPL(XYAnalysisCurve, const AbstractColumn*, yDataColumn, yDataColumn)
BASIC_SHARED_D_READER_IMPL(XYAnalysisCurve, const AbstractColumn*, y2DataColumn, y2DataColumn)
BASIC_SHARED_D_READER_IMPL(XYAnalysisCurve, QString, xDataColumnPath, xDataColumnPath)
BASIC_SHARED_D_READER_IMPL(XYAnalysisCurve, QString, yDataColumnPath, yDataColumnPath)
BASIC_SHARED_D_READER_IMPL(XYAnalysisCurve, QString, y2DataColumnPath, y2DataColumnPath)


bool XYAnalysisCurve::saveCalculations() const {
	return const_cast<XYAnalysisCurve*>(this)->project()->saveCalculations();
}

//##############################################################################
//#################  setter methods and undo commands ##########################
//##############################################################################
STD_SETTER_CMD_IMPL_S(XYAnalysisCurve, SetDataSourceType, XYAnalysisCurve::DataSourceType, dataSourceType)
void XYAnalysisCurve::setDataSourceType(DataSourceType type) {
	Q_D(XYAnalysisCurve);
	if (type != d->dataSourceType)
		exec(new XYAnalysisCurveSetDataSourceTypeCmd(d, type, ki18n("%1: data source type changed")));
}

STD_SETTER_CMD_IMPL_F_S(XYAnalysisCurve, SetDataSourceCurve, const XYCurve*, dataSourceCurve, retransform)
void XYAnalysisCurve::setDataSourceCurve(const XYCurve* curve) {
	Q_D(XYAnalysisCurve);
	if (curve != d->dataSourceCurve) {
		exec(new XYAnalysisCurveSetDataSourceCurveCmd(d, curve, ki18n("%1: data source curve changed")));
		handleSourceDataChanged();

		//handle the changes when different columns were provided for the source curve
		connect(curve, SIGNAL(xColumnChanged(const AbstractColumn*)), this, SLOT(handleSourceDataChanged()));
		connect(curve, SIGNAL(yColumnChanged(const AbstractColumn*)), this, SLOT(handleSourceDataChanged()));
		//TODO? connect(curve, SIGNAL(y2ColumnChanged(const AbstractColumn*)), this, SLOT(handleSourceDataChanged()));

		//handle the changes when the data inside of the source curve columns
		connect(curve, &XYCurve::xDataChanged, this, &XYAnalysisCurve::handleSourceDataChanged);
		connect(curve, &XYCurve::yDataChanged, this, &XYAnalysisCurve::handleSourceDataChanged);

		//TODO: add disconnect in the undo-function
	}
}

STD_SETTER_CMD_IMPL_S(XYAnalysisCurve, SetXDataColumn, const AbstractColumn*, xDataColumn)
void XYAnalysisCurve::setXDataColumn(const AbstractColumn* column) {
	DEBUG("XYAnalysisCurve::setXDataColumn()");
	Q_D(XYAnalysisCurve);
	if (column != d->xDataColumn) {
		exec(new XYAnalysisCurveSetXDataColumnCmd(d, column, ki18n("%1: assign x-data")));
		handleSourceDataChanged();
		if (column) {
			setXDataColumnPath(column->path());
			connect(column->parentAspect(), &AbstractAspect::aspectAboutToBeRemoved,
					this, &XYAnalysisCurve::xDataColumnAboutToBeRemoved);
			connect(column, SIGNAL(dataChanged(const AbstractColumn*)), this, SLOT(handleSourceDataChanged()));
			connect(column, &AbstractAspect::aspectDescriptionChanged, this, &XYAnalysisCurve::xDataColumnNameChanged);
			//TODO disconnect on undo
		} else
			setXDataColumnPath("");
	}
}

STD_SETTER_CMD_IMPL_S(XYAnalysisCurve, SetYDataColumn, const AbstractColumn*, yDataColumn)
void XYAnalysisCurve::setYDataColumn(const AbstractColumn* column) {
	DEBUG("XYAnalysisCurve::setYDataColumn()");
	Q_D(XYAnalysisCurve);
	if (column != d->yDataColumn) {
		exec(new XYAnalysisCurveSetYDataColumnCmd(d, column, ki18n("%1: assign y-data")));
		handleSourceDataChanged();
		if (column) {
			setYDataColumnPath(column->path());
			connect(column->parentAspect(), &AbstractAspect::aspectAboutToBeRemoved,
					this, &XYAnalysisCurve::yDataColumnAboutToBeRemoved);
			connect(column, SIGNAL(dataChanged(const AbstractColumn*)), this, SLOT(handleSourceDataChanged()));
			connect(column, &AbstractAspect::aspectDescriptionChanged, this, &XYAnalysisCurve::yDataColumnNameChanged);
			//TODO disconnect on undo
		} else
			setXDataColumnPath("");
	}
}

STD_SETTER_CMD_IMPL_S(XYAnalysisCurve, SetY2DataColumn, const AbstractColumn*, y2DataColumn)
void XYAnalysisCurve::setY2DataColumn(const AbstractColumn* column) {
	DEBUG("XYAnalysisCurve::setY2DataColumn()");
	Q_D(XYAnalysisCurve);
	if (column != d->y2DataColumn) {
		exec(new XYAnalysisCurveSetY2DataColumnCmd(d, column, ki18n("%1: assign second y-data")));
		handleSourceDataChanged();
		if (column) {
			setY2DataColumnPath(column->path());
			connect(column->parentAspect(), &AbstractAspect::aspectAboutToBeRemoved,
					this, &XYAnalysisCurve::y2DataColumnAboutToBeRemoved);
			connect(column, SIGNAL(dataChanged(const AbstractColumn*)), this, SLOT(handleSourceDataChanged()));
			connect(column, &AbstractAspect::aspectDescriptionChanged, this, &XYAnalysisCurve::y2DataColumnNameChanged);
			//TODO disconnect on undo
		} else
			setXDataColumnPath("");
	}
}

void XYAnalysisCurve::setXDataColumnPath(const QString& path) {
	Q_D(XYAnalysisCurve);
	d->xDataColumnPath = path;
}

void XYAnalysisCurve::setYDataColumnPath(const QString& path) {
	Q_D(XYAnalysisCurve);
	d->yDataColumnPath = path;
}

void XYAnalysisCurve::setY2DataColumnPath(const QString& path) {
	Q_D(XYAnalysisCurve);
	d->y2DataColumnPath = path;
}

//##############################################################################
//#################################  SLOTS  ####################################
//##############################################################################
void XYAnalysisCurve::handleSourceDataChanged() {
	Q_D(XYAnalysisCurve);
	d->sourceDataChangedSinceLastRecalc = true;
	emit sourceDataChanged();
}

void XYAnalysisCurve::xDataColumnAboutToBeRemoved(const AbstractAspect* aspect) {
	Q_D(XYAnalysisCurve);
	if (aspect == d->xDataColumn) {
		d->xDataColumn = nullptr;
		d->retransform();
	}
}

void XYAnalysisCurve::yDataColumnAboutToBeRemoved(const AbstractAspect* aspect) {
	Q_D(XYAnalysisCurve);
	if (aspect == d->yDataColumn) {
		d->yDataColumn = nullptr;
		d->retransform();
	}
}

void XYAnalysisCurve::y2DataColumnAboutToBeRemoved(const AbstractAspect* aspect) {
	Q_D(XYAnalysisCurve);
	if (aspect == d->y2DataColumn) {
		d->y2DataColumn = nullptr;
		d->retransform();
	}
}

void XYAnalysisCurve::xDataColumnNameChanged() {
	Q_D(XYAnalysisCurve);
	setXDataColumnPath(d->xDataColumn->path());
}

void XYAnalysisCurve::yDataColumnNameChanged() {
	Q_D(XYAnalysisCurve);
	setYDataColumnPath(d->yDataColumn->path());
}

void XYAnalysisCurve::y2DataColumnNameChanged() {
	Q_D(XYAnalysisCurve);
	setYDataColumnPath(d->y2DataColumn->path());
}

//##############################################################################
//######################### Private implementation #############################
//##############################################################################
XYAnalysisCurvePrivate::XYAnalysisCurvePrivate(XYAnalysisCurve* owner) : XYCurvePrivate(owner), q(owner) {
}

//no need to delete xColumn and yColumn, they are deleted
//when the parent aspect is removed
XYAnalysisCurvePrivate::~XYAnalysisCurvePrivate() = default;

//##############################################################################
//##################  Serialization/Deserialization  ###########################
//##############################################################################
//! Save as XML
void XYAnalysisCurve::save(QXmlStreamWriter* writer) const {
	Q_D(const XYAnalysisCurve);

	writer->writeStartElement("xyAnalysisCurve");

	//write xy-curve information
	XYCurve::save(writer);

	//write data source specific information
	writer->writeStartElement("dataSource");
	writer->writeAttribute("type", QString::number(static_cast<int>(d->dataSourceType)));
	WRITE_PATH(d->dataSourceCurve, dataSourceCurve);
	WRITE_COLUMN(d->xDataColumn, xDataColumn);
	WRITE_COLUMN(d->yDataColumn, yDataColumn);
	WRITE_COLUMN(d->y2DataColumn, y2DataColumn);
	writer->writeEndElement();

	writer->writeEndElement(); //"xyAnalysisCurve"
}

//! Load from XML
bool XYAnalysisCurve::load(XmlStreamReader* reader, bool preview) {
	Q_D(XYAnalysisCurve);

	KLocalizedString attributeWarning = ki18n("Attribute '%1' missing or empty, default value is used");
	QXmlStreamAttributes attribs;
	QString str;

	while (!reader->atEnd()) {
		reader->readNext();
		if (reader->isEndElement() && reader->name() == "xyAnalysisCurve")
			break;

		if (!reader->isStartElement())
			continue;

		if (reader->name() == "xyCurve") {
			if ( !XYCurve::load(reader, preview) )
				return false;
		} else if (reader->name() == "dataSource") {
			attribs = reader->attributes();
			READ_INT_VALUE("type", dataSourceType, XYAnalysisCurve::DataSourceType);
			READ_PATH(dataSourceCurve);
			READ_COLUMN(xDataColumn);
			READ_COLUMN(yDataColumn);
			READ_COLUMN(y2DataColumn);
		}
	}

	return true;
}
