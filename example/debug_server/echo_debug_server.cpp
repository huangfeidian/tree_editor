#include <tree_editor/debugger/debug_server.h>

using namespace spiritsaway::http;
using namespace spiritsaway::serialize;
using namespace spiritsaway::tree_editor;
using namespace std;
struct echo_receiver
{
	void operator()(const std::string& entity_id, const std::vector<node_trace_cmd>& _cmds)
	{
		cout << "receive data from " << entity_id << " with cmds " << encode(_cmds).dump() << endl;
	}
};
int main()
{
	asio::io_context _cur_ctx;
	debug_cmd_receiver _cur_echo = [](const std::string& entity_id, const std::vector<node_trace_cmd>& _cmds)
	{
		cout << "receive data from " << entity_id << " with cmds " << encode(_cmds).dump() << endl;
	};
	auto _http_server = std::make_shared< http_server<debug_connection, debug_cmd_receiver>>(_cur_ctx, "echo debug server", 8090,  &_cur_echo);
	_http_server->run();
	_cur_ctx.run();
}