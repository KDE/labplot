/*
	File                 : BinaryFilter.cpp
	Project              : LabPlot
	Description          : Binary I/O-filter
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2015-2022 Stefan Gerlach <stefan.gerlach@uni.kn>
	SPDX-FileCopyrightText: 2017 Alexander Semke <alexander.semke@web.de>
	SPDX-License-Identifier: GPL-2.0-or-later
*/
#include "backend/datasources/filters/BinaryFilter.h"
#include "backend/core/column/Column.h"
#include "backend/datasources/AbstractDataSource.h"
#include "backend/datasources/filters/BinaryFilterPrivate.h"
#include "backend/lib/XmlStreamReader.h"
#include "backend/lib/macros.h"

#include <KCompressionDevice>
#include <KLocalizedString>
#include <QDataStream>
#include <QtEndian>
#include <array>

#define IMPORT_DATA(DATATYPE, TARGETTYPE)                                                                                                                      \
	{                                                                                                                                                          \
		DATATYPE value;                                                                                                                                        \
		for (int n = startColumn; n < m_actualCols; ++n) {                                                                                                     \
			for (size_t l = 0; l < std::min(readLines, mNumberLines); l++) {                                                                                   \
				const size_t lineNumber = l * lineBytes;                                                                                                       \
				const size_t index = lineNumber + (n - startColumn) * typeSize;                                                                                \
				if (byteOrder == QDataStream::BigEndian)                                                                                                       \
					value = qFromBigEndian<DATATYPE>(&binary[index]);                                                                                          \
				else                                                                                                                                           \
					value = qFromLittleEndian<DATATYPE>(&binary[index]);                                                                                       \
				/*DEBUG("column = " << n << ", index = " << index << ", value = " << value)*/                                                                  \
				(*static_cast<QVector<TARGETTYPE>*>(dataContainer[n]))[i * mNumberLines + l] = value;                                                          \
			}                                                                                                                                                  \
		}                                                                                                                                                      \
	}

/*!
\class BinaryFilter
\brief Manages the import/export of data organized as columns (vectors) from/to a binary file.

\ingroup datasources
*/
BinaryFilter::BinaryFilter()
	: AbstractFileFilter(FileType::Binary)
	, d(new BinaryFilterPrivate(this)) {
}

BinaryFilter::~BinaryFilter() = default;

/*!
  reads the content of the file \c fileName.
*/
void BinaryFilter::readDataFromFile(const QString& fileName, AbstractDataSource* dataSource, ImportMode importMode) {
	d->readDataFromFile(fileName, dataSource, importMode);
}

/*!
  reads the content of the device \c device.
*/
void BinaryFilter::readDataFromDevice(QIODevice& device, AbstractDataSource* dataSource, ImportMode importMode, int lines) {
	d->readDataFromDevice(device, dataSource, importMode, lines);
}

QVector<QStringList> BinaryFilter::preview(const QString& fileName, int lines) {
	return d->preview(fileName, lines);
}

/*!
writes the content of the data source \c dataSource to the file \c fileName.
*/
void BinaryFilter::write(const QString& fileName, AbstractDataSource* dataSource) {
	d->write(fileName, dataSource);
	// 	emit()
}

/*!
returns the list of all predefined data formats.
*/
QStringList BinaryFilter::dataTypes() {
	return (QStringList() << QStringLiteral("int8 (8 bit signed integer)") << QStringLiteral("int16 (16 bit signed integer)")
						  << QStringLiteral("int32 (32 bit signed integer)") << QStringLiteral("int64 (64 bit signed integer)")
						  << QStringLiteral("uint8 (8 bit unsigned integer)") << QStringLiteral("uint16 (16 bit unsigned integer)")
						  << QStringLiteral("uint32 (32 bit unsigned integer)") << QStringLiteral("uint64 (64 bit unsigned integer)")
						  << QStringLiteral("real32 (single precision floats)") << QStringLiteral("real64 (double precision floats)"));
}

/*!
returns the size of the predefined data types
*/
int BinaryFilter::dataSize(BinaryFilter::DataType type) {
	std::array<int, 10> sizes = {1, 2, 4, 8, 1, 2, 4, 8, 4, 8};

	return sizes[(int)type];
}

