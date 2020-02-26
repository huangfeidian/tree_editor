#include <tree_editor/debugger/debug_cmd.h>
#include <magic_enum.hpp>
#include <any_container/decode.h>

using namespace spiritsaway::tree_editor;

json var_trace_cmd::encode() const
{
	json::array_t result;
	result.emplace_back(std::string(magic_enum::enum_name(_cmd)));
	result.emplace_back(_detail);
	return result;
}
bool var_trace_cmd::decode(const json& data)
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
	if (!serialize::decode(data[0], cmd_name))
	{
		return false;
	}
	auto opt_cmd = magic_enum::enum_cast<var_cmd>(cmd_name);
	if (!opt_cmd)
	{
		return false;
	}
	_cmd = opt_cmd.value();
	if (!serialize::decode(data[1], _detail))
	{
		return false;
	}
	return true;
}
bool variable_memory::has(const std::string& key) const
{
	return _data.find(key) != _data.end();
}
void variable_memory::set(const std::string& key, const json& value)
{
	_data[key] = value;
}
bool variable_memory::remove(const std::string& key)
{
	auto cur_iter = _data.find(key);
	if (cur_iter == _data.end())
	{
		return false;
	}
	else
	{
		_data.erase(cur_iter);
		return true;
	}
}
void variable_memory::clear()
{
	_data.clear();
}
void variable_memory::update(const json& data)
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
void variable_memory::reset(const json& data)
{
	clear();
	update(data);
}
bool variable_memory::replay(const var_trace_cmd& _cmd_info)
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
		if (!serialize::decode(_cmd_info._detail[0], cur_key))
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
		if (!serialize::decode(_cmd_info._detail[0], cur_key))
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
json variable_memory::encode() const
{
	return serialize::encode(_data);
}
bool variable_memory::decode(const json& data)
{
	return serialize::decode(data, _data);
}
const std::unordered_map<std::string, json>& variable_memory::data() const
{
	return _data;
}

json node_trace_cmd::encode() const
{
	json::array_t result;
	result.emplace_back(tree_idx);
	result.emplace_back(node_idx);
	result.emplace_back(ts);
	result.emplace_back(cmd);
	result.emplace_back(detail);
	return result;
}

bool node_trace_cmd::decode(const json& data)
{
	if (!data.is_array())
	{
		return false;
	}
	if (data.size() != 4)
	{
		return false;
	}
	if (!serialize::decode(data[0], tree_idx))
	{
		return false;
	}
	if (!serialize::decode(data[1], node_idx))
	{
		return false;
	}
	if (!serialize::decode(data[2], ts))
	{
		return false;
	}
	if (!serialize::decode(data[3], cmd))
	{
		return false;
	}
	if (!serialize::decode(data[4], detail))
	{
		return false;
	}
	return true;
}
bool tree_state::run_one_cmd(const node_trace_cmd& _cmd)
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
		if (!serialize::decode(_cmd.detail[0], tree_indexes))
		{
			return false;
		}
		if (!serialize::decode(_cmd.detail[1], _vars))
		{
			return false;
		}
		if (!serialize::decode(_cmd.detail[2], temp_nodes))
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
		if (!serialize::decode(_cmd.detail[0], cur_tree_name))
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
		if (!serialize::decode(_cmd.detail[0], mutate_cmd))
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
			if (!serialize::decode(_cmd.detail[1], cur_var_cmd))
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
const std::unordered_map<std::string, json>& tree_state::vars() const
{
	return _vars.data();
}

void tree_state::run_cmd_to(std::uint32_t cmd_idx)
{
	for (std::size_t i = 0; i <= cmd_idx && i < _cmds.size(); i++)
	{
		run_one_cmd(_cmds[i]);
	}
}
std::shared_ptr<tree_state> tree_state::snapshot() const
{
	auto result = std::make_shared<tree_state>();
	result->_vars = _vars;
	result->_current_nodes = _current_nodes;
	result->tree_indexes = tree_indexes;
	return result;
}
bool tree_state_traces::push_cmd(const node_trace_cmd& _cmd)
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