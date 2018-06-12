#ifndef _COMMANDS_H_
#define _COMMANDS_H_

#include "list.h"
#include "element.h"
#include "cart.h"
#include <string>
#include <stdexcept>

enum class command {
	read_list, print_list, add, edit, remove,
	search, add_cart, remove_cart,
	print_cart, empty_cart, save_cart, exit
};

namespace commands {
	inline void space_stripper(std::string & argument) {
		if(argument.empty()) {
			return;
		}
		while(argument.front() == ' ') {
			argument.erase(argument.begin());
		}
		while(argument.back() == ' ') {
			argument.pop_back();
		}
		if(argument.size() < 1) {
			return;
		}
		for(std::string::iterator it = argument.begin() + 1; it != argument.end(); ++it) {
			if(*it == ' ' && *(it - 1) == ' ') {
				argument.erase(it);
				it = argument.begin() + 1;
			}
		}
	}
	bool function_selector() {
		std::string argument;
		command option;
		getline(std::cin, argument);
		try {
			if(argument.empty()) {
				throw std::invalid_argument{ "No commands have been given." };
			}
		{
			std::string temp;
			for(const char & a : argument) {
				if(a != ' ') {
					temp += a;
				}
				else {
					break;
				}
			}
			option = command(std::stoi(temp));
			argument.erase(argument.begin(), argument.begin() + temp.size());
		}
			space_stripper(argument);
			switch(option) {
				case command::read_list:
					{
						list_of_items.clear();
						if(argument.empty()) {
							list_reader::read_list();
						}
						else {
							list_reader::file_name = argument;
							list_reader::read_list(argument);
						}
						break;
					}
				case command::print_list:
					{
						list_manip::print_list();
						break;
					}
				case command::add:
					{
						list_manip::add_item(argument);
						break;
					}
				case command::edit:
					{
						if(!cart::item_cart.empty()) {
							throw std::exception{ "Cannot edit item list while cart is not empty." };
						}
						list_manip::edit_item(argument);
						break;
					}
				case command::remove:
					{
						if(!cart::item_cart.empty()) {
							throw std::exception{ "Cannot edit item list while cart is not empty." };
						}
						list_manip::remove_item(argument);
						break;
					}
				case command::search:
					{
						if(argument.empty()) {
							throw std::invalid_argument{ "No search text was given." };
						}
						search::draw_result(argument);
						break;
					}
				case command::add_cart:
					{
						item_list_type::const_iterator iter = list_reader::list_of_items.find(argument);
						cart::add_to_cart(iter);
						break;
					}
				case command::remove_cart:
					{
						cart::remove_from_cart(argument);
						break;
					}
				case command::print_cart:
					{
						cart::print_cart();
						break;
					}
				case command::empty_cart:
					{
						cart::item_cart.clear();
						break;
					}
				case command::save_cart:
					{
						if(argument.empty()) {
							throw std::invalid_argument{ "You cannot save a cart file without a name." };
						}
						cart::print_cart(argument.c_str());
					}
				case command::exit:
					{
						if(argument.size() == 0) {
							list_manip::resave();
						}
						else {
							list_manip::resave(argument);
						}
						return false;
					} 
				default:
					{
						throw std::invalid_argument{"Invalid command option given."};
					}
			}
		}
		catch(const pugi::xml_parse_result& a) {
			std::cerr << "Parse error: " << a.description() << ", character pos = "
				<< a.offset << std::endl;
		}
		catch(const std::invalid_argument& a) {
			std::cerr << a.what();
		}
		catch(const std::exception& a) {
			std::cerr << a.what();
		}
		return true;
	}
}
#endif // !_COMMANDS_H_