#include "DBCParser.h"

#include <cmath>

#include <QHash>
#include <QStringList>

bool DbcParser::isValid() {
	return m_valid;
}
bool DbcParser::parseFile(const QString& filename) {
	m_valid = false;
	try {
		m_parser.parse_file(filename.toStdString());
		m_valid = true;
	} catch (libdbc::validity_error e) {
		// e.what(); // TODO: turn on
	}
	return m_valid;
}

QVector<double> DbcParser::parseMessage(const uint32_t id, const std::vector<uint8_t>& data) {
	if (!m_valid)
		return QVector<double>();

	std::vector<double> values;
	m_parser.parseMessage(id, data, values);

	return QVector<double>(values.begin(), values.end());
}

/*!
 * \brief numberSignals
 * Determines the number of signals
 * \param ids Vector with all id's found in a log file
 * \return
 */
QStringList DbcParser::signals(const QVector<uint32_t> ids, QHash<uint32_t, int>& idIndex) {
	QStringList s;
	for (const auto id : ids) {
		const auto messages = m_parser.get_messages();
		for (const auto& message : messages) {
			if (message.id() == id) {
				idIndex.insert(id, s.length());
				// const auto message = m_messages.value(id);
				for (const auto& signal_ : message.signals()) {
					s.append(QString::fromStdString(signal_.name + "_" + signal_.unit));
				}
				break;
			}
		}
	}
	return s;
}
