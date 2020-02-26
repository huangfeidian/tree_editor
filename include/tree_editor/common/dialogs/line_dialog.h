#pragma once
#include <QDialog>
#include <string>
#include <vector>
#include <utility>
#include <optional>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qlineedit.h>
#include <qboxlayout.h>
#include <optional>

namespace spiritsaway::tree_editor
{

	class line_dialog : public QDialog
	{

		Q_OBJECT

	public:
		line_dialog(std::string title, std::string init_text, QWidget* parent = 0);

		std::string run();

	public slots:

		void confirm();
		void temp_confirm();

	private:
		std::string _text;
		QLabel* _label;
		QPushButton* _button;
		QLineEdit* _line_edit;
	};
	class uint_dialog : public line_dialog
	{

		Q_OBJECT

	public:
		uint_dialog(std::string title, std::uint32_t init_value, QWidget* parent = 0);

		std::uint32_t run();
	};

}