#include <tree_editor/debugger/debug_server.h>

using namespace spiritsaway::http_server;
using namespace spiritsaway::serialize;
using namespace spiritsaway::tree_editor;
using namespace std;

int main()
{
	asio::io_context _cur_ctx;
	debug_cmd_receiver _cur_echo = [](const std::string& entity_id, const std::vector<node_trace_cmd>& _cmds)
	{
		cout << "receive data from " << entity_id << " with cmds " << encode(_cmds).dump() << endl;
	};
	auto _http_server = std::make_shared <server> (_cur_ctx, "127.0.0.1", "8090", debug_handler(_cur_echo));
	_http_server->run();
	_cur_ctx.run();
}