/*
 * SPDX-FileCopyrightText: 2006-2011 the LibQxt project <http://libqxt.org, foundation@libqxt.org>
 * SPDX-License-Identifier: BSD-3-Clause
*/

#ifndef QXTNAMESPACE_H
#define QXTNAMESPACE_H

#include "qxtglobal.h"

#if (defined BUILD_QXT | defined Q_MOC_RUN) && !defined(QXT_DOXYGEN_RUN)
#include <QObject>

class QXT_CORE_EXPORT Qxt  : public QObject {
	Q_OBJECT
	Q_ENUMS(Rotation)
	Q_ENUMS(DecorationStyle)
	Q_ENUMS(ErrorCode)

public:
#else
namespace Qxt {
#endif
	enum Rotation {
		NoRotation  = 0,
		UpsideDown  = 180,
		Clockwise  = 90,
		CounterClockwise = 270
	};

	enum DecorationStyle {
		NoDecoration,
		Buttonlike,
		Menulike
	};

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

	enum QxtItemDataRole {
		ItemStartTimeRole  = Qt::UserRole + 1,
		ItemDurationRole   = ItemStartTimeRole + 1,
		UserRole           = ItemDurationRole + 23
	};

	enum Timeunit {
		Second,
		Minute,
		Hour,
		Day,
		Week,
		Month,
		Year
	};
};

#endif // QXTNAMESPACE_H
