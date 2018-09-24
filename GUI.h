#ifndef _GUI_H_
#define _GUI_H_

#include "cart.h"
#include "list.h"
#include <string>
#include "commands.h"
#include <nana/gui.hpp>
#include <nana/gui/place.hpp>
#include <nana/gui/widgets/form.hpp>
#include <nana/gui/widgets/textbox.hpp>
#include <nana/gui/widgets/listbox.hpp>
#include <nana/gui/widgets/label.hpp>

namespace gui {
	class market_gui : public nana::form {
	public:
		market_gui();
	private:
		void draw_results(bool all);
		void draw_cart();
		bool advanced_commands(const std::string & command);
		nana::label total{ *this };
		nana::textbox searchbar{ *this };
		nana::listbox results{ *this }, cart{ *this };
		nana::place plc{ *this };
		std::string selected_result_code, selected_item_code;
		bool selected_result = false, selected_item = false;
	};

	void market_gui::draw_results(bool all = false) {
		/* This if statement below is used when we have an exact barcode match so
		it is added automatically into the cart */
		if(search::results.size() == 1 && search::results.front()->first == this->searchbar.caption()) {
			this->selected_result_code = searchbar.caption();
			this->draw_cart();
			this->searchbar.caption("");
			this->results.clear();
			return;
		}
		if(all) {
			for(const auto & a : list_reader::list_of_items) {
				this->results.at(0).append({ a.first, a.second.name(), std::to_string(a.second.cost()) });
			}
		}
		else {
			for(const auto & a : search::results) {
				this->results.at(0).append({ a->first, a->second.name(), std::to_string(a->second.cost()) });
			}
		}
	}

	void market_gui::draw_cart() {
		cart::add_to_cart(this->selected_result_code, commands::error_message);
		this->cart.clear();
		for(const auto & a : cart::item_cart) {
			this->cart.at(0).append({
				a.first->first, a.first->second.name(), std::to_string(a.first->second.cost()), std::to_string(a.second)
				});
		}
		this->total.caption("< bold blue size = 30>" + std::to_string(cart::total) + "</>");
	}

	bool market_gui::advanced_commands(const std::string & command) {
		if(command == "%all") {
			this->results.clear();
			this->draw_results(true);
			return true;
		}
		return false;
	}

	market_gui::market_gui() : nana::form(nana::API::make_center(1280, 720)) {
		nana::API::focus_window(this->searchbar);
		this->caption("Market").bgcolor(nana::colors::white);
		searchbar.tip_string("Search").multi_lines(false);
		total.format(true).caption("<bold blue size=30>0</>").bgcolor(nana::colors::white_smoke);

		// SETTING UP GUI WIDGETS
		plc.div("<vertical a arrange=[3%, 90%]<vertical c >> <vertical b>");
		plc.field("a") << searchbar << cart;
		plc.field("b") << results;
		plc.field("c") << total;
		
		plc.collocate();

		results.append_header("Barcode");
		results.append_header("Name");
		results.append_header("Price");
		cart.append_header("Barcode");
		cart.append_header("Name");
		cart.append_header("Price");
		cart.append_header("Amount");

		// EVENT HANDLING
		searchbar.events().text_changed([this](const nana::arg_textbox & arg) {
			this->results.clear();
			if(searchbar.caption() == "") {
				return;
			}
			else if(searchbar.caption().front() == '%') {
				if(this->advanced_commands(this->searchbar.caption())) {
					return;
				}
			}
			else {
				search::search(searchbar.caption());
				this->draw_results();
			}
		});

		cart.events().selected([this](const nana::arg_listbox & arg) {
			if(!this->selected_item) {
				this->selected_item_code = arg.item.text(0);
				this->selected_item = true;
			}
			else {
				this->selected_item_code.clear();
				this->selected_item = false;
			}
		});

		cart.events().dbl_click([this](const nana::arg_mouse & arg) {
			if(!this->selected_item_code.empty()) {
				if(arg.left_button) {
					cart::remove_from_cart(this->selected_item_code, commands::error_message);
				}
				else {
					cart::remove_from_cart(this->selected_item_code, commands::error_message, true);
				}
				this->selected_item_code.clear();
				this->selected_item = false;
				this->draw_cart(); 
			}
		});

		results.events().selected([this](const nana::arg_listbox & arg) {
			if(this->selected_result_code.empty()) {
				this->selected_result_code = arg.item.text(0);
			}
			else {
				this->selected_result_code.clear();
			}
		});

		results.events().dbl_click([this](const nana::arg_mouse & arg) {
			if(!this->selected_result_code.empty()) {
				this->draw_cart();
				this->selected_result_code.clear();
				results.clear();
				this->searchbar.caption("");
			}
			nana::API::focus_window(this->searchbar);
		});
	}
}
#endif // !_GUI_H_