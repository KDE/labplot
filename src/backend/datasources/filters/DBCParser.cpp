#include "DBCParser.h"

#include <cmath>

#include <QHash>
#include <QStringList>

bool DbcParser::isValid() {
	return m_valid;
}
bool DbcParser::parseFile(const QString& filename) {
	m_valid = false;
#ifdef HAVE_DBC_PARSER
	try {
		m_parser.parse_file(filename.toStdString());
		m_parser.sortSignals();
		m_valid = true;
	} catch (libdbc::validity_error e) {
		// e.what(); // TODO: turn on
	}
#else
	Q_UNUSED(filename)
#endif
	return m_valid;
}

DbcParser::ParseStatus DbcParser::parseMessage(const uint32_t id, const std::vector<uint8_t>& data, std::vector<double>& out) {
	if (!m_valid)
		return ParseStatus::ErrorInvalidFile;
#ifdef HAVE_DBC_PARSER
	switch (m_parser.parseMessage(id, data, out)) {
	case libdbc::Message::ParseSignalsStatus::Success:
		return ParseStatus::Success;
	case libdbc::Message::ParseSignalsStatus::ErrorMessageToLong:
		return ParseStatus::ErrorMessageToLong;
	case libdbc::Message::ParseSignalsStatus::ErrorBigEndian:
		return ParseStatus::ErrorBigEndian;
	case libdbc::Message::ParseSignalsStatus::ErrorUnknownID:
		return ParseStatus::ErrorUnknownID;
	}
#else
	Q_UNUSED(id)
	Q_UNUSED(data)
	Q_UNUSED(out)
#endif
	return ParseStatus::ErrorDBCParserUnsupported;
}

DbcParser::ParseStatus DbcParser::parseMessage(const uint32_t id, const std::array<uint8_t, 8>& data, std::vector<double>& out) {
	if (!m_valid)
		return ParseStatus::ErrorInvalidFile;

#ifdef HAVE_DBC_PARSER
	switch (m_parser.parseMessage(id, data, out)) {
	case libdbc::Message::ParseSignalsStatus::Success:
		return ParseStatus::Success;
	case libdbc::Message::ParseSignalsStatus::ErrorMessageToLong:
		return ParseStatus::ErrorMessageToLong;
	case libdbc::Message::ParseSignalsStatus::ErrorBigEndian:
		return ParseStatus::ErrorBigEndian;
	case libdbc::Message::ParseSignalsStatus::ErrorUnknownID:
		return ParseStatus::ErrorUnknownID;
	}
#else
	Q_UNUSED(id)
	Q_UNUSED(data)
	Q_UNUSED(out)
#endif
	return ParseStatus::ErrorDBCParserUnsupported;
}

/*!
 * \brief numberSignals
 * Determines the number of signals
 * \param ids Vector with all id's found in a log file
 * \return
 */
QStringList DbcParser::signals(const QVector<uint32_t> ids, QHash<uint32_t, int>& idIndex) {
	QStringList s;
#ifdef HAVE_DBC_PARSER
	for (const auto id : ids) {
		const auto messages = m_parser.get_messages();
		for (const auto& message : messages) {
			if (message.id() == id) {
				idIndex.insert(id, s.length());
				// const auto message = m_messages.value(id);
				for (const auto& signal_ : message.signals())
					s.append(QString::fromStdString(signal_.name + "_" + signal_.unit));
				break;
			}
		}
	}
#else
	Q_UNUSED(ids)
	Q_UNUSED(idIndex)
#endif
	return s;
}
