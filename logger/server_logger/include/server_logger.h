#ifndef MATH_PRACTICE_AND_OPERATING_SYSTEMS_SERVER_LOGGER_H
#define MATH_PRACTICE_AND_OPERATING_SYSTEMS_SERVER_LOGGER_H

#include <httplib.h>
#include <logger.h>

#include <nlohmann/json.hpp>
#include <unordered_map>

using json = nlohmann::json;

class server_logger_builder;
class server_logger final : public logger {
	httplib::Client _client;
	std::string _format;
	std::string _dest;
	std::unordered_map<severity, std::vector<std::pair<std::string, bool>>> _streams;

	server_logger(const std::string &dest,
	              const std::unordered_map<logger::severity, std::vector<std::pair<std::string, bool>>> &streams,
	              std::string const &format);

	friend server_logger_builder;

	std::string format_message(std::string const &message, severity severity) const;

	static int inner_getpid();

   public:
	server_logger(server_logger const &other) = delete;

	server_logger &operator=(server_logger const &other) = delete;

	server_logger(server_logger &&other) noexcept = delete;

	server_logger &operator=(server_logger &&other) noexcept = delete;

	~server_logger() noexcept final;

   public:
	[[nodiscard]] logger &log(const std::string &message, logger::severity severity) & override;
};

#endif  // MATH_PRACTICE_AND_OPERATING_SYSTEMS_SERVER_LOGGER_H