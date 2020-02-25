#pragma once

#include <vector>
#include <string>
#include <unordered_map>
#include <unordered_set>

#include <nlohmann/json.hpp>
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
		json encode() const;
		bool decode(const json& data);
	};

	class variable_memory
	{
	protected:
		std::unordered_map<std::string, json> _data;

	public:
		bool has(const std::string& key) const;
		void set(const std::string& key, const json& value);
		bool remove(const std::string& key);
		void clear();
		void update(const json& data);
		void reset(const json& data);
		bool replay(const var_trace_cmd& _cmd_info);
		json encode() const;
		bool decode(const json& data);
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
		json encode() const;
		bool decode(const json& data);
	};
	class tree_state
	{
	public:
		std::unordered_set<std::pair<std::uint32_t, std::uint32_t>, pair_uint_hash> _current_nodes;
		variable_memory _vars;
		std::vector<node_trace_cmd> _cmds;
		std::vector<std::string> tree_indexes;

		virtual std::shared_ptr<tree_state> snapshot() const;

		virtual bool run_one_cmd(const node_trace_cmd& _cmd);
		virtual void run_cmd_to(std::uint32_t cmd_idx);

	};
	class tree_state_traces
	{
		std::vector<std::shared_ptr<tree_state>> _old_states;
		std::shared_ptr<tree_state> _latest_state;
		virtual bool push_cmd(const node_trace_cmd& _cmd);
	};
}

