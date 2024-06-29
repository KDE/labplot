/*
	File                 : VectorBLFFilter.cpp
	Project              : LabPlot
	Description          : Vector BLF I/O-filter
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2023 Martin Marmsoler <martin.marmsoler@gmail.com>
	SPDX-License-Identifier: GPL-3.0-or-later
*/

/* TODO:
 * Feature: implement missing data types and ranks
 * Performance: only fill dataPointer or dataStrings (not both)
 */

#include "backend/datasources/filters/VectorBLFFilter.h"
#include "backend/datasources/filters/VectorBLFFilterPrivate.h"
#include "backend/lib/XmlStreamReader.h"
#include "backend/lib/macros.h"
#include "backend/lib/trace.h"

#include <KLocalizedString>
#include <QDateTime>

#ifdef HAVE_VECTOR_BLF
#include <Vector/BLF/Exceptions.h>
#include <Vector/BLF/File.h>
#endif

//////////////////////////////////////////////////////////////////////

/*!
	\class VectorBLFFilter
	\brief Manages the import/export of data from/to a Vector BLF file.

	\ingroup datasources
*/
VectorBLFFilter::VectorBLFFilter()
	: CANFilter(FileType::VECTOR_BLF, new VectorBLFFilterPrivate(this)) {
}

VectorBLFFilter::~VectorBLFFilter() = default;

QString VectorBLFFilter::fileInfoString(const QString& fileName) {
	QString info;
#ifdef HAVE_VECTOR_BLF
	Vector::BLF::File f;
	f.open(fileName.toLocal8Bit().data());
	if (!f.is_open())
		return info;

	const auto& statistics = f.fileStatistics;

	// application info
	QString appName;
	switch (statistics.applicationId) {
	case Vector::BLF::Unknown:
		appName = i18n("Unknown");
		break;
	case Vector::BLF::Canalyzer:
		appName = QStringLiteral("CANalyzer");
		break;
	case Vector::BLF::Canoe:
		appName = QStringLiteral("CANoe");
		break;
	case Vector::BLF::Canstress:
		appName = QStringLiteral("CANstress");
		break;
	case Vector::BLF::Canlog:
		appName = QStringLiteral("CANlog");
		break;
	case Vector::BLF::Canape:
		appName = QStringLiteral("CANape");
		break;
	case Vector::BLF::Cancasexllog:
		appName = QStringLiteral("CANcaseXL log");
		break;
	case Vector::BLF::Vlconfig:
		appName = QStringLiteral("Vector Logger Configurator");
		break;
	case Vector::BLF::Porschelogger:
		appName = QStringLiteral("Porsche Logger");
		break;
	case Vector::BLF::Caeteclogger:
		appName = QStringLiteral("CAETEC Logger");
		break;
	case Vector::BLF::Vectornetworksimulator:
		appName = QStringLiteral("Vector Network Simulator");
		break;
	case Vector::BLF::Ipetroniklogger:
		appName = QStringLiteral("IPETRONIK Logger");
		break;
	case Vector::BLF::RtPk:
		appName = QStringLiteral("RT PK");
		break;
	case Vector::BLF::Piketec:
		appName = QStringLiteral("PikeTec");
		break;
	case Vector::BLF::Sparks:
		appName = QStringLiteral("Sparks");
		break;
	}

	info += i18n("Application: %1", appName);
	info += QStringLiteral("<br>");
	info += i18n("Application version: %1.%2.%3", statistics.applicationMajor, statistics.applicationMinor, statistics.applicationBuild);
	info += QStringLiteral("<br>");
	info += i18n("Number of Objects: %1", statistics.objectCount);
	info += QStringLiteral("<br>");

	// measurement start time
	auto start = statistics.measurementStartTime;
	QDate startDate(start.year, start.month, start.day);
	QTime startTime(start.hour, start.minute, start.second, start.milliseconds);
	QDateTime startDateTime(startDate, startTime);
	info += i18n("Start Time: %1", startDateTime.toString(QLatin1String("yyyy-MM-dd hh:mm:ss.zzz")));
	info += QStringLiteral("<br>");

	// measurement end time
	auto end = statistics.lastObjectTime;
	QDate endDate(end.year, end.month, end.day);
	QTime endTime(end.hour, end.minute, end.second, end.milliseconds);
	QDateTime endDateTime(endDate, endTime);
	info += i18n("End Time: %1", endDateTime.toString(QLatin1String("yyyy-MM-dd hh:mm:ss.zzz")));
	info += QStringLiteral("<br>");

	// compression
	info += i18n("Compression Level: %1", statistics.compressionLevel);
	info += QStringLiteral("<br>");
	info += i18n("Uncompressed File Size: %1 Bytes", statistics.uncompressedFileSize);
	info += QStringLiteral("<br>");

	f.close();
#else
	Q_UNUSED(fileName)
#endif
	return info;
}

