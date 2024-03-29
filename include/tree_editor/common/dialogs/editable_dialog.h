﻿#pragma once

#include <QDialog>
#include <string>
#include <vector>
#include <utility>
#include <optional>

#include <QStringListModel>

#include <tree_editor/common/dialogs/editable_item.h>

namespace spiritsaway::tree_editor
{
	class basic_node;
	class editable_dialog :public QDialog
	{
		Q_OBJECT

	public:
		editable_dialog(QWidget* parent, basic_node* edit_node);
		void closeEvent(QCloseEvent *event);
		void run();
		void refresh();
		void check_edit(std::shared_ptr<editable_item> change_item);
	private:
		void remove_pre_layout(QLayout* pre_layout);
		basic_node* m_edit_node;
		std::vector<QWidget*> m_widgets_to_delete;
	};
}