/*!
  returns the number of rows (length of vectors) in the file \c fileName.
*/
size_t BinaryFilter::rowNumber(const QString& fileName, const size_t vectors, const BinaryFilter::DataType type, const size_t maxRows) {
	KCompressionDevice device(fileName);
	if (!device.open(QIODevice::ReadOnly))
		return 0;

	// size() and bytesAvailable() return 0 and data may be compressed. Need to read the file once
	size_t rows = 0;
	while (!device.atEnd()) {
		if (rows >= maxRows) // stop when maxRows available
			return rows;
		// one row
		device.read(BinaryFilter::dataSize(type) * vectors);
		rows++;
	}

	return rows;
}

///////////////////////////////////////////////////////////////////////

void BinaryFilter::setVectors(const size_t v) {
	d->vectors = v;
}

size_t BinaryFilter::vectors() const {
	return d->vectors;
}

void BinaryFilter::setDataType(const BinaryFilter::DataType t) {
	d->dataType = t;
}

BinaryFilter::DataType BinaryFilter::dataType() const {
	return d->dataType;
}

void BinaryFilter::setByteOrder(const QDataStream::ByteOrder b) {
	d->byteOrder = b;
}

QDataStream::ByteOrder BinaryFilter::byteOrder() const {
	return d->byteOrder;
}

void BinaryFilter::setSkipStartBytes(const size_t s) {
	d->skipStartBytes = s;
}

size_t BinaryFilter::skipStartBytes() const {
	return d->skipStartBytes;
}

void BinaryFilter::setStartRow(const int s) {
	d->startRow = s;
}

int BinaryFilter::startRow() const {
	return d->startRow;
}

void BinaryFilter::setEndRow(const int e) {
	d->endRow = e;
}

int BinaryFilter::endRow() const {
	return d->endRow;
}

void BinaryFilter::setSkipBytes(const size_t s) {
	d->skipBytes = s;
}

size_t BinaryFilter::skipBytes() const {
	return d->skipBytes;
}

void BinaryFilter::setCreateIndexEnabled(bool b) {
	d->createIndexEnabled = b;
}

void BinaryFilter::setAutoModeEnabled(bool b) {
	d->autoModeEnabled = b;
}

bool BinaryFilter::isAutoModeEnabled() const {
	return d->autoModeEnabled;
}

QString BinaryFilter::fileInfoString(const QString& /*fileName*/) {
	QString info;

	// TODO
	return info;
}

// #####################################################################
// ################### Private implementation ##########################
// #####################################################################

BinaryFilterPrivate::BinaryFilterPrivate(BinaryFilter* owner)
	: q(owner) {
}

/*!
	reads the content of the device \c device to the data source \c dataSource or return as string for preview.
	Uses the settings defined in the data source.
*/
void BinaryFilterPrivate::readDataFromFile(const QString& fileName, AbstractDataSource* dataSource, AbstractFileFilter::ImportMode importMode) {
	DEBUG(Q_FUNC_INFO);

	KCompressionDevice device(fileName);
	numRows = BinaryFilter::rowNumber(fileName, vectors, dataType);

	if (!device.open(QIODevice::ReadOnly)) {
		DEBUG("	could not open file " << STDSTRING(fileName));
		return;
	}
	readDataFromDevice(device, dataSource, importMode);
}

/*!
 * returns 1 if the current read position in the device is at the end and 0 otherwise.
 */
int BinaryFilterPrivate::prepareStreamToRead(QDataStream& in) {
	DEBUG(Q_FUNC_INFO);

	in.setByteOrder(byteOrder);

	// catch case that skipStartBytes or startRow is bigger than file
	if (skipStartBytes >= BinaryFilter::dataSize(dataType) * vectors * numRows || startRow > (int)numRows)
		return 1;

	// skip bytes at start
	for (size_t i = 0; i < skipStartBytes; ++i) {
		qint8 tmp;
		in >> tmp;
	}

	// skip until start row
	for (size_t i = 0; i < (startRow - 1) * vectors; ++i) {
		for (int j = 0; j < BinaryFilter::dataSize(dataType); ++j) {
			qint8 tmp;
			in >> tmp;
		}
	}

	// set range of rows
	if (endRow == -1)
		m_actualRows = (int)numRows - startRow + 1;
	else if (endRow > (int)numRows - startRow + 1)
		m_actualRows = (int)numRows;
	else
		m_actualRows = endRow - startRow + 1;
	m_actualCols = (int)vectors;

	DEBUG("numRows = " << numRows);
	DEBUG("endRow = " << endRow);
	DEBUG("actual rows = " << m_actualRows);
	DEBUG("actual cols = " << m_actualCols);

	return 0;
}

