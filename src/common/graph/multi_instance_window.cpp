﻿#include <graph/multi_instance_window.h>
#include <graph/tree_instance.h>
#include <dialogs/config_dialog.h>
#include <choice_manager.h>
#include <dialogs/line_dialog.h>
#include <qfiledialog.h>

using namespace std;
using namespace spiritsaway::tree_editor;

multi_instance_window::multi_instance_window(QWidget* parent)
	: QMainWindow(parent)
	, _logger(spiritsaway::tree_editor::logger_mgr::instance().create_logger("btree"))
{
	if (!load_config())
	{
		exit(1);
	}
}
multi_instance_window::~multi_instance_window()
{
	return;
}


bool multi_instance_window::load_config()
{
	
	auto config_file_path = std::filesystem::path("./config.json");
	std::string notify_info;
	if (!std::filesystem::exists(config_file_path))
	{
		auto cur_dialog = new config_dialog(this);
		cur_dialog->run();
		if (!cur_dialog->valid)
		{
			QMessageBox::about(this, QString("Error"),
				QString::fromStdString("invalid config"));
			return false;
		}
		
	}
	QMessageBox::about(this, QString("Error"),
		QString::fromStdString("no content checker for config file"));
	return false;

}
void multi_instance_window::add_instance(tree_instance* _cur_instance)
{
	_logger->debug("main_window add_instance {}", _cur_instance->file_name.string());
	_instances.push_back(_cur_instance);
	cur_mdi->setActiveSubWindow(_cur_instance->window);
	_cur_instance->display_tree();
}
void multi_instance_window::remove_instance(tree_instance* _cur_instance)
{
	_logger->debug("main_window remove_instance {}", _cur_instance->file_name.string());
	std::size_t i = 0;
	for (; i < _instances.size(); i++)
	{
		if (_instances[i] == _cur_instance)
		{
			break;
		}
	}
	_cur_instance->_root->destroy();
	_instances.erase(_instances.begin() + i);
	if (_instances.empty())
	{
		return;
	}
	if (i >= _instances.size())
	{
		sub_window_activated(_instances.back()->window);
	}
	else
	{
		sub_window_activated(_instances[i]->window);
	}
}

void multi_instance_window::sub_window_activated(QMdiSubWindow* cur_win)
{
	_logger->debug("main_window sub_window_activated");
	auto pre_instance = active_instance;
	if (cur_win && pre_instance && pre_instance->window == cur_win)
	{
		return;
	}
	tree_instance* next_instance = nullptr;
	for (const auto one_ins : _instances)
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
	active_instance = next_instance;

}

std::shared_ptr<spdlog::logger> multi_instance_window::logger()
{
	return _logger;
}
std::optional<std::size_t> multi_instance_window::already_open(const std::string& file_path) const
{
	for (std::size_t i = 0; i < _instances.size(); i++)
	{
		//_logger->info("cur instance path {} input_path {}", _instances[i]->file_name.string(), file_path);
		if (_instances[i]->file_path == file_path)
		{
			return i;
		}
	}
	return {};

}
void multi_instance_window::refresh()
{
	if (active_instance)
	{
		active_instance->refresh();
	}
}
QMdiSubWindow* multi_instance_window::add_sub_window(QGraphicsView* _view)
{
	return cur_mdi->addSubWindow(_view);
}
void multi_instance_window::closeEvent(QCloseEvent* e)
{
	action_close_all_handler();
}
void multi_instance_window::action_close_handler()
{
	_logger->debug("main_window action_close_handler");
	auto cur_ins = active_instance;

	if (!cur_ins)
	{
		return;
	}
	cur_ins->refresh();
	remove_instance(cur_ins);

}
void multi_instance_window::action_close_all_handler()
{
	_logger->debug("main_window action_close_all_handler");
	auto pre_instances = _instances;
	for (auto one_ins : pre_instances)
	{
		remove_instance(one_ins);
	}
}
void multi_instance_window::action_find_handler()
{
	_logger->debug("main_window action_find_handler");
	auto cur_ins = active_instance;

	if (!cur_ins)
	{
		return;
	}
	cur_ins->search_node();
}
void multi_instance_window::action_goto_handler()
{
	_logger->debug("main_window action_goto_handler");
	auto cur_ins = active_instance;

	if (!cur_ins)
	{
		return;
	}
	auto cur_goto_dialog = new line_dialog("goto node", "", this);
	auto goto_text = cur_goto_dialog->run();
	std::uint32_t result = 0;
	for (const auto i : goto_text)
	{
		if (i < '0' || i > '9')
		{
			auto notify_info = fmt::format("cant get idx from input, shoule be an interger");
			QMessageBox::about(this, QString("Error"),
				QString::fromStdString(notify_info));
			return;
		}
		auto cur_digit = i - '0';
		result = result * 10 + cur_digit;
	}
	cur_ins->focus_on(result);
}
void multi_instance_window::action_open_handler()
{
	return;
}