#include "testing_utils/common.hpp"
#include "testing_utils/defines.hpp"
#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>
#include <libdbc/dbc.hpp>
#include <libdbc/exceptions/error.hpp>
#include <string>

using Catch::Matchers::ContainsSubstring;

TEST_CASE("Testing dbc file loading error issues", "[fileio][error]") {
	auto parser = std::unique_ptr<Libdbc::DbcParser>(new Libdbc::DbcParser());

	SECTION("Loading a non dbc file should throw an error", "[error]") {
		REQUIRE_THROWS_AS(parser->parse_file(TEXT_FILE), Libdbc::NonDbcFileFormatError);
		REQUIRE_THROWS_WITH(parser->parse_file(TEXT_FILE), ContainsSubstring("TextFile.txt"));
	}

	SECTION("Loading a dbc with missing version header throws an error (VERSION)", "[error]") {
		REQUIRE_THROWS_AS(parser->parse_file(MISSING_VERSION_DBC_FILE), Libdbc::DbcFileIsMissingVersion);
		REQUIRE_THROWS_WITH(parser->parse_file(MISSING_VERSION_DBC_FILE), ContainsSubstring("line: (NS_ :)"));
	}

	SECTION("Loading a dbc without the required bit timing section (BS_:)", "[error]") {
		REQUIRE_THROWS_AS(parser->parse_file(MISSING_BIT_TIMING_DBC_FILE), Libdbc::DbcFileIsMissingBitTiming);
		REQUIRE_THROWS_WITH(parser->parse_file(MISSING_BIT_TIMING_DBC_FILE), ContainsSubstring("BU_: DBG DRIVER IO MOTOR SENSOR"));
	}

	SECTION("Loading a dbc with some missing namespace section tags (NS_ :)", "[error]") {
		// Confusion about this type of error. it appears that the header isn't
		// very well standardized for now we ignore this type of error.
		REQUIRE_NOTHROW(parser->parse_file(MISSING_NEW_SYMBOLS_DBC_FILE));
	}

	SECTION("Verify that what() method is accessible for all exceptions", "[error]") {
		auto generic_error = Libdbc::Exception();
		REQUIRE(std::string{generic_error.what()} == "libdbc exception occurred");

		auto validity_check = Libdbc::ValidityError();
		REQUIRE(std::string{validity_check.what()} == "Invalid DBC file");
	}
}

TEST_CASE("Testing dbc file loading", "[fileio]") {
	auto parser = std::unique_ptr<Libdbc::DbcParser>(new Libdbc::DbcParser());

	SECTION("Loading a single simple dbc file", "[dbc]") {
		std::vector<std::string> nodes = {"DBG", "DRIVER", "IO", "MOTOR", "SENSOR"};

		Libdbc::Message msg(500, "IO_DEBUG", 4, "IO");

		std::vector<std::string> receivers{"DBG"};
		Libdbc::Signal sig("IO_DEBUG_test_unsigned", false, 0, 8, false, false, 1, 0, 0, 0, "", receivers);
		msg.append_signal(sig);

		std::vector<Libdbc::Message> msgs = {msg};

		parser->parse_file(SIMPLE_DBC_FILE);

		REQUIRE(parser->get_version() == "1.0.0");

		REQUIRE(parser->get_nodes() == nodes);

		REQUIRE(parser->get_messages() == msgs);

		REQUIRE(parser->get_messages().front().get_signals() == msg.get_signals());
	}
}

TEST_CASE("Testing  big endian, little endian") {
	std::string dbc_contents = PRIMITIVE_DBC + R"(BO_ 234 MSG1: 8 Vector__XXX
 SG_ Sig1 : 55|16@0- (0.1,0) [-3276.8|-3276.7] "C" Vector__XXX
 SG_ Sig2 : 39|16@1- (0.1,0) [-3276.8|-3276.7] "C" Vector__XXX)";
	const auto filename = create_temporary_dbc_with(dbc_contents.c_str());

	auto parser = Libdbc::DbcParser();
	parser.parse_file(filename.c_str());

	REQUIRE(parser.get_messages().size() == 1);
	REQUIRE(parser.get_messages().at(0).name() == "MSG1");
	REQUIRE(parser.get_messages().at(0).size() == 8);
	REQUIRE(parser.get_messages().at(0).get_signals().size() == 2);
	{
		const auto signal = parser.get_messages().at(0).get_signals().at(0);
		REQUIRE(signal.is_bigendian == true);
	}
	{
		const auto signal = parser.get_messages().at(0).get_signals().at(1);
		REQUIRE(signal.is_bigendian == false);
	}
}

