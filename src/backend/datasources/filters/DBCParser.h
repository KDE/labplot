#ifndef DBCPARSER_H
#define DBCPARSER_H

#include <QVector>
#ifdef HAVE_DBC_PARSER
#include <libdbc/dbc.hpp>
#endif

class QString;

class DbcParser {
public:
    enum class ParseStatus {
        Success = 0,
        ErrorInvalidFile = -1,
        ErrorBigEndian = -2,
        ErrorMessageToLong = -3,
        ErrorDBCParserUnsupported = -4,
        ErrorUnknownID = -5,
    };

	bool isValid();
	bool parseFile(const QString& filename);

    ParseStatus parseMessage(const uint32_t id, const std::vector<uint8_t>& data, std::vector<double>& out);
    ParseStatus parseMessage(const uint32_t id, const std::array<uint8_t, 8>& data, std::vector<double>& out);

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
    libdbc::DbcParser m_parser;
#endif
};

#endif // DBCPARSER_H
