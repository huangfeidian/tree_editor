#include <tree_editor/common/graph/basic_node.h>
#include <tree_editor/common/dialogs/editable_item.h>
#include <tree_editor/common/choice_manager.h>

#include <any_container/decode.h>
#include <sstream>
#include <fstream>
#include <iostream>

using namespace spiritsaway::tree_editor;
using namespace spiritsaway::serialize;

json basic_node_desc::encode() const
{
	json::object_t result = json::object_t();
	result["idx"] = idx;
	result["children"] = children;
	if (parent)
	{
		result["parent"] = parent.value();
	}
	result["color"] = color;
	result["is_collapsed"] = is_collpased;
	result["extra"] = extra;
	result["type"] = type;
	result["comment"] = comment;
	return result;
}

bool basic_node_desc::decode(const json& data)
{
	auto temp_iter = data.find("idx");
	if (temp_iter == data.end())
	{
		return false;
	}
	if (!serialize::decode(*temp_iter, idx))
	{
		return false;
	}

	temp_iter = data.find("type");
	if (temp_iter == data.end())
	{
		return false;
	}
	if (!serialize::decode(*temp_iter, type))
	{
		return false;
	}

	temp_iter = data.find("children");
	if (temp_iter == data.end())
	{
		return false;
	}
	if (!serialize::decode(*temp_iter, children))
	{
		return false;
	}

	temp_iter = data.find("comment");
	if (temp_iter == data.end())
	{
		return false;
	}
	if (!serialize::decode(*temp_iter, comment))
	{
		return false;
	}

	temp_iter = data.find("color");
	if (temp_iter == data.end())
	{
		return false;
	}
	if (!serialize::decode(*temp_iter, color))
	{
		return false;
	}

	temp_iter = data.find("is_collapsed");
	if (temp_iter == data.end())
	{
		return false;
	}
	if (!serialize::decode(*temp_iter, is_collpased))
	{
		return false;
	}

	temp_iter = data.find("extra");
	if (temp_iter == data.end())
	{
		return false;
	}
	if (!temp_iter->is_object())
	{
		return false;
	}
	for (auto k : temp_iter->items())
	{
		extra[k.key()] = k.value();
	}
	
	temp_iter = data.find("parent");
	std::uint32_t temp_parent;
	if (temp_iter != data.end())
	{
		if (!serialize::decode(*temp_iter, temp_parent))
		{
			return false;
		}
		parent = temp_parent;
	}
	return true;
}

bool basic_node::set_extra(const json::object_t& extra)
{
	return false;
}
std::string basic_node::check_valid() const
{
	auto max_size = max_child_num();
	auto min_size = min_child_num();
	if (_children.size() > max_size || _children.size() < min_size)
	{
		std::string info = "basic_node " + std::to_string(_idx) + " has " + std::to_string(_children.size()) + " children while min " + std::to_string(min_size) + " max " + std::to_string(max_size);
		return info;
	}
	auto editable_valid = _show_widget->input_valid();
	if (!editable_valid.empty())
	{
		return "basic_node " + std::to_string(_idx) + " editable invalid for " + editable_valid;
	}
	for (const auto i : _children)
	{
		auto temp_result = i->check_valid();
		if (temp_result.size())
		{
			return temp_result;
		}
	}
	return "";
}
bool basic_node::check_item_edit_refresh(std::shared_ptr<editable_item> change_item)
{
	check_edit();
	return false;
}
void basic_node::add_child(basic_node* in_child)
{
	_children.push_back(in_child);
	in_child->_parent = this;
	auto pre_childrens = in_child->_children;
	std::deque<basic_node*> all_basic_nodes;
	all_basic_nodes.push_back(in_child);
	while (!all_basic_nodes.empty())
	{
		auto cur_basic_node = all_basic_nodes.front();
		all_basic_nodes.pop_front();
		cur_basic_node->refresh_editable_items();
		for (auto one_child : cur_basic_node->_children)
		{
			all_basic_nodes.push_back(one_child);
		}
	}

}
bool basic_node::can_collapse() const
{
	return max_child_num() > 0;

}

std::optional<std::uint32_t> basic_node::index_of_child(const basic_node* in_child) const
{
	for (std::size_t i = 0; i < _children.size(); i++)
	{
		if (_children[i] == in_child)
		{
			return i;
		}
	}
	return std::nullopt;
}

void basic_node::remove_child(basic_node* in_child)
{
	auto idx_opt = index_of_child(in_child);
	if (!idx_opt)
	{
		return;
	}
	_children.erase(_children.begin() + idx_opt.value());
	in_child->_parent = nullptr;

}
void basic_node::move_child(basic_node* in_child, bool is_up)
{
	auto idx_opt = index_of_child(in_child);
	if (!idx_opt)
	{
		return;
	}
	auto idx = idx_opt.value();
	if (is_up)
	{
		if (idx == 0)
		{
			return;
		}
		std::swap(_children[idx], _children[idx - 1]);
	}
	else
	{
		if (idx == _children.size() - 1)
		{
			return;
		}
		std::swap(_children[idx], _children[idx + 1]);
	}


}

