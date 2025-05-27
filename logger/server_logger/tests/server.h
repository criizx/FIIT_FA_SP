#ifndef MP_OS_SERVER_H
#define MP_OS_SERVER_H

#include <httplib.h>
#include <logger.h>

#include <shared_mutex>
#include <unordered_map>

class server {
	httplib::Server _svr;
	std::unordered_map<int, std::unordered_map<logger::severity, std::vector<std::pair<std::string, bool>>>> _streams;
	std::shared_mutex _mut;

   public:
	explicit server(uint16_t port = 9200);

	server(const server&) = delete;
	server& operator=(const server&) = delete;
	server(server&&) noexcept = delete;
	server& operator=(server&&) noexcept = delete;
	~server() noexcept = default;
};

#endif  // MP_OS_SERVER_H