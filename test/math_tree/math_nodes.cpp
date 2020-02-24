#include <tree_editor/common/dialogs/editable_item.h>
#include "math_nodes.h"

using namespace spiritsaway::tree_editor;
using namespace std;

json math_node::to_json() const
{
	auto result = config_node::to_json();
	json::object_t extra;
	if (_type == "literal")
	{
		extra["value"] = _show_widget->find("value")->to_json()["value"];
	}
	else if (_type == "reduce")
	{
		extra["value"] = _show_widget->find("reduce_operator")->to_json()["value"];
	}
	result["extra"] = extra;
	return result;
}
std::string math_node::display_text() const
{
	if (_type == "literal")
	{
		return _type + ":" + std::to_string(_idx) + std::to_string(_show_widget->find("value")->_value.get<double>());
	}
	else if (_type == "reduce")
	{
		return _type + ":" + std::to_string(_idx) + _show_widget->find("reduce_operator")->_value.get<std::string>();
	}
	else
	{
		return basic_node::display_text();
	}
	
}
basic_node* math_node::clone_self(basic_node* _parent) const
{
	return config_node::clone_self(_parent);
}
math_node::math_node(const node_config& _config, math_node* _parent, std::uint32_t _idx)
	: config_node(_config, _parent, _idx)
{

}
basic_node* math_node::create_node(std::string _type, basic_node* _parent, std::uint32_t _idx)
{
	auto cur_config = node_config_repo::instance().get_config(_type);
	if (!cur_config)
	{
		return nullptr;
	}
	return new math_node(cur_config.value(), reinterpret_cast<math_node*>(_parent), _idx);
}
bool math_node::set_extra(const json::object_t& data)
{
	auto value_iter = data.find("value");
	if (value_iter == data.end())
	{
		return false;
	}
	if (_type == "literal")
	{
		if (!value_iter->second.is_number_float())
		{
			return false;
		}
		auto cur_widget = _show_widget->find("value");
		if (!cur_widget)
		{
			return false;
		}
		return cur_widget->assign(*value_iter);
	}
	else if (_type == "reduce")
	{
		if (!value_iter->second.is_string())
		{
			return false;
		}
		auto cur_widget = _show_widget->find("value");
		if (!cur_widget)
		{
			return false;
		}
		return cur_widget->assign(*value_iter);
	}
	return true;
}