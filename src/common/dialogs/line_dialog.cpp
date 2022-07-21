#include <tree_editor/common/dialogs/line_dialog.h>

using namespace spiritsaway::tree_editor;
line_dialog::line_dialog(std::string title, std::string init_text, QWidget* parent)
	: QDialog(parent)
{
	QHBoxLayout *cur_layout = new QHBoxLayout;
	m_label = new QLabel(tr(title.c_str()));
	m_line_edit = new QLineEdit();
	m_line_edit->setText(QString::fromStdString(init_text));
	m_button = new QPushButton(tr("Confirm"));
	cur_layout->addWidget(m_label);
	cur_layout->addWidget(m_line_edit);
	cur_layout->addWidget(m_button);
	setLayout(cur_layout);
	connect(m_button, SIGNAL(clicked()), this, SLOT(confirm()));
	connect(m_line_edit, &QLineEdit::editingFinished, this, &line_dialog::temp_confirm);
}

void line_dialog::confirm()
{
	auto temp_text = m_line_edit->text();
	m_text = temp_text.toStdString();
	close();
}
std::string line_dialog::run()
{
	m_line_edit->setFocus();
	exec();
	return m_text;
}
void line_dialog::temp_confirm()
{
	auto temp_text = m_line_edit->text();
	m_text = temp_text.toStdString();
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