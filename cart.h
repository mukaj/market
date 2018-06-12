#ifndef _CART_H_
#define _CART_H_

#include <map>
#include <vector>
#include "element.h"
#include "list.h"
#include "pugixml.hpp"
#include "pugixml.cpp"

using list_reader::list_of_items;

/*TODO: Add or subtract the value of the items from the 'total', this feature needs to be implemented in each
function that has to do with adding or subtracting items */

namespace cart {
	typedef std::vector<std::pair<item_list_type::const_iterator, int>> cart_type;
	static cart_type item_cart;
	static float total = 0;
	inline void print_cart(const std::string & a = "") {
		pugi::xml_document doc;
		pugi::xml_node cart_items = doc.append_child("cart_items");
		for(cart_type::const_iterator iter = item_cart.begin(); iter != item_cart.end(); ++iter) {
			pugi::xml_node temp = cart_items.append_child("item");
			temp.append_attribute("cost").set_value(iter->first->second.cost());
			temp.append_attribute("name").set_value(iter->first->second.name().c_str());
			temp.append_attribute("barcode").set_value(iter->first->first.c_str());
			temp.append_attribute("amount").set_value(iter->second);
		}
		if(a.empty()) {
			doc.save(std::cout);
		}
		else {
			doc.save_file(a.c_str());
		}
	}
	inline void add_to_cart(const item_list_type::const_iterator & item) {
		if(item == list_of_items.end()) {
			throw std::exception{ "No item with that barcode exists." };
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
	/* TODO: Remove the checking if the item with that barcode exists in the list or not, just check the cart
	but this means also changing the find_item function by making it take just a barcode */
	inline void remove_from_cart(const std::string & barcode) {
		cart_type::const_iterator iter = find_item(barcode);
		if(iter == item_cart.end()) {
			throw std::exception{ "No item with that barcode is in the cart" };
		}
		//Needs to be tested for bugs
		//item_list_type::const_iterator list_item = list_of_items.find(barcode); // Why is this here?
		item_cart.erase(iter);
		total -= iter->first->second.cost();
	}
	inline void change_quantity(const std::string & barcode, const int & amount = -1) {
		cart_type::iterator cart_item = find_item(barcode);
		if(cart_item == item_cart.end()) {
			throw std::exception{ "No item with that barcode is in the cart" };
		}
		if(amount == 0) {
			item_cart.erase(cart_item);
		}
		else if(amount == -1) {
			++cart_item->second;
		}
		else {
			cart_item->second = amount;
		}
	}
	///
	void checkout() {} // UNFINISHED FEATURE
	///

}
#endif // !_CART_H_