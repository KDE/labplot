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
  \brief This class contains the background properties of worksheet elements like worksheet background,
  plot background, the area filling in ErrorBar, ErrorBar, etc.

  \ingroup worksheet
*/

#include "ErrorBar.h"
#include "ErrorBarPrivate.h"
#include "backend/core/AbstractColumn.h"
#include "backend/core/Project.h"
#include "backend/lib/XmlStreamReader.h"
#include "backend/lib/commandtemplates.h"
#include "backend/lib/macrosCurve.h"
#include "backend/worksheet/plots/cartesian/CartesianCoordinateSystem.h"
#include "backend/worksheet/plots/cartesian/ErrorBarStyle.h"

#include <KConfigGroup>
#include <KLocalizedString>

CURVE_COLUMN_CONNECT(ErrorBar, Plus, plus, update)
CURVE_COLUMN_CONNECT(ErrorBar, Minus, minus, update)

ErrorBar::ErrorBar(const QString& name)
	: AbstractAspect(name, AspectType::AbstractAspect)
	, d_ptr(new ErrorBarPrivate(this)) {
}

ErrorBar::~ErrorBar() {
	delete d_ptr;
}

void ErrorBar::setPrefix(const QString& prefix) {
	Q_D(ErrorBar);
	d->prefix = prefix;
}

const QString& ErrorBar::prefix() const {
	Q_D(const ErrorBar);
	return d->prefix;
}

void ErrorBar::init(const KConfigGroup& group) {
	Q_D(ErrorBar);
	d->type = (Type)group.readEntry(d->prefix + QStringLiteral("ErrorType"), static_cast<int>(Type::NoError));
}

void ErrorBar::update() {
	Q_D(ErrorBar);
	d->update();
}

