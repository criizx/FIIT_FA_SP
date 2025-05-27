#include "server.h"

#include <logger_builder.h>

#include <fstream>
#include <iostream>

server::server(uint16_t port) {
	_svr.Post("/init", [this](const httplib::Request& req, httplib::Response& res) {
		auto pid = req.get_param_value("pid");
		auto sev = req.get_param_value("sev");
		auto path = req.get_param_value("path");
		auto console = req.get_param_value("console");

		std::cout << "INIT PID: " << pid << " SEVERITY: " << sev << " PATH: " << path << " CONSOLE: " << console
		          << std::endl;

		try {
			int pid_int = std::stoi(pid);
			logger::severity sev_enum = logger_builder::string_to_severity(sev);
			bool console_bool = console == "1";

			std::lock_guard<std::shared_mutex> lock(_mut);
			auto& pid_entry = _streams[pid_int];
			auto& sev_entry = pid_entry[sev_enum];

			sev_entry.emplace_back(std::make_pair(path, console_bool));
		} catch (const std::exception& e) {
			res.status = 400;
			res.body = e.what();
			return;
		}

		res.status = 204;
	});

	_svr.Post("/destroy", [this](const httplib::Request& req, httplib::Response& res) {
		auto pid = req.get_param_value("pid");
		std::cout << "DESTROY PID: " << pid << std::endl;

		try {
			int pid_int = std::stoi(pid);
			std::lock_guard<std::shared_mutex> lock(_mut);
			_streams.erase(pid_int);
		} catch (const std::exception& e) {
			res.status = 400;
			res.body = e.what();
			return;
		}

		res.status = 204;
	});

	_svr.Post("/log", [this](const httplib::Request& req, httplib::Response& res) {
		auto pid = req.get_param_value("pid");
		auto sev = req.get_param_value("sev");
		auto message = req.get_param_value("message");

		std::cout << "LOG PID: " << pid << " SEVERITY: " << sev << " MESSAGE: " << message << std::endl;

		try {
			int pid_int = std::stoi(pid);
			logger::severity sev_enum = logger_builder::string_to_severity(sev);

			std::shared_lock<std::shared_mutex> lock(_mut);
			auto pid_it = _streams.find(pid_int);
			if (pid_it != _streams.end()) {
				auto sev_it = pid_it->second.find(sev_enum);
				if (sev_it != pid_it->second.end()) {
					for (const auto& [path, console] : sev_it->second) {
						if (!path.empty()) {
							std::ofstream file(path, std::ios::app);
							if (file) file << message << '\n';
						}

						if (console) {
							std::cout << message << std::endl;
						}
					}
				}
			}
		} catch (const std::exception& e) {
			res.status = 400;
			res.body = e.what();
			return;
		}

		res.status = 204;
	});

	_svr.listen("0.0.0.0", port);
}