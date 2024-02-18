/*
	File                 : ErrorBar.cpp
	Project              : LabPlot
	Description          : ErrorBar
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2024 Alexander Semke <alexander.semke@web.de>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

/*!
  \class ErrorBar
  \brief This class contains the properties of the error bars used in different plot classes.

  \ingroup worksheet
*/

#include "ErrorBar.h"
#include "ErrorBarPrivate.h"
#include "backend/core/AbstractColumn.h"
#include "backend/core/Project.h"
#include "backend/lib/commandtemplates.h"
#include "backend/lib/macrosCurve.h"
#include "backend/lib/XmlStreamReader.h"
#include "backend/worksheet/Line.h"
#include "backend/worksheet/Worksheet.h"
#include "backend/worksheet/plots/cartesian/CartesianCoordinateSystem.h"

#include <QPainter>
#include <KConfigGroup>
#include <KLocalizedString>

CURVE_COLUMN_CONNECT(ErrorBar, XPlus, xPlus, update)
CURVE_COLUMN_CONNECT(ErrorBar, XMinus, xMinus, update)
CURVE_COLUMN_CONNECT(ErrorBar, YPlus, yPlus, update)
CURVE_COLUMN_CONNECT(ErrorBar, YMinus, yMinus, update)

ErrorBar::ErrorBar(const QString& name,  Dimension dim)
	: AbstractAspect(name, AspectType::AbstractAspect)
	, d_ptr(new ErrorBarPrivate(this, dim)) {
}

ErrorBar::~ErrorBar() {
	delete d_ptr;
}

void ErrorBar::init(const KConfigGroup& group) {
	Q_D(ErrorBar);
	switch (d->dimension) {
	case Dimension::Y:
		d->yErrorType = (ErrorType)group.readEntry(QStringLiteral("ErrorType"), static_cast<int>(ErrorType::NoError));
		break;
	case Dimension::XY:
		d->xErrorType = (ErrorType)group.readEntry(QStringLiteral("XErrorType"), static_cast<int>(ErrorType::NoError));
		d->yErrorType = (ErrorType)group.readEntry(QStringLiteral("YErrorType"), static_cast<int>(ErrorType::NoError));
	}

	d->type = (Type)group.readEntry(QStringLiteral("ErrorBarsType"), static_cast<int>(Type::Simple));
	d->capSize = group.readEntry(QStringLiteral("ErrorBarsCapSize"), Worksheet::convertToSceneUnits(10, Worksheet::Unit::Point));

	d->line = new Line(QString());
	d->line->setHidden(true);
	d->line->setCreateXmlElement(false);
	addChild(d->line);
	d->line->init(group);
	connect(d->line, &Line::updatePixmapRequested, this, &ErrorBar::updatePixmapRequested);
	connect(d->line, &Line::updateRequested, this, &ErrorBar::updateRequested);
}

void ErrorBar::update() {
	Q_D(ErrorBar);
	d->update();
}

void ErrorBar::draw(QPainter* painter, const QPainterPath& path) {
	Q_D(const ErrorBar);
	painter->setOpacity(d->line->opacity());
	painter->setPen(d->line->pen());
	painter->setBrush(Qt::NoBrush);
	painter->drawPath(path);
}

