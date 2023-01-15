#ifndef DBCPARSER_H
#define DBCPARSER_H

#include <QVector>
#ifdef HAVE_DBC_PARSER
#include <libdbc/dbc.hpp>
#endif

class QString;

class DbcParser {
public:
	bool isValid();
	bool parseFile(const QString& filename);

	bool parseMessage(const uint32_t id, const std::vector<uint8_t>& data, std::vector<double>& out);
	bool parseMessage(const uint32_t id, const std::array<uint8_t, 8>& data, std::vector<double>& out);

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
#ifdef HAVE_DBC_PARSER
	libdbc::DbcParser m_parser{libdbc::DbcParser(true)};
#endif
};

#endif // DBCPARSER_H