QPainterPath ErrorBar::painterPath(const ErrorBarStyle* style, const QVector<QPointF>& points, const CartesianCoordinateSystem* cSystem, WorksheetElement::Orientation orientation) const {
	Q_D(const ErrorBar);
	auto errorBarsPath = QPainterPath();
	QVector<QLineF> elines;

	switch (d->type) {
	case ErrorBar::Type::NoError:
		break;
	case ErrorBar::Type::Poisson: {
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
	case ErrorBar::Type::Symmetric: {
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
	case ErrorBar::Type::Asymmetric: {
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

	// map the error bars to scene coordinates
	elines =cSystem->mapLogicalToScene(elines);

	// new painter path for the error bars
	for (const auto& line : qAsConst(elines)) {
		errorBarsPath.moveTo(line.p1());
		errorBarsPath.lineTo(line.p2());
	}

	// add caps for error bars
	const auto capSize = style->capSize();
	if (style->type() == ErrorBarStyle::Type::WithEnds) {
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
BASIC_SHARED_D_READER_IMPL(ErrorBar, ErrorBar::Type, type, type)
BASIC_SHARED_D_READER_IMPL(ErrorBar, const AbstractColumn*, plusColumn, plusColumn)
BASIC_SHARED_D_READER_IMPL(ErrorBar, const AbstractColumn*, minusColumn, minusColumn)
BASIC_SHARED_D_READER_IMPL(ErrorBar, QString, plusColumnPath, plusColumnPath)
BASIC_SHARED_D_READER_IMPL(ErrorBar, QString, minusColumnPath, minusColumnPath)

// ##############################################################################
// #################  setter methods and undo commands ##########################
// ##############################################################################
STD_SETTER_CMD_IMPL_F_S(ErrorBar, SetType, ErrorBar::Type, type, update)
void ErrorBar::setType(Type type) {
	Q_D(ErrorBar);
	if (type != d->type)
		exec(new ErrorBarSetTypeCmd(d, type, ki18n("%1: error type changed")));
}

CURVE_COLUMN_SETTER_CMD_IMPL_F_S(ErrorBar, Plus, plus, update)
void ErrorBar::setPlusColumn(const AbstractColumn* column) {
	Q_D(ErrorBar);
	if (column != d->plusColumn)
		exec(new ErrorBarSetPlusColumnCmd(d, column, ki18n("%1: set error column")));
}

void ErrorBar::setPlusColumnPath(const QString& path) {
	Q_D(ErrorBar);
	d->plusColumnPath = path;
}

CURVE_COLUMN_SETTER_CMD_IMPL_F_S(ErrorBar, Minus, minus, update)
void ErrorBar::setMinusColumn(const AbstractColumn* column) {
	Q_D(ErrorBar);
	if (column != d->minusColumn)
		exec(new ErrorBarSetMinusColumnCmd(d, column, ki18n("%1: set error column")));
}

void ErrorBar::setMinusColumnPath(const QString& path) {
	Q_D(ErrorBar);
	d->minusColumnPath = path;
}

void ErrorBar::plusColumnAboutToBeRemoved(const AbstractAspect* aspect) {
	Q_D(ErrorBar);
	if (aspect == d->plusColumn) {
		d->plusColumn = nullptr;
		d->update();
	}
}

void ErrorBar::minusColumnAboutToBeRemoved(const AbstractAspect* aspect) {
	Q_D(ErrorBar);
	if (aspect == d->minusColumn) {
		d->minusColumn = nullptr;
		d->update();
	}
}

// ##############################################################################
// ####################### Private implementation ###############################
// ##############################################################################
ErrorBarPrivate::ErrorBarPrivate(ErrorBar* owner)
	: q(owner) {
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

	if (!d->prefix.isEmpty()) {
		// for names in the XML file, the first letter is lower case but the camel case still remains.
		QString newPrefix = d->prefix;
		newPrefix.replace(0, 1, d->prefix.at(0).toLower());

		writer->writeAttribute(newPrefix + QStringLiteral("ErrorType"), QString::number(static_cast<int>(d->type)));

		if (d->plusColumn)
			writer->writeAttribute(newPrefix + QStringLiteral("ErrorPlusColumn"), d->plusColumn->path());
		else
			writer->writeAttribute(newPrefix + QStringLiteral("ErrorPlusColumn"), QString());

		if (d->minusColumn)
			writer->writeAttribute(newPrefix + QStringLiteral("ErrorMinusColumn"), d->minusColumn->path());
		else
			writer->writeAttribute(newPrefix + QStringLiteral("ErrorMinusColumn"), QString());
	} else {
		writer->writeAttribute(QStringLiteral("errorType"), QString::number(static_cast<int>(d->type)));
		WRITE_COLUMN(d->plusColumn, errorPlusColumn);
		WRITE_COLUMN(d->minusColumn, errorMinusColumn);
	}
}

//! Load from XML
bool ErrorBar::load(XmlStreamReader* reader, bool preview) {
	if (preview)
		return true;

	Q_D(ErrorBar);
	QString str;
	auto attribs = reader->attributes();
	int type = 0;

	if (!d->prefix.isEmpty()) {
		QString newPrefix = d->prefix;
		newPrefix.replace(0, 1, d->prefix.at(0).toLower());

		type = attribs.value(newPrefix + QStringLiteral("ErrorType")).toInt();
		d->plusColumnPath = attribs.value(newPrefix + QStringLiteral("ErrorPlusColumn")).toString();
		d->minusColumnPath = attribs.value(newPrefix + QStringLiteral("ErrorMinusColumn")).toString();
	} else {
		type = attribs.value(QStringLiteral("errorType")).toInt();
		d->plusColumnPath = attribs.value(QStringLiteral("errorPlusColumn")).toString();
		d->minusColumnPath = attribs.value(QStringLiteral("errorMinusColumn")).toString();
	}

	// prior to XML version 11, a different order of enum values for the error type was used in Histogram
	// (old "{ NoError, Poisson, CustomSymmetric, CustomAsymmetric }" instead of
	// the new "{ NoError, Symmetric, Asymmetric, Poisson }")
	// and we need to map from the old to the new values
	if (Project::xmlVersion() < 11 && parentAspect()->type() == AspectType::Histogram) {
		if (type == 0)
			d->type = ErrorBar::Type::NoError;
		else if (type == 1)
			d->type = ErrorBar::Type::Poisson;
		else if (type == 2)
			d->type = ErrorBar::Type::Symmetric;
		else if (type == 3)
			d->type = ErrorBar::Type::Asymmetric;
	} else
		d->type = static_cast<Type>(type);

	return true;
}