QPainterPath ErrorBar::painterPath(const QVector<QPointF>& points, const CartesianCoordinateSystem* cSystem, WorksheetElement::Orientation orientation) const {
	Q_D(const ErrorBar);
	auto errorBarsPath = QPainterPath();
	QVector<QLineF> elines;
/*
	switch (d->errorType) {
	case ErrorBar::ErrorType::NoError:
		break;
	case ErrorBar::ErrorType::Poisson: {
		if (orientation == WorksheetElement::Orientation::Vertical) {
			for (auto& point : points) {
				double error = sqrt(point.y());
				if (error != 0.)
					elines << QLineF(point.x(), point.y() + error, point.x(), point.y() - error);
			}
		} else {
			for (auto& point : points) {
				double error = sqrt(point.x());
				if (error != 0.)
					elines << QLineF(point.x() - error, point.y(), point.x() + error, point.y());
			}
		}
		break;
	}
	case ErrorBar::ErrorType::Symmetric: {
		int index = 0;
		if (orientation == WorksheetElement::Orientation::Vertical) {
			const auto* errorPlusColumn = d->plusColumn;
			for (auto& point : points) {
				if (errorPlusColumn && errorPlusColumn->isValid(index) && !errorPlusColumn->isMasked(index)) {
					double error = errorPlusColumn->valueAt(index);
					if (error != 0.)
						elines << QLineF(point.x(), point.y() + error, point.x(), point.y() - error);
				}
				++index;
			}
		} else {
			const auto* errorMinusColumn = d->minusColumn;
			for (auto& point : points) {
				if (errorMinusColumn && errorMinusColumn->isValid(index) && !errorMinusColumn->isMasked(index)) {
					double error = errorMinusColumn->valueAt(index);
					if (error != 0.)
						elines << QLineF(point.x() - error, point.y(), point.x() + error, point.y());
				}
				++index;
			}
		}
		break;
	}
	case ErrorBar::ErrorType::Asymmetric: {
		int index = 0;
		if (orientation == WorksheetElement::Orientation::Vertical) {
			for (auto& point : points) {
				double errorPlus = 0.;
				double errorMinus = 0.;
				const auto* errorPlusColumn = d->plusColumn;
				const auto* errorMinusColumn = d->minusColumn;

				if (errorPlusColumn && errorPlusColumn->isValid(index) && !errorPlusColumn->isMasked(index))
					errorPlus = errorPlusColumn->valueAt(index);

				if (errorMinusColumn && errorMinusColumn->isValid(index) && !errorMinusColumn->isMasked(index))
					errorMinus = errorMinusColumn->valueAt(index);

				if (errorPlus != 0. || errorMinus != 0.)
					elines << QLineF(point.x(), point.y() - errorMinus, point.x(), point.y() + errorPlus);

				++index;
			}
		} else {
			for (auto& point : points) {
				double errorPlus = 0.;
				double errorMinus = 0.;
				const auto* errorPlusColumn = d->plusColumn;
				const auto* errorMinusColumn = d->minusColumn;

				if (errorPlusColumn && errorPlusColumn->isValid(index) && !errorPlusColumn->isMasked(index))
					errorPlus = errorPlusColumn->valueAt(index);

				if (errorMinusColumn && errorMinusColumn->isValid(index) && !errorMinusColumn->isMasked(index))
					errorMinus = errorMinusColumn->valueAt(index);

				if (errorPlus != 0. || errorMinus != 0.)
					elines << QLineF(point.x() - errorMinus, point.y(), point.x() + errorPlus, point.y());

				++index;
			}
		}
		break;
	}
	}
*/
	// map the error bars to scene coordinates
	elines = cSystem->mapLogicalToScene(elines);

	// new painter path for the error bars
	for (const auto& line : qAsConst(elines)) {
		errorBarsPath.moveTo(line.p1());
		errorBarsPath.lineTo(line.p2());
	}

	// add caps for error bars
	const auto capSize = d->capSize;
	if (d->type == ErrorBar::Type::WithEnds) {
		if (orientation == WorksheetElement::Orientation::Vertical) {
			for (const auto& line : qAsConst(elines)) {
				const auto& p1 = line.p1();
				errorBarsPath.moveTo(QPointF(p1.x() - capSize / 2., p1.y()));
				errorBarsPath.lineTo(QPointF(p1.x() + capSize / 2., p1.y()));

				const auto& p2 = line.p2();
				errorBarsPath.moveTo(QPointF(p2.x() - capSize / 2., p2.y()));
				errorBarsPath.lineTo(QPointF(p2.x() + capSize / 2., p2.y()));
			}
		} else {
			for (const auto& line : qAsConst(elines)) {
				const auto& p1 = line.p1();
				errorBarsPath.moveTo(QPointF(p1.x(), p1.y() - capSize / 2.));
				errorBarsPath.lineTo(QPointF(p1.x(), p1.y() + capSize / 2.));

				const auto& p2 = line.p2();
				errorBarsPath.moveTo(QPointF(p2.x(), p2.y() - capSize / 2.));
				errorBarsPath.lineTo(QPointF(p2.x(), p2.y() + capSize / 2.));
			}
		}
	}

	return errorBarsPath;
}

