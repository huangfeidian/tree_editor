#pragma once
#include <deque>

#include <http_server/http_server.hpp>
#include <any_container/decode.h>
#include <tree_editor/common/debug_cmd.h>
#include <queue>
#include <iostream>

namespace spiritsaway::tree_editor
{
	
	using namespace spiritsaway::http_server;
	class debug_handler
	{
		debug_cmd_receiver _cmd_queue;

	public:
		debug_handler(const debug_cmd_receiver& in_cmd_queue)
			: _cmd_queue(in_cmd_queue)
		{

		}

		void operator()(std::weak_ptr<request> weak_req, reply_handler rep_handler)
		{
			auto req_ptr = weak_req.lock();
			if(!req_ptr)
			{
				return;
			}
			auto& req = *req_ptr;
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

				_cmd_queue(entity_id, cmds);
				break;
			}
			reply cur_reply;
			cur_reply.headers.push_back(header{ "Content-Type", "text/html" });
			cur_reply.headers.push_back(header{ "Server", "Http Server" });

			if (error_desc.size())
			{
				std::cout << "http get data fail: " << error_desc << std::endl;
				cur_reply.content = error_desc;
				cur_reply.status = reply::status_type::bad_request;
			}
			else
			{
				cur_reply.status = reply::status_type::ok;
				cur_reply.content = "ok";
			}
			rep_handler(cur_reply);
			return;
		}
	};
}