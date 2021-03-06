﻿#include <tree_editor/common/dialogs/line_dialog.h>

using namespace spiritsaway::tree_editor;
line_dialog::line_dialog(std::string title, std::string init_text, QWidget* parent)
	: QDialog(parent)
{
	QHBoxLayout *cur_layout = new QHBoxLayout;
	_label = new QLabel(tr(title.c_str()));
	_line_edit = new QLineEdit();
	_line_edit->setText(QString::fromStdString(init_text));
	_button = new QPushButton(tr("Confirm"));
	cur_layout->addWidget(_label);
	cur_layout->addWidget(_line_edit);
	cur_layout->addWidget(_button);
	setLayout(cur_layout);
	connect(_button, SIGNAL(clicked()), this, SLOT(confirm()));
	connect(_line_edit, &QLineEdit::editingFinished, this, &line_dialog::temp_confirm);
}

void line_dialog::confirm()
{
	auto temp_text = _line_edit->text();
	_text = temp_text.toStdString();
	close();
}
std::string line_dialog::run()
{
	_line_edit->setFocus();
	exec();
	return _text;
}
void line_dialog::temp_confirm()
{
	auto temp_text = _line_edit->text();
	_text = temp_text.toStdString();
}
uint_dialog::uint_dialog(std::string title, std::uint32_t init_value, QWidget* parent)
	:line_dialog(title, std::to_string(init_value), parent)
{

}
std::uint32_t uint_dialog::run()
{
	auto text = line_dialog::run();
	std::uint32_t result = 0;
	for (const auto i : text)
	{
		if (i < '0' || i > '9')
		{
			return 0;
		}
		auto cur_digit = i - '0';
		result = result * 10 + cur_digit;
	}
	return result;
}