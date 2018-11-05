#ifndef _CART_H_
#define _CART_H_

#include <map>
#include <vector>
#include "list.h"
#include "element.h"
#include <pugixml.cpp>
#include <pugixml.hpp>

using list_reader::list_of_items;
typedef std::vector<std::pair<item_list_type::const_iterator, int>> cart_type;

namespace cart {
	static cart_type item_cart;
	unsigned int total = 0;
	inline void add_to_cart(const std::string & barcode, std::string & error_message) {
		item_list_type::const_iterator item = list_of_items.find(barcode);
		if(item == list_of_items.end()) {
			error_message = "No item with that barcode exists.";
			return;
		}
		for(cart_type::iterator iter = item_cart.begin(); iter != item_cart.end(); ++iter) {
			if(iter->first == item) {
				++iter->second;
				total += item->second.cost();
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

	inline void remove_from_cart(const std::string & barcode, std::string & error_message, bool all = false) {
		cart_type::iterator iter = find_item(barcode);
		if(iter == item_cart.end()) {
			error_message = "No item with that barcode is in the cart";
			return;
		}
		if(!all) {
			total -= iter->first->second.cost();
			if(--iter->second <= 0) {
				item_cart.erase(iter);
			}
		}
		else {
			total -= iter->first->second.cost() * iter->second;
			item_cart.erase(iter);
		}
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