TEST_CASE("Testing negative values") {
	std::string dbc_contents = PRIMITIVE_DBC + R"(BO_ 234 MSG1: 58 Vector__XXX
 SG_ Sig1 : 55|16@0- (0.1,0) [-3276.8|-3276.7] "C" Vector__XXX
 SG_ Sig2 : 39|16@0- (0.1,0) [-3276.8|-3276.7] "C" Vector__XXX
 SG_ Sig3 : 23|16@0- (10,0) [-3276.8|-3276.7] "C" Vector__XXX
 SG_ Sig4 : 7|16@0- (1,-10) [0|32767] "" Vector__XXX)";
	const auto filename = create_temporary_dbc_with(dbc_contents.c_str());

	auto parser = Libdbc::DbcParser();
	parser.parse_file(filename.c_str());

	REQUIRE(parser.get_messages().size() == 1);
	REQUIRE(parser.get_messages().at(0).name() == "MSG1");
	REQUIRE(parser.get_messages().at(0).size() == 58);
	REQUIRE(parser.get_messages().at(0).get_signals().size() == 4);

	SECTION("Evaluating first message") {
		const auto signal = parser.get_messages().at(0).get_signals().at(0);
		REQUIRE(signal.factor == 0.1);
		REQUIRE(signal.offset == 0);
		REQUIRE(signal.min == -3276.8);
		REQUIRE(signal.max == -3276.7);
	}
	SECTION("Evaluating second message") {
		const auto signal = parser.get_messages().at(0).get_signals().at(1);
		REQUIRE(signal.factor == 0.1);
		REQUIRE(signal.offset == 0);
		REQUIRE(signal.min == -3276.8);
		REQUIRE(signal.max == -3276.7);
	}
	SECTION("Evaluating third message") {
		const auto signal = parser.get_messages().at(0).get_signals().at(2);
		REQUIRE(signal.factor == 10);
		REQUIRE(signal.offset == 0);
		REQUIRE(signal.min == -3276.8);
		REQUIRE(signal.max == -3276.7);
	}
	SECTION("Evaluating fourth message") {
		const auto signal = parser.get_messages().at(0).get_signals().at(3);
		REQUIRE(signal.factor == 1);
		REQUIRE(signal.offset == -10);
		REQUIRE(signal.min == 0);
		REQUIRE(signal.max == 32767);
	}
}

TEST_CASE("Special characters in unit") {
	std::string dbc_contents = PRIMITIVE_DBC + R"(BO_ 234 MSG1: 255 Vector__XXX
 SG_ Speed : 0|8@1+ (1,0) [0|204] "Km/h"  DEVICE1,DEVICE2,DEVICE3)";
	const auto filename = create_temporary_dbc_with(dbc_contents.c_str());

	auto parser = Libdbc::DbcParser();
	parser.parse_file(filename.c_str());

	REQUIRE(parser.get_messages().size() == 1);
	REQUIRE(parser.get_messages().at(0).name() == "MSG1");
	REQUIRE(parser.get_messages().at(0).size() == 255);
	REQUIRE(parser.get_messages().at(0).get_signals().size() == 1);
	SECTION("Checking that signal with special characters as unit is parsed correctly") {
		const auto signal = parser.get_messages().at(0).get_signals().at(0);
		REQUIRE(signal.unit.compare("Km/h") == 0);
	}
}

