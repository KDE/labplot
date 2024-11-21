/*
	File                 : XYEquationCurve.cpp
	Project              : LabPlot
	Description          : A xy-curve defined by a mathematical equation
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2014-2017 Alexander Semke <alexander.semke@web.de>
	SPDX-FileCopyrightText: 2023 Stefan Gerlach <stefan.gerlach@uni.kn>
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
#include "backend/core/Folder.h"
#include "backend/core/column/Column.h"
#include "backend/gsl/ExpressionParser.h"
#include "backend/lib/XmlStreamReader.h"
#include "backend/lib/commandtemplates.h"
#include "backend/spreadsheet/Spreadsheet.h"
#include "backend/worksheet/plots/cartesian/Symbol.h"

#include <KLocalizedString>
#include <QIcon>

XYEquationCurve::XYEquationCurve(const QString& name)
	: XYCurve(name, new XYEquationCurvePrivate(this), AspectType::XYEquationCurve) {
	init();
}

XYEquationCurve::XYEquationCurve(const QString& name, XYEquationCurvePrivate* dd)
	: XYCurve(name, dd, AspectType::XYEquationCurve) {
	init();
}

// no need to delete the d-pointer here - it inherits from QGraphicsItem
// and is deleted during the cleanup in QGraphicsScene
XYEquationCurve::~XYEquationCurve() = default;

void XYEquationCurve::init() {
	Q_D(XYEquationCurve);

	d->xColumn->setHidden(true);
	addChildFast(d->xColumn);

	d->yColumn->setHidden(true);
	addChildFast(d->yColumn);

	// TODO: read from the saved settings for XYEquationCurve?
	d->lineType = XYCurve::LineType::Line;
	d->symbol->setStyle(Symbol::Style::NoSymbols);

	setUndoAware(false);
	setSuppressRetransform(true);
	setXColumn(d->xColumn);
	setYColumn(d->yColumn);
	setSuppressRetransform(false);
	setUndoAware(true);
}

void XYEquationCurve::recalculate() {
	Q_D(XYEquationCurve);
	d->recalculate();
}

bool XYEquationCurve::dataAvailable() const {
	Q_D(const XYEquationCurve);
	return (d->equationData.count > 0);
}

/*!
	Returns an icon to be used in the project explorer.
*/
QIcon XYEquationCurve::icon() const {
	return QIcon::fromTheme(QStringLiteral("labplot-xy-equation-curve"));
}

// ##############################################################################
// ##########################  getter methods  ##################################
// ##############################################################################
BASIC_SHARED_D_READER_IMPL(XYEquationCurve, XYEquationCurve::EquationData, equationData, equationData)

// ##############################################################################
// #################  setter methods and undo commands ##########################
// ##############################################################################
STD_SETTER_CMD_IMPL_F_S(XYEquationCurve, SetEquationData, XYEquationCurve::EquationData, equationData, recalculate)
void XYEquationCurve::setEquationData(const XYEquationCurve::EquationData& equationData) {
	Q_D(XYEquationCurve);
	if ((equationData.expression1 != d->equationData.expression1) || (equationData.expression2 != d->equationData.expression2)
		|| (equationData.min != d->equationData.min) || (equationData.max != d->equationData.max) || (equationData.count != d->equationData.count))
		exec(new XYEquationCurveSetEquationDataCmd(d, equationData, ki18n("%1: set equation")));
}

// ##############################################################################
// #################################  SLOTS  ####################################
// ##############################################################################

/*!
 * creates a new spreadsheet having the data with the results of the calculation.
 * the new spreadsheet is added to the current folder.
 */
void XYEquationCurve::createDataSpreadsheet() {
	if (!xColumn() || !yColumn())
		return;

	auto* spreadsheet = new Spreadsheet(i18n("%1 - Data", name()));
	spreadsheet->removeColumns(0, spreadsheet->columnCount()); // remove default columns
	spreadsheet->setRowCount(xColumn()->rowCount());

	// x values
	auto* data = static_cast<const Column*>(xColumn())->data();
	auto* xColumn = new Column(QLatin1String("x"), *static_cast<QVector<double>*>(data));
	xColumn->setPlotDesignation(AbstractColumn::PlotDesignation::X);
	spreadsheet->addChild(xColumn);

	// y values
	data = static_cast<const Column*>(yColumn())->data();
	auto* yColumn = new Column(QLatin1String("y"), *static_cast<QVector<double>*>(data));
	yColumn->setPlotDesignation(AbstractColumn::PlotDesignation::Y);
	spreadsheet->addChild(yColumn);

	// add the new spreadsheet to the current folder
	folder()->addChild(spreadsheet);
}

