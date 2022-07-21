#include <qfiledialog.h>
#include <qdockwidget.h>
#include <tree_editor/common/graph/multi_instance_window.h>
#include <tree_editor/common/choice_manager.h>
#include <tree_editor/common/dialogs/line_dialog.h>
#include <ui_debugger_main_window.h>
#include <filesystem>

#include <tree_editor/common/dialogs/path_config_dialog.h>


#include <tree_editor/debugger/debugger_main_window.h>
#include <tree_editor/debugger/log_dialog.h>
#include <tree_editor/common/graph/tree_instance.h>
#include <fstream>

using namespace spiritsaway::tree_editor;

debugger_main_window::debugger_main_window(QWidget* parent)
	: multi_instance_window(parent)
	, ui(new Ui::debugger_main_window)
{
	init_widgets();
	init_actions();
	_reciever = [this](const std::string& entity_id, const std::vector<node_trace_cmd>& _cmds)
	{
		add_cmds(entity_id, _cmds);
	};
	_asio_poll_timer = new QTimer(this);
	_asio_poll_timer->start(500);
	connect(_asio_poll_timer, SIGNAL(timeout()), this, SLOT(asio_poll()));
}

void debugger_main_window::init_widgets()
{
	ui->setupUi(this);
	m_cur_mdi = ui->mdiArea;
	m_cur_mdi->setViewMode(QMdiArea::TabbedView);
	m_cur_mdi->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
	m_cur_mdi->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
	m_cur_mdi->setTabsMovable(true);
	m_cur_mdi->setTabsClosable(true);
	m_cur_mdi->setTabShape(QTabWidget::Rounded);
	connect(m_cur_mdi, SIGNAL(subWindowActivated(QMdiSubWindow *)), this, SLOT(sub_window_activated(QMdiSubWindow*)));

	QDockWidget *cur_docker_widget = new QDockWidget(tr("log_viewer"), this);
	_log_viewer = new log_dialog(_tree_cmds, this);

	//auto temp_layout = new QVBoxLayout();
	//temp_layout->setAlignment(Qt::AlignTop);
	//temp_layout->addWidget(_log_viewer);
	cur_docker_widget->setWidget(_log_viewer);
	cur_docker_widget->setFeatures(QDockWidget::DockWidgetMovable| QDockWidget::DockWidgetFloatable);
	cur_docker_widget->setMinimumWidth(400);

	addDockWidget(Qt::RightDockWidgetArea, cur_docker_widget);
}

debugger_main_window::~debugger_main_window()
{
	delete ui;
}
bool debugger_main_window::is_read_only() const
{
	return true;
}

void debugger_main_window::init_actions()
{
	connect(ui->actionOpen, SIGNAL(triggered()), this, SLOT(action_open_handler()));
	connect(ui->actionHttp, SIGNAL(triggered()), this, SLOT(action_http_handler()));
	connect(ui->actionClose, SIGNAL(triggered()), this, SLOT(action_close_handler()));
	connect(ui->actionCloseAll, SIGNAL(triggered()), this, SLOT(action_close_all_handler()));
	connect(ui->actionGoto, SIGNAL(triggered()), this, SLOT(action_goto_handler()));
	connect(ui->actionFind, SIGNAL(triggered()), this, SLOT(action_find_handler()));
	connect(ui->actionStop, &QAction::triggered, this, [this]()
	{
		set_debug_mode(debug_mode::stop);
	});
	connect(ui->actionRunNext, &QAction::triggered, this, [this]()
	{
		set_debug_mode(debug_mode::run_once);
	});
	connect(ui->actionRunThrough, &QAction::triggered, this, [this]()
	{
		set_debug_mode(debug_mode::run_through);
	});

}
tree_instance* debugger_main_window::ensure_file_open(const std::string& tree_name)
{

	auto cur_file_path = m_data_folder / tree_name;
	std::string cur_file_path_str = cur_file_path.string();
	auto opt_ins_idx = already_open(cur_file_path_str);
	tree_instance* cur_ins;
	if (opt_ins_idx)
	{
		cur_ins = m_instances[opt_ins_idx.value()];
		m_cur_mdi->setActiveSubWindow(cur_ins->window);
		return cur_ins;
	}
	else
	{
		
		auto open_result = action_open_impl(cur_file_path_str);
		if (std::holds_alternative<tree_instance*>(open_result))
		{
			return std::get<tree_instance*>(open_result);
		}
		else
		{
			return nullptr;
		}
	}
}
bool debugger_main_window::focus_on(const std::string& tree_name, std::uint32_t node_idx)
{

	auto cur_ins = ensure_file_open(tree_name);
	if (!cur_ins)
	{
		return false;
	}
	cur_ins->clear_temp_color();
	cur_ins->focus_on(node_idx);
	return true;
}
bool debugger_main_window::node_has_breakpoint(const std::string& tree_name, std::uint32_t node_idx)
{
	tree_instance* cur_ins = ensure_file_open(tree_name);
	if (!cur_ins)
	{
		return true;
	}
	auto cur_node = cur_ins->find_node_by_idx(node_idx);
	if (!cur_node)
	{
		return true;
	}
	else
	{
		return cur_node->m_has_break_point;
	}
	
}
void debugger_main_window::highlight_node(const std::string& tree_name, std::uint32_t node_idx, QColor color)
{
	std::filesystem::path cur_file_path = m_data_folder / tree_name;
	std::string cur_file_path_str = cur_file_path.string();
	auto opt_ins_idx = already_open(cur_file_path_str);
	tree_instance* cur_ins = ensure_file_open(cur_file_path_str);
	if (!cur_ins)
	{
		return;
	}
	else
	{
		cur_ins->set_temp_color(node_idx, color);
	}

}
void debugger_main_window::clear_hightlight()
{
	for (auto one_ins : m_instances)
	{
		one_ins->clear_temp_color();
	}
}
void debugger_main_window::set_debug_mode(debug_mode _new_mode)
{
	switch (_new_mode)
	{
	case debug_mode::stop:
		_log_viewer->debug_stop();
		break;
	case debug_mode::run_through:
		_log_viewer->debug_run_through(debug_max_step);
		break;
	case debug_mode::run_once:
		_log_viewer->debug_run_once();
		break;
	default:
		break;
	}
}
void debugger_main_window::action_open_handler()
{
	if (_debug_source != debug_source::no_debug)
	{
		auto notify_info = fmt::format("already during debug, please close all previous ");
		QMessageBox::about(this, QString("Error"),
			QString::fromStdString(notify_info));
		return;
	}
	multi_instance_window::action_open_handler();
	_debug_source = debug_source::file_debug;
}