bool VectorBLFFilter::isValid(const QString& filename) {
#ifdef HAVE_VECTOR_BLF
	try {
		Vector::BLF::File f;
		f.open(filename.toLocal8Bit().data());
		if (!f.is_open())
			return false; // No file
		f.close();
		return true;
	} catch (const Vector::BLF::Exception& e) {
		return false; // Signature was invalid or something else
	}
#else
	Q_UNUSED(filename)
#endif
	return false;
}

// #####################################################################
// ################### Private implementation ##########################
// #####################################################################

VectorBLFFilterPrivate::VectorBLFFilterPrivate(VectorBLFFilter* owner)
	: CANFilterPrivate(owner)
#ifdef HAVE_VECTOR_BLF
	, q(owner)
#endif
{
}

bool VectorBLFFilterPrivate::isValid(const QString& filename) const {
	return VectorBLFFilter::isValid(filename);
}

/*!
 * depending on the parse status in \c Warning, adds either a warning or set the last error in the filter class
 * to be shown in the import dialog.
 */
void VectorBLFFilterPrivate::addWarningError(const Warning& warning) const {
	switch (warning.status) {
	case ParseStatus::DBCBigEndian:
		q->addWarning(i18n("Big Endian not supported. CAN id: %1.", QStringLiteral("0x%1").arg(warning.CANId, 0, 16)));
		break;
	case ParseStatus::DBCMessageToLong:
		q->addWarning(i18n("Message too long. CAN id: %1.", QStringLiteral("0x%1").arg(warning.CANId, 0, 16)));
		break;
	case ParseStatus::DBCUnknownID:
		q->addWarning(i18n("Unknown id: %1.", QStringLiteral("0x%1").arg(warning.CANId, 0, 16)));
		break;
	case ParseStatus::ErrorInvalidFile:
		q->setLastError(i18n("Invalid BLF file"));
		break;
	case ParseStatus::DBCInvalidConversion:
		q->setLastError(i18n("Unable to calculate conversion: %1.", QStringLiteral("0x%1").arg(warning.CANId, 0, 16)));
		break;
	case ParseStatus::DBCParserUnsupported:
		q->setLastError(i18n("No DBC parser installed."));
		break;
	case ParseStatus::DBCInvalidFile:
		q->setLastError(i18n("Invalid DBC file."));
		break;
	case ParseStatus::ErrorUnknown:
		q->setLastError(i18n("Unknown error,"));
		break;
	case ParseStatus::Success:
		break;
	}
}

VectorBLFFilterPrivate::ParseStatus VectorBLFFilterPrivate::DBCParserParseStatusToVectorBLFStatus(DbcParser::ParseStatus s) {
	switch (s) {
	case DbcParser::ParseStatus::ErrorBigEndian:
		return ParseStatus::DBCBigEndian;
	case DbcParser::ParseStatus::ErrorMessageToLong:
		return ParseStatus::DBCMessageToLong;
	case DbcParser::ParseStatus::ErrorUnknownID:
		return ParseStatus::DBCUnknownID;
	case DbcParser::ParseStatus::ErrorInvalidFile:
		return ParseStatus::DBCInvalidFile;
	case DbcParser::ParseStatus::ErrorDBCParserUnsupported:
		return ParseStatus::DBCParserUnsupported;
	case DbcParser::ParseStatus::ErrorInvalidConversion:
		return ParseStatus::DBCInvalidConversion;
	case DbcParser::ParseStatus::Success:
		return ParseStatus::Success;
	}
	return ParseStatus::ErrorUnknown;
}

#ifdef HAVE_VECTOR_BLF
bool getTime(const Vector::BLF::ObjectHeaderBase* ohb, uint64_t& timestamp) {
	/* ObjectHeader */
	auto* oh = dynamic_cast<const Vector::BLF::ObjectHeader*>(ohb);
	if (oh != nullptr) {
		timestamp = oh->objectTimeStamp;
		switch (oh->objectFlags) {
		case Vector::BLF::ObjectHeader::ObjectFlags::TimeTenMics:
			return false;
		case Vector::BLF::ObjectHeader::ObjectFlags::TimeOneNans:
			return true;
		}
	}

	/* ObjectHeader2 */
	auto* oh2 = dynamic_cast<const Vector::BLF::ObjectHeader2*>(ohb);
	if (oh2 != nullptr) {
		timestamp = oh2->objectTimeStamp;
		switch (oh2->objectFlags) {
		case Vector::BLF::ObjectHeader2::ObjectFlags::TimeTenMics:
			return false;
		case Vector::BLF::ObjectHeader2::ObjectFlags::TimeOneNans:
			return true;
		}
	}
	return true;
}
#endif

