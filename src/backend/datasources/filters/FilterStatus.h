/*
 File                 : FilterStatus.h
 Project              : LabPlot
 Description          : Status Flags of the filters
 --------------------------------------------------------------------
 SPDX-FileCopyrightText: 2025 Martin Marmsoler <martin.marmsoler@gmail.com>
 SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef FILTERSTATUS_H
#define FILTERSTATUS_H

#include <QString>

#define SIMPLE_STATUS_DECL(MessageType) static Status MessageType()

struct Status {
	Status() = delete;

	enum class Type {
		Success,
		UnableToOpenDevice,
		DeviceAtEnd,
		NotEnoughRowsSelected,
		NoNewLine,
		SeparatorDeterminationFailed,
		SequentialDeviceHeaderEnabled,
		SequentialDeviceAutomaticSeparatorDetection,
		SequentialDeviceNoColumnModes,
		InvalidNumberDataColumns,
		InvalidNumberColumnNames,
		NotEnoughMemory,
		UnsupportedDataSource,
		UnableParsingHeader,
		MatrixUnsupportedColumnMode,
		NoDateTimeFormat,
		HeaderDetectionNotAllowed,
		SeparatorDetectionNotAllowed,
		InvalidSeparator,
		SerialDeviceUninitialized,
		NoColumns,
		ColumnModeDeterminationFailed,
		WrongEndColumn,
		WrongEndRow,
		UTF16NotSupported,
		NoDataSource
	};

	Status(Type status, const QString& message)
		: m_status(status)
		, m_statusMessage(message) {
	}

	SIMPLE_STATUS_DECL(Success);
	SIMPLE_STATUS_DECL(DeviceAtEnd);
	SIMPLE_STATUS_DECL(NotEnoughRowsSelected);
	SIMPLE_STATUS_DECL(UnableToOpenDevice);
	SIMPLE_STATUS_DECL(NoNewLine);
	SIMPLE_STATUS_DECL(SeparatorDeterminationFailed);
	SIMPLE_STATUS_DECL(InvalidNumberColumnNames);
	SIMPLE_STATUS_DECL(MatrixUnsupportedColumnMode);
	SIMPLE_STATUS_DECL(NotEnoughMemory);
	SIMPLE_STATUS_DECL(UnableParsingHeader);
	SIMPLE_STATUS_DECL(UnsupportedDataSource);
	SIMPLE_STATUS_DECL(SequentialDeviceHeaderEnabled);
	SIMPLE_STATUS_DECL(SequentialDeviceAutomaticSeparatorDetection);
	SIMPLE_STATUS_DECL(SequentialDeviceNoColumnModes);
	SIMPLE_STATUS_DECL(NoDateTimeFormat);
	SIMPLE_STATUS_DECL(HeaderDetectionNotAllowed);
	SIMPLE_STATUS_DECL(SeparatorDetectionNotAllowed);
	SIMPLE_STATUS_DECL(InvalidSeparator);
	SIMPLE_STATUS_DECL(SerialDeviceUninitialized);
	SIMPLE_STATUS_DECL(WrongEndColumn);
	SIMPLE_STATUS_DECL(WrongEndRow);
	SIMPLE_STATUS_DECL(NoDataSource);
	SIMPLE_STATUS_DECL(NoColumns);
	SIMPLE_STATUS_DECL(ColumnModeDeterminationFailed);
	SIMPLE_STATUS_DECL(UTF16NotSupported);
	static Status InvalidNumberDataColumns(int expectedNumberColumns, int lineIndex, int receivedColumnCount);

	Type type() const {
		return m_status;
	}
	const QString& message() const {
		return m_statusMessage;
	}
	bool success() const {
		return m_status == Type::Success;
	}

private:
	Type m_status{Type::Success};
	QString m_statusMessage;
};

#endif
