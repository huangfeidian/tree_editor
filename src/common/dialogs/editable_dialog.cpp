﻿#include <qboxlayout.h>
#include <qmessagebox.h>
#include <tree_editor/common/dialogs/editable_dialog.h>
#include <tree_editor/common/graph/basic_node.h>

using namespace spiritsaway::tree_editor;

editable_dialog::editable_dialog(QWidget* _in_parent, basic_node* _in_edit_node)
	:QDialog(_in_parent)
	, m_edit_node(_in_edit_node)
{
	refresh();
	
}

void editable_dialog::run()
{
	exec();

}
void editable_dialog::remove_pre_layout(QLayout* pre_layout)
{	
	//std::cout << "editable_dialog begin remove_pre_layout" << pre_layout << std::endl;
	if (!pre_layout)
	{
		return;
	}
	while (auto item = pre_layout->takeAt(0)) 
	{
		auto temp_widget = item->widget();
		if (temp_widget)
		{
			// 这里不能直接删除这个widget 因为这个函数是在widget的slot里调用的
			// 调用期间不能删除发出signal的widget 否则会导致crash
			// 所以先暂时放在widgets_to_delete里 延迟销毁
			temp_widget->hide();
			pre_layout->removeItem(item);
			m_widgets_to_delete.push_back(temp_widget);
			//delete temp_widget;
		}
		auto temp_layout = item->layout();
		remove_pre_layout(temp_layout);
		if (temp_layout)
		{
			delete temp_layout;
		}
		if (!temp_widget && !temp_layout)
		{
			delete item;
		}
	}
	//std::cout << "editable_dialog end remove_pre_layout" << pre_layout<< std::endl;
}
void editable_dialog::refresh()
{
	//std::cout << "editable_dialog begin refresh" << std::endl;
	auto pre_layout = layout();
	if (!m_widgets_to_delete.empty())
	{
		for (auto one_widget : m_widgets_to_delete)
		{
			delete one_widget;
		}
		m_widgets_to_delete.clear();
	}
	remove_pre_layout(pre_layout);
	if (pre_layout)
	{
		delete pre_layout;
	}
	QHBoxLayout *cur_layout = new QHBoxLayout;
	cur_layout->addWidget(m_edit_node->m_show_widget->to_editor([&](std::shared_ptr<editable_item> change_item)
	{
		check_edit(change_item);
		return;
	}));
	setLayout(cur_layout);
	//std::cout << "editable_dialog end refresh" << std::endl;

}
void editable_dialog::check_edit(std::shared_ptr<editable_item> change_item)
{
	if (m_edit_node->check_item_edit_refresh(change_item))
	{
		refresh();
	}
}
void editable_dialog::closeEvent(QCloseEvent* event)
{
	auto notify_info = m_edit_node->check_valid();
	if (!notify_info.empty())
	{
		QMessageBox::about(this, QString("Error"),
			QString::fromStdString(notify_info));
		event->ignore();
		return;
	}
	else
	{
		if (!m_widgets_to_delete.empty())
		{
			for (auto one_widget : m_widgets_to_delete)
			{
				delete one_widget;
			}
			m_widgets_to_delete.clear();
		}
		event->accept();
	}

}