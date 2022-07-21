#pragma once
#include <tree_editor/common/graph/multi_instance_window.h>
#include <tree_editor/common/graph/tree_instance.h>
#include <tree_editor/common/choice_manager.h>
#include <tree_editor/common/dialogs/line_dialog.h>
#include <qfiledialog.h>
#include <deque>
#include <fstream>
#include <streambuf>
#include <any_container/decode.h>

using namespace std;
using namespace spiritsaway::tree_editor;

multi_instance_window::multi_instance_window(QWidget* parent)
	: QMainWindow(parent)
	, m_logger(spiritsaway::tree_editor::logger_mgr::instance().create_logger("btree"))
{

}
multi_instance_window::~multi_instance_window()
{
	return;
}


bool multi_instance_window::load_config()
{
	
	auto config_file_path = std::filesystem::path("./config.json");
	std::string notify_info;
	QMessageBox::about(this, QString("Error"),
		QString::fromStdString("no content checker for config file"));
	return false;

}
std::variant<std::string, json::object_t> multi_instance_window::load_json_file(const std::string& file_path) const
{
	std::ifstream config_file(file_path);
	std::string file_content((std::istreambuf_iterator<char>(config_file)), std::istreambuf_iterator<char>());
	if (!json::accept(file_content))
	{
		return "config file should be json";
		
	}
	auto json_content = json::parse(file_content);
	if (!json_content.is_object())
	{
		return "config content should be json dict";
		
	}
	return json_content.get<json::object_t>();
}
void multi_instance_window::add_instance(tree_instance* _cur_instance)
{
	m_logger->debug("main_window add_instance {}", _cur_instance->file_name.string());
	m_instances.push_back(_cur_instance);
	m_cur_mdi->setActiveSubWindow(_cur_instance->window);
	_cur_instance->display_tree();
}
void multi_instance_window::remove_instance(tree_instance* _cur_instance)
{
	m_logger->debug("main_window remove_instance {}", _cur_instance->file_name.string());
	std::size_t i = 0;
	for (; i < m_instances.size(); i++)
	{
		if (m_instances[i] == _cur_instance)
		{
			break;
		}
	}
	_cur_instance->m_root->destroy();
	m_instances.erase(m_instances.begin() + i);
	if (m_instances.empty())
	{
		return;
	}
	if (i >= m_instances.size())
	{
		sub_window_activated(m_instances.back()->window);
	}
	else
	{
		sub_window_activated(m_instances[i]->window);
	}
}

void multi_instance_window::sub_window_activated(QMdiSubWindow* cur_win)
{
	m_logger->debug("main_window sub_window_activated");
	auto pre_instance = m_active_instance;
	if (cur_win && pre_instance && pre_instance->window == cur_win)
	{
		return;
	}
	tree_instance* next_instance = nullptr;
	for (const auto one_ins : m_instances)
	{
		if (one_ins->window == cur_win)
		{
			next_instance = one_ins;
			break;
		}
	}
	if (pre_instance)
	{
		pre_instance->deactivate_handler();
	}
	if (next_instance)
	{
		next_instance->activate_handler();
	}
	m_active_instance = next_instance;

}

std::shared_ptr<spdlog::logger> multi_instance_window::logger()
{
	return m_logger;
}
std::optional<std::size_t> multi_instance_window::already_open(const std::string& file_path) const
{
	for (std::size_t i = 0; i < m_instances.size(); i++)
	{
		//_logger->info("cur instance path {} input_path {}", _instances[i]->file_name.string(), file_path);
		if (m_instances[i]->file_path == file_path)
		{
			return i;
		}
	}
	return {};

}
void multi_instance_window::refresh()
{
	if (m_active_instance)
	{
		m_active_instance->refresh();
	}
}
QMdiSubWindow* multi_instance_window::add_sub_window(QGraphicsView* _view)
{
	return m_cur_mdi->addSubWindow(_view);
}
void multi_instance_window::closeEvent(QCloseEvent* e)
{
	action_close_all_handler();
}
void multi_instance_window::action_close_handler()
{
	m_logger->debug("main_window action_close_handler");
	auto cur_ins = m_active_instance;

	if (!cur_ins)
	{
		return;
	}
	cur_ins->refresh();
	remove_instance(cur_ins);

}
void multi_instance_window::action_close_all_handler()
{
	m_logger->debug("main_window action_close_all_handler");
	auto pre_instances = m_instances;
	for (auto one_ins : pre_instances)
	{
		remove_instance(one_ins);
	}
}
void multi_instance_window::action_find_handler()
{
	m_logger->debug("main_window action_find_handler");
	auto cur_ins = m_active_instance;

	if (!cur_ins)
	{
		return;
	}
	cur_ins->search_node();
}
void multi_instance_window::action_goto_handler()
{
	m_logger->debug("main_window action_goto_handler");
	auto cur_ins = m_active_instance;

	if (!cur_ins)
	{
		return;
	}
	auto cur_goto_dialog = new uint_dialog("goto node", 0, this);
	std::uint32_t result = cur_goto_dialog->run();
	if (!result)
	{
		return;
	 }
	cur_ins->focus_on(result);
}
void multi_instance_window::action_open_handler()
{
	auto fileName = QFileDialog::getOpenFileName(this,
		tr("Open Tree File"), QString::fromStdString(m_data_folder.string()), tr("Json File (*.json)"));
	if (!fileName.size())
	{
		std::string error_info = "empty file name";
		QMessageBox::about(this, QString("Error"),
			QString::fromStdString(error_info));
		return;
	}
	if (already_open(fileName.toStdString()))
	{
		std::string error_info = "file already open";
		QMessageBox::about(this, QString("Error"),
			QString::fromStdString(error_info));
		return;
	}
	auto open_result = action_open_impl(fileName.toStdString());
	if (std::holds_alternative<std::string>(open_result))
	{
		std::string error_info = std::get<std::string>(open_result);
		QMessageBox::about(this, QString("Error"),
			QString::fromStdString(error_info));
		return;
	}
	else
	{
		auto cur_ins = std::get<tree_instance*>(open_result);
	}
}
basic_node* multi_instance_window::create_node_from_desc(const basic_node_desc& cur_desc, basic_node* parent)
{
	auto cur_config = node_config_repo::instance().get_config(cur_desc.type);
	if (!cur_config)
	{
		return nullptr;
	}
	auto cur_node = new config_node(cur_config.value(), dynamic_cast<config_node*>(parent), cur_desc.idx);
	if (parent)
	{
		parent->add_child(cur_node);
	}
	cur_node->m_color = cur_desc.color;
	cur_node->m_is_collapsed = cur_desc.is_collpased;
	cur_node->m_comment = cur_desc.comment;
	cur_node->refresh_editable_items();
	cur_node->set_extra(json(cur_desc.extra));
	return cur_node;
}
std::variant<std::string, tree_instance*> multi_instance_window::action_open_impl(const std::string& file_name)
{
	auto tree_content_var = load_json_file(file_name);
	if (!std::holds_alternative<json::object_t>(tree_content_var))
	{
		return std::get<std::string>(tree_content_var);
	}
	auto cur_tree_content = std::get<json::object_t>(tree_content_var);
	auto nodes_info_var = load_desc_from_file_path(cur_tree_content);
	if (std::holds_alternative<std::string>(nodes_info_var))
	{
		return std::get<std::string>(nodes_info_var);
	}
	auto root_var = construct_root_from_node_descs(std::get<std::vector<basic_node_desc>>(nodes_info_var));
	if (std::holds_alternative<std::string>(root_var))
	{
		return std::get<std::string>(root_var);
	}
	std::string cur_tree_type;
	auto tree_type_iter = cur_tree_content.find("tree_type");
	if (tree_type_iter != cur_tree_content.end())
	{
		if (tree_type_iter->second.is_string())
		{
			cur_tree_type = tree_type_iter->second.get<std::string>();
		}
	}
	tree_instance* cur_tree_instance = create_instance(file_name, cur_tree_type, std::get<basic_node*>(root_var));
	return cur_tree_instance;
}

