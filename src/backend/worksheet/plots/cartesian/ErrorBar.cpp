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
#include "backend/lib/XmlStreamReader.h"
#include "backend/lib/commandtemplates.h"
#include "backend/lib/macrosCurve.h"
#include "backend/worksheet/Line.h"
#include "backend/worksheet/Worksheet.h"
#include "backend/worksheet/plots/cartesian/CartesianCoordinateSystem.h"

#include <KConfigGroup>
#include <KLocalizedString>
#include <QPainter>

CURVE_COLUMN_CONNECT(ErrorBar, XPlus, xPlus, update)
CURVE_COLUMN_CONNECT(ErrorBar, XMinus, xMinus, update)
CURVE_COLUMN_CONNECT(ErrorBar, YPlus, yPlus, update)
CURVE_COLUMN_CONNECT(ErrorBar, YMinus, yMinus, update)

ErrorBar::ErrorBar(const QString& name, Dimension dim)
	: AbstractAspect(name, AspectType::AbstractAspect)
	, d_ptr(new ErrorBarPrivate(this, dim)) {
	Q_D(ErrorBar);
	d->line = new Line(QString());
	d->line->setHidden(true);
	d->line->setCreateXmlElement(false);
	addChild(d->line);
	connect(d->line, &Line::updatePixmapRequested, this, &ErrorBar::updatePixmapRequested);
	connect(d->line, &Line::updateRequested, this, &ErrorBar::updateRequested);
}

ErrorBar::~ErrorBar() {
	delete d_ptr;
}

