#pragma once
#include <deque>

#include <http_utils/http_server.h>
#include <any_container/decode.h>
#include <tree_editor/common/debug_cmd.h>
#include <queue>
#include <iostream>

namespace spiritsaway::tree_editor
{
	using namespace boost;
	class debug_server: public http_utils::http_server
	{
		debug_cmd_receiver m_cmd_queue;

	public:
		debug_server(asio::io_context& io_context, std::shared_ptr<spdlog::logger> in_logger, const std::string& address, const std::string& port, const debug_cmd_receiver& in_cmd_queue)
			: http_utils::http_server(io_context, in_logger, address, port)
			, m_cmd_queue(in_cmd_queue)
		{

		}

		void handle_request(const http_utils::request& req, http_utils::reply_handler rep_handler) override
		{
			
			std::string error_desc = "";
			std::string entity_id = "";
			std::vector<node_trace_cmd> cmds;
			while (true)
			{
				if (req.method != "POST" || req.uri != "/post/ai_debug/")
				{
					error_desc = "the method should be POST and  post path should be /post/ai_debug";
					break;
				}
				if (!json::accept(req.body))
				{
					error_desc = "the content should be json str";
					break;
				}
				json post_data = json::parse(req.body);
				auto entity_id_iter = post_data.find("entity_id");
				if (entity_id_iter == post_data.end())
				{
					error_desc = "entity_id should be in the data";
					break;
				}
				if (!entity_id_iter->is_string())
				{
					error_desc = "entity_id should be str";
					break;
				}
				entity_id = entity_id_iter->get<std::string>();
				auto cmd_iter = post_data.find("cmds");
				if (cmd_iter == post_data.end())
				{
					error_desc = "cmds should be in data";
					break;
				}
				if (!cmd_iter->is_array())
				{
					error_desc = "cmds should be array";
					break;
				}
				if (!spiritsaway::serialize::decode(*cmd_iter, cmds))
				{
					error_desc = "cmds format not match";
					break;
				}

				m_cmd_queue(entity_id, cmds);
				break;
			}
			http_utils::reply cur_reply;
			cur_reply.headers.push_back(http_utils::header{ "Content-Type", "text/html" });
			cur_reply.headers.push_back(http_utils::header{ "Server", "Http Server" });

			if (error_desc.size())
			{
				std::cout << "http get data fail: " << error_desc << std::endl;
				cur_reply.content = error_desc;
				cur_reply.status_code = std::uint32_t(http_utils::reply::status_type::bad_request);
			}
			else
			{
				cur_reply.status_code = std::uint32_t(http_utils::reply::status_type::ok);
				cur_reply.content = "ok";
			}
			rep_handler(cur_reply);
			return;
		}
	};
}