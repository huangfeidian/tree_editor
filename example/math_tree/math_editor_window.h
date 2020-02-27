#pragma once
#include <filesystem>
#include <tree_editor/editor/editor_main_window.h>

namespace spiritsaway::tree_editor
{
	class math_editor_window : public editor_main_window
	{
	public:
		bool load_config();
		basic_node* create_node_from_desc(const basic_node_desc& desc, basic_node* parent) override;
	private:
		std::string new_file_name();
	};
}