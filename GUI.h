#ifndef _GUI_H_
#define _GUI_H_

#include "cart.h"
#include "list.h"
#include <string>
#include "commands.h"
#include <pugixml.hpp>
#include <pugixml.cpp>
#include <nana/gui.hpp>
#include <nana/gui/place.hpp>
#include <nana/gui/widgets/form.hpp>
#include <nana/gui/widgets/textbox.hpp>
#include <nana/gui/widgets/listbox.hpp>
#include <nana/gui/widgets/label.hpp>
#include <nana/gui/widgets/button.hpp>
#include <nana/gui/msgbox.hpp>

namespace gui {
	using list_reader::list_of_items;
	class utility_window : public nana::form {
	public:
		utility_window() {
			order.div("<vertical a arrange=[10%,10%,10%,15%,15%]>");
			order.field("a") << i_barcode.multi_lines(false).editable(false).tip_string("Barcode")
				<< i_name.multi_lines(false).tip_string("Name")
				<< i_cost.multi_lines(false).tip_string("Cost")
				<< save.caption("Save")
				<< cancel.caption("Cancel");
			order.collocate();
			popup.icon(popup.icon_error);
			i_barcode.set_accept([](const char & character) {
				return (character >= 48 && character <= 57);
			});
			cancel.events().click([this](const nana::arg_click & arg) {
				this->close();
			});
		}
		inline bool fields_empty() {
			return !(!i_name.caption().empty() && !i_barcode.caption().empty()
				&& !i_cost.caption().empty());
		}
		nana::textbox i_barcode{ *this }, i_name{ *this }, i_cost{ *this };
		nana::place order{ *this };
		nana::button save{ *this }, cancel{ *this };
		nana::msgbox popup{ *this, "Error" };
	};

	class market_gui : public nana::form {
	public:
		market_gui();
	private:
		enum class function {
			add, edit, remove, none = -1
		};
		function funtion_to_call = function::none;
		void add_item();
		void edit_item();
		void remove_item();
		void draw_results(bool all);
		void draw_cart();
		void advanced_commands(const std::string & command);
		nana::label total{ *this };
		nana::textbox searchbar{ *this };
		nana::listbox results{ *this }, cart{ *this };
		nana::place plc{ *this };
		std::string selected_result_code, selected_item_code;
		bool selected_result = false, selected_item = false;
	};

	void gui::market_gui::add_item() {
		utility_window add_window;
		this->enabled(false);
		add_window.i_barcode.editable(true);
		add_window.save.events().click([this, &add_window](const nana::arg_click & arg) {
			if(!add_window.fields_empty()) {
				element temp{ add_window.i_name.caption(), std::stoi(add_window.i_cost.caption()) };
				if(!list_of_items.insert(std::make_pair(add_window.i_barcode.caption(), temp)).second) {
					add_window.popup.clear();
					add_window.popup << "There already is an item with that barcode";
					(add_window.popup() != add_window.popup.pick_ok);
				}
				else {
					list_manip::save_list_on_exit = true;
					add_window.close();
				}
			}
			else {
				add_window.popup.clear();
				add_window.popup << "Please fill out all of the fields.";
				(add_window.popup() != add_window.popup.pick_ok);
			}
		});
		add_window.show();
		add_window.wait_for_this();
		this->enabled(true);
		this->searchbar.caption("");
		nana::API::focus_window(this->searchbar);
	}

	void market_gui::edit_item() {
		utility_window edit_window;
		this->enabled(false);
		auto item = list_of_items.find(selected_result_code);
		edit_window.i_barcode.caption(item->first);
		edit_window.i_name.caption(item->second.name());
		edit_window.i_cost.caption(std::to_string(item->second.cost()));
		edit_window.save.events().click([this, &edit_window](const nana::arg_click & arg) {
			if(!edit_window.fields_empty()) {
				element temp{ edit_window.i_name.caption(), std::stoi(edit_window.i_cost.caption()) };
				list_of_items[this->selected_result_code] = temp;
				this->cart.clear();
				cart::total = 0;
				for(const auto & a : cart::item_cart) {
					this->cart.at(0).append({
						a.first->first, a.first->second.name(), std::to_string(a.first->second.cost()), std::to_string(a.second)
						});
					cart::total += a.first->second.cost();
				}
				this->total.caption("<bold blue size = 30>" + std::to_string(cart::total) + "</>");
				list_manip::save_list_on_exit = true;
				edit_window.close();
				this->searchbar.caption("%all");
			}
			else {
				edit_window.popup.clear();
				edit_window.popup << "Please fill out all of the fields.";
				(edit_window.popup() != edit_window.popup.pick_ok);
			}
		});
		edit_window.show();
		edit_window.wait_for_this();
		this->enabled(true);
		this->searchbar.caption("");
		nana::API::focus_window(this->searchbar);
	}

	void market_gui::remove_item() {
		this->enabled(false);
		nana::msgbox remove_window(*this, "Confirmation", nana::msgbox::yes_no);
		remove_window << "Are you sure you want to remove the item?";
		if(remove_window() == remove_window.pick_yes) {
			list_of_items.erase(list_of_items.find(this->selected_result_code));
			list_manip::save_list_on_exit = true;
		}
		this->searchbar.caption("");
		this->enabled(true);
	}

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
			for(const auto & a : list_of_items) {
				this->results.at(0).append({ a.first, a.second.name(), std::to_string(a.second.cost()) });
			}
		}
		else {
			for(const auto & a : search::results) {
				this->results.at(0).append({ a->first, a->second.name(), std::to_string(a->second.cost()) });
			}
		}
	}

	inline void market_gui::draw_cart() {
		cart::add_to_cart(this->selected_result_code, commands::error_message);
		this->cart.clear();
		for(const auto & a : cart::item_cart) {
			this->cart.at(0).append({
				a.first->first, a.first->second.name(), std::to_string(a.first->second.cost()), std::to_string(a.second)
				});
		}
		this->total.caption("<bold blue size = 30>" + std::to_string(cart::total) + "</>");
	}

	inline void market_gui::advanced_commands(const std::string & command) {
		if(command == "%all") {
			this->results.clear();
			this->draw_results(true);
		}
		else if(command == "%edit") {
			this->results.clear();
			this->draw_results(true);
			this->funtion_to_call = function::edit;
		}
		else if(command == "%remove") {
			this->results.clear();
			this->draw_results(true);
			this->funtion_to_call = function::remove;
		}
		else if(command == "%add") {
			this->results.clear();
			this->add_item();
		}
	}

	market_gui::market_gui() : nana::form(nana::API::make_center(1280, 720)) {
		nana::API::focus_window(this->searchbar);
		this->caption("Market (Dev. Adnan Mukaj)").bgcolor(nana::colors::white);
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
		this->events().destroy([this](const nana::arg_destroy & arg) {
			list_manip::resave();
		});

		searchbar.events().text_changed([this](const nana::arg_textbox & arg) {
			this->results.clear();
			this->selected_result_code.clear();
			this->selected_item_code.clear();
			if(searchbar.caption() == "") {
				return;
			}
			else if(searchbar.caption().front() == '%') {
				this->advanced_commands(this->searchbar.caption());
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
				if(this->searchbar.caption().front() != '%') {
					this->draw_cart();
					this->selected_result_code.clear();
					this->searchbar.caption("");
					results.clear();
				}
				else {
					switch(this->funtion_to_call) {
						case function::edit:
							this->edit_item(); break;
						case function::remove:
							this->remove_item(); break;
					}
				}
			}
			nana::API::focus_window(this->searchbar);
		});
	}
}
#endif // !_GUI_H_