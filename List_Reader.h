#ifndef _LIST_READER_H_
#define _LIST_READER_H_

#include <experimental/filesystem>
#include <iostream>
#include <vector>
#include <fstream>
#include <map>
#include <string>
#include "element.h"
#include "pugixml.cpp"
#include "pugixml.hpp"

typedef std::map<std::string, element> item_list_type;

namespace list_reader {
	//This string holds the name of the default file name that we will use to read the list from
	static std::string file_name = "example.xml";
	//This is the map that contains all the elements and their barcode
	static item_list_type list_of_items;
	void read_list(const std::string & file_name = list_reader::file_name) {
		if(!std::experimental::filesystem::exists(file_name)) {
			std::cerr << "No " << file_name << " file found.";
		}
		else {
			pugi::xml_document doc; // This is the document that holds the xml tree
			pugi::xml_parse_result parse_result = doc.load_file(file_name.c_str());
			if(!parse_result) throw parse_result;
			pugi::xml_node root_node = doc.root().child("items");
			for(pugi::xml_node_iterator node_it = root_node.begin(); node_it != root_node.end(); ++node_it) {
				element temp;
				int counter = 0;
				for(pugi::xml_attribute_iterator attr_it = node_it->attributes_begin();
					attr_it != node_it->attributes_end(); ++attr_it) {
					switch(counter) {
						case 0:
							temp.set_cost(attr_it->as_uint());
							break;
						case 1:
							temp.set_name(attr_it->as_string());
							break;
						case 2:
							list_of_items[attr_it->as_string()] = temp;
					}
					++counter;
				}
			}
		}
	}
}
#endif // !_LIST_READER_H_