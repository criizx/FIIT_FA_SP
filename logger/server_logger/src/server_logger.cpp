#include <not_implemented.h>
// #include <httplib.h>
#include "../include/server_logger.h"

#ifdef _WIN32
#include <process.h>
#else
#include <unistd.h>
#endif

server_logger::~server_logger() noexcept { _client.stop(); }

logger &server_logger::log(const std::string &message, logger::severity severity) & {
	auto it = _streams.find(severity);
	if (it == _streams.end()) return *this;

	for (const auto &[path, console] : it->second) {
		std::string formatted = format_message(message, severity);

		httplib::Params params;
		params.emplace("pid", std::to_string(inner_getpid()));
		params.emplace("sev", logger::severity_to_string(severity));
		params.emplace("message", formatted);

		_client.Post("/log", params);
	}

	return *this;
}

server_logger::server_logger(
    const std::string &dest,
    const std::unordered_map<logger::severity, std::vector<std::pair<std::string, bool>>> &streams,
    std::string const &format)
    : _client(dest), _streams(streams), _format(format) {
	int pid = inner_getpid();
	for (const auto &[sev, streams_list] : _streams) {
		for (const auto &stream : streams_list) {
			httplib::Params init_params;
			init_params.emplace("pid", std::to_string(pid));
			init_params.emplace("sev", logger::severity_to_string(sev));
			init_params.emplace("path", stream.first);
			init_params.emplace("console", stream.second ? "1" : "0");

			_client.Post("/init", init_params);
		}
	}
}

int server_logger::inner_getpid() {
#ifdef _WIN32
	return ::_getpid();
#else
	return getpid();
#endif
}

std::string server_logger::format_message(std::string const &message, logger::severity severity) const {
	std::time_t now = std::time(nullptr);
	std::tm *gmt = std::gmtime(&now);
	std::ostringstream ss;

	for (size_t i = 0; i < _format.size(); ++i) {
		if (_format[i] == '%' && i + 1 < _format.size()) {
			switch (_format[++i]) {
				case 'd':
					ss << std::put_time(gmt, "%Y-%m-%d");
					break;
				case 't':
					ss << std::put_time(gmt, "%H:%M:%S");
					break;
				case 's':
					ss << logger::severity_to_string(severity);
					break;
				case 'm':
					ss << message;
					break;
				default:
					ss << '%' << _format[i];
			}
		} else {
			ss << _format[i];
		}
	}

	return ss.str();
}