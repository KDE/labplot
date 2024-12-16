/*
 * SPDX-FileCopyrightText: 2006-2011 the LibQxt project <http://libqxt.org, foundation@libqxt.org>
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef QXTNAMESPACE_H
#define QXTNAMESPACE_H

#include "qxtglobal.h"
#include <Qt>

#if (defined BUILD_QXT | defined Q_MOC_RUN) && !defined(QXT_DOXYGEN_RUN)
#include <QObject>
class QXT_CORE_EXPORT Qxt : public QObject {
	Q_OBJECT

public:
#else
namespace Qxt {
	Q_NAMESPACE
#endif
	enum Rotation { NoRotation = 0, UpsideDown = 180, Clockwise = 90, CounterClockwise = 270 };
	enum DecorationStyle { NoDecoration, Buttonlike, Menulike };

	enum ErrorCode {
		NoError,
		UnknownError,
		LogicalError,
		Bug,
		UnexpectedEndOfFunction,
		NotImplemented,
		CodecError,
		NotInitialised,
		EndOfFile,
		FileIOError,
		FormatError,
		DeviceError,
		SDLError,
		InsufficientMemory,
		SeeErrorString,
		UnexpectedNullParameter,
		ClientTimeout,
		SocketIOError,
		ParserError,
		HeaderTooLong,
		Auth,
		Overflow
	};
#if (defined BUILD_QXT | defined Q_MOC_RUN) && !defined(QXT_DOXYGEN_RUN)
	Q_ENUM(Rotation)
	Q_ENUM(DecorationStyle)
	Q_ENUM(ErrorCode)
#else
	Q_ENUM_NS(Rotation)
	Q_ENUM_NS(DecorationStyle)
	Q_ENUM_NS(ErrorCode)
#endif

	enum QxtItemDataRole { ItemStartTimeRole = Qt::UserRole + 1, ItemDurationRole = ItemStartTimeRole + 1, UserRole = ItemDurationRole + 23 };

	enum Timeunit { Second, Minute, Hour, Day, Week, Month, Year };
}

#endif // QXTNAMESPACE_H