TEST_CASE("Signal Value Description") {
	std::string dbc_contents = PRIMITIVE_DBC + R"(BO_ 234 MSG1: 8 Vector__XXX
 SG_ State1 : 0|8@1+ (1,0) [0|200] "Km/h"  DEVICE1,DEVICE2,DEVICE3
 SG_ State2 : 0|8@1+ (1,0) [0|204] ""  DEVICE1,DEVICE2,DEVICE3
VAL_ 234 State1 123 "Description 1" 0 "Description 2" 90903489 "Big value and special characters &$Â§())!" ;)";
	const auto filename = create_temporary_dbc_with(dbc_contents.c_str());

	auto parser = Libdbc::DbcParser();
	parser.parse_file(filename.c_str());

	REQUIRE(parser.get_messages().size() == 1);
	REQUIRE(parser.get_messages().at(0).name() == "MSG1");
	REQUIRE(parser.get_messages().at(0).get_signals().size() == 2);

	REQUIRE(parser.get_messages().at(0).get_signals().at(0).value_descriptions.size() == 3);
	REQUIRE(parser.get_messages().at(0).get_signals().at(1).value_descriptions.size() == 0);

	const auto signal = parser.get_messages().at(0).get_signals().at(0);
	REQUIRE(signal.value_descriptions.at(0).value == 123);
	REQUIRE(signal.value_descriptions.at(0).description == "Description 1");
	REQUIRE(signal.value_descriptions.at(1).value == 0);
	REQUIRE(signal.value_descriptions.at(1).description == "Description 2");
	REQUIRE(signal.value_descriptions.at(2).value == 90903489);
	REQUIRE(signal.value_descriptions.at(2).description == "Big value and special characters &$Â§())!");
}

TEST_CASE("Signal Value Description Extended CAN id") {
	std::string dbc_contents = PRIMITIVE_DBC + R"(BO_ 3221225472 MSG1: 8 Vector__XXX
 SG_ State1 : 0|8@1+ (1,0) [0|200] "Km/h"  DEVICE1,DEVICE2,DEVICE3
 SG_ State2 : 0|8@1+ (1,0) [0|204] ""  DEVICE1,DEVICE2,DEVICE3
VAL_ 3221225472 State1 123 "Description 1" 0 "Description 2" 4000000000 "Big value and special characters &$Â§())!" ;)";
	const auto filename = create_temporary_dbc_with(dbc_contents.c_str());

	auto parser = Libdbc::DbcParser();
	parser.parse_file(filename.c_str());

	REQUIRE(parser.get_messages().size() == 1);
	REQUIRE(parser.get_messages().at(0).name() == "MSG1");
	REQUIRE(parser.get_messages().at(0).get_signals().size() == 2);

	REQUIRE(parser.get_messages().at(0).get_signals().at(0).value_descriptions.size() == 3);
	REQUIRE(parser.get_messages().at(0).get_signals().at(1).value_descriptions.size() == 0);

	const auto signal = parser.get_messages().at(0).get_signals().at(0);
	REQUIRE(signal.value_descriptions.at(0).value == 123);
	REQUIRE(signal.value_descriptions.at(0).description == "Description 1");
	REQUIRE(signal.value_descriptions.at(1).value == 0);
	REQUIRE(signal.value_descriptions.at(1).description == "Description 2");
	REQUIRE(signal.value_descriptions.at(2).value == 4000000000);
	REQUIRE(signal.value_descriptions.at(2).description == "Big value and special characters &$Â§())!");
}

