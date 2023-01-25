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
    m_parser.Filename(filename.toStdString());
    m_valid = m_parser.ParseFile();
#endif
	return m_valid;
}

bool DbcParser::parseMessage(const uint32_t id, const std::vector<uint8_t>& data, std::vector<double>& out) {
	if (!m_valid)
		return false;
#ifdef HAVE_DBC_PARSER
    dbc::DbcMessage msg(0, id, data);
    if (!m_parser.ParseMessage(msg))
        return false;

    dbc::Message* message = m_parser.GetNetwork()->GetMessage(234);
    if (message) {
        for (const auto& signalPair: message->Signals()) {
            double value;
            signalPair.second.EngValue(value);
            out.push_back(value);
        }
    }
#endif

	return true;
}

bool DbcParser::parseMessage(const uint32_t id, const std::array<uint8_t, 8>& data, std::vector<double>& out) {
	if (!m_valid)
		return false;

#ifdef HAVE_DBC_PARSER
//    dbc::DbcMessage msg(0, id, data);
//    m_parser.ParseMessage(msg);
    return false; // currently not supported
#endif

	return true;
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
        const auto* message = m_parser.GetNetwork()->GetMessageByCanId(id);
        if (!message)
            return QStringList();

        idIndex.insert(id, s.length());
        // const auto message = m_messages.value(id);
        for (const auto& signal_ : message->Signals()) {
            s.append(QString::fromStdString(signal_.second.Name() + "_" + signal_.second.Unit()));
        }
    }
#endif
	return s;
}
