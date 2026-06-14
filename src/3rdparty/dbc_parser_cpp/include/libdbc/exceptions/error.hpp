#ifndef ERROR_HPP
#define ERROR_HPP

#include <exception>
#include <string>

namespace Libdbc {

class Exception : public std::exception {
public:
	const char* what() const throw() override {
		return "libdbc exception occurred";
	}
};

class ValidityError : public Exception {
public:
	const char* what() const throw() override {
		return "Invalid DBC file";
	}
};

class NonDbcFileFormatError : public ValidityError {
public:
	NonDbcFileFormatError(const std::string& path, const std::string& extension) {
		error_msg = {"File is not of DBC format. Expected a .dbc extension. Cannot read this type of file (" + path + "). Found the extension (" + extension
					 + ")."};
	}

	const char* what() const throw() override {
		return error_msg.c_str();
	}

private:
	std::string error_msg;
};

class DbcFileIsMissingVersion : public ValidityError {
public:
	DbcFileIsMissingVersion(const std::string& line) {
		error_msg = {"Invalid dbc file. Missing the required version header. Attempting to read line: (" + line + ")."};
	}

	const char* what() const throw() override {
		return error_msg.c_str();
	}

private:
	std::string error_msg;
};

class DbcFileIsMissingBitTiming : public ValidityError {
public:
	DbcFileIsMissingBitTiming(const std::string& line) {
		error_msg = {"Invalid dbc file. Missing required bit timing in the header. Attempting to read line: (" + line + ")."};
	}

	const char* what() const throw() override {
		return error_msg.c_str();
	}

private:
	std::string error_msg;
};

} // libdbc

#endif // ERROR_HPP
