#ifndef _GUI_HPP_
#define _GUI_HPP_

#include "cart.h"
#include "list.h"
#include <string>
#include <pugixml.hpp>
#include <pugixml.cpp>
#include <nana/gui.hpp>
#include <nana/gui/place.hpp>
#include <nana/gui/msgbox.hpp>
#include <nana/gui/filebox.hpp>
#include <nana/gui/widgets/form.hpp>
#include <nana/gui/widgets/label.hpp>
#include <nana/gui/widgets/button.hpp>
#include <nana/gui/widgets/textbox.hpp>
#include <nana/gui/widgets/listbox.hpp>
#include <nana/gui/widgets/menubar.hpp>

namespace gui {
	using list_reader::list_of_items;
	class utility_window : public nana::form {
	public:
		utility_window(const char *title = "") {
			this->caption(title);
			order.div("<vertical a arrange=[10%,10%,10%,15%,15%]>");
			order.field("a") << i_barcode.multi_lines(false).editable(false).tip_string("Barcode")
				<< i_name.multi_lines(false).tip_string("Name")
				<< i_cost.multi_lines(false).tip_string("Cost")
				<< save.caption("Save")
				<< cancel.caption("Cancel");
			order.collocate();
			popup.icon(popup.icon_error);
			i_barcode.set_accept([](const wchar_t & character) {
				return ((character >= 48 && character <= 57) || character == '\b');
			});
			i_cost.set_accept([](const wchar_t & character) {
				return ((character >= 48 && character <= 57) || character == '\b');
			});
			cancel.events().click([this](const nana::arg_click & arg) {
				this->close();
			});
		}
		inline bool fields_empty() {
			return (i_name.caption().empty() || i_barcode.caption().empty()
				|| i_cost.caption().empty());
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
			none = -1, add, edit, remove
		};
		enum class market_error_type {
			undefined = -1, no_list_read
		};
		function function_to_call = function::none;
		void report_error(market_error_type);
		void read_file();
		void add_item();
		void edit_item();
		void remove_item();
		void draw_cart();
		void draw_results(bool);
		bool loaded_list();
		nana::place plc_{ *this };
		nana::label total_{ *this };
		nana::menubar menubar_{ *this };
		nana::textbox searchbar_{ *this };
		nana::msgbox error_dialog_{ *this,"Error" };
		nana::listbox results_{ *this }, cart_{ *this };
		std::string selected_result_code_, selected_item_code_, error_message_;
		bool selected_result_ = false, selected_item_ = false, loaded_item_list_ = false;
	};

	inline void market_gui::report_error(market_error_type err = market_error_type::undefined) {
		switch(err) {
			case market_error_type::no_list_read:
				error_message_ = "You need to have read an item list in order to use this feature.";
				break;
		}
		error_dialog_ << error_message_;
		error_dialog_();
		error_dialog_.clear();
		error_message_.clear();
	}

	void gui::market_gui::read_file() {
		this->enabled(false);
		nana::filebox a(true);
		a.add_filter("XML File", "*.xml");
		if(a()) {
			try {
				list_reader::read_list(error_message_, a.file());
				if(!error_message_.empty())
					this->report_error();
				loaded_item_list_ = true;
			}
			catch(const pugi::xml_parse_result& parse_error) {
				nana::msgbox error_display(*this, "Error!");
				error_display << "Parse error: " << parse_error.description() << ", character pos = "
					<< parse_error.offset;
				error_display();
			}
		}
		this->enabled(true);
		nana::API::focus_window(*this);
	}

