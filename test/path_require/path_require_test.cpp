#include <qapplication.h>
#include <dialogs/path_config_dialog.h>
#include <iostream>


using namespace spiritsaway::tree_editor;
using namespace std;

int main(int argc, char* argv[])
{
	QApplication a(argc, argv);
	std::vector<path_req_desc> descs;
	path_req_desc desc_1;
	desc_1.extension = ".json";
	desc_1.name = "file";
	desc_1.tips = "path to file";
	path_req_desc desc_2;
	desc_2.extension = "";
	desc_2.name = "folder";
	desc_2.tips = "path to folder";
	std::string dest_name = "path_req_test.json";
	descs.push_back(desc_1);
	descs.push_back(desc_2);
	auto cur_dialog = new path_config_dialog(descs, dest_name);
	cur_dialog->show();
	auto result = cur_dialog->run();
	for (auto one_result : result)
	{
		cout << "one result is " << one_result << endl;
	}
}