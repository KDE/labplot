#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers.hpp>

#include <libdbc/dbc.hpp>

#include "testing_utils/common.hpp"
#include "testing_utils/defines.hpp"

// Testing of parsing messages

TEST_CASE("Parse Message Unknown ID") {
	std::string dbc_contents = PRIMITIVE_DBC + R"(BO_ 234 MSG1: 8 Vector__XXX
 SG_ Msg1Sig1 : 0|8@0+ (1,0) [-3276.8|-3276.7] "C" Vector__XXX
 SG_ MsgSig2 : 8|8@0+ (1,0) [-3276.8|-3276.7] "C" Vector__XXX
BO_ 123 MSG2: 8 Vector__XXX
 SG_ Msg2Sig1 : 0|8@0+ (1,0) [-3276.8|-3276.7] "C" Vector__XXX
 SG_ Msg2Sig1 : 8|8@0+ (1,0) [-3276.8|-3276.7] "C" Vector__XXX
)";
	const auto filename = create_temporary_dbc_with(dbc_contents.c_str());

	Libdbc::DbcParser parser;
	parser.parse_file(filename.c_str());

	SECTION("Evaluating unknown` message id") {
		std::vector<double> out_values;
		CHECK(parser.parse_message(578, std::vector<uint8_t>({0xFF, 0xA2}), out_values) == Libdbc::Message::ParseSignalsStatus::ErrorUnknownID);
	}
}

TEST_CASE("Parse Message Big Number not aligned little endian") {
	std::string dbc_contents = PRIMITIVE_DBC + R"(BO_ 337 STATUS: 8 Vector__XXX
 SG_ Value6 : 27|3@1+ (1,0) [0|7] ""  Vector__XXX
 SG_ Value5 : 16|11@1+ (0.1,-102) [-102|102] "%"  Vector__XXX
 SG_ Value2 : 8|2@1+ (1,0) [0|2] ""  Vector__XXX
 SG_ Value3 : 10|1@1+ (1,0) [0|1] ""  Vector__XXX
 SG_ Value7 : 30|2@1+ (1,0) [0|3] ""  Vector__XXX
 SG_ Value4 : 11|4@1+ (1,0) [0|3] ""  Vector__XXX
 SG_ Value1 : 0|8@1+ (1,0) [0|204] "Km/h"  Vector__XXX
)";
	const auto filename = create_temporary_dbc_with(dbc_contents.c_str());

	Libdbc::DbcParser parser;
	parser.parse_file(filename);

	SECTION("Evaluating first message") {
		std::vector<double> out_values;
		CHECK(parser.parse_message(337, std::vector<uint8_t>({0, 4, 252, 19, 0, 0, 0, 0}), out_values) == Libdbc::Message::ParseSignalsStatus::Success);
		std::vector<double> refData{2, 0, 0, 1, 0, 0, 0};
		CHECK(refData.size() == 7);
		CHECK(out_values.size() == refData.size());
		for (size_t i = 0; i < refData.size(); i++) {
			CHECK(out_values.at(i) == refData.at(i));
		}
	}

	SECTION("Evaluating second message") {
		std::vector<double> out_values;
		CHECK(parser.parse_message(337, std::vector<uint8_t>({47, 4, 60, 29, 0, 0, 0, 0}), out_values) == Libdbc::Message::ParseSignalsStatus::Success);
		std::vector<double> refData{3, 32, 0, 1, 0, 0, 47};
		CHECK(refData.size() == 7);
		CHECK(out_values.size() == refData.size());
		for (size_t i = 0; i < refData.size(); i++) {
			CHECK(out_values.at(i) == refData.at(i));
		}
	}

	SECTION("Evaluating third message") {
		std::vector<double> out_values;
		CHECK(parser.parse_message(337, std::vector<uint8_t>({57, 4, 250, 29, 0, 0, 0, 0}), out_values) == Libdbc::Message::ParseSignalsStatus::Success);
		std::vector<double> refData{3, 51, 0, 1, 0, 0, 57};
		CHECK(refData.size() == 7);
		CHECK(out_values.size() == refData.size());
		for (size_t i = 0; i < refData.size(); i++) {
			CHECK(out_values.at(i) == refData.at(i));
		}
	}
}