std::variant<std::string, std::vector<basic_node_desc>> multi_instance_window::load_desc_from_file_path(const json::object_t& tree_json_content)
{
	
	auto node_info_iter = tree_json_content.find("nodes");
	if (node_info_iter == tree_json_content.end())
	{
		return "tree content should has key nodes";

	}
	std::vector<basic_node_desc> nodes_info;
	if (!spiritsaway::serialize::decode(node_info_iter->second, nodes_info))
	{
		return "tree content cant decode node content";

	}
	return nodes_info;
}
std::variant<std::string, basic_node*> multi_instance_window::construct_root_from_node_descs(const std::vector<basic_node_desc>& nodes_info)
{
	basic_node_desc cur_root_desc;
	bool found_root = false;
	std::unordered_map<std::uint32_t, basic_node_desc> node_relation;
	for (auto one_desc : nodes_info)
	{
		node_relation[one_desc.idx] = one_desc;
		if (one_desc.type == "root")
		{
			cur_root_desc = one_desc;
			if (found_root)
			{
				return "tree content has multi root node";

			}
			found_root = true;
		}
	}
	if (!found_root)
	{
		return "tree content has no root node";

	}
	std::deque<std::uint32_t> idx_queue;
	idx_queue.push_back(cur_root_desc.idx);

	std::unordered_map<std::uint32_t, basic_node*> temp_nodes;
	basic_node* cur_root = nullptr;
	while (!idx_queue.empty())
	{
		auto cur_idx = idx_queue.front();
		idx_queue.pop_front();
		if (temp_nodes.find(cur_idx) != temp_nodes.end())
		{
			return "tree content has no cyclic relation";

		}
		auto cur_desc = node_relation[cur_idx];
		basic_node* cur_parent_p = nullptr;
		if (cur_desc.parent)
		{
			auto parent_idx = cur_desc.parent.value();
			auto cur_parent_iter = temp_nodes.find(parent_idx);
			if (cur_parent_iter == temp_nodes.end())
			{
				return "tree content has no cyclic relation";

			}
			cur_parent_p = cur_parent_iter->second;

		}
		auto cur_node = create_node_from_desc(cur_desc, cur_parent_p);
		if (!cur_node)
		{
			return "cant create node from desc " + cur_desc.encode().dump();
		}

		temp_nodes[cur_desc.idx] = cur_node;
		for (auto one_child : cur_desc.children)
		{
			idx_queue.push_back(one_child);
		}
		if (!cur_root)
		{
			cur_root = cur_node;
		}
	}
	if (!cur_root)
	{
		return "cant construct root";
	}
	return cur_root;

}

void multi_instance_window::try_set_tree_type(const std::string& new_tree_type)
{
	if (m_active_instance)
	{
		auto cur_err = m_active_instance->try_set_tree_type(new_tree_type);
		if (!cur_err.empty())
		{
			QMessageBox::about(this, QString("Error"),
				QString::fromStdString(cur_err));
			return;
		}
	}
}
tree_instance* multi_instance_window::create_instance(const std::string& file_path, const std::string& tree_type, basic_node* cur_root)
{
	return new tree_instance(file_path, tree_type, cur_root, this);
}