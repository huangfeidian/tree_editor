﻿#include <qfiledialog.h>
#include <qmessagebox.h>
#include <qevent.h>
#include <qformlayout.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <nlohmann/json.hpp>
#include <fstream>
#include <spdlog/fmt/fmt.h>

#include <tree_editor/common/dialogs/path_config_dialog.h>
#include <spdlog/fmt/fmt.h>

using namespace std;
using namespace spiritsaway::tree_editor;
using json = nlohmann::json;

path_config_dialog::path_config_dialog(const std::vector<path_req_desc>& _in_path_req, const std::string& _in_dest_file_name, QWidget* parent)
	:QDialog(parent)
	, m_path_req(_in_path_req)
	, m_dest_file_name(_in_dest_file_name)

{
	QFormLayout *formLayout = new QFormLayout;
	for (std::size_t i = 0; i< m_path_req.size(); i++)
	{
		auto cur_label = new QLabel("");
		cur_label->setMinimumWidth(80);
		cur_label->sizePolicy().setHorizontalStretch(2);
		cur_label->sizePolicy().setHorizontalPolicy(QSizePolicy::Maximum);
		auto cur_button = new QPushButton("Select");
		auto cur_layout = new QHBoxLayout();
		cur_layout->addWidget(cur_label, 2);
		cur_layout->addWidget(cur_button);
		connect(cur_button, &QPushButton::clicked, this, [this, i]()
		{
			select_path_handler(i);
		});

		formLayout->addRow(tr(m_path_req[i].tips.data()), cur_layout);
		m_path_labels.push_back(cur_label);
		m_result_path.push_back("");
	}
	auto cur_button = new QPushButton("Confirm");
	connect(cur_button, &QPushButton::clicked, this, &path_config_dialog::confirm_handler);
	formLayout->addRow(cur_button);
	setLayout(formLayout);
	setMinimumWidth(600);
}

void path_config_dialog::confirm_handler()
{
	m_valid = false;
	for (std::size_t i = 0; i < m_path_labels.size(); i++)
	{
		auto cur_text = m_path_labels[i]->text();
		if (cur_text.isEmpty())
		{
			auto notify_info = fmt::format("empty path for {}", m_path_req[i].tips);
			QMessageBox::about(this, QString("Error"),
				QString::fromStdString(notify_info));
			return;
		}
		else
		{
			m_result_path[i] = cur_text.toStdString();
		}
	}
	m_valid = true;
	dump_config();
	close();

}
void path_config_dialog::select_path_handler(std::size_t path_idx)
{
	
	path_req_desc cur_req = m_path_req[path_idx];
	if (cur_req.extension.empty())
	{
		auto folder_name = QFileDialog::getExistingDirectory(this,
			tr("choose btree folder"), QString::fromStdString("./"));
		if (folder_name.isEmpty())
		{
			return;
		}
		else
		{
			m_path_labels[path_idx]->setText(folder_name);
		}
	}
	else
	{
		auto fileName = QFileDialog::getOpenFileName(this,
			tr("choose action file"), QString::fromStdString("./"), tr("Json File (*.json)"));
		if (fileName.isEmpty())
		{
			return;
		}
		else
		{
			m_path_labels[path_idx]->setText(fileName);
		}
	}
}
std::vector<std::string> path_config_dialog::run()
{
	if (m_path_labels.size())
	{
		m_path_labels[0]->setFocus();
	}
	exec();
	if (m_valid)
	{
		return m_result_path;
	}
	else
	{
		return {};
	}
}
void path_config_dialog::dump_config()
{
	json::object_t result = json::object_t();
	for (std::size_t i = 0; i < m_path_req.size(); i++)
	{
		result[m_path_req[i].name] = m_result_path[i];
	}
	
	std::ofstream output(m_dest_file_name);
	output << json(result).dump(4) << std::endl;
	output.close();

}