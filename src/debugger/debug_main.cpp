#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenu>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QTreeView>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>
#include <QDialog>
#include <QApplication>
#include <qfile.h>

#include <fstream>
#include <iostream>

#include <any_container/decode.h>
#include <any_container/any_value.h>


#include <tree_editor/debugger/debugger_main_window.h>

using namespace std;
using namespace spiritsaway::tree_editor;

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    
	debugger_main_window w = debugger_main_window();
	ifstream history_file = ifstream("../../data/history/btree_cmd_log.json");
	std::string file_content = std::string(std::istreambuf_iterator<char>(history_file),
		std::istreambuf_iterator<char>());
	history_file.close();
	if (!json::accept(file_content))
	{
		cout << "invalid history file" << endl;
		return 1;
	}
	auto config_json = json::parse(file_content);
	using temp_cmd_type = std::tuple<std::uint64_t, std::uint32_t, spiritsaway::serialize::any_vector>;
	std::vector<temp_cmd_type> cmds;
	decode(config_json, cmds);

	
	w.showMaximized();

    w.show();
    return a.exec();
}