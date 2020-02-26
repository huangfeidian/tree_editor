#pragma once

#include <nlohmann/json.hpp>
#include <QMainWindow>
#include <optional>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsItem>
#include <QPen>
#include <qmdiarea.h>
#include <qevent.h>
#include <qmdisubwindow.h>
#include <qmessagebox.h>
#include <filesystem>
#include <QCloseEvent>
#include <tree_editor/common/logger.h>
#include <http_server/http_server.h>
#include <filesystem>
#include <variant>

using json = nlohmann::json;

namespace spiritsaway::tree_editor
{
	class tree_instance;
	class basic_node;
	struct basic_node_desc;
}
namespace bt_editor = spiritsaway::tree_editor;


namespace spiritsaway::tree_editor
{

	class multi_instance_window: public QMainWindow
	{
		Q_OBJECT
	public:
		multi_instance_window(QWidget* parent = nullptr);
		virtual ~multi_instance_window();
	public:
		virtual bool is_read_only() const = 0;
		void add_instance(bt_editor::tree_instance* _cur_instance);
		void remove_instance(bt_editor::tree_instance* _cur_instance);
		std::shared_ptr<spdlog::logger> logger();
		std::optional<std::size_t> already_open(const std::string& file_path) const;
		QMdiSubWindow* add_sub_window(QGraphicsView* _view);
		void refresh();
		std::variant<std::string, json::object_t> load_json_file(const std::string& file_path) const;
	protected:
		std::shared_ptr<spdlog::logger> _logger;
		bt_editor::tree_instance* active_instance = nullptr;
		std::vector<bt_editor::tree_instance*> _instances;
		QMdiArea* cur_mdi;
		virtual bool load_config();
		virtual basic_node* create_node_from_desc(const basic_node_desc& _desc, basic_node* parent);
		std::filesystem::path data_folder;
	public slots:
		void sub_window_activated(QMdiSubWindow* cur_window);
		void closeEvent(QCloseEvent* e);
		void action_close_handler();
		void action_close_all_handler();
		void action_find_handler();
		void action_goto_handler();
		void action_open_handler();
	protected:
		virtual std::string action_open_impl();

	};
}