TEST_CASE("Parse Message little endian") {
	std::string dbc_contents = PRIMITIVE_DBC + R"(BO_ 541 STATUS: 8 DEVICE1
 SG_ Temperature : 48|16@1+ (0.01,-40) [-40|125] "C"  DEVICE1
 SG_ SOH : 0|16@1+ (0.01,0) [0|100] "%"  DEVICE1
 SG_ SOE : 32|16@1+ (0.01,0) [0|100] "%"  DEVICE1
 SG_ SOC : 16|16@1+ (0.01,0) [0|100] "%"  DEVICE1)";
	const auto filename = create_temporary_dbc_with(dbc_contents.c_str());

	Libdbc::DbcParser parser;
	parser.parse_file(filename);

	std::vector<uint8_t> data{0x08, 0x27, 0xa3, 0x22, 0xe5, 0x1f, 0x45, 0x14}; // little endian
	std::vector<double> result_values;
	REQUIRE(parser.parse_message(0x21d, data, result_values) == Libdbc::Message::ParseSignalsStatus::Success);
	REQUIRE(result_values.size() == 4);

	REQUIRE(Catch::Approx(result_values.at(0)) == 11.89);
	REQUIRE(Catch::Approx(result_values.at(1)) == 99.92);
	REQUIRE(Catch::Approx(result_values.at(2)) == 81.65);
	REQUIRE(Catch::Approx(result_values.at(3)) == 88.67);
}

TEST_CASE("Parse Message big endian signed values") {
	std::string dbc_contents = PRIMITIVE_DBC + R"(BO_ 545 MSG: 8 BMS2
 SG_ Sig1 : 62|1@0+ (1,0) [0|0] "" Vector__XXX
 SG_ Sig2 : 49|2@0+ (1,0) [0|0] "" Vector__XXX
 SG_ Sig3 : 39|16@0- (0.1,0) [0|0] "A" Vector__XXX
 SG_ Sig4 : 60|1@0+ (1,0) [0|0] "" Vector__XXX
 SG_ Sig5 : 55|1@0+ (1,0) [0|0] "" Vector__XXX
 SG_ Sig6 : 58|1@0+ (1,0) [0|0] "" Vector__XXX
 SG_ Sig7 : 59|1@0+ (1,0) [0|0] "" Vector__XXX
 SG_ Sig8 : 57|1@0+ (1,0) [0|0] "" Vector__XXX
 SG_ Sig9 : 56|1@0+ (1,0) [0|0] "" Vector__XXX
 SG_ Sig10 : 61|1@0+ (1,0) [0|0] "" Vector__XXX
 SG_ Sig11 : 7|16@0+ (0.001,0) [0|65.535] "V" Vector__XXX
 SG_ Sig12 : 23|16@0+ (0.1,0) [0|6553.5] "A" Vector__XXX)";
	const auto filename = create_temporary_dbc_with(dbc_contents.c_str());

	Libdbc::DbcParser p;
	p.parse_file(filename.c_str());

	std::vector<uint8_t> data{13, 177, 0, 216, 251, 180, 0, 31}; // big endian
	std::vector<double> result_values;
	REQUIRE(p.parse_message(545, data, result_values) == Libdbc::Message::ParseSignalsStatus::Success);
	REQUIRE(result_values.size() == 12);
	REQUIRE(Catch::Approx(result_values.at(0)) == 0);
	REQUIRE(Catch::Approx(result_values.at(1)) == 0);
	REQUIRE(Catch::Approx(result_values.at(2)) == -110);
	REQUIRE(Catch::Approx(result_values.at(3)) == 1);
	REQUIRE(Catch::Approx(result_values.at(4)) == 0);
	REQUIRE(Catch::Approx(result_values.at(5)) == 1);
	REQUIRE(Catch::Approx(result_values.at(6)) == 1);
	REQUIRE(Catch::Approx(result_values.at(7)) == 1);
	REQUIRE(Catch::Approx(result_values.at(8)) == 1);
	REQUIRE(Catch::Approx(result_values.at(9)) == 0);
	REQUIRE(Catch::Approx(result_values.at(10)) == 3.5050);
	REQUIRE(Catch::Approx(result_values.at(11)) == 21.6);
}

