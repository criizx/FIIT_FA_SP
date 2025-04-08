#include "../include/client_logger.h"

#include <not_implemented.h>

#include <algorithm>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>

std::unordered_map<std::string, std::pair<size_t, std::ofstream>> client_logger::refcounted_stream::_global_streams;

logger &client_logger::log(const std::string &text, logger::severity severity) & {
	auto it = _output_streams.find(severity);
	if (it == _output_streams.end() || !it->second.second) return *this;

	const std::string formatted_message = make_format(text, severity);

	for (auto &stream : it->second.first) {
		if (stream._stream.first.empty()) {
			std::cout << formatted_message << std::endl;
		} else {
			*stream._stream.second << formatted_message << std::endl;
		}
	}

	return *this;
}

std::string client_logger::make_format(const std::string &message, severity sev) const {
	std::string result;
	auto now = std::chrono::system_clock::now();
	std::time_t now_time = std::chrono::system_clock::to_time_t(now);
	std::tm tm = *std::gmtime(&now_time);

	for (size_t i = 0; i < _format.size(); ++i) {
		if (_format[i] == '%' && i + 1 < _format.size()) {
			char c = _format[++i];
			switch (char_to_flag(c)) {
				case flag::DATE: {
					char buffer[20];
					std::strftime(buffer, sizeof(buffer), "%Y-%m-%d", &tm);
					result += buffer;
					break;
				}
				case flag::TIME: {
					char buffer[20];
					std::strftime(buffer, sizeof(buffer), "%H:%M:%S", &tm);
					result += buffer;
					break;
				}
				case flag::SEVERITY: {
					static const char *severity_names[] = {"TRACE", "DEBUG", "INFO", "WARN", "ERROR", "CRITICAL"};
					result += severity_names[static_cast<int>(sev)];
					break;
				}
				case flag::MESSAGE:
					result += message;
					break;
				default:
					result += '%';
					result += c;
					break;
			}
		} else {
			result += _format[i];
		}
	}
	return result;
}

client_logger::client_logger(
    const std::unordered_map<logger::severity, std::pair<std::forward_list<refcounted_stream>, bool>> &streams,
    std::string format)
    : _output_streams(streams), _format(std::move(format)) {
	for (auto &[severity, stream_pair] : _output_streams) {
		for (auto &stream : stream_pair.first) {
			stream.open();
		}
	}
}

client_logger::flag client_logger::char_to_flag(char c) noexcept {
	switch (c) {
		case 'd':
			return flag::DATE;
		case 't':
			return flag::TIME;
		case 's':
			return flag::SEVERITY;
		case 'm':
			return flag::MESSAGE;
		default:
			return flag::NO_FLAG;
	}
}

client_logger::client_logger(const client_logger &other)
    : _output_streams(other._output_streams), _format(other._format) {
	for (auto &[severity, stream_pair] : _output_streams) {
		for (auto &stream : stream_pair.first) {
			stream.open();
		}
	}
}

client_logger &client_logger::operator=(const client_logger &other) {
	if (this != &other) {
		this->~client_logger();

		_output_streams = other._output_streams;
		_format = other._format;

		for (auto &[severity, stream_pair] : _output_streams) {
			for (auto &stream : stream_pair.first) {
				stream.open();
			}
		}
	}
	return *this;
}
client_logger::client_logger(client_logger &&other) noexcept
    : _output_streams(std::move(other._output_streams)), _format(std::move(other._format)) {
	other._output_streams.clear();
	other._format.clear();
}

client_logger &client_logger::operator=(client_logger &&other) noexcept {
	if (this != &other) {
		this->~client_logger();

		_output_streams = std::move(other._output_streams);
		_format = std::move(other._format);

		other._output_streams.clear();
		other._format.clear();
	}
	return *this;
}
client_logger::~client_logger() noexcept {}

client_logger::refcounted_stream::refcounted_stream(const std::string &path) : _stream(path, nullptr) { open(); }

void client_logger::refcounted_stream::open() {
	if (_stream.second == nullptr) {
		auto &entry = _global_streams[_stream.first];
		if (entry.first++ == 0) {
			entry.second.open(_stream.first, std::ios::app);
			if (!entry.second.is_open()) {
				throw std::runtime_error("log file error opening" + _stream.first);
			}
		}
		_stream.second = &entry.second;
	}
}

client_logger::refcounted_stream::refcounted_stream(const client_logger::refcounted_stream &oth)
    : _stream(oth._stream.first, oth._stream.second) {
	if (!_stream.first.empty()) {
		auto it = _global_streams.find(_stream.first);
		if (it != _global_streams.end()) {
			it->second.first++;
		}
	}
}

client_logger::refcounted_stream &client_logger::refcounted_stream::operator=(
    const client_logger::refcounted_stream &oth) {
	if (this != &oth) {
		if (!_stream.first.empty()) {
			auto it = _global_streams.find(_stream.first);
			if (it != _global_streams.end() && --it->second.first == 0) {
				it->second.second.close();
				_global_streams.erase(it);
			}
		}

		_stream = oth._stream;

		if (!_stream.first.empty()) {
			_global_streams[_stream.first].first++;
		}
	}
	return *this;
}

client_logger::refcounted_stream::refcounted_stream(client_logger::refcounted_stream &&oth) noexcept
    : _stream(std::move(oth._stream)) {
	oth._stream.second = nullptr;
}

client_logger::refcounted_stream &client_logger::refcounted_stream::operator=(
    client_logger::refcounted_stream &&oth) noexcept {
	if (this != &oth) {
		if (!_stream.first.empty()) {
			auto it = _global_streams.find(_stream.first);
			if (it != _global_streams.end() && --it->second.first == 0) {
				it->second.second.close();
				_global_streams.erase(it);
			}
		}

		_stream = std::move(oth._stream);

		oth._stream = {"", nullptr};
	}
	return *this;
}

client_logger::refcounted_stream::~refcounted_stream() {
	if (!_stream.first.empty()) {
		auto it = _global_streams.find(_stream.first);
		if (it != _global_streams.end() && --it->second.first == 0) {
			it->second.second.close();
			_global_streams.erase(it);
		}
	}
}
