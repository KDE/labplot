#include "testing_utils/common.hpp"
#include "testing_utils/defines.hpp"
#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <random>
#include <stdexcept>

// Don't want to use tmpnam due to warnings. So here is an alternative using time and random numbers.
// This should be platform agnostic as well.
static std::string generate_unique_filename();
std::string generate_unique_filename() {
	// Get current time since epoch
	auto now = std::chrono::system_clock::now();
	auto duration = now.time_since_epoch();
	auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();

	// Generate a random number
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_int_distribution<> dis(0, 9999);
	int random_num = dis(gen);

	// Concatenate time and random number to create a unique filename
	return "temp_file_" + std::to_string(milliseconds) + "_" + std::to_string(random_num) + ".dbc";
}

std::string create_temporary_dbc_with(const char* contents) {
	std::filesystem::path temp_dir = std::filesystem::temp_directory_path();

	// Generate a unique temporary file name
	std::string filename = generate_unique_filename();
	std::filesystem::path temp_file = temp_dir / filename;

	std::ofstream file(temp_file);
	if (!file.is_open()) {
		throw std::runtime_error("Failed to create temporary file.");
	}

	file << contents << std::endl;
	file.close();

	return temp_file.string();
}