// ##############################################################################
// ##########################  getter methods  ##################################
// ##############################################################################
ErrorBar::Dimension ErrorBar::dimension() const {
	Q_D(const ErrorBar);
	return d->dimension;
}

BASIC_SHARED_D_READER_IMPL(ErrorBar, ErrorBar::ErrorType, xErrorType, xErrorType)
BASIC_SHARED_D_READER_IMPL(ErrorBar, const AbstractColumn*, xPlusColumn, xPlusColumn)
BASIC_SHARED_D_READER_IMPL(ErrorBar, const AbstractColumn*, xMinusColumn, xMinusColumn)
BASIC_SHARED_D_READER_IMPL(ErrorBar, QString, xPlusColumnPath, xPlusColumnPath)
BASIC_SHARED_D_READER_IMPL(ErrorBar, QString, xMinusColumnPath, xMinusColumnPath)

BASIC_SHARED_D_READER_IMPL(ErrorBar, ErrorBar::ErrorType, yErrorType, yErrorType)
BASIC_SHARED_D_READER_IMPL(ErrorBar, const AbstractColumn*, yPlusColumn, yPlusColumn)
BASIC_SHARED_D_READER_IMPL(ErrorBar, const AbstractColumn*, yMinusColumn, yMinusColumn)
BASIC_SHARED_D_READER_IMPL(ErrorBar, QString, yPlusColumnPath, yPlusColumnPath)
BASIC_SHARED_D_READER_IMPL(ErrorBar, QString, yMinusColumnPath, yMinusColumnPath)

BASIC_SHARED_D_READER_IMPL(ErrorBar, ErrorBar::Type, type, type)
BASIC_SHARED_D_READER_IMPL(ErrorBar, double, capSize, capSize)
Line* ErrorBar::line() const {
	Q_D(const ErrorBar);
	return d->line;
}

// ##############################################################################
// #################  setter methods and undo commands ##########################
// ##############################################################################
// x
STD_SETTER_CMD_IMPL_F_S(ErrorBar, SetXErrorType, ErrorBar::ErrorType, xErrorType, update)
void ErrorBar::setXErrorType(ErrorType errorType) {
	Q_D(ErrorBar);
	if (errorType != d->xErrorType)
		exec(new ErrorBarSetXErrorTypeCmd(d, errorType, ki18n("%1: error type changed")));
}

CURVE_COLUMN_SETTER_CMD_IMPL_F_S(ErrorBar, XPlus, xPlus, update)
void ErrorBar::setXPlusColumn(const AbstractColumn* column) {
	Q_D(ErrorBar);
	if (column != d->xPlusColumn)
		exec(new ErrorBarSetXPlusColumnCmd(d, column, ki18n("%1: set error column")));
}

void ErrorBar::setXPlusColumnPath(const QString& path) {
	Q_D(ErrorBar);
	d->xPlusColumnPath = path;
}

CURVE_COLUMN_SETTER_CMD_IMPL_F_S(ErrorBar, XMinus, xMinus, update)
void ErrorBar::setXMinusColumn(const AbstractColumn* column) {
	Q_D(ErrorBar);
	if (column != d->xMinusColumn)
		exec(new ErrorBarSetXMinusColumnCmd(d, column, ki18n("%1: set error column")));
}

void ErrorBar::setXMinusColumnPath(const QString& path) {
	Q_D(ErrorBar);
	d->xMinusColumnPath = path;
}

void ErrorBar::xPlusColumnAboutToBeRemoved(const AbstractAspect* aspect) {
	Q_D(ErrorBar);
	if (aspect == d->xPlusColumn) {
		d->xPlusColumn = nullptr;
		d->update();
	}
}

