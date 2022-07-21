
#include <qfiledialog.h>
#include <tree_editor/common/graph/tree_instance.h>
#include <tree_editor/common/choice_manager.h>
#include <tree_editor/common/dialogs/line_dialog.h>
#include <ui_editor_main_window.h>
#include <filesystem>
#include <tree_editor/common/dialogs/path_config_dialog.h>
#include <tree_editor/common/graph/basic_node.h>

#include <tree_editor/editor/editor_main_window.h>

using namespace spiritsaway::tree_editor;

editor_main_window::editor_main_window(QWidget *parent)
    : multi_instance_window(parent)
    , ui(new Ui::editor_main_window)

{
	init_widgets();
	init_actions();
	
}

void editor_main_window::init_widgets()
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
}

editor_main_window::~editor_main_window()
{
    delete ui;
}
bool editor_main_window::is_read_only() const
{
	return false;
}
void editor_main_window::init_actions()
{
	// file menu
	
	connect(ui->actionOpen, SIGNAL(triggered()), this, SLOT(action_open_handler()));
	
	connect(ui->actionClose, SIGNAL(triggered()), this, SLOT(action_close_handler()));
	connect(ui->actionCloseAll, SIGNAL(triggered()), this, SLOT(action_close_all_handler()));

	connect(ui->actionNew, SIGNAL(triggered()), this, SLOT(action_new_handler()));
	connect(ui->actionSave, SIGNAL(triggered()), this, SLOT(action_save_handler()));
	connect(ui->actionSaveAs, SIGNAL(triggered()), this, SLOT(action_save_as_handler()));
	connect(ui->actionSaveAll, SIGNAL(triggered()), this, SLOT(action_save_all_handler()));
	connect(ui->actionExport, SIGNAL(triggered()), this, SLOT(action_export_handler()));
	connect(ui->actionExportAll, SIGNAL(triggered()), this, SLOT(action_export_all_handler()));

	connect(ui->actionInsert, SIGNAL(triggered()), this, SLOT(action_insert_handler()));
	connect(ui->actionDelete, SIGNAL(triggered()), this, SLOT(action_del_handler()));
	connect(ui->actionMoveUp, SIGNAL(triggered()), this, SLOT(action_move_up_handler()));
	connect(ui->actionMoveDown, SIGNAL(triggered()), this, SLOT(action_move_down_handler()));
	connect(ui->actionCopy, SIGNAL(triggered()), this, SLOT(action_copy_handler()));
	connect(ui->actionPaste, SIGNAL(triggered()), this, SLOT(action_paste_handler()));
	connect(ui->actionCut, SIGNAL(triggered()), this, SLOT(action_cut_handler()));
	connect(ui->actionGoto, SIGNAL(triggered()), this, SLOT(action_goto_handler()));
	connect(ui->actionFind, SIGNAL(triggered()), this, SLOT(action_find_handler()));
	connect(ui->actionReIndex, SIGNAL(triggered()), this, SLOT(action_reindex_handler()));
	connect(ui->actionTreeType, SIGNAL(triggered()), this, SLOT(action_tree_type_handler()));


}


std::size_t editor_main_window::get_seq()
{
	return ++seq;
}

std::string editor_main_window::new_file_name()
{
	std::string temp = fmt::format("new_tree_{}.json", get_seq());
	while (already_open(temp))
	{
		temp = fmt::format("new_tree_{}.json", get_seq());
	}
	return temp;
}


void editor_main_window::action_new_handler()
{
	m_logger->debug("main_window action_new_handler");
	auto error_info = action_new_impl();
	if (error_info.size())
	{
		QMessageBox::about(this, QString("Error"),
			QString::fromStdString(error_info));
		return;
	}
}

void editor_main_window::action_tree_type_handler()
{
	m_logger->debug("main_window action_tree_type_handler");
	auto error_info = action_tree_type_impl();
	if (error_info.size())
	{
		QMessageBox::about(this, QString("Error"),
			QString::fromStdString(error_info));
		return;
	}
}

void editor_main_window::action_reindex_handler()
{
	auto cur_ins = m_active_instance;
	if (cur_ins)
	{
		cur_ins->reorder_index();
	}
}
void editor_main_window::action_save_handler()
{
	m_logger->debug("main_window action_save_handler");
	auto cur_window = m_cur_mdi->activeSubWindow();
	for (const auto one_tree : m_instances)
	{
		if (one_tree->window == cur_window)
		{
			auto invalid_info = one_tree->save_handler();
			if (invalid_info.empty())
			{
				one_tree->refresh();
				one_tree->set_modfied(false);
				one_tree->update_title();
			}
			return;
		}
	}
	return;
}


