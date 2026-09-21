#include <cstddef>
#include <fast_float/fast_float.h>
#include <istream>
#include <libdbc/utils/utils.hpp>
#include <regex>
#include <string>

namespace Utils {

std::istream& StreamHandler::get_line(std::istream& stream, std::string& line) {
	std::string newline;

	std::getline(stream, newline);

	// Windows CRLF (\r\n)
	if (!newline.empty() && newline[newline.size() - 1] == '\r') {
		line = newline.substr(0, newline.size() - 1);
		// MacOS LF (\r)
	} else if (!newline.empty() && newline[newline.size()] == '\r') {
		line = newline.replace(newline.size(), 1, "\n");
	} else {
		line = newline;
	}

	return stream;
}

std::istream& StreamHandler::get_next_non_blank_line(std::istream& stream, std::string& line) {
	bool is_blank = true;

	const std::regex whitespace_re("\\s*(.*)");
	std::smatch match;

	while (is_blank) {
		Utils::StreamHandler::get_line(stream, line);

		std::regex_search(line, match, whitespace_re);

		if ((!line.empty() && !match.empty()) || (stream.eof())) {
			if ((match.length(1) > 0) || (stream.eof())) {
				is_blank = false;
			}
		}
	}

	return stream;
}

std::istream& StreamHandler::skip_to_next_blank_line(std::istream& stream, std::string& line) {
	bool line_is_empty = false;

	const std::regex whitespace_re("\\s*(.*)");
	std::smatch match;

	while (!line_is_empty) {
		Utils::StreamHandler::get_line(stream, line);

		std::regex_search(line, match, whitespace_re);

		if ((match.length(1) == 0) || (stream.eof())) {
			line_is_empty = true;
		}
	}

	return stream;
}

std::string String::trim(const std::string& line) {
	const char* WhiteSpace = " \t\v\r\n";
	std::size_t start = line.find_first_not_of(WhiteSpace);
	std::size_t end = line.find_last_not_of(WhiteSpace);
	return start == end ? std::string() : line.substr(start, end - start + 1);
}

double String::convert_to_double(const std::string& value, double default_value) {
	double converted_value = default_value;
	// NOLINTNEXTLINE -- Trying to iterators on the value causes the test to infinitly hang on windows builds
	fast_float::from_chars(value.data(), value.data() + value.size(), converted_value);
	return converted_value;
}

} // Namespace Utils
