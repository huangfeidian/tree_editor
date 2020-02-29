#pragma once

#include <vector>
#include <string>
#include <unordered_map>
#include <unordered_set>

#include <nlohmann/json.hpp>
#include <functional>
#include <magic_enum.hpp>
#include <any_container/decode.h>
#include <any_container/encode.h>

using json = nlohmann::json;
namespace spiritsaway::tree_editor
{
	enum class debug_mode
	{
		stop,
		run_through,
		run_once,
	};
	struct pair_uint_hash
	{
		std::size_t operator()(std::pair<std::uint32_t, std::uint32_t> data) const
		{
			return data.first ^ data.second;
		}
	};
	enum class var_cmd
	{
		clear = 0,
		set,
		remove,
		update,
		reset,
	};
	struct var_trace_cmd
	{
		var_cmd _cmd;
		json::array_t _detail;
		json encode() const
		{
			json::array_t result;
			result.emplace_back(std::string(magic_enum::enum_name(_cmd)));
			result.emplace_back(_detail);
			return result;
		}

		bool decode(const json& data)
		{
			if (!data.is_array())
			{
				return false;
			}
			if (data.size() != 2)
			{
				return false;
			}
			std::string cmd_name;
			if (!spiritsaway::serialize::decode(data[0], cmd_name))
			{
				return false;
			}
			auto opt_cmd = magic_enum::enum_cast<var_cmd>(cmd_name);
			if (!opt_cmd)
			{
				return false;
			}
			_cmd = opt_cmd.value();
			if (!spiritsaway::serialize::decode(data[1], _detail))
			{
				return false;
			}
			return true;
		}
	};

	class variable_memory
	{
	protected:
		std::unordered_map<std::string, json> _data;

	public:
		bool has(const std::string& key) const
		{
			return _data.find(key) != _data.end();
		}
		void set(const std::string& key, const json& value)
		{
			_data[key] = value;
		}
		bool remove(const std::string& key)
		{
			return _data.erase(key) == 1;
		}
		void clear()
		{
			_data.clear();
		}
		void update(const json& data)
		{
			if (!data.is_object())
			{
				return;
			}
			for (const auto& one_item : data.items())
			{
				_data[one_item.key()] = one_item.value();
			}
		}
		void reset(const json& data)
		{
			clear();
			update(data);
		}
		bool replay(const var_trace_cmd& _cmd_info)
		{
			switch (_cmd_info._cmd)
			{
			case var_cmd::clear:
			{
				clear();
				return true;
			}
			case var_cmd::set:
			{
				if (_cmd_info._detail.size() != 2)
				{
					return false;
				}
				std::string cur_key;
				if (!spiritsaway::serialize::decode(_cmd_info._detail[0], cur_key))
				{
					return false;
				}
				_data[cur_key] = _cmd_info._detail[1];
				return true;
			}
			case var_cmd::remove:
			{
				if (_cmd_info._detail.size() != 1)
				{
					return false;
				}
				std::string cur_key;
				if (!spiritsaway::serialize::decode(_cmd_info._detail[0], cur_key))
				{
					return false;
				}
				_data.erase(cur_key);
				return true;
			}
			case var_cmd::update:
			{
				if (_cmd_info._detail.size() != 1)
				{
					return false;
				}
				if (!_cmd_info._detail[0].is_object())
				{
					return false;
				}
				update(_cmd_info._detail[0]);
				return true;
			}
			case var_cmd::reset:
			{
				if (_cmd_info._detail.size() != 1)
				{
					return false;
				}
				if (!_cmd_info._detail[0].is_object())
				{
					return false;
				}
				reset(_cmd_info._detail[0]);
				return true;
			}
			default:
				return false;
			}
		}
		json encode() const
		{
			return spiritsaway::serialize::encode(_data);
		}
		bool decode(const json& data)
		{
			return spiritsaway::serialize::decode(data, _data);
		}
		const std::unordered_map<std::string, json>& data() const
		{
			return _data;
		}
	};
	
	enum class nodes_cmd
	{
		snapshot = 0,
		push_tree,
		enter,
		leave,
		mutate,//change internal info of one node
	};
	//first is mutate cmd second is mutate detail
	struct node_trace_cmd
	{
		std::uint32_t tree_idx;
		std::uint32_t node_idx;
		std::uint64_t ts;
		std::string cmd;
		json::array_t detail;
		json encode() const
		{
			json::array_t result;
			result.emplace_back(tree_idx);
			result.emplace_back(node_idx);
			result.emplace_back(ts);
			result.emplace_back(cmd);
			result.emplace_back(detail);
			return result;
		}
		bool decode(const json& data)
		{
			if (!data.is_array())
			{
				return false;
			}
			if (data.size() != 5)
			{
				return false;
			}
			if (!spiritsaway::serialize::decode(data[0], tree_idx))
			{
				return false;
			}
			if (!spiritsaway::serialize::decode(data[1], node_idx))
			{
				return false;
			}
			if (!spiritsaway::serialize::decode(data[2], ts))
			{
				return false;
			}
			if (!spiritsaway::serialize::decode(data[3], cmd))
			{
				return false;
			}
			if (!spiritsaway::serialize::decode(data[4], detail))
			{
				return false;
			}
			return true;
		}
	};
	class tree_state
	{
	public:
		std::unordered_set<std::pair<std::uint32_t, std::uint32_t>, pair_uint_hash> _current_nodes;
		variable_memory _vars;
		std::vector<node_trace_cmd> _cmds;
		std::vector<std::string> tree_indexes;