void editor_main_window::action_save_all_handler()
{
	m_logger->debug("main_window action_save_all_handler");
	auto cur_window = m_cur_mdi->activeSubWindow();
	for (const auto one_tree : m_instances)
	{
		auto invalid_info = one_tree->save_handler();
		if (!invalid_info.empty())
		{
			return;
		}
	}
	auto cur_ins = m_active_instance;
	if (cur_ins)
	{
		cur_ins->refresh();
		cur_ins->set_modfied(false);
		cur_ins->update_title();
	}
	return;
}

void editor_main_window::action_export_handler()
{
	m_logger->debug("main_window action_export_handler");
	auto cur_ins = m_active_instance;
	if (cur_ins)
	{
		cur_ins->export_handler();
	}
}

void editor_main_window::action_export_all_handler()
{
	m_logger->debug("main_window action_export_all_handler");
	auto cur_window = m_cur_mdi->activeSubWindow();
	for (const auto one_tree : m_instances)
	{
		auto invalid_info = one_tree->export_handler();
		if (!invalid_info.empty())
		{
			return;
		}
	}
	
}

void editor_main_window::action_save_as_handler()
{
	m_logger->debug("main_window action_save_as_handler");
	auto cur_ins = m_active_instance;

	if (!cur_ins)
	{
		return;
	}
	auto fileName = QFileDialog::getOpenFileName(this,
		tr("Save File"), QString::fromStdString(m_data_folder.string()), tr("Tree File (*.json)"));
	if (!fileName.size())
	{
		return;
	}
	auto cur_path = std::filesystem::path(fileName.toStdString());
	cur_ins->file_name = cur_path.filename();
	cur_ins->file_path = cur_path;
	cur_ins->window->setWindowTitle(QString::fromStdString(cur_ins->file_name.string()));
	auto invalid_info = cur_ins->save_handler();
	cur_ins->refresh();
}



void editor_main_window::closeEvent(QCloseEvent* ev)
{
	m_logger->debug("main_window closeEvent");
	bool modified = false;
	for (auto one_ins : m_instances)
	{
		if (one_ins->modified)
		{
			modified = true;
			break;
		}
	}
	if (modified)
	{
		QMessageBox::StandardButton  defaultBtn = QMessageBox::NoButton; //缺省按钮
		QMessageBox::StandardButton result;//返回选择的按钮
		result = QMessageBox::question(this, QString("Confirm"),
			QString("close without saving"), QMessageBox::Yes | QMessageBox::No, defaultBtn);
		if (result == QMessageBox::No)
		{
			ev->ignore();
			return;
		}
	}
	action_close_all_handler();
}
void editor_main_window::action_insert_handler()
{
	m_logger->debug("main_window action_insert_handler");
	auto cur_ins = m_active_instance;

	if (!cur_ins)
	{
		return;
	}
	cur_ins->insert_handler();
}
void editor_main_window::action_copy_handler()
{
	m_logger->debug("main_window action_copy_handler");
	auto cur_ins = m_active_instance;

	if (!cur_ins)
	{
		return;
	}
	copy_cut_slice = cur_ins->copy_handler();
}
void editor_main_window::action_del_handler()
{
	m_logger->debug("main_window action_del_handler");
	auto cur_ins = m_active_instance;

	if (!cur_ins)
	{
		return;
	}
	cur_ins->delete_handler();
}

void editor_main_window::action_move_up_handler()
{
	m_logger->debug("main_window action_move_up_handler");
	auto cur_ins = m_active_instance;

	if (!cur_ins)
	{
		return;
	}
	cur_ins->move_handler(true);
}
void editor_main_window::action_move_down_handler()
{
	m_logger->debug("main_window action_move_down_handler");
	auto cur_ins = m_active_instance;

	if (!cur_ins)
	{
		return;
	}
	cur_ins->move_handler(false);
}

void editor_main_window::action_paste_handler()
{
	m_logger->debug("main_window action_paste_handler");
	auto cur_ins = m_active_instance;

	if (!cur_ins)
	{
		return;
	}
	cur_ins->paste_handler(copy_cut_slice);
}

void editor_main_window::action_cut_handler()
{
	m_logger->debug("main_window action_cut_handler");
	auto cur_ins = m_active_instance;

	if (!cur_ins)
	{
		return;
	}
	copy_cut_slice = cur_ins->cut_handler();
}

std::string editor_main_window::action_new_impl()
{
	auto opt_root_config = node_config_repo::instance().get_config("root");
	if (!opt_root_config)
	{
		return "root node desc not present, cant create new file";

	}
	const auto& cur_root_config = opt_root_config.value();
	auto full_file = m_data_folder / new_file_name();
	basic_node_desc cur_root_desc;
	cur_root_desc.idx = 0;
	cur_root_desc.type = "root";
	cur_root_desc.color = cur_root_config.color;
	auto cur_root = create_node_from_desc(cur_root_desc, nullptr);
	if (!cur_root)
	{
		return "cant create root node";
	}
	tree_instance* cur_tree_instance = new tree_instance(full_file.string(), std::string{}, cur_root, this);
	return "";
}

std::string editor_main_window::action_tree_type_impl()
{
	return {};
}