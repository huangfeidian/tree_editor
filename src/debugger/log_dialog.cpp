﻿#include <qfile.h>
#include <chrono>
#include <ctime>
#include <qmenu.h>

#include <any_container/encode.h>

#include <tree_editor/common/dialogs/search_select_dialog.h>
#include <sstream>
#include <tree_editor/common/dialogs/editable_item.h>

#include <tree_editor/debugger/log_dialog.h>
#include <tree_editor/debugger/debugger_main_window.h>
#include <magic_enum/magic_enum.hpp>
#include <tree_editor/common/debug_cmd.h>


using namespace spiritsaway::tree_editor;
namespace
{
	std::string format_timepoint(std::uint64_t milliseconds_since_epoch)
	{
		auto epoch_begin = std::chrono::time_point<std::chrono::system_clock, std::chrono::milliseconds>();
		auto cur_timepoint = epoch_begin + std::chrono::milliseconds(milliseconds_since_epoch);
		auto cur_time_t = std::chrono::system_clock::to_time_t(cur_timepoint);

		struct tm * timeinfo;
		char buffer[80];

		timeinfo = localtime(&cur_time_t);

		strftime(buffer, sizeof(buffer), "%H:%M:%S ", timeinfo);
		return std::string(buffer) + std::to_string(milliseconds_since_epoch % 1000) + "ms";
	}
}
log_dialog::log_dialog(std::deque<node_trace_cmd > & in_cmd_queue, debugger_main_window* parent)
	:QWidget(parent)
	, cmd_queue(in_cmd_queue)
	, _main_window(parent)
	, _logger(parent->logger())
	, _state_history(std::make_shared<tree_state_traces>())
{
	_view = new QTreeView(this);
	auto vboxLayout = new QVBoxLayout(this);
	vboxLayout->setSpacing(0);
	vboxLayout->setContentsMargins(0, 0, 0, 0);

	vboxLayout->addWidget(_view);
	setLayout(vboxLayout);
	std::vector<std::pair<QString, Qt::ItemFlags>> headers;
	headers.emplace_back("Timestamp", Qt::ItemFlag::NoItemFlags);
	headers.emplace_back("Pos", Qt::ItemFlag::NoItemFlags);
	headers.emplace_back("Cmd", Qt::ItemFlag::NoItemFlags);
	headers.emplace_back("Param", Qt::ItemFlag::NoItemFlags);

	_model = new log_tree_model(headers);

	_view->setModel(_model);
	_view->setColumnWidth(0, 120);
	_view->setColumnWidth(1, 30);
	_view->setContextMenuPolicy(Qt::CustomContextMenu);
	connect(_view, SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(on_view_context_menu(const QPoint &)));
	connect(_view, &QTreeView::doubleClicked, this, &log_dialog::on_view_double_clicked);
	vboxLayout->setSizeConstraint(QLayout::SetMaximumSize);
	_poll_timer = new QTimer(this);
	_poll_timer->start(poll_gap);
	connect(_poll_timer, &QTimer::timeout, this, &log_dialog::timer_poll);

}
bool log_dialog::push_cmd(node_trace_cmd one_cmd)
{
	std::vector<std::string> cmd_str;
	cmd_str.push_back(format_timepoint(one_cmd.ts));
	cmd_str.push_back(std::to_string(one_cmd.tree_idx) + ":" + std::to_string(one_cmd.node_idx));
	cmd_str.push_back(one_cmd.cmd);
	cmd_str.push_back(serialize::encode(one_cmd.detail).dump());
	std::vector<QVariant> row_data;
	for (const auto& one_data : cmd_str)
	{
		row_data.push_back(QString::fromStdString(one_data));
	}
	if (_state_history->push_cmd(one_cmd))
	{
		_model->appendRow(row_data);
	}
	else
	{
		if (_state_history->_old_states.size())
		{
			_model->appendRow(row_data, _model->index(_state_history->_old_states.size() - 1, 0));
		}
	}
	return true;
}