void ErrorBar::xMinusColumnAboutToBeRemoved(const AbstractAspect* aspect) {
	Q_D(ErrorBar);
	if (aspect == d->xMinusColumn) {
		d->xMinusColumn = nullptr;
		d->update();
	}
}

// y
STD_SETTER_CMD_IMPL_F_S(ErrorBar, SetYErrorType, ErrorBar::ErrorType, yErrorType, update)
void ErrorBar::setYErrorType(ErrorType errorType) {
	Q_D(ErrorBar);
	if (errorType != d->yErrorType)
		exec(new ErrorBarSetYErrorTypeCmd(d, errorType, ki18n("%1: error type changed")));
}

CURVE_COLUMN_SETTER_CMD_IMPL_F_S(ErrorBar, YPlus, yPlus, update)
void ErrorBar::setYPlusColumn(const AbstractColumn* column) {
	Q_D(ErrorBar);
	if (column != d->yPlusColumn)
		exec(new ErrorBarSetYPlusColumnCmd(d, column, ki18n("%1: set error column")));
}

void ErrorBar::setYPlusColumnPath(const QString& path) {
	Q_D(ErrorBar);
	d->yPlusColumnPath = path;
}

CURVE_COLUMN_SETTER_CMD_IMPL_F_S(ErrorBar, YMinus, yMinus, update)
void ErrorBar::setYMinusColumn(const AbstractColumn* column) {
	Q_D(ErrorBar);
	if (column != d->yMinusColumn)
		exec(new ErrorBarSetYMinusColumnCmd(d, column, ki18n("%1: set error column")));
}

void ErrorBar::setYMinusColumnPath(const QString& path) {
	Q_D(ErrorBar);
	d->yMinusColumnPath = path;
}

void ErrorBar::yPlusColumnAboutToBeRemoved(const AbstractAspect* aspect) {
	Q_D(ErrorBar);
	if (aspect == d->yPlusColumn) {
		d->yPlusColumn = nullptr;
		d->update();
	}
}

void ErrorBar::yMinusColumnAboutToBeRemoved(const AbstractAspect* aspect) {
	Q_D(ErrorBar);
	if (aspect == d->yMinusColumn) {
		d->yMinusColumn = nullptr;
		d->update();
	}
}

STD_SETTER_CMD_IMPL_F_S(ErrorBar, SetType, ErrorBar::Type, type, update)
void ErrorBar::setType(Type type) {
	Q_D(ErrorBar);
	if (type != d->type)
		exec(new ErrorBarSetTypeCmd(d, type, ki18n("%1: error bar type changed")));
}

STD_SETTER_CMD_IMPL_F_S(ErrorBar, SetCapSize, double, capSize, update)
void ErrorBar::setCapSize(qreal size) {
	Q_D(ErrorBar);
	if (size != d->capSize)
		exec(new ErrorBarSetCapSizeCmd(d, size, ki18n("%1: set error bar cap size")));
}

// ##############################################################################
// ####################### Private implementation ###############################
// ##############################################################################
ErrorBarPrivate::ErrorBarPrivate(ErrorBar* owner, ErrorBar::Dimension dim)
	: dimension(dim), q(owner) {
}

QString ErrorBarPrivate::name() const {
	return q->parentAspect()->name();
}

void ErrorBarPrivate::update() {
	Q_EMIT q->updateRequested();
}

