#ifndef _COMMANDS_H_
#define _COMMANDS_H_

#include <string>
#include "cart.h"
#include "list.h"
#include <stdexcept>
#include "element.h"

namespace commands {
	enum class command {
		read_list, print_list_xml,
		search, add_cart, remove_cart,
		empty_cart, exit = -1
	};

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
	std::string error_message;

	bool backend_function_selector() {
		std::string argument;
		command option;
		getline(std::cin, argument);
		try {
			if(!argument.empty()) {
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
					case command::print_list_xml:
						{
							list_manip::print_list(error_message);
							break;
						}
					case command::search:
						{
							if(argument.empty()) {
								error_message = "No search text was given.";
								break;
							}
							search::search(argument);
							break;
						}
					case command::add_cart:
						{
							if(argument.empty()) {
								error_message = "No barcode was given.";
								break;
							}
							cart::add_to_cart(argument, error_message);
							break;
						}
					case command::remove_cart:
						{
							cart::remove_from_cart(argument, error_message);
							break;
						}
					case command::empty_cart:
						{
							cart::item_cart.clear();
							cart::total = 0;
							break;
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
							error_message = "Invalid command option given.";
						}
				}
				if(!error_message.empty()) {
					std::cerr << error_message << std::endl;
					error_message.clear();
				}
			}
			else {
				std::cerr << "No command given." << std::endl;
			}
		}
		catch(const pugi::xml_parse_result& a) {
			std::cerr << "Parse error: " << a.description() << ", character pos = "
				<< a.offset << std::endl;
		}
		catch(const std::exception& a) {
			std::cerr << a.what() << std::endl;
		}
		return true;
	}
}
#endif // !_COMMANDS_H_