void log_dialog::search_content()
{
	std::vector<std::string> select_strings;
	for (std::size_t i = 0; i < _state_history->_old_states.size(); i++)
	{
		for (std::size_t j = 0; j < _state_history->_old_states[i]->_cmds.size(); j++)
		{
			const auto& one_cmd = _state_history->_old_states[i]->_cmds[j];
			std::string cmd_str;
			//cmd_str += std::to_string(i) + " " + std::to_string(j) + " ";
			cmd_str += format_timepoint(one_cmd.ts) + " ";
			cmd_str += one_cmd.cmd + " ";
			cmd_str += serialize::encode(one_cmd.detail).dump() + " ";
			cmd_str += get_comment(i, j) + " ";
			select_strings.push_back(cmd_str);
		}
	}
	auto cur_search_dialog = new search_select_dialog(select_strings, this);
	auto result = cur_search_dialog->run();
	if (result.empty())
	{
		return;
	}
	std::size_t cur_row_idx;
	for (cur_row_idx = 0u; cur_row_idx < select_strings.size(); cur_row_idx++)
	{
		if (select_strings[cur_row_idx] == result)
		{
			break;
		}
	}
	std::uint32_t top_row, secondary_row;
	for (top_row = 0; top_row < _state_history->_old_states.size(); top_row++)
	{
		if (cur_row_idx < _state_history->_old_states[top_row]->_cmds.size())
		{
			secondary_row = cur_row_idx;
			break;
		}
		else
		{
			cur_row_idx -= _state_history->_old_states[top_row]->_cmds.size();
		}
	}
	auto cur_model_idx = get_model_idx(top_row, cur_row_idx, 0);
	_view->setCurrentIndex(cur_model_idx);

}
QModelIndex log_dialog::get_model_idx(std::size_t top_row, std::size_t secondary_row, std::size_t column) const
{
	QModelIndex cur_row_column_idx;
	if (secondary_row != 0)
	{
		auto cur_row_idx = _model->index(top_row, 0);
		cur_row_column_idx = _model->index(secondary_row - 1, column, cur_row_idx);

	}
	else
	{
		cur_row_column_idx = _model->index(top_row, column);
	}
	return cur_row_column_idx;
}
std::string log_dialog::get_comment(std::size_t top_row, std::size_t secondary_row) const
{
	QModelIndex cur_row_comment_idx = get_model_idx(top_row, secondary_row, 3);
	
	auto cur_comment_data = _model->data(cur_row_comment_idx, Qt::DisplayRole);
	QString comment_str = cur_comment_data.toString();
	return comment_str.toStdString();
	
}