/*!
	reads \c lines lines of the device \c device and return as string for preview.
*/
QVector<QStringList> BinaryFilterPrivate::preview(const QString& fileName, int lines) {
	DEBUG(Q_FUNC_INFO << ", fileName = " << STDSTRING(fileName) << ", lines = " << lines);

	KCompressionDevice device(fileName);
	if (!device.open(QIODevice::ReadOnly)) {
		q->setLastError(i18n("Failed to open the device/file."));
		return {};
	}

	numRows = BinaryFilter::rowNumber(fileName, vectors, dataType, lines);

	QDataStream in(&device);
	const int deviceError = prepareStreamToRead(in);

	if (deviceError) {
		q->setLastError(i18n("Data selection empty."));
		return {};
	}

	// all columns as double is ok for preview
	columnModes.resize(m_actualCols);

	// TODO: use given names
	QStringList vectorNames;
	if (createIndexEnabled)
		vectorNames.prepend(i18n("Index"));

	if (lines == -1)
		lines = m_actualRows;

	// read data
	lines = std::min(lines, m_actualRows);
	DEBUG(Q_FUNC_INFO << ", generating preview for " << lines << " lines")
	int progressIndex = 0;
	const qreal progressInterval = 0.01 * lines; // update on every 1% only

	QVector<QStringList> dataStrings;
	for (int i = 0; i < lines; ++i) {
		QStringList lineString;

		// prepend the index if required
		if (createIndexEnabled)
			lineString << QString::number(i + 1);

		for (int n = 0; n < m_actualCols; ++n) {
			switch (dataType) {
			case BinaryFilter::DataType::INT8: {
				qint8 value;
				in >> value;
				lineString << QString::number(value);
				break;
			}
			case BinaryFilter::DataType::INT16: {
				qint16 value;
				in >> value;
				lineString << QString::number(value);
				break;
			}
			case BinaryFilter::DataType::INT32: {
				qint32 value;
				in >> value;
				lineString << QString::number(value);
				break;
			}
			case BinaryFilter::DataType::INT64: {
				qint64 value;
				in >> value;
				lineString << QString::number(value);
				break;
			}
			case BinaryFilter::DataType::UINT8: {
				quint8 value;
				in >> value;
				lineString << QString::number(value);
				break;
			}
			case BinaryFilter::DataType::UINT16: {
				quint16 value;
				in >> value;
				lineString << QString::number(value);
				break;
			}
			case BinaryFilter::DataType::UINT32: {
				quint32 value;
				in >> value;
				lineString << QString::number(value);
				break;
			}
			case BinaryFilter::DataType::UINT64: {
				quint64 value;
				in >> value;
				lineString << QString::number(value);
				break;
			}
			case BinaryFilter::DataType::REAL32: {
				float value;
				in >> value;
				lineString << QString::number(value);
				break;
			}
			case BinaryFilter::DataType::REAL64: {
				double value;
				in >> value;
				lineString << QString::number(value);
				break;
			}
			}
		}
		dataStrings << lineString;

		// ask to update the progress bar only if we have more than 1000 lines
		// only in 1% steps
		progressIndex++;
		if (lines > 1000 && progressIndex > progressInterval) {
			double value = 100. * i / lines;
			Q_EMIT q->completed(static_cast<int>(value));
			progressIndex = 0;
			QApplication::processEvents(QEventLoop::AllEvents, 0);
		}
	}

	return dataStrings;
}

