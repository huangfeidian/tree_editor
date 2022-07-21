#pragma once
#include <optional>
#include <memory>
#include <string>
#include <vector>
#include <nlohmann/json.hpp>
#include "basic_node_desc.h"
using json = nlohmann::json;

namespace spiritsaway::tree_editor
{
	class struct_items;
	class editable_item;

	class basic_node
	{
	public:
		const std::string m_type;
		std::string m_comment;

		std::uint32_t m_idx;
		basic_node* m_parent;
		std::vector<basic_node*> m_children;
		bool m_is_collapsed = false;
		bool m_has_break_point = false;
		std::uint32_t m_color;//rgba
		std::shared_ptr<struct_items> m_show_widget;
		void add_child(basic_node* in_child);
		void remove_child(basic_node* in_child);
		void move_child(basic_node* in_child, bool is_up);
		std::optional<std::uint32_t> index_of_child(const basic_node* in_child) const;
		basic_node(std::string _in_type, basic_node* _in_parent, std::uint32_t _in_idx);
		virtual std::string check_valid() const;
		virtual json to_json() const;
		virtual std::string display_text() const;
		virtual void destroy();
		basic_node* clone_recursive(basic_node* _in_parent) const;
		virtual basic_node* clone_self(basic_node* _in_parent) const = 0;

		virtual bool check_edit();
		virtual bool set_extra(const json::object_t& extra);
		const basic_node* get_root() const;
		virtual void refresh_editable_items();
		virtual std::size_t max_child_num() const = 0;
		virtual std::size_t min_child_num() const = 0;

		virtual bool can_collapse() const;

		virtual bool check_item_edit_refresh(std::shared_ptr<editable_item> change_item);
		virtual basic_node* create_node(std::string _type, basic_node* _in_parent, std::uint32_t _idx) = 0;
		virtual ~basic_node();
	};
	struct node_config
	{
		std::string node_type_name;
		std::size_t max_child_num;
		std::size_t min_child_num;
		std::string comment;
		std::uint32_t color = 0;
		std::shared_ptr<editable_item> editable_info;
		static std::unordered_map<std::string, node_config> configured_nodes;
	};
	class node_config_repo
	{
	private:
		std::unordered_map<std::string, node_config> configures;
		node_config_repo();
	public:
		bool add_config(const node_config& _config);
		void load_config(const json& _config);
		std::optional<node_config> get_config(const std::string& _node_name) const;
		static node_config_repo& instance()
		{
			static node_config_repo _instance;
			return _instance;
		}

	};

	class config_node : public basic_node
	{
	protected:
		const node_config m_config;
	public:
		config_node(const node_config& _config, config_node* _parent, std::uint32_t _idx);
		std::size_t max_child_num() const;
		std::size_t min_child_num() const;
		basic_node* create_node(std::string _type, basic_node* _parent, std::uint32_t _idx);
		basic_node* clone_self(basic_node* _parent) const;
		~config_node();
	};
}