int VectorBLFFilterPrivate::readDataFromFileCommonTime(const QString& fileName, int lines) {
	PERFTRACE(QLatin1String(Q_FUNC_INFO));

	if (!isValid(fileName)) {
		q->setLastError(i18n("Invalid file."));
		return 0;
	}

	const auto validStatus = m_dbcParser.isValid();
	if (validStatus != DbcParser::ParseStatus::Success) {
		addWarningError({DBCParserParseStatusToVectorBLFStatus(validStatus), 0});
		return 0;
	}

	if (m_parseState.ready && m_parseState.requestedLines == lines)
		return m_parseState.readLines;

	m_DataContainer.clear();

#ifdef HAVE_VECTOR_BLF
	Vector::BLF::File file;
	file.open(fileName.toLocal8Bit().data());

	// 1. Reading in messages
	QVector<const Vector::BLF::ObjectHeaderBase*> objectHeaders;
	QVector<uint32_t> ids;
	int message_counter = 0;
	{
		PERFTRACE(QLatin1String(Q_FUNC_INFO) + QLatin1String("Parsing BLF file"));
		Vector::BLF::ObjectHeaderBase* objectHeader = nullptr;
		while (file.good() && ((lines >= 0 && message_counter < lines) || lines < 0)) {
			try {
				objectHeader = file.read();
			} catch (std::runtime_error& e) {
				DEBUG("Exception: " << e.what() << std::endl);
			}
			if (objectHeader == nullptr)
				break;

			if (objectHeader->objectType != Vector::BLF::ObjectType::CAN_MESSAGE2)
				continue;

			int id;
			if (objectHeader->objectType == Vector::BLF::ObjectType::CAN_MESSAGE2) {
				const auto message = reinterpret_cast<Vector::BLF::CanMessage2*>(objectHeader);
				id = message->id;
			} else
				return 0;

			objectHeaders.append(objectHeader);
			if (!ids.contains(id))
				ids.append(id);
			message_counter++;
		}
	}

	// 2. Create vector names
	QHash<uint32_t, int> idIndexTable;
	m_dbcParser.getSignals(ids, DbcParser::PrefixType::None, DbcParser::SuffixType::Unit, idIndexTable, m_signals);

	// 3. allocate memory
	try {
		if (convertTimeToSeconds) {
			auto* vector = new QVector<double>();
			vector->resize(message_counter);
			m_DataContainer.appendVector<double>(vector, AbstractColumn::ColumnMode::Double);
		} else {
			auto* vector = new QVector<qint64>();
			vector->resize(message_counter);
			m_DataContainer.appendVector<qint64>(vector, AbstractColumn::ColumnMode::BigInt); // BigInt is qint64 and not quint64!
		}
		for (int i = 0; i < m_signals.signal_names.length(); i++) {
			auto* vector = new QVector<double>();
			vector->resize(message_counter);
			m_DataContainer.appendVector(vector, AbstractColumn::ColumnMode::Double);
		}
	} catch (std::bad_alloc&) {
		q->setLastError(i18n("Not enough memory."));
		return 0;
	}

	// 4. fill datacontainer
	int message_index = 0;
	bool timeInNS = true;
	if (timeHandlingMode == CANFilter::TimeHandling::ConcatNAN) {
		for (const auto objectHeader : objectHeaders) {
			uint32_t id;
			std::vector<double> values;
			DbcParser::ParseStatus status;
			if (objectHeader->objectType == Vector::BLF::ObjectType::CAN_MESSAGE2) {
				const auto message = reinterpret_cast<const Vector::BLF::CanMessage2*>(objectHeader);
				id = message->id;
				status = m_dbcParser.parseMessage(message->id, message->data, values);
			} else
				return 0;

			if (status != DbcParser::ParseStatus::Success) {
				// id is not available in the dbc file, so it is not possible to decode
				DEBUG("Unable to decode message: " << id << ": " << (int)status);
				addWarningError({DBCParserParseStatusToVectorBLFStatus(status), id});
				continue;
			}

			uint64_t timestamp;
			timeInNS = getTime(objectHeader, timestamp);
			if (convertTimeToSeconds) {
				double timestamp_seconds;
				if (timeInNS)
					timestamp_seconds = (double)timestamp / pow(10, 9); // TimeOneNans
				else
					timestamp_seconds = (double)timestamp / pow(10, 5); // TimeTenMics

				m_DataContainer.setData<double>(0, message_index, timestamp_seconds);
			} else
				m_DataContainer.setData<qint64>(0, message_index, timestamp);

			const size_t startIndex = idIndexTable.value(id) + 1; // +1 because of time
			for (size_t i = 1; i < startIndex; i++)
				m_DataContainer.setData<double>(i, message_index, std::nan("0"));

			for (size_t i = startIndex; i < startIndex + values.size(); i++)
				m_DataContainer.setData<double>(i, message_index, values.at(i - startIndex));

			for (size_t i = startIndex + values.size(); i < m_DataContainer.size(); i++)
				m_DataContainer.setData<double>(i, message_index, std::nan("0"));

			message_index++;
		}
	} else {
		bool firstMessageValid = false;
		for (const auto objectHeader : objectHeaders) {
			uint32_t id;
			std::vector<double> values;
			DbcParser::ParseStatus status;
			if (objectHeader->objectType == Vector::BLF::ObjectType::CAN_MESSAGE2) {
				const auto message = reinterpret_cast<const Vector::BLF::CanMessage2*>(objectHeader);
				id = message->id;
				status = m_dbcParser.parseMessage(message->id, message->data, values);
			} else
				return 0;

			if (status != DbcParser::ParseStatus::Success) {
				// id is not available in the dbc file, so it is not possible to decode
				addWarningError({DBCParserParseStatusToVectorBLFStatus(status), id});
				continue;
			}

			uint64_t timestamp;
			timeInNS = getTime(objectHeader, timestamp);
			if (convertTimeToSeconds) {
				double timestamp_seconds;
				if (timeInNS)
					timestamp_seconds = (double)timestamp / pow(10, 9); // TimeOneNans
				else
					timestamp_seconds = (double)timestamp / pow(10, 5); // TimeTenMics

				m_DataContainer.setData<double>(0, message_index, timestamp_seconds);
			} else
				m_DataContainer.setData<qint64>(0, message_index, timestamp);

			const std::vector<double>::size_type startIndex = idIndexTable.value(id) + 1; // +1 because of time
			if (firstMessageValid) {
				for (std::vector<double>::size_type i = 1; i < startIndex; i++) {
					const auto prevValue = m_DataContainer.data<double>(i, message_index - 1);
					m_DataContainer.setData<double>(i, message_index, prevValue);
				}
				for (std::vector<double>::size_type i = startIndex; i < startIndex + values.size(); i++) {
					m_DataContainer.setData<double>(i, message_index, values.at(i - startIndex));
				}
				for (std::vector<double>::size_type i = startIndex + values.size(); i < m_DataContainer.size(); i++) {
					const auto prevValue = m_DataContainer.data<double>(i, message_index - 1);
					m_DataContainer.setData<double>(i, message_index, prevValue);
				}
			} else {
				for (std::vector<double>::size_type i = 1; i < startIndex; i++)
					m_DataContainer.setData<double>(i, message_index, std::nan("0"));
				for (std::vector<double>::size_type i = startIndex; i < startIndex + values.size(); i++)
					m_DataContainer.setData<double>(i, message_index, values.at(i - startIndex));
				for (std::vector<double>::size_type i = startIndex + values.size(); i < m_DataContainer.size(); i++)
					m_DataContainer.setData<double>(i, message_index, std::nan("0"));
				firstMessageValid = true;
			}
			message_index++;
		}
	}

	// 5. add Time column to vector Names
	if (convertTimeToSeconds)
		m_signals.signal_names.prepend(i18n("Time_s")); // Must be done after allocating memory
	else if (timeInNS)
		m_signals.signal_names.prepend(i18n("Time_ns")); // Must be done after allocating memory
	else
		m_signals.signal_names.prepend(i18n("Time_10µs")); // Must be done after allocating memory
	m_signals.value_descriptions.insert(m_signals.value_descriptions.begin(), std::vector<DbcParser::ValueDescriptions>()); // Time does not have any labels

	for (const auto& message : objectHeaders)
		delete message;

	if (!m_DataContainer.resize(message_index))
		return 0;

	// Use message_counter here, because it will be used as reference for caching
	m_parseState = ParseState(message_counter, message_index);
	return message_index;
#else
	return 0;
#endif // HAVE_VECTOR_BLF
}

int VectorBLFFilterPrivate::readDataFromFileSeparateTime(const QString& /*fileName*/, int /*lines*/) {
	return 0; // Not implemented yet
}

// ##############################################################################
// ##################  Serialization/Deserialization  ###########################
// ##############################################################################

/*!
  Saves as XML.
 */
void VectorBLFFilter::save(QXmlStreamWriter* writer) const {
	writer->writeStartElement(QStringLiteral("VectorBLFFilter"));
	writer->writeEndElement();
}

/*!
  Loads from XML.
*/
bool VectorBLFFilter::load(XmlStreamReader*) {
	return true;
}
