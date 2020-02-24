#pragma once
#include <filesystem>
#include <tree_editor/editor/editor_main_window.h>

namespace spiritsaway::tree_editor
{
	class math_editor_window : public editor_main_window
	{
	public:
		bool load_config();
		std::string action_new_impl();
		std::string action_open_impl();
	private:
		std::filesystem::path data_folder;
		std::string new_file_name();
	};
}