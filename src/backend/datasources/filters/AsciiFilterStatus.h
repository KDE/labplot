/*
 File                 : AsciiFilterStatus.h
 Project              : LabPlot
 Description          : Status Flags of Asciifilter
 --------------------------------------------------------------------
 SPDX-FileCopyrightText: 2025 Martin Marmsoler <martin.marmsoler@gmail.com>
 SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef ASCIIFILTERSTATUS_H
#define ASCIIFILTERSTATUS_H

#include <QString>
#include <klocalizedstring.h>

#define SIMPLE_STATUS(MessageType, Message)                                                                                                                    \
	static Status MessageType() {                                                                                                                              \
		return Status(Type::MessageType, i18n(Message));                                                                                                       \
	}

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

	SIMPLE_STATUS(Success, "Success")
	SIMPLE_STATUS(DeviceAtEnd, "Device at end")
	SIMPLE_STATUS(NotEnoughRowsSelected, "Not enough rows selected. Increase number of rows.")
	SIMPLE_STATUS(UnableToOpenDevice, "Unable to open device")
	SIMPLE_STATUS(NoNewLine, "No new line detected")
	SIMPLE_STATUS(SeparatorDeterminationFailed, "Unable to determine the separator")
	SIMPLE_STATUS(InvalidNumberColumnNames, "Invalid number of column names")
	SIMPLE_STATUS(MatrixUnsupportedColumnMode, "Matrix: Unsupported column mode")
	SIMPLE_STATUS(NotEnoughMemory, "Insufficient memory (RAM)")
	SIMPLE_STATUS(UnableParsingHeader, "Unable to parse header")
	SIMPLE_STATUS(UnsupportedDataSource, "Unsupported data source")
	SIMPLE_STATUS(SequentialDeviceHeaderEnabled, "Header detection for sequential device not supported")
	SIMPLE_STATUS(SequentialDeviceAutomaticSeparatorDetection, "Live: No column separator selected")
	SIMPLE_STATUS(SequentialDeviceNoColumnModes, "Live: No column modes set")
	SIMPLE_STATUS(NoDateTimeFormat, "Datetime column found, but no Datetime format provided")
	SIMPLE_STATUS(HeaderDetectionNotAllowed, "Header reading from device not allowed")
	SIMPLE_STATUS(SeparatorDetectionNotAllowed, "Separator detection not allowed")
	SIMPLE_STATUS(InvalidSeparator, "Invalid separator")
	SIMPLE_STATUS(SerialDeviceUninitialized, "Serial device must be initialized before reading data from it")
	SIMPLE_STATUS(WrongEndColumn, "Wrong end column. Is it smaller than start column?")
	SIMPLE_STATUS(WrongEndRow, "Wrong end row. Is it smaller than start row?")
	SIMPLE_STATUS(NoDataSource, "No data destination set")
	SIMPLE_STATUS(NoColumns, "No columns")
	SIMPLE_STATUS(ColumnModeDeterminationFailed, "Unable to determine column modes. Check if they are correctly written")
	SIMPLE_STATUS(UTF16NotSupported, "UTF16 encoding is not supported")

	static Status InvalidNumberDataColumns(int expectedNumberColumns, int lineIndex, int receivedColumnCount) {
		return Status(
			Type::InvalidNumberDataColumns,
			i18n("Invalid number of data columns. First row column count: %1. %2th row column count >= %3. Check if the correct separator is used and the "
				 "data contains same number of columns.")
				.arg(expectedNumberColumns)
				.arg(lineIndex)
				.arg(receivedColumnCount));
	}

	Type type() const {
		return m_status;
	}
	QString message() const {
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
