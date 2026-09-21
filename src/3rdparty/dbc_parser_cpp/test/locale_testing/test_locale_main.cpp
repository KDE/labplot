#include "testing_utils/common.hpp"
#include "testing_utils/defines.hpp"
#include <libdbc/dbc.hpp>
#include <libdbc/utils/utils.hpp>

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>
#include <catch2/reporters/catch_reporter_event_listener.hpp>
#include <catch2/reporters/catch_reporter_registrars.hpp>

class testRunListener : public Catch::EventListenerBase {
public:
	using Catch::EventListenerBase::EventListenerBase;

	void testRunStarting(Catch::TestRunInfo const&) override {
		// Mac OS uses global and c++ standard uses the std. Using this to remove ambiguity between the two.
		prev_loc = ::setlocale(LC_ALL, nullptr);
		// Set the locale to something that has , instead of . for floats
		std::locale::global(std::locale("de_DE.UTF-8"));
	}

	void testCaseEnded(Catch::TestCaseStats const&) override {
		// Restore the old locale
		std::locale::global(std::locale(prev_loc));
	}

private:
	std::string prev_loc;
};

CATCH_REGISTER_LISTENER(testRunListener)

TEST_CASE("Should parse doubld string locale independently") {
	REQUIRE(Catch::Approx(Utils::String::convert_to_double("6.82")) == 6.82);
}

TEST_CASE("Should process message with floats locale indpendently") {
	std::string dbc_contents = PRIMITIVE_DBC + R"(BO_ 234 MSG1: 8 Vector__XXX
 SG_ Sig1 : 55|16@0- (0.1,0) [-3276.8|-3276.7] "C" Vector__XXX
 SG_ Sig2 : 39|16@0- (0.1,0) [-3276.8|-3276.7] "C" Vector__XXX
 SG_ Sig3 : 23|16@0- (10,0) [-3276.8|-3276.7] "C" Vector__XXX
 SG_ Sig4 : 7|16@0- (1,-10) [0|32767] "" Vector__XXX)";
	const auto filename = create_temporary_dbc_with(dbc_contents.c_str());

	auto parser = Libdbc::DbcParser();
	parser.parse_file(filename);

	REQUIRE(parser.get_messages().size() == 1);
	REQUIRE(parser.get_messages().at(0).name() == "MSG1");
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
