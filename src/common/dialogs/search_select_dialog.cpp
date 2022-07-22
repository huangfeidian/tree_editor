
#include <qlistview.h>

#include <tree_editor/common/dialogs/search_select_dialog.h>
#include <ui_search_select_dialog.h>


search_select_dialog::search_select_dialog(const std::vector<choice_type>& _in_choices, QWidget *parent, std::string in_title) :
    QDialog(parent),
    m_ui(new Ui::search_select_dialog),
	m_choices(_in_choices)
{
    m_ui->setupUi(this);
	this->setWindowTitle(QString::fromStdString(in_title));
	m_str_choices.reserve(m_choices.size());
	for (const auto& one_choice : m_choices)
	{
		m_str_choices.push_back(QString::fromStdString(one_choice));
	}
	//q_choices.sort(Qt::CaseInsensitive);
	m_list_model = new QStringListModel();
	m_list_model->setStringList(m_str_choices);
	m_ui->listView->setModel(m_list_model);
	connect(m_ui->lineEdit, SIGNAL(returnPressed()), this, SLOT(search()));
	connect(m_ui->listView, SIGNAL(doubleClicked(const QModelIndex&)),
		this, SLOT(double_click(const QModelIndex&)));

}

search_select_dialog::~search_select_dialog()
{
    delete m_ui;
	delete m_list_model;
}

void search_select_dialog::search()
{
	auto temp_text = m_ui->lineEdit->text();
	QStringList result_list;
	for (const auto& one_str : m_str_choices)
	{
		if (one_str.indexOf(temp_text, 0, Qt::CaseInsensitive) != -1)
		{
			result_list.push_back(one_str);
		}
	}
	m_list_model->setStringList(result_list);

}
void search_select_dialog::double_click(const QModelIndex& _index)
{
	auto the_item = m_list_model->data(_index, 0);
	if (!the_item.isValid())
	{
		return;
	}
	m_result = the_item.toString().toStdString();
	close_handler();
}
void search_select_dialog::close_handler()
{
	m_valid = true;
	close();
}
search_select_dialog::choice_type search_select_dialog::run()
{
	m_ui->lineEdit->setFocus();
	exec();
	if (!m_valid)
	{
		return choice_type();
	}
	return m_result;
}
