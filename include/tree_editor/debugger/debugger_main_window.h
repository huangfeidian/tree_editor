﻿#pragma once

#include <deque>

#include <tree_editor/common/graph/multi_instance_window.h>

#include "debug_server.h"

QT_BEGIN_NAMESPACE
namespace Ui { class debugger_main_window; }
QT_END_NAMESPACE

using json = nlohmann::json;

namespace bt_editor = spiritsaway::tree_editor;

namespace spiritsaway::tree_editor
{
	class log_dialog;
	enum class debug_source
	{
		no_debug,
		file_debug,
		http_debug,
	};

	class debugger_main_window : public multi_instance_window
	{
		Q_OBJECT

	public:
		debugger_main_window(QWidget* parent = nullptr);
		~debugger_main_window();

	private:
		void init_widgets();
		void init_actions();
	protected:
		Ui::debugger_main_window *ui;
		log_dialog* _log_viewer;
		debug_source _debug_source = debug_source::no_debug;
		std::shared_ptr<http_server<debug_connection, std::deque<node_trace_cmd>>> _http_server;
		asio::io_context _asio_context;
	public:
		std::deque<node_trace_cmd> _tree_cmds;
		bool focus_on(const std::string& tree_name, std::uint32_t node_idx);
		bool node_has_breakpoint(const std::string& tree_name, std::uint32_t node_idx);
		void highlight_node(const std::string& tree_name, std::uint32_t node_idx, QColor color);
		tree_instance* ensure_file_open(const std::string& tree_name);
	private:
		void set_debug_mode(debug_mode _new_mode);
	public:
		bool is_read_only() const;
	public slots:
		void action_http_handler();
		void action_open_handler();

	};
}