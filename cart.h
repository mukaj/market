#ifndef _CART_H_
#define _CART_H_

#include <map>
#include <vector>
#include <pugixml.hpp>
#include <pugixml.cpp>
#include "element.h"
#include "list.h"


using list_reader::list_of_items;

namespace cart {
	typedef std::vector<std::pair<item_list_type::const_iterator, int>> cart_type;
	static cart_type item_cart;
	unsigned int total = 0;
	inline void print_cart(std::string & error_code, const std::string & a = "") {
		pugi::xml_document doc;
		pugi::xml_node cart_items = doc.append_child("cart_items");
		if(item_cart.empty()) {
			error_code = "Cart is empty.";
			return;
		}
		for(cart_type::const_iterator iter = item_cart.begin(); iter != item_cart.end(); ++iter) {
			pugi::xml_node temp = cart_items.append_child("item");
			temp.append_attribute("cost").set_value(iter->first->second.cost());
			temp.append_attribute("name").set_value(iter->first->second.name().c_str());
			temp.append_attribute("barcode").set_value(iter->first->first.c_str());
			temp.append_attribute("amount").set_value(iter->second);
		}
		if(a.empty()) {
			doc.print(std::cout);
		}
		else {
			doc.save_file(a.c_str());
		}
	}
	inline void add_to_cart(const std::string & barcode, std::string & error_message) {
		item_list_type::const_iterator item = list_of_items.find(barcode);
		if(item == list_of_items.end()) {
			error_message = "No item with that barcode exists.";
			return;
		}
		for(cart_type::iterator iter = item_cart.begin(); iter != item_cart.end(); ++iter) {
			if(iter->first == item) {
				++iter->second;
				return;
			}
		}
		total += item->second.cost();
		item_cart.push_back(std::make_pair(item, 1)); // The 1 is the amount of the item we wish to add
	}
	inline cart_type::iterator find_item(const std::string & barcode) {
		for(cart_type::iterator iter = item_cart.begin(); iter != item_cart.end(); ++iter) {
			if(iter->first->first == barcode) {
				return iter;
			}
		}
		return item_cart.end();
	}
	inline void remove_from_cart(const std::string & barcode, std::string & error_message) {
		cart_type::const_iterator iter = find_item(barcode);
		if(iter == item_cart.end()) {
			error_message = "No item with that barcode is in the cart";
			return;
		}	
		total -= iter->first->second.cost();
		item_cart.erase(iter);
	}
	inline void change_quantity(const std::string & barcode, std::string & error_code,
		const int & amount = -1) {
		cart_type::iterator cart_item = find_item(barcode);
		if(cart_item == item_cart.end()) {
			error_code = "No item with that barcode is in the cart";
			return;
		}
		if(amount == 0) {
			item_cart.erase(cart_item);
		}
		/* This is so when the item is scanned and we know the user did
		not manualy enter any value, and so we just add one more of the item */
		else if(amount == -1) { 
			++cart_item->second;
		}
		else {
			cart_item->second = amount;
		}
	}
}
#endif // !_CART_H_