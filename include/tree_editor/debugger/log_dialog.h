#pragma once
#include <QtWidgets/QTreeView>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>
#include <QDialog>
#include <qtimer.h>
#include <optional>

#include <tree_editor/common/debug_cmd.h>
#include <spdlog/logger.h>

#include <deque>
#include "log_item.h"
#include "log_tree_model.h"

namespace spiritsaway::tree_editor
{
	using namespace spiritsaway;
	class debugger_main_window;
	class log_dialog: public QWidget
	{
		Q_OBJECT
	public:
		log_dialog(std::deque<node_trace_cmd>& cmd_queue, debugger_main_window* _window);
		bool push_cmd(node_trace_cmd one_cmd);

	public slots:
		void search_content();
		void goto_graph(std::uint32_t top_row, std::uint32_t secondary_row);
		void show_trees();
		void on_view_double_clicked(QModelIndex cur_idx);
		void on_view_context_menu(const QPoint& pos);
		void show_fronts(const std::shared_ptr<tree_state>& cur_state);
		void show_memory(const std::shared_ptr<tree_state>& cur_state);
		void timer_poll();
		void highlight_fronts(const std::shared_ptr<tree_state>& cur_state);
		void debug_stop();
		void debug_run_through(std::uint32_t max_step);
		void debug_run_once();
	public:
		std::uint32_t poll_gap = 200;
		void reset();
	private:
		std::deque<node_trace_cmd>& cmd_queue;
		QTimer* _poll_timer;
		log_tree_model* _model;
		QTreeView* _view;
		debug_mode _cur_debug_mode;
		std::shared_ptr<spdlog::logger> _logger;
		debugger_main_window* _main_window;
		std::shared_ptr<tree_state> _cur_running_state;
		std::size_t _cur_top_row = 0;
		std::size_t _cur_secondary_row = 0;
		std::shared_ptr<tree_state_traces> _state_history;
		decltype(_cur_running_state->_current_nodes) _pre_fronts;
		std::string get_comment(std::size_t top_row, std::size_t secondary_row) const;
		QModelIndex get_model_idx(std::size_t top_row, std::size_t secondary_row, std::size_t column) const;
		void increate_row_idx();


		std::optional<node_trace_cmd> run_once_impl();

	};
}