basic_node* basic_node::get_root() const
{
	auto p = _parent;
	while (p)
	{
		p = p->_parent;
	}
	return p;
}
void basic_node::refresh_editable_items()
{

}

basic_node::basic_node(std::string _in_type, basic_node* _in_parent, std::uint32_t _in_idx) :
	_type(_in_type),
	_parent(_in_parent),
	_idx(_in_idx),
	_show_widget(std::make_shared<struct_items>("")),
	color(0xffffffff)

{

}
json basic_node::to_json() const
{
	basic_node_desc cur_desc;
	cur_desc.idx = _idx;
	cur_desc.comment = comment;
	cur_desc.color = color;
	cur_desc.type = _type;
	for (const auto i : _children)
	{
		cur_desc.children.push_back(i->_idx);
	}
	if (_parent)
	{
		cur_desc.parent = _parent->_idx;
	}

	return cur_desc.encode();
}
std::string basic_node::display_text() const
{
	return std::to_string(_idx) + ":" + _type + ":" +  comment;
}
void basic_node::destroy()
{
	for (auto one_child : _children)
	{
		one_child->destroy();
	}
	delete this;
}

basic_node* basic_node::clone_recursive(basic_node* _in_parent) const
{
	auto new_node = clone_self(_in_parent);
	for (auto one_child : _children)
	{
		auto temp_child = one_child->clone_recursive(new_node);
		temp_child->comment = one_child->comment;
		temp_child->color = one_child->color;
		temp_child->_is_collapsed = one_child->_is_collapsed;
		new_node->add_child(temp_child);
	}
	return new_node;
}

bool basic_node::check_edit()
{
	if (!_show_widget)
	{
		return false;
	}
	return true;
}
basic_node::~basic_node()
{

}

node_config_repo::node_config_repo()
{

}
bool node_config_repo::add_config(const node_config& _config)
{
	auto cur_iter = configures.find(_config.node_type_name);
	if (cur_iter != configures.end())
	{
		return false;
	}
	configures[_config.node_type_name] = _config;
	return true;
}
void node_config_repo::load_config(const json& _config)
{
	if (!_config.is_object())
	{
		return;
	}
	std::vector<std::string> node_names;
	std::vector<std::string> node_comments;
	for (const auto& one_item : _config.items())
	{
		std::string key = one_item.key();
		const auto& value = one_item.value();
		std::string comment;
		auto comment_iter = value.find("comment");
		if (comment_iter != value.end() && comment_iter->is_string())
		{
			comment = comment_iter->get<std::string>();
		}
		auto child_min_iter = value.find("child_min");
		if (child_min_iter == value.end())
		{
			continue;
		}
		if (!child_min_iter->is_number_unsigned())
		{
			continue;
		}
		std::uint32_t cur_child_min = child_min_iter->get<std::uint32_t>();

		auto child_max_iter = value.find("child_max");
		if (child_max_iter == value.end())
		{
			continue;
		}
		if (!child_max_iter->is_number_unsigned())
		{
			continue;
		}
		std::uint32_t cur_child_max = child_max_iter->get<std::uint32_t>();

		auto editable_iter = value.find("editable_item");
		if (editable_iter == value.end())
		{
			continue;
		}
		if (!editable_iter->is_object())
		{
			continue;
		}
		json editable_info = *editable_iter;
		node_config cur_config;
		cur_config.node_type_name = key;
		cur_config.max_child_num = cur_child_max;
		cur_config.min_child_num = cur_child_min;
		cur_config.comment = comment;
		cur_config.editable_info = editable_item::from_json(editable_info);
		add_config(cur_config);
		if (key != "root")
		{
			// root node cant be create manually
			node_names.push_back(key);
			node_comments.push_back(comment);
		}
		
	}
	choice_manager::instance().add_choice("node_type", node_names, node_comments);
}
std::optional<node_config> node_config_repo::get_config(const std::string& key) const
{
	auto cur_iter = configures.find(key);
	if (cur_iter == configures.end())
	{
		return {};
	}
	else
	{
		return cur_iter->second;
	}
}

config_node::config_node(const node_config& _in_config, config_node* _in_parent, std::uint32_t _in_idx)
	:basic_node(_in_config.node_type_name, _in_parent, _in_idx)
	, _config(_in_config)
{
	if (_config.editable_info)
	{
		auto cur_child = _config.editable_info->clone();
		_show_widget->_children.push_back(cur_child);
	}
	
}


basic_node* config_node::create_node(std::string _type, basic_node* _parent, std::uint32_t _idx)
{
	auto cur_config = node_config_repo::instance().get_config(_type);
	if (!cur_config)
	{
		return nullptr;
	}
	return new config_node(cur_config.value(), reinterpret_cast<config_node*>(_parent), _idx);
}
std::size_t config_node::max_child_num() const
{
	return _config.max_child_num;
}
std::size_t config_node::min_child_num() const
{
	return _config.min_child_num;
}
config_node::~config_node()
{
	return;
}
basic_node* config_node::clone_self(basic_node* _parent)const
{
	auto new_node = new config_node(_config, reinterpret_cast<config_node*>(_parent), 0);
	new_node->_show_widget = std::dynamic_pointer_cast<struct_items>(_show_widget->clone());
	return new_node;
}