// ##############################################################################
// ##################  Serialization/Deserialization  ###########################
// ##############################################################################
//! Save as XML
void ErrorBar::save(QXmlStreamWriter* writer) const {
	Q_D(const ErrorBar);

	// if (!d->prefix.isEmpty()) {
	// 	// for names in the XML file, the first letter is lower case but the camel case still remains.
	// 	QString newPrefix = d->prefix;
	// 	newPrefix.replace(0, 1, d->prefix.at(0).toLower());
 //
	// 	writer->writeAttribute(newPrefix + QStringLiteral("ErrorType"), QString::number(static_cast<int>(d->type)));
 //
	// 	if (d->plusColumn)
	// 		writer->writeAttribute(newPrefix + QStringLiteral("ErrorPlusColumn"), d->plusColumn->path());
	// 	else
	// 		writer->writeAttribute(newPrefix + QStringLiteral("ErrorPlusColumn"), QString());
 //
	// 	if (d->minusColumn)
	// 		writer->writeAttribute(newPrefix + QStringLiteral("ErrorMinusColumn"), d->minusColumn->path());
	// 	else
	// 		writer->writeAttribute(newPrefix + QStringLiteral("ErrorMinusColumn"), QString());
	// } else {
	// 	writer->writeAttribute(QStringLiteral("errorType"), QString::number(static_cast<int>(d->type)));
	// 	WRITE_COLUMN(d->plusColumn, errorPlusColumn);
	// 	WRITE_COLUMN(d->minusColumn, errorMinusColumn);
	// }

	writer->writeAttribute(QStringLiteral("type"), QString::number(static_cast<int>(d->type)));
	writer->writeAttribute(QStringLiteral("capSize"), QString::number(d->capSize));
	d->line->save(writer);
}

//! Load from XML
bool ErrorBar::load(XmlStreamReader* reader, bool preview) {
	if (preview)
		return true;

	Q_D(ErrorBar);
	QString str;
	auto attribs = reader->attributes();
	int errorType = 0;

	// if (!d->prefix.isEmpty()) {
	// 	QString newPrefix = d->prefix;
	// 	newPrefix.replace(0, 1, d->prefix.at(0).toLower());
 //
	// 	errorType = attribs.value(newPrefix + QStringLiteral("ErrorType")).toInt();
	// 	d->plusColumnPath = attribs.value(newPrefix + QStringLiteral("ErrorPlusColumn")).toString();
	// 	d->minusColumnPath = attribs.value(newPrefix + QStringLiteral("ErrorMinusColumn")).toString();
	// } else {
	// 	errorType = attribs.value(QStringLiteral("errorType")).toInt();
	// 	d->plusColumnPath = attribs.value(QStringLiteral("errorPlusColumn")).toString();
	// 	d->minusColumnPath = attribs.value(QStringLiteral("errorMinusColumn")).toString();
	// }
 //
	// // prior to XML version 11, a different order of enum values for the error type was used in Histogram
	// // (old "{ NoError, Poisson, CustomSymmetric, CustomAsymmetric }" instead of
	// // the new "{ NoError, Symmetric, Asymmetric, Poisson }")
	// // and we need to map from the old to the new values
	// if (Project::xmlVersion() < 11 && parentAspect()->type() == AspectType::Histogram) {
	// 	if (errorType == 0)
	// 		d->errorType = ErrorBar::ErrorType::NoError;
	// 	else if (errorType == 1)
	// 		d->errorType = ErrorBar::ErrorType::Poisson;
	// 	else if (errorType == 2)
	// 		d->errorType = ErrorBar::ErrorType::Symmetric;
	// 	else if (errorType == 3)
	// 		d->errorType = ErrorBar::ErrorType::Asymmetric;
	// } else
	// 	d->errorType = static_cast<ErrorType>(errorType);

	READ_INT_VALUE("type", type, ErrorBar::Type);
	READ_DOUBLE_VALUE("capSize", capSize);
	d->line->load(reader, preview);

	return true;
}

// ##############################################################################
// #########################  Theme management ##################################
// ##############################################################################
void ErrorBar::loadThemeConfig(const KConfigGroup& group) {
	Q_D(ErrorBar);
	d->line->loadThemeConfig(group);
}

void ErrorBar::loadThemeConfig(const KConfigGroup& group, const QColor& themeColor) {
	Q_D(ErrorBar);
	d->line->loadThemeConfig(group, themeColor);
}

void ErrorBar::saveThemeConfig(KConfigGroup& group) const {
	Q_D(const ErrorBar);
	d->line->saveThemeConfig(group);
}
