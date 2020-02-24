#include <qboxlayout.h>
#include <qmessagebox.h>
#include <tree_editor/common/dialogs/info_dialog.h>
#include <tree_editor/common/dialogs/editable_item.h>

#include <tree_editor/common/graph/basic_node.h>

using namespace spiritsaway::tree_editor;

info_dialog::info_dialog(QWidget* _in_parent, basic_node* _in_edit_node)
	:QDialog(_in_parent)
	, edit_node(_in_edit_node)
{
	QHBoxLayout *cur_layout = new QHBoxLayout;
	cur_layout->addWidget(edit_node->_show_widget->to_dialog());
	setLayout(cur_layout);
}

void info_dialog::run()
{
	exec();

}

