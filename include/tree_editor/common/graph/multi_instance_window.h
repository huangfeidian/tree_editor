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

#include <filesystem>
#include <variant>
#include <tree_editor/common/graph/basic_node_desc.h>

using json = nlohmann::json;

namespace spiritsaway::tree_editor
{
	class tree_instance;
	class basic_node;

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
		std::shared_ptr<spdlog::logger> m_logger;
		bt_editor::tree_instance* m_active_instance = nullptr;
		std::vector<bt_editor::tree_instance*> m_instances;
		QMdiArea* m_cur_mdi;
		virtual bool load_config();
		virtual basic_node* create_node_from_desc(const basic_node_desc& _desc, basic_node* parent);
		std::filesystem::path m_data_folder;
	public slots:
		void sub_window_activated(QMdiSubWindow* cur_window);
		void closeEvent(QCloseEvent* e);
		virtual void action_close_handler();
		virtual void action_close_all_handler();
		virtual void action_find_handler();
		virtual void action_goto_handler();
		virtual void action_open_handler();
	protected:
		virtual std::variant<std::string, tree_instance*> action_open_impl(const std::string& file_path);
		
		virtual std::variant<std::string, std::vector<basic_node_desc>> load_desc_from_file_path(const json::object_t& tree_json_content);
		virtual std::variant<std::string, basic_node*> construct_root_from_node_descs(const std::vector<basic_node_desc>& nodes_info);
		virtual void try_set_tree_type(const std::string& new_tree_type);
		virtual tree_instance* create_instance(const std::string& file_path, const std::string& tree_type, basic_node* cur_root);
	};
}