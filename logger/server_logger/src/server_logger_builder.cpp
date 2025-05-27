#include "../include/server_logger_builder.h"

#include <not_implemented.h>

#include <fstream>

logger_builder &server_logger_builder::add_file_stream(std::string const &stream_file_path,
                                                       logger::severity severity) & {
	_output_streams[severity].emplace_back(std::pair(stream_file_path, false));
	return *this;
}

logger_builder &server_logger_builder::add_console_stream(logger::severity severity) & {
	_output_streams[severity].emplace_back("", true);
	return *this;
}

logger_builder &server_logger_builder::transform_with_configuration(std::string const &configuration_file_path,
                                                                    std::string const &configuration_path) & {
	std::ifstream config_file(configuration_file_path);
	if (!config_file.is_open()) return *this;

	try {
		json config = json::parse(config_file);
		json logs_config = config[json::json_pointer(configuration_path)];

		if (logs_config.contains("format")) {
			_format = logs_config["format"].get<std::string>();
		}

		if (logs_config.contains("destination")) {
			_destination = logs_config["destination"].get<std::string>();
		}

		if (logs_config.contains("streams")) {
			const std::array<logger::severity, 6> all_severities = {
			    logger::severity::trace,   logger::severity::debug, logger::severity::information,
			    logger::severity::warning, logger::severity::error, logger::severity::critical};

			for (auto const &stream : logs_config["streams"]) {
				std::string type = stream["type"].get<std::string>();
				std::unordered_set<std::string> severities;
				for (auto const &sev : stream["severities"]) {
					severities.insert(sev.get<std::string>());
				}

				for (auto const enum_sev : all_severities) {
					std::string str_sev = logger::severity_to_string(enum_sev);

					std::transform(str_sev.begin(), str_sev.end(), str_sev.begin(), ::toupper);

					if (severities.count(str_sev)) {
						if (type == "file") {
							add_file_stream(stream["path"].get<std::string>(), enum_sev);
						} else if (type == "console") {
							add_console_stream(enum_sev);
						}
					}
				}
			}
		}
	} catch (json::exception const &) {
	}

	return *this;
}

logger_builder &server_logger_builder::clear() & {
	_output_streams.clear();
	_destination = "http://127.0.0.1:9200";
	_format = "%d %t %s %m";
	return *this;
}

logger *server_logger_builder::build() const { return new server_logger(_destination, _output_streams, _format); }

logger_builder &server_logger_builder::set_destination(const std::string &dest) & {
	_destination = dest;
	return *this;
}

logger_builder &server_logger_builder::set_format(const std::string &format) & {
	_format = format;
	return *this;
}