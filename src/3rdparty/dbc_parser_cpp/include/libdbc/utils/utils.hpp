
#ifndef UTILS_HPP
#define UTILS_HPP

#include <iostream>
#include <sstream>
#include <string>

namespace Utils {

class StreamHandler {
public:
	/**
	 * This is a safe non line ending specific get_ine function. This is to help with files
	 * carried over from different systems. i.e Unix file comes to Windows with LF endings
	 * instead of CRLF.
	 *
	 * @param  stream [description]
	 * @param  line   [description]
	 * @return        [description]
	 */
	static std::istream& get_line(std::istream& stream, std::string& line);

	static std::istream& get_next_non_blank_line(std::istream& stream, std::string& line);

	static std::istream& skip_to_next_blank_line(std::istream& stream, std::string& line);
};

class String {
public:
	static std::string trim(const std::string& line);

	template<class Container>
	static void split(const std::string& str, Container& cont, char delim = ' ') {
		std::stringstream stream(str);
		std::string token;

		while (std::getline(stream, token, delim)) {
			cont.push_back(token);
		}
	}

	static double convert_to_double(const std::string& value, double default_value = 0);
};

}

#endif // UTILS_HPP
