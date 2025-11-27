#ifndef DBCPARSER_H
#define DBCPARSER_H

#include <QStringList>
#include <QVector>
#ifdef HAVE_DBC_PARSER
#include <libdbc/dbc.hpp>
#include <libdbc/exceptions/error.hpp>
#endif

#include <vector>

class QString;

class DbcParser {
public:
	enum class ParseStatus {
		Success,
		ErrorInvalidFile,
		ErrorBigEndian,
		ErrorMessageToLong,
		ErrorDBCParserUnsupported,
		ErrorUnknownID,
		ErrorInvalidConversion,
	};

	ParseStatus isValid();
	ParseStatus parseFile(const QString& filename);

	ParseStatus parseMessage(const uint32_t id, const std::vector<uint8_t>& data, std::vector<double>& out);

	struct ValueDescriptions {
		uint32_t value;
		QString description;
	};

	struct Signals {
		QStringList signal_names;
		std::vector<std::vector<ValueDescriptions>> value_descriptions;
	};

	enum class PrefixType {
		None,
		Message, // Use the message name as prefix for the signal_names
	};

	enum class SuffixType {
		None,
		UnitIfAvailable, // _<Unit> only if Unit is not empty, otherwise empty
		Unit // always _<Unit> even if Unit is empty
	};

	/*!
	 * \brief numberSignals
	 * Determines the number of signals
	 * \param ids Vector with all id's found in a log file
	 * \return
	 */
	void getSignals(const QVector<uint32_t>& ids, PrefixType p, SuffixType s, QHash<uint32_t, int>& idIndex, Signals& out) const;

private:
	DbcParser::ParseStatus m_parseFileStatus{DbcParser::ParseStatus::ErrorDBCParserUnsupported};
	// QMap<uint32_t, libdbc::Message> m_messages;
#ifdef HAVE_DBC_PARSER
	Libdbc::DbcParser m_parser;
#endif
};

#endif // DBCPARSER_H
