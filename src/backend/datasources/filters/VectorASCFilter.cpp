#include "VectorASCFilter.h"
#include "VectorASCFilterPrivate.h"

#ifdef HAVE_VECTOR_ASC
#include <Vector/ASC.h>
#include <Vector/ASC/File.h>
#endif

#include "backend/lib/XmlStreamReader.h"
#include "backend/lib/macros.h"
#include "backend/lib/trace.h"

#include <klocalizedstring.h>

//////////////////////////////////////////////////////////////////////

/*!
	\class VectorASCFilter
	\brief Manages the import/export of data from/to a Vector ASC file.

	\ingroup datasources
*/
VectorASCFilter::VectorASCFilter()
	: CANFilter(FileType::VECTOR_ASC, new VectorASCFilterPrivate(this)) {
}

VectorASCFilter::~VectorASCFilter() {
}

QString VectorASCFilter::fileInfoString(const QString& fileName) {
	DEBUG(Q_FUNC_INFO);
	QString info;

	Q_UNUSED(fileName);
	return info;
}

bool VectorASCFilter::isValid(const QString& filename) {
#ifdef HAVE_VECTOR_ASC
	Vector::ASC::File f;
	f.open(filename.toLocal8Bit().data());
	if (!f.is_open())
		return false; // No file
	f.close();
	return true;
#else
	Q_UNUSED(filename)
#endif
}

//#####################################################################
//################### Private implementation ##########################
//#####################################################################

VectorASCFilterPrivate::VectorASCFilterPrivate(VectorASCFilter* owner)
	: CANFilterPrivate(owner)
	, q(owner) {
}

bool VectorASCFilterPrivate::isValid(const QString& filename) const {
	return VectorASCFilter::isValid(filename);
}

QStringList VectorASCFilterPrivate::lastErrors() const {
	QStringList r;
	for (const auto& e : errors) {
		switch (e.e) {
		case ParseStatus::DBCBigEndian:
			r.append(i18n("Big Endian not supported. CAN id: %1", QStringLiteral("0x%1").arg(e.CANId, 0, 16)));
			break;
		case ParseStatus::DBCMessageToLong:
			r.append(i18n("Message too long. CAN id: %1", QStringLiteral("0x%1").arg(e.CANId, 0, 16)));
			break;
		case ParseStatus::DBCUnknownID:
			r.append(i18n("Unknown id: %1", QStringLiteral("0x%1").arg(e.CANId, 0, 16)));
			break;
		case ParseStatus::ErrorInvalidFile:
			r.append(i18n("Invalid asc file"));
			break;
		case ParseStatus::DBCInvalidConversion:
			r.append(i18n("Unable to calculate conversion: %1", QStringLiteral("0x%1").arg(e.CANId, 0, 16)));
			break;
		case ParseStatus::DBCParserUnsupported:
			r.append(i18n("No dbc parser installed"));
			break;
		case ParseStatus::DBCInvalidFile:
			r.append(i18n("Invalid dbc file"));
			break;
		case ParseStatus::ErrorUnknown:
			r.append(i18n("Unknown error"));
			break;
		case ParseStatus::Success:
			break;
		}
	}
	return r;
}

