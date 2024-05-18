#include "DBCParser.h"

#include <cmath>

#include <QHash>
#include <QStringList>

DbcParser::ParseStatus DbcParser::isValid() {
	return m_parseFileStatus;
}
DbcParser::ParseStatus DbcParser::parseFile(const QString& filename) {
	m_parseFileStatus = DbcParser::ParseStatus::ErrorDBCParserUnsupported;
#ifdef HAVE_DBC_PARSER
	try {
		m_parser.parse_file(filename.toStdString());
		m_parseFileStatus = DbcParser::ParseStatus::Success;
	} catch (const Libdbc::ValidityError&) {
		// e.what(); // TODO: turn on
		m_parseFileStatus = DbcParser::ParseStatus::ErrorInvalidFile;
	}
#else
	Q_UNUSED(filename)
#endif
	return m_parseFileStatus;
}

DbcParser::ParseStatus DbcParser::parseMessage(const uint32_t id, const std::vector<uint8_t>& data, std::vector<double>& out) {
#ifdef HAVE_DBC_PARSER
	if (m_parseFileStatus != ParseStatus::Success)
		return m_parseFileStatus;
	switch (m_parser.parse_message(id, data, out)) {
	case Libdbc::Message::ParseSignalsStatus::Success:
		return ParseStatus::Success;
	case Libdbc::Message::ParseSignalsStatus::ErrorMessageToLong:
		return ParseStatus::ErrorMessageToLong;
	case Libdbc::Message::ParseSignalsStatus::ErrorBigEndian:
		return ParseStatus::ErrorBigEndian;
	case Libdbc::Message::ParseSignalsStatus::ErrorUnknownID:
		return ParseStatus::ErrorUnknownID;
	case Libdbc::Message::ParseSignalsStatus::ErrorInvalidConversion:
		return ParseStatus::ErrorInvalidConversion;
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
void DbcParser::getSignals(const QVector<uint32_t> ids, PrefixType p, SuffixType s, QHash<uint32_t, int>& idIndex, Signals& out) const {
	out.signal_names.clear();
	out.value_descriptions.clear();
#ifdef HAVE_DBC_PARSER
	for (const auto id : ids) {
		for (const auto& message : m_parser.get_messages()) {
			if (message.id() == id) {
				idIndex.insert(id, out.value_descriptions.size());
				for (const auto& signal_ : message.get_signals()) {
					std::string signal_name;
					switch (p) {
					case PrefixType::Message:
						signal_name += message.name() + "_";
						break;
					case PrefixType::None:
						break;
					}

					signal_name += signal_.name;

					switch (s) {
					case SuffixType::None:
						break;
					case SuffixType::Unit:
						signal_name += "_" + signal_.unit;
						break;
					case SuffixType::UnitIfAvailable:
						if (signal_.unit.size() != 0)
							signal_name += "_" + signal_.unit;
						break;
					}
					out.signal_names.append(QString::fromStdString(signal_name));
					std::vector<ValueDescriptions> vd;
					vd.reserve(signal_.value_descriptions.size());
					for (const auto& svdescription : signal_.value_descriptions) {
						vd.push_back({svdescription.value, QString::fromStdString(svdescription.description)});
					}
					out.value_descriptions.push_back({vd});
				}
				break;
			}
		}
	}
#else
	Q_UNUSED(ids)
	Q_UNUSED(p)
	Q_UNUSED(s)
	Q_UNUSED(idIndex)
	Q_UNUSED(out)
#endif
}
