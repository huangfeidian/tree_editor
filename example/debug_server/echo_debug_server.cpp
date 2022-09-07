#include <tree_editor/debugger/debug_server.h>
#include <iostream>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/fmt/ostr.h>
#include <spdlog/logger.h>
using namespace spiritsaway;
using namespace spiritsaway::serialize;
using namespace spiritsaway::tree_editor;
using namespace std;

std::shared_ptr<spdlog::logger> create_logger(const std::string& name)
{
	auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
	console_sink->set_level(spdlog::level::debug);
	std::string pattern = "[" + name + "] [%^%l%$] %v";
	console_sink->set_pattern(pattern);

	auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(name + ".log", true);
	file_sink->set_level(spdlog::level::trace);
	auto logger = std::make_shared<spdlog::logger>(name, spdlog::sinks_init_list{ console_sink, file_sink });
	logger->set_level(spdlog::level::trace);
	return logger;
}

int main()
{
	asio::io_context _cur_ctx;
	debug_cmd_receiver _cur_echo = [](const std::string& entity_id, const std::vector<node_trace_cmd>& _cmds)
	{
		cout << "receive data from " << entity_id << " with cmds " << encode(_cmds).dump() << endl;
	};
	auto _http_server = debug_server(_cur_ctx, create_logger("echo_debugger"), "127.0.0.1", "8090", _cur_echo);
	_http_server.run();
	_cur_ctx.run();
}