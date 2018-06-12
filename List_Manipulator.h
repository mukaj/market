#ifndef _LIST_MANIPULATOR_H_
#define _LIST_MANIPULATOR_H_

#include <experimental\filesystem>
#include <cstdio>
#include <fstream>
#include <vector>
#include <map>
#include <string>
#include "pugixml.hpp"
#include "pugixml.cpp"
#include "List_Reader.h"
#include "element.h"

typedef std::map<std::string, element> item_list_type;
typedef std::pair<std::string, element> item_list_pair;
using list_reader::list_of_items;

namespace list_manip {
	/* LIST SAVING & PRINTING */
	bool save_list_on_exit = false;
	inline void print_list(const bool & as_xml = true) {
		if(list_of_items.empty()) {
			throw std::exception{ "The item list is empty" };
		}
		if(as_xml) {
			pugi::xml_document doc;
			pugi::xml_node item_root = doc.append_child("items");
			for(const auto & item : list_reader::list_of_items) {
				pugi::xml_node temp = item_root.append_child("item");
				temp.append_attribute("cost").set_value(item.second.cost());
				temp.append_attribute("name").set_value(item.second.name().c_str());
				temp.append_attribute("barcode").set_value(item.first.c_str());
			}
			doc.save(std::cout);
		}
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
	void remove_item(const std::string & barcode) {
		for(item_list_type::const_iterator iter = list_of_items.begin();
			iter != list_of_items.end(); ++iter) {
			if(iter->first == barcode) {
				save_list_on_exit = true;
				list_of_items.erase(iter);
				return;
			}
		}
		throw std::exception{ "No barcode match was found" };
	}
	/* !ITEM REMOVAL */
}

namespace search {
	inline bool check_if_barcode(const std::string & input) {
		for(const char & i : input) {
			if(!(i>= 48 && i <= 57)) {
				return false;
			}
		}
		return true;
	}
	/*This function is called every time in the search bar a character is added or removed, and this function deals
	with finding and sending the results to the output stream */
	void draw_result(const std::string & input) {
		bool is_barcode = check_if_barcode(input);
		std::vector<item_list_type::const_iterator> results;
		if(is_barcode) {
			//The one with barcode won't work unless the barcode is the exact numberical value	
			item_list_type::const_iterator result = list_of_items.find(input);
			if(result != list_of_items.end()) {
				results.push_back(result);
			}
		}
		else {
			for(item_list_type::const_iterator iter = list_of_items.begin(); iter != list_of_items.end(); ++iter) {
				if(iter->second.name().find(input) != std::string::npos) {
					results.push_back(iter);
				}
			}
		}
		if(results.size() == 0) {
			std::cout << "No matches were found." << std::endl;
			return;
		}
		pugi::xml_document search_results;
		pugi::xml_node root = search_results.append_child("results");
		for(std::vector<item_list_type::const_iterator>::const_iterator iter = results.begin();
			iter != results.end(); ++iter) {
			pugi::xml_node temp = root.append_child("item");
			temp.append_attribute("cost").set_value((*iter)->second.cost());
			temp.append_attribute("name").set_value((*iter)->second.name().c_str());
			temp.append_attribute("barcode").set_value((*iter)->first.c_str());
		}
		search_results.save(std::cout);
	}
}
#endif // !_LIST_MANIPULATOR_H_