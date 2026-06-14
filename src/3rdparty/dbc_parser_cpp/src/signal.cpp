#include <cstdint>
#include <libdbc/signal.hpp>
#include <ostream>
#include <string>
#include <vector>

namespace Libdbc {
Signal::Signal(std::string name,
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
			   std::vector<std::string> receivers)
	: name(name)
	, is_multiplexed(is_multiplexed)
	, start_bit(start_bit)
	, size(size)
	, is_bigendian(is_bigendian)
	, is_signed(is_signed)
	, factor(factor)
	, offset(offset)
	, min(min)
	, max(max)
	, unit(unit)
	, receivers(receivers) {
}

bool Signal::operator==(const Signal& rhs) const {
	return (this->name == rhs.name) && (this->is_multiplexed == rhs.is_multiplexed) && (this->start_bit == rhs.start_bit) && (this->size == rhs.size)
		&& (this->is_bigendian == rhs.is_bigendian) && (this->is_signed == rhs.is_signed) && (this->offset == rhs.offset) && (this->min == rhs.min)
		&& (this->max == rhs.max) && (this->unit == rhs.unit) && (this->receivers == rhs.receivers);
}

bool Signal::operator<(const Signal& rhs) const {
	return start_bit < rhs.start_bit;
}

std::ostream& operator<<(std::ostream& out, const Signal& sig) {
	out << "Signal {name: " << sig.name << ", ";
	out << "Multiplexed: " << (sig.is_multiplexed ? "True" : "False") << ", ";
	out << "Start bit: " << sig.start_bit << ", ";
	out << "Size: " << sig.size << ", ";
	out << "Endianness: " << (sig.is_bigendian ? "Big endian" : "Little endian") << ", ";
	out << "Value Type: " << (sig.is_signed ? "Signed" : "Unsigned") << ", ";
	out << "Min: " << sig.min << ", Max: " << sig.max << ", ";
	out << "Unit: (" << sig.unit << "), ";
	out << "receivers: ";
	for (const auto& reciever : sig.receivers) {
		out << reciever;
	}
	return out << "}";
}
}
