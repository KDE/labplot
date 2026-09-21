
#ifndef SIGNAL_HPP
#define SIGNAL_HPP

#include <cstdint>
#include <iostream>
#include <string>
#include <vector>

namespace Libdbc {
struct Signal {
	struct ValueDescription {
		uint32_t value;
		std::string description;
	};

	std::string name;
	bool is_multiplexed;
	uint32_t start_bit;
	uint32_t size;
	bool is_bigendian;
	bool is_signed;
	double factor;
	double offset;
	double min;
	double max;
	std::string unit;
	std::vector<std::string> receivers;
	std::vector<ValueDescription> value_descriptions;

	Signal() = delete;
	virtual ~Signal() = default;
	explicit Signal(std::string name,
					bool is_multiplexed,
					uint32_t start_bit,
					uint32_t size,
					bool is_bigendian,
					bool is_signed,
					double factor,
					double offset,
					double min,
					double max,
					std::string unit,
					std::vector<std::string> receivers);

	virtual bool operator==(const Signal& rhs) const;
	bool operator<(const Signal& rhs) const;
};

std::ostream& operator<<(std::ostream& out, const Signal& sig);

}

#endif // SIGNAL_HPP
