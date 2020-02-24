#include <fstream>
#include <streambuf>

#include <qfiledialog.h>
#include <tree_editor/common/dialogs/path_config_dialog.h>
#include <tree_editor/common/choice_manager.h>
#include <tree_editor/common/graph/tree_instance.h>

#include <any_container/decode.h>

#include "math_editor_window.h"
#include "math_nodes.h"

using namespace spiritsaway::tree_editor;
using namespace std;
std::string math_editor_window::new_file_name()
{
	std::string temp = fmt::format("new_math_tree_{}.json", get_seq());
	std::filesystem::path temp_path = data_folder / temp;
	while (already_open(temp) || std::filesystem::exists(temp_path))
	{
		temp = fmt::format("new_math_tree_{}.json", get_seq());
		temp_path = data_folder / temp;
	}
	return temp;
}
bool math_editor_window::load_config()
{

	auto config_file_name = "math_node_config.json";
	std::string notify_info;
	std::string node_desc_path;
	std::string choice_desc_path;
	std::string save_path;
	if (!std::filesystem::exists(std::filesystem::path(config_file_name)))
	{
		path_req_desc node_req;
		node_req.name = "math node types";
		node_req.tips = "file to provide all math nodes";
		node_req.extension = ".json";

		path_req_desc choice_req;
		choice_req.name = "math operation choices";
		choice_req.tips = "file to provide all choices";
		choice_req.extension = ".json";

		path_req_desc save_path_req;
		save_path_req.name = "data save dir";
		save_path_req.tips = "directory to save data files";
		save_path_req.extension = "";

		std::vector<path_req_desc> path_reqs;
		path_reqs.push_back(node_req);
		path_reqs.push_back(choice_req);
		path_reqs.push_back(save_path_req);
		
		auto cur_dialog = new path_config_dialog(path_reqs, "math_node_config.json", this);
		auto temp_result = cur_dialog->run();
		if (!cur_dialog->valid)
		{
			QMessageBox::about(this, QString("Error"),
				QString::fromStdString("invalid btree config"));
			return false;
		}
		node_desc_path = temp_result[0];
		choice_desc_path = temp_result[1];
		save_path = temp_result[2];
	}
	else
	{

		std::ifstream config_file(config_file_name);
		std::string file_content((std::istreambuf_iterator<char>(config_file)), std::istreambuf_iterator<char>());
		if (!json::accept(file_content))
		{
			notify_info = "config file should be json";
			QMessageBox::about(this, QString("Error"),
				QString::fromStdString(notify_info));
			return false;
		}
		auto json_content = json::parse(file_content);
		if (!json_content.is_object())
		{
			notify_info = "config content should be json dict";
			QMessageBox::about(this, QString("Error"),
				QString::fromStdString(notify_info));
			return false;
		}
		std::vector<std::string> temp_result;
		std::vector<std::string> path_keys = { "math node types", "math operation choices" , "data save dir" };
		for (auto one_key : path_keys)
		{
			auto cur_value_iter = json_content.find(one_key);
			if (cur_value_iter == json_content.end())
			{
				notify_info = "config content should be should has key " + one_key;
				QMessageBox::about(this, QString("Error"),
					QString::fromStdString(notify_info));
				return false;
			}
			if (!cur_value_iter->is_string())
			{
				notify_info = "config content should for key " + one_key + " should be str";
				QMessageBox::about(this, QString("Error"),
					QString::fromStdString(notify_info));
				return false;
			}
			temp_result.push_back(cur_value_iter->get<std::string>());
		}
		node_desc_path = temp_result[0];
		choice_desc_path = temp_result[1];
		save_path = temp_result[2];
	}

	std::ifstream node_config_file(node_desc_path);
	std::string node_file_content((std::istreambuf_iterator<char>(node_config_file)), std::istreambuf_iterator<char>());
	if (!json::accept(node_file_content))
	{
		notify_info = "node config file should be json";
		QMessageBox::about(this, QString("Error"),
			QString::fromStdString(notify_info));
		return false;
	}
	auto node_json_content = json::parse(node_file_content);
	if (!node_json_content.is_object())
	{
		notify_info = "node config content should be json dict";
		QMessageBox::about(this, QString("Error"),
			QString::fromStdString(notify_info));
		return false;
	}
	node_config_repo::instance().load_config(node_json_content);

	std::ifstream choice_config_file(choice_desc_path);
	std::string choice_file_content((std::istreambuf_iterator<char>(choice_config_file)), std::istreambuf_iterator<char>());
	if (!json::accept(choice_file_content))
	{
		notify_info = "choice config file should be json";
		QMessageBox::about(this, QString("Error"),
			QString::fromStdString(notify_info));
		return false;
	}
	auto choice_json_content = json::parse(choice_file_content);
	if (!choice_json_content.is_object())
	{
		notify_info = "choice config content should be json dict";
		QMessageBox::about(this, QString("Error"),
			QString::fromStdString(notify_info));
		return false;
	}
	choice_manager::instance().load_from_json(choice_json_content);
	data_folder = save_path;
	return true;
}
std::string math_editor_window::action_new_impl()
{
	auto opt_root_config = node_config_repo::instance().get_config("root");
	if (!opt_root_config)
	{
		return "root node desc not present, cant create new file";

	}
	auto full_file = data_folder / new_file_name();

	auto cur_root = new math_node(opt_root_config.value(), nullptr, 0);
	cur_root->refresh_editable_items();
	tree_instance* cur_tree_instance = new tree_instance(full_file.string(), cur_root, this);
	return "";

}
std::string math_editor_window::action_open_impl()
{
	auto fileName = QFileDialog::getOpenFileName(this,
		tr("Open Math Tree"), QString::fromStdString(data_folder.string()), tr("Json File (*.json)"));
	if (!fileName.size())
	{
		return "invalid file name";
	}
	if (already_open(fileName.toStdString()))
	{
		return "file already open";
	}
	std::ifstream tree_file(fileName.toStdString());
	std::string tree_file_content((std::istreambuf_iterator<char>(tree_file)), std::istreambuf_iterator<char>());
	if (!json::accept(tree_file_content))
	{
		return "tree file should be json";
		
	}
	auto tree_json_content = json::parse(tree_file_content);
	if (!tree_json_content.is_object())
	{
		return "tree content should be json dict";
		
	}
	auto node_info_iter = tree_json_content.find("nodes");
	if (node_info_iter == tree_json_content.end())
	{
		return "tree content should has key nodes";
		
	}
	std::vector<basic_node_desc> nodes_info;
	if (!spiritsaway::serialize::decode(*node_info_iter, nodes_info))
	{
		return "tree content cant decode node content";
		
	}
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

	std::unordered_map<std::uint32_t, math_node*> temp_nodes;
	math_node* cur_root;
	while (!idx_queue.empty())
	{
		auto cur_idx = idx_queue.front();
		idx_queue.pop_front();
		if (temp_nodes.find(cur_idx) != temp_nodes.end())
		{
			return "tree content has no cyclic relation";
			
		}
		auto cur_desc = node_relation[cur_idx];
		if (cur_desc.parent)
		{
			auto parent_idx = cur_desc.parent.value();
			auto cur_parent_iter = temp_nodes.find(parent_idx);
			if (cur_parent_iter == temp_nodes.end())
			{
				return "tree content has no cyclic relation";
				
			}
			auto cur_parent = cur_parent_iter->second;
			auto cur_config = node_config_repo::instance().get_config(cur_desc.type);
			if (!cur_config)
			{
				return "invalid node type " + cur_desc.type;
			}
			auto cur_node = new math_node(cur_config.value(), cur_parent, cur_desc.idx);
			cur_node->color = cur_desc.color;
			cur_node->_is_collapsed = cur_desc.is_collpased;
			cur_node->comment = cur_desc.comment;
			temp_nodes[cur_desc.idx] = cur_node;
			for (auto one_child : cur_desc.children)
			{
				idx_queue.push_back(one_child);
			}
			if (!cur_root)
			{
				cur_root = cur_node;
			}
			cur_node->refresh_editable_items();
			cur_node->set_extra(json(cur_desc.extra));
		}
	}

	cur_root->refresh_editable_items();
	tree_instance* cur_tree_instance = new tree_instance(fileName.toStdString(), cur_root, this);
	return "";

}