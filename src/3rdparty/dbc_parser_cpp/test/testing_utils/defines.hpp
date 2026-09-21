#include <string>

// Correctly formated files
static const std::string COMPLEX_DBC_FILE = std::string(TESTDBCFILES_PATH) + "/Complex.dbc";
static const std::string SIMPLE_DBC_FILE = std::string(TESTDBCFILES_PATH) + "/Simple.dbc";

// Files with Errors
static const std::string MISSING_NEW_SYMBOLS_DBC_FILE = std::string(TESTDBCFILES_PATH) + "/MissingNewSymbols.dbc";
static const std::string MISSING_VERSION_DBC_FILE = std::string(TESTDBCFILES_PATH) + "/MissingVersion.dbc";
static const std::string MISSING_BIT_TIMING_DBC_FILE = std::string(TESTDBCFILES_PATH) + "/MissingBitTiming.dbc";
static const std::string TEXT_FILE = std::string(TESTDBCFILES_PATH) + "/TextFile.txt";

static const std::string PRIMITIVE_DBC =
	R"(VERSION "1.0.0"

NS_ :

BS_:

BU_: DBG DRIVER IO MOTOR SENSOR

)";
