/*
	File                 : AsciiFilter.cpp
	Project              : LabPlot
	Description          : ASCII I/O-filter
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2024 Martin Marmsoler <martin.marmsoler@gmail.com>

	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "AsciiFilterNew.h"
#include "AsciiFilterNewPrivate.h"
#include "backend/spreadsheet/Spreadsheet.h"

#include <KCompressionDevice>
#include <KLocalizedString>

#include <QDateTime>

AsciiFilterNew::AsciiFilterNew(): AbstractFileFilter(FileType::Ascii), d_ptr(std::make_unique<AsciiFilterNewPrivate>(this)) {

}

void AsciiFilterNew::setProperties(Properties& properties) {
	Q_D(AsciiFilterNew);
	d->dirty = true;
	d->properties = properties;
}

void AsciiFilterNew::readDataFromFile(const QString& fileName, AbstractDataSource* dataSource, ImportMode importMode) {
	KCompressionDevice file(fileName);
	//readFromDevice(file, dataSource, importMode);
}

qint64 AsciiFilterNew::readFromDevice(QIODevice& device, AbstractDataSource* dataSource, ImportMode importMode, int lines) {
	Q_D(AsciiFilterNew);
	if (d->dirty)
		initialize(device);

	return 0;
}

/*!
 * \brief AsciiFilterNew::initialize
 * Determine all automatic values like separator, numberRows, numberColumns
 */
void AsciiFilterNew::initialize(QIODevice& device) {
	Q_D(AsciiFilterNew);
	d->initialize(device);
}

void AsciiFilterNew::write(const QString& fileName, AbstractDataSource*) {
	// TODO
}

QVector<QStringList> AsciiFilterNew::preview(const QString& fileName, int lines) {
	Q_D(AsciiFilterNew);
	return d->preview(fileName, lines);
}

QString AsciiFilterNew::statusToString(Status e) {
	switch (e) {
	case AsciiFilterNew::Status::Success:
		return i18n("Success");
	case AsciiFilterNew::Status::DeviceAtEnd:
		return i18n("Device at end");
	case AsciiFilterNew::Status::NotEnoughRowsSelected:
		return i18n("Not enough rows selected. Increase number of rows.");
	case AsciiFilterNew::Status::UnableToOpenDevice:
		return i18n("Unable to open device");
	}
	return i18n("Unhandled case");
}

//########################################################################################################################
//##  PRIVATE IMPLEMENTATIONS  ###########################################################################################
//########################################################################################################################
AsciiFilterNewPrivate::AsciiFilterNewPrivate(AsciiFilterNew* owner): q(owner) {

}

AsciiFilterNew::Status AsciiFilterNewPrivate::initialize(QIODevice& device) {
	using Status = AsciiFilterNew::Status;

	if (!properties.automaticSeparatorDetection && properties.numberColumns > 0 && properties.columnModes.size() == properties.numberColumns)
		return Status::Success; // Nothing to do since all unknows are determined

	if (device.isSequential())
		return Status::Success; // Initialization not required

	if (!device.open(QIODevice::ReadOnly))
		return Status::UnableToOpenDevice;

	if (device.atEnd())
		return Status::DeviceAtEnd;

	if (properties.headerEnabled && properties.headerLine > 0) {
		QString header;
		int validRowCounter = 0;
		do {
			QString line;
			const auto status = getLine(device, line);
			if (status != Status::Success)
				return status;

			if (!properties.commentCharacter.isEmpty() && line.startsWith(properties.commentCharacter))
				continue;

			validRowCounter++;

			if (validRowCounter != properties.headerLine)
				continue;

			header = line;

		} while (true);
	}

	return Status::Success;
}

qint64 AsciiFilterNewPrivate::readFromDevice(QIODevice& device, AbstractDataSource* dataSource, AbstractFileFilter::ImportMode importMode, int lines) {
	return 0;
}

QString AsciiFilterNewPrivate::determineSeparator(QString& line) {
	enum class State {
		Normal,
		QuotedText,
	};

	auto state = State::Normal;

	// for (auto c: line) {
	// 	if (properties.removeQuotesEnabled && c == '"') {
	// 		if (state == State::Normal)
	// 			state = State::QuotedText;
	// 		else
	// 			state = State::Normal;
	// 		continue;
	// 	}

	// 	switch state
	// }

	return QString();
}

AsciiFilterNew::Status AsciiFilterNewPrivate::getLine(QIODevice& device, QString& line) {
	using Status = AsciiFilterNew::Status;

	if (!device.canReadLine())
		return Status::UnableToReadLine;

	if (device.atEnd())
		return Status::DeviceAtEnd;

	line = QString::fromUtf8(device.readLine());
	return Status::Success;
}

QVector<QStringList> AsciiFilterNewPrivate::preview(const QString& fileName, int lines) {
	KCompressionDevice file(fileName);
	Spreadsheet spreadsheet(QStringLiteral("AsciiFilterPreviewSpreadsheet"));
	const auto read_lines = readFromDevice(file, &spreadsheet, AbstractFileFilter::ImportMode::Replace, lines);
	// TODO: handle if not enough lines can be read

	QVector<QStringList> p;
	for (int row = 0; row < spreadsheet.rowCount(); row++) {
		QStringList line;
		for (int column = 0; column < spreadsheet.columnCount(); column++) {
			const auto c = spreadsheet.column(column);
			switch (c->columnMode()) {
			case AbstractColumn::ColumnMode::BigInt:
				line << QString::number(c->bigIntAt(row));
				break;
			case AbstractColumn::ColumnMode::Integer:
				line << QString::number(c->integerAt(row));
				break;
			case AbstractColumn::ColumnMode::Double:
				line << QString::number(c->valueAt(row));
				break;
			case AbstractColumn::ColumnMode::Day:
			case AbstractColumn::ColumnMode::Month:
			case AbstractColumn::ColumnMode::DateTime:
				line << c->dateTimeAt(row).toString(properties.dateTimeFormat);
				break;
			}
		}
		p << line;
	}
	return p;
}

void AsciiFilterNewPrivate::setLastError(AsciiFilterNew::Status status) {
	const auto s = AsciiFilterNew::statusToString(status);
	q->setLastError(s);
}