// ##############################################################################
// ######################### Private implementation #############################
// ##############################################################################
XYEquationCurvePrivate::XYEquationCurvePrivate(XYEquationCurve* owner)
	: XYCurvePrivate(owner)
	, xColumn(new Column(QStringLiteral("x"), AbstractColumn::ColumnMode::Double))
	, yColumn(new Column(QStringLiteral("y"), AbstractColumn::ColumnMode::Double))
	, xVector(static_cast<QVector<double>*>(xColumn->data()))
	, yVector(static_cast<QVector<double>*>(yColumn->data()))
	, q(owner) {
}

// no need to delete xColumn and yColumn, they are deleted
// when the parent aspect is removed
XYEquationCurvePrivate::~XYEquationCurvePrivate() = default;

void XYEquationCurvePrivate::recalculate() {
	// resize the vector if a new number of point to calculate was provided
	if (equationData.count != xVector->size()) {
		if (equationData.count >= 1) {
			xVector->resize(equationData.count);
			yVector->resize(equationData.count);
		} else {
			// invalid number of points provided
			xVector->clear();
			yVector->clear();
			recalc();
			Q_EMIT q->dataChanged();
			return;
		}
		xColumn->invalidateProperties();
		yColumn->invalidateProperties();
	} else {
		if (equationData.count < 1)
			return;
	}

	auto* parser = ExpressionParser::getInstance();
	bool valid = false;
	if (equationData.type == XYEquationCurve::EquationType::Cartesian) {
		valid = parser->tryEvaluateCartesian(equationData.expression1, equationData.min, equationData.max, equationData.count, xVector, yVector);
	} else if (equationData.type == XYEquationCurve::EquationType::Polar) {
		valid = parser->tryEvaluatePolar(equationData.expression1, equationData.min, equationData.max, equationData.count, xVector, yVector);
	} else if (equationData.type == XYEquationCurve::EquationType::Parametric) {
		valid = parser->tryEvaluateParametric(equationData.expression1,
											  equationData.expression2,
											  equationData.min,
											  equationData.max,
											  equationData.count,
											  xVector,
											  yVector);
	}

	if (!valid) {
		xVector->clear();
		yVector->clear();
	}
	xColumn->invalidateProperties();
	yColumn->invalidateProperties();

	recalc();
	Q_EMIT q->dataChanged();
}

// ##############################################################################
// ##################  Serialization/Deserialization  ###########################
// ##############################################################################
//! Save as XML
void XYEquationCurve::save(QXmlStreamWriter* writer) const {
	Q_D(const XYEquationCurve);

	writer->writeStartElement(QStringLiteral("xyEquationCurve"));

	// write xy-curve information
	XYCurve::save(writer);

	// write xy-equationCurve specific information
	writer->writeStartElement(QStringLiteral("equationData"));
	writer->writeAttribute(QStringLiteral("type"), QString::number(static_cast<int>(d->equationData.type)));
	writer->writeAttribute(QStringLiteral("expression1"), d->equationData.expression1);
	writer->writeAttribute(QStringLiteral("expression2"), d->equationData.expression2);
	writer->writeAttribute(QStringLiteral("min"), d->equationData.min);
	writer->writeAttribute(QStringLiteral("max"), d->equationData.max);
	writer->writeAttribute(QStringLiteral("count"), QString::number(d->equationData.count));
	writer->writeEndElement();

	writer->writeEndElement();
}

//! Load from XML
bool XYEquationCurve::load(XmlStreamReader* reader, bool preview) {
	Q_D(XYEquationCurve);

	QXmlStreamAttributes attribs;
	QString str;

	while (!reader->atEnd()) {
		reader->readNext();
		if (reader->isEndElement() && reader->name() == QLatin1String("xyEquationCurve"))
			break;

		if (!reader->isStartElement())
			continue;

		if (reader->name() == QLatin1String("xyCurve")) {
			if (!XYCurve::load(reader, preview))
				return false;
		} else if (!preview && reader->name() == QLatin1String("equationData")) {
			attribs = reader->attributes();

			READ_INT_VALUE("type", equationData.type, XYEquationCurve::EquationType);
			READ_STRING_VALUE("expression1", equationData.expression1);
			READ_STRING_VALUE("expression2", equationData.expression2);
			READ_STRING_VALUE("min", equationData.min);
			READ_STRING_VALUE("max", equationData.max);
			READ_INT_VALUE("count", equationData.count, int);
		} else { // unknown element
			reader->raiseUnknownElementWarning();
			if (!reader->skipToEndElement())
				return false;
		}
	}

	// Recalculate, otherwise xColumn and yColumn are not updated
	// and so autoscale is wrong
	recalculate();

	return true;
}
