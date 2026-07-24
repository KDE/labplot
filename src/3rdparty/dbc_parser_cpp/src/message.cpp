#include <cstddef>
#include <cstdint>
#include <libdbc/message.hpp>
#include <libdbc/signal.hpp>
#include <ostream>
#include <string>
#include <vector>

namespace Libdbc {

constexpr unsigned ONE_BYTE = 8;
constexpr unsigned TWO_BYTES = 16;
constexpr unsigned FOUR_BYTES = 32;
constexpr unsigned EIGHT_BYTES = 64;

constexpr unsigned SEVEN_BITS = 7;

Message::Message(uint32_t message_id, const std::string& name, uint8_t size, const std::string& node)
	: m_id(message_id)
	, m_name(name)
	, m_size(size)
	, m_node(node) {
}

bool Message::operator==(const Message& rhs) const {
	return (m_id == rhs.id()) && (m_name == rhs.m_name) && (m_size == rhs.m_size) && (m_node == rhs.m_node);
}

Message::ParseSignalsStatus Message::parse_signals(const std::vector<uint8_t>& data, std::vector<double>& values) const {
	auto size = data.size();
	if (size > ONE_BYTE) {
		return ParseSignalsStatus::ErrorMessageToLong; // not supported yet
	}

	uint64_t data_little_endian = 0;
	uint64_t data_big_endian = 0;
	for (std::size_t i = 0; i < size; i++) {
		data_little_endian |= ((uint64_t)data[i]) << i * ONE_BYTE;
		data_big_endian = (data_big_endian << ONE_BYTE) | (uint64_t)data[i];
	}

	// TODO: does this also work on a big endian machine?

	const auto len = size * 8;
	uint64_t value = 0;
	for (const auto& signal : m_signals) {
		if (signal.is_bigendian) {
			uint32_t start_bit = ONE_BYTE * (signal.start_bit / ONE_BYTE) + (SEVEN_BITS - (signal.start_bit % ONE_BYTE)); // Calculation taken from python CAN
			value = data_big_endian << start_bit;
			value = value >> (len - signal.size);
		} else {
			value = data_little_endian >> signal.start_bit;
		}

		if (signal.is_signed && signal.size > 1) {
			switch (signal.size) {
			case ONE_BYTE:
				values.push_back(static_cast<int8_t>(value) * signal.factor + signal.offset);
				break;
			case TWO_BYTES:
				values.push_back(static_cast<int16_t>(value) * signal.factor + signal.offset);
				break;
			case FOUR_BYTES:
				values.push_back(static_cast<int32_t>(value) * signal.factor + signal.offset);
				break;
			case EIGHT_BYTES:
				values.push_back(static_cast<double>(value) * signal.factor + signal.offset);
				break;
			default: {
				// 2 complement -> decimal
				const bool is_negative = (value & (1ULL << (signal.size - 1))) != 0;
				int64_t nativeInt = 0;
				if (is_negative) {
					nativeInt = static_cast<int64_t>(value | ~((1ULL << signal.size) - 1)); // invert all bits above signal.size
				} else {
					nativeInt = static_cast<int64_t>(value & ((1ULL << signal.size) - 1)); // masking
				}
				values.push_back(static_cast<double>(nativeInt) * signal.factor + signal.offset);
				break;
			}
			}
		} else {
			// use only the relevant bits
			value = value & ((1 << signal.size) - 1); // masking
			values.push_back(static_cast<double>(value) * signal.factor + signal.offset);
		}
	}
	return ParseSignalsStatus::Success;
}

void Message::append_signal(const Signal& signal) {
	m_signals.push_back(signal);
}

std::vector<Signal> Message::get_signals() const {
	return m_signals;
}

uint32_t Message::id() const {
	return m_id;
}

uint8_t Message::size() const {
	return m_size;
}

const std::string& Message::name() const {
	return m_name;
}

void Message::add_value_description(const std::string& signal_name, const std::vector<Signal::ValueDescription>& value_descriptor) {
	for (auto& signal : m_signals) {
		if (signal.name == signal_name) {
			signal.value_descriptions = value_descriptor;
			return;
		}
	}
}

std::ostream& operator<<(std::ostream& out, const Message& msg) {
	out << "Message: {id: " << msg.id() << ", ";
	out << "name: " << msg.m_name << ", ";
	out << "size: " << msg.m_size << ", ";
	out << "node: " << msg.m_node << "}";
	return out;
}
}