TEST_CASE("Parse Message with non byte aligned values") {
	std::string dbc_contents = PRIMITIVE_DBC + R"(BO_ 403 INFORMATION: 8 Vector__XXX
 SG_ Voltage : 30|9@1+ (0.2,0) [0|102.2] "V"  Vector__XXX
 SG_ Phase_Current : 20|10@1- (1,0) [-512|512] "A"  Vector__XXX
 SG_ Iq_Current : 10|10@1- (1,0) [-512|512] "A"  Vector__XXX
 SG_ Id_Current : 0|10@1- (1,0) [-512|512] "A"  Vector__XXX)";
	const auto filename = create_temporary_dbc_with(dbc_contents.c_str());

	Libdbc::DbcParser p;
	p.parse_file(filename);

	std::vector<uint8_t> data{131, 51, 33, 9, 33, 0, 0, 0};
	std::vector<double> result_values;
	REQUIRE(p.parse_message(403, data, result_values) == Libdbc::Message::ParseSignalsStatus::Success);
	REQUIRE(result_values.size() == 4);
	REQUIRE(Catch::Approx(result_values.at(0)) == 26.4);
	REQUIRE(Catch::Approx(result_values.at(1)) == 146);
	REQUIRE(Catch::Approx(result_values.at(2)) == 76);
	REQUIRE(Catch::Approx(result_values.at(3)) == -125);
}

TEST_CASE("Parse Message data length < 8 unsigned") {
	std::string dbc_contents = PRIMITIVE_DBC + R"(BO_ 234 MSG1: 8 Vector__XXX
 SG_ Msg1Sig1 : 7|8@0+ (1,0) [-3276.8|-3276.7] "C" Vector__XXX
 SG_ Msg1Sig2 : 15|8@0+ (1,0) [-3276.8|-3276.7] "km/h" Vector__XXX)";
	const auto filename = create_temporary_dbc_with(dbc_contents.c_str());

	Libdbc::DbcParser p;
	p.parse_file(filename);

	std::vector<uint8_t> data{0x1, 0x2};
	std::vector<double> result_values;
	REQUIRE(p.parse_message(234, data, result_values) == Libdbc::Message::ParseSignalsStatus::Success);
	REQUIRE(result_values.size() == 2);
	REQUIRE(Catch::Approx(result_values.at(0)) == 0x1);
	REQUIRE(Catch::Approx(result_values.at(1)) == 0x2);
}

TEST_CASE("Parse message with BO_ on single line should fail.") {
	std::string dbc_contents = PRIMITIVE_DBC + R"(BO_
234 MSG1: 8 Vector__XXX
 SG_ State1 : 0|8@1+ (1,0) [0|200] "Km/h"  DEVICE1,DEVICE2,DEVICE3
 SG_ State2 : 0|8@1+ (1,0) [0|204] "" DEVICE1,DEVICE2,DEVICE3
VAL_ 234 State1 123 "Description 1" 0 "Description 2" 90903489 "Big value and special characters &$Â§())!")";
	const auto filename = create_temporary_dbc_with(dbc_contents.c_str());

	Libdbc::DbcParser p;
	p.parse_file(filename);

	std::vector<uint8_t> data{0x1, 0x2};
	std::vector<double> result_values;
	REQUIRE(p.get_messages().size() == 0);
	REQUIRE(p.parse_message(234, data, result_values) == Libdbc::Message::ParseSignalsStatus::ErrorUnknownID);
}

TEST_CASE("Parse signal with SG_ on single line should fail.") {
	std::string dbc_contents = PRIMITIVE_DBC + R"(BO_ 234 MSG1: 8 Vector__XXX
 SG_
State1 : 0|8@1+ (1,0) [0|200] "Km/h"  DEVICE1,DEVICE2,DEVICE3
 SG_ State2 : 0|8@1+ (1,0) [0|204] "" DEVICE1,DEVICE2,DEVICE3
VAL_ 234 State1 123 "Description 1" 0 "Description 2" 90903489 "Big value and special characters &$Â§())!")";
	const auto filename = create_temporary_dbc_with(dbc_contents.c_str());

	Libdbc::DbcParser p;
	p.parse_file(filename);

	std::vector<uint8_t> data{0x1, 0x2};
	std::vector<double> result_values;
	REQUIRE(p.get_messages().size() == 1);
	REQUIRE(p.parse_message(234, data, result_values) == Libdbc::Message::ParseSignalsStatus::Success);
	REQUIRE(result_values.size() == 1);
	REQUIRE(Catch::Approx(result_values.at(0)) == 0x1);
}