		virtual std::shared_ptr<tree_state> snapshot() const
		{
			auto result = std::make_shared<tree_state>();
			result->_vars = _vars;
			result->_current_nodes = _current_nodes;
			result->tree_indexes = tree_indexes;
			return result;
		}

		virtual bool run_one_cmd(const node_trace_cmd& _cmd)
		{
			auto opt_cmd_type = magic_enum::enum_cast<nodes_cmd>(_cmd.cmd);
			if (!opt_cmd_type)
			{
				return false;
			}
			switch (opt_cmd_type.value())
			{
			case nodes_cmd::snapshot:
			{
				// trees memory current_nodes
				if (_cmd.detail.size() != 3)
				{
					return false;
				}
				_current_nodes.clear();
				_vars.clear();
				tree_indexes.clear();
				std::vector<std::pair<std::uint32_t, std::uint32_t>> temp_nodes;
				if (!spiritsaway::serialize::decode(_cmd.detail[0], tree_indexes))
				{
					return false;
				}
				if (!spiritsaway::serialize::decode(_cmd.detail[1], _vars))
				{
					return false;
				}
				if (!spiritsaway::serialize::decode(_cmd.detail[2], temp_nodes))
				{
					return false;
				}
				for (auto one_node : temp_nodes)
				{
					_current_nodes.insert(one_node);
				}
				return true;
			}
			case nodes_cmd::enter:
			{
				_current_nodes.insert(std::make_pair(_cmd.tree_idx, _cmd.node_idx));
				return true;
			}
			case nodes_cmd::leave:
			{
				_current_nodes.erase(std::make_pair(_cmd.tree_idx, _cmd.node_idx));
				return true;
			}
			case nodes_cmd::push_tree:
			{
				if (_cmd.detail.size() != 1)
				{
					return false;
				}
				std::string cur_tree_name;
				if (!spiritsaway::serialize::decode(_cmd.detail[0], cur_tree_name))
				{
					return false;
				}
				tree_indexes.push_back(cur_tree_name);
				return true;
			}
			case nodes_cmd::mutate:
			{
				if (_cmd.detail.size() == 0)
				{
					return false;
				}
				std::string mutate_cmd;
				if (!spiritsaway::serialize::decode(_cmd.detail[0], mutate_cmd))
				{
					return false;
				}
				if (mutate_cmd == "variable")
				{
					if (_cmd.detail.size() != 2)
					{
						return false;
					}
					var_trace_cmd cur_var_cmd;
					if (!spiritsaway::serialize::decode(_cmd.detail[1], cur_var_cmd))
					{
						return false;
					}
					return _vars.replay(cur_var_cmd);
				}
				return false;
			}
			default:
				return false;
			}
		}
		virtual void run_cmd_to(std::uint32_t cmd_idx)
		{
			for (std::size_t i = 0; i <= cmd_idx && i < _cmds.size(); i++)
			{
				run_one_cmd(_cmds[i]);
			}
		}
		const std::unordered_map<std::string, json>& vars() const
		{
			return _vars.data();
		}

	};
	class tree_state_traces
	{
	public:
		std::vector<std::shared_ptr<tree_state>> _old_states;
		std::shared_ptr<tree_state> _latest_state;
		tree_state_traces()
			: _latest_state(std::make_shared< tree_state>())
		{

		}
		virtual bool push_cmd(const node_trace_cmd& _cmd)
		{
			auto opt_cmd_type = magic_enum::enum_cast<nodes_cmd>(_cmd.cmd);
			if (!opt_cmd_type)
			{
				_latest_state->run_one_cmd(_cmd);
				_old_states.back()->run_one_cmd(_cmd);
				return false;
			}
			else
			{
				auto cur_cmd_type = opt_cmd_type.value();
				switch (cur_cmd_type)
				{
				case spiritsaway::tree_editor::nodes_cmd::snapshot:
				{
					auto new_state = _latest_state->snapshot();
					new_state->_cmds.push_back(_cmd);
					_old_states.push_back(new_state);
					_latest_state->run_one_cmd(_cmd);
					return true;
				}
				default:
				{
					_latest_state->run_one_cmd(_cmd);
					_old_states.back()->run_one_cmd(_cmd);
					return false;
				}

				}
			}
		}
	};
	using debug_cmd_receiver = std::function<void(const std::string&, const std::vector<node_trace_cmd>&)>;

}