void debugger_main_window::action_http_handler()
{
	if (_debug_source != debug_source::no_debug)
	{
		auto notify_info = fmt::format("already during debug, please close all previous ");
		QMessageBox::about(this, QString("Error"),
			QString::fromStdString(notify_info));
		return;
	}
	auto cur_port_dialog = new uint_dialog("http server port", 8090, this);
	auto result = cur_port_dialog->run();
	
	if (result <= 1024 || result >= 20000)
	{
		auto notify_info = "port number shoude between 1024 and 20000";
		QMessageBox::about(this, QString("Error"),
			QString::fromStdString(notify_info));
		return;
	}
	_debug_source = debug_source::http_debug;
	_http_server = std::make_shared<http_server::server>(_asio_context, "0.0.0.0", std::to_string(result), debug_handler(_reciever));
	_http_server->run();
	return;
}
void debugger_main_window::add_cmds(const std::string& entity_id, const std::vector<node_trace_cmd>& _temp_cmds)
{
	if (entity_id != _debug_entity_id)
	{
		save_debug_file();
		reset_debug_entity(entity_id);
	}
	_total_cmds.insert(_total_cmds.end(), _temp_cmds.begin(), _temp_cmds.end());
	_tree_cmds.insert(_tree_cmds.end(), _temp_cmds.begin(), _temp_cmds.end());
}
void debugger_main_window::save_debug_file()
{
	if (_debug_entity_id.size())
	{
		json::object_t result;
		result["entity_id"] = _debug_entity_id;
		time_t rawtime;
		struct tm * timeinfo;
		char buffer[80];

		time(&rawtime);
		timeinfo = localtime(&rawtime);

		strftime(buffer, sizeof(buffer), "%d-%m-%Y-%H-%M-%S", timeinfo);

		result["ts"] = std::string(buffer);
		result["debug_cmds"] = serialize::encode(_total_cmds);
		std::string file_name = "debug_cmds_" + _debug_entity_id + "_" + std::string(buffer) + ".json";
		std::ofstream output_stream(history_folder / file_name);
		output_stream << json(result).dump(4) << std::endl;
		output_stream.close();

	}
}
void debugger_main_window::reset_debug_entity(const std::string& new_entity_id)
{
	_log_viewer->reset();
	_debug_entity_id = new_entity_id;
	_tree_cmds.clear();
	_total_cmds.clear();
}
void debugger_main_window::action_close_all_handler()
{
	multi_instance_window::action_close_all_handler();
	
	if (_debug_source == spiritsaway::tree_editor::debug_source::http_debug)
	{
		_http_server.reset();
		save_debug_file();
		
	}
	reset_debug_entity("");
	_debug_source = debug_source::no_debug;

	
}
void debugger_main_window::asio_poll()
{
	_asio_context.poll();
}