VectorASCFilterPrivate::ParseStatus VectorASCFilterPrivate::DBCParserParseStatusToVectorASCStatus(DbcParser::ParseStatus s) {
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

int VectorASCFilterPrivate::readDataFromFileCommonTime(const QString& fileName, int lines) {
	PERFTRACE(QLatin1String(Q_FUNC_INFO));

	errors.clear();

	if (!isValid(fileName)) {
		errors.append({ParseStatus::ErrorInvalidFile, 0});
		return 0;
	}

	if (!m_dbcParser.isValid()) {
		errors.append({ParseStatus::DBCInvalidFile, 0});
		return 0;
	}

	if (m_parseState.ready && m_parseState.requestedLines == lines)
		return m_parseState.readLines;

	m_DataContainer.clear();

#ifdef HAVE_VECTOR_ASC
	Vector::ASC::File file;
	file.open(fileName.toLocal8Bit().data());

	// 1. Reading in messages
	QVector<const Vector::ASC::CanMessage*> v;
	Vector::ASC::Event* event = nullptr;
	QVector<uint32_t> ids;
	int message_counter = 0;
	{
		PERFTRACE(QLatin1String(Q_FUNC_INFO) + QLatin1String("Parsing ASC file"));
		while (!file.eof() && ((lines >= 0 && message_counter < lines) || lines < 0)) {
			event = file.read();
			if (event == nullptr)
				continue;

			if (event->eventType != Vector::ASC::Event::EventType::CanMessage)
				continue;

			const auto message = reinterpret_cast<Vector::ASC::CanMessage*>(event);
			const int id = message->id;

			v.append(message);
			if (!ids.contains(id))
				ids.append(id);
			message_counter++;
		}
	}

	// 2. Create vector names
	QHash<uint32_t, int> idIndexTable;
	vectorNames = m_dbcParser.signals(ids, idIndexTable);

	// 3. allocate memory
	if (convertTimeToSeconds) {
		auto* vector = new QVector<double>();
		vector->resize(message_counter);
		m_DataContainer.appendVector<double>(vector, AbstractColumn::ColumnMode::Double);
	} else {
		auto* vector = new QVector<qint64>();
		vector->resize(message_counter);
		m_DataContainer.appendVector<qint64>(vector, AbstractColumn::ColumnMode::BigInt); // BigInt is qint64 and not quint64!
	}
	for (int i = 0; i < vectorNames.length(); i++) {
		auto* vector = new QVector<double>();
		vector->resize(message_counter);
		m_DataContainer.appendVector(vector, AbstractColumn::ColumnMode::Double);
	}

	// 4. fill datacontainer
	int message_index = 0;
	if (timeHandlingMode == CANFilter::TimeHandling::ConcatNAN) {
		for (const auto event : v) {
			std::vector<double> values;
			DbcParser::ParseStatus status;
			const uint32_t id = event->id;
			status = m_dbcParser.parseMessage(event->id, event->data, values);

			if (status != DbcParser::ParseStatus::Success) {
				// id is not available in the dbc file, so it is not possible to decode
				DEBUG("Unable to decode message: " << id << ": " << (int)status);
				errors.append({DBCParserParseStatusToVectorASCStatus(status), id});
				continue;
			}

			double timestamp_seconds = event->time;
			m_DataContainer.setData<double>(0, message_index, timestamp_seconds);

			const size_t startIndex = idIndexTable.value(id) + 1; // +1 because of time
			for (size_t i = 1; i < startIndex; i++) {
				m_DataContainer.setData<double>(i, message_index, std::nan("0"));
			}
			for (size_t i = startIndex; i < startIndex + values.size(); i++) {
				m_DataContainer.setData<double>(i, message_index, values.at(i - startIndex));
			}
			for (size_t i = startIndex + values.size(); i < m_DataContainer.size(); i++) {
				m_DataContainer.setData<double>(i, message_index, std::nan("0"));
			}
			message_index++;
		}
	} else {
		bool firstMessageValid = false;
		for (const auto event : v) {
			std::vector<double> values;
			DbcParser::ParseStatus status;
			const uint32_t id = event->id;
			status = m_dbcParser.parseMessage(event->id, event->data, values);

			if (status != DbcParser::ParseStatus::Success) {
				// id is not available in the dbc file, so it is not possible to decode
				DEBUG("Unable to decode message: " << id << ": " << (int)status);
				errors.append({DBCParserParseStatusToVectorASCStatus(status), id});
				continue;
			}

			double timestamp_seconds = event->time;
			m_DataContainer.setData<double>(0, message_index, timestamp_seconds);

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
	vectorNames.prepend(i18n("Time_s")); // Must be done after allocating memory

	for (const auto& message : v)
		delete message;

	if (!m_DataContainer.resize(message_index))
		return 0;

	// Use message_counter here, because it will be used as reference for caching
	m_parseState = ParseState(message_counter, message_index);
	return message_index;
#else
	return 0;
#endif // HAVE_VECTOR_ASC
}

int VectorASCFilterPrivate::readDataFromFileSeparateTime(const QString& /*fileName*/, int /*lines*/) {
	return 0; // Not implemented yet
}

//##############################################################################
//##################  Serialization/Deserialization  ###########################
//##############################################################################

/*!
  Saves as XML.
 */
void VectorASCFilter::save(QXmlStreamWriter* writer) const {
	writer->writeStartElement(QStringLiteral("VectorASCFilter"));
	writer->writeEndElement();
}

/*!
  Loads from XML.
*/
bool VectorASCFilter::load(XmlStreamReader*) {
	// 	KLocalizedString attributeWarning = ki18n("Attribute '%1' missing or empty, default value is used");
	// 	QXmlStreamAttributes attribs = reader->attributes();
	return true;
}