void ErrorBar::init(const KConfigGroup& group) {
	Q_D(ErrorBar);
	// x and y errors
	switch (d->dimension) {
	case Dimension::Y:
		d->yErrorType = (ErrorType)group.readEntry(QStringLiteral("ErrorType"), static_cast<int>(ErrorType::NoError));
		break;
	case Dimension::XY:
		d->xErrorType = (ErrorType)group.readEntry(QStringLiteral("XErrorType"), static_cast<int>(ErrorType::NoError));
		d->yErrorType = (ErrorType)group.readEntry(QStringLiteral("YErrorType"), static_cast<int>(ErrorType::NoError));
	}

	// styling
	d->type = (Type)group.readEntry(QStringLiteral("ErrorBarsType"), static_cast<int>(Type::Simple));
	d->capSize = group.readEntry(QStringLiteral("ErrorBarsCapSize"), Worksheet::convertToSceneUnits(10, Worksheet::Unit::Point));
	d->line->init(group);
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

/*!
 * calculates and returns the painter path for the error bars.
 * The error bars are placed at the points in logical coordinates \c points.
 * The transformation to the scene coordinates is done via \c cSystem that is owned by the parent plot owning the error bar object.
 * Some plot types like histogram, bar plot, etc. can have different orientations (vertical vs. horizontal) which is provided via \c orientation,
 * the error is specified for the y-dimentions for such plot types and is drawn either vertically or horizontally depending on the orientation of the plot.
 */
QPainterPath ErrorBar::painterPath(const QVector<QPointF>& points, const CartesianCoordinateSystem* cSystem, WorksheetElement::Orientation orientation) const {
	Q_D(const ErrorBar);
	auto path = QPainterPath();

	if (d->dimension == Dimension::XY)
		d->painterPathForX(path, points, cSystem);

	d->painterPathForY(path, points, cSystem, orientation);

	return path;
}

// ##############################################################################
// ##########################  getter methods  ##################################
// ##############################################################################
ErrorBar::Dimension ErrorBar::dimension() const {
	Q_D(const ErrorBar);
	return d->dimension;
}

// x
BASIC_SHARED_D_READER_IMPL(ErrorBar, ErrorBar::ErrorType, xErrorType, xErrorType)
BASIC_SHARED_D_READER_IMPL(ErrorBar, const AbstractColumn*, xPlusColumn, xPlusColumn)
BASIC_SHARED_D_READER_IMPL(ErrorBar, const AbstractColumn*, xMinusColumn, xMinusColumn)
BASIC_SHARED_D_READER_IMPL(ErrorBar, QString, xPlusColumnPath, xPlusColumnPath)
BASIC_SHARED_D_READER_IMPL(ErrorBar, QString, xMinusColumnPath, xMinusColumnPath)

// y
BASIC_SHARED_D_READER_IMPL(ErrorBar, ErrorBar::ErrorType, yErrorType, yErrorType)
BASIC_SHARED_D_READER_IMPL(ErrorBar, const AbstractColumn*, yPlusColumn, yPlusColumn)
BASIC_SHARED_D_READER_IMPL(ErrorBar, const AbstractColumn*, yMinusColumn, yMinusColumn)
BASIC_SHARED_D_READER_IMPL(ErrorBar, QString, yPlusColumnPath, yPlusColumnPath)
BASIC_SHARED_D_READER_IMPL(ErrorBar, QString, yMinusColumnPath, yMinusColumnPath)

// styling
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
	: dimension(dim)
	, q(owner) {
}

QString ErrorBarPrivate::name() const {
	return q->parentAspect()->name();
}

void ErrorBarPrivate::update() {
	Q_EMIT q->updateRequested();
}

void ErrorBarPrivate::painterPathForX(QPainterPath& path, const QVector<QPointF>& points, const CartesianCoordinateSystem* cSystem) const {
	QVector<QLineF> elines;

	switch (xErrorType) {
	case ErrorBar::ErrorType::NoError:
	case ErrorBar::ErrorType::Poisson:
		return;
	case ErrorBar::ErrorType::Symmetric: {
		int index = 0;
		for (auto& point : points) {
			if (xPlusColumn && xPlusColumn->isValid(index) && !xPlusColumn->isMasked(index)) {
				const double error = xPlusColumn->valueAt(index);
				if (error != 0.)
					elines << QLineF(point.x() - error, point.y(), point.x() + error, point.y());
			}
			++index;
		}

		break;
	}
	case ErrorBar::ErrorType::Asymmetric: {
		int index = 0;
		for (auto& point : points) {
			double errorPlus = 0.;
			double errorMinus = 0.;

			if (xPlusColumn && xPlusColumn->isValid(index) && !xPlusColumn->isMasked(index))
				errorPlus = xPlusColumn->valueAt(index);

			if (xMinusColumn && xMinusColumn->isValid(index) && !xMinusColumn->isMasked(index))
				errorMinus = xMinusColumn->valueAt(index);

			if (errorPlus != 0. || errorMinus != 0.)
				elines << QLineF(point.x() - errorMinus, point.y(), point.x() + errorPlus, point.y());

			++index;
		}

		break;
	}
	}

	// map the error bars to scene coordinates
	cSystem->mapLogicalToSceneDefaultMapping(elines);

	// new painter path for the error bars
	for (const auto& line : std::as_const(elines)) {
		path.moveTo(line.p1());
		path.lineTo(line.p2());
	}

	// add caps for error bars
	if (type == ErrorBar::Type::WithEnds) {
		for (const auto& line : std::as_const(elines)) {
			const auto& p1 = line.p1();
			path.moveTo(QPointF(p1.x(), p1.y() - capSize / 2.));
			path.lineTo(QPointF(p1.x(), p1.y() + capSize / 2.));

			const auto& p2 = line.p2();
			path.moveTo(QPointF(p2.x(), p2.y() - capSize / 2.));
			path.lineTo(QPointF(p2.x(), p2.y() + capSize / 2.));
		}
	}
}

// error bars for y
void ErrorBarPrivate::painterPathForY(QPainterPath& path,
									  const QVector<QPointF>& points,
									  const CartesianCoordinateSystem* cSystem,
									  WorksheetElement::Orientation orientation) const {
	QVector<QLineF> elines;

	switch (yErrorType) {
	case ErrorBar::ErrorType::NoError:
		return;
	case ErrorBar::ErrorType::Poisson: {
		if (orientation == WorksheetElement::Orientation::Vertical) {
			for (auto& point : points) {
				const double error = sqrt(point.y());
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
			for (auto& point : points) {
				if (yPlusColumn && yPlusColumn->isValid(index) && !yPlusColumn->isMasked(index)) {
					const double error = yPlusColumn->valueAt(index);
					if (error != 0.)
						elines << QLineF(point.x(), point.y() + error, point.x(), point.y() - error);
				}
				++index;
			}
		} else {
			for (auto& point : points) {
				if (yPlusColumn && yPlusColumn->isValid(index) && !yPlusColumn->isMasked(index)) {
					double error = yPlusColumn->valueAt(index);
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

				if (yPlusColumn && yPlusColumn->isValid(index) && !yPlusColumn->isMasked(index))
					errorPlus = yPlusColumn->valueAt(index);

				if (yMinusColumn && yMinusColumn->isValid(index) && !yMinusColumn->isMasked(index))
					errorMinus = yMinusColumn->valueAt(index);

				if (errorPlus != 0. || errorMinus != 0.)
					elines << QLineF(point.x(), point.y() - errorMinus, point.x(), point.y() + errorPlus);

				++index;
			}
		} else {
			for (auto& point : points) {
				double errorPlus = 0.;
				double errorMinus = 0.;

				if (yPlusColumn && yPlusColumn->isValid(index) && !yPlusColumn->isMasked(index))
					errorPlus = yPlusColumn->valueAt(index);

				if (yMinusColumn && yMinusColumn->isValid(index) && !yMinusColumn->isMasked(index))
					errorMinus = yMinusColumn->valueAt(index);

				if (errorPlus != 0. || errorMinus != 0.)
					elines << QLineF(point.x() - errorMinus, point.y(), point.x() + errorPlus, point.y());

				++index;
			}
		}
		break;
	}
	}

	// map the error bars to scene coordinates
	cSystem->mapLogicalToSceneDefaultMapping(elines);

	// new painter path for the error bars
	for (const auto& line : std::as_const(elines)) {
		path.moveTo(line.p1());
		path.lineTo(line.p2());
	}

	// add caps for error bars
	if (type == ErrorBar::Type::WithEnds) {
		if (orientation == WorksheetElement::Orientation::Vertical) {
			for (const auto& line : std::as_const(elines)) {
				const auto& p1 = line.p1();
				path.moveTo(QPointF(p1.x() - capSize / 2., p1.y()));
				path.lineTo(QPointF(p1.x() + capSize / 2., p1.y()));

				const auto& p2 = line.p2();
				path.moveTo(QPointF(p2.x() - capSize / 2., p2.y()));
				path.lineTo(QPointF(p2.x() + capSize / 2., p2.y()));
			}
		} else {
			for (const auto& line : std::as_const(elines)) {
				const auto& p1 = line.p1();
				path.moveTo(QPointF(p1.x(), p1.y() - capSize / 2.));
				path.lineTo(QPointF(p1.x(), p1.y() + capSize / 2.));

				const auto& p2 = line.p2();
				path.moveTo(QPointF(p2.x(), p2.y() - capSize / 2.));
				path.lineTo(QPointF(p2.x(), p2.y() + capSize / 2.));
			}
		}
	}
}

// ##############################################################################
// ##################  Serialization/Deserialization  ###########################
// ##############################################################################
//! Save as XML
void ErrorBar::save(QXmlStreamWriter* writer) const {
	Q_D(const ErrorBar);

	// x and y error values
	switch (d->dimension) {
	case (Dimension::XY): {
		writer->writeAttribute(QStringLiteral("xErrorType"), QString::number(static_cast<int>(d->xErrorType)));
		WRITE_COLUMN(d->xPlusColumn, xErrorPlusColumn);
		WRITE_COLUMN(d->xMinusColumn, xErrorMinusColumn);
		writer->writeAttribute(QStringLiteral("yErrorType"), QString::number(static_cast<int>(d->yErrorType)));
		WRITE_COLUMN(d->yPlusColumn, yErrorPlusColumn);
		WRITE_COLUMN(d->yMinusColumn, yErrorMinusColumn);
		break;
	}
	case (Dimension::Y): {
		writer->writeAttribute(QStringLiteral("errorType"), QString::number(static_cast<int>(d->yErrorType)));
		WRITE_COLUMN(d->yPlusColumn, errorPlusColumn);
		WRITE_COLUMN(d->yMinusColumn, errorMinusColumn);
	}
	}

	// styling
	writer->writeAttribute(QStringLiteral("type"), QString::number(static_cast<int>(d->type)));
	writer->writeAttribute(QStringLiteral("capSize"), QString::number(d->capSize));
	d->line->save(writer);
}

//! Load from XML
bool ErrorBar::load(XmlStreamReader* reader, bool preview) {
	if (preview)
		return true;

	Q_D(ErrorBar);
	auto attribs = reader->attributes();

	// x and y error values
	switch (d->dimension) {
	case (Dimension::XY): {
		d->xErrorType = static_cast<ErrorBar::ErrorType>(attribs.value(QStringLiteral("xErrorType")).toInt());
		d->xPlusColumnPath = attribs.value(QStringLiteral("xErrorPlusColumn")).toString();
		d->xMinusColumnPath = attribs.value(QStringLiteral("xErrorMinusColumn")).toString();

		d->yErrorType = static_cast<ErrorBar::ErrorType>(attribs.value(QStringLiteral("yErrorType")).toInt());
		d->yPlusColumnPath = attribs.value(QStringLiteral("yErrorPlusColumn")).toString();
		d->yMinusColumnPath = attribs.value(QStringLiteral("yErrorMinusColumn")).toString();
		break;
	}
	case (Dimension::Y): {
		d->yErrorType = static_cast<ErrorBar::ErrorType>(attribs.value(QStringLiteral("errorType")).toInt());
		d->yPlusColumnPath = attribs.value(QStringLiteral("errorPlusColumn")).toString();
		d->yMinusColumnPath = attribs.value(QStringLiteral("errorMinusColumn")).toString();
	}
	}

	// styling
	if (!preview) {
		QString str;
		READ_INT_VALUE("type", type, ErrorBar::Type);
		READ_DOUBLE_VALUE("capSize", capSize);
		d->line->load(reader, preview);
	}

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
