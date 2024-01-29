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
#include "backend/lib/XmlStreamReader.h"
#include "backend/lib/commandtemplates.h"
#include "backend/lib/macrosCurve.h"

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

	if (!d->prefix.isEmpty()) {
		QString newPrefix = d->prefix;
		newPrefix.replace(0, 1, d->prefix.at(0).toLower());

		str = attribs.value(newPrefix + QStringLiteral("ErrorType")).toString();
		if (str.isEmpty())
			reader->raiseMissingAttributeWarning(newPrefix + QStringLiteral("ErrorType"));
		else
			d->type = static_cast<Type>(str.toInt());

		d->plusColumnPath = attribs.value(newPrefix + QStringLiteral("ErrorPlusColumn")).toString();
		d->minusColumnPath = attribs.value(newPrefix + QStringLiteral("ErrorMinusColumn")).toString();
	} else {
		READ_INT_VALUE("errorType", type, Type);
		d->plusColumnPath = attribs.value(QStringLiteral("errorPlusColumn")).toString();
		d->minusColumnPath = attribs.value(QStringLiteral("errorMinusColumn")).toString();
	}

	return true;
}