void log_dialog::show_trees()
{
	std::vector<std::string> trees;
	const auto& all_trees = _state_history->_latest_state->tree_indexes;
	for (std::size_t i = 0; i < all_trees.size(); i++)
	{
		trees.push_back(std::to_string(i) + " " + all_trees[i]);
	}
	auto cur_search_dialog = new search_select_dialog(trees, this);
	auto result = cur_search_dialog->run();
}
void log_dialog::goto_graph(std::uint32_t top_row, std::uint32_t secondary_row)
{
	std::cout << "goto graph top row " << top_row << " second row " << secondary_row;
	const auto& cur_state = _state_history->_old_states[top_row];
	const auto& cur_cmd = cur_state->_cmds[secondary_row];
	auto opt_cmd_type = magic_enum::enum_cast<nodes_cmd>(cur_cmd.cmd);
	if (!opt_cmd_type)
	{
		return;
	}
	switch (opt_cmd_type.value())
	{
	case nodes_cmd::enter:
	case nodes_cmd::leave:
	case nodes_cmd::mutate:
	{
		auto cur_tree_idx = cur_cmd.tree_idx;
		auto cur_node_idx = cur_cmd.node_idx;
		auto tree_name = _state_history->_latest_state->tree_indexes[cur_tree_idx];
		_main_window->focus_on(tree_name, cur_node_idx);
		return;
	}
	default:
		break;
	}
}
void log_dialog::on_view_double_clicked(QModelIndex cur_idx)
{
	std::cout << "double clicked row " << cur_idx.row() << " column " << cur_idx.column() << std::endl;
	if (cur_idx.column() > 0)
	{
		return;
	}
	auto parent = cur_idx.parent();
	std::cout << "parent row is " << parent.row() << std::endl;
	std::uint32_t top_row, secondary_row;
	if (parent.isValid())
	{
		top_row = parent.row();
		secondary_row = cur_idx.row() + 1;
	}
	else
	{
		top_row = cur_idx.row();
		secondary_row = 0;
	}
	_cur_top_row = top_row;
	_cur_secondary_row = 0;
	_cur_running_state = _state_history->_old_states[top_row];
	debug_run_through(secondary_row + 1);
	debug_stop();

}
void log_dialog::on_view_context_menu(const QPoint& pos)
{
	QModelIndex index = _view->indexAt(pos);
	QMenu *menu = new QMenu(this);
	auto search_action = menu->addAction("search");
	QObject::connect(search_action, &QAction::triggered, this, &log_dialog::search_content);
	auto trees_action = menu->addAction("trees");
	QObject::connect(trees_action, &QAction::triggered, this, &log_dialog::show_trees);

	if (index.isValid())
	{
		//std::cout << "on_view_context_menu " << index.row() << " column " << index.column() << std::endl;
		auto parent = index.parent();
		//std::cout << "parent row is " << parent.row() << std::endl;
		std::uint32_t top_row, secondary_row;
		if (parent.isValid())
		{
			top_row = parent.row();
			secondary_row = index.row() + 1;
		}
		else
		{
			top_row = index.row();
			secondary_row = 0;
		}
		//std::cout << "top_row is " << top_row << " secondary_row is " << secondary_row << std::endl;
		auto cur_state = _state_history->_old_states[top_row];
		cur_state->run_cmd_to(secondary_row);
		auto bb_action = menu->addAction("memory");
		QObject::connect(bb_action, &QAction::triggered, this, [cur_state, this]()
		{
			return show_memory(cur_state);
		});
		auto front_action = menu->addAction("fronts");
		QObject::connect(front_action, &QAction::triggered, this, [cur_state, this]()
		{
			return show_fronts(cur_state);
		});
		//auto highlight_action = menu->addAction("highlight_fronts");
		//QObject::connect(highlight_action, &QAction::triggered, this, [cur_state, this]()
		//{
		//	return highlight_fronts(cur_state);
		//});
	}
	menu->exec(_view->viewport()->mapToGlobal(pos));
	
}
void log_dialog::show_fronts(const std::shared_ptr<tree_state>& cur_state)
{
	std::vector<std::string> cur_fronts_str;
	std::vector<std::pair<std::uint32_t, std::uint32_t>> temp_fronts;
	for (auto [tree_idx, node_idx] : cur_state->_current_nodes)
	{
		temp_fronts.emplace_back(tree_idx, node_idx);
		cur_fronts_str.push_back(cur_state->tree_indexes[tree_idx] + " " + std::to_string(node_idx));
	}
	auto cur_search_dialog = new search_select_dialog(cur_fronts_str, this);
	auto result = cur_search_dialog->run();
	if (result.empty())
	{
		return;
	}
	for (std::size_t i = 0; i < cur_fronts_str.size(); i++)
	{
		if (result == cur_fronts_str[i])
		{
			auto[tree_idx, node_idx] = temp_fronts[i];
			_main_window->focus_on(cur_state->tree_indexes[tree_idx], node_idx);
			return;
		}
	}

}
void log_dialog::show_memory(const std::shared_ptr<tree_state>& cur_state)
{

	std::vector<std::string> cur_memory_str;
	for (const auto & [bb_key, bb_value] : cur_state->vars())
	{
		cur_memory_str.push_back(bb_key + ": " + bb_value.dump());
	}
	auto cur_search_dialog = new search_select_dialog(cur_memory_str, this);
	auto result = cur_search_dialog->run();
}
void log_dialog::timer_poll()
{
	std::size_t max_per_round = 100;
	while (max_per_round && !cmd_queue.empty())
	{
		auto cur_cmd = cmd_queue.front();
		cmd_queue.pop_front();
		push_cmd(cur_cmd);
		max_per_round--;
	}
	// std::cout << "timer poll with max_per_round " << 5 - max_per_round<< std::endl;
	if (_cur_debug_mode == debug_mode::run_through)
	{
		debug_run_through(_main_window->debug_max_step);
	}

}
void log_dialog::increate_row_idx()
{
	if (_cur_secondary_row < _state_history->_old_states[_cur_top_row]->_cmds.size())
	{
		_cur_secondary_row++;

	}
	else
	{
		_cur_top_row++;
		_cur_secondary_row = 0;
	}
}
std::optional<node_trace_cmd> log_dialog::run_once_impl()
{
	if (_cur_top_row >= _state_history->_old_states.size())
	{
		return {};
	}
	node_trace_cmd cur_cmd;
	if (_cur_secondary_row < _state_history->_old_states[_cur_top_row]->_cmds.size())
	{
		cur_cmd = _state_history->_old_states[_cur_top_row]->_cmds[_cur_secondary_row];

	}
	else
	{
		_cur_top_row++;
		_cur_secondary_row = 0;
		if (_cur_top_row >= _state_history->_old_states.size())
		{
			return {};
		}
		if (_cur_secondary_row >= _state_history->_old_states[_cur_top_row]->_cmds.size())
		{
			return {};
		}
		cur_cmd = _state_history->_old_states[_cur_top_row]->_cmds[_cur_secondary_row];
	}
	_cur_running_state->run_one_cmd(cur_cmd);
	return cur_cmd;
}
void log_dialog::debug_run_once()
{
	_cur_debug_mode = debug_mode::run_once;
	auto cur_cmd_opt = run_once_impl();
	if (!cur_cmd_opt)
	{
		return;
	}
	auto cur_cmd = cur_cmd_opt.value();
	auto opt_cmd_type = magic_enum::enum_cast<nodes_cmd>(cur_cmd.cmd);
	if (opt_cmd_type)
	{
		switch (opt_cmd_type.value())
		{
		case nodes_cmd::enter:
		case nodes_cmd::mutate:
		{
			auto cur_tree_idx = cur_cmd.tree_idx;
			auto cur_node_idx = cur_cmd.node_idx;
			auto tree_name = _state_history->_latest_state->tree_indexes[cur_tree_idx];
			_main_window->focus_on(tree_name, cur_node_idx);
			highlight_fronts(_cur_running_state);
			break;
		}
		case nodes_cmd::leave:
		{
			highlight_fronts(_cur_running_state);
			break;
		}
		default:
			break;
		}
	}
	increate_row_idx();
	auto cur_model_idx = get_model_idx(_cur_top_row, _cur_secondary_row, 0);
	_view->setCurrentIndex(cur_model_idx);
}
void log_dialog::debug_run_through(std::uint32_t max_step)
{
	_logger->info("debug_run_through {}", max_step);
	_cur_debug_mode = debug_mode::run_through;
	std::size_t cur_step = 0;
	std::uint32_t cur_tree_idx = 0;
	std::uint32_t cur_node_idx = 0;
	while (cur_step < max_step)
	{
		cur_step++;
		auto cur_cmd_opt = run_once_impl();
		if (!cur_cmd_opt)
		{
			_logger->debug("debug run through break 1");
			break;
		}
		increate_row_idx();
		auto cur_cmd = cur_cmd_opt.value();
		if (cur_cmd.cmd == magic_enum::enum_name(nodes_cmd::enter) || cur_cmd.cmd == magic_enum::enum_name(nodes_cmd::mutate))
		{
			cur_tree_idx = cur_cmd.tree_idx;

			cur_node_idx = cur_cmd.node_idx;
			// breakpoint
			if (_main_window->node_has_breakpoint(_cur_running_state->tree_indexes[cur_tree_idx], cur_node_idx))
			{
				_logger->debug("debug run through break 1");

				break;
			}
		}

		else if (cur_cmd.cmd == magic_enum::enum_name(nodes_cmd::leave))
		{
			cur_tree_idx = cur_cmd.tree_idx;
			cur_node_idx = cur_cmd.node_idx;
		}
	}
	if (cur_step == 1)
	{
		_logger->debug("debug run through break 3");

		return;
	}
	auto tree_name = _cur_running_state->tree_indexes[cur_tree_idx];
	
	_logger->info("debug_run_through focus on {}", cur_node_idx);
	highlight_fronts(_cur_running_state);
	_main_window->focus_on(tree_name, cur_node_idx);

	auto cur_model_idx = get_model_idx(_cur_top_row, _cur_secondary_row, 0);
	_view->setCurrentIndex(cur_model_idx);
}
void log_dialog::highlight_fronts(const std::shared_ptr<tree_state>& cur_state)
{
	_main_window->clear_hightlight();
	for (auto one_now_front : cur_state->_current_nodes)
	{
		_main_window->highlight_node(_state_history->_latest_state->tree_indexes[one_now_front.first], one_now_front.second, Qt::magenta);

	}
	_logger->info("pre_fronts is {} new fronts is {}", serialize::encode(_pre_fronts).dump(), serialize::encode(cur_state->_current_nodes).dump());
	_pre_fronts = cur_state->_current_nodes;
	_main_window->refresh();
	
}
void log_dialog::debug_stop()
{
	_cur_debug_mode = debug_mode::stop;
}
void log_dialog::reset()
{
	_model->resetData();
	_cur_debug_mode = debug_mode::stop;
	_cur_top_row = 0;
	_cur_secondary_row = 0;
	_cur_running_state = std::make_shared<tree_state>();
	_state_history = std::make_shared<tree_state_traces>();
	_pre_fronts.clear();
	//_main_window->refresh();
}