TEST_CASE("Signal Value Multiple VAL_") {
	std::string dbc_contents = PRIMITIVE_DBC + R"(BO_ 3221225472 MSG1: 8 Vector__XXX
 SG_ State1 : 0|8@1+ (1,0) [0|200] "Km/h"  DEVICE1,DEVICE2,DEVICE3
 SG_ State2 : 0|8@1+ (1,0) [0|204] ""  DEVICE1,DEVICE2,DEVICE3"
BO_ 123 MSG2: 8 Vector__XXX
 SG_ State1 : 0|8@1+ (1,0) [0|200] "Km/h"  DEVICE1,DEVICE2,DEVICE3
 SG_ State2 : 0|8@1+ (1,0) [0|204] ""  DEVICE1,DEVICE2,DEVICE3
VAL_ 3221225472 State1 123 "Description 1" 0 "Description 2" ;
VAL_ 123 State1 123 "Description 3" 0 "Description 4" ;)";
	const auto filename = create_temporary_dbc_with(dbc_contents.c_str());

	auto parser = Libdbc::DbcParser();
	parser.parse_file(filename.c_str());

	REQUIRE(parser.get_messages().size() == 2);
	REQUIRE(parser.get_messages().at(0).name() == "MSG1");
	REQUIRE(parser.get_messages().at(1).name() == "MSG2");

	REQUIRE(parser.get_messages().at(0).get_signals().size() == 2);

	REQUIRE(parser.get_messages().at(0).get_signals().at(0).value_descriptions.size() == 2);
	REQUIRE(parser.get_messages().at(0).get_signals().at(1).value_descriptions.size() == 0);
	REQUIRE(parser.get_messages().at(1).get_signals().at(0).value_descriptions.size() == 2);
	REQUIRE(parser.get_messages().at(1).get_signals().at(1).value_descriptions.size() == 0);

	const auto signal = parser.get_messages().at(0).get_signals().at(0);
	REQUIRE(signal.value_descriptions.at(0).value == 123);
	REQUIRE(signal.value_descriptions.at(0).description == "Description 1");
	REQUIRE(signal.value_descriptions.at(1).value == 0);
	REQUIRE(signal.value_descriptions.at(1).description == "Description 2");

	const auto signal2 = parser.get_messages().at(1).get_signals().at(0);
	REQUIRE(signal2.value_descriptions.at(0).value == 123);
	REQUIRE(signal2.value_descriptions.at(0).description == "Description 3");
	REQUIRE(signal2.value_descriptions.at(1).value == 0);
	REQUIRE(signal2.value_descriptions.at(1).description == "Description 4");
}

TEST_CASE("Should parse DBC with empty BU_", "[error][optional]") {
	std::string contents = R"(VERSION ""


NS_ :

BS_:

BU_:


BO_ 293 Msg1: 2 Vector__XXX
 SG_ Wert7 : 0|16@1- (1,0) [0|0] "" Vector__XXX

BO_ 292 Msg2: 1 Vector__XXX
 SG_ Wert8 : 56|8@1- (1,0) [0|0] "" Vector__XXX
)";
	const auto filename = create_temporary_dbc_with(contents.c_str());

	auto parser = Libdbc::DbcParser();
	REQUIRE_NOTHROW(parser.parse_file(filename.c_str()));

	REQUIRE(parser.get_messages().size() == 2);
	REQUIRE(parser.get_messages().at(0).name() == "Msg1");
	REQUIRE(parser.get_messages().at(1).name() == "Msg2");
}

TEST_CASE("Should report unused lines since we don't have tracing.", "[parsing]") {
	std::string contents = R"(VERSION ""

NS_ :

BS_:

BU_:


BO_ 293 Msg1: 2 Vector__XXX
 SG_ Whitespace: | 0|16@1- (1,0) [0|0] "" Vector__XXX
 SG_ Wert7 : 0|16@1- (1,0) [0|0] "" Vector__XXX
 SG_ Wert8 : 0|16@1- (1,0) [0|0] "" Vector__XXX

BO_ 292 Msg2: 1 Vector__XXX
 SG_ Wert8 : 56|8@1- (1,0) [0|0] "" Vector__XXX
 SB_ not a correct line

BO_ have a issue here:
)";

	const auto filename = create_temporary_dbc_with(contents.c_str());

	auto parser = Libdbc::DbcParser();
	REQUIRE_NOTHROW(parser.parse_file(filename.c_str()));

	REQUIRE(parser.get_messages().size() == 2);
	REQUIRE(parser.get_messages()[0].size() == 2);
	REQUIRE(parser.get_messages()[1].size() == 1);

	auto unused = parser.unused_lines();

	// We could match them all here but i think just a check that the size is sufficent.
	REQUIRE(unused.size() == 3);
}
