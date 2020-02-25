#include <QApplication>
#include "math_editor_window.h"

using namespace std;
using namespace spiritsaway::tree_editor;

int main(int argc, char* argv[])
{
	QApplication a(argc, argv);
	math_editor_window w = math_editor_window();
	w.showMaximized();
	w.show();
	if (!w.load_config())
	{
		return 0;
	}
	return a.exec();
}