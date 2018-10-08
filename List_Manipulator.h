#ifndef _LIST_MANIPULATOR_H_
#define _LIST_MANIPULATOR_H_

#include <map>
#include <cstdio>
#include <string>
#include <vector>
#include <fstream>
#include "element.h"
#include <algorithm>
#include <filesystem>
#include <pugixml.cpp>
#include <pugixml.hpp>
#include "List_Reader.h"


typedef std::map<std::string, element> item_list_type;
typedef std::pair<std::string, element> item_list_pair;
typedef std::vector<item_list_type::const_iterator> result_list_type;


namespace list_manip {
	using list_reader::list_of_items;
	/* LIST SAVING & PRINTING */
	bool save_list_on_exit = false;
	inline item_list_pair element_via_xml(const std::string & input) {
		pugi::xml_document doc;
		pugi::xml_parse_result parse_result = doc.load_string(input.c_str());
		if(!parse_result) throw parse_result;
		pugi::xml_node item = doc.document_element();
		item_list_pair new_element;
		int attribute_counter = 0;
		for(pugi::xml_attribute_iterator it = item.attributes_begin(); it != item.attributes_end(); ++it) {
			switch(attribute_counter) {
				case 0:
					new_element.second.set_cost(it->as_uint());
					break;
				case 1:
					new_element.second.set_name(it->as_string());
					break;
				case 2:
					new_element.first = it->as_string();
			}
			++attribute_counter;
		}
		return new_element;
	}
	inline void resave(std::string & file_name = list_reader::file_name) {
		if(!list_manip::save_list_on_exit) {
			return;
		}
		pugi::xml_document file;
		pugi::xml_node items = file.root().append_child("items");
		for(item_list_type::const_iterator it = list_reader::list_of_items.begin();
			it != list_reader::list_of_items.end(); ++it) {
			pugi::xml_node temp = items.append_child("item");
			temp.append_attribute("cost").set_value(it->second.cost());
			temp.append_attribute("name").set_value(it->second.name().c_str());
			temp.append_attribute("barcode").set_value(it->first.c_str());
		}
		if(std::experimental::filesystem::exists(file_name)) {
			remove(file_name.c_str());
		}
		file.save_file(file_name.c_str());
	}
	/* !LIST SAVING & PRINTING */
}

namespace search {
	using list_reader::list_of_items;
	std::vector<item_list_type::const_iterator> results;

	inline bool check_if_barcode(const std::string & input) {
		for(const char & i : input) {
			if(!(i >= 48 && i <= 57)) {
				return false;
			}
		}
		return true;
	}

	void search(std::string input, std::string & error_message) {
		bool is_barcode = check_if_barcode(input);
		results.clear();
		if(is_barcode) {
			for(item_list_type::const_iterator it = list_of_items.begin();
				it != list_of_items.end(); ++it) {
				if(it->first.find(input) != std::string::npos) {
					results.push_back(it);
					if(input == it->first) {
						results.clear();
						results.push_back(it);
						return;
					}
				}
			}
		}
		else {
			std::transform(input.begin(), input.end(), input.begin(), ::tolower);
			for(item_list_type::const_iterator iter = list_of_items.begin();
				iter != list_of_items.end(); ++iter) {
				std::string lowercase_item_name = iter->second.name();
				std::transform(lowercase_item_name.begin(), lowercase_item_name.end(), lowercase_item_name.begin(), ::tolower);
				if(lowercase_item_name.find(input) != std::string::npos) {
					results.push_back(iter);
				}
				lowercase_item_name.clear();
			}
		}
	}
}
#endif // !_LIST_MANIPULATOR_H_