/*!
reads the content of the file \c fileName to the data source \c dataSource or return as string for preview.
Uses the settings defined in the data source.
*/
void BinaryFilterPrivate::readDataFromDevice(QIODevice& device, AbstractDataSource* dataSource, AbstractFileFilter::ImportMode importMode, int lines) {
	DEBUG(Q_FUNC_INFO);

	QDataStream in(&device);
	const int deviceError = prepareStreamToRead(in);
	if (deviceError) {
		dataSource->clear();
		DEBUG(Q_FUNC_INFO << ", Device error. Gving up");
		q->setLastError(i18n("Failed to open the device/file or it's empty."));
		return;
	}

	if (createIndexEnabled)
		m_actualCols++;

	// DEBUG("actual cols = " << m_actualCols)
	columnModes.resize(m_actualCols);
	switch (dataType) {
	case BinaryFilter::DataType::INT8:
	case BinaryFilter::DataType::INT16:
	case BinaryFilter::DataType::INT32:
	case BinaryFilter::DataType::UINT8:
	case BinaryFilter::DataType::UINT16:
		columnModes.fill(AbstractColumn::ColumnMode::Integer);
		break;
	case BinaryFilter::DataType::UINT32:
	case BinaryFilter::DataType::INT64:
		columnModes.fill(AbstractColumn::ColumnMode::BigInt);
		break;
	case BinaryFilter::DataType::UINT64:
	case BinaryFilter::DataType::REAL32:
	case BinaryFilter::DataType::REAL64:
		columnModes.fill(AbstractColumn::ColumnMode::Double);
		break;
	}

	// TODO: use given names
	QStringList vectorNames;

	if (createIndexEnabled) {
		vectorNames.prepend(i18n("Index"));
		columnModes[0] = AbstractColumn::ColumnMode::Integer;
	}

	std::vector<void*> dataContainer;
	bool ok = false;
	int columnOffset = dataSource->prepareImport(dataContainer, importMode, m_actualRows, m_actualCols, vectorNames, columnModes, ok);
	if (!ok) {
		q->setLastError(i18n("Not enough memory."));
		return;
	}

	if (lines == -1)
		lines = m_actualRows;

	int startColumn = 0;
	if (createIndexEnabled)
		startColumn++;

	// read data
	lines = std::min(lines, m_actualRows);
	DEBUG(Q_FUNC_INFO << ", Reading " << lines << " lines");
	int progressIndex = 0;
	const qreal progressInterval = 0.01 * lines; // update on every 1% only

	// prepend the index if required
	if (createIndexEnabled)
		for (int i = 0; i < lines; ++i)
			static_cast<QVector<int>*>(dataContainer[0])->operator[](i) = i + 1;

	// chunk to read at once
	const size_t mNumberLines = 100000; // see SpiceReader::mNumberLines
	const int typeSize = BinaryFilter::dataSize(dataType);
	const int lineBytes = m_actualCols * typeSize;

	// DEBUG("lines/mNumberLines = " << lines << "/" << mNumberLines << " -> " << lines/mNumberLines + 1)
	for (size_t i = 0; i <= lines / mNumberLines; ++i) {
		// DEBUG("reading chunk " << i + 1);
		const QByteArray ba = device.read(mNumberLines * lineBytes);
		const char* binary = ba.data();
		const size_t readLines = (int)(ba.length() / lineBytes);
		// DEBUG("Read lines " << readLines)
		switch (dataType) {
		case BinaryFilter::DataType::INT8:
			IMPORT_DATA(qint8, int)
			break;
		case BinaryFilter::DataType::INT16:
			IMPORT_DATA(qint16, int)
			break;
		case BinaryFilter::DataType::INT32:
			IMPORT_DATA(qint32, int)
			break;
		case BinaryFilter::DataType::INT64:
			IMPORT_DATA(qint64, qint64)
			break;
		case BinaryFilter::DataType::UINT8:
			IMPORT_DATA(quint8, int)
			break;
		case BinaryFilter::DataType::UINT16:
			IMPORT_DATA(quint16, int)
			break;
		case BinaryFilter::DataType::UINT32:
			IMPORT_DATA(quint32, qint64)
			break;
		case BinaryFilter::DataType::UINT64:
			IMPORT_DATA(quint64, double)
			break;
		case BinaryFilter::DataType::REAL32:
			IMPORT_DATA(float, double)
			break;
		case BinaryFilter::DataType::REAL64:
			IMPORT_DATA(double, double)
			break;
		}
		// ask to update the progress bar only if we have more than 1000 lines
		// only in 1% steps
		progressIndex++;
		if (lines > 1000 && progressIndex > progressInterval) {
			double value = 100. * i / lines * mNumberLines;
			Q_EMIT q->completed(static_cast<int>(value));
			progressIndex = 0;
			QApplication::processEvents(QEventLoop::AllEvents, 0);
		}
	}

	dataSource->finalizeImport(columnOffset, 1, m_actualCols, QString(), importMode);
}

