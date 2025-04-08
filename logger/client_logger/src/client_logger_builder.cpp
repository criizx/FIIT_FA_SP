#include "../include/client_logger_builder.h"

#include <not_implemented.h>

#include <filesystem>
#include <utility>

using namespace nlohmann;

logger_builder &client_logger_builder::add_file_stream(const std::string &stream_file_path,
                                                       logger::severity severity) & {
	_output_streams[severity].first.emplace_front(stream_file_path);
	return *this;
}

logger_builder &client_logger_builder::add_console_stream(logger::severity severity) & {
	_output_streams[severity].first.emplace_front("");
	return *this;
}

logger_builder &client_logger_builder::transform_with_configuration(const std::string &configuration_file_path,
                                                                    const std::string &configuration_path) & {
	std::ifstream file(configuration_file_path);
	if (!file.is_open()) {
		throw std::runtime_error("Failed to open configuration file: " + configuration_file_path);
	}

	json config;
	file >> config;

	json *current = &config;
	std::istringstream iss(configuration_path);
	std::string part;
	while (std::getline(iss, part, '.')) {
		if (current->contains(part)) {
			current = &(*current)[part];
		} else {
			throw std::runtime_error("Configuration path not found: " + part);
		}
	}

	if (current->contains("format")) {
		_format = current->at("format").get<std::string>();
	}

	for (auto &el : current->items()) {
		logger::severity sev;
		if (el.key() == "trace")
			sev = logger::severity::trace;
		else if (el.key() == "debug")
			sev = logger::severity::debug;
		else if (el.key() == "information")
			sev = logger::severity::information;
		else if (el.key() == "warning")
			sev = logger::severity::warning;
		else if (el.key() == "error")
			sev = logger::severity::error;
		else if (el.key() == "critical")
			sev = logger::severity::critical;
		else
			continue;

		parse_severity(sev, el.value());
	}

	return *this;
}

logger_builder &client_logger_builder::clear() & {
	_output_streams.clear();
	_format = "%m";
	return *this;
}

logger *client_logger_builder::build() const { return new client_logger(_output_streams, _format); }

logger_builder &client_logger_builder::set_format(const std::string &format) & {
	_format = format;
	return *this;
}

void client_logger_builder::parse_severity(logger::severity sev, nlohmann::json &j) {
	if (j.contains("files")) {
		for (auto &file : j["files"]) {
			add_file_stream(file.get<std::string>(), sev);
		}
	}
	if (j.contains("console") && j["console"].get<bool>()) {
		add_console_stream(sev);
	}
}

logger_builder &client_logger_builder::set_destination(const std::string &format) & {
	_format = format;
	return *this;
}