	void gui::market_gui::add_item() {
		utility_window add_window{ "Add Item" };
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
				add_window.popup();
			}
		});
		add_window.show();
		add_window.wait_for_this();
		searchbar_.caption("");
		this->enabled(true);
		nana::API::focus_window(searchbar_);
	}

	void market_gui::edit_item() {
		utility_window edit_window{ "Edit Item" };
		this->enabled(false);
		auto item = list_of_items.find(selected_result_code_);
		edit_window.i_barcode.caption(item->first);
		edit_window.i_name.caption(item->second.name());
		edit_window.i_cost.caption(std::to_string(item->second.cost()));
		edit_window.save.events().click([this, &edit_window](const nana::arg_click & arg) {
			if(!edit_window.fields_empty()) {
				list_of_items[selected_result_code_] = element(edit_window.i_name.caption(), std::stoi(edit_window.i_cost.caption()));
				cart_.clear();
				cart::total = 0;
				for(const auto & a : cart::item_cart) {
					cart_.at(0).append({
						a.first->first, a.first->second.name(), std::to_string(a.first->second.cost()), std::to_string(a.second)
						});
					cart::total += a.first->second.cost() * a.second;
				}
				total_.caption("<bold blue size = 30>" + std::to_string(cart::total) + "</>");
				list_manip::save_list_on_exit = true;
				edit_window.close();
				searchbar_.caption("%all");
			}
			else {
				edit_window.popup.clear();
				edit_window.popup << "Please fill out all of the fields.";
				(edit_window.popup() != edit_window.popup.pick_ok);
			}
		});
		edit_window.show();
		edit_window.wait_for_this();
		searchbar_.caption("");
		this->enabled(true);
		nana::API::focus_window(searchbar_);
	}

	void market_gui::remove_item() {
		this->enabled(false);
		nana::msgbox remove_window(*this, "Confirmation", nana::msgbox::yes_no);
		remove_window << "Are you sure you want to remove the item?";
		if(remove_window() == remove_window.pick_yes) {
			list_of_items.erase(list_of_items.find(selected_result_code_));
			list_manip::save_list_on_exit = true;
		}
		searchbar_.caption("");
		this->enabled(true);
	}

	inline void market_gui::draw_cart() {
		if(!error_message_.empty()) {
			this->report_error();
			return;
		}
		cart_.clear();
		for(const auto & a : cart::item_cart) {
			cart_.at(0).append({
				a.first->first, a.first->second.name(), std::to_string(a.first->second.cost()), std::to_string(a.second)
				});
		}
		total_.caption("<bold blue size = 30>" + std::to_string(cart::total) + "</>");
	}

	void market_gui::draw_results(bool all = false) {
		/* This if statement below is used when we have an exact barcode match so
		it is added automatically into the cart */
		if(search::results.size() == 1 && search::results.front()->first == searchbar_.caption()) {
			selected_result_code_ = searchbar_.caption();
			cart::add_to_cart(selected_result_code_, error_message_);
			selected_result_code_ = "";
			selected_result_ = false;
			this->draw_cart();
			searchbar_.caption("");
			results_.clear();
			return;
		}
		results_.auto_draw(false);
		if(all) {
			searchbar_.caption("");
			for(const auto & a : list_of_items) {
				results_.at(0).append({ a.first, a.second.name(), std::to_string(a.second.cost()) });
			}
		}
		else {
			for(const auto & a : search::results) {
				results_.at(0).append({ a->first, a->second.name(), std::to_string(a->second.cost()) });
			}
		}
		results_.auto_draw(true);
	}

	inline bool market_gui::loaded_list() {
		return loaded_item_list_;
	}

	market_gui::market_gui() : nana::form(nana::API::make_center(1280, 720)) {
		nana::API::focus_window(searchbar_);
		this->caption("Market").bgcolor(nana::colors::white);
		searchbar_.tip_string("Search").multi_lines(false);
		total_.format(true).caption("<bold blue size=30>0</>").bgcolor(nana::colors::white_smoke);
		menubar_.bgcolor(nana::colors::white);

		// SETTING UP GUI WIDGETS
		plc_.div("<vertical <m weight=25><g<vertical a arrange=[7%, 3%, 90%]> <vertical b>>");
		plc_.field("a") << total_ << searchbar_ << cart_;
		plc_.field("b") << results_;
		plc_.field("m") << menubar_;
		plc_.collocate();

		menubar_.push_back("&File");
		menubar_.push_back("&Items");
		menubar_.push_back("&Help");
		results_.append_header("Barcode");
		results_.append_header("Name");
		results_.append_header("Price");
		cart_.append_header("Barcode");
		cart_.append_header("Name");
		cart_.append_header("Price");
		cart_.append_header("Amount");

		// MENUBAR CONFIGURATION
		menubar_.at(0).append("&Read List File", [this](const nana::menu::item_proxy & prx) {
			this->read_file();
		});
		menubar_.at(0).append("&Exit", [this](const nana::menu::item_proxy & prx) {
			this->close();
		});
		menubar_.at(1).append("&Show All Items", [this](const nana::menu::item_proxy & prx) {
			if(this->loaded_list()) {
				results_.clear();
				this->draw_results(true);
			}
			else {
				this->report_error(market_error_type::no_list_read);
				return;
			}
		});
		menubar_.at(1).append("&Add Item", [this](const nana::menu::item_proxy & prx) {
			if(this->loaded_list()) {
				results_.clear();
				this->add_item();
			}
			else {
				this->report_error(market_error_type::no_list_read);
				return;
			}
		});
		menubar_.at(1).append("&Edit Item", [this](const nana::menu::item_proxy & prx) {
			if(this->loaded_list()) {
				results_.clear();
				this->draw_results(true);
				this->function_to_call = function::edit;
			}
			else {
				this->report_error(market_error_type::no_list_read);
				return;
			}
		});
		menubar_.at(1).append("&Remove Item", [this](const nana::menu::item_proxy & prx) {
			if(this->loaded_list()) {
				results_.clear();
				this->draw_results(true);
				this->function_to_call = function::remove;
			}
			else {
				this->report_error(market_error_type::no_list_read);
				return;
			}
		});
		menubar_.at(2).append("About", [this](const nana::menu::item_proxy) {
			nana::msgbox a{ "About" };
			this->enabled(false);
			a << "This software is open-source, and currently being developed.\n©Adnan Mukaj";
			a();
			this->enabled(true);
		});

		// EVENT HANDLING
		this->events().destroy([this](const nana::arg_destroy & arg) {
			list_manip::resave();
		});

		searchbar_.events().text_changed([this](const nana::arg_textbox & arg) {
			results_.clear();
			selected_result_code_.clear();
			selected_item_code_.clear();
			if(searchbar_.caption() == "") {
				return;
			}
			else if(!loaded_item_list_) {
				error_message_ = "In order to make items appear you need to select a file to read items from.";
				this->report_error();
				searchbar_.caption("");
			}
			else {
				search::search(searchbar_.caption(), error_message_);
				if(!error_message_.empty())
					this->report_error();
				this->draw_results();
			}
		});

		cart_.events().selected([this](const nana::arg_listbox & arg) {
			if(!selected_item_) {
				selected_item_code_ = arg.item.text(0);
				selected_item_ = true;
			}
			else {
				selected_item_code_.clear();
				selected_item_ = false;
			}
		});

		cart_.events().dbl_click([this](const nana::arg_mouse & arg) {
			if(!selected_item_code_.empty()) {
				if(arg.left_button) {
					cart::remove_from_cart(selected_item_code_, error_message_);
				}
				else {
					cart::remove_from_cart(selected_item_code_, error_message_, true);
				}
				selected_item_code_.clear();
				selected_item_ = false;
				this->draw_cart();
			}
		});

		results_.events().selected([this](const nana::arg_listbox & arg) {
			if(selected_result_code_.empty()) {
				selected_result_code_ = arg.item.text(0);
			}
			else {
				selected_result_code_.clear();
			}
		});

		results_.events().dbl_click([this](const nana::arg_mouse & arg) {
			if(!selected_result_code_.empty()) {
				if(!searchbar_.caption().empty()) {
					cart::add_to_cart(selected_result_code_, error_message_);
					this->draw_cart();
					selected_result_code_.clear();
					searchbar_.caption("");
					results_.clear();
				}
				else {
					switch(this->function_to_call) {
						case function::edit:
							this->edit_item(); break;
						case function::remove:
							this->remove_item(); break;
					}
					this->function_to_call = function::none;
				}
			}
			nana::API::focus_window(searchbar_);
		});
	}
}
#endif // !_GUI_H_