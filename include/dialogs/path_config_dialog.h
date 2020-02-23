#pragma once

#include <QDialog>
#include <string>
#include <vector>
#include <utility>
#include <optional>
#include <qlabel.h>

#include <QStringListModel>

namespace spiritsaway::tree_editor
{
	struct path_req_desc
	{
		std::string name;
		std::string tips;
		std::string extension;//if empty then foler
	};
	class path_config_dialog :public QDialog
	{
		Q_OBJECT

	public:
		explicit path_config_dialog(const std::vector<path_req_desc>& _path_req, const std::string& dest_file_name, QWidget* parent = nullptr);
		std::vector<std::string>  run();
		bool valid = false;
	private:
		void dump_config();
		const std::vector<path_req_desc> _path_req;
		std::vector<std::string> result_path;
		std::vector<QLabel*> _path_labels;
		const std::string dest_file_name;
	public slots:
		void confirm_handler();

		void select_path_handler(std::size_t path_idx);

	};
}