/*!
	writes the content of \c dataSource to the file \c fileName.
*/
void BinaryFilterPrivate::write(const QString& /*fileName*/, AbstractDataSource* /*dataSource*/) {
	// TODO: writing binary files not supported yet
}

// ##############################################################################
// ##################  Serialization/Deserialization  ###########################
// ##############################################################################

/*!
  Saves as XML.
 */
void BinaryFilter::save(QXmlStreamWriter* writer) const {
	writer->writeStartElement(QStringLiteral("binaryFilter"));
	writer->writeAttribute(QStringLiteral("vectors"), QString::number(d->vectors));
	writer->writeAttribute(QStringLiteral("dataType"), QString::number(static_cast<int>(d->dataType)));
	writer->writeAttribute(QStringLiteral("byteOrder"), QString::number(d->byteOrder));
	writer->writeAttribute(QStringLiteral("autoMode"), QString::number(d->autoModeEnabled));
	writer->writeAttribute(QStringLiteral("startRow"), QString::number(d->startRow));
	writer->writeAttribute(QStringLiteral("endRow"), QString::number(d->endRow));
	writer->writeAttribute(QStringLiteral("skipStartBytes"), QString::number(d->skipStartBytes));
	writer->writeAttribute(QStringLiteral("skipBytes"), QString::number(d->skipBytes));
	writer->writeAttribute(QStringLiteral("createIndex"), QString::number(d->createIndexEnabled));
	writer->writeEndElement();
}

/*!
  Loads from XML.
*/
bool BinaryFilter::load(XmlStreamReader* reader) {
	QXmlStreamAttributes attribs = reader->attributes();

	// read attributes
	QString str = attribs.value(QStringLiteral("vectors")).toString();
	if (str.isEmpty())
		reader->raiseMissingAttributeWarning(QStringLiteral("vectors"));
	else
		d->vectors = (size_t)str.toULong();

	str = attribs.value(QStringLiteral("dataType")).toString();
	if (str.isEmpty())
		reader->raiseMissingAttributeWarning(QStringLiteral("dataType"));
	else
		d->dataType = (BinaryFilter::DataType)str.toInt();

	str = attribs.value(QStringLiteral("byteOrder")).toString();
	if (str.isEmpty())
		reader->raiseMissingAttributeWarning(QStringLiteral("byteOrder"));
	else
		d->byteOrder = (QDataStream::ByteOrder)str.toInt();

	str = attribs.value(QStringLiteral("autoMode")).toString();
	if (str.isEmpty())
		reader->raiseMissingAttributeWarning(QStringLiteral("autoMode"));
	else
		d->autoModeEnabled = str.toInt();

	str = attribs.value(QStringLiteral("startRow")).toString();
	if (str.isEmpty())
		reader->raiseMissingAttributeWarning(QStringLiteral("startRow"));
	else
		d->startRow = str.toInt();

	str = attribs.value(QStringLiteral("endRow")).toString();
	if (str.isEmpty())
		reader->raiseMissingAttributeWarning(QStringLiteral("endRow"));
	else
		d->endRow = str.toInt();

	str = attribs.value(QStringLiteral("skipStartBytes")).toString();
	if (str.isEmpty())
		reader->raiseMissingAttributeWarning(QStringLiteral("skipStartBytes"));
	else
		d->skipStartBytes = (size_t)str.toULong();

	str = attribs.value(QStringLiteral("skipBytes")).toString();
	if (str.isEmpty())
		reader->raiseMissingAttributeWarning(QStringLiteral("skipBytes"));
	else
		d->skipBytes = (size_t)str.toULong();

	str = attribs.value(QStringLiteral("createIndex")).toString();
	if (str.isEmpty())
		reader->raiseMissingAttributeWarning(QStringLiteral("createIndex"));
	else
		d->createIndexEnabled = str.toInt();

	return true;
}
