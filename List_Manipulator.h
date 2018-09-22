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
using list_reader::list_of_items;

namespace list_manip {
	/* LIST SAVING & PRINTING */
	bool save_list_on_exit = false;
	inline void print_list(std::string & error_message) {
		if(list_of_items.empty()) {
			error_message = "The item list is empty.";
			return;
		}
		pugi::xml_document doc;
		pugi::xml_node item_root = doc.append_child("items");
		for(const auto & item : list_reader::list_of_items) {
			pugi::xml_node temp = item_root.append_child("item");
			temp.append_attribute("cost").set_value(item.second.cost());
			temp.append_attribute("name").set_value(item.second.name().c_str());
			temp.append_attribute("barcode").set_value(item.first.c_str());
		}
		doc.print(std::cout);
	}
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
			std::string new_name = file_name + "_OLD";
			rename(file_name.c_str(), new_name.c_str());
		}
		file.save_file(file_name.c_str()); 
	}
	/* !LIST SAVING & PRINTING */


	/* ITEM ADDING */
	void add_item(const std::string & input) {
		pugi::xml_document doc;
		pugi::xml_parse_result parse_result = doc.load_string(input.c_str());
		if(!parse_result) throw parse_result;
		pugi::xml_node item = doc.document_element();
		item_list_pair new_item = element_via_xml(input);
		list_of_items[new_item.first] = new_item.second;
		save_list_on_exit = true;
	}
	/* !ITEM ADDING */


	/* ITEM EDITING */
	void edit_item(const std::string & input) {
		pugi::xml_document doc;
		pugi::xml_parse_result parse_result = doc.load_string(input.c_str());
		if(!parse_result) throw parse_result;
		pugi::xml_node item = doc.document_element();
		item_list_pair edited_item = element_via_xml(input);
		list_of_items.erase(edited_item.first); // This will erase the previous element with that barcode
		list_of_items.insert(edited_item); // And this re-adds that item, now edited
		save_list_on_exit = true;
	}
	/* !ITEM EDITING */


	/* ITEM REMOVAL */
	void remove_item(const std::string & barcode, std::string & error_message) {
		for(item_list_type::const_iterator iter = list_of_items.begin();
			iter != list_of_items.end(); ++iter) {
			if(iter->first == barcode) {
				save_list_on_exit = true;
				list_of_items.erase(iter);
				return;
			}
		}
		error_message = "No barcode match was found";
	}
	/* !ITEM REMOVAL */
}

namespace search {
	std::vector<item_list_type::const_iterator> results;

	inline bool check_if_barcode(const std::string & input) {
		for(const char & i : input) {
			if(!(i>= 48 && i <= 57)) {
				return false;
			}
		}
		return true;
	}

	void search(std::string & input) {
		bool is_barcode = check_if_barcode(input);
		results.clear();
		if(is_barcode) {
			for(item_list_type::const_iterator it = list_of_items.begin();
				it != list_of_items.end();++it) {
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
		if(results.size() == 0) {
			std::cerr << "No matches were found." << std::endl;
		}
	}
}
#endif // !_LIST_MANIPULATOR_H_