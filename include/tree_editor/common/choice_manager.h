#pragma once
#include <string>
#include <unordered_map>
#include <vector>
#include <memory>
#include <nlohmann/json.hpp>
using json = nlohmann::json;

namespace spiritsaway::tree_editor
{

	class choice_manager
	{
	private:
		choice_manager()
		{

		}
		std::unordered_map<std::string, std::pair<std::shared_ptr<std::vector<std::string>>,
			std::shared_ptr<std::vector<std::string>>>> _choices;
	public:
		bool add_choice(const std::string& _choice_type,
			const std::vector<std::string>& _in_choice, const std::vector<std::string>& _in_choice_text)
		{
			auto cur_iter = _choices.find(_choice_type);
			if (cur_iter != _choices.end())
			{
				return false;
			}
			else
			{
				auto choice_ptr = std::make_shared<std::vector<std::string>>(_in_choice);
				auto choice_text_ptr = std::make_shared<std::vector<std::string>>(_in_choice_text);
				_choices[_choice_type] = std::make_pair(choice_ptr, choice_text_ptr);
				return true;
			}
		}
		std::pair<std::shared_ptr<const std::vector<std::string>>,
			std::shared_ptr<const std::vector<std::string>>> get_choice(const std::string& _choice_type)
		{
			auto cur_iter = _choices.find(_choice_type);
			if (cur_iter == _choices.end())
			{
				return {};
			}
			else
			{
				return cur_iter->second;
			}
		}
		std::string get_choice_comment(const std::string& _choice_type, const std::string& _choice_value) const
		{
			auto cur_iter = _choices.find(_choice_type);
			if (cur_iter == _choices.end())
			{
				return _choice_value;
			}
			for (std::uint32_t i = 0; i < cur_iter->second.first->size(); i++)
			{
				if ((*cur_iter->second.first)[i] == _choice_value)
				{
					return (*cur_iter->second.second)[i];
				}
			}
			return _choice_value;
		}
		void load_from_json(const json& _config)
		{
			if (!_config.is_object())
			{
				return;
			}
			for (const auto& one_choice_value : _config.items())
			{
				std::string cur_choice_name = one_choice_value.key();
				const auto& value = one_choice_value.value();
				if (!value.is_object())
				{
					return;
				}
				std::vector<std::string> choice_kinds;
				std::vector<std::string> choice_texts;
				for (const auto& one_choice_desc : value.items())
				{
					if (!one_choice_desc.value().is_string())
					{
						return;
					}
					choice_kinds.push_back(one_choice_desc.key());
					choice_texts.push_back(one_choice_desc.key() + ": " + one_choice_desc.value().get<std::string>());
				}
				add_choice(cur_choice_name, choice_kinds, choice_texts);
			}
		}

		static choice_manager& instance()
		{
			static choice_manager _instance;
			return _instance;
		}
	};
}