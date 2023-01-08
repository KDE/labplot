#ifndef DBCPARSER_H
#define DBCPARSER_H

#include <QVector>
#include <libdbc/dbc.hpp>

class QString;

class DbcParser {
public:
	bool isValid();
	bool parseFile(const QString& filename);

	QVector<double> parseMessage(const uint32_t id, const std::vector<uint8_t>& data);

	/*!
	 * \brief numberSignals
	 * Determines the number of signals
	 * \param ids Vector with all id's found in a log file
	 * \return
	 */
	QStringList signals(const QVector<uint32_t> ids, QHash<uint32_t, int>& idIndex);

private:
	bool m_valid{false};
	// QMap<uint32_t, libdbc::Message> m_messages;
	libdbc::DbcParser m_parser{libdbc::DbcParser(true)};
};

#endif // DBCPARSER_H
